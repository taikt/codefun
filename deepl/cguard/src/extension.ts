import * as vscode from 'vscode';
import { exec, spawn } from 'child_process';
import * as fs from 'fs';
import * as path from 'path';
import * as http from 'http';
import * as zlib from 'zlib';
import * as os from 'os';


// Import moved helpers
import { 
  openCopilotChatWithPrompt, logCodeContentForCopilot, runRuleAnalysis, connectToCopilot, selectBestModel, loadRules, optimizeRulesForPrompt, createFocusedPrompt, runAnalysisWithProgress, saveReport 
} from './helpers/copilot';
import { loadRuleFile, fileExists, createLineByLineDiff, createCodeWithLines } from './helpers/fileUtils';
import { countViolationsBySeverity, generateMarkdownReport, generateInteractiveHTMLReport } from './helpers/report';

// Import và sử dụng các command đã tách ra từ thư mục commands
import { registerCheckLGEDVCommand } from './commands/checkLGEDV';
import { registerCheckMISRACommand } from './commands/checkMISRA';
import { registerCheckCERTCommand } from './commands/checkCERT';
import { registerCheckRapidScanCommand } from './commands/checkRapidScan';
import { registerCheckAllRulesCommand } from './commands/checkAllRules';
import { registerCheckMultipleFilesLGEDVCommand } from './commands/checkMultipleFilesLGEDV';
import { registerCheckMultipleFilesMISRACommand } from './commands/checkMultipleFilesMISRA';
import { registerCheckMultipleFilesCERTCommand } from './commands/checkMultipleFilesCERT';
import { registerCheckMultipleFilesCriticalCommand } from './commands/checkMultipleFilesCritical';
import { registerGenerateLGEDVPromptCommand } from './commands/generateLGEDVPrompt';
import { registerGenerateMisraPromptCommand } from './commands/generateMisraPrompt';
import { registerGenerateCertCppPromptCommand } from './commands/generateCertCppPrompt';
import { registerGenerateRapidScanPromptCommand } from './commands/generateRapidScanPrompt';
import { registerGenerateCriticalRulePromptCommand } from './commands/generateCriticalRulePrompt';
import { registerCheckCustomRuleCommand } from './commands/checkCustomRule';
import { registerGenerateCustomRulePromptCommand } from './commands/generateCustomRulePrompt';
import { registerInteractiveCheckCommand } from './commands/interactiveCheck';
import { registerCheckStaticAnalysisCommand } from './commands/checkStaticAnalysis';

