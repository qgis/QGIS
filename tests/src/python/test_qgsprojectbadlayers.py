# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsProject bad layers handling.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
from builtins import chr
from builtins import range
__author__ = 'Alessandro Pasotti'
__date__ = '20/10/2018'
__copyright__ = 'Copyright 2018, The QGIS Project'

import os
import filecmp

import qgis  # NOQA

from qgis.core import (QgsProject,
                       QgsVectorLayer,
                       QgsCoordinateTransform,
                       QgsMapSettings,
                       QgsRasterLayer,
                       QgsMapLayer,
                       QgsRectangle,
                       QgsDataProvider,
                       QgsReadWriteContext,
                       QgsCoordinateReferenceSystem,
                       )
from qgis.gui import (QgsLayerTreeMapCanvasBridge,
                      QgsMapCanvas)

from qgis.PyQt.QtGui import QFont, QColor
from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtCore import QT_VERSION_STR, QTemporaryDir, QSize
from qgis.PyQt.QtXml import QDomDocument, QDomNode

from qgis.testing import start_app, unittest
from utilities import (unitTestDataPath, renderMapToImage)
from shutil import copyfile

app = start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsProjectBadLayers(unittest.TestCase):

    def setUp(self):
        p = QgsProject.instance()
        p.removeAllMapLayers()

    @classmethod
    def getBaseMapSettings(cls):
        """
        :rtype: QgsMapSettings
        """
        ms = QgsMapSettings()
        crs = QgsCoordinateReferenceSystem()
        """:type: QgsCoordinateReferenceSystem"""
        crs.createFromSrid(4326)
        ms.setBackgroundColor(QColor(152, 219, 249))
        ms.setOutputSize(QSize(420, 280))
        ms.setOutputDpi(72)
        ms.setFlag(QgsMapSettings.Antialiasing, True)
        ms.setFlag(QgsMapSettings.UseAdvancedEffects, False)
        ms.setFlag(QgsMapSettings.ForceVectorOutput, False)  # no caching?
        ms.setDestinationCrs(crs)
        return ms

    def _change_data_source(self, layer, datasource, provider_key):
        """Due to the fact that a project r/w context is not available inside
        the map layers classes, the original style and subset string restore
        happens in app, this function replicates app behavior"""

        options = QgsDataProvider.ProviderOptions()

        subset_string = ''
        if not layer.isValid():
            try:
                subset_string = layer.dataProvider().subsetString()
            except:
                pass

        layer.setDataSource(datasource, layer.name(), provider_key, options)

        if subset_string:
            layer.setSubsetString(subset_string)

        self.assertTrue(layer.originalXmlProperties(), layer.name())
        context = QgsReadWriteContext()
        context.setPathResolver(QgsProject.instance().pathResolver())
        errorMsg = ''
        doc = QDomDocument()
        self.assertTrue(doc.setContent(layer.originalXmlProperties()))
        layer_node = QDomNode(doc.firstChild())
        self.assertTrue(layer.readSymbology(layer_node, errorMsg, context))

    def test_project_roundtrip(self):
        """Tests that a project with bad layers can be saved and restored"""

        p = QgsProject.instance()
        temp_dir = QTemporaryDir()
        for ext in ('shp', 'dbf', 'shx', 'prj'):
            copyfile(os.path.join(TEST_DATA_DIR, 'lines.%s' % ext), os.path.join(temp_dir.path(), 'lines.%s' % ext))
        copyfile(os.path.join(TEST_DATA_DIR, 'raster', 'band1_byte_ct_epsg4326.tif'), os.path.join(temp_dir.path(), 'band1_byte_ct_epsg4326.tif'))
        copyfile(os.path.join(TEST_DATA_DIR, 'raster', 'band1_byte_ct_epsg4326.tif'), os.path.join(temp_dir.path(), 'band1_byte_ct_epsg4326_copy.tif'))
        l = QgsVectorLayer(os.path.join(temp_dir.path(), 'lines.shp'), 'lines', 'ogr')
        self.assertTrue(l.isValid())

        rl = QgsRasterLayer(os.path.join(temp_dir.path(), 'band1_byte_ct_epsg4326.tif'), 'raster', 'gdal')
        self.assertTrue(rl.isValid())
        rl_copy = QgsRasterLayer(os.path.join(temp_dir.path(), 'band1_byte_ct_epsg4326_copy.tif'), 'raster_copy', 'gdal')
        self.assertTrue(rl_copy.isValid())
        self.assertTrue(p.addMapLayers([l, rl, rl_copy]))

        # Save project
        project_path = os.path.join(temp_dir.path(), 'project.qgs')
        self.assertTrue(p.write(project_path))

        # Re-load the project, checking for the XML properties
        p.removeAllMapLayers()
        self.assertTrue(p.read(project_path))
        vector = list(p.mapLayersByName('lines'))[0]
        raster = list(p.mapLayersByName('raster'))[0]
        raster_copy = list(p.mapLayersByName('raster_copy'))[0]
        self.assertTrue(vector.originalXmlProperties() != '')
        self.assertTrue(raster.originalXmlProperties() != '')
        self.assertTrue(raster_copy.originalXmlProperties() != '')
        # Test setter
        raster.setOriginalXmlProperties('pippo')
        self.assertEqual(raster.originalXmlProperties(), 'pippo')

        # Now create and invalid project:
        bad_project_path = os.path.join(temp_dir.path(), 'project_bad.qgs')
        with open(project_path, 'r') as infile:
            with open(bad_project_path, 'w+') as outfile:
                outfile.write(infile.read().replace('./lines.shp', './lines-BAD_SOURCE.shp').replace('band1_byte_ct_epsg4326_copy.tif', 'band1_byte_ct_epsg4326_copy-BAD_SOURCE.tif'))

        # Load the bad project
        p.removeAllMapLayers()
        self.assertTrue(p.read(bad_project_path))
        # Check layer is invalid
        vector = list(p.mapLayersByName('lines'))[0]
        raster = list(p.mapLayersByName('raster'))[0]
        raster_copy = list(p.mapLayersByName('raster_copy'))[0]
        self.assertIsNotNone(vector.dataProvider())
        self.assertIsNotNone(raster.dataProvider())
        self.assertIsNotNone(raster_copy.dataProvider())
        self.assertFalse(vector.isValid())
        self.assertFalse(raster_copy.isValid())
        # Try a getFeatures
        self.assertEqual([f for f in vector.getFeatures()], [])
        self.assertTrue(raster.isValid())
        self.assertEqual(vector.providerType(), 'ogr')

        # Save the project
        bad_project_path2 = os.path.join(temp_dir.path(), 'project_bad2.qgs')
        p.write(bad_project_path2)
        # Re-save the project, with fixed paths
        good_project_path = os.path.join(temp_dir.path(), 'project_good.qgs')
        with open(bad_project_path2, 'r') as infile:
            with open(good_project_path, 'w+') as outfile:
                outfile.write(infile.read().replace('./lines-BAD_SOURCE.shp', './lines.shp').replace('band1_byte_ct_epsg4326_copy-BAD_SOURCE.tif', 'band1_byte_ct_epsg4326_copy.tif'))

        # Load the good project
        p.removeAllMapLayers()
        self.assertTrue(p.read(good_project_path))
        # Check layer is valid
        vector = list(p.mapLayersByName('lines'))[0]
        raster = list(p.mapLayersByName('raster'))[0]
        raster_copy = list(p.mapLayersByName('raster_copy'))[0]
        self.assertTrue(vector.isValid())
        self.assertTrue(raster.isValid())
        self.assertTrue(raster_copy.isValid())

    def test_project_relations(self):
        """Tests that a project with bad layers and relations can be saved with relations"""

        temp_dir = QTemporaryDir()
        p = QgsProject.instance()
        for ext in ('qgs', 'gpkg'):
            copyfile(os.path.join(TEST_DATA_DIR, 'projects', 'relation_reference_test.%s' % ext), os.path.join(temp_dir.path(), 'relation_reference_test.%s' % ext))

        # Load the good project
        project_path = os.path.join(temp_dir.path(), 'relation_reference_test.qgs')
        p.removeAllMapLayers()
        self.assertTrue(p.read(project_path))
        point_a = list(p.mapLayersByName('point_a'))[0]
        point_b = list(p.mapLayersByName('point_b'))[0]
        point_a_source = point_a.publicSource()
        point_b_source = point_b.publicSource()
        self.assertTrue(point_a.isValid())
        self.assertTrue(point_b.isValid())

        # Check relations
        def _check_relations():
            relation = list(p.relationManager().relations().values())[0]
            self.assertTrue(relation.isValid())
            self.assertEqual(relation.referencedLayer().id(), point_b.id())
            self.assertEqual(relation.referencingLayer().id(), point_a.id())

        _check_relations()

        # Now build a bad project
        bad_project_path = os.path.join(temp_dir.path(), 'relation_reference_test_bad.qgs')
        with open(project_path, 'r') as infile:
            with open(bad_project_path, 'w+') as outfile:
                outfile.write(infile.read().replace('./relation_reference_test.gpkg', './relation_reference_test-BAD_SOURCE.gpkg'))

        # Load the bad project
        p.removeAllMapLayers()
        self.assertTrue(p.read(bad_project_path))
        point_a = list(p.mapLayersByName('point_a'))[0]
        point_b = list(p.mapLayersByName('point_b'))[0]
        self.assertFalse(point_a.isValid())
        self.assertFalse(point_b.isValid())

        # This fails because relations are not valid anymore
        with self.assertRaises(AssertionError):
            _check_relations()

        # Changing data source, relations should be restored:
        point_a.setDataSource(point_a_source, 'point_a', 'ogr')
        point_b.setDataSource(point_b_source, 'point_b', 'ogr')
        self.assertTrue(point_a.isValid())
        self.assertTrue(point_b.isValid())

        # Check if relations were restored
        _check_relations()

        # Reload the bad project
        p.removeAllMapLayers()
        self.assertTrue(p.read(bad_project_path))
        point_a = list(p.mapLayersByName('point_a'))[0]
        point_b = list(p.mapLayersByName('point_b'))[0]
        self.assertFalse(point_a.isValid())
        self.assertFalse(point_b.isValid())

        # This fails because relations are not valid anymore
        with self.assertRaises(AssertionError):
            _check_relations()

        # Save the bad project
        bad_project_path2 = os.path.join(temp_dir.path(), 'relation_reference_test_bad2.qgs')
        p.write(bad_project_path2)

        # Now fix the bad project
        bad_project_path_fixed = os.path.join(temp_dir.path(), 'relation_reference_test_bad_fixed.qgs')
        with open(bad_project_path2, 'r') as infile:
            with open(bad_project_path_fixed, 'w+') as outfile:
                outfile.write(infile.read().replace('./relation_reference_test-BAD_SOURCE.gpkg', './relation_reference_test.gpkg'))

        # Load the fixed project
        p.removeAllMapLayers()
        self.assertTrue(p.read(bad_project_path_fixed))
        point_a = list(p.mapLayersByName('point_a'))[0]
        point_b = list(p.mapLayersByName('point_b'))[0]
        point_a_source = point_a.publicSource()
        point_b_source = point_b.publicSource()
        self.assertTrue(point_a.isValid())
        self.assertTrue(point_b.isValid())
        _check_relations()

    def testStyles(self):
        """Test that styles for rasters and vectors are kept when setDataSource is called"""

        temp_dir = QTemporaryDir()
        p = QgsProject.instance()
        for f in (
                'bad_layer_raster_test.tfw',
                'bad_layer_raster_test.tiff',
                'bad_layer_raster_test.tiff.aux.xml',
                'bad_layers_test.gpkg',
                'good_layers_test.qgs'):
            copyfile(os.path.join(TEST_DATA_DIR, 'projects', f), os.path.join(temp_dir.path(), f))

        project_path = os.path.join(temp_dir.path(), 'good_layers_test.qgs')
        p = QgsProject().instance()
        p.removeAllMapLayers()
        self.assertTrue(p.read(project_path))
        self.assertEqual(p.count(), 4)

        ms = self.getBaseMapSettings()
        point_a_copy = list(p.mapLayersByName('point_a copy'))[0]
        point_a = list(p.mapLayersByName('point_a'))[0]
        point_b = list(p.mapLayersByName('point_b'))[0]
        raster = list(p.mapLayersByName('bad_layer_raster_test'))[0]
        self.assertTrue(point_a_copy.isValid())
        self.assertTrue(point_a.isValid())
        self.assertTrue(point_b.isValid())
        self.assertTrue(raster.isValid())
        ms.setExtent(QgsRectangle(2.81861, 41.98138, 2.81952, 41.9816))
        ms.setLayers([point_a_copy, point_a, point_b, raster])
        image = renderMapToImage(ms)
        self.assertTrue(image.save(os.path.join(temp_dir.path(), 'expected.png'), 'PNG'))

        point_a_source = point_a.publicSource()
        point_b_source = point_b.publicSource()
        raster_source = raster.publicSource()
        self._change_data_source(point_a, point_a_source, 'ogr')
        # Attention: we are not passing the subset string here:
        self._change_data_source(point_a_copy, point_a_source, 'ogr')
        self._change_data_source(point_b, point_b_source, 'ogr')
        self._change_data_source(raster, raster_source, 'gdal')
        self.assertTrue(image.save(os.path.join(temp_dir.path(), 'actual.png'), 'PNG'))

        self.assertTrue(filecmp.cmp(os.path.join(temp_dir.path(), 'actual.png'), os.path.join(temp_dir.path(), 'expected.png')), False)

        # Now build a bad project
        p.removeAllMapLayers()
        bad_project_path = os.path.join(temp_dir.path(), 'bad_layers_test.qgs')
        with open(project_path, 'r') as infile:
            with open(bad_project_path, 'w+') as outfile:
                outfile.write(infile.read().replace('./bad_layers_test.', './bad_layers_test-BAD_SOURCE.').replace('bad_layer_raster_test.tiff', 'bad_layer_raster_test-BAD_SOURCE.tiff'))

        p.removeAllMapLayers()
        self.assertTrue(p.read(bad_project_path))
        self.assertEqual(p.count(), 4)
        point_a_copy = list(p.mapLayersByName('point_a copy'))[0]
        point_a = list(p.mapLayersByName('point_a'))[0]
        point_b = list(p.mapLayersByName('point_b'))[0]
        raster = list(p.mapLayersByName('bad_layer_raster_test'))[0]
        self.assertFalse(point_a.isValid())
        self.assertFalse(point_a_copy.isValid())
        self.assertFalse(point_b.isValid())
        self.assertFalse(raster.isValid())
        ms.setLayers([point_a_copy, point_a, point_b, raster])
        image = renderMapToImage(ms)
        self.assertTrue(image.save(os.path.join(temp_dir.path(), 'bad.png'), 'PNG'))
        self.assertFalse(filecmp.cmp(os.path.join(temp_dir.path(), 'bad.png'), os.path.join(temp_dir.path(), 'expected.png')), False)

        self._change_data_source(point_a, point_a_source, 'ogr')
        # We are not passing the subset string!!
        self._change_data_source(point_a_copy, point_a_source, 'ogr')
        self._change_data_source(point_b, point_b_source, 'ogr')
        self._change_data_source(raster, raster_source, 'gdal')
        self.assertTrue(point_a.isValid())
        self.assertTrue(point_a_copy.isValid())
        self.assertTrue(point_b.isValid())
        self.assertTrue(raster.isValid())

        ms.setLayers([point_a_copy, point_a, point_b, raster])
        image = renderMapToImage(ms)
        self.assertTrue(image.save(os.path.join(temp_dir.path(), 'actual_fixed.png'), 'PNG'))

        self.assertTrue(filecmp.cmp(os.path.join(temp_dir.path(), 'actual_fixed.png'), os.path.join(temp_dir.path(), 'expected.png')), False)


if __name__ == '__main__':
    unittest.main()
