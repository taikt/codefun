import anyio
import click
import mcp.types as types
from mcp.server.lowlevel import Server
from mcp.shared._httpx_utils import create_mcp_http_client
from pydantic import AnyUrl, FileUrl
import os
import sys
import logging  # Thêm logging

# Thiết lập cấu hình logging ghi ra stderr và file
logging.basicConfig(
    level=logging.DEBUG,
    format="%(asctime)s [%(levelname)s] %(message)s",
    handlers=[
        logging.StreamHandler(sys.stderr),
        logging.FileHandler("/tmp/mcp_simple_prompt.log")
    ]
)
logger = logging.getLogger(__name__)

# Đặt BASE_DIR là thư mục chứa file server.py
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
MISRA_RULE_URL = "http://cisivi.lge.com:8060/files/copilot_md/common/Misracpp2008Guidelines_High_en.md"
LGEDV_RULE_URL = os.path.join(BASE_DIR, "resources", "LGEDVRuleGuide.md")

RESOURCE_FILES = {
    "lgedv": os.path.join(BASE_DIR, "resources", "LGEDVRuleGuide.md"),
    "misra": os.path.join(BASE_DIR, "resources", "Misracpp2008Guidelines_High_en.md"),
}


async def fetch_misra_rule(url: str) -> list[
    types.TextContent | types.ImageContent | types.AudioContent | types.EmbeddedResource
]:
    logger.debug(f"Fetching MISRA rule from URL: {url}")
    headers = {
        "User-Agent": "MCP MISRA Rule Fetcher"
    }
    async with create_mcp_http_client(headers=headers) as client:
        response = await client.get(url)
        response.raise_for_status()
        logger.debug("Fetched MISRA rule successfully")
        return [types.TextContent(type="text", text=response.text)]


async def fetch_lgedv_rule(url: str) -> list[
    types.TextContent | types.ImageContent | types.AudioContent | types.EmbeddedResource
]:
    logger.debug(f"Fetching LGEDV rule from: {url}")
    headers = {
        "User-Agent": "MCP LGEDV Rule Fetcher"
    }
    if url.startswith("http://") or url.startswith("https://"):
        async with create_mcp_http_client(headers=headers) as client:
            response = await client.get(url)
            response.raise_for_status()
            logger.debug("Fetched LGEDV rule from remote successfully")
            return [types.TextContent(type="text", text=response.text)]
    else:
        # Treat as local file path
        if not os.path.exists(url):
            logger.error(f"LGEDV rule file not found: {url}")
            raise FileNotFoundError(f"LGEDV rule file not found: {url}")
        with open(url, "r", encoding="utf-8") as f:
            text = f.read()
        logger.debug("Fetched LGEDV rule from local file successfully")
        return [types.TextContent(type="text", text=text)]


def get_resource_metadata(name: str, file_path: str) -> types.Resource:
    return types.Resource(
        uri=FileUrl(f"file://{file_path}"),
        name=name,
        title=f"{name.upper()} Rule Guide",
        description=f"{name.upper()} rule guideline markdown resource.",
        mimeType="text/markdown",
    )


def list_cpp_files(dir_path: str = None) -> list[str]:
    """Trả về danh sách file .cpp trong dir_path hoặc CPP_DIR hoặc cwd."""
    if dir_path is None:
        dir_path = os.environ.get("CPP_DIR")
        if not dir_path:
            dir_path = os.getcwd()
    cpp_files = []
    for root, dirs, files in os.walk(dir_path):
        for file in files:
            if file.endswith('.cpp'):
                cpp_files.append(os.path.relpath(os.path.join(root, file), dir_path))
    return cpp_files


