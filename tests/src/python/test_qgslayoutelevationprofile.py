# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLayoutItemElevationProfile.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2023 by Nyall Dawson'
__date__ = '13/01/2023'
__copyright__ = 'Copyright 2023, The QGIS Project'

import os
from time import sleep

from qgis.PyQt.QtCore import QRectF, QDir
from qgis.PyQt.QtGui import QColor
from qgis.core import (QgsPrintLayout,
                       QgsLayoutItemElevationProfile,
                       QgsLayoutItemMap,
                       QgsLayout,
                       QgsMapSettings,
                       QgsVectorLayer,
                       QgsMarkerSymbol,
                       QgsSingleSymbolRenderer,
                       QgsRectangle,
                       QgsProject,
                       QgsLayoutObject,
                       QgsProperty,
                       QgsLayoutMeasurement,
                       QgsLayoutItem,
                       QgsLayoutPoint,
                       QgsLayoutSize,
                       QgsExpression,
                       QgsMapLayerLegendUtils,
                       QgsLegendStyle,
                       QgsFontUtils,
                       QgsLineSymbol,
                       QgsMapThemeCollection,
                       QgsCategorizedSymbolRenderer,
                       QgsRendererCategory,
                       QgsFillSymbol,
                       QgsApplication)
from qgis.testing import (start_app,
                          unittest
                          )
from qgslayoutchecker import QgsLayoutChecker
from test_qgslayoutitem import LayoutItemTestCase
from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsLayoutItemElevationProfile(unittest.TestCase, LayoutItemTestCase):

    @classmethod
    def setUpClass(cls):
        cls.item_class = QgsLayoutItemElevationProfile
        cls.report = "<h1>Python QgsLayoutItemElevationProfile Tests</h1>\n"

    @classmethod
    def tearDownClass(cls):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(cls.report)


if __name__ == '__main__':
    unittest.main()
