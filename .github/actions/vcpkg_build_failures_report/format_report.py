#!/usr/bin/env python3
"""
Render a markdown report from ``parse_failures.py`` JSON output.

The same markdown is suitable for both ``$GITHUB_STEP_SUMMARY`` and a sticky
PR comment. Each failing port is rendered as a collapsible ``<details>``
block with fenced excerpts of the relevant logs.

Optional diff-report JSON (``--diff-report``) lets us tag each failing port
as updated/unchanged in the current PR (cross-reference enhancement).
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

_PLATFORM_EMOJI = {
    "x64-windows": "🪟",
    "x64-windows-release": "🪟",
    "x86-windows": "🪟",
    "x64-linux": "🐧",
    "arm64-osx": "🍎",
    "arm64-osx-dynamic-release": "🍎",
    "x64-osx": "🍎",
}


def _platform_emoji(triplet: str) -> str:
    return _PLATFORM_EMOJI.get(triplet, "💻")


def _load_diff_changed_ports(diff_report_path: Path | None) -> set[str]:
    """Return the set of port names changed (added or updated) by the PR.

    Accepts a JSON file shaped like ``{"added": [...], "updated": [...]}``
    where each list contains port names (strings). Removed ports are not
    interesting for failure cross-ref. Missing/invalid file → empty set.
    """
    if diff_report_path is None or not diff_report_path.exists():
        return set()
    try:
        data = json.loads(diff_report_path.read_text())
    except (OSError, json.JSONDecodeError):
        return set()
    changed: set[str] = set()
    for key in ("added", "updated"):
        for entry in data.get(key, []):
            if isinstance(entry, str):
                changed.add(entry.split(":", 1)[0].strip())
    return changed


def _render_excerpt(source: str, lines: list[str]) -> list[str]:
    out = [f"<sub>from <code>{source}</code></sub>", "", "```"]
    out.extend(lines)
    out.append("```")
    return out


def render(
    failures: list[dict],
    triplet: str,
    *,
    changed_ports: set[str] | None = None,
    artifact_url: str | None = None,
    run_url: str | None = None,
) -> str:
    changed_ports = changed_ports or set()
    emoji = _platform_emoji(triplet)
    lines: list[str] = []

    if not failures:
        lines.append(f"### {emoji} `{triplet}` &mdash; ✅ all ports built")
        return "\n".join(lines) + "\n"

    lines.append(f"### {emoji} `{triplet}` &mdash; ❌ {len(failures)} port(s) failed")
    lines.append("")

    failed_ports = ", ".join(f"`{f['port']}`" for f in failures)
    lines.append(f"**Failed:** {failed_ports}")
    lines.append("")

    for failure in failures:
        port = failure["port"]
        marker = (
            " &nbsp;🔵 _updated in this PR_"
            if port in changed_ports
            else " &nbsp;⚪ _unchanged in this PR_"
            if changed_ports
            else ""
        )
        lines.append("<details>")
        lines.append(f"<summary>📦 <code>{port}</code>{marker}</summary>")
        lines.append("")
        for excerpt in failure.get("excerpts", []):
            lines.extend(_render_excerpt(excerpt["source"], excerpt["lines"]))
            lines.append("")
        lines.append("</details>")
        lines.append("")

    footer: list[str] = []
    if artifact_url:
        footer.append(
            f"📑 [Download full `build-logs-{triplet}` artifact]({artifact_url})"
        )
    if run_url:
        footer.append(f"🔧 [Workflow run]({run_url})")
    if footer:
        lines.append(" &middot; ".join(footer))

    return "\n".join(lines) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "failures_json",
        type=Path,
        help="JSON file produced by parse_failures.py.",
    )
    parser.add_argument(
        "--triplet",
        required=True,
        help="Triplet label (used in heading and emoji selection).",
    )
    parser.add_argument(
        "--diff-report",
        type=Path,
        default=None,
        help="Optional JSON file with added/updated port lists for "
        "cross-referencing failing ports against PR changes.",
    )
    parser.add_argument(
        "--artifact-url",
        default=None,
        help="Optional URL to the uploaded build-logs artifact.",
    )
    parser.add_argument(
        "--run-url",
        default=None,
        help="Optional URL to the GitHub Actions run.",
    )
    parser.add_argument(
        "-o",
        "--output",
        type=Path,
        default=None,
        help="Write markdown to this file (default: stdout).",
    )
    args = parser.parse_args()

    failures = json.loads(args.failures_json.read_text())
    changed = _load_diff_changed_ports(args.diff_report)
    markdown = render(
        failures,
        args.triplet,
        changed_ports=changed,
        artifact_url=args.artifact_url,
        run_url=args.run_url,
    )

    if args.output:
        args.output.write_text(markdown)
    else:
        sys.stdout.write(markdown)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
