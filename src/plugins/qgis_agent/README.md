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
- `list_data_source_layers`
- `load_layer`
- `export_layer`
- `export_map_layout`
- `save_project`
- `query_features`
- `field_statistics`
- `validate_expression`
- `select_features`
- `clear_selection`
- `map_canvas_state`
- `zoom_to_layer`
- `inspect_raster`
- `suggest_raster_threshold`
- `quality_check`
- `edit_vector_layer`
- `save_workflow`
- `list_workflows`
- `run_workflow`
- `run_batch_workflow`
- `list_processing_algorithms`
- `run_processing_algorithm`
- `recent_diagnostics`
- `geocode_place`
- `query_overpass`
- `search_stac`
- `download_remote_file`

Processing output destinations default to temporary outputs and resulting map
layers are added to the active project. Write operations require confirmation
in QGIS unless the confirmation checkbox is disabled. Saved workflows can chain
Processing outputs, resume from a supplied step and prior results, or run the
same workflow over matching files in a directory.
Map layout exports can create an A4 map sheet from the current canvas and write
PDF, SVG, or raster image output with optional title, legend, scale bar, north
arrow, and note. Existing output files require explicit overwrite confirmation.
Every user turn has a withdraw icon. Withdrawing a turn hides that turn's
question, answer, plan, and tool transcript. If the turn loaded Processing
output layers, withdrawing also removes those layers from the active project;
operations outside this recorded layer-loading path are not rolled back.

Greetings and general questions use the same Agent thread as GIS work. There is
no UI-side intent classifier: the Agent decides whether a tool call is needed.
When a user pastes a QGIS or Processing error, the Agent can inspect recent
QGIS Message Log entries, message-bar errors, Agent tool arguments/results, and
Processing history. It then diagnoses the failure, applies the smallest safe
parameter or workflow correction, retries when possible, and validates the
result. Diagnostic capture begins when the plugin is loaded.
Online acquisition is limited to explicit GIS tools instead of giving the
model unrestricted network or shell access. Place names can be resolved with
Nominatim, bounded roads and POIs can be downloaded through Overpass, and
imagery/DEM assets can be discovered through public STAC APIs. File downloads
reject local/private hosts, embedded URL credentials, unsupported geospatial
extensions, existing output paths, oversized responses, and unsafe redirects.
QGIS asks for confirmation before any remote data is written. Downloaded files
and layers created by a turn are removed when that conversation is withdrawn.
The Codex process runs in a dedicated temporary workspace with a read-only turn
sandbox. QGIS remains the authority for GIS mutations and confirmation.

## Requirements

- A working login for at least one supported CLI
- Python 3 for the MCP bridge
- QGIS built with desktop and GUI support

The plugin detects `traex`, `codex`, `claude`, `opencode`, and `gemini`. It
checks `PATH` and common Homebrew, pnpm, Volta, Bun, and nvm installation
directories. On Windows it also checks npm, WinGet, Scoop, Chocolatey, pnpm,
Volta, Bun, nvm, Program Files, and WindowsApps locations, including
`.exe`, `.cmd`, and `.bat` launchers. It prefers the QGIS/OSGeo4W bundled
Python and also checks virtual environments, Conda, pyenv-win, and user Python
installations. Override executable paths with `QGIS_AGENT_TRAEX`,
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
