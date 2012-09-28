import os, glob
import unittest

from qgis.core import QgsRasterLayer, QgsRasterChecker, QgsRasterPipe, QgsRasterFileWriter, QgsRasterProjector
from PyQt4.QtCore import QFileInfo, QString, QStringList, QTemporaryFile, QDir

# Convenience instances in case you may need them
# not used in this test
from utilities import getQgisTestApp
QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()

class TestQgsRasterFileWriter(unittest.TestCase):
    def __init__(self,methodName):
        unittest.TestCase.__init__(self,methodName)
        self.testDataDir = os.path.abspath(os.path.join(__file__, '..', '..', '..', 'testdata'))
        self.report = "<h1>Python Raster File Writer Tests</h1>\n"
      
    def write(self, theRasterName):
        print theRasterName

        path = "%s/%s" % ( self.testDataDir, theRasterName )
        #myFileInfo = QFileInfo( path )
        #myBaseName = myFileInfo.baseName()
        rasterLayer = QgsRasterLayer(path, "test")
        if not rasterLayer.isValid(): return False
        provider = rasterLayer.dataProvider()

        tmpFile = QTemporaryFile()
        tmpFile.open() # fileName is no avialable until open
        tmpName =  tmpFile.fileName()
        tmpFile.close();
        # do not remove when class is destroyd so that we can read the file and see difference
        tmpFile.setAutoRemove ( False )

        fileWriter = QgsRasterFileWriter ( tmpName )
        pipe = QgsRasterPipe()
        if not pipe.set( provider.clone() ): 
            print "Cannot set pipe provider"
            return False

        #nuller = QgsRasterNuller()
        #nuller.setNoData( ... )
        #if not pipe.insert( 1, nuller ): 
        #    print "Cannot set pipe nuller"
        #    return False

        projector = QgsRasterProjector()
        projector.setCRS( provider.crs(), provider.crs() )
        if not pipe.insert( 2, projector ):
            print "Cannot set pipe projector"
            return False

        fileWriter.writeRaster( pipe, provider.xSize(), provider.ySize(), provider.extent(), provider.crs() )

        checker = QgsRasterChecker()
        ok = checker.runTest( "gdal", tmpName, "gdal", path );
        self.report += checker.report();

        # All OK, we can delete the file
        tmpFile.setAutoRemove ( ok );

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
