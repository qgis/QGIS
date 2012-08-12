import os
import unittest

from qgis.core import QgsRasterLayer, QgsPoint
from PyQt4.QtCore import QFileInfo, QString

# Convenience instances in case you may need them
# not used in this test
from utilities import getQgisTestApp
QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()

class TestQgsRasterLayer(unittest.TestCase):

    def testIdentify(self):
        myPath = os.path.abspath(os.path.join(__file__, '..', '..', '..', 'testdata', 'landsat.tif'))
        myFileInfo = QFileInfo(myPath)
        myBaseName = myFileInfo.baseName()
        myRasterLayer = QgsRasterLayer(myPath, myBaseName)
        myMessage = 'Raster not loaded: %s' % myPath
        assert myRasterLayer.isValid(), myMessage
        myPoint = QgsPoint(786690, 3345803)
        #print 'Extents: %s' % myRasterLayer.extent().toString()
        myResult, myRasterValues = myRasterLayer.identify(myPoint)
        assert myResult
        # Get the name of the first band
        myBandName = myRasterValues.keys()[0] 
        myExpectedName = QString('Band 1')
        myMessage = 'Expected "%s" got "%s" for first raster band name' % (
                    myExpectedName, myBandName)
        assert myExpectedName == myBandName, myMessage

        # Convert each band value to a list of ints then to a string

        myValues = myRasterValues.values()
        myIntValues = []
        for myValue in myValues:
          myIntValues.append(int(str(myValue)))
        myValues = str(myIntValues)
        myExpectedValues = '[127, 141, 112, 72, 86, 126, 156, 211, 170]'
        myMessage = 'Expected: %s\nGot: %s' % (myValues, myExpectedValues)
        self.assertEquals(myValues, myExpectedValues, myMessage)


if __name__ == '__main__':
    unittest.main()
