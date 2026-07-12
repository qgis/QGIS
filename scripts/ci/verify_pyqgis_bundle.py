import argparse
import fnmatch
import os
import sys
import zipfile
from pathlib import Path


CHECKS = {
    "macos": (
        ("qgispython support library", lambda p: fnmatch.fnmatch(Path(p).name, "*qgispython*")),
        ("PyQGIS qgis Python package", lambda p: Path(p).name == "__init__.py" and "qgis" in Path(p).parts),
        ("PyQGIS core module", lambda p: fnmatch.fnmatch(Path(p).name, "_core*.so")),
        ("Python runtime", lambda p: fnmatch.fnmatch(Path(p).name, "python3*") or "Python.framework" in Path(p).parts),
        ("PyQt6 package", lambda p: "PyQt6" in Path(p).parts),
        ("PyQt6 Qsci module", lambda p: fnmatch.fnmatch(Path(p).name, "*Qsci*.so")),
    ),
    "windows": (
        ("qgispython support library", lambda p: fnmatch.fnmatch(Path(p).name, "*qgispython*.dll")),
        ("PyQGIS qgis Python package", lambda p: Path(p).name == "__init__.py" and "qgis" in Path(p).parts),
        ("PyQGIS core module", lambda p: fnmatch.fnmatch(Path(p).name, "_core*.pyd")),
        ("Python runtime", lambda p: Path(p).name.lower() == "python.exe" or fnmatch.fnmatch(Path(p).name.lower(), "python3*.dll")),
        ("PyQt6 package", lambda p: "PyQt6" in Path(p).parts),
        ("PyQt6 Qsci module", lambda p: fnmatch.fnmatch(Path(p).name, "*Qsci*.pyd")),
    ),
}


def iter_artifact_paths(path):
    if path.is_dir():
        seen_dirs = set()
        for root, dirs, files in os.walk(path, followlinks=True):
            try:
                stat = os.stat(root)
            except OSError:
                dirs[:] = []
                continue
            key = (stat.st_dev, stat.st_ino)
            if key in seen_dirs:
                dirs[:] = []
                continue
            seen_dirs.add(key)

            root_path = Path(root)
            for name in dirs + files:
                yield (root_path / name).relative_to(path).as_posix()
        return

    if zipfile.is_zipfile(path):
        with zipfile.ZipFile(path) as archive:
            for name in archive.namelist():
                yield name
        return

    raise SystemExit(f"Unsupported artifact path: {path}")


def main():
    parser = argparse.ArgumentParser(description="Verify a packaged Strata artifact contains PyQGIS runtime files.")
    parser.add_argument("--platform", choices=sorted(CHECKS), required=True)
    parser.add_argument("artifact", type=Path)
    args = parser.parse_args()

    if not args.artifact.exists():
        raise SystemExit(f"Artifact does not exist: {args.artifact}")

    paths = list(iter_artifact_paths(args.artifact))
    failed = False
    for label, predicate in CHECKS[args.platform]:
        match = next((path for path in paths if predicate(path)), None)
        if match is None:
            print(f"Missing {label} in {args.artifact}", file=sys.stderr)
            failed = True
        else:
            print(f"{label}: {match}")

    if failed:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
