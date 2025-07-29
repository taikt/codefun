# start_server.ps1
param(
    [string]$Action = "start",
    [string]$Mode = "venv",  # venv (default) or system
    [string]$RunMode = "background" # background (default) or foreground
)

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$ServerScript = Join-Path $ScriptDir "server_aiohttp.py"
$VenvDir = Join-Path $ScriptDir "venv"
$PidFile = Join-Path $ScriptDir "server_aiohttp.pid"
$LogFile = Join-Path $ScriptDir "server_aiohttp.log"
$Port = 8888

function Write-Status($msg) { Write-Host "[INFO] $msg" -ForegroundColor Green }
function Write-WarningMsg($msg) { Write-Host "[WARN] $msg" -ForegroundColor Yellow }
function Write-ErrorMsg($msg) { Write-Host "[ERROR] $msg" -ForegroundColor Red }
function Write-Info($msg) { Write-Host "[DEBUG] $msg" -ForegroundColor Cyan }

function Ensure-Venv {
    if (!(Test-Path "$VenvDir\Scripts\Activate.ps1")) {
        Write-Info "Creating virtual environment..."
        python -m venv $VenvDir
        # Chờ cho đến khi file Activate.ps1 xuất hiện (timeout 10s)
        # $count = 0
        # while (!(Test-Path "$VenvDir\Scripts\Activate.ps1") -and $count -lt 5) {
        #     Start-Sleep -Seconds 1
        #     $count++
        # }
        if (!(Test-Path "$VenvDir\Scripts\Activate.ps1")) {
            Write-ErrorMsg "Failed to create virtual environment! Please check Python installation."
            exit 1
        }
    }
    Write-Info "Activating virtual environment..."
    $PythonExe = Join-Path $VenvDir "Scripts\python.exe"
    & $PythonExe -m pip install -r "$ScriptDir\requirements.txt" | Out-Null
    return $PythonExe
}

function Ensure-SystemPython {
    Write-Info "Using system-wide Python and pip..."
    python -m pip install -r "$ScriptDir\requirements.txt" | Out-Null
    return "python"
}

function Check-Environment {
    if (!(Test-Path $ServerScript)) {
        Write-ErrorMsg "$($ServerScript) not found in $ScriptDir"
        Write-ErrorMsg "Please run this script from the web_srv directory"
        exit 1
    }
}

function Get-ServerPid {
    if (Test-Path $PidFile) {
        $serverpid = Get-Content $PidFile
        if ($serverpid -and (Get-Process -Id $serverpid -ErrorAction SilentlyContinue)) {
            return $serverpid
        } else {
            Remove-Item $PidFile -ErrorAction SilentlyContinue
        }
    }
    # Try to find by process name and port
    $proc = Get-NetTCPConnection -LocalPort $Port -ErrorAction SilentlyContinue | Where-Object { $_.State -eq "Listen" }
    if ($proc) {
        $p = Get-Process -Id $proc.OwningProcess -ErrorAction SilentlyContinue
        if ($p) {
            Set-Content $PidFile $p.Id
            return $p.Id
        }
    }
    return $null
}

function Status-Server {
    Write-Info "Checking server status..."
    $serverpid = Get-ServerPid
    if ($serverpid) {
        Write-Status "Server is running (PID: $serverpid)"
        $tcp = Get-NetTCPConnection -LocalPort $Port -ErrorAction SilentlyContinue | Where-Object { $_.OwningProcess -eq $serverpid }
        if ($tcp) {
            Write-Status "Server listening on port: $Port"
            Write-Status "Dashboard URL: http://localhost:$Port"
            Write-Status "API Health: http://localhost:$Port/api/health"
        } else {
            Write-WarningMsg "Server process found but no listening port detected"
        }
        if (Test-Path $LogFile) {
            Write-Info "Log file: $LogFile"
            Write-Info "Last 3 log lines:"
            Get-Content $LogFile | Select-Object -Last 3 | ForEach-Object { Write-Host "  $_" }
        }
    } else {
        Write-WarningMsg "Server is not running"
    }
}

