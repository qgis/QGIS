# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsComposerLegend.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2016 by Nyall Dawson'
__date__ = '13/07/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from qgis.PyQt.QtCore import QRectF
from qgis.PyQt.QtGui import QColor

from qgis.core import (QgsComposerLegend,
                       QgsComposerMap,
                       QgsComposition,
                       QgsMapSettings,
                       QgsVectorLayer,
                       QgsMarkerSymbol,
                       QgsSingleSymbolRenderer,
                       QgsRectangle,
                       QgsProject,
                       QgsComposerObject,
                       QgsProperty
                       )
from qgis.testing import (start_app,
                          unittest
                          )
from utilities import unitTestDataPath
from qgscompositionchecker import QgsCompositionChecker
import os

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsComposerLegend(unittest.TestCase):

    def testInitialSizeSymbolMapUnits(self):
        """Test initial size of legend with a symbol size in map units"""

        point_path = os.path.join(TEST_DATA_DIR, 'points.shp')
        point_layer = QgsVectorLayer(point_path, 'points', 'ogr')
        QgsProject.instance().addMapLayers([point_layer])

        marker_symbol = QgsMarkerSymbol.createSimple({'color': '#ff0000', 'outline_style': 'no', 'size': '5', 'size_unit': 'MapUnit'})

        point_layer.setRenderer(QgsSingleSymbolRenderer(marker_symbol))

        s = QgsMapSettings()
        s.setLayers([point_layer])
        composition = QgsComposition(QgsProject.instance())
        composition.setPaperSize(297, 210)

        composer_map = QgsComposerMap(composition, 20, 20, 80, 80)
        composer_map.setFrameEnabled(True)
        composer_map.setLayers([point_layer])
        composition.addComposerMap(composer_map)
        composer_map.setNewExtent(point_layer.extent())

        legend = QgsComposerLegend(composition)
        legend.setSceneRect(QRectF(120, 20, 80, 80))
        legend.setFrameEnabled(True)
        legend.setFrameStrokeWidth(2)
        legend.setBackgroundColor(QColor(200, 200, 200))
        legend.setTitle('')
        composition.addComposerLegend(legend)
        legend.setComposerMap(composer_map)

        checker = QgsCompositionChecker(
            'composer_legend_mapunits', composition)
        checker.setControlPathPrefix("composer_legend")
        result, message = checker.testComposition()
        self.assertTrue(result, message)

        QgsProject.instance().removeMapLayers([point_layer.id()])

    def testResizeWithMapContent(self):
        """Test test legend resizes to match map content"""

        point_path = os.path.join(TEST_DATA_DIR, 'points.shp')
        point_layer = QgsVectorLayer(point_path, 'points', 'ogr')
        QgsProject.instance().addMapLayers([point_layer])

        s = QgsMapSettings()
        s.setLayers([point_layer])
        composition = QgsComposition(QgsProject.instance())
        composition.setPaperSize(297, 210)

        composer_map = QgsComposerMap(composition, 20, 20, 80, 80)
        composer_map.setFrameEnabled(True)
        composer_map.setLayers([point_layer])
        composition.addComposerMap(composer_map)
        composer_map.setNewExtent(point_layer.extent())

        legend = QgsComposerLegend(composition)
        legend.setSceneRect(QRectF(120, 20, 80, 80))
        legend.setFrameEnabled(True)
        legend.setFrameStrokeWidth(2)
        legend.setBackgroundColor(QColor(200, 200, 200))
        legend.setTitle('')
        legend.setLegendFilterByMapEnabled(True)
        composition.addComposerLegend(legend)
        legend.setComposerMap(composer_map)

        composer_map.setNewExtent(QgsRectangle(-102.51, 41.16, -102.36, 41.30))

        checker = QgsCompositionChecker(
            'composer_legend_size_content', composition)
        checker.setControlPathPrefix("composer_legend")
        result, message = checker.testComposition()
        self.assertTrue(result, message)

        QgsProject.instance().removeMapLayers([point_layer.id()])

    def testResizeDisabled(self):
        """Test that test legend does not resize if auto size is disabled"""

        point_path = os.path.join(TEST_DATA_DIR, 'points.shp')
        point_layer = QgsVectorLayer(point_path, 'points', 'ogr')
        QgsProject.instance().addMapLayers([point_layer])

        s = QgsMapSettings()
        s.setLayers([point_layer])
        composition = QgsComposition(QgsProject.instance())
        composition.setPaperSize(297, 210)

        composer_map = QgsComposerMap(composition, 20, 20, 80, 80)
        composer_map.setFrameEnabled(True)
        composer_map.setLayers([point_layer])
        composition.addComposerMap(composer_map)
        composer_map.setNewExtent(point_layer.extent())

        legend = QgsComposerLegend(composition)
        legend.setSceneRect(QRectF(120, 20, 80, 80))
        legend.setFrameEnabled(True)
        legend.setFrameStrokeWidth(2)
        legend.setBackgroundColor(QColor(200, 200, 200))
        legend.setTitle('')
        legend.setLegendFilterByMapEnabled(True)

        # disable auto resizing
        legend.setResizeToContents(False)

        composition.addComposerLegend(legend)
        legend.setComposerMap(composer_map)

        composer_map.setNewExtent(QgsRectangle(-102.51, 41.16, -102.36, 41.30))

        checker = QgsCompositionChecker(
            'composer_legend_noresize', composition)
        checker.setControlPathPrefix("composer_legend")
        result, message = checker.testComposition()
        self.assertTrue(result, message)

        QgsProject.instance().removeMapLayers([point_layer.id()])

    def testResizeDisabledCrop(self):
        """Test that if legend resizing is disabled, and legend is too small, then content is cropped"""

        point_path = os.path.join(TEST_DATA_DIR, 'points.shp')
        point_layer = QgsVectorLayer(point_path, 'points', 'ogr')
        QgsProject.instance().addMapLayers([point_layer])

        s = QgsMapSettings()
        s.setLayers([point_layer])
        composition = QgsComposition(QgsProject.instance())
        composition.setPaperSize(297, 210)

        composer_map = QgsComposerMap(composition, 20, 20, 80, 80)
        composer_map.setFrameEnabled(True)
        composer_map.setLayers([point_layer])
        composition.addComposerMap(composer_map)
        composer_map.setNewExtent(point_layer.extent())

        legend = QgsComposerLegend(composition)
        legend.setSceneRect(QRectF(120, 20, 20, 20))
        legend.setFrameEnabled(True)
        legend.setFrameStrokeWidth(2)
        legend.setBackgroundColor(QColor(200, 200, 200))
        legend.setTitle('')
        legend.setLegendFilterByMapEnabled(True)

        # disable auto resizing
        legend.setResizeToContents(False)

        composition.addComposerLegend(legend)
        legend.setComposerMap(composer_map)

        composer_map.setNewExtent(QgsRectangle(-102.51, 41.16, -102.36, 41.30))

        checker = QgsCompositionChecker(
            'composer_legend_noresize_crop', composition)
        checker.setControlPathPrefix("composer_legend")
        result, message = checker.testComposition()
        self.assertTrue(result, message)

        QgsProject.instance().removeMapLayers([point_layer.id()])

    def testDataDefinedTitle(self):
        mapSettings = QgsMapSettings()  # NOQA

        composition = QgsComposition(QgsProject.instance())
        composition.setPaperSize(297, 210)

        legend = QgsComposerLegend(composition)
        composition.addComposerLegend(legend)

        legend.setTitle('original')
        self.assertEqual(legend.title(), 'original')
        self.assertEqual(legend.legendSettings().title(), 'original')

        legend.dataDefinedProperties().setProperty(QgsComposerObject.LegendTitle, QgsProperty.fromExpression("'new'"))
        legend.refreshDataDefinedProperty()
        self.assertEqual(legend.title(), 'original')
        self.assertEqual(legend.legendSettings().title(), 'new')

    def testDataDefinedColumnCount(self):
        mapSettings = QgsMapSettings()  # NOQA

        composition = QgsComposition(QgsProject.instance())
        composition.setPaperSize(297, 210)

        legend = QgsComposerLegend(composition)
        composition.addComposerLegend(legend)

        legend.setColumnCount(2)
        self.assertEqual(legend.columnCount(), 2)
        self.assertEqual(legend.legendSettings().columnCount(), 2)

        legend.dataDefinedProperties().setProperty(QgsComposerObject.LegendColumnCount, QgsProperty.fromExpression("5"))
        legend.refreshDataDefinedProperty()
        self.assertEqual(legend.columnCount(), 2)
        self.assertEqual(legend.legendSettings().columnCount(), 5)


if __name__ == '__main__':
    unittest.main()