// Helper functions from extension_refer.ts
// Function to strip ANSI escape codes
const stripAnsi = (text: string): string => {
  return text.replace(/\x1B\[[0-?]*[ -/]*[@-~]/g, '');
};

// Check if a file is a C++ file based on its extension
const isCppFile = (filename: string): boolean => {
  // Only allow .cpp and .cc files (not .h, .hpp, etc)
  const ext = path.extname(filename).toLowerCase();
  return ext === '.cpp' || ext === '.cc';
};

// Web server for serving HTML reports
async function startWebServerAndOpen(htmlFilePath: string, baseDir?: string, outputChannel?: vscode.OutputChannel): Promise<void> {
  try {
    // Find available port
    const port = await findAvailablePort(8080);
    const directoryToServe = baseDir || path.dirname(htmlFilePath);
    const htmlFileName = path.basename(htmlFilePath);
    
    if (outputChannel) {
      outputChannel.appendLine(`🌐 Starting optimized web server on port ${port}...`);
    }
    
    // Create optimized HTTP server with caching and compression
    const server = http.createServer((req, res) => {
      let filePath = req.url === '/' ? htmlFileName : req.url?.substring(1) || htmlFileName;
      const fullPath = path.join(directoryToServe, filePath);
      
      // Security check - prevent directory traversal
      if (!fullPath.startsWith(directoryToServe)) {
        res.writeHead(403);
        res.end('Forbidden');
        return;
      }
      
      // Add performance headers
      res.setHeader('Cache-Control', 'public, max-age=3600'); // 1 hour cache
      res.setHeader('X-Content-Type-Options', 'nosniff');
      res.setHeader('X-Frame-Options', 'SAMEORIGIN');
      
      fs.readFile(fullPath, (err, data) => {
        if (err) {
          res.writeHead(404, { 'Content-Type': 'text/html' });
          res.end(`
            <!DOCTYPE html>
            <html>
            <head><title>File Not Found</title></head>
            <body>
              <h1>404 - File Not Found</h1>
              <p>The requested file <code>${filePath}</code> was not found.</p>
              <p><a href="/${htmlFileName}">Return to report</a></p>
            </body>
            </html>
          `);
          return;
        }
        
        // Set correct content type and compression
        const ext = path.extname(fullPath).toLowerCase();
        let contentType = 'text/html; charset=utf-8';
        if (ext === '.css') contentType = 'text/css; charset=utf-8';
        else if (ext === '.js') contentType = 'application/javascript; charset=utf-8';
        else if (ext === '.json') contentType = 'application/json; charset=utf-8';
        else if (ext === '.png') contentType = 'image/png';
        else if (ext === '.jpg' || ext === '.jpeg') contentType = 'image/jpeg';
        
        // Add compression for text files
        const shouldCompress = contentType.startsWith('text/') || contentType.includes('javascript') || contentType.includes('json');
        
        if (shouldCompress && req.headers['accept-encoding']?.includes('gzip')) {
          res.setHeader('Content-Encoding', 'gzip');
          res.writeHead(200, { 
            'Content-Type': contentType,
            'Transfer-Encoding': 'chunked'
          });
          zlib.gzip(data, (err: NodeJS.ErrnoException | null, compressed: Buffer) => {
            if (err) {
              res.writeHead(500);
              res.end('Compression error');
              return;
            }
            res.end(compressed);
          });
        } else {
          res.writeHead(200, { 
            'Content-Type': contentType,
            'Content-Length': data.length
          });
          res.end(data);
        }
      });
    });
    
    // Optimize server settings
    server.timeout = 30000; // 30 second timeout
    server.keepAliveTimeout = 5000; // 5 second keep-alive
    server.headersTimeout = 60000; // 60 second headers timeout
    
    server.listen(port, '0.0.0.0', () => { // Listen on all interfaces
      const url = `http://localhost:${port}/${htmlFileName}`;
      
      if (outputChannel) {
        outputChannel.appendLine(`✅ Server running at: ${url}`);
        outputChannel.appendLine(`📂 Serving directory: ${directoryToServe}`);
        outputChannel.appendLine(`⚡ Performance optimizations: Caching, Compression, Keep-Alive`);
        outputChannel.appendLine(`🌐 Network access: http://<your-ip>:${port}/${htmlFileName}`);
      }
      
      // Open in external browser with delay for server to be ready
      setTimeout(() => {
        vscode.env.openExternal(vscode.Uri.parse(url));
      }, 500);
      
      // Show notification with server info
      vscode.window.showInformationMessage(
        `🌐 Optimized HTML Report Server Started`,
        {
          detail: `Server: http://localhost:${port}\nFile: ${htmlFileName}\nOptimizations: Caching + Compression\nNetwork: http://<your-ip>:${port}/${htmlFileName}`,
          modal: false
        }
      );
    });
    
    // Auto-close server after 30 minutes of inactivity
    let lastActivity = Date.now();
    const activityCheck = setInterval(() => {
      if (Date.now() - lastActivity > 30 * 60 * 1000) {
        server.close(() => {
          if (outputChannel) {
            outputChannel.appendLine('🔄 Server auto-closed after 30 minutes of inactivity');
          }
        });
        clearInterval(activityCheck);
      }
    }, 60000); // Check every minute
    
    // Track activity
    server.on('request', () => {
      lastActivity = Date.now();
    });
    
  } catch (error) {
    const errorMsg = error instanceof Error ? error.message : 'Unknown error';
    vscode.window.showErrorMessage(`Failed to start web server: ${errorMsg}`);
    
    if (outputChannel) {
      outputChannel.appendLine(`❌ Web server error: ${errorMsg}`);
      outputChannel.appendLine('💡 Fallback: Opening file directly in VS Code...');
    }
    
    // Fallback to opening in VS Code
    const doc = await vscode.workspace.openTextDocument(htmlFilePath);
    await vscode.window.showTextDocument(doc);
  }
}

// Find available port starting from a base port
async function findAvailablePort(startPort: number): Promise<number> {
  return new Promise((resolve) => {
    const server = http.createServer();
    
    const tryPort = (port: number) => {
      server.listen(port, 'localhost', () => {
        server.close(() => resolve(port));
      }).on('error', () => {
        tryPort(port + 1);
      });
    };
    
    tryPort(startPort);
  });
}

export function getReportDirectory(): string {
  const config = vscode.workspace.getConfiguration('lgedvCodeGuard');
  let dir = config.get<string>('reportDirectory');
  if (dir && dir.trim()) {
    // Nếu user đã setup, dùng đúng đường dẫn user chỉ định (có thể có ${workspaceFolder})
    if (dir.includes('${workspaceFolder}')) {
      const wsFolder = vscode.workspace.workspaceFolders?.[0]?.uri.fsPath || '';
      dir = dir.replace('${workspaceFolder}', wsFolder);
    }
    // Nếu không phải tuyệt đối, resolve theo workspace nếu có, nếu không thì dùng luôn tên thư mục tương đối trong workspace
    if (!path.isAbsolute(dir)) {
      const wsFolder = vscode.workspace.workspaceFolders?.[0]?.uri.fsPath;
      if (wsFolder) {
        dir = path.join(wsFolder, dir);
      } else {
        // Nếu không có workspace, fallback về thư mục hiện tại của extension (nhưng KHÔNG dùng src/web_srv)
        dir = path.resolve(dir); // resolve từ current working dir
      }
    }
    return dir;
  } else {
    // Nếu user để trống, tạo defect_report trong workspace (nếu có workspace)
    const wsFolder = vscode.workspace.workspaceFolders?.[0]?.uri.fsPath;
    if (wsFolder) {
      const defectReportDir = path.join(wsFolder, 'defect_report');
      if (!fs.existsSync(defectReportDir)) {
        fs.mkdirSync(defectReportDir, { recursive: true });
      }
      return defectReportDir;
    } else {
      // Nếu không có workspace, fallback về thư mục hiện tại
      const fallbackDir = path.resolve('defect_report');
      if (!fs.existsSync(fallbackDir)) {
        fs.mkdirSync(fallbackDir, { recursive: true });
      }
      return fallbackDir;
    }
  }
}

function getWebServerDirectory(): string {
  // Ưu tiên lấy src/web_srv trong workspace nếu tồn tại, fallback về __dirname
  const wsFolder = vscode.workspace.workspaceFolders?.[0]?.uri.fsPath;
  if (wsFolder) {
    const wsWebSrv = path.join(wsFolder, 'src', 'web_srv');
    if (fs.existsSync(wsWebSrv)) {
      return wsWebSrv;
    }
  }
  // Fallback: lấy theo vị trí extension đã đóng gói
  return path.join(__dirname, '..', 'src', 'web_srv');
}

function getRuleSetDirectory(): string {
  const config = vscode.workspace.getConfiguration('lgedvCodeGuard');
  const customPath = config.get<string>('ruleSetDirectory');
  return customPath || path.join(__dirname, '..', 'src', 'rule_set');
}

function getAutoOpenBrowser(): boolean {
  const config = vscode.workspace.getConfiguration('lgedvCodeGuard');
  return config.get<boolean>('autoOpenBrowser') ?? true;
}

export function activate(context: vscode.ExtensionContext) {
  
  // Register modularized commands from commands directory
  registerCheckLGEDVCommand(context);
  registerCheckMISRACommand(context);
  registerCheckCERTCommand(context);
  registerCheckRapidScanCommand(context);
  registerCheckAllRulesCommand(context);
  registerCheckMultipleFilesLGEDVCommand(context);
  registerCheckMultipleFilesMISRACommand(context);
  registerCheckMultipleFilesCERTCommand(context);
  registerCheckMultipleFilesCriticalCommand(context);
  registerGenerateLGEDVPromptCommand(context);
  registerGenerateMisraPromptCommand(context);
  registerGenerateCertCppPromptCommand(context);
  registerGenerateRapidScanPromptCommand(context);
  registerGenerateCriticalRulePromptCommand(context);
  registerCheckCustomRuleCommand(context);
  registerGenerateCustomRulePromptCommand(context);
  registerInteractiveCheckCommand(context);
  registerCheckStaticAnalysisCommand(context);

  // Register other (non-modularized) commands directly
  // (Do NOT register commands here that are already modularized above)

  // COMMAND: Start Web Server (use start_server.sh)
  let startWebServerCommand = vscode.commands.registerCommand('lgedv.startWebServer', async () => {
    const webSrvDir = getWebServerDirectory();
    let scriptPath: string;
    let command: string;

    if (os.platform() === 'win32') {
      // Windows: dùng PowerShell script
      scriptPath = path.join(webSrvDir, 'start_server.ps1');
      command = `powershell -ExecutionPolicy Bypass -File "${scriptPath}" start`;
    } else {
      // Linux/macOS: dùng shell script
      scriptPath = path.join(webSrvDir, 'start_server.sh');
      command = `bash "${scriptPath}" start`;
    }

    const terminal = vscode.window.createTerminal({
      name: 'LGEDV Web Server',
      cwd: webSrvDir
    });
    terminal.show();
    terminal.sendText(command);
    vscode.window.showInformationMessage(`🌐 LGEDV Web Server started via ${os.platform() === 'win32' ? 'start_server.ps1' : 'start_server.sh'} (check terminal for output).`);
  });
  
  // COMMAND: Stop Web Server (use start_server.sh)
  let stopWebServerCommand = vscode.commands.registerCommand('lgedv.stopWebServer', async () => {
    const webSrvDir = getWebServerDirectory();
    let scriptPath: string;
    let command: string;

    if (os.platform() === 'win32') {
      scriptPath = path.join(webSrvDir, 'start_server.ps1');
      command = `powershell -ExecutionPolicy Bypass -File "${scriptPath}" stop`;
    } else {
      scriptPath = path.join(webSrvDir, 'start_server.sh');
      command = `bash "${scriptPath}" stop`;
    }

    const terminal = vscode.window.createTerminal({
      name: 'LGEDV Web Server',
      cwd: webSrvDir
    });
    terminal.show();
    terminal.sendText(command);
    vscode.window.showInformationMessage(`🛑 LGEDV Web Server stopped via ${os.platform() === 'win32' ? 'start_server.ps1' : 'start_server.sh'}.`);
  });

  // COMMAND: Restart Web Server (use start_server.sh)
  let restartWebServerCommand = vscode.commands.registerCommand('lgedv.restartWebServer', async () => {
    const webSrvDir = getWebServerDirectory();
    let scriptPath: string;
    let command: string;

    if (os.platform() === 'win32') {
      scriptPath = path.join(webSrvDir, 'start_server.ps1');
      command = `powershell -ExecutionPolicy Bypass -File "${scriptPath}" restart`;
    } else {
      scriptPath = path.join(webSrvDir, 'start_server.sh');
      command = `bash "${scriptPath}" restart`;
    }

    const terminal = vscode.window.createTerminal({
      name: 'LGEDV Web Server',
      cwd: webSrvDir
    });
    terminal.show();
    terminal.sendText(command);
    vscode.window.showInformationMessage(`🔄 LGEDV Web Server restarted via ${os.platform() === 'win32' ? 'start_server.ps1' : 'start_server.sh'}.`);
  });

  // start MCP server command
  let installMCPServerCommand = vscode.commands.registerCommand('lgedv.installMCPServer', async () => {
  const resourcesDir = path.join(__dirname, '..', 'src', 'resources');
  let scriptPath: string;
  let command: string;

  if (os.platform() === 'win32') {
    scriptPath = path.join(resourcesDir, 'install_mcp.ps1');
    command = `powershell -ExecutionPolicy Bypass -File "${scriptPath}"`;
  } else {
    scriptPath = path.join(resourcesDir, 'install_mcp.sh');
    command = `bash "${scriptPath}"`;
  }

  const terminal = vscode.window.createTerminal({
    name: 'LGEDV MCP Server Installer',
    cwd: resourcesDir
  });
  terminal.show();
  terminal.sendText(command);
  vscode.window.showInformationMessage(`🚀 Đang cài MCP server qua script (${os.platform() === 'win32' ? 'install_mcp.ps1' : 'install_mcp.sh'})!`);
});

  context.subscriptions.push(
    startWebServerCommand,
    stopWebServerCommand,
    restartWebServerCommand,
    installMCPServerCommand
  );
}

export function deactivate() {}
