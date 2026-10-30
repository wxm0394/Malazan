import ebooklib
from ebooklib import epub
from bs4 import BeautifulSoup
import uuid
import os

from app.nlp import process_text
from app.db import is_hard_word, lookup_word

def add_ruby_to_text(text: str) -> str:
    """
    Processes plain text, replaces hard words and phrases with HTML <ruby> tags.
    """
    tokens = process_text(text)
    html_fragments = []
    
    i = 0
    n = len(tokens)
    
    while i < n:
        matched = False
        
        # Try matching phrases of length 4, 3, 2
        for length in [4, 3, 2]:
            if i + length <= n:
                phrase_tokens = tokens[i:i+length]
                # If any token is not alphabetical, skip phrase building
                if not all(t["is_alpha"] for t in phrase_tokens):
                    continue
                
                # Space-separated lemma phrase: "look forward to"
                phrase_lemma = " ".join(t["lemma"] for t in phrase_tokens)
                
                # Directly query DB for phrase
                translation = lookup_word(phrase_lemma)
                if translation and translation != "生词":
                    # Valid phrase! Reconstruct the text with its original whitespace
                    clean_text = "".join(t["text"] + (t["whitespace"] if idx < length - 1 else "") for idx, t in enumerate(phrase_tokens))
                    trailing_ws = phrase_tokens[-1]["whitespace"]
                    
                    html_fragments.append(f'<ruby>{clean_text}<rt>{translation}</rt></ruby>{trailing_ws}')
                    i += length
                    matched = True
                    break
                    
        if matched:
            continue
            
        # Single token processing
        t = tokens[i]
        if t["is_alpha"] and not t["is_stop"] and is_hard_word(t["lemma"], t["pos"]):
            translation = lookup_word(t["lemma"])
            if translation and translation != "生词":
                html_fragments.append(f'<ruby>{t["text"]}<rt>{translation}</rt></ruby>{t["whitespace"]}')
            else:
                html_fragments.append(f'{t["text"]}{t["whitespace"]}')
        else:
            html_fragments.append(f'{t["text"]}{t["whitespace"]}')
            
        i += 1
        
    return "".join(html_fragments)

def process_epub(input_path: str, output_path: str):
    """
    Reads an EPUB, injects ruby annotations, and saves to output_path.
    """
    try:
        book = epub.read_epub(input_path)
    except Exception as e:
        print(f"Error reading epub: {e}")
        raise e

    # Add global CSS for ruby and enforce even line heights across the document
    injected_style = """<style>
        body, p, div, span, li, a { line-height: 2.2 !important; }
        ruby { ruby-align: center; ruby-position: over; }
        rt { color: #8b2b2b; font-size: 0.6em; font-weight: bold; line-height: 1; transform: translateY(15%); }
    </style>"""

    for item in book.get_items_of_type(ebooklib.ITEM_DOCUMENT):
        soup = BeautifulSoup(item.get_content(), 'html.parser')
        
        # Inject style into head
        if soup.head:
            soup.head.append(BeautifulSoup(injected_style, 'html.parser'))
        
        # We only want to process text within actual text nodes (paragraphs, spans, etc.)
        # Avoid scripts, styles, etc.
        for element in soup.find_all(['p', 'span', 'div', 'h1', 'h2', 'h3', 'h4', 'h5', 'h6', 'li']):
            # We must be careful not to overwrite child tags like <i>, <b>.
            # A simple approach for MVP: only process elements that have ONLY text and no child elements,
            # or process text nodes recursively.
            
            # Recursive approach: iterate over all NavigableString children
            # Modifying the DOM while iterating can be tricky in BeautifulSoup,
            # so we collect nodes to replace first.
            from bs4 import NavigableString
            
            strings_to_replace = []
            for child in element.children:
                if isinstance(child, NavigableString):
                    if child.strip():
                        strings_to_replace.append(child)
            
            for s in strings_to_replace:
                annotated_html = add_ruby_to_text(str(s))
                # BeautifulSoup cannot directly replace NavigableString with raw HTML string easily,
                # we must parse the annotated html back into soup nodes.
                new_nodes = BeautifulSoup(annotated_html, 'html.parser')
                s.replace_with(new_nodes)
                
        # Update the item content
        item.set_content(str(soup).encode('utf-8'))
        
    ebooklib.epub.write_epub(output_path, book)
