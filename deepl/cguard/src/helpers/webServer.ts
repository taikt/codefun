// Hàm start web server, tìm port
import * as vscode from 'vscode';
import * as http from 'http';
import * as path from 'path';
import * as fs from 'fs';
import * as zlib from 'zlib';

export async function startWebServerAndOpen(htmlFilePath: string, baseDir?: string, outputChannel?: vscode.OutputChannel): Promise<void> {
  try {
    const port = await findAvailablePort(8080);
    const directoryToServe = baseDir || path.dirname(htmlFilePath);
    const htmlFileName = path.basename(htmlFilePath);
    if (outputChannel) {
      outputChannel.appendLine(`🌐 Starting optimized web server on port ${port}...`);
    }
    const server = http.createServer((req, res) => {
      let filePath = req.url === '/' ? htmlFileName : req.url?.substring(1) || htmlFileName;
      const fullPath = path.join(directoryToServe, filePath);
      if (!fullPath.startsWith(directoryToServe)) {
        res.writeHead(403);
        res.end('Forbidden');
        return;
      }
      res.setHeader('Cache-Control', 'public, max-age=3600');
      res.setHeader('X-Content-Type-Options', 'nosniff');
      res.setHeader('X-Frame-Options', 'SAMEORIGIN');
      fs.readFile(fullPath, (err, data) => {
        if (err) {
          res.writeHead(404, { 'Content-Type': 'text/html' });
          res.end(`<!DOCTYPE html><html><head><title>File Not Found</title></head><body><h1>404 - File Not Found</h1><p>The requested file <code>${filePath}</code> was not found.</p><p><a href="/${htmlFileName}">Return to report</a></p></body></html>`);
          return;
        }
        const ext = path.extname(fullPath).toLowerCase();
        let contentType = 'text/html; charset=utf-8';
        if (ext === '.css') contentType = 'text/css; charset=utf-8';
        else if (ext === '.js') contentType = 'application/javascript; charset=utf-8';
        else if (ext === '.json') contentType = 'application/json; charset=utf-8';
        else if (ext === '.png') contentType = 'image/png';
        else if (ext === '.jpg' || ext === '.jpeg') contentType = 'image/jpeg';
        const shouldCompress = contentType.startsWith('text/') || contentType.includes('javascript') || contentType.includes('json');
        if (shouldCompress && req.headers['accept-encoding']?.includes('gzip')) {
          res.setHeader('Content-Encoding', 'gzip');
          res.writeHead(200, { 'Content-Type': contentType, 'Transfer-Encoding': 'chunked' });
          zlib.gzip(data, (err: NodeJS.ErrnoException | null, compressed: Buffer) => {
            if (err) {
              res.writeHead(500);
              res.end('Compression error');
              return;
            }
            res.end(compressed);
          });
        } else {
          res.writeHead(200, { 'Content-Type': contentType, 'Content-Length': data.length });
          res.end(data);
        }
      });
    });
    server.timeout = 30000;
    server.keepAliveTimeout = 5000;
    server.headersTimeout = 60000;
    server.listen(port, '0.0.0.0', () => {
      const url = `http://localhost:${port}/${htmlFileName}`;
      if (outputChannel) {
        outputChannel.appendLine(`✅ Server running at: ${url}`);
        outputChannel.appendLine(`📂 Serving directory: ${directoryToServe}`);
        outputChannel.appendLine(`⚡ Performance optimizations: Caching, Compression, Keep-Alive`);
        outputChannel.appendLine(`🌐 Network access: http://<your-ip>:${port}/${htmlFileName}`);
      }
      setTimeout(() => {
        vscode.env.openExternal(vscode.Uri.parse(url));
      }, 500);
      vscode.window.showInformationMessage(
        `🌐 Optimized HTML Report Server Started`,
        {
          detail: `Server: http://localhost:${port}\nFile: ${htmlFileName}\nOptimizations: Caching + Compression\nNetwork: http://<your-ip>:${port}/${htmlFileName}`,
          modal: false
        }
      );
    });
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
    }, 60000);
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
    const doc = await vscode.workspace.openTextDocument(htmlFilePath);
    await vscode.window.showTextDocument(doc);
  }
}

export async function findAvailablePort(startPort: number): Promise<number> {
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
