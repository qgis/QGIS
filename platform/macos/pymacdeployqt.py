#!/usr/bin/env python3

import argparse
import os
import shutil
import subprocess

from dataclasses import dataclass
from functools import lru_cache
from pathlib import Path
from typing import Dict, List, Set, Tuple

# System paths that should be excluded from copying
SYSTEM_PATHS = [
    "/usr/lib",
    "/System/Library",
    "/Library/Frameworks",
]


@dataclass
class Library:
    path: str
    install_name: str
    dependencies: list[str]
    rpaths: list[str]


@lru_cache(maxsize=None)
def get_macho_info(path: str) -> bytes:
    """Run otool -l and cache the output."""
    result = subprocess.run(["otool", "-l", path], capture_output=True, check=True)
    return result.stdout


def parse_macho_info(path: str) -> Library:
    """Parse otool -l output to extract all needed information."""
    output = get_macho_info(path).decode("utf-8").split("\n")

    install_name = path
    rpaths = []
    dependencies = []

    i = 0
    while i < len(output):
        line = output[i].strip()

        # Look for load command type
        if "cmd LC_" not in line:
            i += 1
            continue

        cmd_type = line.split()[-1]

        if cmd_type == "LC_ID_DYLIB":
            # Next line is cmdsize, name is in the line after
            if i + 2 < len(output):
                name_line = output[i + 2].strip()
                if name_line.startswith("name"):
                    install_name = name_line.split()[1]

        elif cmd_type == "LC_LOAD_DYLIB" or cmd_type == "LC_LOAD_WEAK_DYLIB":
            # Next line is cmdsize, name is in the line after
            if i + 2 < len(output):
                name_line = output[i + 2].strip()
                if name_line.startswith("name"):
                    dep_path = name_line.split()[1]
                    if dep_path != install_name:
                        dependencies.append(dep_path)

        elif cmd_type == "LC_RPATH":
            # Next line is cmdsize, path is in the line after
            if i + 2 < len(output):
                path_line = output[i + 2].strip()
                if path_line.startswith("path"):
                    rpaths.append(path_line.split()[1])

        i += 1

    return Library(path, install_name, dependencies, rpaths)


def is_system_path(path: str) -> bool:
    """Check if the path is a system path that should be excluded."""
    return any(path.startswith(sys_path) for sys_path in SYSTEM_PATHS)


def find_library(lib_name: str, search_paths: list[str]) -> str:
    """Find library in search paths."""
    for path in search_paths:
        full_path = os.path.join(path, lib_name)
        if os.path.exists(full_path):
            return full_path
    return ""


def resolve_at_path(dep_path: str, binary_path: str, rpaths: list[str]) -> str:
    """
    Resolve a path that starts with @rpath, @executable_path, or @loader_path
    Returns resolved absolute path or empty string if not found
    """
    if dep_path.startswith("@rpath/"):
        lib_name = dep_path[len("@rpath/") :]
        # Try all rpaths
        for rpath in rpaths:
            # Handle nested @ paths in rpaths
            if rpath.startswith("@"):
                rpath = resolve_at_path(rpath, binary_path, [])
                if not rpath:
                    continue
            full_path = os.path.join(rpath, lib_name)
            if os.path.exists(full_path):
                return full_path
    elif dep_path.startswith("@executable_path/"):
        lib_name = dep_path[len("@executable_path/") :]
        exe_dir = os.path.dirname(binary_path)
        full_path = os.path.join(exe_dir, lib_name)
        if os.path.exists(full_path):
            return full_path
    elif dep_path.startswith("@loader_path/"):
        lib_name = dep_path[len("@loader_path/") :]
        loader_dir = os.path.dirname(binary_path)
        full_path = os.path.join(loader_dir, lib_name)
        if os.path.exists(full_path):
            return full_path
    return ""


