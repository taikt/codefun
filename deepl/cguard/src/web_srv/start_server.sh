#!/bin/bash

# Server management script for HTML Report Server (aiohttp only)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SERVER_SCRIPT="$SCRIPT_DIR/server_aiohttp.py"
PID_FILE="$SCRIPT_DIR/server_aiohttp.pid"
LOG_FILE="$SCRIPT_DIR/server_aiohttp.log"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_info() {
    echo -e "${BLUE}[DEBUG]${NC} $1"
}

# ---[ Python environment auto-check & setup ]---
PYTHON_OK=1
PYTHON_BIN="python3"
VENV_DIR="$SCRIPT_DIR/venv"

# Check python3
if ! command -v python3 >/dev/null 2>&1; then
    print_error "Python3 not found! Please install python3."
    exit 1
fi

# Check/cài python3-venv nếu có sudo
if ! $PYTHON_BIN -m venv --help >/dev/null 2>&1; then
    if command -v sudo >/dev/null 2>&1; then
        print_status "Installing python3-venv..."
        sudo apt update && sudo apt install -y python3-venv python3-full
    else
        print_error "python3-venv not found and no sudo permission. Please ask admin to install python3-venv."
        exit 1
    fi
fi

if [ ! -d "$VENV_DIR" ]; then
    python3 -m venv "$VENV_DIR"
    if [ ! -f "$VENV_DIR/bin/activate" ]; then
        print_error "Failed to create venv! Please check python3-venv, permissions, and Python version."
        exit 1
    fi
fi
source "$VENV_DIR/bin/activate"
pip install -r "$SCRIPT_DIR/requirements.txt"

# Đảm bảo pip có sẵn
if ! command -v pip >/dev/null 2>&1; then
    print_status "Installing pip in venv..."
    $PYTHON_BIN -m ensurepip --upgrade
fi

# ---[ End Python env check ]---

# Check if we're in the right directory
check_environment() {
    if [[ ! -f "$SERVER_SCRIPT" ]]; then
        print_error "$(basename $SERVER_SCRIPT) not found in $SCRIPT_DIR"
        print_error "Please run this script from the web_srv directory"
        print_error "Or check that you have generated server_aiohttp.py"
        exit 1
    fi
}

# Get server process ID
get_server_pid() {
    if [[ -f "$PID_FILE" ]]; then
        local pid=$(cat "$PID_FILE" 2>/dev/null)
        if [[ -n "$pid" ]] && kill -0 "$pid" 2>/dev/null; then
            echo "$pid"
            return 0
        else
            rm -f "$PID_FILE" 2>/dev/null
        fi
    fi
    local pid=$(pgrep -f "python3.*$(basename "$SERVER_SCRIPT")" 2>/dev/null | head -1)
    if [[ -n "$pid" ]]; then
        echo "$pid" > "$PID_FILE"
        echo "$pid"
        return 0
    fi
    return 1
}

# Check server status
status_server() {
    print_info "Checking server status..."
    local pid=$(get_server_pid)
    if [[ -n "$pid" ]]; then
        print_status "Server is running (PID: $pid)"
        local port_info=$(netstat -tlnp 2>/dev/null | grep ":888[0-9]" | grep "$pid" | head -1)
        if [[ -n "$port_info" ]]; then
            local port=$(echo "$port_info" | awk '{print $4}' | cut -d':' -f2)
            print_status "Server listening on port: $port"
            print_status "Dashboard URL: http://localhost:$port"
            print_status "API Health: http://localhost:$port/api/health"
        else
            print_warning "Server process found but no listening port detected"
        fi
        if [[ -f "$LOG_FILE" ]]; then
            print_info "Log file: $LOG_FILE"
            print_info "Last 3 log lines:"
            tail -3 "$LOG_FILE" 2>/dev/null | sed 's/^/  /'
        fi
        return 0
    else
        print_warning "Server is not running"
        return 1
    fi
}

