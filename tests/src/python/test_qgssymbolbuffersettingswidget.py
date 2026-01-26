"""QGIS Unit tests for QgsSymbolBufferSettingsWidget.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtGui import QColor

from qgis.core import Qgis, QgsMapUnitScale, QgsSymbolBufferSettings, QgsFillSymbol
from qgis.gui import QgsSymbolBufferSettingsWidget
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsSymbolBufferSettingsWidget(QgisTestCase):

    def testWidget(self):
        widget = QgsSymbolBufferSettingsWidget(None)

        s = QgsSymbolBufferSettings()
        s.setEnabled(True)

        s.setSize(6.8)
        s.setSizeUnit(Qgis.RenderUnit.Inches)
        s.setSizeMapUnitScale(QgsMapUnitScale(minScale=1, maxScale=10))

        s.setJoinStyle(Qt.PenJoinStyle.MiterJoin)
        s.setFillSymbol(
            QgsFillSymbol.createSimple({"color": "#00ff00", "outline_color": "red"})
        )

        widget.setBufferSettings(s)

        new_settings = widget.bufferSettings()

        self.assertTrue(new_settings.enabled())
        self.assertEqual(new_settings.size(), 6.8)
        self.assertEqual(new_settings.sizeUnit(), Qgis.RenderUnit.Inches)
        self.assertEqual(new_settings.sizeMapUnitScale().minScale, 1)
        self.assertEqual(new_settings.sizeMapUnitScale().maxScale, 10)
        self.assertEqual(new_settings.joinStyle(), Qt.PenJoinStyle.MiterJoin)
        self.assertEqual(new_settings.fillSymbol()[0].color(), QColor(0, 255, 0))
        self.assertEqual(new_settings.fillSymbol()[0].strokeColor(), QColor(255, 0, 0))


if __name__ == "__main__":
    unittest.main()
