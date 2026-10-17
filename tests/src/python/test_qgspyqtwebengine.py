"""QGIS functional tests for the Qt WebEngine Python bindings.

From build dir, run: ctest -R PyQgsPyQtWebEngine -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License.
"""

import unittest
from importlib import import_module

from qgis.PyQt.QtCore import QCoreApplication, QEventLoop, Qt, QTimer

QCoreApplication.setAttribute(Qt.ApplicationAttribute.AA_ShareOpenGLContexts)

from qgis.gui import QgsGui
from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtWebEngineCore import QWebEnginePage
from qgis.testing import QgisTestCase, start_app

start_app()


class TestPyQtWebEngine(QgisTestCase):
    def test_imports(self):
        """The WebEngine Python modules are available."""
        for module in ("QtWebEngineCore", "QtWebEngineQuick", "QtWebEngineWidgets"):
            with self.subTest(module=module):
                import_module(f"qgis.PyQt.{module}")

    def test_html_and_javascript(self):
        """WebEngine can load HTML and execute JavaScript without network access."""
        self.assertTrue(QgsGui.hasWebEngine())
        page = QWebEnginePage()
        loaded = QSignalSpy(page.loadFinished)
        page.setHtml('<html><body><p id="probe">QGIS WebEngine OK</p></body></html>')
        if not loaded:
            self.assertTrue(loaded.wait(60000), "Timed out loading WebEngine HTML")
        self.assertTrue(loaded[0][0], "WebEngine failed to load HTML")

        loop = QEventLoop()
        timer = QTimer()
        timer.setSingleShot(True)
        timer.timeout.connect(loop.quit)
        results = []

        def javascript_finished(value):
            results.append(value)
            loop.quit()

        page.runJavaScript(
            "document.getElementById('probe').textContent", javascript_finished
        )
        if not results:
            timer.start(60000)
            loop.exec()
        timer.stop()
        self.assertEqual(results, ["QGIS WebEngine OK"])


if __name__ == "__main__":
    unittest.main()
