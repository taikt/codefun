# auto_start_server.ps1
# Script tự động tạo venv, kích hoạt và chạy server_aiohttp.py trên Windows

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$VenvDir = Join-Path $ScriptDir "venv"
$ServerScript = Join-Path $ScriptDir "server_aiohttp.py"

# Tạo venv nếu chưa có
if (!(Test-Path "$VenvDir\Scripts\Activate.ps1")) {
    Write-Host "[INFO] Creating virtual environment..." -ForegroundColor Cyan
    python -m venv $VenvDir
}

# Kích hoạt venv và cài package
Write-Host "[INFO] Activating virtual environment..." -ForegroundColor Cyan
$PythonExe = Join-Path $VenvDir "Scripts\python.exe"
& $PythonExe -m pip install --upgrade pip
& $PythonExe -m pip install -r "$ScriptDir\requirements.txt"

# Chạy server
Write-Host "[INFO] Starting server_aiohttp.py..." -ForegroundColor Green
& $PythonExe "$ServerScript"
