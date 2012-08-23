import unittest

from qgis.core import (QgsRectangle,
                       QgsCoordinateReferenceSystem,
                       QgsCoordinateTransform,
                       QGis)

# Convenience instances in case you may need them
# not used in this test
#from utilities import getQgisTestApp
#QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()

class TestQgsCoordinateTransform(unittest.TestCase):

    def testTransformBoundingBox(self):
        myExtent = QgsRectangle(242270, 6043737, 246330, 6045897)
        myGeoCrs = QgsCoordinateReferenceSystem()
        myGeoCrs.createFromId(4326, QgsCoordinateReferenceSystem.EpsgCrsId)
        myUtmCrs = QgsCoordinateReferenceSystem()
        myUtmCrs.createFromId(32756, QgsCoordinateReferenceSystem.EpsgCrsId)
        myXForm = QgsCoordinateTransform(myGeoCrs, myUtmCrs)
        myProjectedExtent = myXForm.transformBoundingBox(myExtent)
        #myProjectedExtent.xMinimum()
        #myProjectedExtent.xMaximum()
        myExpectedExtent = ''
        myMessage = ('Expected:\n%s\nGot:\n%s\n' %
                      ( myExpectedExtent,
                        myProjectedExtent.toString())
                      
        assert myExpectedExtent == myProjectedExtent, myMessage

if __name__ == '__main__':
    unittest.main()

