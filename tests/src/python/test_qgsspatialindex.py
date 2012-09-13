import unittest

from qgis.core import (QGis,
                       QgsRectangle,
                       QgsPoint)

from utilities import getQgisTestApp
QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()

class TestQgsSpatialIndex(unittest.TestCase):

    def testIndex(self):
      idx = QgsSpatialIndex()
      fid = 0
      for y in range(5, 15, 5):
        for x in range(5, 25, 5):
          ft = QgsFeature()
          ft.setFeatureId(fid)
          ft.setGeometry(QgsGeometry.fromPoint(QgsPoint(x, y)))
          idx.insertFeature(ft)
          fid += 1

      # intersection test
      rect = QgsRectangle(7.0, 3.0, 17.0, 13.0)
      fids = idx.intersects(rect)

      assert len(fids) == 0, "No intersections"

      fids.sort()
      myMessage = ('Expected: %s\nGot: %s\n' %
                    ([1, 2, 5, 6], fids))
      assert fids == [1, 2, 5, 6], myMessage

      # nearest neighbor test
      fids = idx.nearestNeighbor(QgsPoint(8.75, 6.25), 3)
      assert len(fids) == 0, "No intersections"

      fids.sort()
      myMessage = ('Expected: %s\nGot: %s\n' %
                    ([0, 1, 5], fids))
      assert fids == [0, 1, 5], myMessage