def collect_dependencies(
    binary_path: str, lib_dirs: list[str], processed: set[str]
) -> dict[str, Library]:
    """Recursively collect all dependencies for a binary."""
    result = {}
    search_paths = lib_dirs.copy()

    def process_binary(path: str) -> None:
        if path in processed:
            return

        processed.add(path)
        real_path, _ = resolve_symlink(path)
        lib_info = parse_macho_info(real_path)
        result[path] = lib_info

        # Add library's directory to search paths if it's not a system path
        lib_dir = os.path.dirname(real_path)
        if lib_dir not in search_paths and not is_system_path(lib_dir):
            search_paths.append(lib_dir)

        # Process dependencies
        for dep in lib_info.dependencies:
            if dep.startswith("@"):
                # If we couldn't resolve it earlier, try again with updated search paths
                resolved_path = resolve_at_path(dep, path, lib_info.rpaths)
                if resolved_path:
                    dep = resolved_path

            if not os.path.isabs(dep):
                dep = find_library(os.path.basename(dep), search_paths)

            if dep and os.path.exists(dep):
                process_binary(dep)

    process_binary(binary_path)
    return result


def resolve_symlink(path: str) -> tuple[str, list[str]]:
    """
    Resolve a symlink chain to its final destination and return the real file path
    along with the chain of symlinks that led to it.
    """
    symlink_chain = []
    current_path = path

    while os.path.islink(current_path):
        symlink_chain.append(os.path.basename(current_path))
        current_path = os.path.realpath(current_path)

    return current_path, symlink_chain


def is_macho(filepath: str) -> bool:
    """
    Checks if a file is a Mach-O binary by reading the first 4 bytes.

    Args:
        filepath: Path to the file to check

    Returns:
        True if it is a Mach-O file
    """
    # Mach-O magic numbers
    MAGIC_64 = 0xCFFAEDFE  # 64-bit mach-o
    MAGIC_32 = 0xCEFAEDFE  # 32-bit mach-o

    try:
        # Open file in binary mode and read first 4 bytes
        with open(filepath, "rb") as f:
            magic = int.from_bytes(f.read(4), byteorder="big")

        if magic in (MAGIC_64, MAGIC_32):
            return True
        else:
            return False

    except OSError:
        return False


def handle_resources_binaries(app_bundle: str) -> None:
    """
    Move Mach-O files from Contents/Resources to Contents/PlugIns/_Resources
    and replace them with symlinks.
    """
    resources_dir = os.path.join(app_bundle, "Contents", "Resources")
    plugins_resources_dir = os.path.join(
        app_bundle, "Contents", "PlugIns", "_Resources"
    )

    if not os.path.exists(resources_dir):
        return

    # Find all Mach-O files in Resources
    for root, _, files in os.walk(resources_dir):
        for file in files:
            path = os.path.join(root, file)
            try:
                if is_macho(file):
                    # Calculate relative path from Resources root
                    rel_path = os.path.relpath(path, resources_dir)
                    new_path = os.path.join(plugins_resources_dir, rel_path)

                    # Create directory structure in PlugIns/_Resources
                    os.makedirs(os.path.dirname(new_path), exist_ok=True)

                    # Move the file and create symlink
                    shutil.move(path, new_path)
                    relative_target = os.path.relpath(new_path, os.path.dirname(path))
                    os.symlink(relative_target, path)
            except subprocess.CalledProcessError:
                continue


