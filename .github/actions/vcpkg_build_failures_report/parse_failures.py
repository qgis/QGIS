#!/usr/bin/env python3
"""
Scan a vcpkg buildtrees directory for failed port builds.

vcpkg writes a per-port log directory under ``buildtrees/<port>/`` containing
many ``*-out.log`` / ``*-err.log`` pairs (one per build step: extract, patch,
configure, build, install...). Most of these exist for every port whether it
succeeds or fails. The reliable failure signal is the per-port orchestrator
log ``stdout-<triplet>.log`` containing both ``CMake Error`` and
``Command failed:`` produced by ``vcpkg_execute_required_process``.

This script:
  - Walks every port directory.
  - Identifies failing ports via the orchestrator log signature.
  - Extracts the ``See logs for more information:`` pointers and reads tails
    of the referenced detailed logs.
  - Emits JSON to stdout describing each failure.
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

# Signature of a real port-build failure emitted by
# scripts/cmake/vcpkg_execute_required_process.cmake. Matching both strings
# avoids false positives from informational "error" tokens in unrelated logs.
_CMAKE_ERROR_RE = re.compile(r"^CMake Error at .*?:\d+ \(message\):", re.MULTILINE)
_COMMAND_FAILED_RE = re.compile(r"^\s*Command failed:", re.MULTILINE)

# Captures the path on the line *after* "See logs for more information:".
_LOG_REF_RE = re.compile(
    r"See logs for more information:\s*\n\s*(\S+\.log)", re.MULTILINE
)

# Top-level error in vcpkg-manifest-install.log when a portfile aborts before
# any buildtree is produced (e.g. CMake Error at portfile.cmake / find_library
# REQUIRED, vcpkg_extract_source_archive failures, fail_port_install). Captures
# port name and triplet.
_MANIFEST_BUILD_FAILED_RE = re.compile(
    r"^error: building (?P<port>[A-Za-z0-9._+\-]+):(?P<triplet>[A-Za-z0-9._+\-]+) failed with: BUILD_FAILED\s*$",
    re.MULTILINE,
)
# Match a CMake Error block whose call-stack mentions a specific port's
# portfile, so we can attribute portfile-execute aborts to the right port even
# when no BUILD_FAILED line follows (e.g. early abort before vcpkg's reporter).
_MANIFEST_PORTFILE_ERROR_RE = re.compile(
    r"CMake Error at (?:[^\n]*?[/\\])?ports[/\\](?P<port>[A-Za-z0-9._+\-]+)[/\\]portfile\.cmake:\d+",
)

# The CMake Error block runs until the "Call Stack" footer or EOF.
_ERROR_BLOCK_RE = re.compile(
    r"(CMake Error at [^\n]+\(message\):.*?)(?=\n\nCall Stack|\Z)",
    re.DOTALL,
)

# Maximum lines kept per excerpt source. Keeps the resulting comment short
# enough to be readable while still surfacing the actual error.
_EXCERPT_TAIL_LINES = 40

# Lines we want to surface as "key" errors when present, regardless of where
# they appear in the log. Excludes plain "warning"-only lines to avoid
# drowning real errors in MSBuild D9025 noise.
_KEY_ERROR_RE = re.compile(
    r"(?i)\b(error|fatal|failed|undefined reference|cannot (?:find|open))\b"
)
_NOISE_RE = re.compile(r"(?i)\bwarning\b")

# Max key lines kept per source.
_KEY_LINES_MAX = 15


def _read(path: Path) -> str:
    try:
        return path.read_text(encoding="utf-8", errors="replace")
    except OSError as exc:
        return f"<<could not read {path}: {exc}>>"


def _extract_cmake_error_block(stdout_text: str) -> list[str]:
    match = _ERROR_BLOCK_RE.search(stdout_text)
    if not match:
        return []
    lines = match.group(1).strip().splitlines()
    return lines[:_EXCERPT_TAIL_LINES]


def _tail(text: str, n: int = _EXCERPT_TAIL_LINES) -> list[str]:
    lines = text.splitlines()
    return lines[-n:] if len(lines) > n else lines


def _key_error_lines(text: str, n: int = _KEY_LINES_MAX) -> list[str]:
    """Return up to ``n`` representative error lines from ``text``.

    Filters lines matching :data:`_KEY_ERROR_RE` while skipping lines that
    are pure warnings (MSBuild's repeated ``D9025`` warnings are the main
    offender). Returns the *last* ``n`` matches so the final / decisive
    error is always included.
    """
    matches: list[str] = []
    for line in text.splitlines():
        if _KEY_ERROR_RE.search(line) and not _NOISE_RE.search(line):
            matches.append(line.rstrip())
    if not matches:
        return []
    return matches[-n:]


def _parse_port(port_dir: Path) -> dict | None:
    """Return a failure record for ``port_dir`` or ``None`` if it didn't fail."""
    stdout_logs = sorted(port_dir.glob("stdout-*.log"))
    if not stdout_logs:
        return None

    for stdout_log in stdout_logs:
        text = _read(stdout_log)
        if not (_CMAKE_ERROR_RE.search(text) and _COMMAND_FAILED_RE.search(text)):
            continue

        triplet = stdout_log.stem.removeprefix("stdout-")

        excerpts: list[dict] = []
        cmake_block = _extract_cmake_error_block(text)
        if cmake_block:
            excerpts.append({"source": stdout_log.name, "lines": cmake_block})

        referenced: list[str] = []
        for ref in _LOG_REF_RE.findall(text):
            # vcpkg prints absolute paths; only the basename is reliably
            # present in the uploaded artifact.
            ref_name = Path(ref.replace("\\", "/")).name
            if ref_name in referenced:
                continue
            referenced.append(ref_name)
            ref_file = port_dir / ref_name
            if not ref_file.exists():
                continue
            ref_text = _read(ref_file)
            key_lines = _key_error_lines(ref_text)
            if key_lines:
                excerpts.append(
                    {
                        "source": f"{ref_name} (key errors)",
                        "lines": key_lines,
                    }
                )
            else:
                # No clear error markers — fall back to the raw tail so the
                # user still sees something actionable.
                excerpts.append(
                    {
                        "source": f"{ref_name} (tail)",
                        "lines": _tail(ref_text),
                    }
                )

        return {
            "port": port_dir.name,
            "triplet": triplet,
            "stdout_log": stdout_log.name,
            "referenced_logs": referenced,
            "excerpts": excerpts,
        }
    return None


