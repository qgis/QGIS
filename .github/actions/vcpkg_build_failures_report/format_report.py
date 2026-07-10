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
        data = json.loads(diff_report_path.read_text(encoding="utf-8"))
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
    per_port_artifact_url: str | None = None,
    inline_logs_url: str | None = None,
    run_url: str | None = None,
    build_status: str = "",
    commit_sha: str = "",
    commit_url: str = "",
) -> str:
    changed_ports = changed_ports or set()
    emoji = _platform_emoji(triplet)
    lines: list[str] = []

    if not failures:
        if build_status.lower() == "failure":
            lines.append(
                f"### {emoji} `{triplet}` &mdash; \u26a0\ufe0f vcpkg failed before producing per-port logs"
            )
            lines.append("")
            lines.append(
                "vcpkg returned a non-zero exit code but no port left a parseable buildtree. "
                "This usually means a portfile-execute abort (e.g. `find_library` REQUIRED, "
                "`vcpkg_extract_source_archive` download failure), or a vcpkg-internal error "
                "such as a missing tool or registry/baseline mismatch."
            )
            lines.append("")
            lines.append("Look at the artifacts and the workflow log:")
            lines.append("")
        else:
            lines.append(f"### {emoji} `{triplet}` &mdash; \u2705 all ports built")
            return "\n".join(lines) + "\n"
    else:
        lines.append(
            f"### {emoji} `{triplet}` &mdash; \u274c {len(failures)} port(s) failed"
        )
        lines.append("")

        failed_ports = ", ".join(f"`{f['port']}`" for f in failures)
        lines.append(f"**Failed:** {failed_ports}")
        lines.append("")

        for failure in failures:
            port = failure["port"]
            marker = (
                " &nbsp;\U0001f535 _updated in this PR_"
                if port in changed_ports
                else " &nbsp;\u26aa _unchanged in this PR_"
                if changed_ports
                else ""
            )
            lines.append("<details>")
            lines.append(f"<summary>\U0001f4e6 <code>{port}</code>{marker}</summary>")
            lines.append("")
            for excerpt in failure.get("excerpts", []):
                lines.extend(_render_excerpt(excerpt["source"], excerpt["lines"]))
                lines.append("")
            lines.append("</details>")
            lines.append("")

    footer: list[str] = []
    if inline_logs_url:
        footer.append(f"📜 [View inline logs in workflow]({inline_logs_url})")
    if per_port_artifact_url:
        footer.append(
            f"📦 [Download `failing-port-logs-{triplet}` artifact]({per_port_artifact_url})"
        )
    if artifact_url:
        footer.append(
            f"📑 [Download full `build-logs-{triplet}` artifact]({artifact_url})"
        )
    if run_url:
        footer.append(f"🔧 [Workflow run]({run_url})")
    if commit_sha:
        short = commit_sha[:7]
        if commit_url:
            footer.append(f"📌 [Commit `{short}`]({commit_url})")
        else:
            footer.append(f"📌 Commit `{short}`")
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
        "--per-port-artifact-url",
        default=None,
        help="Optional URL to the failing-port-logs-<triplet> artifact "
        "(focused per-port log subset).",
    )
    parser.add_argument(
        "--inline-logs-url",
        default=None,
        help="Optional deep link to the workflow step that echoed each "
        "failing port's logs inline (::group:: blocks).",
    )
    parser.add_argument(
        "--run-url",
        default=None,
        help="Optional URL to the GitHub Actions run.",
    )
    parser.add_argument(
        "--build-status",
        default="",
        help="Optional outcome of the upstream build/configure step "
        "('success' or 'failure'). When 'failure' and no per-port failures "
        "were parsed, render a 'vcpkg failed before producing per-port logs' "
        "banner instead of the misleading 'all ports built' line.",
    )
    parser.add_argument(
        "--commit-sha",
        default="",
        help="Optional commit SHA the report is for; surfaced in the footer.",
    )
    parser.add_argument(
        "--commit-url",
        default="",
        help="Optional URL the commit SHA links to.",
    )
    parser.add_argument(
        "-o",
        "--output",
        type=Path,
        default=None,
        help="Write markdown to this file (default: stdout).",
    )
    args = parser.parse_args()

    failures = json.loads(args.failures_json.read_text(encoding="utf-8"))
    changed = _load_diff_changed_ports(args.diff_report)
    markdown = render(
        failures,
        args.triplet,
        changed_ports=changed,
        artifact_url=args.artifact_url,
        per_port_artifact_url=args.per_port_artifact_url,
        inline_logs_url=args.inline_logs_url,
        run_url=args.run_url,
        build_status=args.build_status,
        commit_sha=args.commit_sha,
        commit_url=args.commit_url,
    )

    if args.output:
        args.output.write_text(markdown, encoding="utf-8")
    else:
        sys.stdout.write(markdown)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
