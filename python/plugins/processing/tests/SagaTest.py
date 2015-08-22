# -*- coding: utf-8 -*-

"""
***************************************************************************
    SagaTest.py
    ---------------------
    Date                 : March 2013
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
__date__ = 'March 2013'
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

from processing.tests.TestData import polygons2, polygonsGeoJson, raster


class SagaTest(unittest.TestCase):

    """Tests for saga algorithms"""

    def test_sagametricconversions(self):
        outputs = processing.runalg('saga:metricconversions', raster(), 0,
                                    None)
        output = outputs['CONV']
        self.assertTrue(os.path.isfile(output))
        dataset = gdal.Open(output, GA_ReadOnly)
        strhash = hash(unicode(dataset.ReadAsArray(0).tolist()))
        self.assertEqual(strhash, -2137931723)

    def test_sagasortgrid(self):
        outputs = processing.runalg('saga:sortgrid', raster(), True, None)
        output = outputs['OUTPUT']
        self.assertTrue(os.path.isfile(output))
        dataset = gdal.Open(output, GA_ReadOnly)
        strhash = hash(unicode(dataset.ReadAsArray(0).tolist()))
        self.assertEqual(strhash, 1320073153)

    def test_SagaVectorAlgorithmWithSelection(self):
        layer = processing.getObject(polygons2())
        feature = layer.getFeatures().next()
        selected = [feature.id()]
        layer.setSelectedFeatures(selected)
        outputs = processing.runalg('saga:polygoncentroids', polygons2(),
                                    True, None)
        layer.setSelectedFeatures([])
        output = outputs['CENTROIDS']
        layer = dataobjects.getObjectFromUri(output, True)
        fields = layer.pendingFields()
        expectednames = ['ID', 'POLY_NUM_B', 'POLY_ST_B']
        expectedtypes = ['Real', 'Real', 'String']
        names = [unicode(f.name()) for f in fields]
        types = [unicode(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features = processing.features(layer)
        self.assertEqual(1, len(features))
        feature = features.next()
        attrs = feature.attributes()
        expectedvalues = ['2', '1', 'string a']
        values = [unicode(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt = 'POINT(270806.69221918 4458924.97720492)'
        self.assertEqual(wkt, unicode(feature.geometry().exportToWkt()))

    def test_SagaVectorAlgorithWithUnsupportedInputAndOutputFormat(self):
        """This tests both the exporting to shp and then the format
        change in the output layer.
        """

        layer = processing.getObject(polygonsGeoJson())
        feature = layer.getFeatures().next()
        selected = [feature.id()]
        layer.setSelectedFeatures(selected)
        outputs = processing.runalg('saga:polygoncentroids',
                                    polygonsGeoJson(), True,
                                    getTempFilename('geojson'))
        layer.setSelectedFeatures([])
        output = outputs['CENTROIDS']
        layer = dataobjects.getObjectFromUri(output, True)
        fields = layer.pendingFields()
        expectednames = ['ID', 'POLY_NUM_A', 'POLY_ST_A']
        expectedtypes = ['Real', 'Real', 'String']
        names = [unicode(f.name()) for f in fields]
        types = [unicode(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features = processing.features(layer)
        self.assertEqual(1, len(features))
        feature = features.next()
        attrs = feature.attributes()
        expectedvalues = ['0', '1.1', 'string a']
        values = [unicode(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt = 'POINT(270787.49991451 4458955.46775295)'
        self.assertEqual(wkt, unicode(feature.geometry().exportToWkt()))

    def test_SagaRasterAlgorithmWithUnsupportedOutputFormat(self):
        outputs = processing.runalg('saga:convergenceindex', raster(), 0, 0,
                                    getTempFilename('img'))
        output = outputs['RESULT']
        self.assertTrue(os.path.isfile(output))
        dataset = gdal.Open(output, GA_ReadOnly)
        strhash = hash(unicode(dataset.ReadAsArray(0).tolist()))
        self.assertEqual(strhash, 485390137)


def suite():
    suite = unittest.makeSuite(SagaTest, 'test')
    return suite


def runtests():
    result = unittest.TestResult()
    testsuite = suite()
    testsuite.run(result)
    return result
