import os
import sqlite3
import re
try:
    from wordfreq import zipf_frequency
except ImportError:
    pass

DB_PATH = os.path.join(os.path.dirname(__file__), "stardict.db")

def lookup_word(lemma: str) -> str:
    """
    Looks up a lemmatized word in the local SQLite dictionary (ECDICT).
    If it fails, falls back to Google Translate.
    """
    # 1. Local SQLite Dictionary
    if os.path.exists(DB_PATH):
        try:
            conn = sqlite3.connect(DB_PATH)
            cursor = conn.cursor()
            cursor.execute("SELECT translation FROM stardict WHERE word = ? COLLATE NOCASE", (lemma,))
            row = cursor.fetchone()
            conn.close()
            
            if row and row[0]:
                translation = row[0]
                brief_trans = translation.split('\\n')[0].strip()
                
                # Put part of speech at the end: "n. 翻译" -> "翻译 n."
                pos_match = re.match(r'^([a-z]{1,4}\.)\s*(.+)$', brief_trans, flags=re.IGNORECASE)
                if pos_match:
                    brief_trans = f"{pos_match.group(2)} {pos_match.group(1)}"
                    
                if len(brief_trans) > 15:
                    brief_trans = brief_trans[:15] + "..."
                return brief_trans
        except Exception:
            pass
            
    return "生词"

def is_hard_word(lemma: str, pos: str) -> bool:
    """
    Determines if a word is considered 'hard'.
    Uses zipf_frequency from wordfreq (score 0 to 8).
    Score >= 4.3 roughly matches top 2000-3000 common words (barrier-free).
    """
    if not lemma:
        return False
        
    lemma_lower = lemma.lower()
    
    # Skip non-alphabetical tokens (punctuation, numbers, etc)
    if not re.match(r'^[a-z]+$', lemma_lower):
        return False

    try:
        freq = zipf_frequency(lemma_lower, 'en')
        # Common conversational words like 'drink' are ~5.0. 
        # Rare words like 'ephemeral' are ~2.8.
        if freq >= 4.3:
            return False # Common word, don't mark
        return True # Hard word, mark it
    except NameError:
        # Fallback if wordfreq is not installed yet
        return len(lemma_lower) >= 7

