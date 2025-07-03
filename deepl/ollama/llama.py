import asyncio
import subprocess
from pathlib import Path
from ollama import AsyncClient

def is_cpp_file(filename):
    """Check if the file is a C++ file."""
    cpp_extensions = {'.cpp', '.h', '.hpp', '.cxx', '.cc'}
    return Path(filename).suffix.lower() in cpp_extensions


async def analyze_with_ollama(file_path, content, output_file):
    """
    Send the file content to the qwen3 model via ollama for error analysis and write the results to a file.
    """
    # Add line numbers to the content
    lines = content.splitlines()
    numbered_content = "\n".join(f"{i+1}: {line}" for i, line in enumerate(lines))
    
    message = {
    "role": "user",
    "content": (
        f"Analyze the following C++ code for potential errors. "
        f"Provide a simple report that indicates which code line has an issue, "
        f"using the line numbers provided in the format 'Line X: ...'. "
        f"Please include simple explanations, and list the issues with their corresponding line numbers.\n\n"
        f"{numbered_content}\n/no_think"
    )
}
    print(f"\nAnalyzing {file_path} with Ollama (qwen3:8b)...")
    try:
        with open(output_file, 'a', encoding='utf-8') as f:
            f.write(f"\n=== Ollama Analysis for {file_path} ===\n")
            response = await AsyncClient().chat(
                model="qwen3:8b", messages=[message], stream=False
            )
            content = response["message"]["content"]
            f.write(content + "\n")
            print(content)
    except Exception as e:
        error_msg = f"Error analyzing {file_path} with Ollama: {str(e)}\n"
        with open(output_file, 'a', encoding='utf-8') as f:
            f.write(error_msg)
        print(error_msg)

async def scan_directory(source_dir, output_file):
    """
    Scan all files in the source directory, analyze with Clang, Clang-Tidy, Cppcheck, and Ollama, and write results to a file.
    """
    source_path = Path(source_dir)
    if not source_path.exists() or not source_path.is_dir():
        print(f"Error: Directory '{source_dir}' does not exist or is not a directory.")
        return

    print(f"Scanning directory: {source_dir}")
    with open(output_file, 'w', encoding='utf-8') as f:
        f.write("C++ Code Analysis Report\n")
        f.write("=" * 30 + "\n")
    
    for file_path in source_path.rglob('*'):
        if file_path.is_file() and is_cpp_file(file_path):
            try:
                with open(file_path, 'r', encoding='utf-8') as file:
                    content = file.read()
                
                # Run Ollama analysis last
                await analyze_with_ollama(file_path, content, output_file)
                
            except Exception as e:
                error_msg = f"Error reading file {file_path}: {str(e)}\n"
                with open(output_file, 'a', encoding='utf-8') as f:
                    f.write(error_msg)
                print(error_msg)

if __name__ == "__main__":
    source_directory = "./"
    output_file = "analysis_report.txt"
    asyncio.run(scan_directory(source_directory, output_file))
