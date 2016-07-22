# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsCrsCache.

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

from qgis.core import (QgsCrsCache,
                       QgsCoordinateReferenceSystem,
                       Qgis,
                       QgsUnitTypes
                       )

from qgis.testing import start_app, unittest
from qgis.PyQt.QtCore import QLocale

# Convenience instances in case you may need them
# not used in this test

start_app()


class TestQgsCrsCache(unittest.TestCase):

    def testInstance(self):
        """ test retrieving global instance """
        self.assertTrue(QgsCrsCache.instance())

    def testcrsByOgcWmsCrs(self):
        """ test retrieving CRS from cache using Ogc WMS definition """

        crs = QgsCrsCache.instance().crsByOgcWmsCrs('EPSG:4326')
        self.assertTrue(crs.isValid())
        self.assertEqual(crs.authid(), 'EPSG:4326')
        # a second time, so crs is fetched from cache
        crs = QgsCrsCache.instance().crsByOgcWmsCrs('EPSG:4326')
        self.assertTrue(crs.isValid())
        self.assertEqual(crs.authid(), 'EPSG:4326')

        # invalid
        crs = QgsCrsCache.instance().crsByOgcWmsCrs('i am not a CRS')
        self.assertFalse(crs.isValid())
        # a second time, so invalid crs is fetched from cache
        crs = QgsCrsCache.instance().crsByOgcWmsCrs('i am not a CRS')
        self.assertFalse(crs.isValid())

    def testcrsByEpsgId(self):
        """ test retrieving CRS from cache using EPSG id """

        crs = QgsCrsCache.instance().crsByEpsgId(3111)
        self.assertTrue(crs.isValid())
        self.assertEqual(crs.authid(), 'EPSG:3111')
        # a second time, so crs is fetched from cache
        crs = QgsCrsCache.instance().crsByEpsgId(3111)
        self.assertTrue(crs.isValid())
        self.assertEqual(crs.authid(), 'EPSG:3111')

        # invalid
        crs = QgsCrsCache.instance().crsByEpsgId(-9999)
        self.assertFalse(crs.isValid())
        # a second time, so invalid crs is fetched from cache
        crs = QgsCrsCache.instance().crsByEpsgId(-9999)
        self.assertFalse(crs.isValid())

    def testcrsByProj4(self):
        """ test retrieving CRS from cache using proj4 """

        crs = QgsCrsCache.instance().crsByProj4('+proj=utm +zone=55 +south +ellps=GRS80 +towgs84=0,0,0,0,0,0,0 +units=m +no_defs ')
        self.assertTrue(crs.isValid())
        self.assertEqual(crs.authid(), 'EPSG:28355')
        # a second time, so crs is fetched from cache
        crs = QgsCrsCache.instance().crsByProj4('+proj=utm +zone=55 +south +ellps=GRS80 +towgs84=0,0,0,0,0,0,0 +units=m +no_defs ')
        self.assertTrue(crs.isValid())
        self.assertEqual(crs.authid(), 'EPSG:28355')

        # invalid
        crs = QgsCrsCache.instance().crsByProj4('asdasdasd')
        self.assertFalse(crs.isValid())
        # a second time, so invalid crs is fetched from cache
        crs = QgsCrsCache.instance().crsByProj4('asdasdasd')
        self.assertFalse(crs.isValid())

    def testcrsByWkt(self):
        """ test retrieving CRS from cache using wkt """

        # EPSG3111
        wkt = 'PROJCS["GDA94 / Vicgrid94",GEOGCS["GDA94",DATUM["Geocentric_Datum_of_Australia_1994",SPHEROID["GRS 1980",6378137,298.257222101,AUTHORITY["EPSG","7019"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY["EPSG","6283"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree",0.01745329251994328,AUTHORITY["EPSG","9122"]],AUTHORITY["EPSG","4283"]],UNIT["metre",1,AUTHORITY["EPSG","9001"]],PROJECTION["Lambert_Conformal_Conic_2SP"],PARAMETER["standard_parallel_1",-36],PARAMETER["standard_parallel_2",-38],PARAMETER["latitude_of_origin",-37],PARAMETER["central_meridian",145],PARAMETER["false_easting",2500000],PARAMETER["false_northing",2500000],AUTHORITY["EPSG","3111"],AXIS["Easting",EAST],AXIS["Northing",NORTH]]'

        crs = QgsCrsCache.instance().crsByWkt(wkt)
        self.assertTrue(crs.isValid())
        self.assertEqual(crs.authid(), 'EPSG:3111')
        # a second time, so crs is fetched from cache
        crs = QgsCrsCache.instance().crsByWkt(wkt)
        self.assertTrue(crs.isValid())
        self.assertEqual(crs.authid(), 'EPSG:3111')

        # invalid
        crs = QgsCrsCache.instance().crsByWkt('asdasdasd')
        self.assertFalse(crs.isValid())
        # a second time, so invalid crs is fetched from cache
        crs = QgsCrsCache.instance().crsByWkt('asdasdasd')
        self.assertFalse(crs.isValid())

    def testcrsBySrsId(self):
        """ test retrieving CRS from cache using srs id """

        crs = QgsCrsCache.instance().crsBySrsId(3452)
        self.assertTrue(crs.isValid())
        self.assertEqual(crs.authid(), 'EPSG:4326')
        # a second time, so crs is fetched from cache
        crs = QgsCrsCache.instance().crsBySrsId(3452)
        self.assertTrue(crs.isValid())
        self.assertEqual(crs.authid(), 'EPSG:4326')

        # invalid
        crs = QgsCrsCache.instance().crsBySrsId(-9999)
        self.assertFalse(crs.isValid())
        # a second time, so invalid crs is fetched from cache
        crs = QgsCrsCache.instance().crsBySrsId(-9999)
        self.assertFalse(crs.isValid())

if __name__ == '__main__':
    unittest.main()
