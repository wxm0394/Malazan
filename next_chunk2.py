import sys

def anno(en, zh):
    return f'\\( \\overset{{\\textcolor{{#8b2b2b}}{{\\text{{{zh}}}}}}}{{\\text{{{en}}}}} \\)'

content = f"""
<div class="paragraph">
  "What's this?" a voice {anno("bellowed", "咆哮/大吼")}.
</div>

<div class="paragraph">
  <b>Rigga</b> swung to face the road. An {anno("outrider", "前锋/开路骑兵")} had slowed his mount. The Seer released the girl's hair.
</div>

<div class="paragraph">
  The girl {anno("staggered back", "踉跄后退")} a step. A rock on the road's edge {anno("turned underfoot", "在脚下滚动")} and she fell. When she looked up the {anno("outrider", "开路骑兵")} had {anno("trotted past", "小步跑过")}. Another {anno("thundered up", "轰鸣着冲上前来")} in his {anno("wake", "尾流/后面")}.
</div>

<div class="paragraph">
  "Leave the {anno("pretty one", "漂亮小妞")} alone, hag," this one {anno("growled", "低吼")}, and as he rode by he leaned in his saddle and {anno("swung", "挥出")} an open, {anno("gauntleted", "戴着金属护手")} hand. The {anno("iron-scaled", "铁鳞覆盖的")} glove {anno("cracked against", "重重击中")} <b>Rigga</b>'s head, {anno("spinning her around", "把她打得原地转圈")}. She {anno("toppled", "倒下")}.
</div>

<div class="paragraph">
  The fishergirl screamed as <b>Rigga</b> landed heavily across her {anno("thighs", "大腿")}. A thread of {anno("crimson spit", "猩红的唾沫")} {anno("spattered", "溅在")} her face. {anno("Whimpering", "呜咽着")}, the girl pushed herself back across the {anno("gravel", "碎石")}, then used her feet to {anno("shove away", "推开")} <b>Rigga</b>'s body. She climbed to her knees.
</div>

<div class="paragraph">
  Something within <b>Rigga</b>'s prophecy seemed {anno("lodged", "卡住/驻留")} in the girl's head, heavy as a stone and hidden from {anno("light", "光芒")}. She found she could not {anno("retrieve", "找回/回想")} a single word the Seer had said. She {anno("reached out", "伸出手")} and {anno("grasped", "紧抓")} <b>Rigga</b>'s woolen shawl. Carefully, she {anno("rolled the old woman over", "把老妇人翻过来")}. Blood covered one side of <b>Rigga</b>'s head, {anno("running down", "流淌")}, behind the ear. More blood {anno("smeared", "涂抹/弄脏")} her lined chin and {anno("stained", "染红")} her mouth. The eyes stared {anno("sightlessly", "空洞地/没有视力地")}.
</div>

<div class="paragraph">
  The fishergirl {anno("pulled back", "退缩")}, unable to {anno("catch her breath", "喘上气来")}. {anno("Desperate", "绝望中")}, she looked about. The column of soldiers had passed, leaving nothing but dust and the distant {anno("tremble", "震颤")} of hoofs. <b>Rigga</b>'s bag of turnips had {anno("spilled", "撒落")} onto the road. Among the {anno("trampled", "被践踏的")} vegetables lay five {anno("tallow", "动物油脂制作的")} candles. The girl managed a {anno("ragged lungful", "破布般粗重的深呼吸")} of dusty air. Wiping her nose, she looked to her own basket.
</div>

<div class="paragraph">
  "Never mind the candles," she mumbled, in a thick, odd voice. "They're gone, aren't they, now? Just a {anno("scattering", "散落")} of bones. Never mind." She crawled toward the bundles of twine that had fallen from the {anno("breached", "破裂的")} basket, and when she spoke again her voice was young, normal. "We need the twine. We'll {anno("work all night", "干一整夜")} and {anno("get one ready", "准备好一张网")}. Dadda's waiting. He's right at the door, he's looking up the track, he's waiting to see me."
</div>

<div class="paragraph">
  She stopped, a {anno("shiver", "颤抖")} {anno("running through her", "穿过全身")}. The sun's light was almost gone. An {anno("unseasonal chill", "反常的寒意")} {anno("bled from", "从...渗出")} the shadows, which now {anno("flowed like water", "如水般流淌")} across the road.
</div>

<div class="paragraph">
  "Here it comes, then," the girl {anno("grated", "摩擦着嗓子说")} softly, in a voice that wasn't her own.
</div>
"""

with open("temp_output2.html", "w", encoding="utf-8") as f:
    f.write(content)