function Start-Server {
    $PythonExe = $null
    if ($Mode -eq "system") {
        $PythonExe = [string](Ensure-SystemPython)
    } else {
        $PythonExe = [string](Ensure-Venv)
    }
    # Đảm bảo đường dẫn tuyệt đối
    $PythonExe = (Resolve-Path $PythonExe).Path
    $ServerScript = (Resolve-Path $ServerScript).Path
    Write-Info "PythonExe: $PythonExe"
    Write-Info "ServerScript: $ServerScript"
    $serverpid = Get-ServerPid
    if ($serverpid) {
        Write-WarningMsg "Server is already running (PID: $serverpid)"
        Write-Info "Use 'start_server.ps1 status' to check details"
        Status-Server
        return
    }
    Write-Info "Server location: $ScriptDir"
    Write-Status "Starting server..."
    if ($RunMode -eq "background") {
        Write-Info "Starting server in BACKGROUND mode..."
        $LogFileErr = Join-Path $ScriptDir "server_aiohttp.err.log"
        $process = Start-Process -FilePath $PythonExe -ArgumentList $ServerScript `
            -WorkingDirectory $ScriptDir `
            -RedirectStandardOutput $LogFile `
            -RedirectStandardError $LogFileErr `
            -NoNewWindow `
            -PassThru
        if ($process) {
            Set-Content $PidFile $process.Id
            Write-Status "Server started in background (PID: $($process.Id))"
            Write-Info "Log file (stdout): $LogFile"
            Write-Info "Log file (stderr): $LogFileErr"
        } else {
            Write-ErrorMsg "Failed to start server in background. Check permissions and paths."
        }
    } else {
        Write-Info "Starting server in FOREGROUND mode..."
        & $PythonExe $ServerScript *> $LogFile
    }
    # Sau khi chạy, kiểm tra trạng thái
    Status-Server
    Write-Status "Server startup completed."
}

function Stop-Server {
    Write-Info "Stopping HTML Report Server..."
    $serverpid = Get-ServerPid
    if (!$serverpid) {
        Write-WarningMsg "Server is not running"
        return
    }
    Write-Status "Stopping server (PID: $serverpid)..."
    try {
        Stop-Process -Id $serverpid -Force
        Write-Status "Server stopped successfully"
    } catch {
        Write-ErrorMsg "Failed to stop server"
    }
    Remove-Item $PidFile -ErrorAction SilentlyContinue
    Start-Sleep -Seconds 1
    $tcp = Get-NetTCPConnection -LocalPort $Port -ErrorAction SilentlyContinue | Where-Object { $_.OwningProcess -eq $serverpid }
    if (!$tcp) {
        Write-Status "All server ports have been released"
    } else {
        Write-WarningMsg "Some ports may still be in use:"
        $tcp | ForEach-Object { Write-Host "  $_" }
    }
}

function Restart-Server {
    Write-Info "Restarting HTML Report Server..."
    Write-Info "Force stopping any existing servers..."
    $procs = Get-Process | Where-Object { $_.ProcessName -like "python*" }
    foreach ($p in $procs) {
        try {
            Stop-Process -Id $p.Id -Force
        } catch {}
    }
    Write-Info "Waiting for port $Port to be released..."
    $count = 0
    while ($count -lt 5) {
        $tcp = Get-NetTCPConnection -LocalPort $Port -ErrorAction SilentlyContinue
        if (!$tcp) {
            Write-Info "Port $Port is now free"
            break
        }
        Start-Sleep -Seconds 1
        $count++
    }
    if ($count -eq 5) {
        Write-WarningMsg "Port may still be in use, but proceeding..."
    }
    Remove-Item $PidFile, $LogFile -ErrorAction SilentlyContinue
    Write-Info "Starting fresh server..."
    Start-Server
}

Check-Environment

switch ($Action.ToLower()) {
    "start"   { Start-Server }
    "stop"    { Stop-Server }
    "restart" { Restart-Server }
    "status"  { Status-Server }
    default   {
        Write-ErrorMsg "Unknown command: $Action"
        Write-Host "Usage: .\start_server.ps1 [start|stop|restart|status] [-Mode venv|system]"
    }
}