import asyncio
import subprocess
from pathlib import Path
from ollama import AsyncClient

def is_cpp_file(filename):
    """Kiểm tra xem file có phải là file C++ không."""
    cpp_extensions = {'.cpp', '.h', '.hpp', '.cxx', '.cc'}
    return Path(filename).suffix.lower() in cpp_extensions


async def analyze_with_ollama(file_path, content, output_file):
    """
    Gửi nội dung file tới mô hình qwen3 qua ollama để phân tích lỗi và ghi kết quả vào file.
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
    print(f"\nĐang phân tích {file_path} với Ollama (qwen3:8b)...")
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
        error_msg = f"Lỗi khi phân tích {file_path} với Ollama: {str(e)}\n"
        with open(output_file, 'a', encoding='utf-8') as f:
            f.write(error_msg)
        print(error_msg)

async def scan_directory(source_dir, output_file):
    """
    Quét tất cả file trong thư mục source, phân tích bằng Clang, Clang-Tidy, Cppcheck và Ollama, ghi kết quả vào file.
    """
    source_path = Path(source_dir)
    if not source_path.exists() or not source_path.is_dir():
        print(f"Lỗi: Thư mục '{source_dir}' không tồn tại hoặc không phải thư mục.")
        return

    print(f"Đang quét thư mục: {source_dir}")
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
                error_msg = f"Lỗi khi đọc file {file_path}: {str(e)}\n"
                with open(output_file, 'a', encoding='utf-8') as f:
                    f.write(error_msg)
                print(error_msg)

if __name__ == "__main__":
    source_directory = "cpp_test"
    output_file = "analysis_report.txt"
    asyncio.run(scan_directory(source_directory, output_file))