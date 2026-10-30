import sys
import re

new_content = sys.stdin.read()

with open("chapter_one_annotated.html", "r", encoding="utf-8") as f:
    html = f.read()

# Replace </div>\n\n</body>\n</html> with the new content + closing tags
html = html.replace('</div>\n\n</body>\n</html>', new_content + '\n</div>\n\n</body>\n</html>')

with open("chapter_one_annotated.html", "w", encoding="utf-8") as f:
    f.write(html)
