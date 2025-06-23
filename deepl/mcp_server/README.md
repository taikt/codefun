# MCP server

A simple MCP server by taikt

## Usage

## Todo
execute server tu dong tu vscode extension
https://code.visualstudio.com/api/extension-guides/mcp

## Example

cd /home/worker/src/codefun/deepl/mcp_server
### tao moi truong ao 1 lan dau
rm -rf venv
sudo apt install python3.12-venv
python3 -m venv venv 

### kich hoat moi truong ao neu mo new terminal
source venv/bin/activate 

### luon cai lai khi thay doi server code
pip install -e .

## run server
start server trong mcp.json
mo copilot chat cua so moi (copilot tu dong ket noi server)
go: fetch_lgedv_rule

## xem log
cat /tmp/mcp_simple_prompt.log

## upload package
Truy cập https://pypi.org/account/register/
Đăng ký tài khoản và xác thực email.

(xoa existing venv)
rm -rf venv

pip install build twine
(trong thu muc chua pyproject.toml)
pip install build
python -m build

Kết quả: thư mục dist/ sẽ có file .whl và .tar.gz

upload len PyPI
twine upload dist/*

## user install
python3 -m venv venv
source venv/bin/activate
pip install codeguard-taikt

## user setup mcp.json
"servers": {
  "codeguard": {
    "command": "/duong_dan_toi_venv/bin/codeguard",
    "args": ["--transport", "stdio"]
  }
}

## When use MCP resource
dung khi co file markdown, vi cau truc don gian, tham khao cach lay lgedv rule(markdown) bang mcp resource trong code hien tai 
Neu lay resouce la cpp file, no khong co cau truc nen AI agent kho hieu, khong the parse
metadata type.

## note
Có, rất cần thiết phải có file __init__.py trong thư mục package (ví dụ: mcp_server/) nếu bạn muốn Python nhận diện đó là một package hợp lệ để import (ví dụ: import mcp_server.server). File này có thể rỗng, chỉ cần tồn tại.

File __main__.py không bắt buộc, trừ khi bạn muốn chạy package đó như một module (python -m mcp_server). Đối với trường hợp sử dụng entry point script như mcp-taikt = "mcp_server.server:main", bạn không cần __main__.py, chỉ cần __init__.py là đủ.

Tóm lại:
- __init__.py: Bắt buộc để Python nhận diện package.
- __main__.py: Chỉ cần nếu muốn chạy python -m <package>. Không bắt buộc cho entry point script.

