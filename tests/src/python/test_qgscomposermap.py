# -*- coding: utf-8 -*-
'''
test_qgscomposermap.py 
                     --------------------------------------
               Date                 : 31 Juli 2012
               Copyright            : (C) 2012 by Dr. Horst Düster / Dr. Marco Hugentobler
               email                : horst.duester@sourcepole.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
'''
import unittest
from utilities import *
from PyQt4.QtCore import * 
from PyQt4.QtGui import *
from PyQt4.QtXml import *
from qgis.core import *
from qgscompositionchecker import QgsCompositionChecker

QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()

class TestQgsComposerMap(unittest.TestCase):
        
    def testCase(self):
        TEST_DATA_DIR = unitTestDataPath()        
        rasterFileInfo = QFileInfo(TEST_DATA_DIR+QDir().separator().toAscii()+"landsat.tif")
        mRasterLayer = QgsRasterLayer(rasterFileInfo.filePath(),  rasterFileInfo.completeBaseName())
        rasterRenderer = QgsMultiBandColorRenderer( mRasterLayer.dataProvider(), 2, 3, 4 )
        mRasterLayer.setRenderer( rasterRenderer )
        QgsMapLayerRegistry.instance().addMapLayer( mRasterLayer )
        
          # create composition with composer map
        mMapRenderer = QgsMapRenderer()
        layerStringList = QStringList()
        layerStringList.append( mRasterLayer.id() )
        mMapRenderer.setLayerSet( layerStringList )        
        mMapRenderer.setProjectionsEnabled( False )
        mComposition = QgsComposition( mMapRenderer )
        mComposition.setPaperSize( 297, 210 )
        mComposerMap = QgsComposerMap( mComposition, 20, 20, 200, 100 )
        mComposition.addComposerMap( mComposerMap )
        self.grid(mComposerMap,  mComposition,  TEST_DATA_DIR)
        self.overviewMap(mComposerMap,  mComposition,  TEST_DATA_DIR)
        
    def grid(self,  mComposerMap,  mComposition,  TEST_DATA_DIR):
        mComposerMap.setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) )
        mComposerMap.setGridEnabled( True )
        mComposerMap.setGridIntervalX( 2000 )
        mComposerMap.setGridIntervalY( 2000 )
        mComposerMap.setShowGridAnnotation( True )
        mComposerMap.setGridPenWidth( 0.5 )
        mComposerMap.setGridAnnotationPrecision( 0 )
        mComposerMap.setGridAnnotationPosition( QgsComposerMap.Disabled, QgsComposerMap.Left )
        mComposerMap.setGridAnnotationPosition( QgsComposerMap.OutsideMapFrame, QgsComposerMap.Right )
        mComposerMap.setGridAnnotationPosition( QgsComposerMap.Disabled, QgsComposerMap.Top )
        mComposerMap.setGridAnnotationPosition( QgsComposerMap.OutsideMapFrame, QgsComposerMap.Bottom )
        mComposerMap.setGridAnnotationDirection( QgsComposerMap.Horizontal, QgsComposerMap.Right )
        mComposerMap.setGridAnnotationDirection( QgsComposerMap.Horizontal, QgsComposerMap.Bottom )
        checker = QgsCompositionChecker() 
        testResult = checker.testComposition( "Composer map grid", mComposition,  TEST_DATA_DIR  + QDir().separator().toAscii() +  "control_images" + QDir().separator().toAscii() + "expected_composermap" + QDir().separator().toAscii() + "composermap_landsat_grid.png" )
        mComposerMap.setGridEnabled( False )
        mComposerMap.setShowGridAnnotation( False )
        mTestName = "gaga"
#        myMessage = "<DartMeasurementFile name=\"Rendered Image " + mTestName + "\" type=\"image/png\">" + renderedFilePath +  "</DartMeasurementFile> <DartMeasurementFile name=\"Expected Image " + mTestName + "\" type=\"image/png\">" +  mExpectedImageFile + "</DartMeasurementFile> <DartMeasurementFile name=\"Difference Image " + mTestName + "\" type=\"image/png\">" +  diffFilePath + "</DartMeasurementFile>"
        assert testResult == True #,  myMessage
        
    def overviewMap(self,  mComposerMap,  mComposition,  TEST_DATA_DIR):
        overviewMap = QgsComposerMap( mComposition, 20, 130, 70, 70 )
        mComposition.addComposerMap( overviewMap )
        # zoom in
        mComposerMap.setNewExtent( QgsRectangle( 785462.375, 3341423.125, 789262.375, 3343323.125 ) )
        overviewMap.setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3350923.125 ) )
        overviewMap.setOverviewFrameMap( mComposerMap.id() )
        checker = QgsCompositionChecker() 
        testResult = checker.testComposition( "Composer map overview", mComposition,  TEST_DATA_DIR + QDir().separator().toAscii() +"control_images" + QDir().separator().toAscii() + "expected_composermap" + QDir().separator().toAscii() + "composermap_landsat_overview.png" )
        mComposition.removeComposerItem( overviewMap )
        assert testResult == True


#    def uniqueId(self,  mComposerMap,  mComposition):
#        doc = QDomDocument()
#        documentElement = doc.createElement( "ComposerItemClipboard" )
#        mComposerMap.writeXML( documentElement, doc )
#        mComposition.addItemsFromXML( documentElement, doc, 0, false )
#        
#        #test if both composer maps have different ids
#        newMap = QgsComposerMap()
#        mapList = mComposition.composerMapItems()
#        
#        for mapIt in mapList:
#            if mapIt != mComposerMap:
#              newMap = mapIt
#              break
#
#        oldId = mComposerMap.id()
#        newId = newMap.id()
#        
#        mComposition.removeComposerItem( newMap );
#        print "old: "+str(oldId)
#        print "new "+str(newId)
#        assert oldId != newId 

if __name__ == '__main__':
    unittest.main()

