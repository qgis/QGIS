# -*- coding: utf-8 -*-

"""
***************************************************************************
    GdalAlgorithmTests.py
    ---------------------
    Date                 : January 2016
    Copyright            : (C) 2016 by Matthias Kuhn
    Email                : matthias@opengis.ch
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Matthias Kuhn'
__date__ = 'January 2016'
__copyright__ = '(C) 2016, Matthias Kuhn'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = ':%H$'

import AlgorithmsTestBase
from processing.algs.gdal.OgrToPostGis import OgrToPostGis
from processing.algs.gdal.GdalUtils import GdalUtils
from processing.algs.gdal.translate import translate
from qgis.core import (QgsProcessingContext,
                       QgsProcessingFeedback,
                       QgsApplication,
                       QgsVectorLayer,
                       QgsFeature,
                       QgsGeometry,
                       QgsPointXY,
                       QgsProject,
                       QgsProcessingUtils,
                       QgsProcessingFeatureSourceDefinition)
import nose2
import os
import shutil
import tempfile

from qgis.testing import (
    start_app,
    unittest
)

testDataPath = os.path.join(os.path.dirname(__file__), 'testdata')


class TestGdalAlgorithms(unittest.TestCase, AlgorithmsTestBase.AlgorithmsTest):

    @classmethod
    def setUpClass(cls):
        start_app()
        from processing.core.Processing import Processing
        Processing.initialize()
        cls.cleanup_paths = []

    @classmethod
    def tearDownClass(cls):
        for path in cls.cleanup_paths:
            shutil.rmtree(path)

    def test_definition_file(self):
        return 'gdal_algorithm_tests.yaml'

    def testCommandName(self):
        # Test that algorithms report a valid commandName
        p = QgsApplication.processingRegistry().providerById('gdal')
        for a in p.algorithms():
            self.assertTrue(a.commandName(), 'Algorithm {} has no commandName!'.format(a.id()))

    def testGetOgrCompatibleSourceFromMemoryLayer(self):
        # create a memory layer and add to project and context
        layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer",
                               "testmem", "memory")
        self.assertTrue(layer.isValid())
        pr = layer.dataProvider()
        f = QgsFeature()
        f.setAttributes(["test", 123])
        f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(100, 200)))
        f2 = QgsFeature()
        f2.setAttributes(["test2", 457])
        f2.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(100, 200)))
        self.assertTrue(pr.addFeatures([f, f2]))
        self.assertEqual(layer.featureCount(), 2)
        QgsProject.instance().addMapLayer(layer)
        context = QgsProcessingContext()
        context.setProject(QgsProject.instance())

        alg = QgsApplication.processingRegistry().createAlgorithmById('gdal:buffervectors')
        self.assertIsNotNone(alg)
        parameters = {'INPUT': 'testmem'}
        feedback = QgsProcessingFeedback()
        # check that memory layer is automatically saved out to shape when required by GDAL algorithms
        ogr_data_path, ogr_layer_name = alg.getOgrCompatibleSource('INPUT', parameters, context, feedback,
                                                                   executing=True)
        self.assertTrue(ogr_data_path)
        self.assertTrue(ogr_data_path.endswith('.shp'))
        self.assertTrue(os.path.exists(ogr_data_path))
        self.assertTrue(ogr_layer_name)

        # make sure that layer has correct features
        res = QgsVectorLayer(ogr_data_path, 'res')
        self.assertTrue(res.isValid())
        self.assertEqual(res.featureCount(), 2)

        QgsProject.instance().removeMapLayer(layer)

    def testGetOgrCompatibleSourceFromFeatureSource(self):
        # create a memory layer and add to project and context
        layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer",
                               "testmem", "memory")
        self.assertTrue(layer.isValid())
        pr = layer.dataProvider()
        f = QgsFeature()
        f.setAttributes(["test", 123])
        f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(100, 200)))
        f2 = QgsFeature()
        f2.setAttributes(["test2", 457])
        f2.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(100, 200)))
        self.assertTrue(pr.addFeatures([f, f2]))
        self.assertEqual(layer.featureCount(), 2)
        # select first feature
        layer.selectByIds([next(layer.getFeatures()).id()])
        self.assertEqual(len(layer.selectedFeatureIds()), 1)
        QgsProject.instance().addMapLayer(layer)
        context = QgsProcessingContext()
        context.setProject(QgsProject.instance())

        alg = QgsApplication.processingRegistry().createAlgorithmById('gdal:buffervectors')
        self.assertIsNotNone(alg)
        parameters = {'INPUT': QgsProcessingFeatureSourceDefinition('testmem', True)}
        feedback = QgsProcessingFeedback()
        # check that memory layer is automatically saved out to shape when required by GDAL algorithms
        ogr_data_path, ogr_layer_name = alg.getOgrCompatibleSource('INPUT', parameters, context, feedback,
                                                                   executing=True)
        self.assertTrue(ogr_data_path)
        self.assertTrue(ogr_data_path.endswith('.shp'))
        self.assertTrue(os.path.exists(ogr_data_path))
        self.assertTrue(ogr_layer_name)

        # make sure that layer has only selected feature
        res = QgsVectorLayer(ogr_data_path, 'res')
        self.assertTrue(res.isValid())
        self.assertEqual(res.featureCount(), 1)

        QgsProject.instance().removeMapLayer(layer)

    def testOgrLayerNameExtraction(self):
        outdir = tempfile.mkdtemp()
        self.cleanup_paths.append(outdir)

        def _copyFile(dst):
            shutil.copyfile(os.path.join(testDataPath, 'custom', 'grass7', 'weighted.csv'), dst)

        # OGR provider - single layer
        _copyFile(os.path.join(outdir, 'a.csv'))
        name = GdalUtils.ogrLayerName(outdir)
        self.assertEqual(name, 'a')

        # OGR provider - multiple layers
        _copyFile(os.path.join(outdir, 'b.csv'))
        name1 = GdalUtils.ogrLayerName(outdir + '|layerid=0')
        name2 = GdalUtils.ogrLayerName(outdir + '|layerid=1')
        self.assertEqual(sorted([name1, name2]), ['a', 'b'])

        name = GdalUtils.ogrLayerName(outdir + '|layerid=2')
        self.assertIsNone(name)

        # OGR provider - layername takes precedence
        name = GdalUtils.ogrLayerName(outdir + '|layername=f')
        self.assertEqual(name, 'f')

        name = GdalUtils.ogrLayerName(outdir + '|layerid=0|layername=f')
        self.assertEqual(name, 'f')

        name = GdalUtils.ogrLayerName(outdir + '|layername=f|layerid=0')
        self.assertEqual(name, 'f')

        # SQLiite provider
        name = GdalUtils.ogrLayerName('dbname=\'/tmp/x.sqlite\' table="t" (geometry) sql=')
        self.assertEqual(name, 't')

        # PostgreSQL provider
        name = GdalUtils.ogrLayerName(
            'port=5493 sslmode=disable key=\'edge_id\' srid=0 type=LineString table="city_data"."edge" (geom) sql=')
        self.assertEqual(name, 'city_data.edge')

    def testGdalTranslate(self):
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        source = os.path.join(testDataPath, 'dem.tif')
        translate_alg = translate()
        translate_alg.initAlgorithm()

        # with no NODATA value
        self.assertEqual(
            translate_alg.getConsoleCommands({'INPUT': source,
                                              'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_translate',
             '-ot Float32 -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])
        # with NODATA value
        self.assertEqual(
            translate_alg.getConsoleCommands({'INPUT': source,
                                              'NODATA': 9999,
                                              'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_translate',
             '-a_nodata 9999.0 ' +
             '-ot Float32 -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])
        # with "0" NODATA value
        self.assertEqual(
            translate_alg.getConsoleCommands({'INPUT': source,
                                              'NODATA': 0,
                                              'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_translate',
             '-a_nodata 0.0 ' +
             '-ot Float32 -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])
        # with target srs
        self.assertEqual(
            translate_alg.getConsoleCommands({'INPUT': source,
                                              'TARGET_CRS': 'EPSG:3111',
                                              'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_translate',
             '-a_srs EPSG:3111 ' +
             '-ot Float32 -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])
        # with target srs
        self.assertEqual(
            translate_alg.getConsoleCommands({'INPUT': source,
                                              'TARGET_CRS': 'EPSG:3111',
                                              'OUTPUT': 'd:/temp/check.jpg'}, context, feedback),
            ['gdal_translate',
             '-a_srs EPSG:3111 ' +
             '-ot Float32 -of JPEG ' +
             source + ' ' +
             'd:/temp/check.jpg'])
        # with copy subdatasets
        self.assertEqual(
            translate_alg.getConsoleCommands({'INPUT': source,
                                              'COPY_SUBDATASETS': True,
                                              'OUTPUT': 'd:/temp/check.tif'}, context, feedback),
            ['gdal_translate',
             '-sds ' +
             '-ot Float32 -of GTiff ' +
             source + ' ' +
             'd:/temp/check.tif'])


class TestGdalOgrToPostGis(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        # start_app()
        from processing.core.Processing import Processing
        Processing.initialize()

    @classmethod
    def tearDownClass(cls):
        pass

    # See https://issues.qgis.org/issues/15706
    def test_getConnectionString(self):
        obj = OgrToPostGis()
        obj.initAlgorithm({})

        parameters = {}
        context = QgsProcessingContext()

        # NOTE: defaults are debatable, see
        # https://github.com/qgis/QGIS/pull/3607#issuecomment-253971020
        self.assertEqual(obj.getConnectionString(parameters, context),
                         "host=localhost port=5432 active_schema=public")

        parameters['HOST'] = 'remote'
        self.assertEqual(obj.getConnectionString(parameters, context),
                         "host=remote port=5432 active_schema=public")

        parameters['HOST'] = ''
        self.assertEqual(obj.getConnectionString(parameters, context),
                         "port=5432 active_schema=public")

        parameters['PORT'] = '5555'
        self.assertEqual(obj.getConnectionString(parameters, context),
                         "port=5555 active_schema=public")

        parameters['PORT'] = ''
        self.assertEqual(obj.getConnectionString(parameters, context),
                         "active_schema=public")

        parameters['USER'] = 'usr'
        self.assertEqual(obj.getConnectionString(parameters, context),
                         "active_schema=public user=usr")

        parameters['PASSWORD'] = 'pwd'
        self.assertEqual(obj.getConnectionString(parameters, context),
                         "password=pwd active_schema=public user=usr")


if __name__ == '__main__':
    nose2.main()
