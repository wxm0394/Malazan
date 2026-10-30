import sys

def anno(en, zh):
    return f'\\( \\overset{{\\textcolor{{#8b2b2b}}{{\\text{{{zh}}}}}}}{{\\text{{{en}}}}} \\)'

content = f"""
<div class="paragraph">
  "That was in the days of the <b>Emperor</b>," the hag continued. "<b>Hood</b> {anno("roast the bastard's soul on a spit", "把那混蛋的灵魂穿在烤叉上烤")}. But {anno("look on", "继续看吧")}, lass. <b>Laseen</b> {anno("scatters bones with the best of them", "比起杀人如麻也不遑多让")}. Heh, she started with his, didn't she, now?"
</div>

<div class="paragraph">
  The fishergirl nodded {anno("faintly", "微弱地")}. As {anno("befitted", "符合...的身份")} the {anno("lowborn", "出身低微的人")}, they waited by the roadside, the old woman {anno("burdened beneath", "吃力地背着")} a rough sack filled with {anno("turnips", "萝卜")}, the girl with a heavy basket {anno("balanced", "顶/平衡")} on her head. Every minute or so the old woman {anno("shifted", "挪动/替换")} the sack from one {anno("bony", "骨瘦如柴的")} shoulder to the other. With the riders {anno("crowding", "挤压")} them on the road and the ditch behind them a {anno("steep drop", "陡坡")} to broken rocks, she had no place to {anno("put down", "放下")} the sack.
</div>

<div class="paragraph">
  "Scatters bones, I said. Bones of husbands, bones of sons, bones of wives and bones of daughters. {anno("All the same", "全都一样")} to her. {anno("All the same", "全都一样")} to the Empire." The old woman spat a second time. "Three husbands and two sons, ten coin {anno("apiece", "每人")} a year. Five {anno("of ten's", "个十就是")} fifty. Fifty coin a year's {anno("cold company", "冰冷的陪伴")}, lass. Cold in winter, cold in bed."
</div>

<div class="paragraph">
  The fishergirl wiped dust from her forehead. Her bright eyes {anno("darted", "飞快扫视")} among the soldiers passing before her. The young men {anno("atop", "在...之上")} their {anno("high-backed saddles", "高背马鞍")} held expressions {anno("stern", "严厉的/坚毅的")} and fixed straight ahead. The few women who rode among them sat tall and somehow {anno("fiercer", "比...更凶猛")} than the men. The sunset cast red {anno("glints", "闪光")} from their {anno("helms", "头盔")}, flashing so that the girl's eyes {anno("stung", "刺痛")} and her vision {anno("blurred", "变得模糊")}.
</div>

<div class="paragraph">
  "You're the fisherman's daughter," the old woman said. "I seen you {anno("afore", "以前")} on the road, and down on the {anno("strand", "海滩/岸边")}. Seen you and your dad at market. Missing an arm, ain't he? More bones for her collection {anno("is likely", "十有八九")}, eh?" She made a {anno("chopping motion", "挥砍动作")} with one hand, then nodded. "Mine's the first house on the {anno("track", "小径")}. I use the coin to buy candles. Five candles I burn every night, five candles to keep old <b>Rigga</b> company. It's a tired house, full of tired things and me one of them, lass. What you got in the basket there?"
</div>

<div class="paragraph">
  Slowly the fishergirl realized that a question {anno("had been asked of her", "在问她")}. She {anno("pulled her attention from", "把注意力从...移开")} the soldiers and smiled down at the old woman. "I'm sorry," she said, "the horses are so loud."
</div>

<div class="paragraph">
  <b>Rigga</b> raised her voice. "I asked what you got in your basket, lass?"
</div>

<div class="paragraph">
  "Twine. Enough for three nets. We need to {anno("get one ready", "准备好一个")} for tomorrow. Dadda lost his last one something in the {anno("deep waters", "深水区")} took it and a {anno("whole catch", "全部渔获")}, too. Ilgrand Lender wants the money he loaned us and we need a catch tomorrow. A good one." She smiled again and {anno("swept her gaze back to", "把目光扫回到")} the soldiers. "Isn't it wonderful?" she breathed.
</div>

<div class="paragraph">
  <b>Rigga</b>'s hand {anno("shot out", "猛地伸出")} and {anno("snagged", "紧紧抓住")} the girl's thick black hair, {anno("yanked it hard", "用力猛拽")}.
</div>

<div class="paragraph">
  The girl cried out. The basket on her head {anno("lurched", "猛烈倾斜")}, then {anno("slid down", "滑落")} onto one shoulder. She {anno("grabbed frantically for it", "拼命去抓")} but it was too heavy. The basket {anno("struck", "撞击")} the ground and {anno("split apart", "裂开")}.
</div>

<div class="paragraph">
  "Aaai!" the girl {anno("gasped", "喘息着惊呼")}, {anno("attempting to kneel", "试图跪下")}. But <b>Rigga</b> pulled and {anno("snapped her head around", "把她的头猛转过来")}.
</div>

<div class="paragraph">
  "You listen to me, lass!" The old woman's {anno("sour breath", "酸臭的气息")} {anno("hissed", "嘶嘶作响")} against the girl's face. "The Empire's been {anno("grinding this land down", "碾压/压榨这片土地")} for a hundred years. You was born in it. I wasn't. When I was your age <u>Itko Kan</u> was a country. We {anno("flew a banner", "飘扬着自己的旗帜")} and it was ours. We were free, lass."
</div>

<div class="paragraph">
  The girl was {anno("sickened", "感到恶心")} by <b>Rigga</b>'s breath. She {anno("squeezed shut", "紧紧闭上")} her eyes.
</div>

<div class="paragraph">
  "Mark this truth, child, else the {anno("Cloak of Lies", "谎言之篷")} blinds you forever." <b>Rigga</b>'s voice {anno("took on", "带上了")} a {anno("droning cadence", "嗡嗡的/单调的韵律")}, and {anno("all at once", "突然间")} the girl {anno("stiffened", "僵住了")}. <b>Rigga</b>, <b>Riggalai the Seer</b>, the {anno("wax-witch", "蜡巫")} who trapped souls in candles and burned them. Souls {anno("devoured in flame", "在火焰中被吞噬")}. <b>Rigga</b>'s words {anno("carried the chilling tone", "带着令人胆寒的语调")} of {anno("prophecy", "预言")}. "Mark this truth. I am the last to speak to you. You are the last to hear me. Thus are we linked, you and I, {anno("beyond all else", "超越一切")}."
</div>

<div class="paragraph">
  <b>Rigga</b>'s fingers {anno("snagged tighter", "抓得更紧")} in the girl's hair. "Across the sea the <b>Empress</b> has {anno("driven her knife into virgin soil", "把刀刺入未开垦的处女地")}. The blood now comes in a {anno("tide", "狂潮")} and it'll {anno("sweep you under", "把你卷入水底")}, child, if you're not careful. They'll put a sword in your hand, they'll give you a fine horse, and they'll send you across that sea. But a shadow will {anno("embrace your soul", "拥抱/占据你的灵魂")}. Now, listen! {anno("Bury this deep!", "把它深深埋在心底！")} <b>Rigga</b> will {anno("preserve", "保护/保存")} you because we are linked, you and I. But it is all I can do, understand? {anno("Look to", "指望/寻找")} the Lord {anno("spawned", "诞生/繁衍")} in Darkness; his is the hand that shall free you, though he'll {anno("know it not", "对此毫不知情")}."
</div>
"""

with open("temp_output.html", "w", encoding="utf-8") as f:
    f.write(content)

