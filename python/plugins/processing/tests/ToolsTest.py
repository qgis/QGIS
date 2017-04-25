# -*- coding: utf-8 -*-

"""
***************************************************************************
    ToolsTest
    ---------------------
    Date                 : July 2016
    Copyright            : (C) 2016 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Nyall Dawson'
__date__ = 'July 2016'
__copyright__ = '(C) 2016, Nyall Dawson'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import shutil
import tempfile

from qgis.core import (QgsVectorLayer,
                       QgsFeatureRequest,
                       QgsProcessingContext)
from qgis.testing import start_app, unittest

from processing.tests.TestData import points
from processing.tools import vector

testDataPath = os.path.join(os.path.dirname(__file__), 'testdata')

start_app()


class VectorTest(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.cleanup_paths = []

    @classmethod
    def tearDownClass(cls):
        for path in cls.cleanup_paths:
            shutil.rmtree(path)

    def testFeatures(self):
        test_data = points()
        test_layer = QgsVectorLayer(test_data, 'test', 'ogr')

        context = QgsProcessingContext()

        # disable check for geometry validity
        context.setFlags(QgsProcessingContext.Flags(0))

        # test with all features
        features = vector.features(test_layer, context)
        self.assertEqual(len(features), 9)
        self.assertEqual(set([f.id() for f in features]), set([0, 1, 2, 3, 4, 5, 6, 7, 8]))

        # test with selected features
        context.setFlags(QgsProcessingContext.UseSelection)
        test_layer.selectByIds([2, 4, 6])
        features = vector.features(test_layer, context)
        self.assertEqual(len(features), 3)
        self.assertEqual(set([f.id() for f in features]), set([2, 4, 6]))

        # selection, but not using selected features
        context.setFlags(QgsProcessingContext.Flags(0))
        test_layer.selectByIds([2, 4, 6])
        features = vector.features(test_layer, context)
        self.assertEqual(len(features), 9)
        self.assertEqual(set([f.id() for f in features]), set([0, 1, 2, 3, 4, 5, 6, 7, 8]))

        # using selected features, but no selection
        context.setFlags(QgsProcessingContext.UseSelection)
        test_layer.removeSelection()
        features = vector.features(test_layer, context)
        self.assertEqual(len(features), 9)
        self.assertEqual(set([f.id() for f in features]), set([0, 1, 2, 3, 4, 5, 6, 7, 8]))

        # test that feature request is honored
        context.setFlags(QgsProcessingContext.Flags(0))
        features = vector.features(test_layer, context, QgsFeatureRequest().setFilterFids([1, 3, 5]))
        self.assertEqual(set([f.id() for f in features]), set([1, 3, 5]))

        # test that feature request is honored when using selections
        context.setFlags(QgsProcessingContext.UseSelection)
        test_layer.selectByIds([2, 4, 6])
        features = vector.features(test_layer, context, QgsFeatureRequest().setFlags(QgsFeatureRequest.NoGeometry))
        self.assertTrue(all([not f.hasGeometry() for f in features]))
        features = vector.features(test_layer, context, QgsFeatureRequest().setFlags(QgsFeatureRequest.NoGeometry))
        self.assertEqual(set([f.id() for f in features]), set([2, 4, 6]))

        # test exception is raised when filtering invalid geoms
        context.setInvalidGeometryCheck(QgsFeatureRequest.GeometryAbortOnInvalid)
        #test_layer_invalid_geoms = QgsVectorLayer(invalid_geometries(), 'test', 'ogr')
        #with self.assertRaises(GeoAlgorithmExecutionException):
        #    features = vector.features(test_layer_invalid_geoms, context)
        #    feats = [f for f in features]

    def testValues(self):
        context = QgsProcessingContext()

        # disable check for geometry validity
        context.setFlags(QgsProcessingContext.Flags(0))

        test_data = points()
        test_layer = QgsVectorLayer(test_data, 'test', 'ogr')

        # field by index
        res = vector.values(test_layer, context, 1)
        self.assertEqual(res[1], [1, 2, 3, 4, 5, 6, 7, 8, 9])

        # field by name
        res = vector.values(test_layer, context, 'id')
        self.assertEqual(res['id'], [1, 2, 3, 4, 5, 6, 7, 8, 9])

        # two fields
        res = vector.values(test_layer, context, 1, 2)
        self.assertEqual(res[1], [1, 2, 3, 4, 5, 6, 7, 8, 9])
        self.assertEqual(res[2], [2, 1, 0, 2, 1, 0, 0, 0, 0])

        # two fields by name
        res = vector.values(test_layer, context, 'id', 'id2')
        self.assertEqual(res['id'], [1, 2, 3, 4, 5, 6, 7, 8, 9])
        self.assertEqual(res['id2'], [2, 1, 0, 2, 1, 0, 0, 0, 0])

        # two fields by name and index
        res = vector.values(test_layer, context, 'id', 2)
        self.assertEqual(res['id'], [1, 2, 3, 4, 5, 6, 7, 8, 9])
        self.assertEqual(res[2], [2, 1, 0, 2, 1, 0, 0, 0, 0])

        # test with selected features
        context.setFlags(QgsProcessingContext.UseSelection)
        test_layer.selectByIds([2, 4, 6])
        res = vector.values(test_layer, context, 1)
        self.assertEqual(set(res[1]), set([5, 7, 3]))

    def testUniqueValues(self):

        context = QgsProcessingContext()
        # disable check for geometry validity
        context.setFlags(QgsProcessingContext.Flags(0))

        test_data = points()
        test_layer = QgsVectorLayer(test_data, 'test', 'ogr')

        # field by index
        v = vector.uniqueValues(test_layer, context, 2)
        self.assertEqual(len(v), len(set(v)))
        self.assertEqual(set(v), set([2, 1, 0]))

        # field by name
        v = vector.uniqueValues(test_layer, context, 'id2')
        self.assertEqual(len(v), len(set(v)))
        self.assertEqual(set(v), set([2, 1, 0]))

        # test with selected features
        context.setFlags(QgsProcessingContext.UseSelection)
        test_layer.selectByIds([2, 4, 6])
        v = vector.uniqueValues(test_layer, context, 'id')
        self.assertEqual(len(v), len(set(v)))
        self.assertEqual(set(v), set([5, 7, 3]))

    def testOgrLayerNameExtraction(self):
        outdir = tempfile.mkdtemp()
        self.cleanup_paths.append(outdir)

        def _copyFile(dst):
            shutil.copyfile(os.path.join(testDataPath, 'custom', 'grass7', 'weighted.csv'), dst)

        # OGR provider - single layer
        _copyFile(os.path.join(outdir, 'a.csv'))
        name = vector.ogrLayerName(outdir)
        self.assertEqual(name, 'a')

        # OGR provider - multiple layers
        _copyFile(os.path.join(outdir, 'b.csv'))
        name1 = vector.ogrLayerName(outdir + '|layerid=0')
        name2 = vector.ogrLayerName(outdir + '|layerid=1')
        self.assertEqual(sorted([name1, name2]), ['a', 'b'])

        name = vector.ogrLayerName(outdir + '|layerid=2')
        self.assertIsNone(name)

        # OGR provider - layername takes precedence
        name = vector.ogrLayerName(outdir + '|layername=f')
        self.assertEqual(name, 'f')

        name = vector.ogrLayerName(outdir + '|layerid=0|layername=f')
        self.assertEqual(name, 'f')

        name = vector.ogrLayerName(outdir + '|layername=f|layerid=0')
        self.assertEqual(name, 'f')

        # SQLiite provider
        name = vector.ogrLayerName('dbname=\'/tmp/x.sqlite\' table="t" (geometry) sql=')
        self.assertEqual(name, 't')

        # PostgreSQL provider
        name = vector.ogrLayerName('port=5493 sslmode=disable key=\'edge_id\' srid=0 type=LineString table="city_data"."edge" (geom) sql=')
        self.assertEqual(name, 'city_data.edge')


if __name__ == '__main__':
    unittest.main()
