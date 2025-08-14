# CodeGuard - tool for detecting C++ vulnerability
## ✨ Features
MCP server CodeGuard supports following features using AI:
- Detect standard Misra C++ violation
- Detect standard Certcpp C++ violation
- Detect Custom rules defined by user
- Analyze race condition, memory leak and other resource leaks (socket, file, etc)
- Provide refactoring code to avoid C++ vulnerability and increase code quality


## 🎯 Requirements
- VSCode 1.101.1 or higher
- For Linux, ubuntu 20.04 or higher


## 🧩 Installation 
### Windows
- Install Python from https://www.python.org/downloads/windows/ 
- Open Command Palette (Ctrl+Shift+P), type `LGEDV: Install MCP Server` and select the command.
- The extension will install MCP server in `C:\Program Files\MCP Server CodeGuard` (default).
- After installation, a sample configuration file `.vscode/mcp.json` is created in your project folder.

![MCP server installation](https://raw.githubusercontent.com/taikt/codefun/main/deepl/cguard/src/images/mcp_usage.gif)

### Linux
- Install Python using your package manager (e.g., `sudo apt install python3 python3-venv python3-pip`).
- Open Command Palette (Ctrl+Shift+P), type `LGEDV: Install MCP Server` and select the command.
- The extension will install MCP server in `~/mcp_server` (default).
- After installation, a sample configuration file `.vscode/mcp.json` is created in your project folder.
- If you need to adjust the path for `CPP_DIR` or `CUSTOM_RULE_PATH`, edit the `.vscode/mcp.json` file to match your project.


### MCP Server Configuration
Create `.vscode/mcp.json` in your project codebase with the following format (Window sample)
```json
{
  "servers": {
    "lgedv": {
      "command": "C:\\Program Files\\MCP Server CodeGuard\\venv\\Scripts\\codeguard.exe",
      "args": ["--transport", "stdio"],
      "env": {
        "CPP_DIR": "C:\\path\\to\\source_code", // change this line
        "CUSTOM_RULE_PATH": "C:\\path\\to\\CustomRule.md" // change this line
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



## 🌐 MCP Server Usage

For rule-based checks, you must download the rule set before running the check prompt. If the rule set was downloaded, skip this step.

### MCP Server Prompts 
- **LGEDV Rule Check**
  - Download the latest LGEDV rules by calling the tool: `#fetch_lgedv_rule`.
  - Run the prompt `/check_lgedv` to check LGEDV rule violations.

- **MISRA Rule Check**
  - Download the latest MISRA rules by calling the tool: `#fetch_misra_rule`.
  - Run the prompt `/check_misra` to check MISRA C++ rule violations.

- **CERTCPP Rule Check**
  - Download the latest CERTCPP rules by calling the tool: `#fetch_certcpp_rule`.
  - Run the prompt `/check_certcpp` to check CERT C++ rule violations.

- **Custom Rule Check**
  - Download or specify your custom rule file by calling the tool: `#fetch_custom_rule`.
  - Run the prompt `/check_custom` to check user-defined custom rule violations.

- **Race Condition, Memory Leak, Resource Leak Analysis**
  - Configure the C++ source path in your `mcp.json` file, e.g: `"CPP_DIR": "D:\\src\\codefun\\code_test"`.
  - Due to AI token limitations, MCP server will check up to **3 files** in the specified source path per analysis run.
  - Available prompts:
  - `/check_races`: Analyze race condition
  - `/check_leaks`: Analyze memory leak
  - `/check_resources`: Analyze resource leak


## Uninstall MCP Server
- Delete the MCP server installation folder (e.g., `C:\\Program Files\\MCP Server CodeGuard`) to completely remove it.


<!--
## 🎯 Using CodeGuard VSCode Extension
### Setup custom rule File & report directory

- Open VSCode, go to `File` > `Preferences` > `Settings` (or press `Ctrl+,`).
- Search for `lge` in the settings search bar.
- Set the path for **Custom Rule File** (optional):
   - This is the markdown file containing your custom rules, used for the `check_custom` prompt.
   - Example: `D:\src\codefun\rules\MyCustomRule.md`
- Set the path for **Report Directory**:
   - This is the folder where violation reports will be saved.
   - Example: `D:\src\codefun\defect_report` or `${workspaceFolder}/defect_report`
- Save your settings. The extension will use these paths for custom rule checking and report output.



### Start/stop/restart Web Server (view violation report)
- Open Command Palette (Ctrl+Shift+P), type `LGEDV: Start Web Server` and select the command to start server.
- Open web browser (localhost:8888) to view violation reports.
- On Windows: runs `./start_server.ps1 stop/restart` via PowerShell to stop or restart server.
- On Linux/macOS: runs `./start_server.sh stop/restart` via Bash to stop or restart server.

### Check LGEDV Rule for Open File
- Command: `LGEDV: Check LGEDV Rule`
- Usage: Run on the currently open C++ file to detect LGEDV rule violations.

### Check MISRA Rule for Open File
- Command: `LGEDV: Check MISRA Rule`
- Usage: Run on the currently open C++ file to detect MISRA C++ violations.

### Check CERTCPP Rule for Open File
- Command: `LGEDV: Check CERTCPP Rule`
- Usage: Run on the currently open C++ file to detect CERT C++ violations.

### Check Custom Rule for Open File
- Command: `LGEDV: Check Custom Rule`
- Usage: Run on the currently open C++ file to detect user-defined custom rule violations.

### Check All Rules for Open File
- Command: `LGEDV: Check All Rules`
- Usage: Run all rule checks (LGEDV, MISRA, CERTCPP, Custom) on the current file.

### Generate LGEDV/MISRA/CERT/Custom Prompts
- Command: `LGEDV: Generate LGEDV Prompt`, `LGEDV: Generate MISRA Prompt`, ...
- Usage: Generate AI prompt for code review or refactoring based on selected rule set.

### Check Multiple Files
- Command: `LGEDV: Check Multiple Files (LGEDV/MISRA/CERT/Custom)`
- Usage: Run rule checks on multiple files in the workspace.


### Static Analysis (cppcheck, clang, clang-tidy)

#### Windows

**Install cppcheck:**
- Download installer from https://cppcheck.sourceforge.io/
- Run the installer. If not added to PATH automatically, add the install folder to your system PATH.
 
**Install clang/clang-tidy:**
- Download LLVM installer from https://releases.llvm.org/download.html.
- Run the installer and select "Add LLVM to the system PATH" if available.

#### Linux
- Install cppcheck: sudo apt install cppcheck
- Install clang/clang-tidy: sudo apt install clang clang-tidy

#### Run command
Command: `LGEDV: Check Rules by Static Analysic` on currently open file to detect C++ violations.

-->







