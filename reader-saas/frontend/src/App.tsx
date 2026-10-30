import { useState } from 'react'
import './App.css'

function App() {
  const [isProcessing, setIsProcessing] = useState(false);
  const [downloadUrl, setDownloadUrl] = useState<string | null>(null);
  const [fileName, setFileName] = useState<string>('');
  const [errorMsg, setErrorMsg] = useState<string>('');

  const handleFileChange = async (e: React.ChangeEvent<HTMLInputElement>) => {
    const file = e.target.files?.[0];
    if (!file) return;
    if (!file.name.endsWith('.epub')) {
      setErrorMsg("Please upload an EPUB file.");
      return;
    }

    setFileName(file.name);
    setErrorMsg('');
    setIsProcessing(true);
    setDownloadUrl(null);

    const formData = new FormData();
    formData.append("file", file);

    try {
      const response = await fetch("http://localhost:8000/api/upload", {
        method: "POST",
        body: formData,
      });

      if (!response.ok) {
        throw new Error("Failed to process document");
      }

      // Convert response to blob for downloading
      const blob = await response.blob();
      const url = window.URL.createObjectURL(blob);
      setDownloadUrl(url);

    } catch (err: any) {
      setErrorMsg(err.message || "An error occurred");
    } finally {
      setIsProcessing(false);
    }
  };

  const reset = () => {
    setDownloadUrl(null);
    setFileName('');
    setErrorMsg('');
    setIsProcessing(false);
  }

  return (
    <div className="app-container">
      <h1>Annotated EPUB Generator</h1>
      <p className="subtitle">Upload your EPUB. We scan the book, translate hard words, and return an annotated copy.</p>
      
      {!isProcessing && !downloadUrl && (
        <div className="upload-zone">
          <div className="upload-icon">📚</div>
          <div className="upload-text">Drag & drop an EPUB file here</div>
          <div className="upload-hint">or click to browse</div>
          <input type="file" accept=".epub" onChange={handleFileChange} />
          {errorMsg && <div style={{ color: 'red', marginTop: '15px' }}>{errorMsg}</div>}
        </div>
      )}

      {isProcessing && (
        <div className="processing-state">
          <div className="loader"></div>
          <h3>Processing {fileName}...</h3>
          <p className="upload-hint">Extracting layout, analyzing vocabulary, and injecting ruby tags. This might take a moment.</p>
        </div>
      )}

      {downloadUrl && (
        <div className="success-state">
          <div className="upload-icon">✅</div>
          <h3>Processing Complete!</h3>
          <p>Your annotated book is ready.</p>
          <a 
            href={downloadUrl} 
            download={fileName.replace('.epub', '_annotated.epub')}
            className="download-btn"
          >
            Download Annotated EPUB
          </a>
          <button onClick={reset} className="reset-btn">Process Another</button>
        </div>
      )}
    </div>
  )
}

export default App
