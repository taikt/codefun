#!/usr/bin/env python3
"""
Test script for server_simple.py to verify VS Code settings integration
"""
import os
import json
import tempfile
from server_simple import get_report_directory

def test_get_report_directory():
    print("🧪 Testing get_report_directory() function...")
    
    # Test 1: No settings file (should use default)
    print("\n1️⃣ Testing with no settings file:")
    result = get_report_directory()
    print(f"Result: {result}")
    
    # Test 2: Create a temporary settings file
    print("\n2️⃣ Testing with temporary settings file:")
    temp_dir = tempfile.mkdtemp()
    vscode_dir = os.path.join(temp_dir, '.vscode')
    os.makedirs(vscode_dir, exist_ok=True)
    
    settings_file = os.path.join(vscode_dir, 'settings.json')
    test_report_dir = '/tmp/test_reports'
    
    settings_content = {
        "lgedvCodeGuard.reportDirectory": test_report_dir,
        "other.setting": "value"
    }
    
    with open(settings_file, 'w') as f:
        json.dump(settings_content, f, indent=2)
    
    # Change to temp directory to test
    original_cwd = os.getcwd()
    try:
        os.chdir(temp_dir)
        result = get_report_directory()
        print(f"Result: {result}")
        print(f"Expected: {os.path.abspath(test_report_dir)}")
        
        if os.path.abspath(result) == os.path.abspath(test_report_dir):
            print("✅ PASS: Correctly read from settings")
        else:
            print("❌ FAIL: Did not read from settings correctly")
    finally:
        os.chdir(original_cwd)
    
    # Test 3: Test with comments in JSON
    print("\n3️⃣ Testing with JSON comments:")
    settings_content_with_comments = '''{
    // This is a comment
    "lgedvCodeGuard.reportDirectory": "/tmp/test_reports_comment",
    "other.setting": "value" // Another comment
}'''
    
    with open(settings_file, 'w') as f:
        f.write(settings_content_with_comments)
    
    try:
        os.chdir(temp_dir)
        result = get_report_directory()
        print(f"Result: {result}")
        
        if '/tmp/test_reports_comment' in result:
            print("✅ PASS: Correctly handled JSON with comments")
        else:
            print("❌ FAIL: Could not handle JSON with comments")
    finally:
        os.chdir(original_cwd)
    
    # Cleanup
    import shutil
    shutil.rmtree(temp_dir)
    
    print("\n🏁 Test completed!")

if __name__ == "__main__":
    test_get_report_directory()
