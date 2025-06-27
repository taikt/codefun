#!/usr/bin/env python3
import http.server
import socketserver
import json
import os
import glob
import time

def get_report_directory():
    """
    Get report directory from VS Code settings.
    Falls back to default if settings not found or empty.
    """
    default_dir = 'defect_report'  # Relative to web_srv directory
    
    try:
        # VS Code settings locations (in order of preference):
        # 1. Workspace settings (.vscode/settings.json in workspace root)
        # 2. User settings (~/.config/Code/User/settings.json on Linux)
        settings_paths = [
            # Workspace settings (highest priority)
            '/home/worker/src/.vscode/settings.json',
            # User settings (Linux)
            os.path.expanduser('~/.config/Code/User/settings.json'),
            # User settings (alternative Linux location)
            os.path.expanduser('~/.vscode/settings.json'),
        ]
        
        print("🔍 Searching for VS Code settings in official locations...")
        for settings_path in settings_paths:
            if os.path.exists(settings_path):
                print(f"📄 Found settings file: {settings_path}")
                try:
                    with open(settings_path, 'r', encoding='utf-8') as f:
                        content = f.read()
                        # Handle JSON with comments (jsonc format used by VS Code)
                        lines = []
                        for line in content.split('\n'):
                            # Remove line comments
                            if '//' in line:
                                comment_pos = line.find('//')
                                # Make sure it's not inside a string
                                in_string = False
                                quote_count = 0
                                for i, char in enumerate(line[:comment_pos]):
                                    if char == '"' and (i == 0 or line[i-1] != '\\'):
                                        quote_count += 1
                                if quote_count % 2 == 0:  # Even number of quotes = not in string
                                    line = line[:comment_pos]
                            lines.append(line)
                        clean_content = '\n'.join(lines)
                        
                        settings = json.loads(clean_content)
                        report_dir = settings.get('lgedvCodeGuard.reportDirectory')
                        if report_dir and report_dir.strip():
                            abs_report_dir = os.path.abspath(os.path.expanduser(report_dir))
                            print(f"✅ Using configured report directory: {abs_report_dir}")
                            return abs_report_dir
                        else:
                            print(f"⚠️ Setting 'lgedvCodeGuard.reportDirectory' not found or empty in {settings_path}")
                            
                except json.JSONDecodeError as e:
                    print(f"⚠️ Invalid JSON in {settings_path}: {e}")
                except Exception as e:
                    print(f"⚠️ Error reading {settings_path}: {e}")
            else:
                print(f"❌ Not found: {settings_path}")
                
    except Exception as e:
        print(f"⚠️ Error searching for VS Code settings: {e}")
    
    abs_default_dir = os.path.abspath(default_dir)
    print(f"📁 Using default report directory: {abs_default_dir}")
    return abs_default_dir

class Handler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        self.report_dir = get_report_directory()
        super().__init__(*args, **kwargs)
    
    def do_GET(self):
        if self.path == '/api/health':
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(b'{"status": "ok"}')
        elif self.path.startswith('/api/files'):
            # Look for HTML files in configured report directory
            files = []
            
            # Parse query parameters for filtering
            filter_type = None
            if '?' in self.path:
                query_part = self.path.split('?')[1]
                params = dict(param.split('=') for param in query_part.split('&') if '=' in param)
                filter_type = params.get('type')
            
            start_time = time.time()
            print(f"🔍 Processing request: {self.path}")
            print(f"📂 Looking in directory: {self.report_dir}")
            
            if os.path.exists(self.report_dir):
                # Cache file list to avoid repeated glob operations
                if not hasattr(self, '_cached_files'):
                    html_files = glob.glob(os.path.join(self.report_dir, '*.html'))
                    self._cached_files = [os.path.basename(f) for f in html_files if os.path.basename(f) != 'index.html']
                
                all_files = self._cached_files
                
                # Filter based on type parameter
                if filter_type == 'misra':
                    files = [f for f in all_files if '-misra-' in f]
                elif filter_type == 'lgedv':
                    files = [f for f in all_files if '-lgedv-' in f]
                elif filter_type == 'cert':
                    files = [f for f in all_files if '-cert-' in f]
                elif filter_type == 'rapidscan':
                    files = [f for f in all_files if '-rapidscan-' in f]
                elif filter_type == 'critical':
                    files = [f for f in all_files if '-critical-' in f]
                else:
                    files = all_files
            else:
                print(f"⚠️ Report directory not found: {self.report_dir}")
            
            processing_time = time.time() - start_time
            print(f"⚡ Processed in {processing_time:.3f}s - Filter: {filter_type}, Found: {len(files)} files")
            
            response = json.dumps({'files': files, 'count': len(files), 'filter': filter_type})
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')  
            self.end_headers()
            self.wfile.write(response.encode())
        elif self.path.endswith('.html') and self.path != '/' and self.path != '/index.html':
            # Serve HTML files from configured report directory
            filename = self.path.lstrip('/')
            file_path = os.path.join(self.report_dir, filename)
            if os.path.exists(file_path):
                with open(file_path, 'rb') as f:
                    content = f.read()
                self.send_response(200)
                self.send_header('Content-Type', 'text/html')
                self.send_header('Content-Length', str(len(content)))
                self.end_headers()
                self.wfile.write(content)
            else:
                self.send_error(404, f"File not found: {filename}")
        else:
            super().do_GET()

def main():
    # Stay in current directory (web_srv) where index.html is located
    print(f"📁 Serving from: {os.getcwd()}")
    
    # Write PID
    with open('server.pid', 'w') as f:
        f.write(str(os.getpid()))
    
    print("Server starting on port 8888...")
    
    # Use TCPServer with SO_REUSEADDR to avoid "Address already in use"
    class ReuseTCPServer(socketserver.TCPServer):
        allow_reuse_address = True
    
    with ReuseTCPServer(("", 8888), Handler) as httpd:
        httpd.serve_forever()

if __name__ == "__main__":
    main()
