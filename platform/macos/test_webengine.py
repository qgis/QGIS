#!/usr/bin/env python3
"""Run manually with Contents/MacOS/python to check bundled Qt runtime paths."""

import sys
from pathlib import Path

from qgis.PyQt.QtCore import QCoreApplication, QLibraryInfo


def main():
    app = QCoreApplication(sys.argv)
    contents = Path(sys.executable).resolve().parents[1]
    # Do not let a developer's Qt installation hide missing bundled assets.
    paths = QLibraryInfo.LibraryPath
    for key in (paths.LibraryExecutablesPath, paths.DataPath, paths.TranslationsPath):
        path = Path(QLibraryInfo.path(key)).resolve()
        if not path.is_relative_to(contents):
            raise RuntimeError(f"Qt runtime path escapes the bundle: {path}")
    print("Bundled Qt WebEngine runtime paths: OK")
    del app


if __name__ == "__main__":
    main()
