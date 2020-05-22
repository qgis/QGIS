# -*- coding: utf-8 -*-
"""QGIS Unit tests for the OGR/Geopackage provider.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2020-05-14'
__copyright__ = 'Copyright 2020, The QGIS Project'

import os
import re
import tempfile
import shutil
import glob
import osgeo.gdal
import osgeo.ogr
import sys

from osgeo import gdal
from qgis.core import (
    QgsApplication,
    QgsSettings,
    QgsFeature,
    QgsField,
    QgsGeometry,
    QgsVectorLayer,
    QgsFeatureRequest,
    QgsVectorDataProvider,
    QgsWkbTypes,
    QgsVectorLayerExporter,
)
from qgis.PyQt.QtCore import QVariant
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath
from providertestbase import ProviderTestCase

start_app()
TEST_DATA_DIR = unitTestDataPath()


def GDAL_COMPUTE_VERSION(maj, min, rev):
    return ((maj) * 1000000 + (min) * 10000 + (rev) * 100)


class ErrorReceiver():

    def __init__(self):
        self.msg = None

    def receiveError(self, msg):
        self.msg = msg


class TestPyQgsGpkgProvider(unittest.TestCase, ProviderTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        # Create test layer
        cls.basetestpath = tempfile.mkdtemp()
        cls.repackfilepath = tempfile.mkdtemp()

        srcpath = os.path.join(TEST_DATA_DIR, 'provider')
        shutil.copy(os.path.join(srcpath, 'geopackage.gpkg'), cls.basetestpath)
        shutil.copy(os.path.join(srcpath, 'geopackage_poly.gpkg'), cls.basetestpath)
        cls.basetestfile = os.path.join(cls.basetestpath, 'geopackage.gpkg')
        cls.basetestpolyfile = os.path.join(cls.basetestpath, 'geopackage_poly.gpkg')
        cls.vl = QgsVectorLayer(cls.basetestfile, 'test', 'ogr')
        assert(cls.vl.isValid())
        cls.source = cls.vl.dataProvider()
        cls.vl_poly = QgsVectorLayer(cls.basetestpolyfile, 'test', 'ogr')
        assert (cls.vl_poly.isValid())
        cls.poly_provider = cls.vl_poly.dataProvider()

        cls.dirs_to_cleanup = [cls.basetestpath, cls.repackfilepath]

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        for dirname in cls.dirs_to_cleanup:
            shutil.rmtree(dirname, True)

    def getSource(self):
        tmpdir = tempfile.mkdtemp()
        self.dirs_to_cleanup.append(tmpdir)
        srcpath = os.path.join(TEST_DATA_DIR, 'provider')
        shutil.copy(os.path.join(srcpath, 'geopackage.gpkg'), tmpdir)
        datasource = os.path.join(tmpdir, 'geopackage.gpkg')

        vl = QgsVectorLayer(datasource, 'test', 'ogr')
        return vl

    def enableCompiler(self):
        QgsSettings().setValue('/qgis/compileExpressions', True)
        return True

    def disableCompiler(self):
        QgsSettings().setValue('/qgis/compileExpressions', False)

    def treat_time_as_string(self):
        return True

    def uncompiledFilters(self):
        return set(['cnt = 10 ^ 2',
                    '"name" ~ \'[OP]ra[gne]+\'',
                    'sqrt(pk) >= 2',
                    'radians(cnt) < 2',
                    'degrees(pk) <= 200',
                    'cos(pk) < 0',
                    'sin(pk) < 0',
                    'tan(pk) < 0',
                    'acos(-1) < pk',
                    'asin(1) < pk',
                    'atan(3.14) < pk',
                    'atan2(3.14, pk) < 1',
                    'exp(pk) < 10',
                    'ln(pk) <= 1',
                    'log(3, pk) <= 1',
                    'log10(pk) < 0.5',
                    'floor(3.14) <= pk',
                    'ceil(3.14) <= pk',
                    'pk < pi()',
                    'floor(cnt / 66.67) <= 2',
                    'ceil(cnt / 66.67) <= 2',
                    'pk < pi() / 2',
                    'x($geometry) < -70',
                    'y($geometry) > 70',
                    'xmin($geometry) < -70',
                    'ymin($geometry) > 70',
                    'xmax($geometry) < -70',
                    'ymax($geometry) > 70',
                    'disjoint($geometry,geom_from_wkt( \'Polygon ((-72.2 66.1, -65.2 66.1, -65.2 72.0, -72.2 72.0, -72.2 66.1))\'))',
                    'intersects($geometry,geom_from_wkt( \'Polygon ((-72.2 66.1, -65.2 66.1, -65.2 72.0, -72.2 72.0, -72.2 66.1))\'))',
                    'contains(geom_from_wkt( \'Polygon ((-72.2 66.1, -65.2 66.1, -65.2 72.0, -72.2 72.0, -72.2 66.1))\'),$geometry)',
                    'distance($geometry,geom_from_wkt( \'Point (-70 70)\')) > 7',
                    'intersects($geometry,geom_from_gml( \'<gml:Polygon srsName="EPSG:4326"><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>-72.2,66.1 -65.2,66.1 -65.2,72.0 -72.2,72.0 -72.2,66.1</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon>\'))',
                    'x($geometry) < -70',
                    'y($geometry) > 79',
                    'xmin($geometry) < -70',
                    'ymin($geometry) < 76',
                    'xmax($geometry) > -68',
                    'ymax($geometry) > 80',
                    'area($geometry) > 10',
                    'perimeter($geometry) < 12',
                    'relate($geometry,geom_from_wkt( \'Polygon ((-68.2 82.1, -66.95 82.1, -66.95 79.05, -68.2 79.05, -68.2 82.1))\')) = \'FF2FF1212\'',
                    'relate($geometry,geom_from_wkt( \'Polygon ((-68.2 82.1, -66.95 82.1, -66.95 79.05, -68.2 79.05, -68.2 82.1))\'), \'****F****\')',
                    'crosses($geometry,geom_from_wkt( \'Linestring (-68.2 82.1, -66.95 82.1, -66.95 79.05)\'))',
                    'overlaps($geometry,geom_from_wkt( \'Polygon ((-68.2 82.1, -66.95 82.1, -66.95 79.05, -68.2 79.05, -68.2 82.1))\'))',
                    'within($geometry,geom_from_wkt( \'Polygon ((-75.1 76.1, -75.1 81.6, -68.8 81.6, -68.8 76.1, -75.1 76.1))\'))',
                    'overlaps(translate($geometry,-1,-1),geom_from_wkt( \'Polygon ((-75.1 76.1, -75.1 81.6, -68.8 81.6, -68.8 76.1, -75.1 76.1))\'))',
                    'overlaps(buffer($geometry,1),geom_from_wkt( \'Polygon ((-75.1 76.1, -75.1 81.6, -68.8 81.6, -68.8 76.1, -75.1 76.1))\'))',
                    'intersects(centroid($geometry),geom_from_wkt( \'Polygon ((-74.4 78.2, -74.4 79.1, -66.8 79.1, -66.8 78.2, -74.4 78.2))\'))',
                    'intersects(point_on_surface($geometry),geom_from_wkt( \'Polygon ((-74.4 78.2, -74.4 79.1, -66.8 79.1, -66.8 78.2, -74.4 78.2))\'))',
                    '"dt" = to_datetime(\'000www14ww13ww12www4ww5ww2020\',\'zzzwwwsswwmmwwhhwwwdwwMwwyyyy\')',
                    'to_time("time") >= make_time(12, 14, 14)',
                    'to_time("time") = to_time(\'000www14ww13ww12www\',\'zzzwwwsswwmmwwhhwww\')',
                    '"date" = to_date(\'www4ww5ww2020\',\'wwwdwwMwwyyyy\')'
                    ])

    def partiallyCompiledFilters(self):
        return set(['"name" NOT LIKE \'Ap%\'',
                    'name LIKE \'Apple\'',
                    'name LIKE \'aPple\'',
                    'name LIKE \'Ap_le\'',
                    'name LIKE \'Ap\\_le\''
                    ])


if __name__ == '__main__':
    unittest.main()
