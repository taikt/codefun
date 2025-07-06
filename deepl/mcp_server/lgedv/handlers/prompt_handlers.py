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
        """Handle race condition analysis prompt"""
        try:
            dir_path = arguments.get("dir_path") if arguments else None
            if dir_path is None:
                dir_path = get_cpp_dir()
            
            logger.info(f"Starting race condition analysis for: {dir_path}")
            
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
                description="AI-powered race condition analysis with MCP context.",
            )
            
            logger.info("Race condition analysis prompt completed successfully")
            return result
            
        except Exception as e:
            logger.exception(f"Error in race condition analysis prompt: {e}")
            # Return fallback prompt
            return self._create_fallback_race_prompt(dir_path if 'dir_path' in locals() else 'current directory', str(e))
    
    async def _handle_memory_leak_analysis(self, arguments: Dict[str, str] = None) -> types.GetPromptResult:
        """Handle memory leak analysis prompt với rich AI context"""
        dir_path = arguments.get("dir_path") if arguments else None
        if not dir_path:
            dir_path = get_cpp_dir()
        
        logger.info(f"Handling memory leak analysis for directory: {dir_path}")
        
        try:
            # Sử dụng tool handler mới để tạo rich prompt
            from lgedv.handlers.tool_handlers import ToolHandler
            tool_handler = ToolHandler()
            
            # Gọi tool analysis để tạo rich prompt
            tool_result = await tool_handler._handle_ai_memory_analysis({"dir_path": dir_path})
            
            # Lấy rich prompt content từ tool
            rich_prompt_content = tool_result[0].text if tool_result else "Error generating analysis content"
            
            messages = [
                types.PromptMessage(
                    role="user", 
                    content=types.TextContent(type="text", text=rich_prompt_content),
                )
            ]
            
            result = types.GetPromptResult(
                messages=messages,
                description="Comprehensive Memory Leak Analysis with Rich Context",
            )
            
            logger.info("Memory leak analysis prompt completed with rich context")
            return result
            
        except Exception as e:
            logger.error(f"Error in memory leak analysis: {e}")
            # Fallback to old format if error
            return self._create_fallback_memory_leak_prompt(dir_path, str(e))

    def _create_context_summary(self, analysis_result: Dict, dir_path: str) -> str:
        """Tạo context summary từ kết quả phân tích"""
        context_summary = f"""
**CODEBASE ANALYSIS CONTEXT:**
- Total files analyzed: {analysis_result['summary']['total_files_analyzed']}
- Shared resources found: {analysis_result['summary']['total_shared_resources']}
- Files with threads: {analysis_result['summary']['files_with_threads']}
- Basic race conditions detected: {analysis_result['summary']['potential_races']}

**SHARED RESOURCES DETECTED:**
"""
        
        # Add shared resources info
        for resource_name, accesses in analysis_result.get('shared_resources', {}).items():
            context_summary += f"\n- Resource: `{resource_name}` ({len(accesses)} accesses)\n"
            for access in accesses[:3]:  # Limit to first 3 accesses
                context_summary += f"  - File: {access['file']}, Line: {access['line']}\n"
        
        # Add thread usage info
        context_summary += f"""

**THREAD USAGE DETECTED:**
"""
        for file_path, threads in analysis_result.get('thread_usage', {}).items():
            if threads:
                context_summary += f"\n- File: {file_path}\n"
                for thread in threads[:2]:  # Limit to first 2 threads
                    context_summary += f"  - Line {thread['line']}: {thread['code'][:60]}...\n"
        
        # Add sample file contents
        context_summary += self._add_sample_file_contents(analysis_result)
        
        return context_summary
    
    def _add_sample_file_contents(self, analysis_result: Dict) -> str:
        """Thêm sample file contents vào context"""
        context_part = """

**SAMPLE C++ FILE CONTENTS:**
"""
        
        # Tìm files có threads để ưu tiên
        files_with_threads = []
        for file_path, threads in analysis_result.get('thread_usage', {}).items():
            if threads:
                files_with_threads.append(file_path)
        
        # Lấy 2 files đầu tiên có threads, hoặc random 2 files
        files_to_show = files_with_threads[:2] if files_with_threads else []
        if not files_to_show:
            # Fallback: lấy 2 files bất kỳ
            all_files = list(analysis_result.get('thread_usage', {}).keys())
            files_to_show = all_files[:2]
        
        for file_path in files_to_show:
            try:
                with open(file_path, 'r', encoding='utf-8') as f:
                    file_content = f.read()
                    # Giới hạn độ dài để tránh token overflow
                    if len(file_content) > 2000:
                        file_content = file_content[:2000] + "\n\n// ... [TRUNCATED] ..."
                    
                    context_part += f"""
### 📄 File: {file_path}
```cpp
{file_content}
```
"""
            except Exception as e:
                context_part += f"\n❌ Error reading {file_path}: {e}\n"
        
        return context_part
    
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
