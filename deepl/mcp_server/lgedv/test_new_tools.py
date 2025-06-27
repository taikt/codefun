#!/usr/bin/env python3
"""
Test script for new MCP tools
"""
import asyncio
import sys
import os

# Add the server directory to path
sys.path.insert(0, '/home/worker/src/codefun/deepl/mcp_server/lgedv')

from server import fetch_certcpp_rule, fetch_critical_rule, fetch_rapidScan_rule
from server import CERTCPP_RULE_URL, CRITICAL_RULE_URL, RAPIDSCAN_RULE_URL

async def test_tools():
    print("🧪 Testing new MCP tools...")
    
    # Test CERT C++ rule
    print("\n1️⃣ Testing fetch_certcpp_rule...")
    try:
        result = await fetch_certcpp_rule(CERTCPP_RULE_URL)
        content = result[0].text[:200] + "..." if len(result[0].text) > 200 else result[0].text
        print(f"✅ CERT C++ rule loaded: {len(result[0].text)} chars")
        print(f"📄 Preview: {content}")
    except Exception as e:
        print(f"❌ CERT C++ rule failed: {e}")
    
    # Test Critical rule
    print("\n2️⃣ Testing fetch_critical_rule...")
    try:
        result = await fetch_critical_rule(CRITICAL_RULE_URL)
        content = result[0].text[:200] + "..." if len(result[0].text) > 200 else result[0].text
        print(f"✅ Critical rule loaded: {len(result[0].text)} chars")
        print(f"📄 Preview: {content}")
    except Exception as e:
        print(f"❌ Critical rule failed: {e}")
    
    # Test RapidScan rule
    print("\n3️⃣ Testing fetch_rapidScan_rule...")
    try:
        result = await fetch_rapidScan_rule(RAPIDSCAN_RULE_URL)
        content = result[0].text[:200] + "..." if len(result[0].text) > 200 else result[0].text
        print(f"✅ RapidScan rule loaded: {len(result[0].text)} chars")
        print(f"📄 Preview: {content}")
    except Exception as e:
        print(f"❌ RapidScan rule failed: {e}")
    
    print("\n🎉 All tests completed!")

if __name__ == "__main__":
    asyncio.run(test_tools())
