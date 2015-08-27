# -*- coding: utf-8 -*-

"""
***************************************************************************
    GdalTest.py
    ---------------------
    Date                 : April 2013
    Copyright            : (C) 2013 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Victor Olaya'
__date__ = 'April 2013'
__copyright__ = '(C) 2013, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import unittest
from osgeo import gdal
from osgeo.gdalconst import GA_ReadOnly

import processing
from processing.tools import dataobjects
from processing.tools.system import getTempFilename

from processing.tests.TestData import raster, union


class GdalTest(unittest.TestCase):

    def test_gdalogrsieve(self):
        outputs = processing.runalg('gdalogr:sieve', raster(), 2, 0, None)
        output = outputs['dst_filename']
        self.assertTrue(os.path.isfile(output))
        dataset = gdal.Open(output, GA_ReadOnly)
        strhash = hash(unicode(dataset.ReadAsArray(0).tolist()))
        self.assertEqual(strhash, -1353696889)

    def test_gdalogrsieveWithUnsupportedOutputFormat(self):
        outputs = processing.runalg('gdalogr:sieve', raster(), 2, 0,
                                    getTempFilename('img'))
        output = outputs['dst_filename']
        self.assertTrue(os.path.isfile(output))
        dataset = gdal.Open(output, GA_ReadOnly)
        strhash = hash(unicode(dataset.ReadAsArray(0).tolist()))
        self.assertEqual(strhash, -1353696889)

    def test_gdalogrwarpreproject(self):
        outputs = processing.runalg(
            'gdalogr:warpreproject',
            raster(),
            'EPSG:23030',
            'EPSG:4326',
            0,
            0,
            '',
            None,
        )
        output = outputs['OUTPUT']
        self.assertTrue(os.path.isfile(output))
        dataset = gdal.Open(output, GA_ReadOnly)
        strhash = hash(unicode(dataset.ReadAsArray(0).tolist()))
        self.assertEqual(strhash, -2021328784)

    def test_gdalogrmerge(self):
        outputs = processing.runalg('gdalogr:merge', raster(), False, False,
                                    None)
        output = outputs['OUTPUT']
        self.assertTrue(os.path.isfile(output))
        dataset = gdal.Open(output, GA_ReadOnly)
        strhash = hash(unicode(dataset.ReadAsArray(0).tolist()))
        self.assertEqual(strhash, -1353696889)

    def test_gdalogrogr2ogr(self):
        outputs = processing.runalg('gdalogr:ogr2ogr', union(), 3, '', None)
        output = outputs['OUTPUT_LAYER']
        layer = dataobjects.getObjectFromUri(output, True)
        fields = layer.pendingFields()
        expectednames = [
            'id',
            'poly_num_a',
            'poly_st_a',
            'id_2',
            'poly_num_b',
            'poly_st_b',
        ]
        expectedtypes = [
            'Integer',
            'Real',
            'String',
            'Integer',
            'Real',
            'String',
        ]
        names = [unicode(f.name()) for f in fields]
        types = [unicode(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features = processing.features(layer)
        self.assertEqual(8, len(features))
        feature = features.next()
        attrs = feature.attributes()
        expectedvalues = [
            '1',
            '1.1',
            'string a',
            '2',
            '1',
            'string a',
        ]
        values = [unicode(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt = 'POLYGON((270807.08580285 4458940.1594565,270798.42294527 4458914.62661676,270780.81854858 4458914.21983449,270763.52289518 4458920.715993,270760.3449542 4458926.6570575,270763.78234766 4458958.22561242,270794.30290024 4458942.16424502,270807.08580285 4458940.1594565))'
        self.assertEqual(wkt, unicode(feature.geometry().exportToWkt()))

    def test_gdalogrogr2ogrWrongExtension(self):
        outputs = processing.runalg('gdalogr:ogr2ogr', union(), 3, '',
                                    getTempFilename('wrongext'))
        output = outputs['OUTPUT_LAYER']
        layer = dataobjects.getObjectFromUri(output, True)
        fields = layer.pendingFields()
        expectednames = [
            'id',
            'poly_num_a',
            'poly_st_a',
            'id_2',
            'poly_num_b',
            'poly_st_b',
        ]
        expectedtypes = [
            'Integer',
            'Real',
            'String',
            'Integer',
            'Real',
            'String',
        ]
        names = [unicode(f.name()) for f in fields]
        types = [unicode(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features = processing.features(layer)
        self.assertEqual(8, len(features))
        feature = features.next()
        attrs = feature.attributes()
        expectedvalues = [
            '1',
            '1.1',
            'string a',
            '2',
            '1',
            'string a',
        ]
        values = [unicode(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt = 'POLYGON((270807.08580285 4458940.1594565,270798.42294527 4458914.62661676,270780.81854858 4458914.21983449,270763.52289518 4458920.715993,270760.3449542 4458926.6570575,270763.78234766 4458958.22561242,270794.30290024 4458942.16424502,270807.08580285 4458940.1594565))'
        self.assertEqual(wkt, unicode(feature.geometry().exportToWkt()))


def suite():
    suite = unittest.makeSuite(GdalTest, 'test')
    return suite


def runtests():
    result = unittest.TestResult()
    testsuite = suite()
    testsuite.run(result)
    return result
