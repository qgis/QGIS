# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsCRSCache.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '06/06/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.core import (QgsCRSCache,
                       QgsCoordinateReferenceSystem,
                       QGis,
                       QgsUnitTypes
                       )

from qgis.testing import start_app, unittest
from qgis.PyQt.QtCore import QLocale

# Convenience instances in case you may need them
# not used in this test

start_app()


class TestQgsCRSCache(unittest.TestCase):

    def testInstance(self):
        """ test retrieving global instance """
        self.assertTrue(QgsCRSCache.instance())

    def testcrsByOgcWmsCrs(self):
        """ test retrieving CRS from cache using Ogc WMS definition """

        crs = QgsCRSCache.instance().crsByOgcWmsCrs('EPSG:4326')
        self.assertTrue(crs.isValid())
        self.assertEqual(crs.authid(), 'EPSG:4326')
        # a second time, so crs is fetched from cache
        crs = QgsCRSCache.instance().crsByOgcWmsCrs('EPSG:4326')
        self.assertTrue(crs.isValid())
        self.assertEqual(crs.authid(), 'EPSG:4326')

        # invalid
        crs = QgsCRSCache.instance().crsByOgcWmsCrs('i am not a CRS')
        self.assertFalse(crs.isValid())
        # a second time, so invalid crs is fetched from cache
        crs = QgsCRSCache.instance().crsByOgcWmsCrs('i am not a CRS')
        self.assertFalse(crs.isValid())

    def testcrsByEpsgId(self):
        """ test retrieving CRS from cache using EPSG id """

        crs = QgsCRSCache.instance().crsByEpsgId(3111)
        self.assertTrue(crs.isValid())
        self.assertEqual(crs.authid(), 'EPSG:3111')
        # a second time, so crs is fetched from cache
        crs = QgsCRSCache.instance().crsByEpsgId(3111)
        self.assertTrue(crs.isValid())
        self.assertEqual(crs.authid(), 'EPSG:3111')

        # invalid
        crs = QgsCRSCache.instance().crsByEpsgId(-9999)
        self.assertFalse(crs.isValid())
        # a second time, so invalid crs is fetched from cache
        crs = QgsCRSCache.instance().crsByEpsgId(-9999)
        self.assertFalse(crs.isValid())

    def testcrsByProj4(self):
        """ test retrieving CRS from cache using proj4 """

        crs = QgsCRSCache.instance().crsByProj4('+proj=utm +zone=55 +south +ellps=GRS80 +towgs84=0,0,0,0,0,0,0 +units=m +no_defs ')
        self.assertTrue(crs.isValid())
        self.assertEqual(crs.authid(), 'EPSG:28355')
        # a second time, so crs is fetched from cache
        crs = QgsCRSCache.instance().crsByProj4('+proj=utm +zone=55 +south +ellps=GRS80 +towgs84=0,0,0,0,0,0,0 +units=m +no_defs ')
        self.assertTrue(crs.isValid())
        self.assertEqual(crs.authid(), 'EPSG:28355')

        # invalid
        crs = QgsCRSCache.instance().crsByProj4('asdasdasd')
        self.assertFalse(crs.isValid())
        # a second time, so invalid crs is fetched from cache
        crs = QgsCRSCache.instance().crsByProj4('asdasdasd')
        self.assertFalse(crs.isValid())

    def testcrsBySrsId(self):
        """ test retrieving CRS from cache using srs id """

        crs = QgsCRSCache.instance().crsBySrsId(3452)
        self.assertTrue(crs.isValid())
        self.assertEqual(crs.authid(), 'EPSG:4326')
        # a second time, so crs is fetched from cache
        crs = QgsCRSCache.instance().crsBySrsId(3452)
        self.assertTrue(crs.isValid())
        self.assertEqual(crs.authid(), 'EPSG:4326')

        # invalid
        crs = QgsCRSCache.instance().crsBySrsId(-9999)
        self.assertFalse(crs.isValid())
        # a second time, so invalid crs is fetched from cache
        crs = QgsCRSCache.instance().crsBySrsId(-9999)
        self.assertFalse(crs.isValid())

if __name__ == '__main__':
    unittest.main()
