
# Features
CodeGuard supports following features using AI:
- Detect standard Misra C++ violation
- Detect standard Certcpp C++ violation
- Detect Custom rules defined by user
- Provide refactoring code to avoid C++ vulnerability and increase code quality
- Provide an integrated web server to display violations in Gerrit style
- Combine with MCP (Model Context Protocol) server to boost AI capability

# Requirements
- VSCode 1.101.1 or higher
- For Linux, ubuntu 20.04 or higher

# Installation Guide

## Install CodeGuard Extension & MCP Server

### Windows
1. Install the CodeGuard extension from the VSCode Marketplace.
2. Open Command Palette (Ctrl+Shift+P), type `LGEDV: Install MCP Server` and select the command.
3. The extension will open a terminal and run the PowerShell script `install_mcp.ps1` to install MCP server in `C:\mcp_test` (default).
4. After installation, you will see a success notification and a sample configuration file `.vscode/mcp.json` created in your project folder.
5. If you encounter Python errors, please install Python 3.9+ from https://www.python.org/downloads/windows/ before running the installation command.

### Linux
1. Install the CodeGuard extension from the VSCode Marketplace.
2. Open Command Palette (Ctrl+Shift+P), type `LGEDV: Install MCP Server` and select the command.
3. The extension will open a terminal and run the Bash script `install_mcp.sh` to install MCP server in `~/mcp_test` (default).
4. After installation, you will see a success notification and a sample configuration file `.vscode/mcp.json` created in your project folder.
5. If you need to adjust the path for `CPP_DIR` or `CUSTOM_RULE_PATH`, edit the `.vscode/mcp.json` file to match your project.
6. If you encounter Python errors, please install Python 3.9+ using your package manager (e.g., `sudo apt install python3 python3-venv python3-pip`).

# MCP Server Configuration (`.vscode/mcp.json`)

Example:
```json
{
  "servers": {
    "lgedv": {
      "command": "C:\\mcp_test\\venv\\Scripts\\codeguard.exe",
      "args": ["--transport", "stdio"],
      "env": {
        "CPP_DIR": "D:\\src\\codefun\\code_test",           // Path to C++ source files for analysis (memory leak, resource leak, race condition)
        "CUSTOM_RULE_PATH": "C:\\path\\to\\CustomRule.md"   // Path to user-defined custom rule file (used for 'check_custom' prompt)
      }
    }
  }
}
```

- `CPP_DIR`: Path to the folder containing C++ files to be analyzed (memory leak, resource leak, race condition).
- `CUSTOM_RULE_PATH`: Path to the custom rule file defined by the user, used for the `check_custom` prompt.

Make sure to update these values to match your project before running MCP server or using the extension.

# How to Use the Extension

## Main Features & Commands

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

### 8. Start Web Server (View Violation Reports)
- Command: `LGEDV: Start Web Server`
- Usage: Start integrated web server to view violation reports in browser.
- On Windows: runs `start_server.ps1` via PowerShell.
- On Linux/macOS: runs `start_server.sh` via Bash.

### 9. Stop/Restart Web Server
- Command: `LGEDV: Stop Web Server`, `LGEDV: Restart Web Server`
- Usage: Stop or restart the integrated web server.

# MCP Server Usage

MCP server can be used independently or via the extension. You only need to use the following prompts (the server will automatically call the correct tool):

### Main prompts
- `check_lgedv`: Check LGEDV rule violations
- `check_misra`: Check MISRA C++ rule violations
- `check_certcpp`: Check CERT C++ rule violations
- `check_custom`: Check user-defined custom rule violations
- `check_races`: Analyze race conditions in code
- `check_leaks`: Analyze memory leaks in code
- `check_resources`: Analyze resource leaks in code

Each prompt will automatically run the corresponding analysis and return results or suggestions for code improvement.

# Uninstall MCP Server
- Delete the MCP server installation folder (e.g., `C:\mcp_test` or `~/mcp_test`) to completely remove it.

# Notes
- Make sure you have the required permissions to run scripts on your OS.