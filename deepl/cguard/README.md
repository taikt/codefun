# CodeGuard - tool for detecting C++ vulnerability
## Features
CodeGuard supports following features using AI:
- Detect standard Misra C++ violation
- Detect standard Certcpp C++ violation
- Detect Custom rules defined by user
- Provide refactoring code to avoid C++ vulnerability and increase code quality
- Provide an integrated web server to display violations in Gerrit style
- Combine with MCP (Model Context Protocol) server to boost AI capability



## Requirements
- VSCode 1.101.1 or higher
- For Linux, ubuntu 20.04 or higher


---
## Recommendations

- You can use the MCP server independently or together with the extension.
- **Recommended:** Use MCP server directly and combine with Copilot Chat for best results. This setup provides access to advanced AI models and delivers higher quality code review and analysis.
- **Limitation:** When using only the VSCode extension, the AI model quality is limited by what VSCode provides, which may result in less accurate or less powerful code review compared to MCP + Copilot Chat.

For professional code analysis and review, prefer using MCP server with Copilot Chat integration.

# Use MCP server
## Install MCP Server

### Windows
1. Install Python 3.9+ from https://www.python.org/downloads/windows/ 
2. Open Command Palette (Ctrl+Shift+P), type `LGEDV: Install MCP Server` and select the command.
3. The extension will open a terminal and run the PowerShell script `install_mcp.ps1` to install MCP server in `C:\mcp_server` (default).
4. After installation, you will see a success notification and a sample configuration file `.vscode/mcp.json` created in your project folder.


### Linux
1. Install Python 3.9+ using your package manager (e.g., `sudo apt install python3 python3-venv python3-pip`).
2. Open Command Palette (Ctrl+Shift+P), type `LGEDV: Install MCP Server` and select the command.
3. The extension will open a terminal and run the Bash script `install_mcp.sh` to install MCP server in `~/mcp_server` (default).
4. After installation, you will see a success notification and a sample configuration file `.vscode/mcp.json` created in your project folder.
5. If you need to adjust the path for `CPP_DIR` or `CUSTOM_RULE_PATH`, edit the `.vscode/mcp.json` file to match your project.




### MCP Server Configuration
Create `.vscode/mcp.json` in your project codebase with the following format:
```json
{
  "servers": {
    "lgedv": {
      "command": "C:\\mcp_test\\venv\\Scripts\\codeguard.exe",
      "args": ["--transport", "stdio"],
      "env": {
        "CPP_DIR": "C:\\path\\to\\source_code", // Path to C++ source files for analysis 
        "CUSTOM_RULE_PATH": "C:\\path\\to\\CustomRule.md" // Path to user-defined custom rule file
      }
    }
  }
}
```
- `CPP_DIR`: Path to the folder containing C++ files to be analyzed (memory leak, resource leak, race condition).
- e.g. "CPP_DIR": "D:\\src\\codefun\\code_test"
- `CUSTOM_RULE_PATH`: Path to the custom rule file defined by the user, used for the `check_custom` prompt.
- e.g.  "CUSTOM_RULE_PATH": "C:\\path\\to\\CustomRule.md"
- Open mcp.json on vscode and start MCP server by click start button.

---


## MCP Server Usage

MCP server can be used independently or via the extension. You only need to use the following prompts 
### Main prompts
- `check_lgedv`: Check LGEDV rule violations
- `check_misra`: Check MISRA C++ rule violations
- `check_certcpp`: Check CERT C++ rule violations
- `check_custom`: Check user-defined custom rule violations
- `check_races`: Analyze race conditions in code
- `check_leaks`: Analyze memory leaks in code
- `check_resources`: Analyze resource leaks in code


## Uninstall MCP Server
- Delete the MCP server installation folder (e.g., `C:\mcp_server` or `~/mcp_server`) to completely remove it.



# Use extension
## How to Setup Custom Rule File & Report Directory in Extension

1. Open VSCode, go to `File` > `Preferences` > `Settings` (or press `Ctrl+,`).
2. Search for `lge` in the settings search bar.
3. Set the path for **Custom Rule File** (optional):
   - This is the markdown file containing your custom rules, used for the `check_custom` prompt.
   - Example: `D:\src\codefun\rules\MyCustomRule.md`
4. Set the path for **Report Directory**:
   - This is the folder where violation reports will be saved.
   - Example: `D:\src\codefun\defect_report` or `${workspaceFolder}/defect_report`
5. Save your settings. The extension will use these paths for custom rule checking and report output.

## Extension usage

### 1. Check LGEDV Rule for Open File
- Command: `LGEDV: Check LGEDV Rule`
- Usage: Run on the currently open C++ file to detect LGEDV rule violations.

### 2. Check MISRA Rule for Open File
- Command: `LGEDV: Check MISRA Rule`
- Usage: Run on the currently open C++ file to detect MISRA C++ violations.

### 3. Check CERTCPP Rule for Open File
- Command: `LGEDV: Check CERTCPP Rule`
- Usage: Run on the currently open C++ file to detect CERT C++ violations.

### 4. Check Custom Rule for Open File
- Command: `LGEDV: Check Custom Rule`
- Usage: Run on the currently open C++ file to detect user-defined custom rule violations.

### 5. Check All Rules for Open File
- Command: `LGEDV: Check All Rules`
- Usage: Run all rule checks (LGEDV, MISRA, CERTCPP, Custom) on the current file.

### 6. Generate LGEDV/MISRA/CERT/Custom Prompts
- Command: `LGEDV: Generate LGEDV Prompt`, `LGEDV: Generate MISRA Prompt`, ...
- Usage: Generate AI prompt for code review or refactoring based on selected rule set.

### 7. Check Multiple Files
- Command: `LGEDV: Check Multiple Files (LGEDV/MISRA/CERT/Custom)`
- Usage: Run rule checks on multiple files in the workspace.

### 8. Start/stop/restart Web Server (View Violation Reports)
- Open Command Palette (Ctrl+Shift+P), type `LGEDV: Start Web Server` and select the command to start server.
- Open web browser (localhost:8888) to view violation reports.
- On Windows: runs `./start_server.ps1 stop/restart` via PowerShell to stop or restart server.
- On Linux/macOS: runs `./start_server.sh stop/restart` via Bash to stop or restart server.


