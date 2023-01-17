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
import tempfile

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
                       QgsRasterLayer,
                       QgsApplication,
                       QgsGeometry,
                       QgsCoordinateReferenceSystem,
                       QgsExpressionContextUtils,
                       QgsFlatTerrainProvider)
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

    def test_layers(self):
        project = QgsProject()
        layout = QgsPrintLayout(project)
        profile = QgsLayoutItemElevationProfile(layout)
        layout.addLayoutItem(profile)

        self.assertFalse(profile.layers())

        layer1 = QgsVectorLayer(os.path.join(unitTestDataPath(), 'france_parts.shp'), 'france', "ogr")
        self.assertTrue(layer1.isValid())
        project.addMapLayers([layer1])

        layer2 = QgsRasterLayer(os.path.join(unitTestDataPath(), 'landsat.tif'), 'landsat', "gdal")
        self.assertTrue(layer2.isValid())
        project.addMapLayers([layer2])

        layer3 = QgsVectorLayer(os.path.join(unitTestDataPath(), 'lines.shp'), 'lines', "ogr")
        self.assertTrue(layer3.isValid())
        project.addMapLayers([layer3])

        profile.setLayers([layer2, layer3])
        self.assertEqual(profile.layers(), [layer2, layer3])

        project.layoutManager().addLayout(layout)

        # test that layers are written/restored
        with tempfile.TemporaryDirectory() as temp_dir:
            self.assertTrue(project.write(os.path.join(temp_dir, 'p.qgs')))

            p2 = QgsProject()
            self.assertTrue(p2.read(os.path.join(temp_dir, 'p.qgs')))

            layout2 = p2.layoutManager().printLayouts()[0]
            profile2 = [i for i in layout2.items() if isinstance(i, QgsLayoutItemElevationProfile)][0]

            self.assertEqual([m.id() for m in profile2.layers()], [layer2.id(), layer3.id()])

    def test_settings(self):
        project = QgsProject()
        layout = QgsPrintLayout(project)
        profile = QgsLayoutItemElevationProfile(layout)
        layout.addLayoutItem(profile)
        project.layoutManager().addLayout(layout)

        # test that default settings are written/restored
        with tempfile.TemporaryDirectory() as temp_dir:
            self.assertTrue(project.write(os.path.join(temp_dir, 'p.qgs')))

            p2 = QgsProject()
            self.assertTrue(p2.read(os.path.join(temp_dir, 'p.qgs')))

            layout2 = p2.layoutManager().printLayouts()[0]
            profile2 = [i for i in layout2.items() if isinstance(i, QgsLayoutItemElevationProfile)][0]

            self.assertFalse(profile2.crs().isValid())
            self.assertEqual(profile2.tolerance(), 0)
            self.assertIsNone(profile2.profileCurve())

        curve = QgsGeometry.fromWkt('LineString(0 0, 10 10)')
        profile.setProfileCurve(curve.constGet().clone())
        self.assertEqual(profile.profileCurve().asWkt(), 'LineString (0 0, 10 10)')

        profile.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        self.assertEqual(profile.crs(), QgsCoordinateReferenceSystem('EPSG:3857'))
        profile.setTolerance(101)
        self.assertEqual(profile.tolerance(), 101)

        # test that settings are written/restored
        with tempfile.TemporaryDirectory() as temp_dir:
            self.assertTrue(project.write(os.path.join(temp_dir, 'p.qgs')))

            p2 = QgsProject()
            self.assertTrue(p2.read(os.path.join(temp_dir, 'p.qgs')))

            layout2 = p2.layoutManager().printLayouts()[0]
            profile2 = [i for i in layout2.items() if isinstance(i, QgsLayoutItemElevationProfile)][0]

            self.assertEqual(profile2.crs(), QgsCoordinateReferenceSystem('EPSG:3857'))
            self.assertEqual(profile2.tolerance(), 101)
            self.assertEqual(profile2.profileCurve().asWkt(), 'LineString (0 0, 10 10)')

    def test_request(self):
        """
        Test generating a request for the item
        """
        project = QgsProject()
        layout = QgsPrintLayout(project)
        profile = QgsLayoutItemElevationProfile(layout)
        layout.addLayoutItem(profile)
        project.layoutManager().addLayout(layout)

        curve = QgsGeometry.fromWkt('LineString(0 0, 10 10)')
        profile.setProfileCurve(curve.constGet().clone())
        profile.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        profile.setTolerance(101)

        QgsExpressionContextUtils.setLayoutItemVariable(profile, 'my_var', 202)

        req = profile.profileRequest()
        self.assertEqual(req.tolerance(), 101)
        self.assertEqual(req.profileCurve().asWkt(), 'LineString (0 0, 10 10)')
        self.assertEqual(req.crs(), QgsCoordinateReferenceSystem('EPSG:3857'))
        self.assertEqual(req.expressionContext().variable('my_var'), '202')

        project.elevationProperties().setTerrainProvider(QgsFlatTerrainProvider())

        req = profile.profileRequest()
        self.assertIsInstance(req.terrainProvider(), QgsFlatTerrainProvider)


if __name__ == '__main__':
    unittest.main()
