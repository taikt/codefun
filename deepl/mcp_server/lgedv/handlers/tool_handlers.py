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
import pprint

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
        dir_path = get_cpp_dir()
        logger.info(f"[detect_races] Using CPP_DIR: {dir_path}")
        result = analyze_race_conditions_in_codebase(dir_path)
        logger.info(f"detect_races completed for dir: {dir_path}")
        # Log markdown report for each file
        markdown_reports = result.get('summary', {}).get('markdown_reports', {})
        for file_path, markdown in markdown_reports.items():
            logger.info("[THREAD ENTRY MARKDOWN] File: %s\n%s", file_path, markdown)
        # Log file summaries (markdown + race info)
        file_summaries = result.get('summary', {}).get('file_summaries', {})
        for file_path, summary in file_summaries.items():
            logger.info("[RACE FILE SUMMARY] File: %s\n%s", file_path, summary)
        # Log detected races for debugging
        # detected_races = result.get('potential_race_conditions', [])
        # logger.info(f"[detect_races] detected_races: {detected_races}")
        # Log thread usage for debugging
        thread_usage = result.get('thread_usage', {})
        logger.info(f"[detect_races] thread_usage: {thread_usage}")
        # Create rich metadata prompt for AI analysis similar to memory analyzer
        metadata_section = self._create_race_analysis_metadata(result, dir_path)
        code_context_section = self._create_race_code_context_section(result, dir_path)
        analysis_prompt = self._create_race_analysis_prompt_section(result)
        # Remove redundant thread entrypoint summary section
        # Combine all sections into comprehensive prompt
        full_prompt = (
            "# 🔍 Race Condition Analysis Request\n\n"
            f"{metadata_section}\n\n"
            f"{code_context_section}\n"
            f"{analysis_prompt}\n\n"
            "## 🔧 Please Provide:\n\n"
            "1. **Detailed Analysis**: Review each potential race condition and assess its validity\n"
            "2. **Risk Assessment**: Categorize findings by severity and likelihood  \n"
            "3. **Synchronization Strategy**: Recommend appropriate locking mechanisms\n"
            "4. **Code Examples**: Show before/after code with proper synchronization\n\n"
            "Focus on actionable recommendations that can be immediately implemented.\n"
        )
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
        """Create per-file code context section for race condition analysis."""
        file_summaries = result.get('summary', {}).get('file_summaries', {})
        markdown_reports = result.get('summary', {}).get('markdown_reports', {})
        section_lines = []
        for idx, (file_path, summary) in enumerate(file_summaries.items(), 1):
            rel_path = os.path.relpath(file_path, dir_path)
            markdown = markdown_reports.get(file_path, "")
            section_lines.append(f"### {idx}. 📁 **{os.path.basename(file_path)}**\n**Path**: `{file_path}`\n{summary}")
            # Thread usage and entrypoints are already included in summary, so do not duplicate
        return "\n\n".join(section_lines)
    
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
    
    def _prioritize_files_by_leak_severity(self, leaks: list) -> list:
        """Prioritize files by number of critical/high severity memory leaks"""
        file_leak_map = {}
        for leak in leaks:
            files_involved = leak.get('files_involved', [])
            severity = leak.get('severity', 'medium')
            for file_path in files_involved:
                if file_path not in file_leak_map:
                    file_leak_map[file_path] = {
                        'critical_count': 0,
                        'high_count': 0,
                        'medium_count': 0,
                        'low_count': 0,
                        'total_leaks': 0,
                        'leaks': []
                    }
                if severity == 'critical':
                    file_leak_map[file_path]['critical_count'] += 1
                elif severity == 'high':
                    file_leak_map[file_path]['high_count'] += 1
                elif severity == 'medium':
                    file_leak_map[file_path]['medium_count'] += 1
                else:
                    file_leak_map[file_path]['low_count'] += 1
                file_leak_map[file_path]['total_leaks'] += 1
                file_leak_map[file_path]['leaks'].append(leak)

        #for file_path, info in file_leak_map.items():
        #    logger.info("File: %s\nInfo:\n%s", file_path, pprint.pformat(info))   
        logger.info("Prioritized file leak map:")     
        for file_path, info in file_leak_map.items():
            leak_vars = [leak.get('variable', 'unknown') for leak in info['leaks'][:3]]
            logger.info(
                "File: %s | Critical: %d | High: %d | Medium: %d | Low: %d | Total: %d | Leak Vars: %s%s",
                file_path,
                info['critical_count'],
                info['high_count'],
                info['medium_count'],
                info['low_count'],
                info['total_leaks'],
                ', '.join(leak_vars),
                f" (+{len(info['leaks'])-3} more)" if len(info['leaks']) > 3 else ""
            )

        prioritized_files = sorted(
            file_leak_map.items(),
            key=lambda x: (x[1]['critical_count'], x[1]['high_count'], x[1]['total_leaks']),
            reverse=True
        )
        return prioritized_files
    
    def _prioritize_files_by_resource_leak_severity(self, leaks: list) -> list:
        """Prioritize files by number and severity of resource leaks (critical/high first)"""
        file_leak_map = {}
        for leak in leaks:
            files_involved = leak.get('files_involved', [])
            severity = leak.get('severity', 'medium')
            for file_path in files_involved:
                if file_path not in file_leak_map:
                    file_leak_map[file_path] = {
                        'critical_count': 0,
                        'high_count': 0,
                        'medium_count': 0,
                        'low_count': 0,
                        'total_leaks': 0,
                        'leaks': []
                    }
                if severity == 'critical':
                    file_leak_map[file_path]['critical_count'] += 1
                elif severity == 'high':
                    file_leak_map[file_path]['high_count'] += 1
                elif severity == 'medium':
                    file_leak_map[file_path]['medium_count'] += 1
                else:
                    file_leak_map[file_path]['low_count'] += 1
                file_leak_map[file_path]['total_leaks'] += 1
                file_leak_map[file_path]['leaks'].append(leak)
        
        logger.info("Prioritized resource file leak map:")
        for file_path, info in file_leak_map.items():
            leak_vars = [leak.get('variable', 'unknown') for leak in info['leaks'][:3]]
            logger.info(
                "File: %s | Critical: %d | High: %d | Medium: %d | Low: %d | Total: %d | Leak Vars: %s%s",
                file_path,
                info['critical_count'],
                info['high_count'],
                info['medium_count'],
                info['low_count'],
                info['total_leaks'],
                ', '.join(leak_vars),
                f" (+{len(info['leaks'])-3} more)" if len(info['leaks']) > 3 else ""
            )

        # sap xep theo thu tu uu tien: critical > high > total_leaks
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
            logger.info(f"Resource leak analysis found {len(leaks)} leaks in directory: {cpp_dir}")

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
        Prioritize files by leak severity for code context (like memory analyzer)
        """
        cpp_dir = get_cpp_dir()
        logger.info(f"Starting AI resource leak analysis on directory: {cpp_dir}")
        try:
            analyzer = ResourceAnalyzer()
            leaks = analyzer.analyze_directory()
            logger.info(f"AI resource leak analysis found {len(leaks)} leaks in directory: {cpp_dir}")
            if not leaks:
                return [types.TextContent(
                    type="text",
                    text=f"# 🔍 Linux C++ Resource Leak Analysis\n\n❌ No resource leaks detected in: {cpp_dir}"
                )]
            # Prioritize files by leak severity
            prioritized_files = self._prioritize_files_by_resource_leak_severity(leaks)
            file_contents = ""
            max_files = 3
            max_file_size = 1200 # 1200 characters per file
            for i, (file_path, info) in enumerate(prioritized_files[:max_files]):
                try:
                    with open(file_path, 'r', encoding='utf-8') as f:
                        content = f.read()
                    if len(content) > max_file_size:
                        content = content[:max_file_size] + "\n\n// ... [TRUNCATED FOR BREVITY] ..."
                    file_name = os.path.basename(file_path)
                    file_contents += f"### {i+1}. `{file_name}`\n\n```cpp\n{content}\n```\n\n"
                except Exception as e:
                    file_name = os.path.basename(file_path)
                    file_contents += f"### {i+1}. `{file_name}` (Error: {e})\n\n"
            prompt_content = f"""# 🔍 Linux C++ Resource Leak Analysis Request\n\n## 📊 **Analysis Metadata**\n\n- **Directory**: `{cpp_dir}`\n- **Analysis Type**: Linux C++ Resource Leak Detection\n- **Files Found**: {len(prioritized_files)}\n- **Files Shown**: {min(len(prioritized_files), max_files)}\n\n## 📝 **Source Code Context**\n\n{file_contents}\n## 🎯 **Analysis Guidelines**\n\nPlease analyze the above C++ code for Linux resource management issues:\n\n1. **File Descriptors**: Check for unmatched open()/close() calls\n2. **Sockets**: Verify socket descriptors are properly closed  \n3. **Memory Mappings**: Ensure mmap() has corresponding munmap()\n4. **Directory Handles**: Check opendir()/closedir() pairs\n5. **IPC Resources**: Verify shared memory, semaphores, message queues cleanup\n\n## 📋 **Expected Output Format**\n\nFor each potential resource leak:\n- **Resource Type**: File descriptor, socket, mmap, etc.\n- **Severity**: Critical/High/Medium/Low based on system impact\n- **Location**: File and line number\n- **Problem**: What resource is not being cleaned up\n- **Fix**: Specific code changes needed\n- **RAII Solution**: Modern C++ wrapper suggestions\n\nFocus on Linux-specific resources and provide actionable recommendations.\n"""
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
        #detected_races = race_result.get('potential_race_conditions', [])
        
        metadata_section += f"**Files Analyzed:** {summary.get('total_files_analyzed', 0)} C++ files\n"
        #metadata_section += f"**Race Conditions Found:** {len(detected_races)}\n"
        
        # if detected_races:
        #     # Count by severity
        #     severity_counts = {'critical': 0, 'high': 0, 'medium': 0, 'low': 0}
        #     for race in detected_races:
        #         severity = race.get('severity', 'medium').lower()
        #         if severity in severity_counts:
        #             severity_counts[severity] += 1
            
        #     metadata_section += f"**Severity Breakdown:** {severity_counts['critical']} Critical, "
        #     metadata_section += f"{severity_counts['high']} High, {severity_counts['medium']} Medium, "
        #     metadata_section += f"{severity_counts['low']} Low\n"
        
        metadata_section += "\n"
        return metadata_section
    
    def _create_race_code_context_section(self, result: dict, dir_path: str) -> str:
        """
        Create rich code context section with actual source files and thread entry summaries.
        Ưu tiên:
        1. Tối đa 3 file phát hiện race (file_race_count).
        2. Nếu < 3, bổ sung file có nhiều thread entry points nhất.
        3. Nếu vẫn < 3, bổ sung file có nhiều code nhất (dựa vào số dòng).
        Đảm bảo luôn chọn đủ 3 file nếu số lượng file C++ đầu vào >= 3.
        """
        import collections
        context_section = "## 📁 Source Code Context\n\n"
        summary = result.get('summary', {})
        detected_races = result.get('potential_race_conditions', [])
        markdown_reports = summary.get('markdown_reports', {})
        thread_usage = result.get('thread_usage', {})

        # Lấy danh sách tất cả file C++ đầu vào
        all_cpp_files = summary.get('all_cpp_files', [])
        if not all_cpp_files:
            # Fallback: lấy từ file_summaries nếu không có all_cpp_files
            all_cpp_files = list(summary.get('file_summaries', {}).keys())

        # 1. Ưu tiên file phát hiện race
        file_race_count = collections.defaultdict(int)
        for race in detected_races:
            for file_path in race.get('files_involved', []):
                file_race_count[file_path] += 1
        top_files = sorted(file_race_count.items(), key=lambda x: x[1], reverse=True)
        selected_files = [file_path for file_path, _ in top_files]

        # 2. Nếu < 3, bổ sung file có nhiều thread entry points nhất
        if len(selected_files) < 3:
            remaining_files = [f for f in all_cpp_files if f not in selected_files]
            thread_entry_counts = {f: len(thread_usage.get(f, [])) for f in remaining_files}
            thread_sorted = sorted(thread_entry_counts.items(), key=lambda x: x[1], reverse=True)
            for f, _ in thread_sorted:
                if f not in selected_files:
                    selected_files.append(f)
                if len(selected_files) == 3:
                    break

        # 3. Nếu vẫn < 3, bổ sung file có nhiều code nhất (số dòng)
        if len(selected_files) < 3:
            code_line_counts = {}
            for f in all_cpp_files:
                if f not in selected_files:
                    try:
                        with open(f, 'r', encoding='utf-8') as file:
                            code_line_counts[f] = sum(1 for _ in file)
                    except Exception:
                        code_line_counts[f] = 0
            code_sorted = sorted(code_line_counts.items(), key=lambda x: x[1], reverse=True)
            for f, _ in code_sorted:
                if f not in selected_files:
                    selected_files.append(f)
                if len(selected_files) == 3:
                    break

        # Nếu số lượng file đầu vào < 3 thì lấy hết
        if len(all_cpp_files) < 3:
            final_files = list(dict.fromkeys(all_cpp_files))  # Giữ thứ tự, loại trùng
        else:
            final_files = selected_files[:3]

        # Build context_section
        file_count = 0
        max_file_size = 50000
        for file_path in final_files:
            file_count += 1
            try:
                with open(file_path, 'r', encoding='utf-8') as f:
                    content = f.read()
                if len(content) > max_file_size:
                    content = content[:max_file_size] + "\n\n// ... [TRUNCATED FOR BREVITY] ..."
                filename = os.path.basename(file_path)
                context_section += f"### {file_count}. 📁 **{filename}**\n"
                context_section += f"**Path**: `{file_path}`\n"
                # Race summary
                file_races = [r for r in detected_races if file_path in r.get('files_involved', [])]
                if file_races:
                    context_section += f"**Race Conditions**: {len(file_races)} found\n"
                    for race in file_races[:2]:
                        context_section += f"- {race.get('type', 'unknown')}: {race.get('description', 'No description')}\n"
                # Thread usage summary
                file_threads = thread_usage.get(file_path, [])
                if file_threads:
                    context_section += f"**Thread Usage**: {len(file_threads)} thread-related operations\n"
                # Thread entry markdown
                markdown = markdown_reports.get(file_path, "")
                if markdown:
                    context_section += f"**Thread Entry Points**:\n{markdown}\n"
                # Actual code
                context_section += f"\n```cpp\n{content}\n```\n\n"
            except Exception as e:
                filename = os.path.basename(file_path)
                context_section += f"### {file_count}. 📁 **{filename}** (Error reading: {e})\n\n"
        files_analyzed = summary.get('total_files_analyzed', 0)
        remaining_files = files_analyzed - len(final_files)
        if remaining_files > 0:
            context_section += f"### 📊 **Additional Files**: {remaining_files} more C++ files will not be analyzed because of token limitation\n\n"
        return context_section
    
    def _create_race_analysis_prompt_section(self, race_result: dict) -> str:
        """Create analysis prompt section with only Priority Analysis Guidelines (comment out detailed findings)"""
        # Commented out: Detailed Race Condition Findings
        # detected_races = race_result.get('potential_race_conditions', [])
        # prompt_section = "## 🔍 Detailed Race Condition Findings:\n\n"
        # prompt_section += f"**Total Detected Race Conditions**: {len(detected_races)} issues requiring attention\n\n"
        # if not detected_races:
        #     prompt_section += "✅ **No potential race conditions detected in static analysis.**\n\n"
        #     prompt_section += "However, please perform a manual review focusing on:\n"
        #     prompt_section += "1. Shared state access patterns\n"
        #     prompt_section += "2. Thread synchronization mechanisms\n"
        #     prompt_section += "3. Atomic operations usage\n"
        #     prompt_section += "4. Lock-free programming patterns\n\n"
        #     return prompt_section
        # for i, race in enumerate(detected_races, 1):
        #     severity_emoji = {"critical": "🔴", "high": "🟠", "medium": "🟡", "low": "🟢"}.get(race.get('severity', 'medium').lower(), "🟡")
        #     prompt_section += f"### {severity_emoji} Race Condition #{i}: {race.get('type', 'Unknown')} - {race.get('severity', 'Medium')} Priority\n"
        #     prompt_section += f"- **Description**: {race.get('description', 'No description')}\n"
        #     prompt_section += f"- **Files Involved**: {', '.join(race.get('files_involved', []))}\n"
        #     prompt_section += f"- **Line Numbers**: {', '.join(map(str, race.get('line_numbers', [])))}\n"
        #     prompt_section += f"- **Severity**: {race.get('severity', 'Medium')}\n"
        #     if 'potential_causes' in race:
        #         prompt_section += f"- **Potential Causes**: {race.get('potential_causes', 'Unknown')}\n"
        #     if 'recommended_actions' in race:
        #         prompt_section += f"- **Recommended Actions**: {race.get('recommended_actions', 'Unknown')}\n"
        #     prompt_section += "\n" + "─" * 60 + "\n\n"
        # Only keep Priority Analysis Guidelines
        prompt_section = "## 🎯 Priority Analysis Guidelines:\n\n"
        prompt_section += "1. Focus on shared state accessed by multiple threads.\n"
        prompt_section += "2. Ensure proper synchronization (mutexes, locks, atomics).\n"
        prompt_section += "3. Review thread creation and join/detach logic.\n"
        prompt_section += "4. Check for lock-free and concurrent data structure usage.\n"
        prompt_section += "5. Provide before/after code examples for fixes.\n\n"
        return prompt_section
