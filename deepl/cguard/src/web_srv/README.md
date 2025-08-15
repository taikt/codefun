# 🎯 Hệ Thống Server HTML Report

# Setup
(neu khong the start webserver, check setup duoi)
rm -rf venv
sudo apt install python3.12-venv
python3 -m venv venv
source venv/bin/activate
pip install aiohttp


## 🚀 Cách sử dụng nhanh

```bash
cd /home/worker/src/code_test/web_srv

# Khởi động server
./start_server.sh start

# Truy cập dashboard
# Browser: http://localhost:8888

# Kiểm tra API
curl http://localhost:8888/api/health
curl http://localhost:8888/api/files

# Dừng server
./start_server.sh stop
```

## 🔧 Lệnh quản lý server

```bash
# Khởi động server trong background
./start_server.sh start

# Kiểm tra trạng thái server
./start_server.sh status

# Dừng server và giải phóng tài nguyên
./start_server.sh stop

# Khởi động lại server
./start_server.sh restart
```


## 🌐 API Endpoints

### Health Check
- **URL**: `http://localhost:8888/api/health`
- **Response**: `{"status": "ok"}`

### File Listing (Auto-discovery)
- **URL**: `http://localhost:8888/api/files`
- **Response**: Danh sách tất cả file HTML report
- **Source**: Tự động tìm từ thư mục `../defect_report/`

### Dashboard
- **URL**: `http://localhost:8888`
- **Tính năng**: Tự động hiển thị tất cả report HTML


# kiem tra process nao dang chiem port 8888
sudo apt-get install lsof
lsof -i :8888
kill -9 557752

# kiem tra va stop tren window
netstat -ano | findstr :8888
Stop-Process -Id <PID> -Force

# set directory report
worker@tai2-tran-tsu-my26-taikt:~/.vscode$ cat settings.json 
{
  "C_Cpp.codeAnalysis.clangTidy.enabled": true,
  "C_Cpp.codeAnalysis.clangTidy.path": "/home/worker/src/llvm-project/build/bin/clang-tidy",
  "C_Cpp.codeAnalysis.clangTidy.checks.enabled": [
    "custom-SingleReturnCheck"
  ],
  "cmake.sourceDirectory": "/home/worker/src/llvm-project",
  "lgedvCodeGuard.reportDirectory": "/home/worker/src/defect_report"
}