import sys

def anno(en, zh, pos=""):
    if pos:
        zh_text = f"{zh} {pos}"
    else:
        zh_text = zh
    return f'\\( \\overset{{\\textcolor{{#8b2b2b}}{{\\text{{{zh_text}}}}}}}{{\\text{{{en}}}}} \\)'

content = f"""
<div class="paragraph">
  "I {anno("suspect", "怀疑", "v.")}," <b>Cotillion</b> said, "{anno("greed", "贪婪", "n.")} will {anno("suffice", "足够/满足", "v.")}, once the {anno("slate is wiped clean", "旧账一笔勾销")}. {anno("Sarcasm", "讥讽", "n.")} filled his next words. "I'm sure you can {anno("manage", "处理/应付", "v.")} the {anno("sorcery", "巫术", "n.")} in that, can't you?"
</div>

<div class="paragraph">
  <b>Ammanas</b> giggled. "{anno("Beware", "当心", "v.")} of shadows {anno("bearing", "带来", "v.")} gifts."
</div>

<div class="paragraph">
  <b>Cotillion</b> faced the girl again. He lifted his arms out to the sides. The shadows that held his {anno("features", "面容/特征", "n.")} in darkness now flowed out around his body.
</div>

<div class="paragraph">
  <b>Ammanas</b> spoke, and to the girl his words seemed to come from a great distance. "She's {anno("ideal", "理想的", "adj.")}. The <b>Empress</b> could never {anno("track her down", "追踪到她")}, could never even {anno("so much as", "连...都不(至于)")} guess." He raised his voice. "It's not so bad a thing, lass, to be the {anno("pawn", "棋子", "n.")} of a god."
</div>

<div class="paragraph">
  "Prod and pull," the fishergirl said quickly.
</div>

<div class="paragraph">
  <b>Cotillion</b> {anno("hesitated", "犹豫", "v.")} at her strange comment, then he {anno("shrugged", "耸肩", "v.")}. The shadows {anno("whirled", "旋转", "v.")} out to {anno("engulf", "吞噬", "v.")} the girl. With their cold touch her mind {anno("fell away", "消失/剥落")}, down into darkness. Her last {anno("fleeting", "短暂的/飞逝的", "adj.")} {anno("sensation", "感觉", "n.")} was of the soft wax of the candle in her right hand, and how it seemed to {anno("well up", "涌出/溢出")} between the fingers of her {anno("clenched", "紧握的", "adj.")} fist.
</div>

<div class="paragraph">
  The captain {anno("shifted", "挪动", "v.")} in his saddle and {anno("glanced", "瞥见", "v.")} at the woman riding beside him. "We've {anno("closed the road", "封路")} on both sides. Adjunct. Moved the local {anno("traffic", "交通/行人", "n.")} {anno("inland", "向内陆", "adv.")}. So far, no word's {anno("leaked", "泄露", "v.")}." He wiped sweat from his {anno("brow", "额头", "n.")} and {anno("winced", "退缩/皱眉", "v.")}. The hot woolen cap beneath his {anno("helm", "头盔", "n.")} had {anno("rubbed", "摩擦", "v.")} his forehead {anno("raw", "破皮/擦伤", "adj.")}.
</div>

<div class="paragraph">
  "Something wrong, Captain?"
</div>

<div class="paragraph">
  He shook his head, {anno("squinting", "眯着眼看", "v.")} up the road. "Helmet's loose. Had more hair the last time I wore it."
</div>

<div class="paragraph">
  The {anno("Adjunct", "副官", "n.")} to the <b>Empress</b> did not reply.
</div>

<div class="paragraph">
  The {anno("mid-morning", "仲晨", "n.")} sun made the road's white, dusty surface almost {anno("blinding", "令人目眩的", "adj.")}. The captain felt sweat {anno("running down", "流下")} his body, and the {anno("mail", "锁子甲", "n.")} of his helm's {anno("lobster tail", "龙虾尾(头盔后摆)")} kept {anno("nipping", "夹", "v.")} the hairs on his neck. Already his lower back {anno("ached", "疼痛", "v.")}. It had been years since he'd last ridden a horse, and the {anno("roll", "策马的律动", "n.")} was {anno("slow in coming", "迟迟无法适应")}. With every {anno("saddle-bounce", "马鞍颠簸")} he felt {anno("vertebrae", "脊椎骨", "n.")} {anno("crunch", "嘎吱作响", "v.")}.
</div>

<div class="paragraph">
  It had been a long time since somebody's {anno("title", "头衔", "n.")} had been enough to {anno("straighten him up", "让他挺直腰板")}. But this was the <b>Adjunct</b> to the <b>Empress</b>, <b>Laseen</b>'s personal {anno("servant", "仆人/心腹", "n.")}, an {anno("extension", "延伸", "n.")} of her <b>Imperial</b> will. The last thing the captain wanted was to show his {anno("misery", "痛苦", "n.")} to this young, dangerous woman.
</div>

<div class="paragraph">
  Up ahead the road began its long, {anno("winding", "蜿蜒的", "adj.")} {anno("ascent", "上升/攀登", "n.")}. A {anno("salty", "咸的", "adj.")} wind blew from their left, {anno("whistling", "呼啸", "v.")} through the {anno("newly budding", "刚发芽的")} trees lining that side of the road. By {anno("mid-afternoon", "午后", "n.")}, that wind would breathe hot as a baker's oven, {anno("carrying with it", "随之带来")} the {anno("stench", "恶臭", "n.")} of the {anno("mudflats", "泥滩", "n.")}. And the sun's heat would bring something else as well. The captain hoped to be back in <u>Kan</u> by then.
</div>

<div class="paragraph">
  He tried not to think about the place they rode toward. Leave that to the <b>Adjunct</b>. In his years of service to the Empire, he'd seen enough to know when to {anno("shut everything down", "封闭一切感知")} inside his {anno("skull", "头盖骨", "n.")}. This was one of those times.
</div>

<div class="paragraph">
  The <b>Adjunct</b> spoke. "You've been {anno("stationed", "驻扎", "v.")} here long, Captain?"
</div>

<div class="paragraph">
  "Aye," the man {anno("growled", "低吼", "v.")}.
</div>

<div class="paragraph">
  The woman waited, then asked, "How long?"
</div>

<div class="paragraph">
  He {anno("hesitated", "犹豫", "v.")}. "Thirteen years. Adjunct."
</div>

<div class="paragraph">
  "You fought for the <b>Emperor</b>, then," she said.
</div>

<div class="paragraph">
  "Aye."
</div>

<div class="paragraph">
  "And {anno("survived", "幸存", "v.")} the {anno("purge", "大清洗", "n.")}."
</div>

<div class="paragraph">
  The captain {anno("threw her a look", "瞥了她一眼")}. If she felt his {anno("gaze", "注视", "n.")}, she gave no {anno("indication", "迹象/表示", "n.")}. Her eyes remained on the road ahead; she rolled easily in the saddle, the {anno("scabbarded", "装在鞘里的", "adj.")} {anno("longsword", "长剑", "n.")} {anno("hitched high", "高高挂起")} under her left arm, ready for {anno("mounted battle", "马战")}. Her hair was either cut short or {anno("drawn up", "盘起")} under her {anno("helm", "头盔", "n.")}. Her {anno("figure", "身段/体型", "n.")} was {anno("lithe", "柔软的/轻盈的", "adj.")} enough, the captain {anno("mused", "沉思/暗想", "v.")}.
</div>

<div class="paragraph">
  "Finished?" she asked. "I was asking about the {anno("purges", "清洗", "n.")} {anno("commanded", "下令", "v.")} by <b>Empress Laseen</b> following her {anno("predecessor's", "前任的", "n.")} {anno("untimely", "过早的/不合时宜的", "adj.")} death."
</div>

<div class="paragraph">
  The captain {anno("gritted", "咬紧", "v.")} his teeth, ducked his chin to {anno("draw up", "拉紧")} the helm's strap he hadn't had time to {anno("shave", "刮胡子", "v.")} and the {anno("buckle", "扣环", "n.")} was {anno("chafing", "摩擦破皮", "v.")}. "Not everyone was killed. Adjunct. The people of <u>Itko Kan</u> aren't exactly {anno("excitable", "易激动的", "adj.")}. None of those {anno("riots", "暴乱", "n.")} and mass {anno("executions", "处决", "n.")} that hit other parts of the Empire. We all just {anno("sat tight", "按兵不动")} and waited."
</div>

<div class="paragraph">
  "I {anno("take it", "认为/推断")}," the <b>Adjunct</b> said, with a slight smile, "you're not {anno("noble-born", "贵族出身的", "adj.")}. Captain."
</div>

<div class="paragraph">
  He {anno("grunted", "咕哝", "v.")}. "If I'd been noble-born, I wouldn't have survived, even here in <u>Itko Kan</u>. We both know that. Her orders were {anno("specific", "明确的", "adj.")}, and even the {anno("droll", "滑稽的/古怪的", "adj.")} Kanese didn't dare {anno("disobey", "违抗", "v.")} the <b>Empress</b>." He {anno("scowled", "皱眉怒视", "v.")}. "No, {anno("up through the ranks", "从小兵一路升上来")}. Adjunct."
</div>

<div class="paragraph">
  "Your last {anno("engagement", "交战/战役", "n.")}?"
</div>

<div class="paragraph">
  "<u>Wickan Plains</u>."
</div>

<div class="paragraph">
  They rode on in silence for a time, passing the {anno("occasional", "偶尔的", "adj.")} soldier stationed on the road. Off to their left the trees {anno("fell away", "逐渐稀疏/消失")} to {anno("ragged", "参差不齐的", "adj.")} {anno("heather", "石楠花", "n.")}, and the sea beyond showed its {anno("white-capped", "白浪滔天的", "adj.")} {anno("expanse", "广阔区域", "n.")}. The <b>Adjunct</b> spoke. "This area you've {anno("contained", "控制/封锁", "v.")}, how many of your guard have you {anno("deployed", "部署", "v.")} to patrol it?"
</div>

<div class="paragraph">
  "Eleven hundred," the captain replied.
</div>

<div class="paragraph">
  Her head turned at this, her cool {anno("gaze", "凝视", "n.")} {anno("tightening", "收紧", "v.")} beneath the {anno("rim", "边缘", "n.")} of her helm.
</div>

<div class="paragraph">
  The captain studied her {anno("expression", "表情", "n.")}. "The {anno("carnage", "大屠杀", "n.")} stretches half a {anno("league", "里格(距离单位)", "n.")} from the sea. Adjunct, and a {anno("quarter-league", "四分之一里格")} inland."
</div>

<div class="paragraph">
  The woman said nothing.
</div>

<div class="paragraph">
  They {anno("approached", "接近", "v.")} the {anno("summit", "顶点", "n.")}. A {anno("score", "二十个(左右)", "n.")} of soldiers had gathered there, and others waited along the slope's {anno("rise", "隆起/斜坡", "n.")}. All had turned to watch them.
</div>

<div class="paragraph">
  "Prepare yourself. Adjunct."
</div>

<div class="paragraph">
  The woman studied the faces {anno("lining", "排列在...两边", "v.")} the roadside. She knew these to be {anno("hardened", "久经沙场的", "adj.")} men and women, {anno("veterans", "老兵", "n.")} of the {anno("siege", "围攻", "n.")} of <u>Li Heng</u> and the <u>Wickan Wars</u> out on the north plains. But something had been {anno("clawed", "抓出/刻入", "v.")} into their eyes that had left them {anno("raw", "擦伤的/脆弱的", "adj.")} and {anno("exposed", "毫无防备的", "adj.")}. They looked upon her with a {anno("yearning", "渴望", "n.")} that she found {anno("disturbing", "令人不安的", "adj.")}, as if they {anno("hungered", "饥渴", "v.")} for answers. She fought the {anno("urge", "冲动", "n.")} to speak to them as she passed, to offer whatever comforting words she could. Such gifts were not hers to give, however, nor had they ever been. In this she was much the same as the <b>Empress</b>.
</div>

<div class="paragraph">
  From beyond the {anno("summit", "山顶", "n.")} she heard the cries of gulls and crows, a sound that rose into a {anno("high-pitched", "高音调的", "adj.")} {anno("roar", "轰鸣/喧嚣", "n.")} as they reached the {anno("crest", "顶点/山脊", "n.")}. {anno("Ignoring", "无视", "v.")} the soldiers on either side, the <b>Adjunct</b> moved her horse forward. The captain followed. They came to the crest and looked down. The road {anno("dipped", "下沉/倾斜", "v.")} here for perhaps a fifth of a league, climbing again at the far end to a {anno("promontory", "岬角/海角", "n.")}.
</div>

<div class="paragraph">
  Thousands of gulls and crows covered the ground, {anno("spilling over", "蔓延/溢出")} into the {anno("ditches", "沟渠", "n.")} and among the low, rough {anno("heather", "石楠花", "n.")} and {anno("gorse", "金雀花", "n.")}. Beneath this {anno("churning", "翻腾的", "adj.")} sea of black and white the ground was a {anno("uniform", "统一的/一片的", "adj.")} red. Here and there rose the {anno("ribbed", "肋骨突出的", "adj.")} humps of horses, and from among the {anno('squalling', '尖叫的', 'adj.')} birds came the {anno('glint', '闪光', 'n.')} of iron.
</div>
"""

with open("temp_output4.html", "w", encoding="utf-8") as f:
    f.write(content)
