#!/usr/bin/env python3
import aiohttp
import aiohttp.web
import json
import os
import glob
import sys

async def get_report_directory():
    # Always use defect_report directory relative to this script if no user config
    script_dir = os.path.dirname(os.path.abspath(__file__))
    default_dir = os.path.join(script_dir, 'defect_report')
    settings_paths = [
        os.path.join(os.getcwd(), '.vscode', 'settings.json'),
        os.path.expanduser('~/.config/Code/User/settings.json'),
        os.path.expanduser('~/.vscode/settings.json'),
    ]
    for settings_path in settings_paths:
        if os.path.exists(settings_path):
            try:
                with open(settings_path, 'r', encoding='utf-8') as f:
                    content = f.read()
                    lines = []
                    for line in content.split('\n'):
                        if '//' in line:
                            comment_pos = line.find('//')
                            in_string = False
                            quote_count = 0
                            for i, char in enumerate(line[:comment_pos]):
                                if char == '"' and (i == 0 or line[i-1] != '\\'):
                                    quote_count += 1
                            if quote_count % 2 == 0:
                                line = line[:comment_pos]
                        lines.append(line)
                    clean_content = '\n'.join(lines)
                    settings = json.loads(clean_content)
                    report_dir = settings.get('lgedvCodeGuard.reportDirectory')
                    if report_dir and report_dir.strip():
                        # If path contains ${workspaceFolder}, replace with cwd (workspace)
                        if '${workspaceFolder}' in report_dir:
                            workspace_dir = os.getcwd()
                            report_dir = report_dir.replace('${workspaceFolder}', workspace_dir)
                        abs_report_dir = os.path.abspath(os.path.expanduser(report_dir))
                        if os.path.isdir(abs_report_dir):
                            print(f"[INFO] Using report directory from user settings: {abs_report_dir}")
                            return abs_report_dir
                        else:
                            print(f"[WARN] User settings reportDirectory does not exist: {abs_report_dir}")
            except Exception as e:
                print(f"[WARN] Failed to parse settings: {settings_path}: {e}")
    # If not found or not exist, use defect_report in workspace if possible
    workspace_dir = os.getcwd()
    ws_defect_dir = os.path.join(workspace_dir, 'defect_report')
    if os.path.isdir(workspace_dir):
        print(f"[INFO] Using workspace defect_report directory: {ws_defect_dir}")
        return ws_defect_dir
    print(f"[INFO] Using default report directory: {default_dir}")
    return os.path.abspath(default_dir)

async def health(request):
    return aiohttp.web.json_response({'status': 'ok'})

async def files(request):
    report_dir = request.app['report_dir']
    filter_type = request.query.get('type')
    files = []
    if os.path.exists(report_dir):
        html_files = glob.glob(os.path.join(report_dir, '*.html'))
        all_files = [os.path.basename(f) for f in html_files if os.path.basename(f) != 'index.html']
        if filter_type == 'misra':
            files = [f for f in all_files if '-misra-' in f]
        elif filter_type == 'lgedv':
            files = [f for f in all_files if '-lgedv-' in f]
        elif filter_type == 'cert':
            files = [f for f in all_files if '-cert-' in f]
        elif filter_type == 'custom':
            files = [f for f in all_files if f.endswith('custom-report.html')]
        elif filter_type == 'static':
            files = [f for f in all_files if f.endswith('.static-report.html')]
        else:
            files = all_files
    return aiohttp.web.json_response({'files': files, 'count': len(files), 'filter': filter_type})

async def serve_html(request):
    report_dir = request.app['report_dir']
    filename = request.match_info['filename']
    file_path = os.path.join(report_dir, filename)
    print(f"[DEBUG] serve_html: file_path={file_path}")
    if os.path.exists(file_path):
        print(f"[DEBUG] serve_html: file found, serving {file_path}")
        return aiohttp.web.FileResponse(file_path, headers={'Content-Type': 'text/html'})
    else:
        print(f"[DEBUG] serve_html: file not found: {file_path}")
        return aiohttp.web.Response(status=404, text=f'File not found: {filename}')

async def index(request):
    # Serve index.html from the same directory as this script
    script_dir = os.path.dirname(os.path.abspath(__file__))
    index_path = os.path.join(script_dir, 'index.html')
    if os.path.exists(index_path):
        return aiohttp.web.FileResponse(index_path, headers={'Content-Type': 'text/html'})
    return aiohttp.web.Response(status=404, text='index.html not found')

async def create_app():
    app = aiohttp.web.Application()
    report_dir = await get_report_directory()
    # Ensure the report directory exists for new users
    os.makedirs(report_dir, exist_ok=True)
    print(f"[INFO] Web server will serve HTML reports from: {report_dir}")
    sys.stdout.flush()
    app['report_dir'] = report_dir
    app.router.add_get('/', index)
    app.router.add_get('/api/health', health)
    app.router.add_get('/api/files', files)
    app.router.add_get('/{filename:.+\\.html}', serve_html)
    return app

if __name__ == '__main__':
    import asyncio
    app = asyncio.run(create_app())
    aiohttp.web.run_app(app, port=8888)
