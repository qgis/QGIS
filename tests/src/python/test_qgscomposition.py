# -*- coding: utf-8 -*-
'''
test_qgscomposerhtml.py
                     --------------------------------------
               Date                 : September 2012
               Copyright            : (C) 2012 by Tim Sutton
               email                : tim@linfiniti.com
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
import sys
import os
from utilities import unitTestDataPath, getQgisTestApp
from PyQt4.QtCore import QFileInfo, QDir, QStringList
from PyQt4.QtXml import QDomDocument
from qgis.core import (QgsComposition,
                       QgsPoint,
                       QgsRasterLayer,
                       QgsMultiBandColorRenderer,
                       QgsMapLayerRegistry,
                       QgsMapRenderer
                       )

# support python < 2.7 via unittest2
# needed for expected failure decorator
if sys.version_info[0:2] < (2,7):
    try:
        from unittest2 import TestCase, expectedFailure
    except ImportError:
        print "You need to install unittest2 to run the salt tests"
        sys.exit(1)
else:
    from unittest import TestCase, expectedFailure

QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsComposition(TestCase):

    def setUp(self):
        """Run before each test."""
        pass

    def tearDown(self):
        """Run after each test."""
        pass

    @expectedFailure
    def testSubstitutionMap(self):
        """Test that we can use degree symbols in substitutions.
        """
        # Create a point and convert it to text containing a degree symbol.
        myPoint = QgsPoint(12.3, -33.33)
        myCoordinates = myPoint.toDegreesMinutesSeconds(2)
        myTokens = myCoordinates.split(',')
        myLongitude = myTokens[0]
        myLatitude = myTokens[1]
        myText = 'Latitude: %s, Longitude: %s' % (myLatitude, myLongitude)

        # Load the composition with the substitutions
        myComposition = QgsComposition(CANVAS.mapRenderer())
        mySubstitutionMap = {'replace-me': myText }
        myFile = os.path.join(TEST_DATA_DIR, 'template-for-substitution.qpt')
        myTemplateFile = file(myFile, 'rt')
        myTemplateContent = myTemplateFile.read()
        myTemplateFile.close()
        myDocument = QDomDocument()
        myDocument.setContent(myTemplateContent)
        myComposition.loadFromTemplate(myDocument, mySubstitutionMap)

        # We should be able to get map0
        myMap = myComposition.getComposerMapById(0)
        myMessage = ('Map 0 could not be found in template %s', myFile)
        assert myMap is not None, myMessage

    def testNoSubstitutionMap(self):
        """Test that we can get a map if we use no text substitutions."""
        myComposition = QgsComposition(CANVAS.mapRenderer())
        myFile = os.path.join(TEST_DATA_DIR, 'template-for-substitution.qpt')
        myTemplateFile = file(myFile, 'rt')
        myTemplateContent = myTemplateFile.read()
        myTemplateFile.close()
        myDocument = QDomDocument()
        myDocument.setContent(myTemplateContent)
        myComposition.loadFromTemplate(myDocument)

        # We should be able to get map0
        myMap = myComposition.getComposerMapById(0)
        myMessage = ('Map 0 could not be found in template %s', myFile)
        assert myMap is not None, myMessage

    def testPrintMapFromTemplate(self):
        """Test that we can get a map to render in the template."""
        myPath = os.path.join(TEST_DATA_DIR, 'landsat.tif')
        myFileInfo = QFileInfo(myPath)
        myRasterLayer = QgsRasterLayer(myFileInfo.filePath(),
                                       myFileInfo.completeBaseName())
        myRenderer = QgsMultiBandColorRenderer(
                        myRasterLayer.dataProvider(), 2, 3, 4)
        #mRasterLayer.setRenderer( rasterRenderer )
        myPipe = myRasterLayer.pipe()
        assert myPipe.set( myRenderer ), "Cannot set pipe renderer"

        QgsMapLayerRegistry.instance().addMapLayer(myRasterLayer)

        myMapRenderer = QgsMapRenderer()
        myLayerStringList = QStringList()
        myLayerStringList.append(myRasterLayer.id())
        myMapRenderer.setLayerSet(myLayerStringList)
        myMapRenderer.setProjectionsEnabled(False)

        myComposition = QgsComposition(myMapRenderer)
        myFile = os.path.join(TEST_DATA_DIR, 'template-for-substitution.qpt')
        myTemplateFile = file(myFile, 'rt')
        myTemplateContent = myTemplateFile.read()
        myTemplateFile.close()
        myDocument = QDomDocument()
        myDocument.setContent(myTemplateContent)
        myComposition.loadFromTemplate(myDocument)

        # now render the map, first zooming to the raster extents
        myMap = myComposition.getComposerMapById(0)
        myMessage = ('Map 0 could not be found in template %s', myFile)
        assert myMap is not None, myMessage

        myExtent = myRasterLayer.extent()
        myMap.setNewExtent(myExtent)

        myImagePath = os.path.join(str(QDir.tempPath()),
                                   'template_map_render_python.png')

        myPageNumber = 0
        myImage = myComposition.printPageAsRaster(myPageNumber)
        myImage.save(myImagePath)
        assert os.path.exists(myImagePath), 'Map render was not created.'

        # Not sure if this is a predictable way to test but its quicker than
        # rendering.
        myFileSize = QFileInfo(myImagePath).size()
        myExpectedFileSize = 100000
        myMessage = ('Expected file size to be greater than %s, got %s' %
                     (myExpectedFileSize, myFileSize))
        assert myFileSize > myExpectedFileSize, myMessage




if __name__ == '__main__':
    unittest.main()
