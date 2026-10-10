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
        "name": "list_data_source_layers",
        "description": "Inspect a local data source and list loadable sublayers. Use this before loading GeoPackage or other multi-layer containers.",
        "inputSchema": {
            "type": "object",
            "properties": {"path": {"type": "string"}},
            "required": ["path"],
            "additionalProperties": False,
        },
    },
    {
        "name": "load_layer",
        "description": "Load one or more local vector or raster data sources into the active QGIS project. Supports Shapefile, GeoJSON, GeoPackage sublayers, GeoTIFF, VRT, IMG, ASC, and other installed providers.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "path": {"type": "string", "description": "One absolute local file path."},
                "paths": {
                    "type": "array",
                    "items": {"type": "string"},
                    "description": "Multiple absolute local file paths.",
                },
                "name": {"type": "string", "description": "Optional display name. Best used when loading one path."},
                "type": {
                    "type": "string",
                    "enum": ["auto", "vector", "raster"],
                    "default": "auto",
                },
                "provider": {"type": "string", "description": "Optional explicit QGIS provider key."},
                "sublayer": {"type": "string", "description": "Optional GeoPackage layer name."},
            },
            "additionalProperties": False,
        },
    },
    {
        "name": "export_layer",
        "description": "Export a vector or raster layer to an absolute file path. Existing outputs require overwrite=true and QGIS asks for confirmation.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "layer": {"type": "string"},
                "path": {"type": "string"},
                "format": {"type": "string", "description": "OGR or GDAL driver name; inferred from vector file extension when omitted."},
                "encoding": {"type": "string", "default": "UTF-8"},
                "selected_only": {"type": "boolean", "default": False},
                "overwrite": {"type": "boolean", "default": False},
            },
            "required": ["layer", "path"],
            "additionalProperties": False,
        },
    },
    {
        "name": "export_map_layout",
        "description": (
            "Export the current map canvas as an A4 print layout with optional title, legend, scale bar, north arrow, "
            "and note. Supports PDF, SVG, PNG, JPG/JPEG, TIF/TIFF, and BMP output paths. Existing outputs require "
            "overwrite=true and QGIS asks for confirmation."
        ),
        "inputSchema": {
            "type": "object",
            "properties": {
                "path": {"type": "string", "description": "Absolute output path ending in pdf, svg, png, jpg, jpeg, tif, tiff, or bmp."},
                "title": {"type": "string", "description": "Optional layout title. Defaults to project title or QGIS Map."},
                "note": {"type": "string", "description": "Optional bottom note. Defaults to CRS, scale, and export time."},
                "layout_name": {"type": "string", "description": "Temporary layout name used during export."},
                "orientation": {
                    "type": "string",
                    "enum": ["landscape", "portrait"],
                    "default": "landscape",
                },
                "layers": {
                    "type": "array",
                    "items": {"type": "string"},
                    "description": "Optional layer IDs or exact layer names. Defaults to the current map canvas layers.",
                },
                "scale": {"type": "number", "description": "Optional map scale denominator, such as 25000 for 1:25000. Defaults to the current canvas extent."},
                "dpi": {"type": "number", "minimum": 72, "maximum": 1200, "default": 300},
                "margin_mm": {"type": "number", "minimum": 0, "maximum": 40, "default": 10},
                "include_legend": {"type": "boolean", "default": True},
                "include_scale_bar": {"type": "boolean", "default": True},
                "include_north_arrow": {"type": "boolean", "default": True},
                "include_note": {"type": "boolean", "default": True},
                "filter_legend_by_map": {"type": "boolean", "default": True},
                "overwrite": {"type": "boolean", "default": False},
            },
            "required": ["path"],
            "additionalProperties": False,
        },
    },
    {
        "name": "save_project",
        "description": "Save the current QGIS project, or save it to a new absolute .qgz path. QGIS asks for confirmation.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "path": {"type": "string"},
                "overwrite": {"type": "boolean", "default": False},
            },
            "additionalProperties": False,
        },
    },
    {
        "name": "query_features",
        "description": "Read a bounded sample of vector features and attributes, optionally filtered by a QGIS expression or current selection.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "layer": {"type": "string"},
                "expression": {"type": "string"},
                "fields": {"type": "array", "items": {"type": "string"}},
                "limit": {"type": "integer", "minimum": 1, "maximum": 200, "default": 20},
                "selected_only": {"type": "boolean", "default": False},
                "include_geometry": {"type": "boolean", "default": False},
            },
            "required": ["layer"],
            "additionalProperties": False,
        },
    },
    {
        "name": "field_statistics",
        "description": "Inspect a vector field's type, range, null count, unique values, numeric mean and standard deviation, and z-score outlier candidates.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "layer": {"type": "string"},
                "field": {"type": "string"},
                "unique_limit": {"type": "integer", "minimum": 1, "maximum": 500, "default": 50},
                "outlier_z_score": {"type": "number", "minimum": 0, "default": 3},
                "outlier_limit": {"type": "integer", "minimum": 1, "maximum": 500, "default": 50},
            },
            "required": ["layer", "field"],
            "additionalProperties": False,
        },
    },
    {
        "name": "validate_expression",
        "description": "Validate a QGIS expression against a vector layer and return the number of matching features.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "layer": {"type": "string"},
                "expression": {"type": "string"},
            },
            "required": ["layer", "expression"],
            "additionalProperties": False,
        },
    },
    {
        "name": "select_features",
        "description": "Select vector features using a validated QGIS expression. The previous selection can be restored by withdrawing the turn.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "layer": {"type": "string"},
                "expression": {"type": "string"},
                "behavior": {
                    "type": "string",
                    "enum": ["replace", "add", "remove", "intersect"],
                    "default": "replace",
                },
            },
            "required": ["layer", "expression"],
            "additionalProperties": False,
        },
    },
    {
        "name": "clear_selection",
        "description": "Clear the selection of a vector layer. The previous selection can be restored by withdrawing the turn.",
        "inputSchema": {
            "type": "object",
            "properties": {"layer": {"type": "string"}},
            "required": ["layer"],
            "additionalProperties": False,
        },
    },
    {
        "name": "map_canvas_state",
        "description": "Read the visible map extent, map CRS, scale, and map units per pixel.",
        "inputSchema": {"type": "object", "properties": {}, "additionalProperties": False},
    },
    {
        "name": "zoom_to_layer",
        "description": "Zoom to a layer, its selected features, or features matching a QGIS expression. Withdrawing the turn restores the previous map extent.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "layer": {"type": "string"},
                "selected_only": {"type": "boolean", "default": False},
                "expression": {"type": "string"},
            },
            "required": ["layer"],
            "additionalProperties": False,
        },
    },
    {
        "name": "inspect_raster",
        "description": "Inspect raster dimensions, CRS, bands, data types, NoData values, and sampled statistics. Use this before DEM, DSM, NDVI, NDWI, or MNDWI analysis.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "layer": {"type": "string"},
                "sample_size": {
                    "type": "integer",
                    "minimum": 1000,
                    "maximum": 2000000,
                    "default": 250000,
                },
            },
            "required": ["layer"],
            "additionalProperties": False,
        },
    },
    {
        "name": "suggest_raster_threshold",
        "description": "Suggest an Otsu threshold for a raster band from a sampled histogram. Review the result before using it for water or land-cover classification.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "layer": {"type": "string"},
                "band": {"type": "integer", "minimum": 1, "default": 1},
                "bins": {
                    "type": "integer",
                    "minimum": 16,
                    "maximum": 4096,
                    "default": 256,
                },
                "sample_size": {
                    "type": "integer",
                    "minimum": 1000,
                    "maximum": 5000000,
                    "default": 500000,
                },
            },
            "required": ["layer"],
            "additionalProperties": False,
        },
    },
    {
        "name": "quality_check",
        "description": "Generate a vector data-quality report covering empty or invalid geometry, duplicate geometry, CRS mismatch, required fields, and null values.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "layer": {"type": "string"},
                "required_fields": {"type": "array", "items": {"type": "string"}},
                "issue_limit": {"type": "integer", "minimum": 1, "maximum": 1000, "default": 100},
                "feature_limit": {
                    "type": "integer",
                    "minimum": 1,
                    "maximum": 1000000,
                    "default": 100000,
                },
            },
            "required": ["layer"],
            "additionalProperties": False,
        },
    },
    {
        "name": "edit_vector_layer",
        "description": "Apply a confirmed edit transaction to a vector layer. Supports add, rename, or delete field; add feature; update attributes or geometry by expression; and delete features. Withdrawing the turn invokes the layer undo stack.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "layer": {"type": "string"},
                "action": {
                    "type": "string",
                    "enum": ["add_field", "rename_field", "delete_field", "add_feature", "update_attributes", "update_geometry", "delete_features"],
                },
                "field": {"type": "string"},
                "new_name": {"type": "string"},
                "field_type": {"type": "string"},
                "expression": {"type": "string"},
                "values": {"type": "object"},
                "attributes": {"type": "object"},
                "geometry_wkt": {"type": "string"},
                "reason": {"type": "string"},
            },
            "required": ["layer", "action"],
            "additionalProperties": False,
        },
    },
    {
        "name": "save_workflow",
        "description": "Save a reusable JSON workflow made of QGIS Processing algorithm steps.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "name": {"type": "string"},
                "steps": {
                    "type": "array",
                    "items": {
                        "type": "object",
                        "properties": {
                            "id": {"type": "string"},
                            "algorithm_id": {"type": "string"},
                            "parameters": {"type": "object"},
                        },
                        "required": ["algorithm_id", "parameters"],
                    },
                },
            },
            "required": ["name", "steps"],
            "additionalProperties": False,
        },
    },
    {
        "name": "list_workflows",
        "description": "List saved QGIS Agent Processing workflows.",
        "inputSchema": {"type": "object", "properties": {}, "additionalProperties": False},
    },
    {
        "name": "run_workflow",
        "description": "Run a saved or ad-hoc sequence of Processing algorithms. Use ${step_id.OUTPUT} to pass an earlier output to a later step, and start_step plus resume_results to resume from recorded outputs.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "name": {"type": "string"},
                "steps": {"type": "array", "items": {"type": "object"}},
                "start_step": {"type": "integer", "minimum": 0, "default": 0},
                "resume_results": {"type": "object"},
                "report_path": {"type": "string"},
            },
            "additionalProperties": False,
        },
    },
    {
        "name": "run_batch_workflow",
        "description": "Run the same Processing workflow for every matching file in a directory. Use ${input} for the current absolute input path and ${input_name}, ${input_stem}, ${input_dir}, or ${output_dir} inside string parameters.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "input_directory": {"type": "string"},
                "output_directory": {"type": "string"},
                "patterns": {
                    "type": "array",
                    "items": {"type": "string"},
                    "default": ["*"],
                },
                "max_files": {
                    "type": "integer",
                    "minimum": 1,
                    "maximum": 10000,
                    "default": 100,
                },
                "steps": {"type": "array", "items": {"type": "object"}},
                "report_path": {"type": "string"},
            },
            "required": ["input_directory", "steps"],
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
    {
        "name": "recent_diagnostics",
        "description": (
            "Read recent QGIS warnings and errors, QGIS Agent tool calls with their exact arguments and results, "
            "and Processing history. Use this first when the user pastes an error or asks why an operation failed."
        ),
        "inputSchema": {
            "type": "object",
            "properties": {
                "limit": {
                    "type": "integer",
                    "minimum": 1,
                    "maximum": 50,
                    "default": 15,
                },
                "since_minutes": {
                    "type": "integer",
                    "minimum": 1,
                    "maximum": 10080,
                    "default": 120,
                },
                "errors_only": {"type": "boolean", "default": True},
                "include_processing_history": {
                    "type": "boolean",
                    "default": True,
                },
            },
            "additionalProperties": False,
        },
    },
    {
        "name": "geocode_place",
        "description": (
            "Resolve a place name with OpenStreetMap Nominatim and return candidate coordinates plus bounding boxes "
            "formatted for query_overpass. Use this before downloading data for a named study area."
        ),
        "inputSchema": {
            "type": "object",
            "properties": {
                "query": {"type": "string", "minLength": 2, "maxLength": 300},
                "country_codes": {
                    "type": "string",
                    "description": "Optional comma-separated ISO 3166-1 alpha-2 country codes, such as cn or us,ca.",
                },
                "limit": {
                    "type": "integer",
                    "minimum": 1,
                    "maximum": 10,
                    "default": 5,
                },
            },
            "required": ["query"],
            "additionalProperties": False,
        },
    },
    {
        "name": "query_overpass",
        "description": (
            "Download bounded OpenStreetMap roads or POIs through the public Overpass API, convert the response to "
            "GeoJSON, save it locally, and load it into QGIS. Use separate center-mode queries for POIs and "
            "geometry-mode way queries for roads. The user confirms the download."
        ),
        "inputSchema": {
            "type": "object",
            "properties": {
                "bbox": {
                    "type": "array",
                    "items": {"type": "number"},
                    "minItems": 4,
                    "maxItems": 4,
                    "description": "Bounding box as [south, west, north, east] in WGS84.",
                },
                "selectors": {
                    "type": "array",
                    "items": {
                        "type": "string",
                        "description": 'Safe Overpass selector such as nwr["shop"="mall"] or way["highway"].',
                    },
                    "minItems": 1,
                    "maxItems": 20,
                },
                "geometry_mode": {
                    "type": "string",
                    "enum": ["center", "geometry"],
                    "default": "center",
                    "description": "center creates points suitable for POI heatmaps; geometry preserves ways for roads.",
                },
                "output_path": {
                    "type": "string",
                    "description": "Optional new absolute .geojson path. Defaults to Downloads/QGIS-Agent.",
                },
                "name": {"type": "string", "description": "QGIS layer name."},
                "timeout_seconds": {
                    "type": "integer",
                    "minimum": 10,
                    "maximum": 180,
                    "default": 90,
                },
            },
            "required": ["bbox", "selectors"],
            "additionalProperties": False,
        },
    },
    {
        "name": "search_stac",
        "description": (
            "Search a public STAC API for satellite imagery or DEM assets. This returns bounded metadata and public "
            "asset URLs without downloading them. Review collection licensing and use download_remote_file for a selected asset."
        ),
        "inputSchema": {
            "type": "object",
            "properties": {
                "api_url": {
                    "type": "string",
                    "default": "https://earth-search.aws.element84.com/v1",
                },
                "collections": {
                    "type": "array",
                    "items": {"type": "string"},
                    "minItems": 1,
                    "maxItems": 10,
                },
                "bbox": {
                    "type": "array",
                    "items": {"type": "number"},
                    "minItems": 4,
                    "maxItems": 4,
                    "description": "Bounding box as [west, south, east, north] in WGS84.",
                },
                "datetime": {
                    "type": "string",
                    "description": "STAC datetime or interval, e.g. 2025-01-01/2025-12-31.",
                },
                "query": {
                    "type": "object",
                    "description": 'Optional STAC query extension, e.g. {"eo:cloud_cover":{"lt":20}}.',
                },
                "asset_keys": {
                    "type": "array",
                    "items": {"type": "string"},
                    "description": "Optional asset keys to return, such as visual, red, nir, or dem.",
                },
                "limit": {
                    "type": "integer",
                    "minimum": 1,
                    "maximum": 100,
                    "default": 10,
                },
                "timeout_seconds": {
                    "type": "integer",
                    "minimum": 10,
                    "maximum": 180,
                    "default": 60,
                },
            },
            "required": ["collections", "bbox"],
            "additionalProperties": False,
        },
    },
    {
        "name": "download_remote_file",
        "description": (
            "Download one selected public HTTP/HTTPS geospatial asset to a new local file and optionally load it in QGIS. "
            "Local/private network hosts, embedded credentials, overwrites, unsupported extensions, and oversized responses are rejected. "
            "The user confirms before downloading."
        ),
        "inputSchema": {
            "type": "object",
            "properties": {
                "url": {"type": "string"},
                "output_path": {"type": "string", "description": "New absolute local output path."},
                "name": {"type": "string", "description": "Optional QGIS layer name."},
                "type": {
                    "type": "string",
                    "enum": ["auto", "vector", "raster"],
                    "default": "auto",
                },
                "load": {"type": "boolean", "default": True},
                "max_size_mb": {
                    "type": "integer",
                    "minimum": 1,
                    "maximum": 512,
                    "default": 100,
                },
                "timeout_seconds": {
                    "type": "integer",
                    "minimum": 10,
                    "maximum": 600,
                    "default": 120,
                },
                "sha256": {
                    "type": "string",
                    "description": "Optional expected SHA-256 checksum.",
                },
            },
            "required": ["url"],
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
