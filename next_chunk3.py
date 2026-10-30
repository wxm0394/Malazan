import sys

def anno(en, zh):
    return f'\\( \\overset{{\\textcolor{{#8b2b2b}}{{\\text{{{zh}}}}}}}{{\\text{{{en}}}}} \\)'

content = f"""
<div class="paragraph">
  A {anno("soft-gloved", "戴着软手套的")} hand fell on her shoulder. She {anno("ducked down", "迅速蹲下")}, {anno("cowering", "畏缩着")}.
</div>

<div class="paragraph">
  "Easy, girl," said a man's voice. "It's over. {anno("Nothing to be done for her now", "现在对她无能为力了/救不了她了")}."
</div>

<div class="paragraph">
  The fishergirl looked up. A man {anno("swathed", "包裹/裹着")} in black leaned over her, his face {anno("obscured", "被遮挡")} beneath a hood's shadow. "But he hit her," the girl said, in a child's voice. "And we have nets to tie, me and Dadda..."
</div>

<div class="paragraph">
  "Let's {anno("get you on your feet", "扶你起来")}," the man said, moving his {anno("long-fingered", "手指修长的")} hands down under her arms. He {anno("straightened", "直起身")}, lifting her {anno("effortlessly", "毫不费力地")}. Her sandaled feet {anno("dangled", "悬吊着/晃荡")} in the air before he set her down.
</div>

<div class="paragraph">
  Now she saw a second man, shorter, also clothed in black. This one stood on the road and was {anno("turned away", "背过身去")}, his gaze in the direction the soldiers had gone. He spoke, his voice {anno("reed-thin", "像芦苇般细弱的")}. "Wasn't much of a life," he said, not turning to face her. "A minor talent, long since {anno("dried up", "枯竭")} of the Gift. Oh, she might have {anno("managed", "勉强做到")} one more, but we'll never know, will we?"
</div>

<div class="paragraph">
  The fishergirl {anno("stumbled over to", "跌跌撞撞地走向")} <b>Rigga</b>'s bag and picked up a candle. She straightened, her eyes suddenly hard, then deliberately {anno("spat on to", "吐口水在")} the road.
</div>

<div class="paragraph">
  The shorter man's head {anno("snapped toward", "猛地转过来向着")} her. Within the hood it seemed the shadows played alone.
</div>

<div class="paragraph">
  The girl {anno("shrank back", "往后缩")} a step, "It was a good life," she whispered. "She had these candles, you see. Five of them. Five for—"
</div>

<div class="paragraph">
  "{anno("Necromancy", "死灵法术/通灵术")}," the short man {anno("cut in", "插嘴/打断")}.
</div>

<div class="paragraph">
  The taller man, still at her side, said softly, "I see them, child. I understand what they mean."
</div>

<div class="paragraph">
  The other man {anno("snorted", "哼了一声")}. "The witch {anno("harbored", "藏有/庇护")} five frail, weak souls. Nothing {anno("grand", "宏大的/了不起的")}." He {anno("cocked", "偏过")} his head. "I can hear them now. Calling for her."
</div>

<div class="paragraph">
  Tears filled the girl's eyes. A {anno("wordless anguish", "无法言语的痛苦")} seemed to {anno("well up from", "从...涌出")} that black stone in her mind. She wiped her cheeks. "Where did you come from?" she asked abruptly. "We didn't see you on the road."
</div>

<div class="paragraph">
  The man beside her {anno("half turned to", "半转过身面向")} the {anno("gravel track", "碎石小径")}. "On the {anno("other side", "另一边/彼岸")}," he said, a smile in his tone. "Waiting, just like you."
</div>

<div class="paragraph">
  The other giggled. "On the other side indeed." He faced down the road again and raised his arms.
</div>

<div class="paragraph">
  The girl {anno("drew in a sharp breath", "猛吸了一口冷气")} as darkness {anno("descended", "降临")}. A loud, {anno("tearing sound", "撕裂声")} filled the air for a moment, then the darkness {anno("dissipated", "消散")} and the girl's eyes widened.
</div>

<div class="paragraph">
  Seven massive Hounds now sat around the man in the road. The eyes of these beasts {anno("glowed", "发光")} yellow, and all were turned in the same direction as the man himself.
</div>

<div class="paragraph">
  She heard him hiss, "{anno("Eager", "渴望的/急切的")}, are we? Then go!"
</div>

<div class="paragraph">
  Silently, the Hounds {anno("bolted down", "猛冲下")} the road.
</div>

<div class="paragraph">
  Their master turned and said to the man beside her, "Something to {anno("gnaw on", "啃咬/折磨")} <b>Laseen</b>'s mind." He giggled again.
</div>

<div class="paragraph">
  "Must you {anno("complicate", "使...复杂化")} things?" the other answered {anno("wearily", "疲惫地")}.
</div>

<div class="paragraph">
  The short man {anno("stiffened", "僵硬/绷紧")}. "They are within sight of the column." He cocked his head. From up the road came the scream of horses. He sighed. "You've {anno("reached a decision", "做出了决定")}, <b>Cotillion</b>?"
</div>

<div class="paragraph">
  The other {anno("grunted amusedly", "饶有兴致地哼了一声")}. "Using my name, <b>Ammanas</b>, means you've just decided for me. We can hardly leave her here now, can we?"
</div>

<div class="paragraph">
  "Of course we can, old friend. Just not breathing."
</div>

<div class="paragraph">
  <b>Cotillion</b> looked down on the girl. "No," he said quietly, "she'll do."
</div>

<div class="paragraph">
  The fishergirl bit her lip. Still {anno("clutching", "紧紧抓住")} <b>Rigga</b>'s candle, she took another step back, her wide eyes {anno("darting", "飞快地扫视")} from one man to the other.
</div>

<div class="paragraph">
  "Pity," <b>Ammanas</b> said.
</div>

<div class="paragraph">
  <b>Cotillion</b> seemed to nod, then he {anno("cleared his throat", "清了清嗓子")} and said, "It'll take time."
</div>

<div class="paragraph">
  An amused note {anno("entered", "加入了")} <b>Ammanas</b>'s reply. "And have we time? True vengeance needs the slow, careful {anno("stalking", "潜行追踪")} of the victim. Have you forgotten the pain she once {anno("delivered us", "带给我们")}? <b>Laseen</b>'s back is against the wall already. She might fall without our help. Where would be the satisfaction in that?"
</div>

<div class="paragraph">
  <b>Cotillion</b>'s response was cool and dry. "You've always {anno("underestimated", "低估")} the Empress. Hence our present {anno("circumstances", "处境/情况")}... No." He {anno("gestured at", "对...做手势")} the fishergirl. "We'll need this one. <b>Laseen</b>'s {anno("raised the ire", "激起了怒火")} of Moon's Spawn, and that's a {anno("hornet's nest", "马蜂窝")} if ever there was one. The timing is perfect."
</div>

<div class="paragraph">
  Faintly, above the screaming horses, came the {anno("shrieks", "尖叫声")} of men and women, a sound that {anno("pierced", "刺透")} the girl's heart. Her eyes darted to <b>Rigga</b>'s motionless form on the roadside, then back to <b>Ammanas</b>, who now approached her. She thought to scream, but the wordless anguish {anno("reared up", "腾起/涌起")} again, {anno("stifling", "令人窒息的/压抑的")}, and her vision went black.
</div>
"""

with open("temp_output3.html", "w", encoding="utf-8") as f:
    f.write(content)

