"""Press-to-talk microphone recording."""

import os
import tempfile
import threading
import time
from typing import Callable, Optional

import numpy as np
import sounddevice as sd
from scipy.io import wavfile


# Recording settings
SAMPLE_RATE = 16000  # 16kHz for speech
CHANNELS = 1


class PTTRecorder:
    """Press-to-talk audio recorder."""

    def __init__(self, sample_rate: int = SAMPLE_RATE, channels: int = CHANNELS):
        self.sample_rate = sample_rate
        self.channels = channels
        self.recording = False
        self.audio_data = []
        self._stream: Optional[sd.InputStream] = None

    def _audio_callback(self, indata, frames, time_info, status):
        """Called for each audio block during recording."""
        if status:
            print(f"[PTT] Status: {status}")
        if self.recording:
            self.audio_data.append(indata.copy())

    def start_recording(self):
        """Start recording audio."""
        if self.recording:
            return
        
        self.audio_data = []
        self.recording = True
        
        self._stream = sd.InputStream(
            samplerate=self.sample_rate,
            channels=self.channels,
            dtype=np.int16,
            callback=self._audio_callback
        )
        self._stream.start()
        print("[PTT] Recording started...")

    def stop_recording(self) -> Optional[str]:
        """
        Stop recording and save to temp WAV file.
        
        Returns:
            Path to WAV file, or None if no audio recorded
        """
        if not self.recording:
            return None
        
        self.recording = False
        
        if self._stream:
            self._stream.stop()
            self._stream.close()
            self._stream = None
        
        if not self.audio_data:
            print("[PTT] No audio data recorded")
            return None
        
        # Combine all audio chunks
        audio = np.concatenate(self.audio_data, axis=0)
        
        # Save to temp file
        temp_file = tempfile.NamedTemporaryFile(
            suffix=".wav",
            delete=False,
            dir=tempfile.gettempdir()
        )
        temp_path = temp_file.name
        temp_file.close()
        
        wavfile.write(temp_path, self.sample_rate, audio)
        duration = len(audio) / self.sample_rate
        print(f"[PTT] Recording saved: {temp_path} ({duration:.1f}s)")
        
        return temp_path


class PTTController:
    """
    Press-to-talk controller with hotkey support.
    
    Uses keyboard library for hotkey detection.
    """

    def __init__(
        self,
        hotkey: str = "ctrl+space",
        on_transcript: Optional[Callable[[str], None]] = None
    ):
        self.hotkey = hotkey
        self.on_transcript = on_transcript
        self.recorder = PTTRecorder()
        self._running = False

    def _on_hotkey_press(self):
        """Called when hotkey is pressed."""
        self.recorder.start_recording()

    def _on_hotkey_release(self):
        """Called when hotkey is released."""
        wav_path = self.recorder.stop_recording()
        
        if wav_path and self.on_transcript:
            # Import here to avoid circular dependency
            from stt_elevenlabs import transcribe
            from shrink import shrink
            
            text = transcribe(wav_path)
            if text:
                short_text = shrink(text)
                self.on_transcript(short_text)
            
            # Clean up temp file
            try:
                os.remove(wav_path)
            except OSError:
                pass

    def start(self):
        """Start listening for hotkey."""
        import keyboard
        
        self._running = True
        keyboard.on_press_key(
            self.hotkey.split("+")[-1],  # Get the main key
            lambda e: self._on_hotkey_press() if keyboard.is_pressed(self.hotkey.split("+")[0]) else None,
            suppress=False
        )
        keyboard.on_release_key(
            self.hotkey.split("+")[-1],
            lambda e: self._on_hotkey_release() if not keyboard.is_pressed(self.hotkey.split("+")[-1]) else None,
            suppress=False
        )
        print(f"[PTT] Listening for {self.hotkey}...")

    def stop(self):
        """Stop listening."""
        import keyboard
        self._running = False
        keyboard.unhook_all()
        print("[PTT] Stopped")


# Simple test
if __name__ == "__main__":
    print("Press-to-talk test")
    print("Hold SPACE to record, release to stop")
    print("Press ESC to exit")
    print("-" * 40)
    
    recorder = PTTRecorder()
    
    import keyboard
    
    def on_space_press(e):
        if not recorder.recording:
            recorder.start_recording()
    
    def on_space_release(e):
        if recorder.recording:
            path = recorder.stop_recording()
            if path:
                print(f"Saved to: {path}")
    
    keyboard.on_press_key("space", on_space_press)
    keyboard.on_release_key("space", on_space_release)
    
    keyboard.wait("esc")
    print("Done")
