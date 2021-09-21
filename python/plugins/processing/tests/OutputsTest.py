# -*- coding: utf-8 -*-

"""
***************************************************************************
    OutputsTest
    ---------------------
    Date                 : May 2021
    Copyright            : (C) 2021 by René-Luc DHONT
    Email                : rldhont at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'René-Luc DHONT'
__date__ = 'May 2021'
__copyright__ = '(C) 2021, René-Luc DHONT'

import os
import shutil

from qgis.core import (QgsProcessingParameterDefinition,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterFile,
                       QgsProcessing,
                       QgsProcessingOutputFile)
from qgis.testing import start_app, unittest

from processing.core.outputs import getOutputFromString

testDataPath = os.path.join(os.path.dirname(__file__), 'testdata')

start_app()


class OutputsTest(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.cleanup_paths = []

    @classmethod
    def tearDownClass(cls):
        for path in cls.cleanup_paths:
            shutil.rmtree(path)

    def testOutputStringDesc(self):
        desc = 'QgsProcessingOutputString|out_string|Output String'
        out = getOutputFromString(desc)
        self.assertIsNotNone(out)
        self.assertEqual(out.type(), 'outputString')
        self.assertEqual(out.name(), 'out_string')
        self.assertEqual(out.description(), 'Output String')

    def testOutputNumberDesc(self):
        desc = 'QgsProcessingOutputNumber|out_number|Output Number'
        out = getOutputFromString(desc)
        self.assertIsNotNone(out)
        self.assertEqual(out.type(), 'outputNumber')
        self.assertEqual(out.name(), 'out_number')
        self.assertEqual(out.description(), 'Output Number')

    def testOutputBooleanDesc(self):
        desc = 'QgsProcessingOutputBoolean|out_bool|Output Boolean'
        out = getOutputFromString(desc)
        self.assertIsNotNone(out)
        self.assertEqual(out.type(), 'outputBoolean')
        self.assertEqual(out.name(), 'out_bool')
        self.assertEqual(out.description(), 'Output Boolean')

    def testOutputMapLayerDesc(self):
        desc = 'QgsProcessingOutputMapLayer|out_layer|Output Layer'
        out = getOutputFromString(desc)
        self.assertIsNotNone(out)
        self.assertEqual(out.type(), 'outputLayer')
        self.assertEqual(out.name(), 'out_layer')
        self.assertEqual(out.description(), 'Output Layer')

    def testOutputVectorLayerDesc(self):
        desc = 'QgsProcessingOutputVectorLayer|out_vector|Output Vector'
        out = getOutputFromString(desc)
        self.assertIsNotNone(out)
        self.assertEqual(out.type(), 'outputVector')
        self.assertEqual(out.name(), 'out_vector')
        self.assertEqual(out.description(), 'Output Vector')
        self.assertEqual(out.dataType(), QgsProcessing.TypeVectorAnyGeometry)

        desc = 'QgsProcessingOutputVectorLayer|out_vector|Output Vector|-1'
        out = getOutputFromString(desc)
        self.assertIsNotNone(out)
        self.assertEqual(out.type(), 'outputVector')
        self.assertEqual(out.name(), 'out_vector')
        self.assertEqual(out.description(), 'Output Vector')
        self.assertEqual(out.dataType(), QgsProcessing.TypeVectorAnyGeometry)

        desc = 'QgsProcessingOutputVectorLayer|out_vector|Output Vector|QgsProcessing.TypeVectorAnyGeometry'
        out = getOutputFromString(desc)
        self.assertIsNotNone(out)
        self.assertEqual(out.type(), 'outputVector')
        self.assertEqual(out.name(), 'out_vector')
        self.assertEqual(out.description(), 'Output Vector')
        self.assertEqual(out.dataType(), QgsProcessing.TypeVectorAnyGeometry)

        desc = 'QgsProcessingOutputVectorLayer|out_points|Output Vector Point|0'
        out = getOutputFromString(desc)
        self.assertIsNotNone(out)
        self.assertEqual(out.type(), 'outputVector')
        self.assertEqual(out.name(), 'out_points')
        self.assertEqual(out.description(), 'Output Vector Point')
        self.assertEqual(out.dataType(), QgsProcessing.TypeVectorPoint)

        desc = 'QgsProcessingOutputVectorLayer|out_lines|Output Vector Line|1'
        out = getOutputFromString(desc)
        self.assertIsNotNone(out)
        self.assertEqual(out.type(), 'outputVector')
        self.assertEqual(out.name(), 'out_lines')
        self.assertEqual(out.description(), 'Output Vector Line')
        self.assertEqual(out.dataType(), QgsProcessing.TypeVectorLine)

        desc = 'QgsProcessingOutputVectorLayer|out_poly|Output Vector Polygon|2'
        out = getOutputFromString(desc)
        self.assertIsNotNone(out)
        self.assertEqual(out.type(), 'outputVector')
        self.assertEqual(out.name(), 'out_poly')
        self.assertEqual(out.description(), 'Output Vector Polygon')
        self.assertEqual(out.dataType(), QgsProcessing.TypeVectorPolygon)

        desc = 'QgsProcessingOutputVectorLayer|out_points|Output Vector Point|QgsProcessing.TypeVectorPoint'
        out = getOutputFromString(desc)
        self.assertIsNotNone(out)
        self.assertEqual(out.type(), 'outputVector')
        self.assertEqual(out.name(), 'out_points')
        self.assertEqual(out.description(), 'Output Vector Point')
        self.assertEqual(out.dataType(), QgsProcessing.TypeVectorPoint)

        desc = 'QgsProcessingOutputVectorLayer|out_lines|Output Vector Line|QgsProcessing.TypeVectorLine'
        out = getOutputFromString(desc)
        self.assertIsNotNone(out)
        self.assertEqual(out.type(), 'outputVector')
        self.assertEqual(out.name(), 'out_lines')
        self.assertEqual(out.description(), 'Output Vector Line')
        self.assertEqual(out.dataType(), QgsProcessing.TypeVectorLine)

        desc = 'QgsProcessingOutputVectorLayer|out_poly|Output Vector Polygon|QgsProcessing.TypeVectorPolygon'
        out = getOutputFromString(desc)
        self.assertIsNotNone(out)
        self.assertEqual(out.type(), 'outputVector')
        self.assertEqual(out.name(), 'out_poly')
        self.assertEqual(out.description(), 'Output Vector Polygon')
        self.assertEqual(out.dataType(), QgsProcessing.TypeVectorPolygon)

    def testOutputRasterLayerDesc(self):
        desc = 'QgsProcessingOutputRasterLayer|out_raster|Output Raster'
        out = getOutputFromString(desc)
        self.assertIsNotNone(out)
        self.assertEqual(out.type(), 'outputRaster')
        self.assertEqual(out.name(), 'out_raster')
        self.assertEqual(out.description(), 'Output Raster')

    def testOutputMultiLayersDesc(self):
        desc = 'QgsProcessingOutputMultipleLayers|out_multi_layers|Output Multi Layers'
        out = getOutputFromString(desc)
        self.assertIsNotNone(out)
        self.assertEqual(out.type(), 'outputMultilayer')
        self.assertEqual(out.name(), 'out_multi_layers')
        self.assertEqual(out.description(), 'Output Multi Layers')

    def testOutputFileDesc(self):
        desc = 'QgsProcessingOutputFile|out_file|Output File'
        out = getOutputFromString(desc)
        self.assertIsNotNone(out)
        self.assertEqual(out.type(), 'outputFile')
        self.assertEqual(out.name(), 'out_file')
        self.assertEqual(out.description(), 'Output File')

    def testOutputFolderDesc(self):
        desc = 'QgsProcessingOutputFolder|out_folder|Output Folder'
        out = getOutputFromString(desc)
        self.assertIsNotNone(out)
        self.assertEqual(out.type(), 'outputFolder')
        self.assertEqual(out.name(), 'out_folder')
        self.assertEqual(out.description(), 'Output Folder')

    def testOutputHtmlDesc(self):
        desc = 'QgsProcessingOutputHtml|out_html|Output Html'
        out = getOutputFromString(desc)
        self.assertIsNotNone(out)
        self.assertEqual(out.type(), 'outputHtml')
        self.assertEqual(out.name(), 'out_html')
        self.assertEqual(out.description(), 'Output Html')


if __name__ == '__main__':
    unittest.main()
