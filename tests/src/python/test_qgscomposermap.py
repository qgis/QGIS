# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsComposerMap.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2012 by Dr. Horst Düster / Dr. Marco Hugentobler'
__date__ = '20/08/2012'
__copyright__ = 'Copyright 2012, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

import os

from qgis.PyQt.QtCore import QFileInfo
from qgis.PyQt.QtXml import QDomDocument
from qgis.PyQt.QtGui import QPainter

from qgis.core import (QgsComposerMap,
                       QgsRectangle,
                       QgsRasterLayer,
                       QgsComposition,
                       QgsMapRenderer,
                       QgsMapLayerRegistry,
                       QgsMultiBandColorRenderer,
                       )

from qgis.testing import start_app, unittest
from utilities import unitTestDataPath
from qgscompositionchecker import QgsCompositionChecker

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsComposerMap(unittest.TestCase):

    def __init__(self, methodName):
        """Run once on class initialization."""
        unittest.TestCase.__init__(self, methodName)
        myPath = os.path.join(TEST_DATA_DIR, 'rgb256x256.png')
        rasterFileInfo = QFileInfo(myPath)
        mRasterLayer = QgsRasterLayer(rasterFileInfo.filePath(),
                                      rasterFileInfo.completeBaseName())
        rasterRenderer = QgsMultiBandColorRenderer(
            mRasterLayer.dataProvider(), 1, 2, 3)
        mRasterLayer.setRenderer(rasterRenderer)
        #pipe = mRasterLayer.pipe()
        #assert pipe.set(rasterRenderer), 'Cannot set pipe renderer'
        QgsMapLayerRegistry.instance().addMapLayers([mRasterLayer])

        # create composition with composer map
        self.mMapRenderer = QgsMapRenderer()
        layerStringList = []
        layerStringList.append(mRasterLayer.id())
        self.mMapRenderer.setLayerSet(layerStringList)
        self.mMapRenderer.setProjectionsEnabled(False)
        self.mComposition = QgsComposition(self.mMapRenderer)
        self.mComposition.setPaperSize(297, 210)
        self.mComposerMap = QgsComposerMap(self.mComposition, 20, 20, 200, 100)
        self.mComposerMap.setFrameEnabled(True)
        self.mComposition.addComposerMap(self.mComposerMap)

    def testOverviewMap(self):
        overviewMap = QgsComposerMap(self.mComposition, 20, 130, 70, 70)
        overviewMap.setFrameEnabled(True)
        self.mComposition.addComposerMap(overviewMap)
        # zoom in
        myRectangle = QgsRectangle(96, -152, 160, -120)
        self.mComposerMap.setNewExtent(myRectangle)
        myRectangle2 = QgsRectangle(0, -256, 256, 0)
        overviewMap.setNewExtent(myRectangle2)
        overviewMap.setOverviewFrameMap(self.mComposerMap.id())
        checker = QgsCompositionChecker('composermap_overview', self.mComposition)
        checker.setControlPathPrefix("composer_mapoverview")
        myTestResult, myMessage = checker.testComposition()
        self.mComposition.removeComposerItem(overviewMap)
        assert myTestResult, myMessage

    def testOverviewMapBlend(self):
        overviewMap = QgsComposerMap(self.mComposition, 20, 130, 70, 70)
        overviewMap.setFrameEnabled(True)
        self.mComposition.addComposerMap(overviewMap)
        # zoom in
        myRectangle = QgsRectangle(96, -152, 160, -120)
        self.mComposerMap.setNewExtent(myRectangle)
        myRectangle2 = QgsRectangle(0, -256, 256, 0)
        overviewMap.setNewExtent(myRectangle2)
        overviewMap.setOverviewFrameMap(self.mComposerMap.id())
        overviewMap.setOverviewBlendMode(QPainter.CompositionMode_Multiply)
        checker = QgsCompositionChecker('composermap_overview_blending', self.mComposition)
        checker.setControlPathPrefix("composer_mapoverview")
        myTestResult, myMessage = checker.testComposition()
        self.mComposition.removeComposerItem(overviewMap)
        assert myTestResult, myMessage

    def testOverviewMapInvert(self):
        overviewMap = QgsComposerMap(self.mComposition, 20, 130, 70, 70)
        overviewMap.setFrameEnabled(True)
        self.mComposition.addComposerMap(overviewMap)
        # zoom in
        myRectangle = QgsRectangle(96, -152, 160, -120)
        self.mComposerMap.setNewExtent(myRectangle)
        myRectangle2 = QgsRectangle(0, -256, 256, 0)
        overviewMap.setNewExtent(myRectangle2)
        overviewMap.setOverviewFrameMap(self.mComposerMap.id())
        overviewMap.setOverviewInverted(True)
        checker = QgsCompositionChecker('composermap_overview_invert', self.mComposition)
        checker.setControlPathPrefix("composer_mapoverview")
        myTestResult, myMessage = checker.testComposition()
        self.mComposition.removeComposerItem(overviewMap)
        assert myTestResult, myMessage

    def testOverviewMapCenter(self):
        overviewMap = QgsComposerMap(self.mComposition, 20, 130, 70, 70)
        overviewMap.setFrameEnabled(True)
        self.mComposition.addComposerMap(overviewMap)
        # zoom in
        myRectangle = QgsRectangle(192, -288, 320, -224)
        self.mComposerMap.setNewExtent(myRectangle)
        myRectangle2 = QgsRectangle(0, -256, 256, 0)
        overviewMap.setNewExtent(myRectangle2)
        overviewMap.setOverviewFrameMap(self.mComposerMap.id())
        overviewMap.setOverviewInverted(False)
        overviewMap.setOverviewCentered(True)
        checker = QgsCompositionChecker('composermap_overview_center', self.mComposition)
        checker.setControlPathPrefix("composer_mapoverview")
        myTestResult, myMessage = checker.testComposition()
        self.mComposition.removeComposerItem(overviewMap)
        assert myTestResult, myMessage

    # Fails because addItemsFromXML has been commented out in sip
    @unittest.expectedFailure
    def testuniqueId(self):
        doc = QDomDocument()
        documentElement = doc.createElement('ComposerItemClipboard')
        self.mComposition.writeXML(documentElement, doc)
        self.mComposition.addItemsFromXML(documentElement, doc, 0, False)

        # test if both composer maps have different ids
        newMap = QgsComposerMap()
        mapList = self.mComposition.composerMapItems()

        for mapIt in mapList:
            if mapIt != self.mComposerMap:
                newMap = mapIt
                break

        oldId = self.mComposerMap.id()
        newId = newMap.id()

        self.mComposition.removeComposerItem(newMap)
        myMessage = 'old: %s new: %s' % (oldId, newId)
        assert oldId != newId, myMessage

    def testWorldFileGeneration(self):
        myRectangle = QgsRectangle(781662.375, 3339523.125, 793062.375, 3345223.125)
        self.mComposerMap.setNewExtent(myRectangle)
        self.mComposerMap.setMapRotation(30.0)

        self.mComposition.setGenerateWorldFile(True)
        self.mComposition.setWorldFileMap(self.mComposerMap)

        p = self.mComposition.computeWorldFileParameters()
        pexpected = (4.180480199790922, 2.4133064516129026, 779443.7612381146,
                     2.4136013686911886, -4.179969388427311, 3342408.5663611)
        ptolerance = (0.001, 0.001, 1, 0.001, 0.001, 1e+03)
        for i in range(0, 6):
            assert abs(p[i] - pexpected[i]) < ptolerance[i]

if __name__ == '__main__':
    unittest.main()
