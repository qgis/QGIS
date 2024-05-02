"""QGIS Unit tests for QgsHeatmapRenderer

From build dir, run: ctest -R PyQgsHeatmapRenderer -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import os

from qgis.PyQt.QtGui import QColor
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    QgsHeatmapRenderer,
    QgsGradientColorRamp,
    QgsReadWriteContext,
    QgsColorRampLegendNodeSettings
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()

TEST_DATA_DIR = unitTestDataPath()


class TestQgsHeatmapRenderer(QgisTestCase):

    def test_clone(self):
        """
        Test cloning renderer
        """

        renderer = QgsHeatmapRenderer()
        renderer.setColorRamp(
            QgsGradientColorRamp(QColor(255, 0, 0), QColor(255, 200, 100)))

        legend_settings = QgsColorRampLegendNodeSettings()
        legend_settings.setMaximumLabel('my max')
        legend_settings.setMinimumLabel('my min')
        renderer.setLegendSettings(legend_settings)

        renderer2 = renderer.clone()
        self.assertEqual(renderer2.colorRamp().color1(), QColor(255, 0, 0))
        self.assertEqual(renderer2.colorRamp().color2(), QColor(255, 200, 100))
        self.assertEqual(renderer2.legendSettings().minimumLabel(), 'my min')
        self.assertEqual(renderer2.legendSettings().maximumLabel(), 'my max')

    def test_write_read_xml(self):
        """
        Test writing renderer to xml and restoring
        """

        renderer = QgsHeatmapRenderer()
        renderer.setColorRamp(
            QgsGradientColorRamp(QColor(255, 0, 0), QColor(255, 200, 100)))

        legend_settings = QgsColorRampLegendNodeSettings()
        legend_settings.setMaximumLabel('my max')
        legend_settings.setMinimumLabel('my min')
        renderer.setLegendSettings(legend_settings)

        doc = QDomDocument("testdoc")
        elem = renderer.save(doc, QgsReadWriteContext())

        renderer2 = QgsHeatmapRenderer.create(elem, QgsReadWriteContext())
        self.assertEqual(renderer2.colorRamp().color1(), QColor(255, 0, 0))
        self.assertEqual(renderer2.colorRamp().color2(), QColor(255, 200, 100))
        self.assertEqual(renderer2.legendSettings().minimumLabel(), 'my min')
        self.assertEqual(renderer2.legendSettings().maximumLabel(), 'my max')


if __name__ == "__main__":
    unittest.main()
