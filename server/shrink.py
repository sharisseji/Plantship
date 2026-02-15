"""Text shrinking for LCD display."""

import re
from typing import Optional

# Max characters for voice box on 3.5" LCD
MAX_CHARS = 20

# Intent mapping: common phrases -> short codes
INTENT_MAP = {
    # Lights
    r"turn on.*light": "LIGHTS ON",
    r"turn off.*light": "LIGHTS OFF",
    r"lights on": "LIGHTS ON",
    r"lights off": "LIGHTS OFF",
    r"bedroom light": "BED LIGHT",
    r"living room light": "LR LIGHT",
    r"kitchen light": "KIT LIGHT",
    
    # Temperature
    r"(what('s| is) the|check) temp": "CHECK TEMP",
    r"too (hot|warm)": "TOO HOT",
    r"too cold": "TOO COLD",
    r"set temp": "SET TEMP",
    
    # Humidity/moisture
    r"(what('s| is) the|check) humid": "CHECK HUMID",
    r"(what('s| is) the|check) moist": "CHECK MOIST",
    r"water.*plant": "WATER PLANT",
    r"plant.*dry": "PLANT DRY",
    
    # General
    r"hello|hi|hey": "HELLO",
    r"thank": "THANKS",
    r"help": "HELP",
    r"status|report": "STATUS",
    r"stop|cancel": "CANCEL",
    r"yes|confirm|ok": "OK",
    r"no|deny|nope": "NO",
}

# Filler words to remove
FILLER_WORDS = {
    "the", "a", "an", "is", "are", "was", "were", "be", "been",
    "please", "could", "would", "can", "should", "just", "like",
    "um", "uh", "ah", "oh", "well", "so", "very", "really",
    "i", "me", "my", "you", "your", "we", "our", "it", "its"
}


def match_intent(text: str) -> Optional[str]:
    """Try to match text to a known intent."""
    text_lower = text.lower().strip()
    
    for pattern, intent in INTENT_MAP.items():
        if re.search(pattern, text_lower):
            return intent
    
    return None


def extract_keywords(text: str, max_words: int = 3) -> str:
    """Extract important keywords from text."""
    words = text.lower().split()
    keywords = [w for w in words if w not in FILLER_WORDS and len(w) > 1]
    
    # Take first N keywords
    selected = keywords[:max_words]
    return " ".join(selected).upper()


def shrink(text: str) -> str:
    """
    Shrink text to fit LCD display.
    
    Strategy:
    1. Try intent mapping first
    2. Fall back to keyword extraction
    3. Hard truncate if still too long
    """
    if not text:
        return ""
    
    # Clean input
    text = text.strip()
    
    # Try intent mapping
    intent = match_intent(text)
    if intent:
        return intent[:MAX_CHARS]
    
    # Extract keywords
    short = extract_keywords(text)
    if short:
        return short[:MAX_CHARS]
    
    # Last resort: just uppercase and truncate
    return text.upper()[:MAX_CHARS]


# Test
if __name__ == "__main__":
    test_phrases = [
        "Turn on the bedroom lights please",
        "What's the temperature right now?",
        "The plant looks dry, maybe water it",
        "Hello there!",
        "Can you check the humidity level?",
        "This is a very long sentence that should be truncated",
        "Set temperature to 72 degrees",
    ]
    
    print("Text shrinking tests:")
    print("-" * 50)
    for phrase in test_phrases:
        result = shrink(phrase)
        print(f"{phrase[:30]:30} -> {result}")
