#!/usr/bin/env python3
"""
Demo tokenization để hiểu cách AI xử lý token
"""

# Simulate tokenization (không cần OpenAI API)
def simulate_tokenization_vietnamese():
    """Mô phỏng cách tokenize tiếng Việt"""
    
    text = "AI model dung thuat toan BPE de xu ly ngon ngu"
    print(f"Original text: {text}")
    print()
    
    # Giả lập BPE tokenization process
    print("=== BPE TOKENIZATION PROCESS ===")
    
    # Step 1: Character level
    chars = list(text)
    print(f"Step 1 - Characters: {chars}")
    
    # Step 2: Merge common patterns (mô phỏng)
    # Trong thực tế, AI đã học được patterns này từ training
    common_patterns = {
        "AI": "AI",
        " model": " model", 
        " dung": " dung",
        " thuat": " thuat",
        " toan": " toan",
        " BPE": " BPE",
        " de": " de",
        " xu": " xu",
        " ly": " ly", 
        " ngon": " ngon",
        " ngu": " ngu"
    }
    
    # Simulate merged tokens
    tokens = []
    i = 0
    while i < len(text):
        # Tìm longest match
        found = False
        for length in range(6, 0, -1):  # Từ dài đến ngắn
            if i + length <= len(text):
                substring = text[i:i+length]
                if substring in common_patterns:
                    tokens.append(f"'{substring}'")
                    i += length
                    found = True
                    break
        if not found:
            tokens.append(f"'{text[i]}'")
            i += 1
    
    print(f"Step 2 - Tokens: {tokens}")
    print(f"Total tokens: {len(tokens)}")
    print()
    
    # Step 3: How AI understands context
    print("=== CONTEXT UNDERSTANDING ===")
    
    context_analysis = {
        "'AI'": "Technical term - Machine Learning context",
        "' model'": "ML concept - follows AI", 
        "' dung'": "Vietnamese verb - 'use/utilize'",
        "' thuat'": "Part of compound 'thuật toán' (algorithm)",
        "' toan'": "Completes 'thuật toán' concept",
        "' BPE'": "Technical acronym - Byte Pair Encoding",
        "' de'": "Vietnamese preposition - 'to/for'",
        "' xu'": "Start of 'xử lý' (process)", 
        "' ly'": "Completes 'xử lý' concept",
        "' ngon'": "Start of 'ngôn ngữ' (language)",
        "' ngu'": "Completes 'ngôn ngữ' concept"
    }
    
    for token, meaning in context_analysis.items():
        print(f"{token:12} → {meaning}")
    
    print()
    print("=== AI SEMANTIC UNDERSTANDING ===")
    print("Full context interpretation:")
    print("- 'AI model' → Machine Learning model")
    print("- 'dung thuat toan' → uses algorithm") 
    print("- 'BPE' → Byte Pair Encoding technique")
    print("- 'de xu ly ngon ngu' → to process language")
    print()
    print("🎯 Final understanding: AI model uses BPE algorithm to process language")

def demonstrate_subword_patterns():
    """Minh họa cách AI nhận biết patterns trong subwords"""
    
    print("\n=== SUBWORD PATTERN RECOGNITION ===")
    
    examples = {
        "running": ["run", "ning"],
        "unhappy": ["un", "happy"], 
        "tokenization": ["token", "ization"],
        "preprocessing": ["pre", "process", "ing"],
        "thuật toán": ["thuật", " toán"],
        "xử lý": ["xử", " lý"],
        "ngôn ngữ": ["ngôn", " ngữ"]
    }
    
    for word, tokens in examples.items():
        print(f"'{word}' → {tokens}")
        
        # Analyze semantic components
        if word == "running":
            print("  Semantic: ROOT(run) + SUFFIX(ning) = continuous action")
        elif word == "unhappy": 
            print("  Semantic: PREFIX(un) + ROOT(happy) = negation")
        elif word == "tokenization":
            print("  Semantic: ROOT(token) + SUFFIX(ization) = process of making tokens")
        elif word == "thuật toán":
            print("  Semantic: thuật(technique) + toán(math) = algorithm")
        elif word == "xử lý":
            print("  Semantic: xử(handle) + lý(logic) = process")
        elif word == "ngôn ngữ":
            print("  Semantic: ngôn(speech) + ngữ(language) = language")
        print()

def show_context_vectors():
    """Mô phỏng cách AI dùng vector để hiểu context"""
    
    print("=== CONTEXT VECTOR SIMULATION ===")
    print("(Simplified representation of how AI encodes meaning)")
    print()
    
    # Giả lập vector representations (thực tế có 768-4096 dimensions)
    token_vectors = {
        "AI": [0.8, 0.2, -0.1, 0.9, 0.3],      # High tech, positive context
        "model": [0.7, 0.5, 0.2, 0.8, 0.1],   # Mathematical, structured
        "dung": [0.1, -0.2, 0.6, 0.3, 0.4],   # Action verb, Vietnamese
        "thuat": [0.3, 0.4, 0.5, 0.7, 0.6],   # Technical term prefix
        "toan": [0.5, 0.7, 0.4, 0.9, 0.8],    # Mathematics, completes "thuat"
        "BPE": [0.9, 0.1, -0.3, 0.8, 0.7]     # Technical acronym, NLP
    }
    
    print("Token vectors (5-dim simplified):")
    for token, vector in token_vectors.items():
        print(f"{token:8} → {vector}")
    
    print()
    print("Context relationships (cosine similarity simulation):")
    print("- 'AI' ↔ 'model': 0.92 (high similarity - related concepts)")
    print("- 'thuat' ↔ 'toan': 0.87 (high - forms compound word)")  
    print("- 'AI' ↔ 'BPE': 0.78 (medium-high - both tech terms)")
    print("- 'dung' ↔ 'model': 0.45 (medium - verb-object relationship)")
    print()
    print("🧠 AI combines these relationships to understand full meaning!")

if __name__ == "__main__":
    simulate_tokenization_vietnamese()
    demonstrate_subword_patterns() 
    show_context_vectors()
