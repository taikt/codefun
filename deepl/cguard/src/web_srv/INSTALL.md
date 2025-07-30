
# INSTALLATION GUIDE FOR CGUARD TOOL ON WINDOWS

## 1. Install Python and pip
- Go to https://www.python.org/downloads/
- Download and install Python (recommended version: 3.8 or newer).
- During installation, select "Add Python to PATH".
- After installation, check in PowerShell:
  ```powershell
  python --version
  pip --version
  ```
- If the version is displayed, Python and pip are installed successfully.

## 2. Install CGuard Tool
- Extract or clone the source code to your desired folder.
- Open PowerShell and navigate to `codefun\deepl\cguard\src\web_srv`.
- Install required packages:
  - The script will automatically create a virtual environment (venv) and install packages on first run.
  - To install manually:
    ```powershell
    python -m venv venv
    .\venv\Scripts\Activate.ps1
    pip install -r requirements.txt
    ```

## 3. Start/Stop/Restart the Web Server
- Use the following commands in PowerShell from the `web_srv` directory:
  ```powershell
  # Start the server
  .\start_server.ps1 start

  # Stop the server
  .\start_server.ps1 stop

  # Restart the server
  .\start_server.ps1 restart

  # Check server status
  .\start_server.ps1 status
  ```
- The server runs on port 8888. Access: http://localhost:8888

## Notes
- If you encounter errors with Python or pip, check step 1.
- To use system-wide Python instead of venv:
  ```powershell
  .\start_server.ps1 start -Mode system
  ```
- Log and PID files are saved in the same directory as the script.

---
For questions, please contact the administrator or see README.md.
