# QGIS Agent

QGIS Agent is an experimental native QGIS plugin that connects the local QGIS
session to a locally installed coding-agent CLI through an MCP bridge. TraeX is
the default provider. The dock lets you switch providers and model choices;
changing either setting restarts the provider session before the next task.
Supported CLIs are detected at startup and shown in the provider dropdown.

## Architecture

- The C++ plugin provides the dock panel and executes QGIS operations on the
  application thread.
- `mcp_bridge.py` exposes QGIS operations as MCP tools over standard input and
  output.
- TraeX and Codex use their native long-lived `app-server`. Claude Code,
  OpenCode, and Gemini CLI use the bundled `cli_adapter.py`, which presents the
  same thread/turn event protocol to the QGIS panel and resumes the selected
  CLI's session across turns.
- The dock uses a chat-style action button. It sends the current message when
  idle and interrupts the active turn while a task is running.
- A compact loading indicator is shown directly below the active Agent reply
  and is hidden as soon as the turn finishes, fails, or is interrupted.
- Press Enter to send from the input box. Use Shift+Enter for a newline.
- Agent answers, provider reasoning summaries, plans, and tool activity are
  rendered from app-server delta/lifecycle events while the turn is running.
  Hidden model chain-of-thought is not exposed.
- The app-server receives a temporary MCP configuration. The plugin does not
  modify the user's global Codex configuration.
- The bridge connects only to a random loopback port protected by a per-session
  token.

## Available tools

- `project_summary`
- `inspect_layer`
- `list_processing_algorithms`
- `run_processing_algorithm`

Processing output destinations default to temporary outputs and resulting map
layers are added to the active project. Write operations require confirmation
in QGIS unless the confirmation checkbox is disabled.
Every user turn has a withdraw icon. Withdrawing a turn hides that turn's
question, answer, plan, and tool transcript. If the turn loaded Processing
output layers, withdrawing also removes those layers from the active project;
operations outside this recorded layer-loading path are not rolled back.

Greetings and general questions use the same Agent thread as GIS work. There is
no UI-side intent classifier: the Agent decides whether a tool call is needed.
The Codex process runs in a dedicated temporary workspace with a read-only turn
sandbox. QGIS remains the authority for GIS mutations and confirmation.

## Requirements

- A working login for at least one supported CLI
- Python 3 for the MCP bridge
- QGIS built with desktop and GUI support

The plugin detects `traex`, `codex`, `claude`, `opencode`, and `gemini`. It
checks `PATH` and common Homebrew, pnpm, Volta, Bun, and nvm installation
directories. Override executable paths with `QGIS_AGENT_TRAEX`,
`QGIS_AGENT_CODEX`, `QGIS_AGENT_CLAUDE`, `QGIS_AGENT_OPENCODE`,
`QGIS_AGENT_GEMINI`, and `QGIS_AGENT_PYTHON`. Provider homes can be overridden
with `QGIS_AGENT_TRAE_HOME` and `QGIS_AGENT_CODEX_HOME`.

For TraeX, the editable model dropdown defaults to `GPT-5.5` and passes the
selected or typed model through the app-server configuration. Choosing the
provider default omits the explicit model override and uses the active TraeX
config.

## Build

```sh
cmake --build build --target plugin_qgis_agent
```

The build output is written to the configured QGIS plugin directory, for
example:

```text
build/output/Contents/PlugIns/qgis/libplugin_qgis_agent.so
```
