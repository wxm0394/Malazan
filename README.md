# The Malazan Book of the Fallen - Tactical Core Engine

> 《玛拉赞落魄之书》（*The Malazan Book of the Fallen*）第一卷《月之花园》（*Gardens of the Moon*）达鲁基斯坦（Darujhistan）战役核心引擎。
> 架构：自研现代 C++20 Sparse-Set ECS + 2.5D Isometric 3D 等距立体图形渲染（基于 SDL3） + 能量时间片驱动 + JSON 数据驱动。

---

## 核心系统与原著还原

1. **立体双层战术高程 (Vertical Elevation)**：
   * **街面层 (Street Level, Z=0)**：鹅卵石小巷、集市广场、不灭蓝火煤气路灯。
   * **屋顶层 (Rooftop Level, Z=1)**：青石斜瓦与露台（飞贼与刺客通道），支持高空俯冲暗杀与高地伤害加成。
2. **多重沃伦法门编织与反噬 (Multi-Warren Channeling)**：
   * 真实还原快班（Quick Ben）的多法门调谐能力（Telas 火、Rashan 阴影、Meanas 幻术、Denul 治愈、Tellann 远古死火）。
   * 施法压力过载会诱发深渊裂隙反噬，剧烈共振将唤醒地底**阿扎斯之屋（Azath House）**。
3. **达鲁基斯坦蓝火煤气管网 (Blue Fire Gas System)**：
   * 街道路灯连通不灭蓝火管线，被火法或莫兰斯炸药（Moranth Munitions）命中将诱发连环殉爆，炸塌周围立体屋顶。
4. **龙之塔罗命理系统 (Deck of Dragons Fate Engine)**：
   * 动态翻开神格卡牌（暗之骑士、暗影绳索、双子运神奥彭、死神胡德、锁链之主），实时重塑全城物理与法术规则。
5. **达鲁基斯坦三幕完整主线 (Three-Act Storyline)**：
   * **第一幕**：暗巷围猎·凤凰客栈保卫战（清除帝国爪牙特工）。
   * **第二幕**：太古坚冰·贾格特暴君雷斯特（Raest）破封降临（破碎其 Omtose Phellack 坚冰护甲）。
   * **第三幕**：万神汇聚·阿扎斯破土永恒封印（古木神迹囚禁暴君）。

---

## 依赖与快速构建

### 依赖环境
* C++20 兼容编译器（推荐 GCC 13+ / GCC 15 / Clang 17+）
* CMake 3.20+
* SDL3 (`libsdl3-dev`)

### 编译与运行
```bash
# 构建引擎
cmake -B build -S malazan-engine
cmake --build build

# 启动 2.5D 高清等距立体战术视口
./build/darujhistan_slice

# 运行全主线逻辑自动化回归验证
./build/darujhistan_slice --headless
```

---

## 战术视口操作快捷键

| 按键 | 功能说明 |
| :--- | :--- |
| `[W, A, S, D / 方向键]` | 控制卡拉姆移动 / 斜瓦间轻功飞跃（每次移动推进世界时间片） |
| `[TAB]` | 切换街面 (Street) / 屋顶 (Rooftop) 立体视口 |
| `[1]` | 指令快班释放【暗影灼热风暴】（Telas + Rashan 复合轰击） |
| `[2]` | 指令卡拉姆掷出莫兰斯燃烧弹诱爆蓝火煤气管网 |
| `[3]` | 屋顶向下俯冲突刺暗杀 |
| `[4]` | 占卜翻开下一张**龙之塔罗牌（Deck of Dragons）** |
| `[5]` | 触发第三幕终局**阿扎斯破土神迹封印** |
| `[SPACE]` | 空过时间片，推进全城敌对 AI 巡逻与感知决策 |
| `[ESC]` | 退出视口 |
