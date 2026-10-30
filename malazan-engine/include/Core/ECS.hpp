#pragma once
#include <vector>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <cstdint>
#include <cassert>
#include <tuple>
#include <algorithm>
#include <limits>

namespace Malazan {

using Entity = uint32_t;
constexpr Entity NULL_ENTITY = 0xFFFFFFFF;

// 稀疏集接口基类
class ISparsePool {
public:
    virtual ~ISparsePool() = default;
    virtual void remove(Entity entity) = 0;
    virtual bool has(Entity entity) const = 0;
    [[nodiscard]] virtual size_t size() const = 0;
    [[nodiscard]] virtual const std::vector<Entity>& entities() const = 0;
};

// 具体的组件稀疏集存储（Data-Oriented Design，内存密集连续）
template <typename T>
class SparsePool : public ISparsePool {
public:
    template <typename... Args>
    T& emplace(Entity entity, Args&&... args) {
        if (has(entity)) {
            size_t dense_idx = m_sparse[entity];
            m_dense[dense_idx] = T{std::forward<Args>(args)...};
            return m_dense[dense_idx];
        }

        if (entity >= m_sparse.size()) {
            m_sparse.resize(entity + 1, NULL_ENTITY);
        }

        size_t dense_idx = m_dense.size();
        m_sparse[entity] = static_cast<uint32_t>(dense_idx);
        m_dense_entities.push_back(entity);
        m_dense.emplace_back(std::forward<Args>(args)...);

        return m_dense.back();
    }

    void remove(Entity entity) override {
        if (!has(entity)) return;

        size_t dense_idx = m_sparse[entity];
        size_t last_idx = m_dense.size() - 1;

        if (dense_idx != last_idx) {
            // Swap with last element for O(1) removal
            Entity last_entity = m_dense_entities[last_idx];
            m_dense[dense_idx] = std::move(m_dense[last_idx]);
            m_dense_entities[dense_idx] = last_entity;
            m_sparse[last_entity] = static_cast<uint32_t>(dense_idx);
        }

        m_dense.pop_back();
        m_dense_entities.pop_back();
        m_sparse[entity] = NULL_ENTITY;
    }

    [[nodiscard]] bool has(Entity entity) const override {
        return entity < m_sparse.size() && m_sparse[entity] != NULL_ENTITY;
    }

    [[nodiscard]] T& get(Entity entity) {
        assert(has(entity) && "Entity does not have component!");
        return m_dense[m_sparse[entity]];
    }

    [[nodiscard]] const T& get(Entity entity) const {
        assert(has(entity) && "Entity does not have component!");
        return m_dense[m_sparse[entity]];
    }

    [[nodiscard]] size_t size() const override {
        return m_dense.size();
    }

    [[nodiscard]] const std::vector<Entity>& entities() const override {
        return m_dense_entities;
    }

    [[nodiscard]] std::vector<T>& dense() {
        return m_dense;
    }

private:
    std::vector<T> m_dense;
    std::vector<Entity> m_dense_entities;
    std::vector<uint32_t> m_sparse;
};

// 实体组件注册表 (ECS Registry)
class Registry {
public:
    Registry() = default;

    Entity create_entity() {
        Entity id = m_next_entity_id++;
        m_active_entities.push_back(id);
        return id;
    }

    void destroy_entity(Entity entity) {
        for (auto& [type, pool] : m_pools) {
            pool->remove(entity);
        }
        std::erase(m_active_entities, entity);
    }

    template <typename T, typename... Args>
    T& emplace(Entity entity, Args&&... args) {
        return get_or_create_pool<T>().emplace(entity, std::forward<Args>(args)...);
    }

    template <typename T>
    void remove(Entity entity) {
        get_or_create_pool<T>().remove(entity);
    }

    template <typename T>
    [[nodiscard]] bool has(Entity entity) {
        return get_or_create_pool<T>().has(entity);
    }

    template <typename T>
    [[nodiscard]] T& get(Entity entity) {
        return get_or_create_pool<T>().get(entity);
    }

    // 视图类：用于高效遍历同时拥有指定组件的实体集合
    template <typename... Comps>
    class MultiView {
    public:
        MultiView(Registry* reg) : m_reg(reg) {
            // 找到包含元素最少的组件池以优化遍历迭代次数
            find_smallest_pool();
        }

        struct Iterator {
            Registry* reg;
            const std::vector<Entity>* primary_entities;
            size_t index;

            Iterator(Registry* r, const std::vector<Entity>* ents, size_t idx)
                : reg(r), primary_entities(ents), index(idx) {
                advance_to_valid();
            }

            void advance_to_valid() {
                if (!primary_entities) return;
                while (index < primary_entities->size()) {
                    Entity ent = (*primary_entities)[index];
                    if ((reg->has<Comps>(ent) && ...)) {
                        break;
                    }
                    index++;
                }
            }

            Iterator& operator++() {
                index++;
                advance_to_valid();
                return *this;
            }

            bool operator!=(const Iterator& other) const {
                return index != other.index;
            }

            auto operator*() const {
                Entity ent = (*primary_entities)[index];
                return std::tuple<Entity, Comps&...>(ent, reg->get<Comps>(ent)...);
            }
        };

        Iterator begin() {
            if (!m_primary_pool) return Iterator(m_reg, nullptr, 0);
            return Iterator(m_reg, &m_primary_pool->entities(), 0);
        }

        Iterator end() {
            if (!m_primary_pool) return Iterator(m_reg, nullptr, 0);
            return Iterator(m_reg, &m_primary_pool->entities(), m_primary_pool->entities().size());
        }

    private:
        void find_smallest_pool() {
            size_t min_size = std::numeric_limits<size_t>::max();
            auto check_pool = [&]<typename C>() {
                auto& pool = m_reg->get_or_create_pool<C>();
                if (pool.size() < min_size) {
                    min_size = pool.size();
                    m_primary_pool = &pool;
                }
            };
            (check_pool.template operator()<Comps>(), ...);
        }

        Registry* m_reg{nullptr};
        ISparsePool* m_primary_pool{nullptr};
    };

    template <typename... Comps>
    [[nodiscard]] MultiView<Comps...> view() {
        return MultiView<Comps...>(this);
    }

private:
    template <typename T>
    SparsePool<T>& get_or_create_pool() {
        std::type_index idx = typeid(T);
        auto it = m_pools.find(idx);
        if (it == m_pools.end()) {
            auto pool = std::make_unique<SparsePool<T>>();
            auto* raw = pool.get();
            m_pools[idx] = std::move(pool);
            return *raw;
        }
        return *static_cast<SparsePool<T>*>(it->second.get());
    }

    Entity m_next_entity_id{1};
    std::vector<Entity> m_active_entities;
    std::unordered_map<std::type_index, std::unique_ptr<ISparsePool>> m_pools;
};

} // namespace Malazan