@click.command()
@click.option("--port", default=8000, help="Port to listen on for SSE")
@click.option(
    "--transport",
    type=click.Choice(["stdio", "sse"]),
    default="stdio",
    help="Transport type",
)
def main(port: int, transport: str):
    try:
        logger.info("Server started and ready to receive requests")
        logger.info(f"Starting server with transport: {transport}, port: {port}")
        app = Server("mcp-misra-tool")
        logger.debug("App server object created")

        logger.debug("Registering @app.call_tool handler...")
        @app.call_tool()
        async def fetch_tool(
            name: str, arguments: dict
        ) -> list[
            types.TextContent
            | types.ImageContent
            | types.AudioContent
            | types.EmbeddedResource
        ]:
            logger.info(f"fetch_tool called with name: {name}, arguments: {arguments}")
            try:
                if name == "fetch_misra_rule":
                    url = arguments.get("url")
                    if not url or not (url.startswith("http://") or url.startswith("https://")):
                        url = MISRA_RULE_URL
                    result = await fetch_misra_rule(url)
                    logger.info(f"[taikt] fetch_misra_rule completed for url: {url}")
                    return result
                elif name == "fetch_lgedv_rule":
                    url = arguments.get("url")
                    if not url:
                        url = LGEDV_RULE_URL
                    result = await fetch_lgedv_rule(url)
                    logger.info(f"[taikt] fetch_lgedv_rule completed for url: {url}")
                    return result
                elif name == "list_cpp_files":
                    dir_path = arguments.get("dir_path")
                    files = list_cpp_files(dir_path)
                    logger.info(f"[taikt] list_cpp_files found {len(files)} files")
                    return [types.TextContent(type="text", text="\n".join(files))]
                elif name == "get_cpp_files_content":
                    dir_path = arguments.get("dir_path")
                    if dir_path is None:
                        dir_path = os.environ.get("CPP_DIR") or os.getcwd()
                    cpp_files = list_cpp_files(dir_path)
                    logger.info(f"[taikt] get_cpp_files_content found {len(cpp_files)} files")
                    contents = []
                    for file in cpp_files:
                        abs_path = os.path.join(dir_path, file)
                        try:
                            with open(abs_path, "r", encoding="utf-8") as f:
                                code = f.read()
                            contents.append(f"// File: {file}\n{code}\n")
                        except Exception as e:
                            logger.error(f"Failed to read {abs_path}: {e}")
                    return [types.TextContent(type="text", text="\n".join(contents))]
                else:
                    logger.error(f"Unknown tool: {name}")
                    raise ValueError(f"Unknown tool: {name}")
            except Exception as e:
                logger.exception(f"Exception in fetch_tool: {e}")
                raise
        logger.debug("Registered @app.call_tool handler")

        logger.debug("Registering @app.list_tools handler...")
        @app.list_tools()
        async def list_tools() -> list[types.Tool]:
            logger.info("list_tools called")
            logger.debug("list_tools called")
            return [
                types.Tool(
                    name="fetch_misra_rule",
                    description="Fetches the MISRA C++ 2008 rule markdown from remote server.",
                    inputSchema={
                        "type": "object",
                        "properties": {
                            "url": {
                                "type": "string",
                                "description": "URL to fetch MISRA rule (optional, default is preset)",
                            }
                        },
                    },
                ),
                types.Tool(
                    name="fetch_lgedv_rule",
                    description="Fetches the LGEDV rule markdown from remote server.",
                    inputSchema={
                        "type": "object",
                        "properties": {
                            "url": {
                                "type": "string",
                                "description": "URL to fetch LGEDV rule (optional, default is preset)",
                            }
                        },
                    },
                ),
                types.Tool(
                    name="list_cpp_files",
                    description="List all .cpp files in the current CPP_DIR or given directory.",
                    inputSchema={
                        "type": "object",
                        "properties": {
                            "dir_path": {
                                "type": "string",
                                "description": "Directory to search for .cpp files (optional, default is CPP_DIR or cwd)",
                            }
                        },
                    },
                ),
                types.Tool(
                    name="get_cpp_files_content",
                    description="Get the content of all .cpp files in the current CPP_DIR or given directory as a single response for context.",
                    inputSchema={
                        "type": "object",
                        "properties": {
                            "dir_path": {
                                "type": "string",
                                "description": "Directory to search for .cpp files (optional, default is CPP_DIR or cwd)",
                            }
                        },
                    },
                ),
            ]
        logger.debug("Registered @app.list_tools handler")

        logger.debug("Registering @app.list_prompts handler...")
        @app.list_prompts()
        async def list_prompts() -> list[types.Prompt]:
            logger.info("list_prompts called")
            logger.debug("list_prompts called")
            return [
                types.Prompt(
                    name="lgedv_all_files",
                    description="check code violations based on LGEDV rule guide for all cpp files.",
                    arguments=[
                        types.PromptArgument(
                            name="context",
                            description="Additional context to consider",
                            required=False,
                        ),
                        types.PromptArgument(
                            name="topic",
                            description="Specific topic to focus on",
                            required=False,
                        ),
                    ],
                ),
                types.Prompt(
                    name="misra_all_files",
                    description="check code violations based on MISRA C++ 2008 rule guide for all cpp files.",
                    arguments=[
                        types.PromptArgument(
                            name="context",
                            description="Additional context to consider",
                            required=False,
                        ),
                        types.PromptArgument(
                            name="topic",
                            description="Specific topic to focus on",
                            required=False,
                        ),
                    ],
                ),
                types.Prompt(
                    name="lgedv_current_file",
                    description="check code violations based on LGEDV rule guide for current file.",
                    arguments=[
                        types.PromptArgument(
                            name="context",
                            description="Additional context to consider",
                            required=False,
                        ),
                    ],
                ),
                types.Prompt(
                    name="misra_current_file",
                    description="check code violations based on MISRA C++ 2008 rule guide for current file.",
                    arguments=[
                        types.PromptArgument(
                            name="context",
                            description="Additional context to consider",
                            required=False,
                        ),
                    ],
                ),
                types.Prompt(
                    name="fetch_misra_rule",
                    description="Fetch the MISRA C++ 2008 rule markdown from remote server.",
                    arguments=[],
                ),
            ]
        logger.debug("Registered @app.list_prompts handler")

        logger.debug("Registering @app.get_prompt handler...")
        @app.get_prompt()
        async def get_prompt(
            name: str, arguments: dict[str, str] | None = None
        ) -> types.GetPromptResult:
            logger.info(f"get_prompt called with name: {name}, arguments: {arguments}")
            try:
                if name == "lgedv_all_files":
                    prompt = (
                        "Find C++ violations based on LGEDV rules for all .cpp files in the directory. "
                        "For each file, write down the file name first. "
                        "For each violation, indicate the rule content, the exact line number(s) in the file where the rule is violated, "
                        "and explain why the rule is violated. "
                        "Finally, suggest detailed code to fix the violation. "
                        "Please read the files directly from the workspace to ensure line numbers are accurate."
                        "Please check full the file's content."
                    )
                    messages = [
                        types.PromptMessage(
                            role="user",
                            content=types.TextContent(type="text", text=prompt),
                        )
                    ]
                    result = types.GetPromptResult(
                        messages=messages,
                        description="A prompt for LGEDV rule on all cpp files.",
                    )
                    logger.info(f"get_prompt {name} completed")
                    return result
                elif name == "misra_all_files":
                    prompt = (
                        "Find C++ violations based on MISRA C++ 2008 rules for all .cpp files in the directory. "
                        "For each file, write down the file name first. "
                        "For each violation, indicate the rule content, the exact line number(s) in the file where the rule is violated, "
                        "and explain why the rule is violated. "
                        "Finally, suggest detailed code to fix the violation. "
                        "Please read the files directly from the workspace to ensure line numbers are accurate."
                        "Please check full the file's content."
                    )
                    messages = [
                        types.PromptMessage(
                            role="user",
                            content=types.TextContent(type="text", text=prompt),
                        )
                    ]
                    result = types.GetPromptResult(
                        messages=messages,
                        description="A prompt for MISRA rule on all cpp files.",
                    )
                    logger.info(f"get_prompt {name} completed")
                    return result
                elif name == "lgedv_current_file":
                    prompt = (
                        "Find C++ violations based on lgedv rules for current file in the directory. "
                        "For each violation, indicate the rule content, the exact line number(s) in the file where the rule is violated, "
                        "and explain why the rule is violated. "
                        "Finally, suggest detailed code to fix the violation. "
                        "Please read the file directly from the workspace to ensure line numbers are accurate."
                        "Please check full the file's content."
                    )
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
                    logger.info(f"get_prompt {name} completed")
                    return result
                elif name == "misra_current_file":
                    prompt = (
                        "Find C++ violations based on Misra rules for current file in the directory. "
                        "For each violation, indicate the rule content, the exact line number(s) in the file where the rule is violated, "
                        "and explain why the rule is violated. "
                        "Finally, suggest detailed code to fix the violation. "
                        "Please read the file directly from the workspace to ensure line numbers are accurate."
                        "Please check full the file's content."
                    )
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
                    logger.info(f"get_prompt {name} completed")
                    return result
                elif name == "fetch_misra_rule":
                    import httpx
                    async with httpx.AsyncClient() as client:
                        resp = await client.get(MISRA_RULE_URL)
                        resp.raise_for_status()
                        rule_content = resp.text
                    logger.info("Fetched MISRA rule content for prompt")
                    return types.GetPromptResult(
                        messages=[
                            types.PromptMessage(
                                role="assistant",
                                content=types.TextContent(type="text", text=rule_content),
                            )
                        ],
                        description="MISRA C++ 2008 rule markdown fetched from remote server.",
                    )
                else:
                    logger.error(f"Unknown prompt: {name}")
                    raise ValueError(f"Unknown prompt: {name}")
            except Exception as e:
                logger.exception(f"Exception in get_prompt: {e}")
                raise
        logger.debug("Registered @app.get_prompt handler")

        logger.debug("Registering @app.list_resources handler...")
        @app.list_resources()
        async def list_resources() -> list[types.Resource]:
            logger.info("list_resources called")
            resources = [
                get_resource_metadata("lgedv", RESOURCE_FILES["lgedv"]),
                get_resource_metadata("misra", RESOURCE_FILES["misra"]),
            ]
            return resources
        logger.debug("Registered @app.list_resources handler")

        logger.info("All handlers registered. Entering main event loop or waiting for requests...")
        if transport == "sse":
            from mcp.server.sse import SseServerTransport
            from starlette.applications import Starlette
            from starlette.responses import Response
            from starlette.routing import Mount, Route

            sse = SseServerTransport("/messages/")

            async def handle_sse(request):
                logger.info("Handling SSE connection")
                try:
                    async with sse.connect_sse(
                        request.scope, request.receive, request._send
                    ) as streams:
                        await app.run(
                            streams[0], streams[1], app.create_initialization_options()
                        )
                    logger.info("SSE connection handled successfully")
                except Exception as e:
                    logger.exception(f"Exception in handle_sse: {e}")
                return Response()
        else:
            from mcp.server.stdio import stdio_server

            async def arun():
                logger.info("Running stdio server")
                try:
                    async with stdio_server() as streams:
                        await app.run(
                            streams[0], streams[1], app.create_initialization_options()
                        )
                    logger.info("stdio server run completed")
                except Exception as e:
                    logger.exception(f"Exception in stdio server: {e}")
            anyio.run(arun)
        logger.info("Server stopped")
        return 0
    except Exception as e:
        logger.exception(f"Fatal error in main: {e}")
        print(f"Fatal error in main: {e}", file=sys.stderr)
        raise