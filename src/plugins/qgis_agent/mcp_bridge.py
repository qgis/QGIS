#!/usr/bin/env python3

import json
import os
import sys
import urllib.error
import urllib.request


LOG_PATH = os.environ.get("QGIS_AGENT_MCP_LOG", "")


TOOLS = [
    {
        "name": "project_summary",
        "description": "Inspect the active QGIS project, its CRS, active layer, and loaded layers.",
        "inputSchema": {"type": "object", "properties": {}, "additionalProperties": False},
    },
    {
        "name": "inspect_layer",
        "description": "Inspect one QGIS layer by layer ID or exact layer name, including fields and feature counts.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "layer": {
                    "type": "string",
                    "description": "A QGIS layer ID or exact layer name.",
                }
            },
            "required": ["layer"],
            "additionalProperties": False,
        },
    },
    {
        "name": "list_processing_algorithms",
        "description": "Search QGIS Processing algorithms and return parameter schemas for matching algorithms.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "query": {
                    "type": "string",
                    "description": "Case-insensitive search text for algorithm ID, name, or group.",
                    "default": "",
                },
                "limit": {
                    "type": "integer",
                    "description": "Maximum algorithms to return.",
                    "minimum": 1,
                    "maximum": 50,
                    "default": 12,
                },
            },
            "additionalProperties": False,
        },
    },
    {
        "name": "run_processing_algorithm",
        "description": (
            "Run a QGIS Processing algorithm in the active project. "
            "Use layer IDs returned by project_summary. Missing destination parameters default to temporary outputs. "
            "QGIS asks the user to confirm before execution."
        ),
        "inputSchema": {
            "type": "object",
            "properties": {
                "algorithm_id": {"type": "string"},
                "parameters": {
                    "type": "object",
                    "description": "Processing parameter values keyed by parameter name.",
                },
                "reason": {
                    "type": "string",
                    "description": "Short explanation shown in the QGIS confirmation dialog.",
                },
            },
            "required": ["algorithm_id", "parameters"],
            "additionalProperties": False,
        },
    },
]


def log_event(event, detail=""):
    if not LOG_PATH:
        return
    with open(LOG_PATH, "a", encoding="utf-8") as log_file:
        log_file.write(f"{event}\t{detail}\n")


def read_message():
    line = sys.stdin.buffer.readline()
    if not line:
        return None
    message = json.loads(line)
    log_event("request", message.get("method", ""))
    return message


def write_message(message):
    payload = json.dumps(message, ensure_ascii=True, separators=(",", ":")).encode("utf-8")
    sys.stdout.buffer.write(payload + b"\n")
    sys.stdout.buffer.flush()
    log_event("response", str(message.get("id", "")))


def rpc_call(tool_name, arguments):
    connection_file = os.environ.get("QGIS_AGENT_CONNECTION_FILE", "")
    if connection_file:
        with open(connection_file, encoding="utf-8") as config_file:
            connection = json.load(config_file)
        port = str(connection["port"])
        token = connection["token"]
    else:
        port = os.environ["QGIS_AGENT_PORT"]
        token = os.environ["QGIS_AGENT_TOKEN"]
    body = json.dumps(
        {"tool": tool_name, "arguments": arguments}, separators=(",", ":")
    ).encode("utf-8")
    request = urllib.request.Request(
        f"http://127.0.0.1:{port}/tool",
        data=body,
        headers={
            "Authorization": f"Bearer {token}",
            "Content-Type": "application/json",
        },
        method="POST",
    )
    try:
        with urllib.request.urlopen(request, timeout=3600) as response:
            return json.loads(response.read().decode("utf-8"))
    except urllib.error.HTTPError as error:
        detail = error.read().decode("utf-8", errors="replace")
        raise RuntimeError(f"QGIS tool request failed ({error.code}): {detail}") from error
    except urllib.error.URLError as error:
        raise RuntimeError(f"Cannot reach the QGIS Agent plugin: {error.reason}") from error


def handle_request(message):
    method = message.get("method")
    request_id = message.get("id")

    if method == "initialize":
        result = {
            "protocolVersion": "2025-06-18",
            "capabilities": {"tools": {}},
            "serverInfo": {"name": "qgis-agent", "version": "0.1.0"},
        }
    elif method == "tools/list":
        result = {"tools": TOOLS}
    elif method == "tools/call":
        params = message.get("params", {})
        tool_name = params.get("name", "")
        arguments = params.get("arguments") or {}
        log_event("tool", tool_name)
        try:
            response = rpc_call(tool_name, arguments)
            result = {
                "content": [
                    {
                        "type": "text",
                        "text": json.dumps(response, ensure_ascii=False, indent=2),
                    }
                ],
                "isError": not response.get("ok", False),
            }
        except Exception as error:
            result = {
                "content": [{"type": "text", "text": str(error)}],
                "isError": True,
            }
    elif method == "ping":
        result = {}
    else:
        return {
            "jsonrpc": "2.0",
            "id": request_id,
            "error": {"code": -32601, "message": f"Unsupported method: {method}"},
        }

    return {"jsonrpc": "2.0", "id": request_id, "result": result}


def main():
    while True:
        message = read_message()
        if message is None:
            return
        if "id" not in message:
            continue
        write_message(handle_request(message))


if __name__ == "__main__":
    main()
