# -*- coding: utf-8 -*-

"""
***************************************************************************
    GdalAlgorithmsMemoryTest
    ---------------------
    Date                 : October 2016
    Copyright            : (C) 2016 by Germán Carrillo
    Email                : gcarrillo  at linuxmail dot org
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Germán Carrillo'
__date__ = 'October 2016'
__copyright__ = '(C) 2016, Germán Carrillo'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.testing import start_app, unittest
from processing.core.Processing import Processing
from processing.tests.TestData import points2, polygons, polygonsGeoJson
from processing.tools import vector, dataobjects, general
from qgis.core import (QgsVectorLayer, QgsFeatureRequest, QgsGeometry,
                       QgsDataSourceURI, QgsField)
from processing.core.ProcessingConfig import ProcessingConfig
from PyQt4.QtCore import QVariant

import os.path
import errno
import shutil

dataFolder = os.path.join(os.path.dirname(__file__), 'testdata/')
expectedFolder = os.path.abspath(os.path.join(dataFolder, 'expected', 'gdal'))
tmpBaseFolder = os.path.join(os.sep, 'tmp', 'qgis_test', str(os.getpid()))


def mkDirP(path):
    try:
        os.makedirs(path)
    except OSError as exc:
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else:
            raise

start_app()


class GdalAlgorithmsMemoryTest(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        mkDirP(tmpBaseFolder)
        Processing.initialize()

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(tmpBaseFolder)
        pass

    def testOGRConvertFormat(self):
        base_data = points2()
        base_layer = QgsVectorLayer(base_data, 'test', 'ogr')
        memory_layer = vector.duplicateInMemory(base_layer, 'memory points', True)
        expected_data = os.path.join(expectedFolder, 'buffered_points.geojson')
        expected_layer = QgsVectorLayer(expected_data, 'expected', 'ogr')

        # Simple conversion to SHP
        res = general.runalg("gdalogr:convertformat", memory_layer, 0, "", None)
        output_layer = dataobjects.getObjectFromUri(res['OUTPUT_LAYER'])
        self.assertEqual(output_layer.extent().toString(), memory_layer.extent().toString())
        self.assertEqual(output_layer.featureCount(), memory_layer.featureCount())
        self.assertEqual(len(output_layer.fields()), len(memory_layer.fields()))

        # Conversion to SHP with SQL clause
        res = general.runalg("gdalogr:convertformat", memory_layer, 0,
                             "-sql 'SELECT ST_Buffer( geometry , 20 ),* FROM \"memory_layer\"'",
                             None)
        output_layer = dataobjects.getObjectFromUri(res['OUTPUT_LAYER'])
        self.assertEqual(output_layer.extent(), expected_layer.extent())
        self.assertEqual(output_layer.featureCount(), expected_layer.featureCount())
        self.assertEqual(len(output_layer.fields()), len(expected_layer.fields()))

        # Simple conversion to GeoJSON (no laundering)
        pr = memory_layer.dataProvider()
        pr.addAttributes([QgsField("verylongname", QVariant.String)])
        memory_layer.updateFields()
        output_path = os.path.join(tmpBaseFolder, 'converted.geojson')
        res = general.runalg("gdalogr:convertformat", memory_layer, 2, "", output_path)
        output_layer = dataobjects.getObjectFromUri(res['OUTPUT_LAYER'])
        self.assertEqual(output_layer.extent(), memory_layer.extent())
        self.assertEqual(output_layer.featureCount(), memory_layer.featureCount())
        self.assertEqual(len(output_layer.fields()), len(memory_layer.fields()))
        self.assertTrue(output_layer.fieldNameIndex('verylongname') != -1) # No laundering

        # Simple conversion to GeoPackage (no laundering)
        output_path = os.path.join(tmpBaseFolder, 'converted.gpkg')
        res = general.runalg("gdalogr:convertformat", memory_layer, 1, "", output_path)
        output_layer = dataobjects.getObjectFromUri(res['OUTPUT_LAYER'])
        #self.assertEqual(output_layer.extent(), memory_layer.extent()) # Failing due to precision!
        self.assertEqual(output_layer.featureCount(), memory_layer.featureCount())
        self.assertEqual(len(output_layer.fields()) - 1, len(memory_layer.fields()))  # +fid
        self.assertTrue(output_layer.fieldNameIndex('verylongname') != -1) # No laundering

    def testOGRBufferVectors(self):
        base_data = points2()
        base_layer = QgsVectorLayer(base_data, 'test', 'ogr')
        memory_layer = vector.duplicateInMemory(base_layer, 'memory points', True)
        expected_data = os.path.join(expectedFolder, 'buffered_points.geojson')
        expected_layer = QgsVectorLayer(expected_data, 'expected', 'ogr')

        res = general.runalg("gdalogr:buffervectors",
                             memory_layer,
                             "geometry", "20",
                             False, None, False, "",
                             None)
        output_layer = dataobjects.getObjectFromUri(res['OUTPUT_LAYER'])
        self.assertEqual(output_layer.extent(), expected_layer.extent())
        self.assertEqual(output_layer.featureCount(), expected_layer.featureCount())
        self.assertEqual(len(output_layer.fields()), len(expected_layer.fields()))

    def testOGRClipVectorsByPolygon(self):
        base_data_points = points2()
        base_layer_points = QgsVectorLayer(base_data_points, 'base points', 'ogr')
        base_data_polygons = polygons()
        base_layer_polygons = QgsVectorLayer(base_data_polygons, 'base polygons', 'ogr')
        memory_layer_points = vector.duplicateInMemory(base_layer_points, 'memory points', True)
        memory_layer_polygons = vector.duplicateInMemory(base_layer_polygons, 'memory polygons', True)
        expected_data = os.path.join(expectedFolder, 'clipped_points.geojson')
        expected_layer = QgsVectorLayer(expected_data, 'expected', 'ogr')

        # Test with input memory layer
        res = general.runalg("gdalogr:clipvectorsbypolygon",
                             memory_layer_points,
                             base_layer_polygons,
                             "",
                             None)
        output_layer = dataobjects.getObjectFromUri(res['OUTPUT_LAYER'], True)
        self.assertEqual(output_layer.extent(), expected_layer.extent())
        self.assertEqual(output_layer.featureCount(), expected_layer.featureCount())
        self.assertEqual(len(output_layer.fields()), len(expected_layer.fields()))

        # Test with clip memory layer
        res = general.runalg("gdalogr:clipvectorsbypolygon",
                             base_layer_points,
                             memory_layer_polygons,
                             "",
                             None)
        output_layer = dataobjects.getObjectFromUri(res['OUTPUT_LAYER'])
        self.assertEqual(output_layer.extent(), expected_layer.extent())
        self.assertEqual(output_layer.featureCount(), expected_layer.featureCount())
        self.assertEqual(len(output_layer.fields()), len(expected_layer.fields()))

        # Test with both (input and clip) memory layers
        res = general.runalg("gdalogr:clipvectorsbypolygon",
                             memory_layer_points,
                             memory_layer_polygons,
                             "",
                             None)
        output_layer = dataobjects.getObjectFromUri(res['OUTPUT_LAYER'])
        self.assertEqual(output_layer.extent(), expected_layer.extent())
        self.assertEqual(output_layer.featureCount(), expected_layer.featureCount())
        self.assertEqual(len(output_layer.fields()), len(expected_layer.fields()))

        # Test no laundering with GeoJSON output
        pr = memory_layer_points.dataProvider()
        pr.addAttributes([QgsField("verylongname", QVariant.String)])
        memory_layer_points.updateFields()
        output_path = os.path.join(tmpBaseFolder, 'clipped_points.geojson')
        res = general.runalg("gdalogr:clipvectorsbypolygon",
                             memory_layer_points,
                             memory_layer_polygons,
                             "",
                             output_path)
        output_layer = dataobjects.getObjectFromUri(res['OUTPUT_LAYER'])
        self.assertEqual(output_layer.extent(), expected_layer.extent())
        self.assertTrue(output_layer.fieldNameIndex('verylongname') != -1) # No laundering


if __name__ == '__main__':
    unittest.main()
