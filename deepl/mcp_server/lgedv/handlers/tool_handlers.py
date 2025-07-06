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
from lgedv.modules.config import setup_logging

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
        """Handle detect_races tool"""
        dir_path = arguments.get("dir_path")
        result = analyze_race_conditions_in_codebase(dir_path)
        logger.info(f"detect_races completed for dir: {dir_path}")
        
        # Chuyển đổi kết quả thành định dạng trả về
        files_with_races = result.get("shared_resources", {})
        race_results = []
        for resource_name, accesses in files_with_races.items():
            race_results.append(
                types.TextContent(
                    type="text", 
                    text=f"Resource: {resource_name}, Accesses: {accesses}"
                )
            )
        return race_results
    
    async def _handle_ai_memory_analysis(self, arguments: dict) -> List[types.TextContent]:
        """Handle AI-powered memory leak analysis with rich metadata and code context"""
        dir_path = arguments.get("dir_path")
        
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
