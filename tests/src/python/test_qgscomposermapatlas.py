# -*- coding: utf-8 -*-
'''
test_qgscomposermapatlas.py 
                     --------------------------------------
               Date                 : Oct 2012
               Copyright            : (C) 2012 by Dr. Hugo Mercier
               email                : hugo dot mercier at oslandia dot com
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

class TestQgsComposerMapAtlas(unittest.TestCase):
        
    def testCase(self):
        TEST_DATA_DIR = unitTestDataPath()        
        vectorFileInfo = QFileInfo( TEST_DATA_DIR + QDir().separator().toAscii() + "france_parts.shp")
        mVectorLayer = QgsVectorLayer( vectorFileInfo.filePath(), vectorFileInfo.completeBaseName(), "ogr" )

        QgsMapLayerRegistry.instance().addMapLayer( mVectorLayer )
        
        # create composition with composer map
        mMapRenderer = QgsMapRenderer()
        layerStringList = QStringList()
        layerStringList.append( mVectorLayer.id() )
        mMapRenderer.setLayerSet( layerStringList )
        mMapRenderer.setProjectionsEnabled( True )

        # select epsg:2154
        crs = QgsCoordinateReferenceSystem()
        crs.createFromSrid( 2154 )
        mMapRenderer.setDestinationCrs( crs )

        mComposition = QgsComposition( mMapRenderer )
        mComposition.setPaperSize( 297, 210 )

        # fix the renderer, fill with green
        props = { "color": "0,127,0" }
        fillSymbol = QgsFillSymbolV2.createSimple( props )
        renderer = QgsSingleSymbolRendererV2( fillSymbol )
        mVectorLayer.setRendererV2( renderer )

        # the atlas map
        mAtlasMap = QgsComposerMap( mComposition, 20, 20, 130, 130 )
        mAtlasMap.setFrameEnabled( True )
        mAtlasMap.setAtlasCoverageLayer( mVectorLayer )
        mComposition.addComposerMap( mAtlasMap )
        mComposition.setAtlasMap( mAtlasMap )

        # an overview
        mOverview = QgsComposerMap( mComposition, 180, 20, 50, 50 )
        mOverview.setFrameEnabled( True )
        mOverview.setOverviewFrameMap( mAtlasMap.id() )
        mComposition.addComposerMap( mOverview )
        nextent = QgsRectangle( 49670.718, 6415139.086, 699672.519, 7065140.887 )
        mOverview.setNewExtent( nextent )

        # header label
        mLabel1 = QgsComposerLabel( mComposition )
        mComposition.addComposerLabel( mLabel1 )
        mLabel1.setText( "[% \"NAME_1\" %] area" )
        mLabel1.adjustSizeToText()
        mLabel1.setItemPosition( 150, 5 )

        # feature number label
        mLabel2 = QgsComposerLabel( mComposition )
        mComposition.addComposerLabel( mLabel2 )
        mLabel2.setText( "# [%$feature || ' / ' || $numfeatures%]" )
        mLabel2.adjustSizeToText()
        mLabel2.setItemPosition( 150, 200 )

        self.filename_test( mComposition )
        self.autoscale_render_test( mComposition, mLabel1, TEST_DATA_DIR )
        self.fixedscale_render_test( mComposition, mLabel1, TEST_DATA_DIR )
        self.hidden_render_test( mComposition, mLabel1, TEST_DATA_DIR )
        
    def filename_test( self, mComposition ):
        atlasRender = QgsAtlasRendering( mComposition )
        atlasRender.begin( "'output_' || $feature" )
        for i in range(0, atlasRender.numFeatures()):
            atlasRender.prepareForFeature( i )
            expected = QString( "output_%1" ).arg(i+1)
            print atlasRender.currentFilename()
            assert atlasRender.currentFilename() == expected
        atlasRender.end()

    def autoscale_render_test( self, mComposition, mLabel1, TEST_DATA_DIR ):
        atlasMap = mComposition.atlasMap()
        atlasMap.setAtlasFixedScale( False )
        atlasMap.setAtlasMargin( 0.10 )

        atlasRender = QgsAtlasRendering( mComposition )

        atlasRender.begin()

        for i in range(0, 2):
            atlasRender.prepareForFeature( i )
            mLabel1.adjustSizeToText()

            checker = QgsCompositionChecker()
            res = checker.testComposition( "Atlas autoscale test", mComposition, \
                                               QString( TEST_DATA_DIR ) + QDir.separator() + \
                                               "control_images" + QDir.separator() + \
                                               "expected_composermapatlas" + QDir.separator() + \
                                               QString( "autoscale_%1.png" ).arg( i ) )
            assert res[0] == True
        atlasRender.end()
        
    def fixedscale_render_test( self, mComposition, mLabel1, TEST_DATA_DIR ):
        atlasMap = mComposition.atlasMap()
        atlasMap.setNewExtent( QgsRectangle( 209838.166, 6528781.020, 610491.166, 6920530.620 ) );
        atlasMap.setAtlasFixedScale( True )

        atlasRender = QgsAtlasRendering( mComposition )

        atlasRender.begin()

        for i in range(0, 2):
            atlasRender.prepareForFeature( i )
            mLabel1.adjustSizeToText()

            checker = QgsCompositionChecker()
            res = checker.testComposition( "Atlas fixed scale test", mComposition, \
                                               QString( TEST_DATA_DIR ) + QDir.separator() + \
                                               "control_images" + QDir.separator() + \
                                               "expected_composermapatlas" + QDir.separator() + \
                                               QString( "fixedscale_%1.png" ).arg( i ) )
            assert res[0] == True
        atlasRender.end()

    def hidden_render_test( self, mComposition, mLabel1, TEST_DATA_DIR ):
        atlasMap = mComposition.atlasMap()
        atlasMap.setNewExtent( QgsRectangle( 209838.166, 6528781.020, 610491.166, 6920530.620 ) );
        atlasMap.setAtlasFixedScale( True )
        atlasMap.setAtlasHideCoverage( True )

        atlasRender = QgsAtlasRendering( mComposition )

        atlasRender.begin()

        for i in range(0, 2):
            atlasRender.prepareForFeature( i )
            mLabel1.adjustSizeToText()

            checker = QgsCompositionChecker()
            res = checker.testComposition( "Atlas hidden test", mComposition, \
                                               QString( TEST_DATA_DIR ) + QDir.separator() + \
                                               "control_images" + QDir.separator() + \
                                               "expected_composermapatlas" + QDir.separator() + \
                                               QString( "hiding_%1.png" ).arg( i ) )
            assert res[0] == True
        atlasRender.end()

if __name__ == '__main__':
    unittest.main()