def parse_buildtrees(buildtrees: Path) -> list[dict]:
    if not buildtrees.is_dir():
        # Treat a missing buildtrees directory as "no failures" rather than
        # an error: the action runs on both success and failure paths, and
        # successful builds may have a cleaned / absent buildtrees tree.
        return []

    failures: list[dict] = []
    for port_dir in sorted(buildtrees.iterdir()):
        if not port_dir.is_dir():
            continue
        record = _parse_port(port_dir)
        if record is not None:
            failures.append(record)
    return failures


def _extract_block_around(
    text: str, start: int, *, before: int = 5, after: int = 35
) -> list[str]:
    """Return up to ``before+after`` lines around the byte offset ``start``."""
    lines = text.splitlines()
    # Translate byte offset to line index.
    consumed = 0
    line_idx = 0
    for i, line in enumerate(lines):
        consumed += len(line) + 1  # +1 for newline
        if consumed > start:
            line_idx = i
            break
    lo = max(0, line_idx - before)
    hi = min(len(lines), line_idx + after)
    return [ln.rstrip() for ln in lines[lo:hi]]


def parse_manifest_install_log(log_path: Path, *, known_ports: set[str]) -> list[dict]:
    """Synthesize failure records from ``vcpkg-manifest-install.log``.

    Used to surface portfile-execute aborts (e.g. ``find_library`` REQUIRED
    failures, ``vcpkg_extract_source_archive`` failures) that abort before
    vcpkg ever creates a buildtree directory, so the regular buildtrees scan
    finds nothing. Skips ports already represented in ``known_ports`` to
    avoid duplicating the buildtrees-based record.
    """
    if not log_path.is_file():
        return []
    text = _read(log_path)

    # Map port -> (triplet, anchor_offset) from BUILD_FAILED lines.
    failed: dict[str, tuple[str, int]] = {}
    for m in _MANIFEST_BUILD_FAILED_RE.finditer(text):
        port = m.group("port")
        if port in known_ports or port in failed:
            continue
        # Anchor at the *first* portfile-error preceding this line, so the
        # excerpt centers on the actionable CMake Error rather than on the
        # generic "building X failed" footer.
        anchor = m.start()
        for em in _MANIFEST_PORTFILE_ERROR_RE.finditer(text, 0, m.start()):
            if em.group("port") == port:
                anchor = em.start()
        failed[port] = (m.group("triplet"), anchor)

    # Also catch portfile-error blocks without a trailing BUILD_FAILED line
    # (rare, but possible if vcpkg itself crashed before its own reporter
    # ran). Use "unknown" triplet as we can't infer it from the block alone.
    for em in _MANIFEST_PORTFILE_ERROR_RE.finditer(text):
        port = em.group("port")
        if port in known_ports or port in failed:
            continue
        failed[port] = ("", em.start())

    out: list[dict] = []
    for port, (triplet, anchor) in failed.items():
        excerpt_lines = _extract_block_around(text, anchor)
        out.append(
            {
                "port": port,
                "triplet": triplet,
                "stdout_log": log_path.name,
                "referenced_logs": [],
                "excerpts": [
                    {
                        "source": f"{log_path.name} (portfile-execute abort)",
                        "lines": excerpt_lines,
                    }
                ],
                "source": "manifest-install-log",
            }
        )
    return out


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "buildtrees",
        type=Path,
        help="Path to the vcpkg buildtrees directory (or an extracted "
        "build-logs-<triplet> artifact root).",
    )
    parser.add_argument(
        "-o",
        "--output",
        type=Path,
        default=None,
        help="Write JSON output to this file (default: stdout).",
    )
    parser.add_argument(
        "--manifest-install-log",
        type=Path,
        default=None,
        help="Optional path to vcpkg-manifest-install.log. Used to surface "
        "portfile-execute aborts (e.g. CMake Error in portfile.cmake) that "
        "abort before vcpkg creates a buildtree directory.",
    )
    args = parser.parse_args()

    failures = parse_buildtrees(args.buildtrees)
    if args.manifest_install_log:
        known = {f["port"] for f in failures}
        failures.extend(
            parse_manifest_install_log(args.manifest_install_log, known_ports=known)
        )
    payload = json.dumps(failures, indent=2)
    if args.output:
        args.output.write_text(payload, encoding="utf-8")
    else:
        sys.stdout.write(payload + "\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
