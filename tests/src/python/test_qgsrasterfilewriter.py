# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsRasterFileWriter.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Radim Blazek'
__date__ = '20/08/2012'
__copyright__ = 'Copyright 2012, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
import glob
import qgis

from PyQt4.QtCore import (QTemporaryFile,
                          QDir)
from qgis.core import (QgsRasterLayer,
                       QgsRasterChecker,
                       QgsRasterPipe,
                       QgsRasterFileWriter,
                       QgsRasterProjector)
from utilities import (unitTestDataPath,
                       getQgisTestApp,
                       TestCase,
                       unittest
                       #expectedFailure
                       )
# Convenience instances in case you may need them
# not used in this test
QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()

class TestQgsRasterFileWriter(TestCase):

    def __init__(self,methodName):
        unittest.TestCase.__init__(self,methodName)
        self.testDataDir = unitTestDataPath()
        self.report = "<h1>Python Raster File Writer Tests</h1>\n"

    def write(self, theRasterName):
        print theRasterName

        path = "%s/%s" % ( self.testDataDir, theRasterName )
        rasterLayer = QgsRasterLayer(path, "test")
        if not rasterLayer.isValid(): return False
        provider = rasterLayer.dataProvider()

        tmpFile = QTemporaryFile()
        tmpFile.open() # fileName is no avialable until open
        tmpName =  tmpFile.fileName()
        tmpFile.close()
        # do not remove when class is destroyed so that we can read
        # the file and see difference
        tmpFile.setAutoRemove ( False )

        fileWriter = QgsRasterFileWriter ( tmpName )
        pipe = QgsRasterPipe()
        if not pipe.set( provider.clone() ):
            print "Cannot set pipe provider"
            return False

        projector = QgsRasterProjector()
        projector.setCRS( provider.crs(), provider.crs() )
        if not pipe.insert( 2, projector ):
            print "Cannot set pipe projector"
            return False

        fileWriter.writeRaster(
            pipe,
            provider.xSize(),
            provider.ySize(),
            provider.extent(),
            provider.crs() )

        checker = QgsRasterChecker()
        ok = checker.runTest( "gdal", tmpName, "gdal", path )
        self.report += checker.report()

        # All OK, we can delete the file
        tmpFile.setAutoRemove ( ok )

        return ok

    def testWrite(self):
        for name in glob.glob( "%s/raster/*.tif" % self.testDataDir ):
            baseName = os.path.basename ( name )
            allOk = True
            ok = self.write( "raster/%s" % baseName )
            if not ok: allOk = False

        reportFilePath = "%s/qgistest.html" % QDir.tempPath()
        reportFile = open(reportFilePath,'a')
        reportFile.write( self.report )
        reportFile.close()

        assert allOk, "Raster file writer test failed"

if __name__ == '__main__':
    unittest.main()
