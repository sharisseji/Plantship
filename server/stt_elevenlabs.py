"""ElevenLabs Speech-to-Text integration."""

import os
import requests
from typing import Optional


ELEVENLABS_STT_URL = "https://api.elevenlabs.io/v1/speech-to-text"


def transcribe(wav_path: str, api_key: Optional[str] = None) -> Optional[str]:
    """
    Transcribe audio file using ElevenLabs STT.
    
    Args:
        wav_path: Path to WAV audio file
        api_key: ElevenLabs API key (or uses ELEVENLABS_API_KEY env var)
    
    Returns:
        Transcribed text, or None on error
    """
    api_key = api_key or os.getenv("ELEVENLABS_API_KEY")
    
    if not api_key:
        print("[STT] No API key provided")
        return None
    
    if not os.path.exists(wav_path):
        print(f"[STT] File not found: {wav_path}")
        return None
    
    headers = {
        "xi-api-key": api_key,
    }
    
    try:
        with open(wav_path, "rb") as f:
            files = {
                "file": (os.path.basename(wav_path), f, "audio/wav"),
            }
            data = {
                "model_id": "scribe_v1",  # ElevenLabs Scribe model
            }
            
            print(f"[STT] Transcribing {wav_path}...")
            response = requests.post(
                ELEVENLABS_STT_URL,
                headers=headers,
                files=files,
                data=data,
                timeout=30
            )
        
        if response.status_code == 200:
            result = response.json()
            text = result.get("text", "")
            print(f"[STT] Transcript: {text}")
            return text
        else:
            print(f"[STT] Error {response.status_code}: {response.text}")
            return None
            
    except requests.RequestException as e:
        print(f"[STT] Request failed: {e}")
        return None


# Test (requires API key)
if __name__ == "__main__":
    from dotenv import load_dotenv
    load_dotenv()
    
    # Create a test audio file path
    test_file = "test_audio.wav"
    
    if os.path.exists(test_file):
        result = transcribe(test_file)
        if result:
            print(f"Transcription: {result}")
    else:
        print(f"No test file found at {test_file}")
        print("Record an audio file and save it as test_audio.wav to test.")
