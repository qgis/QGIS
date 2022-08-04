# -*- coding: utf-8 -*-
"""QGIS Unit tests for Processing Package Layers algorithm.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Alessandro Pasotti'
__date__ = '2022-07'
__copyright__ = 'Copyright 2022, The QGIS Project'

import re
import os

from osgeo import gdal, ogr
from processing.core.Processing import Processing
from processing.gui.AlgorithmExecutor import execute
from qgis.analysis import QgsNativeAlgorithms
from qgis.core import (QgsApplication, QgsVectorLayer,
                       QgsGeometry, QgsProcessingContext,
                       QgsProcessingFeedback, QgsSettings,
                       QgsProviderRegistry, QgsProject, QgsRelation
                       )
from qgis.PyQt.QtCore import QCoreApplication, QTemporaryDir
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()


class ConsoleFeedBack(QgsProcessingFeedback):
    _errors = []

    def reportError(self, error, fatalError=False):
        print(error)
        self._errors.append(error)

    def pushInfo(self, info):
        print(info)

    def setProgressText(self, info):
        print(info)

    def pushDebugInfo(self, info):
        print(info)


class TestPackageLayers(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain(
            "QGIS_TestPyQgsPackageLayers.com")
        QCoreApplication.setApplicationName("QGIS_TestPyQgsPackageLayers")
        QgsSettings().clear()
        Processing.initialize()
        QgsApplication.processingRegistry().addProvider(QgsNativeAlgorithms())
        cls.registry = QgsApplication.instance().processingRegistry()
        cls.tmp_dir = QTemporaryDir()
        cls.temp_path = os.path.join(cls.tmp_dir.path(), 'package_layers.gpkg')
        cls.temp_export_path = os.path.join(cls.tmp_dir.path(), 'package_layers_export.gpkg')

        # Create test DB

        """
        Test data:

        Region 1
            Province 1
                City 1
                City 2
            Province 2
                City 3
        Region 2
            Province 3
            Province 4
                City 4
        """

        ds = ogr.GetDriverByName('GPKG').CreateDataSource(cls.temp_path)
        lyr = ds.CreateLayer('region', geom_type=ogr.wkbNone)
        lyr.CreateField(ogr.FieldDefn('name', ogr.OFTString))
        f = ogr.Feature(lyr.GetLayerDefn())
        f['name'] = 'region one'
        lyr.CreateFeature(f)
        f = ogr.Feature(lyr.GetLayerDefn())
        f['name'] = 'region two'
        lyr.CreateFeature(f)

        lyr = ds.CreateLayer('province', geom_type=ogr.wkbNone)
        lyr.CreateField(ogr.FieldDefn('name', ogr.OFTString))
        lyr.CreateField(ogr.FieldDefn('region', ogr.OFTInteger))
        f = ogr.Feature(lyr.GetLayerDefn())
        f['name'] = 'province one'
        f['region'] = 1
        lyr.CreateFeature(f)
        f = ogr.Feature(lyr.GetLayerDefn())
        f['name'] = 'province two'
        f['region'] = 1
        lyr.CreateFeature(f)
        f = ogr.Feature(lyr.GetLayerDefn())
        f['name'] = 'province three'
        f['region'] = 2
        lyr.CreateFeature(f)
        f = ogr.Feature(lyr.GetLayerDefn())
        f['name'] = 'province four'
        f['region'] = 2
        lyr.CreateFeature(f)

        lyr = ds.CreateLayer('city', geom_type=ogr.wkbNone)
        lyr.CreateField(ogr.FieldDefn('name', ogr.OFTString))
        lyr.CreateField(ogr.FieldDefn('province', ogr.OFTInteger))
        f = ogr.Feature(lyr.GetLayerDefn())
        f['name'] = 'city one'
        f['province'] = 1
        lyr.CreateFeature(f)
        f = ogr.Feature(lyr.GetLayerDefn())
        f['name'] = 'city two'
        f['province'] = 1
        lyr.CreateFeature(f)
        f = ogr.Feature(lyr.GetLayerDefn())
        f['name'] = 'city three'
        f['province'] = 2
        lyr.CreateFeature(f)
        f = ogr.Feature(lyr.GetLayerDefn())
        f['name'] = 'city four'
        f['province'] = 4
        lyr.CreateFeature(f)

        f = None
        ds = None

        region = QgsVectorLayer(cls.temp_path + '|layername=region', 'region')
        province = QgsVectorLayer(cls.temp_path + '|layername=province', 'province')
        city = QgsVectorLayer(cls.temp_path + '|layername=city', 'city')

        QgsProject.instance().addMapLayers([region, province, city])

        relMgr = QgsProject.instance().relationManager()

        rel = QgsRelation()
        rel.setId('rel1')
        rel.setName('province -> region')
        rel.setReferencingLayer(province.id())
        rel.setReferencedLayer(region.id())
        rel.addFieldPair('region', 'fid')
        assert rel.isValid()

        relMgr.addRelation(rel)

        rel = QgsRelation()
        rel.setId('rel2')
        rel.setName('city -> province')
        rel.setReferencingLayer(city.id())
        rel.setReferencedLayer(province.id())
        rel.addFieldPair('province', 'fid')
        assert rel.isValid()

        relMgr.addRelation(rel)

    def tearDown(self):
        super().tearDown()
        os.unlink(self.temp_export_path)

    def test_simple_export(self):
        """Test export with no selected features"""

        alg = self.registry.createAlgorithmById("qgis:package")
        self.assertIsNotNone(alg)

        def _test(parameters):

            feedback = ConsoleFeedBack()
            context = QgsProcessingContext()
            context.setProject(QgsProject.instance())
            # Note: the following returns true also in case of errors ...
            self.assertTrue(execute(alg, parameters, context, feedback))
            # ... so we check the log
            self.assertEqual(feedback._errors, [])

            # Check export
            l = QgsVectorLayer(self.temp_export_path + '|layername=province', 'province')
            self.assertTrue(l.isValid())
            self.assertEqual(l.featureCount(), 4)

            l = QgsVectorLayer(self.temp_export_path + '|layername=region', 'region')
            self.assertTrue(l.isValid())
            self.assertEqual(l.featureCount(), 2)

            l = QgsVectorLayer(self.temp_export_path + '|layername=city', 'city')
            self.assertTrue(l.isValid())
            self.assertEqual(l.featureCount(), 4)

        parameters = {
            'EXPORT_RELATED_LAYERS': True,
            'LAYERS': [QgsProject.instance().mapLayersByName('province')[0]],
            'OUTPUT': self.temp_export_path,
            'OVERWRITE': True,
            'SELECTED_FEATURES_ONLY': False
        }

        # Test province
        _test(parameters)

        # Test region
        parameters['LAYERS'] = [QgsProject.instance().mapLayersByName('region')[0]]
        _test(parameters)

        # Test city
        parameters['LAYERS'] = [QgsProject.instance().mapLayersByName('city')[0]]
        _test(parameters)

    def test_selected_features_export(self):
        """Test export with selected features"""

        alg = self.registry.createAlgorithmById("qgis:package")
        self.assertIsNotNone(alg)

        def _test(parameters, expected_ids):

            feedback = ConsoleFeedBack()
            context = QgsProcessingContext()
            context.setProject(QgsProject.instance())
            # Note: the following returns true also in case of errors ...
            self.assertTrue(execute(alg, parameters, context, feedback))
            # ... so we check the log
            self.assertEqual(feedback._errors, [])

            # Check export
            for layer_name in list(expected_ids.keys()):
                l = QgsVectorLayer(self.temp_export_path + '|layername={}'.format(layer_name), layer_name)
                self.assertTrue(l.isValid())
                ids = set([l.id() for l in l.getFeatures()])
                self.assertEqual(ids, expected_ids[layer_name], layer_name + str(ids))

        region = QgsProject.instance().mapLayersByName('region')[0]
        province = QgsProject.instance().mapLayersByName('province')[0]
        city = QgsProject.instance().mapLayersByName('city')[0]

        parameters = {
            'EXPORT_RELATED_LAYERS': True,
            'LAYERS': [province],
            'OUTPUT': self.temp_export_path,
            'OVERWRITE': True,
            'SELECTED_FEATURES_ONLY': True
        }

        # Test province
        province.selectByIds([1])
        _test(parameters, {'region': {1}, 'province': {1}, 'city': {1, 2}})
        province.selectByIds([])

        # Test region
        parameters['LAYERS'] = [region]
        region.selectByIds([1])
        _test(parameters, {'region': {1}, 'province': {1, 2}, 'city': {1, 2, 3}})
        region.selectByIds([])

        # Test city
        parameters['LAYERS'] = [city]
        city.selectByIds([3])
        _test(parameters, {'region': {1}, 'province': {2}, 'city': {3}})
        city.selectByIds([])

        # Test multiple selection
        parameters['LAYERS'] = [city, province]
        city.selectByIds([3])
        province.selectByIds([3])
        _test(parameters, {'region': {1, 2}, 'province': {2, 3}, 'city': {3}})
        city.selectByIds([])
        province.selectByIds([])

        # Test referencing with selection
        parameters['LAYERS'] = [region]
        region.selectByIds([2])
        _test(parameters, {'region': {2}, 'province': {3, 4}, 'city': {4}})
        region.selectByIds([])

        # Test referencing with selection, empty city expected not to be exported
        parameters['LAYERS'] = [province]
        province.selectByIds([3])
        _test(parameters, {'region': {2}, 'province': {3}})
        province.selectByIds([])


if __name__ == '__main__':
    unittest.main()
