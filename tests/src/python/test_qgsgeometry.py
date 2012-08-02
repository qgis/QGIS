import unittest

from qgis.core import (QgsGeometry, QGis)

# Convenience instances in case you may need them
# not used in this test
#from utilities import getQgisTestApp
#QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()

class TestQgsGeometry(unittest.TestCase):

    def testWktPointLoading(self):
        myWKT='POINT(10 10)'
        myGeometry = QgsGeometry.fromWkt(myWKT)
        myMessage = ('Expected:\n%s\nGot:\n%s\n' %
                      (QGis.Point, myGeometry.type()))
        assert myGeometry.type() == QGis.Point, myMessage


if __name__ == '__main__':
    unittest.main()

