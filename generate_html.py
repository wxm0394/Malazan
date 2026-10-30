import os

def anno(en, zh):
    # Use KaTeX syntax for math formula overset
    # \scriptsize makes the Chinese text smaller
    # \textcolor sets the medieval ink red color
    return f'\\( \\overset{{\\textcolor{{#8b2b2b}}{{\\text{{{zh}}}}}}}{{\\text{{{en}}}}} \\)'

html_content = f"""
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<title>Gardens of the Moon - Chapter One</title>

<!-- KaTeX Styles and Scripts -->
<link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/katex@0.16.8/dist/katex.min.css">
<script defer src="https://cdn.jsdelivr.net/npm/katex@0.16.8/dist/katex.min.js"></script>
<script defer src="https://cdn.jsdelivr.net/npm/katex@0.16.8/dist/contrib/auto-render.min.js" onload="renderMathInElement(document.body);"></script>

<style>
  body {{
    background-color: #f4ecd8;
    background-image: url('data:image/svg+xml;utf8,<svg width="200" height="200" xmlns="http://www.w3.org/2000/svg"><filter id="noise"><feTurbulence type="fractalNoise" baseFrequency="0.8" numOctaves="4" stitchTiles="stitch"/></filter><rect width="100%" height="100%" filter="url(%23noise)" opacity="0.05"/></svg>');
    color: #2c221a;
    font-family: "Book Antiqua", "Palatino Linotype", "Georgia", serif;
    font-size: 18px;
    line-height: 2.8; /* Give enough room for formulas */
    padding: 60px 10vw;
    margin: 0;
  }}
  .paper-container {{
    max-width: 800px;
    margin: 0 auto;
    background: rgba(255, 252, 242, 0.6);
    padding: 60px 80px;
    box-shadow: 0 4px 15px rgba(0,0,0,0.1), inset 0 0 40px rgba(139, 69, 19, 0.05);
    border: 1px solid #d2b48c;
    border-radius: 4px;
  }}
  
  /* KaTeX font overrides */
  .katex .text {{
    font-family: inherit !important; /* Inherit the parchment font */
  }}
  .katex .mord.text {{
    font-family: inherit !important;
  }}
  .katex-display {{
    margin: 0 !important;
  }}
  
  u {{
    text-decoration: underline;
    text-decoration-color: #2c221a;
    text-decoration-thickness: 1px;
    text-underline-offset: 4px;
  }}
  b {{
    font-weight: bold;
    color: #111;
  }}
  .paragraph {{
    margin-bottom: 30px;
    text-indent: 2em;
    /* Force left alignment to prevent ANY justify gaps around formulas */
    text-align: left !important; 
  }}
  .header {{
    margin-bottom: 50px;
    font-style: italic;
    color: #443322;
    text-align: center;
    line-height: 1.8;
    border-bottom: 1px solid #d2b48c;
    padding-bottom: 20px;
  }}
</style>
</head>
<body>

<div class="paper-container">

<div class="header">
  1161st Year of <b>Burn</b>'s Sleep<br>
  103rd Year of the <u>Malazan Empire</u><br>
  7th Year of <b>Empress Laseen</b>'s Rule
</div>

<div class="paragraph">
  "{anno('Prod and pull', '推推搡搡')}," the old woman was saying, "{anno("'tis", 'it is(古语)')} {anno('the way of', '...的行事作风')} the <b>Empress</b>, {anno('as like', '就像是')} the gods themselves." She {anno('leaned to one side', '向一侧倾斜')} and {anno('spat', '吐口水')}, then {anno('brought a soiled cloth to', '拿脏布捂住')} her {anno('wrinkled', '布满皱纹的')} lips. "Three husbands and two sons I {anno('saw off to war', '送别上战场')}."
</div>

<div class="paragraph">
  The fishergirl's eyes {anno('shone', '闪耀')} as she watched the {anno('column', '纵队')} of {anno('mounted soldiers', '骑兵')} {anno('thunder past', '轰鸣而过')}, and she only {anno('half listened to', '心不在焉地听')} the {anno('hag', '老太婆')} standing beside her. The girl's breath {anno('had risen to', '随着...加快')} the {anno('pace', '步伐/节奏')} of the {anno('magnificent', '雄伟的')} horses. She felt her face {anno('burning', '发烫')}, a {anno('flush', '红晕')} that {anno('had nothing to do with', '与...无关')} the heat. The day {anno('was dying', '渐暗/消逝')}, the sun's red {anno('smear', '红色余晖')} over the trees on her right, and the sea's {anno('sighing', '叹息声')} against her face had {anno('grown cool', '变得凉爽')}.
</div>

</div>

</body>
</html>
"""

with open("chapter_one_annotated.html", "w", encoding="utf-8") as f:
    f.write(html_content)

print("HTML successfully regenerated: chapter_one_annotated.html")