# Start the server
start_server() {
    print_info "Starting HTML Report Server (aiohttp)..."
    local pid=$(get_server_pid)
    if [[ -n "$pid" ]]; then
        print_warning "Server is already running (PID: $pid)"
        print_info "Use './start_server.sh status' to check details"
        return 1
    fi
    print_info "Server location: $SCRIPT_DIR"
    print_status "Starting server..."
    # Activate venv if exists
    if [[ -d "$SCRIPT_DIR/venv" ]]; then
        print_info "Activating Python virtual environment (venv)"
        source "$SCRIPT_DIR/venv/bin/activate"
        PYTHON_BIN="python"
    else
        PYTHON_BIN="python3"
    fi
    nohup $PYTHON_BIN "$SERVER_SCRIPT" > "$LOG_FILE" 2>&1 &
    local new_pid=$!
    echo "$new_pid" > "$PID_FILE"
    sleep 2
    if kill -0 "$new_pid" 2>/dev/null; then
        print_status "Server started successfully (PID: $new_pid)"
        sleep 1
        # In log đường dẫn report HTML
        local report_dir_log=$(grep "Web server will serve HTML reports from:" "$LOG_FILE" | tail -1)
        if [[ -n "$report_dir_log" ]]; then
            print_status "$report_dir_log"
        else
            print_info "(Chưa xác định được thư mục report, kiểm tra log sau vài giây nếu cần)"
        fi
        local port_info=$(netstat -tlnp 2>/dev/null | grep "$new_pid" | grep ":888[0-9]" | head -1)
        if [[ -n "$port_info" ]]; then
            local port=$(echo "$port_info" | awk '{print $4}' | cut -d':' -f2)
            print_status "Server is listening on port: $port"
            print_status "� Dashboard: http://localhost:$port"
            print_status "🔗 API Health: http://localhost:$port/api/health"
        else
            print_info "Server is starting up, port detection may take a moment..."
        fi
        print_info "Log file: $LOG_FILE"
        print_info "Use './start_server.sh status' to check server status"
        print_info "Use './start_server.sh stop' to stop the server"
        print_status "Server startup completed. Terminal will exit automatically in 5 seconds..."
        sleep 5
        exit 0
    else
        print_error "Failed to start server"
        rm -f "$PID_FILE" 2>/dev/null
        if [[ -f "$LOG_FILE" ]]; then
            print_error "Check log file for details: $LOG_FILE"
            print_error "Last few log lines:"
            tail -5 "$LOG_FILE" 2>/dev/null | sed 's/^/  /'
        fi
        exit 1
    fi
}

# Stop the server
stop_server() {
    print_info "Stopping HTML Report Server..."
    local pid=$(get_server_pid)
    if [[ -z "$pid" ]]; then
        print_warning "Server is not running"
        return 0
    fi
    print_status "Stopping server (PID: $pid)..."
    if kill -TERM "$pid" 2>/dev/null; then
        print_info "Sent SIGTERM to server, waiting for graceful shutdown..."
        local count=0
        while [[ $count -lt 10 ]] && kill -0 "$pid" 2>/dev/null; do
            sleep 1
            ((count++))
        done
        if kill -0 "$pid" 2>/dev/null; then
            print_warning "Server did not shut down gracefully, forcing termination..."
            if kill -KILL "$pid" 2>/dev/null; then
                print_status "Server force terminated"
            else
                print_error "Failed to force terminate server"
                return 1
            fi
        else
            print_status "Server stopped gracefully"
        fi
    else
        print_error "Failed to send stop signal to server"
        return 1
    fi
    rm -f "$PID_FILE" 2>/dev/null
    sleep 1
    local remaining_ports=$(netstat -tlnp 2>/dev/null | grep ":888[0-9]" | grep "python3")
    if [[ -z "$remaining_ports" ]]; then
        print_status "All server ports have been released"
    else
        print_warning "Some ports may still be in use:"
        echo "$remaining_ports" | sed 's/^/  /'
    fi
    print_status "Server stopped successfully"
    return 0
}

# Restart the server
restart_server() {
    print_info "Restarting HTML Report Server..."
    print_info "Force stopping any existing servers..."
    pkill -f "$(basename "$SERVER_SCRIPT")" 2>/dev/null || true
    pkill -f "python3.*8888" 2>/dev/null || true
    print_info "Waiting for processes to terminate..."
    sleep 3
    pkill -9 -f "$(basename "$SERVER_SCRIPT")" 2>/dev/null || true
    print_info "Waiting for port 8888 to be released..."
    local count=0
    while [[ $count -lt 15 ]]; do
        if python3 -c "import socket; s=socket.socket(); s.bind(('', 8888)); s.close()" 2>/dev/null; then
            print_info "Port 8888 is now free"
            break
        fi
        sleep 1
        ((count++))
    done
    if [[ $count -eq 15 ]]; then
        print_warning "Port may still be in use, but proceeding..."
    fi
    rm -f "$PID_FILE" "$LOG_FILE" 2>/dev/null
    print_info "Starting fresh server..."
    start_server
}

# Show usage
show_usage() {
    echo "HTML Report Server Management Script (aiohttp only)"
    echo ""
    echo "Usage: $0 {start|stop|restart|status}"
    echo ""
    echo "Commands:"
    echo "  start    - Start the server in background (aiohttp)"
    echo "  stop     - Stop the server and release all resources"
    echo "  restart  - Stop and start the server"
    echo "  status   - Check current server status"
    echo ""
    echo "Examples:"
    echo "  $0 start           # Start aiohttp server"
    echo "  $0 status          # Check status of aiohttp server"
    echo "  $0 restart         # Restart aiohttp server"
    echo "  $0 stop            # Stop aiohttp server"
    echo ""
}

# Main script logic
main() {
    check_environment
    case "$1" in
        start)
            start_server
            ;;
        stop)
            stop_server
            ;;
        restart)
            restart_server
            ;;
        status)
            status_server
            ;;
        "")
            print_info "No command specified, starting server (use 'start' explicitly in future)"
            start_server
            ;;
        *)
            print_error "Unknown command: $1"
            show_usage
            exit 1
            ;;
    esac
}

main "$@"
