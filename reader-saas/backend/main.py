from fastapi import FastAPI, File, UploadFile, BackgroundTasks
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import FileResponse
from pydantic import BaseModel
import os
import shutil
import uuid

from app.epub_processor import process_epub

app = FastAPI(title="Smart English Reader SaaS API (Export Mode)")

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"], 
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

UPLOAD_DIR = "uploads"
OUTPUT_DIR = "outputs"
os.makedirs(UPLOAD_DIR, exist_ok=True)
os.makedirs(OUTPUT_DIR, exist_ok=True)

@app.get("/")
def read_root():
    return {"status": "ok", "message": "Smart Reader Export API is running"}

@app.post("/api/upload")
async def upload_file(file: UploadFile = File(...)):
    """
    Receives an EPUB file, processes it, and returns the modified file.
    """
    if not file.filename.endswith('.epub'):
        return {"error": "Only EPUB files are supported currently"}
        
    job_id = str(uuid.uuid4())
    input_path = os.path.join(UPLOAD_DIR, f"{job_id}_{file.filename}")
    output_filename = file.filename.replace('.epub', '_annotated.epub')
    output_path = os.path.join(OUTPUT_DIR, f"{job_id}_{output_filename}")
    
    with open(input_path, "wb") as buffer:
        shutil.copyfileobj(file.file, buffer)
        
    try:
        # Process the epub
        process_epub(input_path, output_path)
    except Exception as e:
        return {"error": f"Failed to process EPUB: {str(e)}"}
        
    # Return the processed file directly as an attachment for the browser to download
    return FileResponse(
        path=output_path, 
        filename=output_filename, 
        media_type='application/epub+zip'
    )

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)
