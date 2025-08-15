# CodeGuard - Detect C++ vulnerability using AI
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
- Install Python including PIP from https://www.python.org/downloads/windows/ 
- Open Command Palette (Ctrl+Shift+P), type `LGEDV CodeGuard: Install MCP Server` and select the command.
- The extension will install MCP server in `C:\Program Files\MCP Server CodeGuard` (default).
- Sample Configuration file mcp.json is created at C:\Program Files\MCP Server CodeGuard\.vscode\mcp.json
- In VSCode, open folder of source code to analyze. At Explorer bar, right click to create a folder named .vscode
- Copy mcp.json from C:\Program Files\MCP Server CodeGuard\.vscode\mcp.json to .vscode folder,. which is created above
- Change mcp.json to meet your project requirement
```json
"lgedv": {
      "command": "D:\\src\\codefun\\deepl\\mcp_server\\venv\\Scripts\\codeguard.exe",
      "args": ["--transport", "stdio"],
      "env": {
        "src_dir": "\\path\\to\\source_code",
        "custom_path": "\\path\\to\\custom_file",
        "reset_cache": "true",
        "prompt_lang": "vi", // en: english, vi: vietnamese
        "report_dir": "\\path\\to\\http_report"
      }
    }
```
- `src_dir`: Path to the folder containing C++ files to be analyzed (memory leak, resource leak, race condition).
- `custom_path`: Path to the custom rule file defined by the user, used for the `check_custom` prompt.
- `prompt_lang`: to select prompt type as English or Vietnamese. By default, prompt type is English.
- `Reset_cache`: to auto clean previous analysic cache when starting MCP server. If the flag is false, MCP server will skip to analyze files that already checked.
- `report_dir`: contain HTTP report directory.

![installation](https://raw.githubusercontent.com/taikt/codefun/main/deepl/cguard/src/images/install_win.gif)

### Linux
- Install Python using your package manager (e.g., `sudo apt install python3 python3-venv python3-pip`).
- Open Command Palette (Ctrl+Shift+P), type `LGEDV CodeGuard: Install MCP Server` and select the command.
- The extension will install MCP server in `~/mcp_server` (default).
- Sample Configuration file mcp.json is created at ~/mcp_server/mcp.json
- In VSCode, open folder of source code to analyze. At Explorer bar, right click to create a folder named .vscode
- Copy mcp.json from ~/mcp_server/mcp.json to folder .vscode, which is created above
- Change mcp.json to meet your project requirement


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
  - Due to AI token limitations, MCP server will check up to **3 files** in the specified source path per analysis run.
  - Available prompts:
  - `/check_races`: Analyze race condition
  - `/check_leaks`: Analyze memory leak
  - `/check_resources`: Analyze resource leak
  - `/reset_analysis`: Clear all previous analysics
  - `/reset_race_check`: Clear analysis cache for race condition
  - `/reset_mem_check`: Clear analysis cache for memory leak
  - `/reset_resouce_check`: Clear analysis cache for resource leak 


## Uninstall MCP Server
- Delete the MCP server installation folder (e.g., `C:\\Program Files\\MCP Server CodeGuard`) to completely remove it.
