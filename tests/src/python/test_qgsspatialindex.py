# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsSpatialIndex.

.. note:: This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
"""
__author__ = 'Alexander Bruy'
__date__ = '20/01/2011'
__copyright__ = 'Copyright 2012, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.core import (QgsSpatialIndex,
                       QgsFeature,
                       QgsGeometry,
                       QgsRectangle,
                       QgsPoint)

from qgis.testing import start_app, unittest

start_app()


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
        myExpectedValue = 4
        myValue = len(fids)
        myMessage = 'Expected: %s Got: %s' % (myExpectedValue, myValue)
        self.assertEqual(myValue, myExpectedValue, myMessage)
        fids.sort()
        myMessage = ('Expected: %s\nGot: %s\n' %
                     ([1, 2, 5, 6], fids))
        assert fids == [1, 2, 5, 6], myMessage

        # nearest neighbor test
        fids = idx.nearestNeighbor(QgsPoint(8.75, 6.25), 3)
        myExpectedValue = 0
        myValue = len(fids)
        myMessage = 'Expected: %s Got: %s' % (myExpectedValue, myValue)

        fids.sort()
        myMessage = ('Expected: %s\nGot: %s\n' %
                     ([0, 1, 5], fids))
        assert fids == [0, 1, 5], myMessage
