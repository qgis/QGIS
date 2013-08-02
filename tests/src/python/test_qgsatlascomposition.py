# -*- coding: utf-8 -*-
'''
test_qgsatlascomposition.py
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
import qgis
import unittest
from utilities import *
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4.QtXml import *
from qgis.core import *
from qgscompositionchecker import QgsCompositionChecker

QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()

class TestQgsAtlasComposition(unittest.TestCase):

    def testCase(self):
        self.TEST_DATA_DIR = unitTestDataPath()
        vectorFileInfo = QFileInfo( self.TEST_DATA_DIR + QDir().separator() + "france_parts.shp")
        mVectorLayer = QgsVectorLayer( vectorFileInfo.filePath(), vectorFileInfo.completeBaseName(), "ogr" )

        QgsMapLayerRegistry.instance().addMapLayers( [mVectorLayer] )

        # create composition with composer map
        mMapRenderer = QgsMapRenderer()
        layerStringList = []
        layerStringList.append( mVectorLayer.id() )
        mMapRenderer.setLayerSet( layerStringList )
        mMapRenderer.setProjectionsEnabled( True )

        # select epsg:2154
        crs = QgsCoordinateReferenceSystem()
        crs.createFromSrid( 2154 )
        mMapRenderer.setDestinationCrs( crs )

        self.mComposition = QgsComposition( mMapRenderer )
        self.mComposition.setPaperSize( 297, 210 )

        # fix the renderer, fill with green
        props = { "color": "0,127,0" }
        fillSymbol = QgsFillSymbolV2.createSimple( props )
        renderer = QgsSingleSymbolRendererV2( fillSymbol )
        mVectorLayer.setRendererV2( renderer )

        # the atlas map
        self.mAtlasMap = QgsComposerMap( self.mComposition, 20, 20, 130, 130 )
        self.mAtlasMap.setFrameEnabled( True )
        self.mComposition.addComposerMap( self.mAtlasMap )

        # the atlas
        self.mAtlas = QgsAtlasComposition( self.mComposition )
        self.mAtlas.setCoverageLayer( mVectorLayer )
        self.mAtlas.setComposerMap( self.mAtlasMap )

        # an overview
        mOverview = QgsComposerMap( self.mComposition, 180, 20, 50, 50 )
        mOverview.setFrameEnabled( True )
        mOverview.setOverviewFrameMap( self.mAtlasMap.id() )
        self.mComposition.addComposerMap( mOverview )
        nextent = QgsRectangle( 49670.718, 6415139.086, 699672.519, 7065140.887 )
        mOverview.setNewExtent( nextent )

        # set the fill symbol of the overview map
        props2 = { "color": "127,0,0,127" }
        fillSymbol2 = QgsFillSymbolV2.createSimple( props2 )
        mOverview.setOverviewFrameMapSymbol( fillSymbol2 );

        # header label
        self.mLabel1 = QgsComposerLabel( self.mComposition )
        self.mComposition.addComposerLabel( self.mLabel1 )
        self.mLabel1.setText( "[% \"NAME_1\" %] area" )
        self.mLabel1.adjustSizeToText()
        self.mLabel1.setItemPosition( 150, 5 )

        qWarning( "header label font: %s exactMatch:%s" % ( self.mLabel1.font().toString(), self.mLabel1.font().exactMatch() ) )

        # feature number label
        self.mLabel2 = QgsComposerLabel( self.mComposition )
        self.mComposition.addComposerLabel( self.mLabel2 )
        self.mLabel2.setText( "# [%$feature || ' / ' || $numfeatures%]" )
        self.mLabel2.adjustSizeToText()
        self.mLabel2.setItemPosition( 150, 200 )

        qWarning( "feature number label font: %s exactMatch:%s" % ( self.mLabel2.font().toString(), self.mLabel2.font().exactMatch() ) )

        self.filename_test()
        self.autoscale_render_test()
        self.fixedscale_render_test()
        self.hidden_render_test()

    def filename_test( self ):

        self.mAtlas.setFilenamePattern( "'output_' || $feature" )
        self.mAtlas.beginRender()
        for i in range(0, self.mAtlas.numFeatures()):
            self.mAtlas.prepareForFeature( i )
            expected =  "output_%d" % (i+1)
            assert self.mAtlas.currentFilename() == expected
        self.mAtlas.endRender()

    def autoscale_render_test( self ):
        self.mAtlas.setFixedScale( False )
        self.mAtlas.setMargin( 0.10 )

        self.mAtlas.beginRender()

        for i in range(0, 2):
            self.mAtlas.prepareForFeature( i )
            self.mLabel1.adjustSizeToText()

            checker = QgsCompositionChecker()
            res = checker.testComposition( "Atlas autoscale test", self.mComposition, \
                                               self.TEST_DATA_DIR + QDir.separator() + \
                                               "control_images" + QDir.separator() + \
                                               "expected_composermapatlas" + QDir.separator() + \
                                               "autoscale_%d.png" % i )
            assert res[0] == True
        self.mAtlas.endRender()

    def fixedscale_render_test( self ):
        self.mAtlasMap.setNewExtent( QgsRectangle( 209838.166, 6528781.020, 610491.166, 6920530.620 ) );
        self.mAtlas.setFixedScale( True )

        self.mAtlas.beginRender()

        for i in range(0, 2):
            self.mAtlas.prepareForFeature( i )
            self.mLabel1.adjustSizeToText()

            checker = QgsCompositionChecker()
            res = checker.testComposition( "Atlas fixed scale test", self.mComposition, \
                                               self.TEST_DATA_DIR + QDir.separator() + \
                                               "control_images" + QDir.separator() + \
                                               "expected_composermapatlas" + QDir.separator() + \
                                               "fixedscale_%d.png" % i )
            assert res[0] == True
        self.mAtlas.endRender()

    def hidden_render_test( self ):
        self.mAtlasMap.setNewExtent( QgsRectangle( 209838.166, 6528781.020, 610491.166, 6920530.620 ) );
        self.mAtlas.setFixedScale( True )
        self.mAtlas.setHideCoverage( True )

        self.mAtlas.beginRender()

        for i in range(0, 2):
            self.mAtlas.prepareForFeature( i )
            self.mLabel1.adjustSizeToText()

            checker = QgsCompositionChecker()
            res = checker.testComposition( "Atlas hidden test", self.mComposition, \
                                               self.TEST_DATA_DIR + QDir.separator() + \
                                               "control_images" + QDir.separator() + \
                                               "expected_composermapatlas" + QDir.separator() + \
                                               "hiding_%d.png" % i )
            assert res[0] == True
        self.mAtlas.endRender()

    def sorting_render_test( self ):
        self.mAtlasMap.setNewExtent( QgsRectangle( 209838.166, 6528781.020, 610491.166, 6920530.620 ) );
        self.mAtlas.setFixedScale( True )
        self.mAtlas.setHideCoverage( False )

        self.mAtlas.setSortFeatures( True )
        self.mAtlas.setSortKeyAttributeIndex( 4 ) # departement name
        self.mAtlas.setSortAscending( False )

        self.mAtlas.beginRender()

        for i in range(0, 2):
            self.mAtlas.prepareForFeature( i )
            self.mLabel1.adjustSizeToText()

            checker = QgsCompositionChecker()
            res = checker.testComposition( "Atlas sorting test", self.mComposition, \
                                               self.TEST_DATA_DIR + QDir.separator() + \
                                               "control_images" + QDir.separator() + \
                                               "expected_composermapatlas" + QDir.separator() + \
                                               "sorting_%d.png" % i )
            assert res[0] == True
        self.mAtlas.endRender()

    def filtering_render_test( self ):
        self.mAtlasMap.setNewExtent( QgsRectangle( 209838.166, 6528781.020, 610491.166, 6920530.620 ) );
        self.mAtlas.setFixedScale( True )
        self.mAtlas.setHideCoverage( False )

        self.mAtlas.setSortFeatures( False )

        self.mAtlas.setFilterFeatures( True )
        self.mAtlas.setFeatureFilter( "substr(NAME_1,1,1)='P'" ) # select only 'Pays de la loire'

        self.mAtlas.beginRender()

        for i in range(0, 1):
            self.mAtlas.prepareForFeature( i )
            self.mLabel1.adjustSizeToText()

            checker = QgsCompositionChecker()
            res = checker.testComposition( "Atlas filtering test", self.mComposition, \
                                               self.TEST_DATA_DIR + QDir.separator() + \
                                               "control_images" + QDir.separator() + \
                                               "expected_composermapatlas" + QDir.separator() + \
                                               "filtering_%d.png" % i )
            assert res[0] == True
        self.mAtlas.endRender()

if __name__ == '__main__':
    unittest.main()

