"""
Prompt handlers for MCP server operations
Xử lý các MCP prompts cho phân tích code
"""
import os
from typing import Dict
from mcp import types
from lgedv.prompts.prompt_templates import PromptTemplates
from lgedv.analyzers.race_analyzer import analyze_race_conditions_in_codebase
from lgedv.analyzers.memory_analyzer import MemoryAnalyzer
from lgedv.modules.config import get_cpp_dir, setup_logging

logger = setup_logging()

class PromptHandler:
    """Handler cho các MCP prompts"""
    
    def __init__(self):
        self.templates = PromptTemplates()
    
    async def handle_prompt(self, name: str, arguments: Dict[str, str] = None) -> types.GetPromptResult:
        """
        Route và xử lý prompt calls
        
        Args:
            name: Tên prompt
            arguments: Arguments cho prompt
            
        Returns:
            GetPromptResult
        """
        logger.info(f"Prompt called: {name} with arguments: {arguments}")
        
        try:
            if name == "check_lgedv":
                return await self._handle_lgedv_check()
            elif name == "check_misra":
                return await self._handle_misra_check()
            elif name == "check_certcpp":
                return await self._handle_certcpp_check()
            elif name == "check_custom":
                return await self._handle_custom_check()
            elif name == "check_races":
                return await self._handle_race_condition_analysis(arguments)
            elif name == "check_leaks":
                return await self._handle_memory_leak_analysis(arguments)
            elif name == "check_resources":
                return await self._handle_resource_leak_analysis(arguments)
            else:
                raise ValueError(f"Unknown prompt: {name}")
                
        except Exception as e:
            logger.exception(f"Error in prompt handler for {name}: {e}")
            raise
    
    async def _handle_lgedv_check(self) -> types.GetPromptResult:
        """Handle LGEDV code checking prompt"""
        prompt = self.templates.get_lgedv_analysis_prompt()
        
        messages = [
            types.PromptMessage(
                role="user",
                content=types.TextContent(type="text", text=prompt),
            )
        ]
        
        result = types.GetPromptResult(
            messages=messages,
            description="A prompt for LGEDV rule on current file.",
        )
        
        logger.info("LGEDV check prompt completed")
        return result
    
    async def _handle_misra_check(self) -> types.GetPromptResult:
        """Handle MISRA code checking prompt"""
        prompt = self.templates.get_misra_analysis_prompt()
        
        messages = [
            types.PromptMessage(
                role="user",
                content=types.TextContent(type="text", text=prompt),
            )
        ]
        
        result = types.GetPromptResult(
            messages=messages,
            description="A prompt for MISRA rule on current file.",
        )
        
        logger.info("MISRA check prompt completed")
        return result
    
    async def _handle_certcpp_check(self) -> types.GetPromptResult:
        """Handle CERT C++ code checking prompt"""
        prompt = self.templates.get_certcpp_analysis_prompt()
        
        messages = [
            types.PromptMessage(
                role="user",
                content=types.TextContent(type="text", text=prompt),
            )
        ]
        
        result = types.GetPromptResult(
            messages=messages,
            description="A prompt for CERT C++ rule on current file.",
        )
        
        logger.info("CERT C++ check prompt completed")
        return result
    
    async def _handle_custom_check(self) -> types.GetPromptResult:
        """Handle Custom rule checking prompt"""
        prompt = self.templates.get_custom_analysis_prompt()
        
        messages = [
            types.PromptMessage(
                role="user",
                content=types.TextContent(type="text", text=prompt),
            )
        ]
        
        result = types.GetPromptResult(
            messages=messages,
            description="A prompt for Custom rule on current file.",
        )
        
        logger.info("Custom check prompt completed")
        return result
    
    async def _handle_race_condition_analysis(self, arguments: Dict[str, str] = None) -> types.GetPromptResult:
        """Handle race condition analysis prompt - sử dụng CPP_DIR từ config"""
        try:
            # Sử dụng CPP_DIR trực tiếp từ config
            dir_path = get_cpp_dir()
            logger.info(f"[check_races] Using CPP_DIR: {dir_path}")
            
            # Sử dụng analyzer để thu thập context
            analysis_result = analyze_race_conditions_in_codebase(dir_path)
            
            logger.info(f"Analysis completed. Found {analysis_result['summary']['total_files_analyzed']} files")
            
            # Tạo context summary
            context_summary = self._create_context_summary(analysis_result, dir_path)
            
            # Tạo prompt với context
            prompt = self.templates.get_race_condition_analysis_prompt(context_summary)
            
            messages = [
                types.PromptMessage(
                    role="user",
                    content=types.TextContent(type="text", text=prompt),
                )
            ]
            
            result = types.GetPromptResult(
                messages=messages,
                description="Race condition analysis using CPP_DIR",
            )
            
            logger.info("Race condition analysis prompt completed")
            return result
            
        except Exception as e:
            logger.exception(f"Error in race condition analysis prompt: {e}")
            # Return fallback prompt
            return self._create_fallback_race_prompt(dir_path if 'dir_path' in locals() else 'current directory', str(e))
    
    async def _handle_memory_leak_analysis(self, arguments: Dict[str, str] = None) -> types.GetPromptResult:
        """Handle memory leak analysis prompt - sử dụng CPP_DIR từ config"""
        # Sử dụng CPP_DIR trực tiếp từ config
        dir_path = get_cpp_dir()
        logger.info(f"[check_leaks] Using CPP_DIR: {dir_path}")
        
        try:
            from lgedv.handlers.tool_handlers import ToolHandler
            tool_handler = ToolHandler()
            tool_result = await tool_handler._handle_ai_memory_analysis({"dir_path": dir_path})
            rich_prompt_content = tool_result[0].text if tool_result else "Error generating analysis content"
            
            messages = [
                types.PromptMessage(
                    role="user",
                    content=types.TextContent(type="text", text=rich_prompt_content),
                )
            ]
            
            result = types.GetPromptResult(
                messages=messages,
                description="Memory Leak Analysis using CPP_DIR",
            )
            
            logger.info("Memory leak analysis prompt completed")
            return result
            
        except Exception as e:
            logger.error(f"Error in memory leak analysis: {e}")
            return self._create_fallback_memory_leak_prompt(dir_path, str(e))

    async def _handle_resource_leak_analysis(self, arguments: Dict[str, str] = None) -> types.GetPromptResult:
        """Handle resource leak analysis prompt - sử dụng CPP_DIR từ config giống check_leaks"""
        # Sử dụng CPP_DIR trực tiếp từ config
        dir_path = get_cpp_dir()
        logger.info(f"[check_resources] Using CPP_DIR: {dir_path}")
        
        try:
            from lgedv.handlers.tool_handlers import ToolHandler
            tool_handler = ToolHandler()
            tool_result = await tool_handler._handle_analyze_resources({})
            rich_prompt_content = tool_result[0].text if tool_result else "Error generating analysis content"
            
            messages = [
                types.PromptMessage(
                    role="user",
                    content=types.TextContent(type="text", text=rich_prompt_content),
                )
            ]
            
            result = types.GetPromptResult(
                messages=messages,
                description="Resource Leak Analysis using CPP_DIR",
            )
            
            logger.info("Resource leak analysis prompt completed")
            return result
            
        except Exception as e:
            logger.error(f"Error in resource leak analysis: {e}")
            return self._create_fallback_resource_leak_prompt(dir_path, str(e))
            
            # Combine sections into comprehensive prompt
            full_prompt = f"""# 🔍 Linux C++ Resource Leak Analysis Request

{metadata_section}

{code_context_section}

{analysis_prompt}

## � Please Provide:

1. **Detailed Analysis**: Review each potential resource leak and assess its validity
2. **Risk Assessment**: Categorize findings by severity and impact on system resources
3. **Fix Recommendations**: Specific code changes for each resource leak
4. **RAII Implementation**: Suggest modern C++ resource management patterns
5. **Linux Best Practices**: Proper cleanup patterns for file descriptors, sockets, etc.

## 📋 Expected Output Format:

For each resource leak found:
- **Resource Type**: (e.g., File Descriptor, Socket, Memory Mapping)
- **Severity**: Critical/High/Medium/Low
- **Location**: File and line number
- **Current Code**: Show the problematic code
- **Fixed Code**: Show the corrected version with proper cleanup
- **RAII Wrapper**: Suggest or show a RAII wrapper class if applicable
- **Explanation**: Why this leak is problematic and how the fix works

Focus on Linux-specific resource management and actionable recommendations that can be immediately implemented.
"""

            messages = [
                types.PromptMessage(
                    role="user",
                    content=types.TextContent(type="text", text=full_prompt),
                )
            ]
            
            result = types.GetPromptResult(
                messages=messages,
                description=f"Resource leak analysis prompt for {cpp_dir}",
            )
            
            logger.info(f"Resource leak analysis prompt completed. Analysis available for Copilot review")
            return result
            
        except Exception as e:
            logger.error(f"Error in resource leak analysis: {e}")
            return self._create_fallback_resource_leak_prompt(dir_path, str(e))
    
    def _format_resource_leak_summary(self, leaks: list) -> str:
        """Format a summary of resource leaks by type and severity"""
        summary = {}
        severity_counts = {'critical': 0, 'high': 0, 'medium': 0, 'low': 0}
        
        for leak in leaks:
            leak_type = leak.get('type', 'unknown')
            severity = leak.get('severity', 'medium')
            
            if leak_type not in summary:
                summary[leak_type] = 0
            
            summary[leak_type] += 1
            severity_counts[severity] += 1
        
        summary_text = f"**By Severity:** {severity_counts['critical']} Critical, {severity_counts['high']} High, {severity_counts['medium']} Medium, {severity_counts['low']} Low\n\n"
        summary_text += "**By Resource Type:**\n"
        
        for leak_type, count in summary.items():
            summary_text += f"- {leak_type.title()}: {count} leak(s)\n"
        
        return summary_text
    
    def _create_fallback_race_prompt(self, dir_path: str, error_msg: str) -> types.GetPromptResult:
        """Tạo fallback prompt khi có lỗi"""
        fallback_prompt = f"""You are an expert C++ concurrency analyst. 

There was an error analyzing the codebase automatically: {error_msg}

Please use the `detect_races` tool first to manually analyze the C++ files in the directory: {dir_path}

Then provide your expert analysis of potential race conditions, focusing on:
1. Unprotected shared state modifications
2. Missing synchronization mechanisms
3. Thread-unsafe patterns
4. Potential deadlock scenarios

Use this format for each issue found:

## 🚨 **RACE CONDITION #[number]**: [Brief Description]
**Type:** [data_race|deadlock|missing_sync]
**Severity:** [Critical|High|Medium|Low]
**Files Involved:** [list of files]
**Problem Description:** [explanation]
**Fix Recommendation:** [suggested solution]
"""
        messages = [
            types.PromptMessage(
                role="user", 
                content=types.TextContent(type="text", text=fallback_prompt),
            )
        ]
        
        result = types.GetPromptResult(
            messages=messages,
            description="Race condition analysis (fallback mode).",
        )
        
        logger.info("Race condition analysis fallback prompt completed")
        return result
    
    def _create_fallback_memory_leak_prompt(self, dir_path: str, error_msg: str) -> types.GetPromptResult:
        """Tạo fallback prompt cho phân tích rò rỉ bộ nhớ"""
        fallback_prompt = f"""You are an expert C++ memory management analyst. 

There was an error analyzing the codebase for memory leaks automatically: {error_msg}

Please use the `analyze_leaks` tool first to manually analyze the C++ files in the directory: {dir_path}

Then provide your expert analysis of potential memory leaks, focusing on:
1. Unreleased memory allocations
2. Dangling pointers
3. Memory corruption issues
4. Inefficient memory usage patterns

Use this format for each issue found:

## 🚨 **MEMORY LEAK #[number]**: [Brief Description]
**Severity:** [Critical|High|Medium|Low]
**Files Involved:** [list of files]
**Problem Description:** [explanation]
**Fix Recommendation:** [suggested solution]
"""
        messages = [
            types.PromptMessage(
                role="user", 
                content=types.TextContent(type="text", text=fallback_prompt),
            )
        ]
        
        result = types.GetPromptResult(
            messages=messages,
            description="Memory leak analysis (fallback mode).",
        )
        
        logger.info("Memory leak analysis fallback prompt completed")
        return result

    def _create_fallback_resource_leak_prompt(self, dir_path: str, error_msg: str) -> types.GetPromptResult:
        """Create fallback prompt for resource leak analysis"""
        fallback_prompt = f"""You are an expert Linux C++ resource management analyst.

There was an error analyzing the codebase for resource leaks automatically: {error_msg}

Please use the `analyze_resources` tool first to manually analyze the C++ files in the directory: {dir_path}

Then provide your expert analysis of potential resource leaks, focusing on:

## 🎯 **Analysis Focus Areas**

1. **File Resources:**
   - Unmatched open()/close() calls
   - FILE* streams not properly closed
   - Missing fclose() for fopen()

2. **Socket Resources:**
   - Socket descriptors not closed
   - Network connections left open
   - Unmatched socket()/close() pairs

3. **Memory Mapping:**
   - mmap() without corresponding munmap()
   - Shared memory segments not cleaned up

4. **IPC Resources:**
   - Message queues not destroyed
   - Semaphores not cleaned up
   - Shared memory not detached

5. **Directory Handles:**
   - opendir() without closedir()
   - Directory streams left open

## 📋 **Report Format**
For each resource leak found, use this format:

### 🚨 **RESOURCE LEAK #[number]**: [Resource Type]
- **Severity:** [Critical|High|Medium|Low]
- **File:** [filename]
- **Line:** [line number]
- **Resource:** [specific resource name/variable]
- **Description:** [what resource is leaking and why]
- **Fix:** [specific remediation steps]

Focus on Linux-specific resources and provide actionable recommendations for each finding.
"""
        
        messages = [
            types.PromptMessage(
                role="user", 
                content=types.TextContent(type="text", text=fallback_prompt),
            )
        ]
        
        result = types.GetPromptResult(
            messages=messages,
            description="Resource leak analysis (fallback mode).",
        )
        
        logger.info("Resource leak analysis fallback prompt completed")
        return result
