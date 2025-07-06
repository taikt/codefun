"""
Tool handlers for MCP server operations
Xử lý các MCP tools (fetch rules, analyze files, etc.)
"""
import os
from typing import List, Union, Dict
from mcp import types
from lgedv.modules.rule_fetcher import (
    fetch_misra_rule, fetch_lgedv_rule, fetch_certcpp_rule,
    fetch_critical_rule, fetch_custom_rule
)
from lgedv.modules.file_utils import list_cpp_files, get_cpp_files_content
from lgedv.analyzers.race_analyzer import analyze_race_conditions_in_codebase
from lgedv.analyzers.memory_analyzer import MemoryAnalyzer, analyze_leaks
from lgedv.analyzers.resource_analyzer import ResourceAnalyzer
from lgedv.modules.config import setup_logging, get_cpp_dir

logger = setup_logging()

class ToolHandler:
    """Handler cho các MCP tools"""
    
    async def handle_tool_call(self, name: str, arguments: dict) -> List[Union[
        types.TextContent, types.ImageContent, types.AudioContent, types.EmbeddedResource
    ]]:
        """
        Route và xử lý tool calls
        
        Args:
            name: Tên tool
            arguments: Arguments cho tool
            
        Returns:
            List content response
        """
        logger.info(f"Tool called: {name} with arguments: {arguments}")
        
        try:
            # Rule fetching tools
            if name == "fetch_misra_rule":
                return await self._handle_fetch_misra_rule(arguments)
            elif name == "fetch_lgedv_rule":
                return await self._handle_fetch_lgedv_rule(arguments)
            elif name == "fetch_certcpp_rule":
                return await self._handle_fetch_certcpp_rule(arguments)
            elif name == "fetch_critical_rule":
                return await self._handle_fetch_critical_rule(arguments)
            elif name == "fetch_custom_rule":
                return await self._handle_fetch_custom_rule(arguments)
            
            # File operations
            elif name == "list_cpp_files":
                return await self._handle_list_cpp_files(arguments)
            elif name == "get_cpp_files_content":
                return await self._handle_get_cpp_files_content(arguments)
            
            # Analysis tools
            elif name == "detect_races":
                return await self._handle_detect_races(arguments)
            elif name == "analyze_leaks":
                return await self._handle_ai_memory_analysis(arguments)
            elif name == "analyze_resources":
                return await self._handle_analyze_resources(arguments)
            
            else:
                raise ValueError(f"Unknown tool: {name}")
                
        except Exception as e:
            logger.exception(f"Error in tool handler for {name}: {e}")
            raise
    
    async def _handle_fetch_misra_rule(self, arguments: dict) -> List[types.TextContent]:
        """Handle fetch_misra_rule tool"""
        url = arguments.get("url")
        result = await fetch_misra_rule(url)
        logger.info(f"fetch_misra_rule completed for url: {url}")
        return result
    
    async def _handle_fetch_lgedv_rule(self, arguments: dict) -> List[types.TextContent]:
        """Handle fetch_lgedv_rule tool"""
        url = arguments.get("url")
        result = await fetch_lgedv_rule(url)
        logger.info(f"fetch_lgedv_rule completed for url: {url}")
        return result
    
    async def _handle_fetch_certcpp_rule(self, arguments: dict) -> List[types.TextContent]:
        """Handle fetch_certcpp_rule tool"""
        url = arguments.get("url")
        result = await fetch_certcpp_rule(url)
        logger.info(f"fetch_certcpp_rule completed for url: {url}")
        return result
    
    async def _handle_fetch_critical_rule(self, arguments: dict) -> List[types.TextContent]:
        """Handle fetch_critical_rule tool"""
        url = arguments.get("url")
        result = await fetch_critical_rule(url)
        logger.info(f"fetch_critical_rule completed for url: {url}")
        return result
    
    async def _handle_fetch_custom_rule(self, arguments: dict) -> List[types.TextContent]:
        """Handle fetch_custom_rule tool"""
        url = arguments.get("url")
        result = await fetch_custom_rule(url)
        logger.info(f"fetch_custom_rule completed for url: {url}")
        return result
    
    async def _handle_list_cpp_files(self, arguments: dict) -> List[types.TextContent]:
        """Handle list_cpp_files tool"""
        dir_path = arguments.get("dir_path")
        files = list_cpp_files(dir_path)
        logger.info(f"list_cpp_files found {len(files)} files")
        return [types.TextContent(type="text", text="\n".join(files))]
    
    async def _handle_get_cpp_files_content(self, arguments: dict) -> List[types.TextContent]:
        """Handle get_cpp_files_content tool"""
        dir_path = arguments.get("dir_path")
        content = get_cpp_files_content(dir_path)
        logger.info(f"get_cpp_files_content completed for dir: {dir_path}")
        return [types.TextContent(type="text", text=content)]
    
    async def _handle_detect_races(self, arguments: dict) -> List[types.TextContent]:
        """Handle detect_races tool - sử dụng CPP_DIR từ config"""
        # Sử dụng CPP_DIR trực tiếp, không nhận tham số dir_path nữa
        dir_path = get_cpp_dir()
        logger.info(f"[detect_races] Using CPP_DIR: {dir_path}")
        
        result = analyze_race_conditions_in_codebase(dir_path)
        logger.info(f"detect_races completed for dir: {dir_path}")
        
        # Create rich metadata prompt for AI analysis similar to memory analyzer
        metadata_section = self._create_race_analysis_metadata(result, dir_path)
        code_context_section = self._create_race_code_context_section(result, dir_path)
        analysis_prompt = self._create_race_analysis_prompt_section(result)
        
        # Combine all sections into comprehensive prompt
        full_prompt = f"""# 🔍 Race Condition Analysis Request

{metadata_section}

{code_context_section}

{analysis_prompt}

## 🔧 Please Provide:

1. **Detailed Analysis**: Review each potential race condition and assess its validity
2. **Risk Assessment**: Categorize findings by severity and likelihood  
3. **Synchronization Strategy**: Recommend appropriate locking mechanisms
4. **Code Examples**: Show before/after code with proper synchronization

Focus on actionable recommendations that can be immediately implemented.
"""
        
        return [types.TextContent(type="text", text=full_prompt)]
    
    async def _handle_ai_memory_analysis(self, arguments: dict) -> List[types.TextContent]:
        """Handle AI-powered memory leak analysis - sử dụng CPP_DIR từ config"""
        # Sử dụng CPP_DIR trực tiếp, không nhận tham số dir_path nữa
        dir_path = get_cpp_dir()
        logger.info(f"[analyze_leaks] Using CPP_DIR: {dir_path}")
        
        # Use the new MemoryAnalyzer
        analyzer = MemoryAnalyzer()
        result = analyzer.analyze_codebase(dir_path)
        logger.info(f"AI memory leak analysis v2 completed for dir: {dir_path}")
        
        # Create rich metadata prompt for AI analysis
        metadata_section = self._create_memory_analysis_metadata(result, dir_path)
        code_context_section = self._create_code_context_section(result, dir_path)
        analysis_prompt = self._create_analysis_prompt_section(result)
        
        # Combine all sections into comprehensive prompt
        full_prompt = f"""# 🔍 Memory Leak Analysis Request

{metadata_section}

{code_context_section}

{analysis_prompt}

## 🔧 Please Provide:

1. **Detailed Analysis**: Review each potential memory leak and assess its validity
2. **Risk Assessment**: Categorize findings by severity and likelihood  
3. **Fix Recommendations**: Specific code changes for each issue
4. **Best Practices**: Suggest modern C++ approaches (RAII, smart pointers)
5. **Prevention Strategies**: How to avoid similar issues in the future

## 📋 Expected Output Format:

For each memory leak found:
- **Issue Type**: (e.g., Missing delete, Mismatched allocation)
- **Severity**: Critical/High/Medium/Low
- **Location**: File and line number
- **Current Code**: Show the problematic code
- **Fixed Code**: Show the corrected version
- **Explanation**: Why this is a problem and how the fix works

Focus on actionable recommendations that can be immediately implemented.
"""
        
        return [types.TextContent(type="text", text=full_prompt)]
    
    def _create_memory_analysis_metadata(self, result: dict, dir_path: str) -> str:
        """Create rich metadata section for memory analysis"""
        summary = result.get('summary', {})
        leaks = result.get('detected_leaks', [])
        
        # Calculate detailed statistics
        total_allocations = sum(leak.get('allocations', 0) for leak in leaks)
        total_deallocations = sum(leak.get('deallocations', 0) for leak in leaks)
        ratio = total_deallocations / total_allocations if total_allocations > 0 else 0.0
        
        # Categorize leaks by severity
        critical_leaks = [l for l in leaks if l.get('severity') == 'critical']
        high_leaks = [l for l in leaks if l.get('severity') == 'high']
        medium_leaks = [l for l in leaks if l.get('severity') == 'medium']
        
        # Calculate leak types
        no_dealloc_leaks = [l for l in leaks if l.get('deallocations', 0) == 0]
        partial_dealloc_leaks = [l for l in leaks if l.get('deallocations', 0) > 0 and l.get('allocations', 0) > l.get('deallocations', 0)]
        
        # Get file statistics
        files_analyzed = result.get('files_analyzed', 0)
        memory_flows = result.get('memory_flows', {})
        cross_file_flows = len([f for f in memory_flows.values() if f.get('is_cross_file', False)])
        
        metadata = f"""## 🎯 Key Areas to Focus:
1. **Unmatched Allocations**: `new`/`malloc` without corresponding `delete`/`free`
2. **Mismatched Operations**: `new[]` with `delete`, `malloc` with `delete`, etc.
3. **Exception Safety**: Memory leaks in exception paths
4. **Class Destructors**: Missing deallocations in destructors
5. **Smart Pointer Usage**: Areas that could benefit from RAII

## 📊 Comprehensive Analysis Results:

### 🗂️ Project Overview:
- **Directory Analyzed**: `{dir_path}`
- **Total Files Processed**: {files_analyzed}
- **Total Variables Tracked**: {len(memory_flows)}
- **Analysis Scope**: Full codebase with dynamic grouping

### 📈 Memory Operations Summary:
- **Total Allocations Found**: {total_allocations}
- **Total Deallocations Found**: {total_deallocations}
- **Allocation/Deallocation Ratio**: {ratio:.2f} {'✅' if ratio >= 0.9 else '⚠️' if ratio >= 0.5 else '🔴'}
- **Memory Operations Balance**: {'Good' if ratio >= 0.9 else 'Poor' if ratio < 0.5 else 'Fair'}

### 🚨 Leak Severity Distribution:
- **Critical**: {len(critical_leaks)} leaks (Immediate action required)
- **High**: {len(high_leaks)} leaks (Review soon)  
- **Medium**: {len(medium_leaks)} leaks (Monitor and fix)
- **Total Issues**: {len(leaks)} potential memory leaks

### 🔍 Leak Pattern Analysis:
- **Definite Leaks** (No cleanup): {len(no_dealloc_leaks)} cases 🔴
- **Partial Leaks** (Incomplete cleanup): {len(partial_dealloc_leaks)} cases 🟡
- **Cross-File Memory Flows**: {cross_file_flows} complex cases
- **Single-File Leaks**: {len(leaks) - cross_file_flows} localized cases

### 🎯 Risk Assessment:
- **Immediate Risk**: {'HIGH' if len(no_dealloc_leaks) > 5 else 'MEDIUM' if len(no_dealloc_leaks) > 0 else 'LOW'}
- **Code Quality**: {'Poor' if ratio < 0.3 else 'Fair' if ratio < 0.7 else 'Good'}
- **Maintenance Burden**: {'High' if len(leaks) > 10 else 'Medium' if len(leaks) > 5 else 'Low'}"""
        
        return metadata
    
    def _create_code_context_section(self, result: dict, dir_path: str) -> str:
        """Create OPTIMIZED code context section with smart token management and file prioritization"""
        code_section = "## 📄 Relevant Code Context:\n\n"
        
        # Get files with memory leaks and prioritize them
        files_with_leaks = self._prioritize_files_by_leak_severity(result.get('detected_leaks', []))
        
        # TOKEN OPTIMIZATION: Aggressive limits for better efficiency
        max_files = 3  # Max files to include (reduced from 5)
        max_file_size = 1000  # Max characters per file (reduced from 1500)
        
        processed_files = 0
        
        for file_path, leak_info in files_with_leaks[:max_files]:
            if processed_files >= max_files:
                code_section += f"### ⚠️ Additional Files ({len(files_with_leaks) - max_files} files truncated for token optimization)\n\n"
                break
                
            try:
                # Read file content with smart truncation
                with open(file_path, 'r', encoding='utf-8') as f:
                    content = f.read()
                
                # SMART TRUNCATION: Limit file size
                if len(content) > max_file_size:
                    content = content[:max_file_size] + "\n\n// ... [TRUNCATED for token optimization] ..."
                
                filename = file_path.split('/')[-1]
                code_section += f"### 📁 {filename}\n"
                code_section += f"**Path**: `{file_path}`\n"
                code_section += f"**Lines with Memory Operations**: {leak_info['leak_lines']}\n\n"
                
                # Add leak markers to content
                marked_content = self._add_leak_markers_to_content(content, leak_info['leaks'])
                code_section += f"```cpp\n{marked_content}\n```\n\n"
                
                processed_files += 1
                
            except Exception as e:
                filename = file_path.split('/')[-1] if file_path else "unknown"
                code_section += f"### 📁 {filename}\n"
                code_section += f"```\n// Error reading file: {e}\n```\n\n"
        
        return code_section
    
    def _get_leak_lines_for_file(self, file_path: str, leaks: list) -> dict:
        """Get all lines with memory operations for a specific file"""
        leak_lines = {}
        
        for leak in leaks:
            # Check allocation details
            for alloc in leak.get('allocation_details', []):
                if alloc.get('file') == file_path:
                    line_num = alloc.get('line')
                    if line_num:
                        leak_lines[line_num] = {
                            'type': 'allocation',
                            'details': alloc.get('details', ''),
                            'variable': leak.get('variable', ''),
                            'leak_type': leak.get('type', '')
                        }
            
            # Check deallocation details
            for dealloc in leak.get('deallocation_details', []):
                if dealloc.get('file') == file_path:
                    line_num = dealloc.get('line')
                    if line_num:
                        leak_lines[line_num] = {
                            'type': 'deallocation',
                            'details': dealloc.get('details', ''),
                            'variable': leak.get('variable', ''),
                            'leak_type': leak.get('type', '')
                        }
        
        return leak_lines
    def _create_analysis_prompt_section(self, result: dict) -> str:
        """Create detailed analysis prompt section with complete leak information"""
        leaks = result.get('detected_leaks', [])
        
        prompt_section = "## 🔍 Detailed Memory Leak Findings:\n\n"
        prompt_section += f"**Total Detected Leaks**: {len(leaks)} issues requiring attention\n\n"
        
        # TOKEN OPTIMIZATION: Limit detailed findings to top 5 critical leaks
        max_detailed_leaks = 5
        leaks_to_show = sorted(leaks, key=lambda x: (
            x.get('severity') == 'critical',
            x.get('severity') == 'high', 
            x.get('deallocations', 0) == 0,  # Prioritize definite leaks
            x.get('allocations', 0)
        ), reverse=True)[:max_detailed_leaks]
        
        if len(leaks) > max_detailed_leaks:
            prompt_section += f"**Showing Top {max_detailed_leaks} Priority Leaks** (of {len(leaks)} total)\n\n"
        
        for i, leak in enumerate(leaks_to_show, 1):
            severity_emoji = {"critical": "🔴", "high": "🟠", "medium": "🟡"}.get(leak.get('severity', 'medium'), "🟡")
            
            prompt_section += f"### {severity_emoji} Leak #{i}: {leak.get('type', 'Unknown')} - {leak.get('severity', 'Medium')} Priority\n"
            prompt_section += f"- **Variable**: `{leak.get('variable', 'Unknown')}`\n"
            prompt_section += f"- **Files Involved**: {len(leak.get('files_involved', []))} file(s)\n"
            prompt_section += f"- **Allocations**: {leak.get('allocations', 0)}\n"
            prompt_section += f"- **Deallocations**: {leak.get('deallocations', 0)}\n"
            prompt_section += f"- **Leak Risk**: {'HIGH - No cleanup found!' if leak.get('deallocations', 0) == 0 else 'MEDIUM - Partial cleanup'}\n"
            
            # Show concise allocation information
            if leak.get('allocation_details'):
                prompt_section += f"- **🔴 Allocation Points** ({len(leak['allocation_details'])} locations):\n"
                for j, alloc in enumerate(leak['allocation_details'][:2], 1):  # Limit to first 2
                    file_short = alloc.get('file', 'Unknown').split('/')[-1]
                    prompt_section += f"  {j}. `{file_short}:{alloc.get('line', '?')}` - **{alloc.get('details', 'Unknown')}**\n"
                if len(leak['allocation_details']) > 2:
                    prompt_section += f"  ... and {len(leak['allocation_details']) - 2} more\n"
            else:
                prompt_section += f"- **🔴 Allocation Points**: No allocation details found\n"
            
            # Show concise deallocation information  
            if leak.get('deallocation_details'):
                prompt_section += f"- **🟢 Deallocation Points** ({len(leak['deallocation_details'])} locations):\n"
                for j, dealloc in enumerate(leak['deallocation_details'][:2], 1):  # Limit to first 2
                    file_short = dealloc.get('file', 'Unknown').split('/')[-1]
                    prompt_section += f"  {j}. `{file_short}:{dealloc.get('line', '?')}` - **{dealloc.get('details', 'Unknown')}**\n"
                if len(leak['deallocation_details']) > 2:
                    prompt_section += f"  ... and {len(leak['deallocation_details']) - 2} more\n"
            else:
                prompt_section += f"- **⚠️ NO DEALLOCATIONS FOUND** - This is a definite memory leak!\n"
            
            # Add file context for each leak
            if leak.get('files_involved'):
                prompt_section += f"- **Files to Review**: {', '.join([f.split('/')[-1] for f in leak.get('files_involved', [])])}\n"
            
            prompt_section += "\n" + "─" * 60 + "\n\n"
        
        # Add summary recommendations
        prompt_section += "## 🎯 Priority Analysis Guidelines:\n\n"
        prompt_section += "1. **Focus on leaks with 0 deallocations first** - These are guaranteed leaks\n"
        prompt_section += "2. **Check allocation/deallocation type mismatches** - `new`/`delete[]`, `malloc`/`delete`, etc.\n"
        prompt_section += "3. **Look for exception safety issues** - Leaks when exceptions occur\n"
        prompt_section += "4. **Verify RAII compliance** - Use smart pointers where possible\n"
        prompt_section += "5. **Check constructor/destructor pairs** - Ensure proper cleanup in class destructors\n\n"
        
        return prompt_section
    
    def _prioritize_files_by_leak_severity(self, detected_leaks: list) -> list:
        """Prioritize files by number of critical/high severity leaks"""
        file_leak_map = {}
        
        for leak in detected_leaks:
            files_involved = leak.get('files_involved', [])
            severity = leak.get('severity', 'medium')
            
            for file_path in files_involved:
                if file_path not in file_leak_map:
                    file_leak_map[file_path] = {
                        'critical_count': 0,
                        'high_count': 0,
                        'medium_count': 0,
                        'total_leaks': 0,
                        'leak_lines': [],
                        'leaks': []
                    }
                
                # Count by severity
                if severity == 'critical':
                    file_leak_map[file_path]['critical_count'] += 1
                elif severity == 'high':
                    file_leak_map[file_path]['high_count'] += 1
                else:
                    file_leak_map[file_path]['medium_count'] += 1
                
                file_leak_map[file_path]['total_leaks'] += 1
                file_leak_map[file_path]['leaks'].append(leak)
                
                # Collect leak lines
                for alloc in leak.get('allocation_details', []):
                    if alloc.get('file') == file_path and alloc.get('line'):
                        file_leak_map[file_path]['leak_lines'].append(alloc.get('line'))
        
        # Sort by priority: critical count desc, then high count desc, then total desc
        prioritized_files = sorted(
            file_leak_map.items(),
            key=lambda x: (x[1]['critical_count'], x[1]['high_count'], x[1]['total_leaks']),
            reverse=True
        )
        
        return prioritized_files
    
    def _add_leak_markers_to_content(self, content: str, leaks: list) -> str:
        """Add visual markers to code content to highlight memory leaks"""
        lines = content.split('\n')
        
        # Create a map of line numbers to leak info
        line_markers = {}
        for leak in leaks:
            for alloc in leak.get('allocation_details', []):
                line_num = alloc.get('line')
                if line_num and 1 <= line_num <= len(lines):
                    marker = f"🔴 ALLOCATION: {leak.get('variable', 'unknown')} - {alloc.get('details', '')}"
                    line_markers[line_num] = marker
            
            for dealloc in leak.get('deallocation_details', []):
                line_num = dealloc.get('line')
                if line_num and 1 <= line_num <= len(lines):
                    marker = f"🟢 DEALLOCATION: {leak.get('variable', 'unknown')} - {dealloc.get('details', '')}"
                    line_markers[line_num] = marker
        
        # Add markers to lines
        marked_lines = []
        for i, line in enumerate(lines, 1):
            if i in line_markers:
                marked_lines.append(f"{line}  // {line_markers[i]}")
            else:
                marked_lines.append(line)
        
        return '\n'.join(marked_lines)
    
    async def _handle_analyze_resources(self, arguments: dict) -> List[types.TextContent]:
        """
        Handle resource leak analysis tool
        
        Returns:
            List[types.TextContent]: Analysis results
        """
        cpp_dir = get_cpp_dir()
        logger.info(f"Starting resource leak analysis on directory: {cpp_dir}")
        
        try:
            # Initialize resource analyzer
            analyzer = ResourceAnalyzer()
            
            # Analyze directory for resource leaks - analyzer will use CPP_DIR internally
            leaks = analyzer.analyze_directory()
            
            # Format results
            if not leaks:
                result_text = f"✅ **Resource Leak Analysis Complete**\n\nNo resource leaks detected in {cpp_dir}\n\nAnalyzed resources:\n- File descriptors (open/close)\n- Socket connections (socket/close)\n- Memory mappings (mmap/munmap)\n- Directory handles (opendir/closedir)\n- IPC resources (shared memory, semaphores)\n- Other Linux resources\n\n📊 **Status**: CLEAN"
            else:
                result_text = f"🔍 **Resource Leak Analysis Results**\n\nDirectory: {cpp_dir}\nTotal leaks found: {len(leaks)}\n\n"
                
                # Group leaks by type and severity
                leak_summary = self._summarize_resource_leaks(leaks)
                result_text += f"📋 **Summary by Type:**\n{leak_summary}\n\n"
                
                # Detailed leak information
                result_text += "📝 **Detailed Findings:**\n\n"
                for i, leak in enumerate(leaks, 1):
                    result_text += f"**{i}. {leak['resource_type'].upper().replace('_', ' ')} Leak**\n"
                    result_text += f"   - Variable: {leak['variable']}\n"
                    result_text += f"   - Resource Type: {leak['resource_type']}\n"
                    result_text += f"   - Files Involved: {len(leak['files_involved'])} file(s)\n"
                    result_text += f"   - Severity: {leak['severity']}\n"
                    result_text += f"   - Open Operations: {leak['open_operations']}\n"
                    result_text += f"   - Close Operations: {leak['close_operations']}\n"
                    
                    # Show file details
                    for file_info in leak.get('files_involved', []):
                        file_short = file_info.split('/')[-1] if isinstance(file_info, str) else str(file_info)
                        result_text += f"   - File: {file_short}\n"
                    
                    # Show open details
                    if leak.get('open_details'):
                        result_text += f"   - Open Locations:\n"
                        for detail in leak['open_details'][:3]:  # Limit to first 3
                            file_short = detail['file'].split('/')[-1]
                            result_text += f"     • {file_short}:{detail['line']} ({detail['details']})\n"
                    
                    # Show recommendation
                    if leak.get('recommendation'):
                        result_text += f"   - Fix: {leak['recommendation']}\n"
                    result_text += "\n"
            
            logger.info(f"Resource leak analysis completed. Found {len(leaks)} leaks")
            
            return [types.TextContent(
                type="text",
                text=result_text
            )]
            
        except Exception as e:
            error_msg = f"❌ Resource leak analysis failed: {str(e)}"
            logger.error(error_msg)
            return [types.TextContent(
                type="text", 
                text=error_msg
            )]
    
    def _summarize_resource_leaks(self, leaks: list) -> str:
        """
        Summarize resource leaks by type and severity
        
        Args:
            leaks: List of resource leak findings
            
        Returns:
            str: Formatted summary
        """
        summary = {}
        severity_counts = {'critical': 0, 'high': 0, 'medium': 0, 'low': 0}
        
        for leak in leaks:
            leak_type = leak.get('resource_type', 'unknown')
            severity = leak.get('severity', 'medium')
            
            if leak_type not in summary:
                summary[leak_type] = {'critical': 0, 'high': 0, 'medium': 0, 'low': 0, 'total': 0}
            
            summary[leak_type][severity] += 1
            summary[leak_type]['total'] += 1
            severity_counts[severity] += 1
        
        result = f"Overall: {severity_counts['critical']} critical, {severity_counts['high']} high, {severity_counts['medium']} medium, {severity_counts['low']} low\n\n"
        
        for leak_type, counts in summary.items():
            result += f"• {leak_type.replace('_', ' ').title()}: {counts['total']} total "
            if counts['critical'] > 0:
                result += f"({counts['critical']} critical) "
            if counts['high'] > 0:
                result += f"({counts['high']} high) "
            result += "\n"
        
        return result
    
    async def _handle_ai_resource_analysis(self, arguments: dict) -> List[types.TextContent]:
        """
        Handle AI resource leak analysis - tạo rich prompt cho Copilot analysis
        Simplified version to prevent hanging
        """
        cpp_dir = get_cpp_dir()
        logger.info(f"Starting AI resource leak analysis on directory: {cpp_dir}")
        
        try:
            # Simple file listing and content extraction instead of complex analysis
            from lgedv.modules.file_utils import list_cpp_files, get_cpp_files_content
            
            # Get C++ files
            cpp_files = list_cpp_files(cpp_dir)
            if not cpp_files:
                return [types.TextContent(
                    type="text",
                    text=f"# 🔍 Linux C++ Resource Leak Analysis\n\n❌ No C++ files found in: {cpp_dir}"
                )]
            
            # Get file contents (limited)
            file_contents = ""
            for i, filename in enumerate(cpp_files[:3]):  # Limit to first 3 files
                try:
                    file_path = os.path.join(cpp_dir, filename)
                    with open(file_path, 'r', encoding='utf-8') as f:
                        content = f.read()
                        # Truncate if too long
                        if len(content) > 1000:
                            content = content[:1000] + "\n\n// ... [TRUNCATED FOR BREVITY] ..."
                        file_contents += f"### {i+1}. `{filename}`\n\n```cpp\n{content}\n```\n\n"
                except Exception as e:
                    file_contents += f"### {i+1}. `{filename}` (Error: {e})\n\n"
            
            # Create simple prompt
            prompt_content = f"""# 🔍 Linux C++ Resource Leak Analysis Request

## 📊 **Analysis Metadata**

- **Directory**: `{cpp_dir}`
- **Analysis Type**: Linux C++ Resource Leak Detection
- **Files Found**: {len(cpp_files)}
- **Files Shown**: {min(len(cpp_files), 3)}

## 📝 **Source Code Context**

{file_contents}

## 🎯 **Analysis Guidelines**

Please analyze the above C++ code for Linux resource management issues:

1. **File Descriptors**: Check for unmatched open()/close() calls
2. **Sockets**: Verify socket descriptors are properly closed  
3. **Memory Mappings**: Ensure mmap() has corresponding munmap()
4. **Directory Handles**: Check opendir()/closedir() pairs
5. **IPC Resources**: Verify shared memory, semaphores, message queues cleanup

## 📋 **Expected Output Format**

For each potential resource leak:
- **Resource Type**: File descriptor, socket, mmap, etc.
- **Severity**: Critical/High/Medium/Low based on system impact
- **Location**: File and line number
- **Problem**: What resource is not being cleaned up
- **Fix**: Specific code changes needed
- **RAII Solution**: Modern C++ wrapper suggestions

Focus on Linux-specific resources and provide actionable recommendations.
"""

            logger.info(f"AI resource leak analysis content generated: {len(prompt_content)} chars")
            
            return [types.TextContent(
                type="text",
                text=prompt_content
            )]
            
        except Exception as e:
            error_msg = f"❌ AI resource leak analysis failed: {str(e)}"
            logger.error(error_msg)
            return [types.TextContent(
                type="text", 
                text=error_msg
            )]
    
    def _create_resource_analysis_metadata(self, analysis_result: dict, cpp_dir: str) -> str:
        """Create metadata section for resource analysis"""
        detected_leaks = analysis_result.get("detected_leaks", [])
        summary = analysis_result.get("summary", {})
        
        metadata = f"""## 📊 **Analysis Metadata**

- **Directory**: `{cpp_dir}`
- **Analysis Type**: Linux C++ Resource Leak Detection
- **Files Analyzed**: {summary.get('total_files_analyzed', 0)}
- **Resource Operations Found**: {summary.get('resource_operations_found', 0)}
- **Potential Leaks**: {len(detected_leaks)}
- **Cross-file Flows**: {summary.get('cross_file_flows', 0)}

### 📈 **Severity Breakdown**
"""
        
        severity_counts = {'critical': 0, 'high': 0, 'medium': 0, 'low': 0}
        for leak in detected_leaks:
            severity = leak.get('severity', 'medium')
            severity_counts[severity] += 1
        
        metadata += f"- Critical: {severity_counts['critical']}\n"
        metadata += f"- High: {severity_counts['high']}\n"
        metadata += f"- Medium: {severity_counts['medium']}\n"
        metadata += f"- Low: {severity_counts['low']}\n"
        
        return metadata
    
    def _create_resource_code_context_section(self, analysis_result: dict, cpp_dir: str) -> str:
        """Create code context section with source files"""
        context_section = "## 📝 **Source Code Context**\n\n"
        
        # Get analyzed files
        analyzed_files = set()
        detected_leaks = analysis_result.get("detected_leaks", [])
        
        for leak in detected_leaks:
            files_involved = leak.get('files_involved', [])
            analyzed_files.update(files_involved)
        
        # Limit to first 3 files for token efficiency
        for i, file_path in enumerate(list(analyzed_files)[:3], 1):
            try:
                with open(file_path, 'r', encoding='utf-8') as f:
                    content = f.read()
                    
                # Truncate if too long
                if len(content) > 1500:
                    content = content[:1500] + "\n\n// ... [TRUNCATED FOR BREVITY] ..."
                
                file_name = os.path.basename(file_path)
                context_section += f"### {i}. `{file_name}`\n\n"
                context_section += f"```cpp\n{content}\n```\n\n"
                
            except Exception as e:
                file_name = os.path.basename(file_path)
                context_section += f"### {i}. `{file_name}` (Error reading: {e})\n\n"
        
        if len(analyzed_files) > 3:
            context_section += f"... and {len(analyzed_files) - 3} more files analyzed\n\n"
        
        return context_section
    
    def _create_resource_findings_section(self, analysis_result: dict) -> str:
        """Create findings section with detected resource leaks"""
        detected_leaks = analysis_result.get("detected_leaks", [])
        
        if not detected_leaks:
            return "## ✅ **Analysis Results**: No resource leaks detected\n"
        
        findings_section = f"## 🚨 **Resource Leak Findings** ({len(detected_leaks)} total)\n\n"
        
        # Group by resource type
        leak_groups = {}
        for leak in detected_leaks:
            resource_type = leak.get('resource_type', 'unknown')
            if resource_type not in leak_groups:
                leak_groups[resource_type] = []
            leak_groups[resource_type].append(leak)
        
        for resource_type, type_leaks in leak_groups.items():
            display_type = resource_type.replace('_', ' ').title()
            findings_section += f"### 🔍 **{display_type} Leaks** ({len(type_leaks)} found)\n\n"
            
            # Show first 3 leaks of each type for token efficiency
            for i, leak in enumerate(type_leaks[:3], 1):
                variable = leak.get('variable', 'unknown')
                severity = leak.get('severity', 'medium')
                files_involved = leak.get('files_involved', [])
                file_names = [os.path.basename(f) for f in files_involved]
                
                findings_section += f"{i}. **Variable `{variable}`** - {severity} severity\n"
                findings_section += f"   - Files: {', '.join(file_names)}\n"
                findings_section += f"   - Opens: {leak.get('open_operations', 0)}, Closes: {leak.get('close_operations', 0)}\n"
                
                # Show open locations
                open_details = leak.get('open_details', [])
                if open_details:
                    findings_section += f"   - Open locations:\n"
                    for detail in open_details[:2]:  # Show first 2
                        file_name = os.path.basename(detail.get('file', ''))
                        line = detail.get('line', 'unknown')
                        findings_section += f"     • {file_name}:{line}\n"
                
                if leak.get('recommendation'):
                    findings_section += f"   - Fix: {leak['recommendation']}\n"
                
                findings_section += "\n"
            
            if len(type_leaks) > 3:
                findings_section += f"   ... and {len(type_leaks) - 3} more {display_type.lower()} leaks\n\n"
        
        return findings_section

    def _create_race_analysis_metadata(self, race_result: dict, dir_path: str) -> str:
        """Create metadata section for race condition analysis"""
        metadata_section = "## 📊 Analysis Metadata\n\n"
        metadata_section += f"**Target Directory:** `{dir_path}`\n"
        
        summary = race_result.get('summary', {})
        detected_races = race_result.get('potential_race_conditions', [])
        
        metadata_section += f"**Files Analyzed:** {summary.get('total_files_analyzed', 0)} C++ files\n"
        metadata_section += f"**Race Conditions Found:** {len(detected_races)}\n"
        
        if detected_races:
            # Count by severity
            severity_counts = {'critical': 0, 'high': 0, 'medium': 0, 'low': 0}
            for race in detected_races:
                severity = race.get('severity', 'medium').lower()
                if severity in severity_counts:
                    severity_counts[severity] += 1
            
            metadata_section += f"**Severity Breakdown:** {severity_counts['critical']} Critical, "
            metadata_section += f"{severity_counts['high']} High, {severity_counts['medium']} Medium, "
            metadata_section += f"{severity_counts['low']} Low\n"
        
        metadata_section += "\n"
        return metadata_section
    
    def _create_race_code_context_section(self, race_result: dict, dir_path: str) -> str:
        """Create rich code context section with actual source files"""
        context_section = "## 📁 Source Code Context\n\n"
        
        summary = race_result.get('summary', {})
        detected_races = race_result.get('potential_race_conditions', [])
        thread_usage = race_result.get('thread_usage', {})
        files_analyzed = summary.get('total_files_analyzed', 0)
        
        if files_analyzed == 0:
            context_section += f"❌ No C++ files found in: {dir_path}\n\n"
            return context_section
        
        # Get files with race conditions or thread usage
        priority_files = set()
        for race in detected_races:
            files_involved = race.get('files_involved', [])
            priority_files.update(files_involved)
        
        # Add files with thread usage
        for file_path, threads in thread_usage.items():
            if threads:  # Has thread usage
                priority_files.add(file_path)
        
        # Show actual source code for priority files
        file_count = 0
        max_files = 3  # Limit for token efficiency
        
        for file_path in list(priority_files)[:max_files]:
            file_count += 1
            try:
                with open(file_path, 'r', encoding='utf-8') as f:
                    content = f.read()
                
                # Truncate if too long
                if len(content) > 1200:
                    content = content[:1200] + "\n\n// ... [TRUNCATED FOR BREVITY] ..."
                
                filename = os.path.basename(file_path)
                context_section += f"### {file_count}. 📁 **{filename}**\n"
                context_section += f"**Path**: `{file_path}`\n"
                
                # Show race conditions for this file
                file_races = [r for r in detected_races if file_path in r.get('files_involved', [])]
                if file_races:
                    context_section += f"**Race Conditions**: {len(file_races)} found\n"
                    for race in file_races[:2]:
                        context_section += f"- {race.get('type', 'unknown')}: {race.get('description', 'No description')}\n"
                
                # Show thread usage
                file_threads = thread_usage.get(file_path, [])
                if file_threads:
                    context_section += f"**Thread Usage**: {len(file_threads)} thread-related operations\n"
                
                context_section += f"\n```cpp\n{content}\n```\n\n"
                
            except Exception as e:
                filename = os.path.basename(file_path)
                context_section += f"### {file_count}. 📁 **{filename}** (Error reading: {e})\n\n"
        
        # Show summary if we have more files
        remaining_files = files_analyzed - len(priority_files)
        if remaining_files > 0:
            context_section += f"### 📊 **Additional Files**: {remaining_files} more C++ files analyzed\n\n"
        
        return context_section
    
    def _create_race_analysis_prompt_section(self, race_result: dict) -> str:
        """Create analysis prompt section with race condition details"""
        prompt_section = "## 🎯 Race Condition Analysis Focus\n\n"
        
        detected_races = race_result.get('potential_race_conditions', [])
        
        if not detected_races:
            prompt_section += "✅ **No potential race conditions detected in static analysis.**\n\n"
            prompt_section += "However, please perform a manual review focusing on:\n"
            prompt_section += "1. Shared state access patterns\n"
            prompt_section += "2. Thread synchronization mechanisms\n"
            prompt_section += "3. Atomic operations usage\n"
            prompt_section += "4. Lock-free programming patterns\n\n"
            return prompt_section
        
        # Group races by type for better organization
        race_types = {}
        for race in detected_races:
            race_type = race.get('type', 'unknown')
            if race_type not in race_types:
                race_types[race_type] = []
            race_types[race_type].append(race)
        
        prompt_section += f"🚨 **{len(detected_races)} potential race conditions found:**\n\n"
        
        for race_type, type_races in race_types.items():
            display_type = race_type.replace('_', ' ').title()
            prompt_section += f"### {display_type} ({len(type_races)} found)\n\n"
            
            for i, race in enumerate(type_races[:3], 1):  # Show first 3 of each type
                severity = race.get('severity', 'medium')
                files_involved = race.get('files_involved', [])
                description = race.get('description', 'No description available')
                
                prompt_section += f"{i}. **Severity: {severity.title()}**\n"
                if files_involved:
                    file_list = ', '.join([os.path.basename(f) for f in files_involved])
                    prompt_section += f"   - Files: {file_list}\n"
                prompt_section += f"   - Issue: {description}\n"
                
                if race.get('recommendation'):
                    prompt_section += f"   - Suggested Fix: {race['recommendation']}\n"
                
                prompt_section += "\n"
            
            if len(type_races) > 3:
                prompt_section += f"   ... and {len(type_races) - 3} more {display_type.lower()} issues\n\n"
        
        # Add analysis guidelines
        prompt_section += "## 🎯 Priority Analysis Guidelines:\n\n"
        prompt_section += "1. **Focus on critical/high severity races first** - These can cause data corruption\n"
        prompt_section += "2. **Check shared data access patterns** - Look for unprotected reads/writes\n"
        prompt_section += "3. **Verify synchronization mechanisms** - Mutex, semaphores, atomic operations\n"
        prompt_section += "4. **Look for lock ordering issues** - Potential deadlock scenarios\n"
        prompt_section += "5. **Check thread-local storage usage** - Ensure proper isolation\n\n"
        
        return prompt_section
