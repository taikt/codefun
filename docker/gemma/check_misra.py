import json
import torch
from transformers import AutoModelForCausalLM, AutoTokenizer
import clang.cindex
from pathlib import Path

# Danh sách quy tắc MISRA
MISRA_RULES = [
    {
        "rule_id": "MISRA C++ 3-1-1",
        "description": "Không sử dụng các đặc tả hàm inline trong định nghĩa hàm."
    },
    {
        "rule_id": "MISRA C++ 5-0-1",
        "description": "Biểu thức logic không được chứa toán tử && hoặc || ở cấp độ cao nhất."
    }
]

# Hàm trích xuất hàm và câu lệnh if từ mã C++ bằng libclang
def extract_code_snippets(file_path):
    try:
        index = clang.cindex.Index.create()
        tu = index.parse(file_path, args=['-std=c++17'])
        snippets = []
        
        def traverse(node):
            # Trích xuất hàm
            if node.kind == clang.cindex.CursorKind.FUNCTION_DECL:
                tokens = [t.spelling for t in node.get_tokens()]
                code = "".join(tokens)
                snippets.append({"code": code, "line": node.location.line, "type": "function"})
            # Trích xuất câu lệnh if
            elif node.kind == clang.cindex.CursorKind.IF_STMT:
                tokens = [t.spelling for t in node.get_tokens()]
                code = "".join(tokens)
                snippets.append({"code": code, "line": node.location.line, "type": "if_stmt"})
            for child in node.get_children():
                traverse(child)
        
        traverse(tu.cursor)
        return snippets
    except Exception as e:
        print(f"Error parsing {file_path}: {e}")
        return []

# Hàm kiểm tra mã bằng mô hình fine-tuned
def check_code_snippet(model, tokenizer, code, rule_id, description):
    prompt = f"""
    Dưới đây là đoạn mã C++:
    ```cpp
    {code}
    ```
    Kiểm tra đoạn mã này dựa trên quy tắc {rule_id}: "{description}"
    Trả về JSON với các trường: violation (true/false), description, suggestion.
    """
    inputs = tokenizer(prompt, return_tensors="pt").to("mps" if torch.backends.mps.is_available() else "cpu")
    outputs = model.generate(**inputs, max_length=512)
    result = tokenizer.decode(outputs[0], skip_special_tokens=True)
    try:
        return json.loads(result)
    except:
        return {}

# Hàm chính để kiểm tra nhiều tệp C++
def check_misra_files(cpp_dir, model_path, output_file="/app/output/misra_report.json"):
    # Tải mô hình và tokenizer
    tokenizer = AutoTokenizer.from_pretrained(model_path)
    model = AutoModelForCausalLM.from_pretrained(
        model_path,
        device_map="mps" if torch.backends.mps.is_available() else "cpu",
        torch_dtype=torch.float16
    )
    
    # Tìm tất cả tệp C++ trong thư mục
    cpp_files = list(Path(cpp_dir).glob("*.cpp"))
    if not cpp_files:
        print(f"No C++ files found in {cpp_dir}")
        return
    
    # Lưu kết quả vi phạm
    report = []
    
    # Duyệt qua từng tệp
    for cpp_file in cpp_files:
        print(f"Processing {cpp_file}")
        snippets = extract_code_snippets(str(cpp_file))
        
        # Kiểm tra từng đoạn mã với từng quy tắc
        for snippet in snippets:
            for rule in MISRA_RULES:
                result = check_code_snippet(model, tokenizer, snippet["code"], rule["rule_id"], rule["description"])
                if result and result.get("violation", False):
                    result.update({
                        "file": str(cpp_file),
                        "line": snippet["line"],
                        "type": snippet["type"],
                        "code": snippet["code"]
                    })
                    report.append(result)
    
    # Lưu báo cáo
    with Path(output_file).open('w', encoding='utf-8') as f:
        json.dump(report, f, indent=2, ensure_ascii=False)
    
    print(f"Report saved to {output_file}. Found {len(report)} violations.")

# Chạy chương trình
if __name__ == "__main__":
    cpp_dir = "/app/cpp_files"
    model_path = "/app/output/gemma2b-misra-finetuned"
    if not Path(model_path).exists():
        raise FileNotFoundError(f"Model not found at {model_path}. Run fine_tune_gemma.py first.")
    check_misra_files(cpp_dir, model_path)