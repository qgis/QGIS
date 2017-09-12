# -*- coding: utf-8 -*-

"""
***************************************************************************
    ToolsTest
    ---------------------
    Date                 : July 2017
    Copyright            : (C) 2017 by Nyall Dawson
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

from qgis.testing import start_app, unittest
from processing.tests.TestData import points2
from processing.tools import vector
from qgis.core import (QgsVectorLayer, QgsFeatureRequest)
from processing.core.ProcessingConfig import ProcessingConfig

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
        name = vector.ogrLayerName(outdir + '|layerid=0')
        self.assertEqual(name, 'b')
        name = vector.ogrLayerName(outdir + '|layerid=1')
        self.assertEqual(name, 'a')

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

    def testFeatures(self):
        ProcessingConfig.initialize()

        test_data = points2()
        test_layer = QgsVectorLayer(test_data, 'test', 'ogr')

        # test with all features
        features = vector.features(test_layer)
        self.assertEqual(len(features), 8)
        self.assertEqual(set([f.id() for f in features]), set([0, 1, 2, 3, 4, 5, 6, 7]))

        # test with selected features
        previous_value = ProcessingConfig.getSetting(ProcessingConfig.USE_SELECTED)
        ProcessingConfig.setSettingValue(ProcessingConfig.USE_SELECTED, True)
        test_layer.selectByIds([2, 4, 6])
        features = vector.features(test_layer)
        self.assertEqual(len(features), 3)
        self.assertEqual(set([f.id() for f in features]), set([2, 4, 6]))

        # selection, but not using selected features
        ProcessingConfig.setSettingValue(ProcessingConfig.USE_SELECTED, False)
        test_layer.selectByIds([2, 4, 6])
        features = vector.features(test_layer)
        self.assertEqual(len(features), 8)
        self.assertEqual(set([f.id() for f in features]), set([0, 1, 2, 3, 4, 5, 6, 7]))

        # using selected features, but no selection
        ProcessingConfig.setSettingValue(ProcessingConfig.USE_SELECTED, True)
        test_layer.removeSelection()
        features = vector.features(test_layer)
        self.assertEqual(len(features), 8)
        self.assertEqual(set([f.id() for f in features]), set([0, 1, 2, 3, 4, 5, 6, 7]))

        # test that feature request is honored
        ProcessingConfig.setSettingValue(ProcessingConfig.USE_SELECTED, False)
        features = vector.features(test_layer, QgsFeatureRequest().setFilterFids([1, 3, 5]))
        self.assertEqual(set([f.id() for f in features]), set([1, 3, 5]))

        # test that feature request is honored when using selections
        ProcessingConfig.setSettingValue(ProcessingConfig.USE_SELECTED, True)
        test_layer.selectByIds([2, 4, 6])
        features = vector.features(test_layer, QgsFeatureRequest().setFlags(QgsFeatureRequest.NoGeometry))
        self.assertTrue(all([f.geometry() == None for f in features]))
        features = vector.features(test_layer, QgsFeatureRequest().setFlags(QgsFeatureRequest.NoGeometry))
        self.assertEqual(set([f.id() for f in features]), set([2, 4, 6]))

        ProcessingConfig.setSettingValue(ProcessingConfig.USE_SELECTED, previous_value)


if __name__ == '__main__':
    unittest.main()
