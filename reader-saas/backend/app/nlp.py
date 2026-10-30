import spacy

# Load spaCy model (ensure it's downloaded via `python -m spacy download en_core_web_sm`)
try:
    nlp = spacy.load("en_core_web_sm")
except OSError:
    print("Warning: en_core_web_sm model not found. Run `python -m spacy download en_core_web_sm`")
    nlp = None

def process_text(text: str):
    """
    Parses a paragraph, tokenizes it, and returns a list of words with their base forms (lemmas).
    """
    if not nlp:
        return [{"text": text, "lemma": text, "is_alpha": False}]
        
    doc = nlp(text)
    result = []
    
    for token in doc:
        result.append({
            "text": token.text,
            "lemma": token.lemma_.lower() if token.lemma_ else token.text.lower(),
            "pos": token.pos_,
            "is_alpha": token.is_alpha,
            "is_stop": token.is_stop,
            "whitespace": token.whitespace_
        })
        
    return result
