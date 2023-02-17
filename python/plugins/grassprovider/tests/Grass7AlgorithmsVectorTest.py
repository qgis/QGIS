# -*- coding: utf-8 -*-

"""
***************************************************************************
    Grass7AlgorithmsVectorTest.py
    -----------------------------
    Date                 : April 2018
    Copyright            : (C) 2018 by Nyall Dawson
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
__date__ = 'March 2018'
__copyright__ = '(C) 2018, Nyall Dawson'

import AlgorithmsTestBase

import nose2
import shutil
import os
import tempfile
import re

from qgis.core import (QgsVectorLayer,
                       QgsApplication,
                       QgsFeature,
                       QgsGeometry,
                       QgsPointXY,
                       QgsProcessingContext,
                       QgsProject,
                       QgsProcessingFeedback,
                       QgsProcessingFeatureSourceDefinition)
from qgis.testing import (
    start_app,
    unittest
)
from grassprovider.Grass7AlgorithmProvider import Grass7AlgorithmProvider
from grassprovider.Grass7Utils import Grass7Utils


testDataPath = os.path.join(os.path.dirname(__file__), 'testdata')


class TestGrass7AlgorithmsVectorTest(unittest.TestCase, AlgorithmsTestBase.AlgorithmsTest):

    @classmethod
    def setUpClass(cls):
        start_app()
        cls.provider = Grass7AlgorithmProvider()
        QgsApplication.processingRegistry().addProvider(cls.provider)
        cls.cleanup_paths = []

        cls.temp_dir = tempfile.mkdtemp()
        cls.cleanup_paths.append(cls.temp_dir)

        assert Grass7Utils.installedVersion()

    @classmethod
    def tearDownClass(cls):
        QgsApplication.processingRegistry().removeProvider(cls.provider)
        for path in cls.cleanup_paths:
            shutil.rmtree(path)

    def test_definition_file(self):
        return 'grass7_algorithms_vector_tests.yaml'

    def testMemoryLayerInput(self):
        # create a memory layer and add to project and context
        layer = QgsVectorLayer("Point?crs=epsg:3857&field=fldtxt:string&field=fldint:integer",
                               "testmem", "memory")
        self.assertTrue(layer.isValid())
        pr = layer.dataProvider()
        f = QgsFeature()
        f.setAttributes(["test", 123])
        f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(100, 200)))
        f2 = QgsFeature()
        f2.setAttributes(["test2", 457])
        f2.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(110, 200)))
        self.assertTrue(pr.addFeatures([f, f2]))
        self.assertEqual(layer.featureCount(), 2)
        QgsProject.instance().addMapLayer(layer)
        context = QgsProcessingContext()
        context.setProject(QgsProject.instance())

        alg = QgsApplication.processingRegistry().createAlgorithmById('grass7:v.buffer')
        self.assertIsNotNone(alg)

        temp_file = os.path.join(self.temp_dir, 'grass_output.shp')
        parameters = {'input': 'testmem',
                      'cats': '',
                      'where': '',
                      'type': [0, 1, 4],
                      'distance': 1,
                      'minordistance': None,
                      'angle': 0,
                      'column': None,
                      'scale': 1,
                      'tolerance': 0.01,
                      '-s': False,
                      '-c': False,
                      '-t': False,
                      'output': temp_file,
                      'GRASS_REGION_PARAMETER': None,
                      'GRASS_SNAP_TOLERANCE_PARAMETER': -1,
                      'GRASS_MIN_AREA_PARAMETER': 0.0001,
                      'GRASS_OUTPUT_TYPE_PARAMETER': 0,
                      'GRASS_VECTOR_DSCO': '',
                      'GRASS_VECTOR_LCO': ''}
        feedback = QgsProcessingFeedback()

        results, ok = alg.run(parameters, context, feedback)
        self.assertTrue(ok)
        self.assertTrue(os.path.exists(temp_file))

        # make sure that layer has correct features
        res = QgsVectorLayer(temp_file, 'res')
        self.assertTrue(res.isValid())
        self.assertEqual(res.featureCount(), 2)

        QgsProject.instance().removeMapLayer(layer)

    def testFeatureSourceInput(self):
        # create a memory layer and add to project and context
        layer = QgsVectorLayer("Point?crs=epsg:3857&field=fldtxt:string&field=fldint:integer",
                               "testmem", "memory")
        self.assertTrue(layer.isValid())
        pr = layer.dataProvider()
        f = QgsFeature()
        f.setAttributes(["test", 123])
        f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(100, 200)))
        f2 = QgsFeature()
        f2.setAttributes(["test2", 457])
        f2.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(110, 200)))
        self.assertTrue(pr.addFeatures([f, f2]))
        self.assertEqual(layer.featureCount(), 2)

        # select first feature
        layer.selectByIds([next(layer.getFeatures()).id()])
        self.assertEqual(len(layer.selectedFeatureIds()), 1)

        QgsProject.instance().addMapLayer(layer)
        context = QgsProcessingContext()
        context.setProject(QgsProject.instance())

        alg = QgsApplication.processingRegistry().createAlgorithmById('grass7:v.buffer')
        self.assertIsNotNone(alg)
        temp_file = os.path.join(self.temp_dir, 'grass_output_sel.shp')
        parameters = {'input': QgsProcessingFeatureSourceDefinition('testmem', True),
                      'cats': '',
                      'where': '',
                      'type': [0, 1, 4],
                      'distance': 1,
                      'minordistance': None,
                      'angle': 0,
                      'column': None,
                      'scale': 1,
                      'tolerance': 0.01,
                      '-s': False,
                      '-c': False,
                      '-t': False,
                      'output': temp_file,
                      'GRASS_REGION_PARAMETER': None,
                      'GRASS_SNAP_TOLERANCE_PARAMETER': -1,
                      'GRASS_MIN_AREA_PARAMETER': 0.0001,
                      'GRASS_OUTPUT_TYPE_PARAMETER': 0,
                      'GRASS_VECTOR_DSCO': '',
                      'GRASS_VECTOR_LCO': ''}
        feedback = QgsProcessingFeedback()

        results, ok = alg.run(parameters, context, feedback)
        self.assertTrue(ok)
        self.assertTrue(os.path.exists(temp_file))

        # make sure that layer has correct features
        res = QgsVectorLayer(temp_file, 'res')
        self.assertTrue(res.isValid())
        self.assertEqual(res.featureCount(), 1)

        QgsProject.instance().removeMapLayer(layer)

    def testOutputToGeopackage(self):
        # create a memory layer and add to project and context
        layer = QgsVectorLayer("Point?crs=epsg:3857&field=fldtxt:string&field=fldint:integer",
                               "testmem", "memory")
        self.assertTrue(layer.isValid())
        pr = layer.dataProvider()
        f = QgsFeature()
        f.setAttributes(["test", 123])
        f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(100, 200)))
        f2 = QgsFeature()
        f2.setAttributes(["test2", 457])
        f2.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(110, 200)))
        self.assertTrue(pr.addFeatures([f, f2]))
        self.assertEqual(layer.featureCount(), 2)
        QgsProject.instance().addMapLayer(layer)
        context = QgsProcessingContext()
        context.setProject(QgsProject.instance())

        alg = QgsApplication.processingRegistry().createAlgorithmById('grass7:v.buffer')
        self.assertIsNotNone(alg)

        temp_file = os.path.join(self.temp_dir, 'grass_output.gpkg')
        parameters = {'input': 'testmem',
                      'cats': '',
                      'where': '',
                      'type': [0, 1, 4],
                      'distance': 1,
                      'minordistance': None,
                      'angle': 0,
                      'column': None,
                      'scale': 1,
                      'tolerance': 0.01,
                      '-s': False,
                      '-c': False,
                      '-t': False,
                      'output': temp_file,
                      'GRASS_REGION_PARAMETER': None,
                      'GRASS_SNAP_TOLERANCE_PARAMETER': -1,
                      'GRASS_MIN_AREA_PARAMETER': 0.0001,
                      'GRASS_OUTPUT_TYPE_PARAMETER': 0,
                      'GRASS_VECTOR_DSCO': '',
                      'GRASS_VECTOR_LCO': ''}
        feedback = QgsProcessingFeedback()

        results, ok = alg.run(parameters, context, feedback)
        self.assertTrue(ok)
        self.assertTrue(os.path.exists(temp_file))

        # make sure that layer has correct features
        res = QgsVectorLayer(temp_file, 'res')
        self.assertTrue(res.isValid())
        self.assertEqual(res.featureCount(), 2)

        QgsProject.instance().removeMapLayer(layer)

    def testVectorLayerInput(self):
        alg = QgsApplication.processingRegistry().createAlgorithmById('grass7:v.buffer')
        self.assertIsNotNone(alg)
        self.assertFalse(alg.commands)

        def get_command(alg):
            command = alg.commands[-1]
            command = re.sub(r'output=".*?"', 'output="###"', command)
            command = command.replace(testDataPath, 'testdata')
            return command

        # GML source
        source = os.path.join(testDataPath, 'points.gml')
        vl = QgsVectorLayer(source)
        self.assertTrue(vl.isValid())
        alg.loadVectorLayer('test_layer', vl, external=False)
        self.assertEqual(get_command(alg), 'v.in.ogr min_area=None snap=None input="testdata/points.gml" output="###" --overwrite -o')
        # try with external -- not support for GML, so should fall back to v.in.ogr
        alg.loadVectorLayer('test_layer', vl, external=True)
        self.assertEqual(get_command(alg), 'v.in.ogr min_area=None snap=None input="testdata/points.gml" output="###" --overwrite -o')

        # SHP source
        source = os.path.join(testDataPath, 'lines_z.shp')
        vl = QgsVectorLayer(source)
        self.assertTrue(vl.isValid())
        alg.loadVectorLayer('test_layer', vl, external=False)
        self.assertEqual(get_command(alg), 'v.in.ogr min_area=None snap=None input="testdata/lines_z.shp" output="###" --overwrite -o')
        # try with external -- should work for shapefile
        alg.loadVectorLayer('test_layer', vl, external=True)
        self.assertEqual(get_command(alg), 'v.external input="testdata/lines_z.shp" output="###" --overwrite -o')

        # GPKG source
        source = os.path.join(testDataPath, 'pol.gpkg')
        vl = QgsVectorLayer(source + '|layername=pol2')
        self.assertTrue(vl.isValid())
        alg.loadVectorLayer('test_layer', vl, external=False)
        self.assertEqual(get_command(alg), 'v.in.ogr min_area=None snap=None input="testdata/pol.gpkg" layer="pol2" output="###" --overwrite -o')
        # try with external -- should work for Geopackage (although grass itself tends to crash here!)
        alg.loadVectorLayer('test_layer', vl, external=True)
        self.assertEqual(get_command(alg), 'v.external input="testdata/pol.gpkg" layer="pol2" output="###" --overwrite -o')

        # different layer
        source = os.path.join(testDataPath, 'pol.gpkg')
        vl = QgsVectorLayer(source + '|layername=pol3')
        self.assertTrue(vl.isValid())
        alg.loadVectorLayer('test_layer', vl, external=False)
        self.assertEqual(get_command(alg), 'v.in.ogr min_area=None snap=None input="testdata/pol.gpkg" layer="pol3" output="###" --overwrite -o')
        alg.loadVectorLayer('test_layer', vl, external=True)
        self.assertEqual(get_command(alg), 'v.external input="testdata/pol.gpkg" layer="pol3" output="###" --overwrite -o')

        # GPKG no layer: you get what you get and you don't get upset
        source = os.path.join(testDataPath, 'pol.gpkg')
        vl = QgsVectorLayer(source)
        self.assertTrue(vl.isValid())
        alg.loadVectorLayer('test_layer', vl, external=False)
        self.assertEqual(get_command(alg), 'v.in.ogr min_area=None snap=None input="testdata/pol.gpkg" output="###" --overwrite -o')
        alg.loadVectorLayer('test_layer', vl, external=True)
        self.assertEqual(get_command(alg), 'v.external input="testdata/pol.gpkg" output="###" --overwrite -o')


if __name__ == '__main__':
    nose2.main()
