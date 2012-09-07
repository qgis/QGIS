import os
import unittest

from PyQt4.QtCore import QDir

from qgis.core import QgsVectorLayer
from utilities import getQgisTestApp, unitTestDataPath

QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()


class TestQgsVectorLayer(unittest.TestCase):
    
    def test_FeatureCount(self):
        myPath = os.path.join(unitTestDataPath(), 'lines.shp')
        myLayer = QgsVectorLayer(myPath, 'Lines', 'ogr')
        myCount = myLayer.featureCount()
        myExpectedCount = 6
        myMessage = '\nExpected: %s\nGot: %s' % (myCount, myExpectedCount)
        assert myCount == myExpectedCount, myMessage

if __name__ == '__main__':
    unittest.main()

