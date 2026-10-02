#!/usr/bin/env python3
"""Run with Contents/MacOS/python to smoke-test a deployed macOS bundle."""

import os
import sys
from importlib import import_module
from pathlib import Path

from qgis.PyQt.QtCore import QCoreApplication, QLibraryInfo, Qt, QTimer

QCoreApplication.setAttribute(Qt.ApplicationAttribute.AA_ShareOpenGLContexts)

from qgis.PyQt.QtWebEngineCore import QWebEnginePage
from qgis.core import QgsApplication
from qgis.gui import QgsGui


def main():
    for module in ("QtWebEngineQuick", "QtWebEngineWidgets"):
        import_module(f"qgis.PyQt.{module}")

    app = QgsApplication([os.fsencode(sys.argv[0])], False)
    contents = Path(sys.executable).resolve().parents[1]
    # Do not let a developer's Qt installation hide missing bundled assets.
    paths = QLibraryInfo.LibraryPath
    for key in (paths.LibraryExecutablesPath, paths.DataPath, paths.TranslationsPath):
        path = Path(QLibraryInfo.path(key)).resolve()
        if not path.is_relative_to(contents):
            raise RuntimeError(f"Qt runtime path escapes the bundle: {path}")

    if not QgsGui.hasWebEngine():
        raise RuntimeError("QGIS was compiled without Qt WebEngine")
    page = QWebEnginePage()
    timer = QTimer()
    timer.setSingleShot(True)
    timer.timeout.connect(lambda: app.exit(1))

    def loaded(ok):
        if not ok:
            app.exit(1)
            return
        page.runJavaScript(
            "document.getElementById('probe').textContent",
            lambda value: app.exit(0 if value == "QGIS WebEngine OK" else 1),
        )

    page.loadFinished.connect(loaded)
    timer.start(60000)
    page.setHtml('<html><body><p id="probe">QGIS WebEngine OK</p></body></html>')
    result = app.exec()
    del page
    if result:
        raise RuntimeError("Bundled Qt WebEngine failed to render HTML within 60 seconds")
    print("Bundled Qt WebEngine imports, renderer and JavaScript: OK")


if __name__ == "__main__":
    main()
