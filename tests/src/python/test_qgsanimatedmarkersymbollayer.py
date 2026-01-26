"""
***************************************************************************
    test_qgsanimatedmarkersymbollayer.py
    ---------------------
    Date                 : April 2022
    Copyright            : (C) 2022 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Nyall Dawson"
__date__ = "April 2022"
__copyright__ = "(C) 2022, Nyall Dawson"

import os

from qgis.PyQt.QtCore import QSize
from qgis.core import (
    QgsAnimatedMarkerSymbolLayer,
    QgsMapSettings,
    QgsMarkerSymbol,
    QgsRectangle,
    QgsSingleSymbolRenderer,
    QgsVectorLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsAnimatedMarkerSymbolLayer(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "symbol_animatedmarker"

    def testRenderFrame1(self):
        point_shp = os.path.join(TEST_DATA_DIR, "points.shp")
        point_layer = QgsVectorLayer(point_shp, "Lines", "ogr")
        self.assertTrue(point_layer.isValid())

        marker_symbol = QgsMarkerSymbol()
        marker_symbol.deleteSymbolLayer(0)
        marker_symbol.appendSymbolLayer(QgsAnimatedMarkerSymbolLayer())
        marker_symbol[0].setPath(os.path.join(TEST_DATA_DIR, "qgis_logo_animated.gif"))
        marker_symbol[0].setSize(20)

        point_layer.setRenderer(QgsSingleSymbolRenderer(marker_symbol))

        ms = QgsMapSettings()
        ms.setOutputSize(QSize(200, 200))
        ms.setOutputDpi(96)
        ms.setExtent(QgsRectangle(-118.5, 19.0, -81.4, 50.4))
        ms.setLayers([point_layer])
        ms.setFrameRate(10)
        ms.setCurrentFrame(1)

        self.assertTrue(
            self.render_map_settings_check(
                "animatedmarker_frame1", "animatedmarker_frame1", ms
            )
        )

    def testRenderFrame2(self):
        point_shp = os.path.join(TEST_DATA_DIR, "points.shp")
        point_layer = QgsVectorLayer(point_shp, "Lines", "ogr")
        self.assertTrue(point_layer.isValid())

        marker_symbol = QgsMarkerSymbol()
        marker_symbol.deleteSymbolLayer(0)
        marker_symbol.appendSymbolLayer(QgsAnimatedMarkerSymbolLayer())
        marker_symbol[0].setPath(os.path.join(TEST_DATA_DIR, "qgis_logo_animated.gif"))
        marker_symbol[0].setSize(20)

        point_layer.setRenderer(QgsSingleSymbolRenderer(marker_symbol))

        ms = QgsMapSettings()
        ms.setOutputSize(QSize(200, 200))
        ms.setOutputDpi(96)
        ms.setExtent(QgsRectangle(-118.5, 19.0, -81.4, 50.4))
        ms.setLayers([point_layer])
        ms.setFrameRate(10)
        ms.setCurrentFrame(2)

        self.assertTrue(
            self.render_map_settings_check(
                "animatedmarker_frame2", "animatedmarker_frame2", ms
            )
        )


if __name__ == "__main__":
    unittest.main()
