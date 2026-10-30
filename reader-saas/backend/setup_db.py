import urllib.request
import zipfile
import os

def download_file(url, filename):
    print(f"Downloading {url} to {filename}...")
    urllib.request.urlretrieve(url, filename)
    print("Download complete.")

def setup_vocab():
    print("Setting up Top 2000 Vocabulary...")
    vocab_url = "https://raw.githubusercontent.com/first20hours/google-10000-english/master/google-10000-english-no-swears.txt"
    vocab_file = "app/top2000.txt"
    
    urllib.request.urlretrieve(vocab_url, "temp_vocab.txt")
    
    with open("temp_vocab.txt", "r") as f:
        words = f.read().splitlines()
    
    # Take top 2000 words
    top_2000 = set(words[:2000])
    
    # Add some extremely basic words that might be missing or single letters
    top_2000.update(["a", "i", "the", "be", "to", "of", "and", "in", "that", "have"])
    
    with open(vocab_file, "w") as f:
        for w in top_2000:
            f.write(w + "\n")
            
    os.remove("temp_vocab.txt")
    print(f"Saved {len(top_2000)} words to {vocab_file}")

def setup_dictionary():
    print("Setting up ECDICT SQLite database...")
    db_zip_url = "https://github.com/skywind3000/ECDICT/releases/download/1.0.28/ecdict-sqlite-28.zip"
    db_zip_file = "ecdict.zip"
    db_target = "app/stardict.db"
    
    if os.path.exists(db_target):
        print("Database already exists. Skipping.")
        return
        
    download_file(db_zip_url, db_zip_file)
    
    print("Extracting database...")
    with zipfile.ZipFile(db_zip_file, 'r') as zip_ref:
        zip_ref.extract("stardict.db", "app/")
        
    os.remove(db_zip_file)
    print("Dictionary setup complete.")

if __name__ == "__main__":
    # Ensure app directory exists
    os.makedirs("app", exist_ok=True)
    setup_vocab()
    setup_dictionary()
    print("All setups finished successfully!")
