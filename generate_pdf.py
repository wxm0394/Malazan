import os
from weasyprint import HTML

html_content = """
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<style>
  @page {
    margin: 50px 60px;
  }
  body {
    font-family: "Times New Roman", "PingFang SC", "Microsoft YaHei", serif;
    font-size: 16px;
    line-height: 2.8; /* Increased line-height for ruby text */
    color: #222;
    text-align: justify;
  }
  ruby {
    ruby-align: center;
    ruby-position: over;
  }
  rt {
    font-family: "PingFang SC", "Microsoft YaHei", sans-serif;
    font-size: 11px;
    color: #555;
    font-weight: normal;
  }
  u {
    text-decoration: underline;
    text-decoration-color: #333;
    text-decoration-thickness: 1px;
    text-underline-offset: 3px;
  }
  b {
    font-weight: bold;
    color: #000;
  }
  .paragraph {
    margin-bottom: 24px;
    text-indent: 2em;
  }
  .header {
    margin-bottom: 40px;
    font-style: italic;
    color: #444;
    text-align: left;
    line-height: 1.8;
  }
</style>
</head>
<body>

<div class="header">
  1161st Year of <b>Burn</b>'s Sleep<br>
  103rd Year of the <u>Malazan Empire</u><br>
  7th Year of <b>Empress Laseen</b>'s Rule
</div>

<div class="paragraph">
  "<ruby>Prod and pull<rt>推推搡搡</rt></ruby>," the old woman was saying, "<ruby>'tis<rt>it is(古语)</rt></ruby> <ruby>the way of<rt>...的行事作风</rt></ruby> the <b>Empress</b>, <ruby>as like<rt>就像是</rt></ruby> the gods themselves." She <ruby>leaned to one side<rt>往一侧倾斜</rt></ruby> and <ruby>spat<rt>吐口水</rt></ruby>, then <ruby>brought a soiled cloth to<rt>拿脏布捂住</rt></ruby> her <ruby>wrinkled<rt>起皱的</rt></ruby> lips. "Three husbands and two sons I <ruby>saw off to war<rt>送别上战场</rt></ruby>."
</div>

<div class="paragraph">
  The fishergirl's eyes <ruby>shone<rt>闪耀</rt></ruby> as she watched the <ruby>column<rt>纵队</rt></ruby> of <ruby>mounted soldiers<rt>骑兵</rt></ruby> <ruby>thunder past<rt>轰鸣而过</rt></ruby>, and she only <ruby>half listened to<rt>心不在焉地听</rt></ruby> the <ruby>hag<rt>老太婆</rt></ruby> standing beside her. The girl's breath <ruby>had risen to<rt>随着...加快</rt></ruby> the <ruby>pace<rt>步伐/节奏</rt></ruby> of the <ruby>magnificent<rt>雄伟的</rt></ruby> horses. She felt her face <ruby>burning<rt>发烫</rt></ruby>, a <ruby>flush<rt>红晕</rt></ruby> that <ruby>had nothing to do with<rt>与...无关</rt></ruby> the heat. The day <ruby>was dying<rt>渐暗/消逝</rt></ruby>, the sun's red <ruby>smear<rt>涂抹/余晖</rt></ruby> over the trees on her right, and the sea's <ruby>sighing<rt>叹息声</rt></ruby> against her face had <ruby>grown cool<rt>变得凉爽</rt></ruby>.
</div>

</body>
</html>
"""

with open("annotated_chapter.html", "w", encoding="utf-8") as f:
    f.write(html_content)

HTML("annotated_chapter.html").write_pdf("annotated_chapter.pdf")
print("PDF successfully generated: annotated_chapter.pdf")