def deploy_libraries(app_bundle: str, lib_dirs: list[str]) -> None:
    """Deploy all libraries to the app bundle."""
    frameworks_dir = os.path.join(app_bundle, "Contents", "Frameworks")
    os.makedirs(frameworks_dir, exist_ok=True)

    print("Handle resources binaries")
    # Handle Resources binaries first
    handle_resources_binaries(app_bundle)

    print("Handle main binaries")
    # Find all binaries in the app bundle
    binaries = []
    for root, _, files in os.walk(app_bundle):
        for file in files:
            path = os.path.join(root, file)
            try:
                if not os.path.islink(path) and is_macho(path):
                    binaries.append(path)
            except subprocess.CalledProcessError:
                continue

    processed_libs = set()
    all_dependencies = {}

    # Collect all dependencies
    for binary in binaries:
        print(f"Analyzing {binary}")
        deps = collect_dependencies(binary, lib_dirs, processed_libs)
        all_dependencies.update(deps)
        # Copy libraries and prepare install_name_tool commands
    commands = {}  # path -> list of changes
    lib_mapping = {}  # old_install_name -> new_install_name

    # First pass: copy libraries and record their new install names
    for lib_path, lib_info in all_dependencies.items():
        if lib_path.startswith(app_bundle):
            continue

        # Skip system libraries
        if is_system_path(lib_path):
            continue

        # Resolve symlinks to get real file
        real_lib_path, symlink_chain = resolve_symlink(lib_path)

        # Skip if the real file is in a system path
        if is_system_path(real_lib_path):
            continue

        lib_name = os.path.basename(real_lib_path)
        new_path = os.path.join(frameworks_dir, lib_name)
        new_install_name = f"@rpath/{lib_name}"

        # Record the mapping from old install name to new install name
        lib_mapping[lib_info.install_name] = new_install_name

        # Copy the real file if not already present
        if not os.path.exists(new_path):
            shutil.copy2(real_lib_path, new_path)

        # Recreate symlink chain
        current_name = lib_name
        for link_name in reversed(symlink_chain):
            link_path = os.path.join(frameworks_dir, link_name)
            if not os.path.exists(link_path):
                os.symlink(current_name, link_path)
            current_name = link_name

        # Prepare commands for the library itself
        if new_path not in commands:
            commands[new_path] = []

        # Set its own install name
        commands[new_path].append(("-id", new_install_name))

    # Second pass: update each binary's direct dependencies
    for binary_path, lib_info in all_dependencies.items():
        if binary_path not in commands:
            commands[binary_path] = []

        # Update only the direct dependencies of this binary
        for dep in lib_info.dependencies:
            if dep in lib_mapping:
                commands[binary_path].append(("-change", dep, lib_mapping[dep]))

    frameworks_dir = os.path.join(app_bundle, "Contents", "Frameworks")

    def calculate_relative_frameworks_path(binary_path: str) -> str:
        """Calculate relative path from binary to Frameworks directory."""
        binary_dir = os.path.dirname(binary_path)
        rel_path = os.path.relpath(frameworks_dir, binary_dir)
        return rel_path

    # Set @loader_path/../Frameworks as the only rpath for all binaries
    for binary in binaries:
        if binary not in commands:
            commands[binary] = []

        # Get existing rpaths
        lib_info = parse_macho_info(binary)
        # Delete absolute rpaths
        for rpath in lib_info.rpaths:
            if rpath.startswith("/"):
                commands[binary].append(("-delete_rpath", rpath))

        # Add proper search path for all executables
        rel_frameworks_path = calculate_relative_frameworks_path(binary)
        new_path = f"@loader_path/{rel_frameworks_path}"
        if (
            binary.startswith(f"{app_bundle}/Contents/MacOS")
            and new_path not in lib_info.rpaths
        ):
            commands[binary].append(("-add_rpath", new_path))

    # Execute install_name_tool commands
    for path, changes in commands.items():
        print(f"Changing {path}")
        cmd = ["install_name_tool"]
        if not changes:
            continue
        print(f"  {changes}")
        for command_tuple in changes:
            cmd.extend(command_tuple)
        print(f"Executing {cmd} {path}")
        subprocess.run(cmd + [path], check=True)


def main():
    parser = argparse.ArgumentParser(description="Enhanced macdeployqt implementation")
    parser.add_argument("app_bundle", help="Path to the app bundle")
    parser.add_argument(
        "--libdir",
        action="append",
        default=[],
        help="Additional library search directories",
    )

    args = parser.parse_args()

    lib_dirs = args.libdir + [os.path.join(args.app_bundle, "Contents", "Frameworks")]
    deploy_libraries(args.app_bundle, lib_dirs)


if __name__ == "__main__":
    main()
