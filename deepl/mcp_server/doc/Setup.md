# TODO
- moi lan check_leaks (3 files) => luu vao /tmp/lgedv/memory_analysis_889ee30a_checked.json
{
  "timestamp": "2025-08-06T00:12:39.053479",
  "analysis_type": "memory_analysis",
  "directory": "/home/worker/src/code_test/memory_leak/test_1",
  "checked_files": [
    "t2.cpp",
    "t3.cpp",
    "t4.cpp"
  ]
}
- khi goi check_leaks lan tiep, can kiem tra tren cac file remaining (bo di cac file trong checked_file) => chon tiep 3 files
- gia tri 889ee30a: unique cho moi lan mo chat session

# Build and upload package
## pypi
# https://pypi.org/manage/account/token/
# taikt_token
pypi-AgEIcHlwaS5vcmcCJGFmMjc0ZjU2LTgyNzgtNGUzMC1hZWQ5LTU5OGExYzVjNGM3OAACKlszLCJhNDJkOGMyOC0zZTY3LTRhODMtOGZkMi01MmRkMGVjYjdhNTIiXQAABiAahj-MaomotqQ63iqH-U3BQWAmqi4ZQJBuePnDU50QYg
(user: taikt, Yeuem2025@, google authenticator)

## build server (linux)
cd /home/worker/src/codefun/deepl/mcp_server
(trong thu muc chua pyproject.toml)
python3 -m venv venv
source venv/bin/activate
pip install -e .

## package (linux)
rm -rf dist
pip install build twine
python -m build
Kết quả: thư mục dist/ sẽ có file .whl và .tar.gz

## upload len PyPI (linux)
twine upload dist/*

# Install server for user (with pip)
## on Linux
sudo apt install python3 python3-venv python3-pip
python3 -m venv venv
source venv/bin/activate
pip install codeguard-lgedv

## on window
### 1. Cài đặt Python và pip
- Truy cập trang chủ Python: https://www.python.org/downloads/windows/
- Tải bản Python 3.9+ 
### 2. Tạo môi trường ảo (venv)
- Tạo thư mục dự án (ví dụ: `C:\mcp_test`)
  python -m venv venv
  venv\Scripts\activate

### 3. Cài đặt MCP server từ PyPI
  pip install codeguard-lgedv
  dir venv\Scripts\codeguard.exe

### 4. Cấu hình file mcp.json 
- create folder .vscode on current codebase
- create file mcp.json in .vscode folder
- Ví dụ nội dung:
  ```json
  {
    "servers": {
        "lgedv": {
            "command": "C:\\Users\\tai2.tran\\mcp_server\\venv\\Scripts\\codeguard.exe",
            "args": ["--transport", "stdio"],
            "env": {
            "src_dir": "E:\\src\\code_test\\mem_leak",
            "custom_path": "E:\\src\\codefun\\deepl\\mcp_server\\lgedv\\resources\\CustomRule.md" 
            }
        }
    }
  }
  ```




