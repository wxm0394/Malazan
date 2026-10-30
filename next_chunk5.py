import sys

def anno(en, zh, pos=""):
    if pos:
        zh_text = f"{zh} {pos}"
    else:
        zh_text = zh
    return f'\\( \\overset{{\\textcolor{{#8b2b2b}}{{\\text{{{zh_text}}}}}}}{{\\text{{{en}}}}} \\)'

content = f"""
<div class="paragraph">
  The captain {anno("reached up", "抬起手")} and {anno("unstrapped", "解开带子", "v.")} his helm. He lifted it slowly from his head, then set it down over his {anno("saddle horn", "马鞍桩/鞍前突起")}. "Adjunct..."
</div>

<div class="paragraph">
  "I am named <b>Lorn</b>," the woman said softly.
</div>

<div class="paragraph">
  "One hundred and seventy-five men and women. Two hundred and ten horses. The Nineteenth {anno("Regiment", "团", "n.")} of the <u>Itko Kanese</u> Eighth {anno("Cavalry", "骑兵", "n.")}." The captain's throat {anno("tightened", "发紧", "v.")} briefly. He looked at <b>Lorn</b>. "Dead." His horse {anno("shied", "受惊退缩", "v.")} under him as it caught an {anno("updraft", "上升气流", "n.")}. He {anno("closed savagely", "猛地收紧")} on the {anno("reins", "缰绳", "n.")} and the animal {anno("stilled", "安静下来", "v.")}, {anno("nostrils", "鼻孔", "n.")} wide and ears back, muscles {anno("trembling", "颤抖", "v.")} under him. The <b>Adjunct</b>'s {anno("stallion", "种马/公马", "n.")} made no move. "All had their weapons {anno("bared", "拔出", "v.")}. All fought whatever enemy {anno("attacked", "攻击", "v.")} them. But the dead are all ours."
</div>

<div class="paragraph">
  "You've checked the beach below?" <b>Lorn</b> asked, still {anno("staring down", "俯视")} on the road.
</div>

<div class="paragraph">
  "No signs of a {anno("landing", "登陆", "n.")}," the captain replied. "No tracks anywhere, neither {anno("seaward", "向海的", "adv.")} nor inland. There are more dead than these. Adjunct. Farmers, {anno("peasants", "乡下人/农民", "n.")}, {anno("fisherfolk", "渔民", "n.")}, travelers on the road. All of them {anno("torn apart", "被撕碎")}, limbs {anno("scattered", "散落", "v.")}... children, {anno("livestock", "牲畜", "n.")}, dogs." He stopped {anno("abruptly", "突然地", "adv.")} and {anno("turned away", "背过身去")}. "Over four hundred dead," he {anno("grated", "咬牙说道", "v.")}. "We're not {anno("certain of", "确定")} the exact {anno("count", "总数", "n.")}."
</div>

<div class="paragraph">
  "Of course," <b>Lorn</b> said, her tone {anno("devoid of", "没有任何...")} feeling. "No witnesses?"
</div>

<div class="paragraph">
  "None."
</div>

<div class="paragraph">
  A man was riding toward them on the road below, {anno("leaning close", "俯身贴近")} to his horse's ear as he {anno("talked the frightened animal through", "柔声安抚受惊的动物走过")} the {anno("carnage", "大屠杀/屠宰场", "n.")}. Birds rose in {anno("shrieking", "尖叫的", "adj.")} {anno("complaint", "抱怨", "n.")} in front of him, {anno("settling", "落下", "v.")} again once he had passed.
</div>

<div class="paragraph">
  "Who is that?" the <b>Adjunct</b> asked.
</div>

<div class="paragraph">
  The captain {anno("grunted", "咕哝", "v.")}. "Lieutenant <b>Ganoes Paran</b>. He's new to my {anno("command", "指挥部/麾下", "n.")}. From <u>Unta</u>."
</div>

<div class="paragraph">
  <b>Lorn</b>'s eyes {anno("narrowed", "眯起", "v.")} on the young man. He'd reached the edge of the {anno("depression", "洼地/坑洼", "n.")}, stopping to {anno("relay", "传达", "v.")} orders to the work crews. He {anno("leaned back", "向后靠")} in his saddle then and {anno("glanced", "瞥见", "v.")} in their direction. "<b>Paran</b>. From House <b>Paran</b>?"
</div>

<div class="paragraph">
  "Aye, gold in his veins and all that."
</div>

<div class="paragraph">
  "Call him up here."
</div>

<div class="paragraph">
  The captain {anno("gestured", "打手势", "v.")} and the lieutenant kicked his mount's {anno("flanks", "侧腹", "n.")}. Moments later he {anno("reined in", "勒马停下")} beside the captain and {anno("saluted", "敬礼", "v.")}.
</div>
"""

with open("temp_output5.html", "w", encoding="utf-8") as f:
    f.write(content)

