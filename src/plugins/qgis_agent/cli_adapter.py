#!/usr/bin/env python3

import argparse
import json
import os
import signal
import subprocess
import sys
import threading
import uuid
from pathlib import Path


class CliAdapter:
    def __init__(self, provider, executable, bridge, connection_file, workspace, model):
        self.provider = provider
        self.executable = executable
        self.bridge = bridge
        self.connection_file = connection_file
        self.workspace = Path(workspace)
        self.workspace.mkdir(parents=True, exist_ok=True)
        self.model = model
        self.thread_id = str(uuid.uuid4())
        self.provider_session_id = None
        self.instructions = ""
        self.current_process = None
        self.current_turn_id = None
        self.interrupted = False
        self.lock = threading.Lock()
        self.write_lock = threading.Lock()

    def send(self, payload):
        data = json.dumps(payload, ensure_ascii=True, separators=(",", ":"))
        with self.write_lock:
            sys.stdout.write(data + "\n")
            sys.stdout.flush()

    def response(self, request_id, result):
        self.send({"id": request_id, "result": result})

    def error_response(self, request_id, code, message):
        self.send(
            {
                "id": request_id,
                "error": {"code": code, "message": message},
            }
        )

    def notification(self, method, params):
        self.send({"method": method, "params": params})

    def handle(self, message):
        method = message.get("method")
        request_id = message.get("id")
        params = message.get("params") or {}

        if method == "initialize":
            self.response(
                request_id,
                {
                    "serverInfo": {
                        "name": f"qgis-agent-{self.provider}-adapter",
                        "version": "0.1.0",
                    }
                },
            )
            return

        if method == "initialized":
            return

        if method == "thread/start":
            self.instructions = params.get("developerInstructions") or ""
            self.response(
                request_id,
                {
                    "thread": {
                        "id": self.thread_id,
                        "status": {"type": "idle"},
                    }
                },
            )
            return

        if method == "turn/start":
            with self.lock:
                if self.current_turn_id is not None:
                    self.error_response(request_id, -32001, "A turn is already running.")
                    return
                turn_id = str(uuid.uuid4())
                self.current_turn_id = turn_id
                self.interrupted = False
            self.response(
                request_id,
                {
                    "turn": {
                        "id": turn_id,
                        "status": "inProgress",
                        "items": [],
                        "error": None,
                    }
                },
            )
            self.notification(
                "turn/started",
                {
                    "threadId": self.thread_id,
                    "turn": {
                        "id": turn_id,
                        "status": "inProgress",
                        "items": [],
                        "error": None,
                    },
                },
            )
            prompt = self.read_prompt(params.get("input"))
            threading.Thread(
                target=self.run_turn,
                args=(turn_id, prompt),
                daemon=True,
            ).start()
            return

        if method == "turn/interrupt":
            self.interrupt_turn(params.get("turnId"))
            self.response(request_id, {})
            return

        if request_id is not None:
            self.error_response(request_id, -32601, f"Unsupported method: {method}")

    @staticmethod
    def read_prompt(items):
        if not isinstance(items, list):
            return ""
        text = []
        for item in items:
            if isinstance(item, dict) and item.get("type") == "text":
                value = item.get("text")
                if isinstance(value, str):
                    text.append(value)
        return "\n".join(text)

    def interrupt_turn(self, turn_id):
        with self.lock:
            if turn_id != self.current_turn_id:
                return
            self.interrupted = True
            process = self.current_process
        if process is None or process.poll() is not None:
            return
        try:
            os.killpg(process.pid, signal.SIGTERM)
        except ProcessLookupError:
            pass

    def run_turn(self, turn_id, prompt):
        answer_id = f"answer-{turn_id}"
        answer = []
        answer_started = False
        tools = {}
        stderr_lines = []
        try:
            command, environment = self.provider_command(prompt)
            process = subprocess.Popen(
                command,
                cwd=self.workspace,
                env=environment,
                stdin=subprocess.DEVNULL,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                bufsize=1,
                start_new_session=True,
            )
            with self.lock:
                self.current_process = process
                interrupted_before_start = self.interrupted
            if interrupted_before_start:
                os.killpg(process.pid, signal.SIGTERM)

            stderr_thread = threading.Thread(
                target=self.collect_stderr,
                args=(process, stderr_lines),
                daemon=True,
            )
            stderr_thread.start()

            for raw_line in process.stdout:
                line = raw_line.strip()
                if not line:
                    continue
                try:
                    event = json.loads(line)
                except json.JSONDecodeError:
                    continue
                if self.provider == "claude":
                    answer_started = self.handle_claude_event(
                        turn_id, answer_id, event, answer, answer_started, tools
                    )
                elif self.provider == "opencode":
                    answer_started = self.handle_opencode_event(
                        turn_id, answer_id, event, answer, answer_started, tools
                    )
                else:
                    answer_started = self.handle_gemini_event(
                        turn_id, answer_id, event, answer, answer_started, tools
                    )

            exit_code = process.wait()
            stderr_thread.join(timeout=1)
            interrupted = self.interrupted
            if answer_started:
                self.complete_answer(turn_id, answer_id, "".join(answer))
            if interrupted:
                self.complete_turn(turn_id, "interrupted")
            elif exit_code == 0 and answer_started:
                self.complete_turn(turn_id, "completed")
            else:
                detail = "\n".join(stderr_lines[-8:]).strip()
                self.complete_turn(
                    turn_id,
                    "failed",
                    detail
                    or (
                        f"{self.provider} completed without returning a response."
                        if exit_code == 0
                        else f"{self.provider} exited with code {exit_code}."
                    ),
                )
        except Exception as error:
            self.complete_turn(turn_id, "failed", str(error))
        finally:
            with self.lock:
                self.current_process = None
                self.current_turn_id = None
                self.interrupted = False

    @staticmethod
    def collect_stderr(process, lines):
        for line in process.stderr:
            text = line.rstrip()
            if text:
                lines.append(text)
                if len(lines) > 100:
                    del lines[:-100]

    def provider_command(self, prompt):
        environment = os.environ.copy()
        if self.provider == "claude":
            mcp_config = self.workspace / "claude-mcp.json"
            qgis_tools = (
                "mcp__qgis_agent__project_summary,"
                "mcp__qgis_agent__inspect_layer,"
                "mcp__qgis_agent__list_processing_algorithms,"
                "mcp__qgis_agent__run_processing_algorithm"
            )
            mcp_config.write_text(
                json.dumps(
                    {
                        "mcpServers": {
                            "qgis_agent": {
                                "type": "stdio",
                                "command": sys.executable,
                                "args": [self.bridge],
                                "env": self.mcp_environment(),
                            }
                        }
                    }
                ),
                encoding="utf-8",
            )
            command = [
                self.executable,
                "-p",
                "--output-format",
                "stream-json",
                "--include-partial-messages",
                "--verbose",
                "--permission-mode",
                "dontAsk",
                "--strict-mcp-config",
                "--tools",
                qgis_tools,
                "--allowedTools",
                qgis_tools,
                "--append-system-prompt",
                self.instructions,
            ]
            if self.model:
                command.extend(["--model", self.model])
            if self.provider_session_id:
                command.extend(["--resume", self.provider_session_id])
            else:
                self.provider_session_id = str(uuid.uuid4())
                command.extend(["--session-id", self.provider_session_id])
            command.extend([f"--mcp-config={mcp_config}", "--", prompt])
            return command, environment

        if self.provider == "opencode":
            environment["OPENCODE_CONFIG_CONTENT"] = json.dumps(
                {
                    "mcp": {
                        "qgis_agent": {
                            "type": "local",
                            "command": [sys.executable, self.bridge],
                            "environment": self.mcp_environment(),
                            "enabled": True,
                            "timeout": 3600000,
                        }
                    },
                    "tools": {
                        "bash": False,
                        "read": False,
                        "edit": False,
                        "write": False,
                        "glob": False,
                        "grep": False,
                        "webfetch": False,
                        "websearch": False,
                    },
                },
                separators=(",", ":"),
            )
            command = [
                self.executable,
                "run",
                "--format",
                "json",
                "--pure",
                "--dir",
                str(self.workspace),
            ]
            if self.model:
                command.extend(["--model", self.model])
            if self.provider_session_id:
                command.extend(["--session", self.provider_session_id])
            command.append(self.prompt_with_instructions(prompt))
            return command, environment

        gemini_settings = self.workspace / ".gemini" / "settings.json"
        gemini_settings.parent.mkdir(parents=True, exist_ok=True)
        gemini_settings.write_text(
            json.dumps(
                {
                    "mcpServers": {
                        "qgis_agent": {
                            "command": sys.executable,
                            "args": [self.bridge],
                            "env": self.mcp_environment(),
                            "timeout": 3600000,
                            "trust": True,
                        }
                    }
                }
            ),
            encoding="utf-8",
        )
        command = [
            self.executable,
            "-p",
            self.prompt_with_instructions(prompt),
            "--output-format",
            "stream-json",
            "--approval-mode",
            "default",
            "--skip-trust",
            "--allowed-mcp-server-names",
            "qgis_agent",
        ]
        if self.model:
            command.extend(["--model", self.model])
        if self.provider_session_id:
            command.extend(["--resume", self.provider_session_id])
        else:
            self.provider_session_id = str(uuid.uuid4())
            command.extend(["--session-id", self.provider_session_id])
        return command, environment

    def prompt_with_instructions(self, prompt):
        if not self.instructions:
            return prompt
        return f"{self.instructions}\n\nUser request:\n{prompt}"

    def mcp_environment(self):
        result = {"QGIS_AGENT_CONNECTION_FILE": self.connection_file}
        log_path = os.environ.get("QGIS_AGENT_MCP_LOG")
        if log_path:
            result["QGIS_AGENT_MCP_LOG"] = log_path
        return result

    def start_answer(self, turn_id, answer_id):
        self.notification(
            "item/started",
            {
                "threadId": self.thread_id,
                "turnId": turn_id,
                "item": {
                    "type": "agentMessage",
                    "id": answer_id,
                    "text": "",
                },
            },
        )

    def answer_delta(self, turn_id, answer_id, text):
        if not text:
            return
        self.notification(
            "item/agentMessage/delta",
            {
                "threadId": self.thread_id,
                "turnId": turn_id,
                "itemId": answer_id,
                "delta": text,
            },
        )

    def complete_answer(self, turn_id, answer_id, text):
        self.notification(
            "item/completed",
            {
                "threadId": self.thread_id,
                "turnId": turn_id,
                "item": {
                    "type": "agentMessage",
                    "id": answer_id,
                    "text": text,
                },
            },
        )

    def reasoning_delta(self, turn_id, item_id, text):
        if not text:
            return
        self.notification(
            "item/reasoning/summaryTextDelta",
            {
                "threadId": self.thread_id,
                "turnId": turn_id,
                "itemId": item_id,
                "summaryIndex": 0,
                "delta": text,
            },
        )

    def start_tool(self, turn_id, item_id, tool_name):
        server, tool = self.tool_identity(tool_name)
        self.notification(
            "item/started",
            {
                "threadId": self.thread_id,
                "turnId": turn_id,
                "item": {
                    "type": "mcpToolCall",
                    "id": item_id,
                    "server": server,
                    "tool": tool,
                    "status": "inProgress",
                },
            },
        )

    def complete_tool(self, turn_id, item_id, tool_name, status):
        server, tool = self.tool_identity(tool_name)
        self.notification(
            "item/completed",
            {
                "threadId": self.thread_id,
                "turnId": turn_id,
                "item": {
                    "type": "mcpToolCall",
                    "id": item_id,
                    "server": server,
                    "tool": tool,
                    "status": status,
                },
            },
        )

    @staticmethod
    def tool_identity(name):
        prefixes = ("mcp__qgis_agent__", "qgis_agent_", "qgis_agent.")
        for prefix in prefixes:
            if name.startswith(prefix):
                return "qgis_agent", name[len(prefix) :]
        return "cli", name or "tool"

    def complete_turn(self, turn_id, status, error=None):
        turn = {
            "id": turn_id,
            "status": status,
            "items": [],
            "error": None if error is None else {"message": error},
        }
        self.notification(
            "turn/completed",
            {"threadId": self.thread_id, "turn": turn},
        )

    def handle_claude_event(
        self, turn_id, answer_id, event, answer, answer_started, tools
    ):
        session_id = event.get("session_id")
        if isinstance(session_id, str) and session_id:
            self.provider_session_id = session_id

        if event.get("type") == "stream_event":
            stream_event = event.get("event") or {}
            event_type = stream_event.get("type")
            if event_type == "content_block_start":
                block = stream_event.get("content_block") or {}
                if block.get("type") == "tool_use":
                    item_id = block.get("id") or f"tool-{uuid.uuid4()}"
                    tools[item_id] = block.get("name") or "tool"
                    self.start_tool(turn_id, item_id, tools[item_id])
            elif event_type == "content_block_delta":
                delta = stream_event.get("delta") or {}
                if delta.get("type") == "text_delta":
                    text = delta.get("text") or ""
                    if text and not answer_started:
                        self.start_answer(turn_id, answer_id)
                        answer_started = True
                    answer.append(text)
                    self.answer_delta(turn_id, answer_id, text)
            return answer_started

        if event.get("type") == "assistant":
            for block in (event.get("message") or {}).get("content") or []:
                if block.get("type") == "tool_use":
                    item_id = block.get("id") or f"tool-{uuid.uuid4()}"
                    if item_id not in tools:
                        tools[item_id] = block.get("name") or "tool"
                        self.start_tool(turn_id, item_id, tools[item_id])
                elif block.get("type") == "text" and not answer_started:
                    text = block.get("text") or ""
                    if text:
                        self.start_answer(turn_id, answer_id)
                        answer_started = True
                        answer.append(text)
                        self.answer_delta(turn_id, answer_id, text)
            return answer_started

        if event.get("type") == "user":
            message = event.get("message") or {}
            for block in message.get("content") or []:
                if block.get("type") != "tool_result":
                    continue
                item_id = block.get("tool_use_id")
                if item_id in tools:
                    self.complete_tool(
                        turn_id,
                        item_id,
                        tools[item_id],
                        "failed" if block.get("is_error") else "completed",
                    )
            return answer_started

        if event.get("type") == "result" and not answer_started:
            text = event.get("result") or ""
            if text:
                self.start_answer(turn_id, answer_id)
                answer_started = True
                answer.append(text)
                self.answer_delta(turn_id, answer_id, text)
        return answer_started

    def handle_opencode_event(
        self, turn_id, answer_id, event, answer, answer_started, tools
    ):
        session_id = event.get("sessionID")
        if isinstance(session_id, str) and session_id:
            self.provider_session_id = session_id

        event_type = event.get("type")
        part = event.get("part") or {}
        if event_type == "text":
            text = part.get("text") or ""
            if text and not answer_started:
                self.start_answer(turn_id, answer_id)
                answer_started = True
            answer.append(text)
            self.answer_delta(turn_id, answer_id, text)
        elif event_type in ("tool", "tool_use"):
            item_id = part.get("callID") or part.get("id") or f"tool-{uuid.uuid4()}"
            tool_name = part.get("tool") or "tool"
            state = part.get("state") or {}
            status = state.get("status")
            if item_id not in tools:
                tools[item_id] = tool_name
                self.start_tool(turn_id, item_id, tool_name)
            if status in ("completed", "error"):
                self.complete_tool(
                    turn_id,
                    item_id,
                    tool_name,
                    "completed" if status == "completed" else "failed",
                )
                tools.pop(item_id, None)
        elif event_type in ("error", "session_error"):
            error = event.get("error") or part.get("error") or event.get("message")
            raise RuntimeError(self.error_text(error))
        return answer_started

    def handle_gemini_event(
        self, turn_id, answer_id, event, answer, answer_started, tools
    ):
        session_id = event.get("session_id")
        if isinstance(session_id, str) and session_id:
            self.provider_session_id = session_id

        event_type = event.get("type")
        if event_type == "message" and event.get("role") == "assistant":
            text = event.get("content") or ""
            if text and not answer_started:
                self.start_answer(turn_id, answer_id)
                answer_started = True
            answer.append(text)
            self.answer_delta(turn_id, answer_id, text)
        elif event_type == "tool_use":
            item_id = event.get("tool_id") or f"tool-{uuid.uuid4()}"
            tool_name = event.get("tool_name") or "tool"
            tools[item_id] = tool_name
            self.start_tool(turn_id, item_id, tool_name)
        elif event_type == "tool_result":
            item_id = event.get("tool_id") or f"tool-{uuid.uuid4()}"
            tool_name = tools.pop(item_id, "tool")
            self.complete_tool(
                turn_id,
                item_id,
                tool_name,
                "completed" if event.get("status") == "success" else "failed",
            )
        elif event_type == "error":
            if event.get("severity") == "error":
                raise RuntimeError(self.error_text(event.get("message")))
        elif event_type == "result" and event.get("status") == "error":
            raise RuntimeError(self.error_text(event.get("error")))
        return answer_started

    @staticmethod
    def error_text(error):
        if isinstance(error, str):
            return error
        if isinstance(error, dict):
            return str(error.get("message") or error.get("name") or error)
        return str(error or "Unknown provider error")


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--provider", choices=("claude", "opencode", "gemini"), required=True
    )
    parser.add_argument("--executable", required=True)
    parser.add_argument("--bridge", required=True)
    parser.add_argument("--connection-file", required=True)
    parser.add_argument("--workspace", required=True)
    parser.add_argument("--model", default="")
    return parser.parse_args()


def main():
    args = parse_args()
    adapter = CliAdapter(
        args.provider,
        args.executable,
        args.bridge,
        args.connection_file,
        args.workspace,
        args.model,
    )
    try:
        for line in sys.stdin:
            if not line.strip():
                continue
            try:
                adapter.handle(json.loads(line))
            except Exception as error:
                request_id = None
                try:
                    request_id = json.loads(line).get("id")
                except Exception:
                    pass
                if request_id is not None:
                    adapter.error_response(request_id, -32603, str(error))
    finally:
        adapter.interrupt_turn(adapter.current_turn_id)


if __name__ == "__main__":
    main()
