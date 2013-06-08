# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsPalLabeling

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Larry Shaffer'
__date__ = '12/10/2012'
__copyright__ = 'Copyright 2012, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
from PyQt4.QtCore import *
from PyQt4.QtGui import *

from qgis.core import (QgsPalLabeling,
                       QgsPalLayerSettings,
                       QgsVectorLayer,
                       QgsMapLayerRegistry,
                       QgsMapRenderer,
                       QgsCoordinateReferenceSystem,
                       QgsRenderChecker,
                       QGis)

from utilities import (getQgisTestApp,
                       TestCase,
                       unittest,
                       expectedFailure,
                       unitTestDataPath)

# Convenience instances in case you may need them
QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsPalLabeling(TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""

        # Store/load the FreeSansQGIS labeling test font
        myFontDB = QFontDatabase()
        cls._testFontID = myFontDB.addApplicationFont(
            os.path.join(TEST_DATA_DIR, 'font', 'FreeSansQGIS.ttf'))
        myMessage = ('\nCould not store test font in font database, '
                     'skipping test suite')
        assert cls._testFontID != -1, myMessage

        cls._testFont = myFontDB.font('FreeSansQGIS', 'Medium', 12)
        myAppFont = QApplication.font()
        myMessage = ('\nCould not load test font from font database, '
                     'skipping test suite')
        assert cls._testFont.toString() != myAppFont.toString(), myMessage

        # initialize class MapRegistry, Canvas, MapRenderer, Map and PAL
        cls._MapRegistry = QgsMapLayerRegistry.instance()
        cls._Canvas = CANVAS
        # to match render test comparisons background
        cls._Canvas.setCanvasColor(QColor(152, 219, 249))
        cls._Map = cls._Canvas.map()
        cls._Map.resize(QSize(600, 400))
        cls._MapRenderer = cls._Canvas.mapRenderer()
        cls._MapRenderer.setOutputSize(QSize(600, 400), 72)

        cls._Pal = QgsPalLabeling();
        cls._MapRenderer.setLabelingEngine(cls._Pal)
        cls._PalEngine = cls._MapRenderer.labelingEngine()
        myMessage = ('\nCould initialize PAL labeling engine, '
                     'skipping test suite')
        assert cls._PalEngine, myMessage

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        # remove test font
        myFontDB = QFontDatabase()
        myResult = myFontDB.removeApplicationFont(cls._testFontID)
        myMessage = ('\nFailed to remove test font from font database')
        assert myResult, myMessage

    def setUp(self):
        """Run before each test."""
        pass

    def tearDown(self):
        """Run after each test."""
        pass

    def test_AddPALToVectorLayer(self):
        """Check if we can set a label field, verify that PAL is assigned
            and that output is rendered correctly"""
        # TODO: add UTM PAL-specific shps, with 4326 as on-the-fly cross-check
#        setCanvasCrs(26913)

        myShpFile = os.path.join(TEST_DATA_DIR, 'lines.shp')
        myVectorLayer = QgsVectorLayer(myShpFile, 'Lines', 'ogr')

        self._MapRegistry.addMapLayer(myVectorLayer)

        myLayers = QStringList()
        myLayers.append(myVectorLayer.id())
        self._MapRenderer.setLayerSet(myLayers)
        self._MapRenderer.setExtent(myVectorLayer.extent())
        self._Canvas.zoomToFullExtent()

        # check layer labeling is PAL with customProperty access
        # should not be activated on layer load
        myPalSet = myVectorLayer.customProperty( "labeling" ).toString()
        myMessage = '\nExpected: Empty QString\nGot: %s' % (str(myPalSet))
        assert str(myPalSet) == '', myMessage

        # simulate clicking checkbox, setting label field and clicking apply
        self._testFont.setPointSize(20)
        myPalLyr = QgsPalLayerSettings()

        myPalLyr.enabled = True
        myPalLyr.fieldName = 'Name'
        myPalLyr.placement = QgsPalLayerSettings.Line
        myPalLyr.placementFlags = QgsPalLayerSettings.AboveLine
        myPalLyr.xQuadOffset = 0
        myPalLyr.yQuadOffset = 0
        myPalLyr.xOffset = 0
        myPalLyr.yOffset = 0
        myPalLyr.angleOffset = 0
        myPalLyr.centroidWhole = False
        myPalLyr.textFont = self._testFont
        myPalLyr.textNamedStyle = QString("Medium")
        myPalLyr.textColor = Qt.black
        myPalLyr.textTransp = 0
        myPalLyr.previewBkgrdColor = Qt.white
        myPalLyr.priority = 5
        myPalLyr.obstacle = True
        myPalLyr.dist = 0
        myPalLyr.scaleMin = 0
        myPalLyr.scaleMax = 0
        myPalLyr.bufferSize = 1
        myPalLyr.bufferColor = Qt.white
        myPalLyr.bufferTransp = 0
        myPalLyr.bufferNoFill = False
        myPalLyr.bufferJoinStyle = Qt.RoundJoin
        myPalLyr.formatNumbers = False
        myPalLyr.decimals = 3
        myPalLyr.plusSign = False
        myPalLyr.labelPerPart = False
        myPalLyr.displayAll = True
        myPalLyr.mergeLines = False
        myPalLyr.minFeatureSize = 0.0
        myPalLyr.vectorScaleFactor = 1.0
        myPalLyr.rasterCompressFactor = 1.0
        myPalLyr.addDirectionSymbol = False
        myPalLyr.upsidedownLabels = QgsPalLayerSettings.Upright
        myPalLyr.fontSizeInMapUnits = False
        myPalLyr.bufferSizeInMapUnits = False
        myPalLyr.labelOffsetInMapUnits = True
        myPalLyr.distInMapUnits = False
        myPalLyr.wrapChar = ""
        myPalLyr.preserveRotation = True

        myPalLyr.writeToLayer(myVectorLayer)

        # check layer labeling is PAL with customProperty access
        myPalSet = myVectorLayer.customProperty( "labeling" ).toString()
        myMessage = '\nExpected: pal\nGot: %s' % (str(myPalSet))
        assert str(myPalSet) == 'pal', myMessage

        # check layer labeling is PAL via engine interface
        myMessage = '\nCould not get whether PAL enabled from labelingEngine'
        assert self._PalEngine.willUseLayer(myVectorLayer), myMessage

        #
        myChecker = QgsRenderChecker()
        myChecker.setControlName("expected_pal_aboveLineLabeling")
        myChecker.setMapRenderer(self._MapRenderer)

        myResult = myChecker.runTest("pal_aboveLineLabeling_python");
        myMessage = ('\nVector layer \'above line\' label engine '
                     'rendering test failed')
        assert myResult, myMessage

        # compare against a straight rendering/save as from QgsMapCanvasMap
        # unnecessary? works a bit different than QgsRenderChecker, though
#        myImage = os.path.join(unicode(QDir.tempPath()),
#                               'render_pal_aboveLineLabeling.png')
#        self._Map.render()
#        self._Canvas.saveAsImage(myImage)
#        myChecker.setRenderedImage(myImage)
#        myResult = myChecker.compareImages("pal_aboveLineLabeling_python")
#        myMessage = ('\nVector layer \'above line\' label engine '
#                     'comparison to QgsMapCanvasMap.render() test failed')
#        assert myResult, myMessage

        self._MapRegistry.removeMapLayer(myVectorLayer.id())

#    @expectedFailure
#    def testIssue####(self):
#        """Test we can .
#        """
#        pass

if __name__ == '__main__':
    unittest.main()


