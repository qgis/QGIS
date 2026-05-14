#!/usr/bin/env python3
"""
Bump vcpkg registry baselines in a vcpkg.json manifest to the latest HEAD commits.

Outputs (via $GITHUB_OUTPUT when available):
  updated=true/false
  summary=<markdown summary of changes>
"""

import argparse
import json
import os
import subprocess
import sys


def get_git_registries(manifest_path):
    """Extract all git registries from vcpkg.json (default + additional)."""
    with open(manifest_path) as f:
        data = json.load(f)

    cfg = data.get("vcpkg-configuration", {})
    registries = []

    default = cfg.get("default-registry", {})
    if default.get("kind") == "git":
        registries.append(default)

    for reg in cfg.get("registries", []):
        if reg.get("kind") == "git":
            registries.append(reg)

    return registries


def resolve_head(repo_url):
    """Resolve the latest HEAD commit SHA for a remote git repository."""
    result = subprocess.run(
        ["git", "ls-remote", repo_url, "HEAD"],
        capture_output=True,
        text=True,
        check=True,
    )
    return result.stdout.split()[0] if result.stdout.strip() else None


def update_baseline(manifest_path, old_baseline, new_baseline):
    """Replace a baseline SHA in the manifest file."""
    with open(manifest_path) as f:
        content = f.read()

    content = content.replace(old_baseline, new_baseline, 1)

    with open(manifest_path, "w") as f:
        f.write(content)


def main():
    parser = argparse.ArgumentParser(
        description="Bump vcpkg registry baselines to latest HEAD commits."
    )
    parser.add_argument(
        "manifest",
        nargs="?",
        default="vcpkg/vcpkg.json",
        help="Path to vcpkg.json manifest (default: vcpkg/vcpkg.json)",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Only print what would be updated, don't modify files",
    )
    parser.add_argument(
        "--github-output",
        help="Path to GitHub Actions output file (default: $GITHUB_OUTPUT)",
    )
    args = parser.parse_args()

    manifest_path = args.manifest

    registries = get_git_registries(manifest_path)
    if not registries:
        print("No git registries found in", manifest_path)
        sys.exit(0)

    updated = False
    summary_lines = []

    for reg in registries:
        repo_url = reg["repository"]
        old_baseline = reg["baseline"]
        repo_name = repo_url.rstrip("/").rsplit("/", 1)[-1]

        new_baseline = resolve_head(repo_url)
        if not new_baseline:
            print(f"::warning::Could not resolve HEAD for {repo_url}")
            continue

        if old_baseline == new_baseline:
            print(f"{repo_name} is already up to date ({old_baseline})")
            continue

        print(f"Updating {repo_name}: {old_baseline} -> {new_baseline}")
        if not args.dry_run:
            update_baseline(manifest_path, old_baseline, new_baseline)
        summary_lines.append(
            f"- **{repo_name}**: `{old_baseline[:12]}` → `{new_baseline[:12]}`"
        )
        updated = True

    # Write outputs for GitHub Actions
    github_output = args.github_output or os.environ.get("GITHUB_OUTPUT")
    if github_output:
        with open(github_output, "a") as f:
            f.write(f"updated={str(updated).lower()}\n")
            f.write("summary<<EOF\n")
            f.write("\n".join(summary_lines) + "\n")
            f.write("EOF\n")
    else:
        print(f"\nupdated={updated}")
        if summary_lines:
            print("\n".join(summary_lines))


if __name__ == "__main__":
    main()
