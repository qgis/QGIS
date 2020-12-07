# -*- coding: utf-8 -*-

"""
***************************************************************************
    test_qgsblendmodes.py
    ---------------------
    Date                 : May 2013
    Copyright            : (C) 2013 by Nyall Dawson, Massimo Endrighi
    Email                : nyall dot dawson at gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Nyall Dawson'
__date__ = 'May 2013'
__copyright__ = '(C) 2013, Nyall Dawson, Massimo Endrighi'

import qgis  # NOQA

import os

from qgis.PyQt.QtCore import QSize
from qgis.PyQt.QtGui import QPainter, QColor

from qgis.core import (QgsVectorLayer,
                       QgsVectorSimplifyMethod,
                       QgsProject,
                       QgsMultiRenderChecker,
                       QgsRasterLayer,
                       QgsMultiBandColorRenderer,
                       QgsRectangle,
                       QgsMapSettings
                       )

from qgis.testing import unittest

from qgis.testing.mocked import get_iface

from utilities import unitTestDataPath

TEST_DATA_DIR = unitTestDataPath()


class TestQgsBlendModes(unittest.TestCase):

    def __init__(self, methodName):
        """Run once on class initialization."""
        unittest.TestCase.__init__(self, methodName)

        self.iface = get_iface()

        # initialize class MapRegistry, Canvas, MapRenderer, Map and PAL
        self.mMapRegistry = QgsProject.instance()

        # create point layer
        myShpFile = os.path.join(TEST_DATA_DIR, 'points.shp')
        self.mPointLayer = QgsVectorLayer(myShpFile, 'Points', 'ogr')
        self.mMapRegistry.addMapLayer(self.mPointLayer)

        self.mSimplifyMethod = QgsVectorSimplifyMethod()
        self.mSimplifyMethod.setSimplifyHints(QgsVectorSimplifyMethod.NoSimplification)

        # create polygon layer
        myShpFile = os.path.join(TEST_DATA_DIR, 'polys.shp')
        self.mPolygonLayer = QgsVectorLayer(myShpFile, 'Polygons', 'ogr')
        self.mPolygonLayer.setSimplifyMethod(self.mSimplifyMethod)
        self.mMapRegistry.addMapLayer(self.mPolygonLayer)

        # create line layer
        myShpFile = os.path.join(TEST_DATA_DIR, 'lines.shp')
        self.mLineLayer = QgsVectorLayer(myShpFile, 'Lines', 'ogr')
        self.mLineLayer.setSimplifyMethod(self.mSimplifyMethod)
        self.mMapRegistry.addMapLayer(self.mLineLayer)

        # create two raster layers
        myRasterFile = os.path.join(TEST_DATA_DIR, 'rgb256x256.png')
        self.mRasterLayer1 = QgsRasterLayer(myRasterFile, "raster1")
        self.mRasterLayer2 = QgsRasterLayer(myRasterFile, "raster2")
        myMultiBandRenderer1 = QgsMultiBandColorRenderer(self.mRasterLayer1.dataProvider(), 1, 2, 3)
        self.mRasterLayer1.setRenderer(myMultiBandRenderer1)
        self.mMapRegistry.addMapLayer(self.mRasterLayer1)
        myMultiBandRenderer2 = QgsMultiBandColorRenderer(self.mRasterLayer2.dataProvider(), 1, 2, 3)
        self.mRasterLayer2.setRenderer(myMultiBandRenderer2)
        self.mMapRegistry.addMapLayer(self.mRasterLayer2)

        # to match blend modes test comparisons background
        self.mapSettings = QgsMapSettings()
        self.mapSettings.setLayers([self.mRasterLayer1, self.mRasterLayer2])
        self.mapSettings.setBackgroundColor(QColor(152, 219, 249))
        self.mapSettings.setOutputSize(QSize(400, 400))
        self.mapSettings.setOutputDpi(96)

        self.extent = QgsRectangle(-118.8888888888887720, 22.8002070393376783, -83.3333333333331581, 46.8719806763287536)

    def testVectorBlending(self):
        """Test that blend modes work for vector layers."""

        # Add vector layers to map
        myLayers = [self.mLineLayer, self.mPolygonLayer]
        self.mapSettings.setLayers(myLayers)
        self.mapSettings.setExtent(self.extent)

        # Set blending modes for both layers
        self.mLineLayer.setBlendMode(QPainter.CompositionMode_Difference)
        self.mPolygonLayer.setBlendMode(QPainter.CompositionMode_Difference)

        checker = QgsMultiRenderChecker()
        checker.setControlName("expected_vector_blendmodes")
        checker.setMapSettings(self.mapSettings)
        checker.setColorTolerance(1)

        myResult = checker.runTest("vector_blendmodes", 20)
        myMessage = ('vector blending failed')
        assert myResult, myMessage

        # Reset layers
        self.mLineLayer.setBlendMode(QPainter.CompositionMode_SourceOver)
        self.mPolygonLayer.setBlendMode(QPainter.CompositionMode_SourceOver)

    def testVectorFeatureBlending(self):
        """Test that feature blend modes work for vector layers."""

        # Add vector layers to map
        myLayers = [self.mLineLayer, self.mPolygonLayer]
        self.mapSettings.setLayers(myLayers)
        self.mapSettings.setExtent(self.extent)

        # Set feature blending for line layer
        self.mLineLayer.setFeatureBlendMode(QPainter.CompositionMode_Plus)

        checker = QgsMultiRenderChecker()
        checker.setControlName("expected_vector_featureblendmodes")
        checker.setMapSettings(self.mapSettings)
        checker.setColorTolerance(1)

        myResult = checker.runTest("vector_featureblendmodes", 20)
        myMessage = ('vector feature blending failed')
        assert myResult, myMessage

        # Reset layers
        self.mLineLayer.setFeatureBlendMode(QPainter.CompositionMode_SourceOver)

    def testVectorLayerOpacity(self):
        """Test that layer opacity works for vector layers."""

        # Add vector layers to map
        myLayers = [self.mLineLayer, self.mPolygonLayer]
        self.mapSettings.setLayers(myLayers)
        self.mapSettings.setExtent(self.extent)

        # Set feature blending for line layer
        self.mLineLayer.setOpacity(0.5)

        checker = QgsMultiRenderChecker()
        checker.setControlName("expected_vector_layertransparency")
        checker.setMapSettings(self.mapSettings)
        checker.setColorTolerance(1)

        myResult = checker.runTest("vector_layertransparency", 20)
        myMessage = ('vector layer transparency failed')
        assert myResult, myMessage

    def testRasterBlending(self):
        """Test that blend modes work for raster layers."""
        # Add raster layers to map
        myLayers = [self.mRasterLayer1, self.mRasterLayer2]
        self.mapSettings.setLayers(myLayers)
        self.mapSettings.setExtent(self.mRasterLayer1.extent())

        # Set blending mode for top layer
        self.mRasterLayer1.setBlendMode(QPainter.CompositionMode_Difference)
        checker = QgsMultiRenderChecker()
        checker.setControlName("expected_raster_blendmodes")
        checker.setMapSettings(self.mapSettings)
        checker.setColorTolerance(1)
        checker.setColorTolerance(1)

        myResult = checker.runTest("raster_blendmodes", 20)
        myMessage = ('raster blending failed')
        assert myResult, myMessage


if __name__ == '__main__':
    unittest.main()
