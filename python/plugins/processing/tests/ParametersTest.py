"""
***************************************************************************
    ParametersTest
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

__author__ = "René-Luc DHONT"
__date__ = "May 2021"
__copyright__ = "(C) 2021, René-Luc DHONT"

import os
import shutil

from qgis.core import (
    QgsProcessingParameterDefinition,
    QgsProcessingParameterNumber,
    QgsProcessingParameterFile,
    QgsProcessing,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from processing.core.parameters import getParameterFromString

testDataPath = os.path.join(os.path.dirname(__file__), "testdata")

start_app()


class ParametersTest(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        cls.cleanup_paths = []

    @classmethod
    def tearDownClass(cls):
        for path in cls.cleanup_paths:
            shutil.rmtree(path)

    def testParameterStringDesc(self):
        desc = "QgsProcessingParameterString|in_string|Input String"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "string")
        self.assertEqual(param.name(), "in_string")
        self.assertEqual(param.description(), "Input String")
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

        desc = "QgsProcessingParameterString|in_string|Input String|default value"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "string")
        self.assertEqual(param.name(), "in_string")
        self.assertEqual(param.description(), "Input String")
        self.assertEqual(param.defaultValue(), "default value")
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

        desc = "QgsProcessingParameterString|in_string|Input String|default value|True"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "string")
        self.assertEqual(param.name(), "in_string")
        self.assertEqual(param.description(), "Input String")
        self.assertEqual(param.defaultValue(), "default value")
        self.assertTrue(param.multiLine())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

        desc = "QgsProcessingParameterString|in_string|Input String||False|True"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "string")
        self.assertEqual(param.name(), "in_string")
        self.assertEqual(param.description(), "Input String")
        self.assertEqual(param.defaultValue(), "")
        self.assertFalse(param.multiLine())
        self.assertTrue(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

    def testParameterNumberDesc(self):
        desc = "QgsProcessingParameterNumber|in_number|Input Number"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "number")
        self.assertEqual(param.name(), "in_number")
        self.assertEqual(param.description(), "Input Number")
        self.assertEqual(param.dataType(), QgsProcessingParameterNumber.Type.Integer)
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

        desc = "QgsProcessingParameterNumber|in_number|Input Number|QgsProcessingParameterNumber.Double"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "number")
        self.assertEqual(param.name(), "in_number")
        self.assertEqual(param.description(), "Input Number")
        self.assertEqual(param.dataType(), QgsProcessingParameterNumber.Type.Double)
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

        desc = "QgsProcessingParameterNumber|in_number|Input Number|QgsProcessingParameterNumber.Integer|10"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "number")
        self.assertEqual(param.name(), "in_number")
        self.assertEqual(param.description(), "Input Number")
        self.assertEqual(param.dataType(), QgsProcessingParameterNumber.Type.Integer)
        self.assertEqual(param.defaultValue(), 10)
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

        desc = "QgsProcessingParameterNumber|in_number|Input Number|QgsProcessingParameterNumber.Integer|None|True"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "number")
        self.assertEqual(param.name(), "in_number")
        self.assertEqual(param.description(), "Input Number")
        self.assertEqual(param.dataType(), QgsProcessingParameterNumber.Type.Integer)
        self.assertIsNone(param.defaultValue())
        self.assertTrue(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

        desc = "QgsProcessingParameterNumber|in_number|Input Number|QgsProcessingParameterNumber.Integer|10|False|0"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "number")
        self.assertEqual(param.name(), "in_number")
        self.assertEqual(param.description(), "Input Number")
        self.assertEqual(param.dataType(), QgsProcessingParameterNumber.Type.Integer)
        self.assertEqual(param.defaultValue(), 10)
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )
        self.assertEqual(param.minimum(), 0)

        desc = "QgsProcessingParameterNumber|in_number|Input Number|QgsProcessingParameterNumber.Integer|10|False|0|20"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "number")
        self.assertEqual(param.name(), "in_number")
        self.assertEqual(param.description(), "Input Number")
        self.assertEqual(param.dataType(), QgsProcessingParameterNumber.Type.Integer)
        self.assertEqual(param.defaultValue(), 10)
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )
        self.assertEqual(param.minimum(), 0)
        self.assertEqual(param.maximum(), 20)

    def testParameterBooleanDesc(self):
        desc = "QgsProcessingParameterBoolean|in_bool|Input Boolean"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "boolean")
        self.assertEqual(param.name(), "in_bool")
        self.assertEqual(param.description(), "Input Boolean")
        self.assertFalse(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

        desc = "QgsProcessingParameterBoolean|in_bool|Input Boolean|True"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "boolean")
        self.assertEqual(param.name(), "in_bool")
        self.assertEqual(param.description(), "Input Boolean")
        self.assertTrue(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

        desc = "QgsProcessingParameterBoolean|in_bool|Input Boolean|False|True"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "boolean")
        self.assertEqual(param.name(), "in_bool")
        self.assertEqual(param.description(), "Input Boolean")
        self.assertFalse(param.defaultValue())
        self.assertTrue(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

    def testParameterCrsDesc(self):
        desc = "QgsProcessingParameterCrs|in_crs|Input CRS"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "crs")
        self.assertEqual(param.name(), "in_crs")
        self.assertEqual(param.description(), "Input CRS")
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

        desc = "QgsProcessingParameterCrs|in_crs|Input CRS|EPSG:2154"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "crs")
        self.assertEqual(param.name(), "in_crs")
        self.assertEqual(param.description(), "Input CRS")
        self.assertEqual(param.defaultValue(), "EPSG:2154")
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

        desc = "QgsProcessingParameterCrs|in_crs|Input CRS|None|True"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "crs")
        self.assertEqual(param.name(), "in_crs")
        self.assertEqual(param.description(), "Input CRS")
        self.assertIsNone(param.defaultValue())
        self.assertTrue(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

    def testParameterExtentDesc(self):
        desc = "QgsProcessingParameterExtent|in_extent|Input Extent"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "extent")
        self.assertEqual(param.name(), "in_extent")
        self.assertEqual(param.description(), "Input Extent")
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

        desc = "QgsProcessingParameterExtent|in_extent|Input Extent|None|True"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "extent")
        self.assertEqual(param.name(), "in_extent")
        self.assertEqual(param.description(), "Input Extent")
        self.assertIsNone(param.defaultValue())
        self.assertTrue(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

    def testParameterFileDesc(self):
        desc = "QgsProcessingParameterFile|in_file|Input File"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "file")
        self.assertEqual(param.name(), "in_file")
        self.assertEqual(param.description(), "Input File")
        self.assertEqual(param.behavior(), QgsProcessingParameterFile.Behavior.File)
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

        desc = "QgsProcessingParameterFile|in_folder|Input Folder|1"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "file")
        self.assertEqual(param.name(), "in_folder")
        self.assertEqual(param.description(), "Input Folder")
        self.assertEqual(param.behavior(), QgsProcessingParameterFile.Behavior.Folder)
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

        desc = "QgsProcessingParameterFile|in_folder|Input Folder|QgsProcessingParameterFile.Folder"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "file")
        self.assertEqual(param.name(), "in_folder")
        self.assertEqual(param.description(), "Input Folder")
        self.assertEqual(param.behavior(), QgsProcessingParameterFile.Behavior.Folder)
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

        desc = "QgsProcessingParameterFile|in_file|Input File|0|gpkg"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "file")
        self.assertEqual(param.name(), "in_file")
        self.assertEqual(param.description(), "Input File")
        self.assertEqual(param.behavior(), QgsProcessingParameterFile.Behavior.File)
        self.assertEqual(param.extension(), "gpkg")
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

        desc = "QgsProcessingParameterFile|in_file|Input File|0|png|None|False|PNG Files (*.png);; JPG Files (*.jpg *.jpeg)"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "file")
        self.assertEqual(param.name(), "in_file")
        self.assertEqual(param.description(), "Input File")
        self.assertEqual(param.behavior(), QgsProcessingParameterFile.Behavior.File)
        self.assertEqual(param.extension(), "")
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )
        self.assertEqual(
            param.fileFilter(), "PNG Files (*.png);; JPG Files (*.jpg *.jpeg)"
        )

    def testParameterVectorDestDesc(self):
        desc = "QgsProcessingParameterVectorDestination|param_vector_dest|Vector Destination"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "vectorDestination")
        self.assertEqual(param.name(), "param_vector_dest")
        self.assertEqual(param.description(), "Vector Destination")
        self.assertEqual(
            param.dataType(), QgsProcessing.SourceType.TypeVectorAnyGeometry
        )
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )
        self.assertTrue(param.createByDefault())

        desc = "QgsProcessingParameterVectorDestination|param_vector_dest|Vector Destination Point|0"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "vectorDestination")
        self.assertEqual(param.name(), "param_vector_dest")
        self.assertEqual(param.description(), "Vector Destination Point")
        self.assertEqual(param.dataType(), QgsProcessing.SourceType.TypeVectorPoint)
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )
        self.assertTrue(param.createByDefault())

        desc = "QgsProcessingParameterVectorDestination|param_vector_dest|Vector Destination Point|QgsProcessing.TypeVectorPoint"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "vectorDestination")
        self.assertEqual(param.name(), "param_vector_dest")
        self.assertEqual(param.description(), "Vector Destination Point")
        self.assertEqual(param.dataType(), QgsProcessing.SourceType.TypeVectorPoint)
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )
        self.assertTrue(param.createByDefault())

        desc = "QgsProcessingParameterVectorDestination|param_vector_dest|Vector Destination Line|1"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "vectorDestination")
        self.assertEqual(param.name(), "param_vector_dest")
        self.assertEqual(param.description(), "Vector Destination Line")
        self.assertEqual(param.dataType(), QgsProcessing.SourceType.TypeVectorLine)
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )
        self.assertTrue(param.createByDefault())

        desc = "QgsProcessingParameterVectorDestination|param_vector_dest|Vector Destination Line|QgsProcessing.TypeVectorLine"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "vectorDestination")
        self.assertEqual(param.name(), "param_vector_dest")
        self.assertEqual(param.description(), "Vector Destination Line")
        self.assertEqual(param.dataType(), QgsProcessing.SourceType.TypeVectorLine)
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )
        self.assertTrue(param.createByDefault())

        desc = "QgsProcessingParameterVectorDestination|param_vector_dest|Vector Destination Polygon|2"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "vectorDestination")
        self.assertEqual(param.name(), "param_vector_dest")
        self.assertEqual(param.description(), "Vector Destination Polygon")
        self.assertEqual(param.dataType(), QgsProcessing.SourceType.TypeVectorPolygon)
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )
        self.assertTrue(param.createByDefault())

        desc = "QgsProcessingParameterVectorDestination|param_vector_dest|Vector Destination Polygon|QgsProcessing.TypeVectorPolygon"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "vectorDestination")
        self.assertEqual(param.name(), "param_vector_dest")
        self.assertEqual(param.description(), "Vector Destination Polygon")
        self.assertEqual(param.dataType(), QgsProcessing.SourceType.TypeVectorPolygon)
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )
        self.assertTrue(param.createByDefault())

        desc = "QgsProcessingParameterVectorDestination|param_vector_dest|Vector Destination Table|5"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "vectorDestination")
        self.assertEqual(param.name(), "param_vector_dest")
        self.assertEqual(param.description(), "Vector Destination Table")
        self.assertEqual(param.dataType(), QgsProcessing.SourceType.TypeVector)
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )
        self.assertTrue(param.createByDefault())

        desc = "QgsProcessingParameterVectorDestination|param_vector_dest|Vector Destination Table|QgsProcessing.TypeVector"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "vectorDestination")
        self.assertEqual(param.name(), "param_vector_dest")
        self.assertEqual(param.description(), "Vector Destination Table")
        self.assertEqual(param.dataType(), QgsProcessing.SourceType.TypeVector)
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )
        self.assertTrue(param.createByDefault())

        desc = "QgsProcessingParameterVectorDestination|param_vector_dest|Vector Destination|-1|None|True|False"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "vectorDestination")
        self.assertEqual(param.name(), "param_vector_dest")
        self.assertEqual(param.description(), "Vector Destination")
        self.assertEqual(
            param.dataType(), QgsProcessing.SourceType.TypeVectorAnyGeometry
        )
        self.assertIsNone(param.defaultValue())
        self.assertTrue(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )
        self.assertFalse(param.createByDefault())

    def testParameterRasterDestDesc(self):
        desc = "QgsProcessingParameterRasterDestination|param_raster_dest|Raster Destination"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "rasterDestination")
        self.assertEqual(param.name(), "param_raster_dest")
        self.assertEqual(param.description(), "Raster Destination")
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )
        self.assertTrue(param.createByDefault())

        desc = "QgsProcessingParameterRasterDestination|param_raster_dest|Raster Destination|None|True|False"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "rasterDestination")
        self.assertEqual(param.name(), "param_raster_dest")
        self.assertEqual(param.description(), "Raster Destination")
        self.assertIsNone(param.defaultValue())
        self.assertTrue(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )
        self.assertFalse(param.createByDefault())

    def testParameterFolderDestDesc(self):
        desc = "QgsProcessingParameterFolderDestination|param_folder_dest|Folder Destination"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "folderDestination")
        self.assertEqual(param.name(), "param_folder_dest")
        self.assertEqual(param.description(), "Folder Destination")
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )
        self.assertTrue(param.createByDefault())

        desc = "QgsProcessingParameterFolderDestination|param_folder_dest|Folder Destination|None|True|False"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "folderDestination")
        self.assertEqual(param.name(), "param_folder_dest")
        self.assertEqual(param.description(), "Folder Destination")
        self.assertIsNone(param.defaultValue())
        self.assertTrue(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )
        self.assertFalse(param.createByDefault())

    def testParameterFileDestDesc(self):
        desc = "QgsProcessingParameterFileDestination|param_file_dest|File Destination"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "fileDestination")
        self.assertEqual(param.name(), "param_file_dest")
        self.assertEqual(param.description(), "File Destination")
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )
        self.assertTrue(param.createByDefault())

        desc = "QgsProcessingParameterFileDestination|param_html_dest|HTML File Destination|HTML Files (*.html)"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "fileDestination")
        self.assertEqual(param.name(), "param_html_dest")
        self.assertEqual(param.description(), "HTML File Destination")
        self.assertEqual(param.fileFilter(), "HTML Files (*.html)")
        self.assertEqual(param.defaultFileExtension(), "html")
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )
        self.assertTrue(param.createByDefault())

        desc = "QgsProcessingParameterFileDestination|param_img_dest|Img File Destination|PNG Files (*.png);; JPG Files (*.jpg *.jpeg)"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "fileDestination")
        self.assertEqual(param.name(), "param_img_dest")
        self.assertEqual(param.description(), "Img File Destination")
        self.assertEqual(
            param.fileFilter(), "PNG Files (*.png);; JPG Files (*.jpg *.jpeg)"
        )
        self.assertEqual(param.defaultFileExtension(), "png")
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )
        self.assertTrue(param.createByDefault())

        desc = "QgsProcessingParameterFileDestination|param_csv_dest|CSV File Destination|CSV Files (*.csv)|None|True|False"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "fileDestination")
        self.assertEqual(param.name(), "param_csv_dest")
        self.assertEqual(param.description(), "CSV File Destination")
        self.assertEqual(param.fileFilter(), "CSV Files (*.csv)")
        self.assertEqual(param.defaultFileExtension(), "csv")
        self.assertIsNone(param.defaultValue())
        self.assertTrue(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )
        self.assertFalse(param.createByDefault())

    def testParameterFeatureSourceDesc(self):
        desc = "QgsProcessingParameterFeatureSource|in_vector|Input Vector"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "source")
        self.assertEqual(param.name(), "in_vector")
        self.assertEqual(param.description(), "Input Vector")
        self.assertListEqual(param.dataTypes(), [])
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

        desc = "QgsProcessingParameterFeatureSource|in_vector|Input Vector|0"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "source")
        self.assertEqual(param.name(), "in_vector")
        self.assertEqual(param.description(), "Input Vector")
        self.assertListEqual(
            param.dataTypes(), [QgsProcessing.SourceType.TypeVectorPoint]
        )
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

        desc = "QgsProcessingParameterFeatureSource|in_vector|Input Vector|QgsProcessing.TypeVectorPoint"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "source")
        self.assertEqual(param.name(), "in_vector")
        self.assertEqual(param.description(), "Input Vector")
        self.assertListEqual(
            param.dataTypes(), [QgsProcessing.SourceType.TypeVectorPoint]
        )
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

        desc = "QgsProcessingParameterFeatureSource|in_vector|Input Vector|1"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "source")
        self.assertEqual(param.name(), "in_vector")
        self.assertEqual(param.description(), "Input Vector")
        self.assertListEqual(
            param.dataTypes(), [QgsProcessing.SourceType.TypeVectorLine]
        )
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

        desc = "QgsProcessingParameterFeatureSource|in_vector|Input Vector|QgsProcessing.TypeVectorLine"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "source")
        self.assertEqual(param.name(), "in_vector")
        self.assertEqual(param.description(), "Input Vector")
        self.assertListEqual(
            param.dataTypes(), [QgsProcessing.SourceType.TypeVectorLine]
        )
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

        desc = "QgsProcessingParameterFeatureSource|in_vector|Input Vector|2"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "source")
        self.assertEqual(param.name(), "in_vector")
        self.assertEqual(param.description(), "Input Vector")
        self.assertListEqual(
            param.dataTypes(), [QgsProcessing.SourceType.TypeVectorPolygon]
        )
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

        desc = "QgsProcessingParameterFeatureSource|in_vector|Input Vector|QgsProcessing.TypeVectorPolygon"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "source")
        self.assertEqual(param.name(), "in_vector")
        self.assertEqual(param.description(), "Input Vector")
        self.assertListEqual(
            param.dataTypes(), [QgsProcessing.SourceType.TypeVectorPolygon]
        )
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

        desc = "QgsProcessingParameterFeatureSource|in_vector|Input Vector|5"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "source")
        self.assertEqual(param.name(), "in_vector")
        self.assertEqual(param.description(), "Input Vector")
        self.assertListEqual(param.dataTypes(), [QgsProcessing.SourceType.TypeVector])
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

        desc = "QgsProcessingParameterFeatureSource|in_vector|Input Vector|QgsProcessing.TypeVector"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "source")
        self.assertEqual(param.name(), "in_vector")
        self.assertEqual(param.description(), "Input Vector")
        self.assertListEqual(param.dataTypes(), [QgsProcessing.SourceType.TypeVector])
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

        desc = "QgsProcessingParameterFeatureSource|in_vector|Input Vector|1;2"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "source")
        self.assertEqual(param.name(), "in_vector")
        self.assertEqual(param.description(), "Input Vector")
        self.assertListEqual(
            param.dataTypes(),
            [
                QgsProcessing.SourceType.TypeVectorLine,
                QgsProcessing.SourceType.TypeVectorPolygon,
            ],
        )
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

        desc = "QgsProcessingParameterFeatureSource|in_vector|Input Vector|QgsProcessing.TypeVectorLine;QgsProcessing.TypeVectorPolygon"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "source")
        self.assertEqual(param.name(), "in_vector")
        self.assertEqual(param.description(), "Input Vector")
        self.assertListEqual(
            param.dataTypes(),
            [
                QgsProcessing.SourceType.TypeVectorLine,
                QgsProcessing.SourceType.TypeVectorPolygon,
            ],
        )
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

        desc = "QgsProcessingParameterFeatureSource|in_vector|Input Vector|-1|None|True"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "source")
        self.assertEqual(param.name(), "in_vector")
        self.assertEqual(param.description(), "Input Vector")
        self.assertListEqual(
            param.dataTypes(), [QgsProcessing.SourceType.TypeVectorAnyGeometry]
        )
        self.assertIsNone(param.defaultValue())
        self.assertTrue(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

    def testParameterVectorLayerDesc(self):
        desc = "QgsProcessingParameterVectorLayer|in_vector|Input Vector"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "vector")
        self.assertEqual(param.name(), "in_vector")
        self.assertEqual(param.description(), "Input Vector")
        self.assertListEqual(param.dataTypes(), [])
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

        desc = "QgsProcessingParameterVectorLayer|in_vector|Input Vector|0"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "vector")
        self.assertEqual(param.name(), "in_vector")
        self.assertEqual(param.description(), "Input Vector")
        self.assertListEqual(
            param.dataTypes(), [QgsProcessing.SourceType.TypeVectorPoint]
        )
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

        desc = "QgsProcessingParameterVectorLayer|in_vector|Input Vector|QgsProcessing.TypeVectorPoint"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "vector")
        self.assertEqual(param.name(), "in_vector")
        self.assertEqual(param.description(), "Input Vector")
        self.assertListEqual(
            param.dataTypes(), [QgsProcessing.SourceType.TypeVectorPoint]
        )
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

        desc = "QgsProcessingParameterVectorLayer|in_vector|Input Vector|1"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "vector")
        self.assertEqual(param.name(), "in_vector")
        self.assertEqual(param.description(), "Input Vector")
        self.assertListEqual(
            param.dataTypes(), [QgsProcessing.SourceType.TypeVectorLine]
        )
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

        desc = "QgsProcessingParameterVectorLayer|in_vector|Input Vector|QgsProcessing.TypeVectorLine"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "vector")
        self.assertEqual(param.name(), "in_vector")
        self.assertEqual(param.description(), "Input Vector")
        self.assertListEqual(
            param.dataTypes(), [QgsProcessing.SourceType.TypeVectorLine]
        )
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

        desc = "QgsProcessingParameterVectorLayer|in_vector|Input Vector|2"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "vector")
        self.assertEqual(param.name(), "in_vector")
        self.assertEqual(param.description(), "Input Vector")
        self.assertListEqual(
            param.dataTypes(), [QgsProcessing.SourceType.TypeVectorPolygon]
        )
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

        desc = "QgsProcessingParameterVectorLayer|in_vector|Input Vector|QgsProcessing.TypeVectorPolygon"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "vector")
        self.assertEqual(param.name(), "in_vector")
        self.assertEqual(param.description(), "Input Vector")
        self.assertListEqual(
            param.dataTypes(), [QgsProcessing.SourceType.TypeVectorPolygon]
        )
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

        desc = "QgsProcessingParameterVectorLayer|in_vector|Input Vector|5"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "vector")
        self.assertEqual(param.name(), "in_vector")
        self.assertEqual(param.description(), "Input Vector")
        self.assertListEqual(param.dataTypes(), [QgsProcessing.SourceType.TypeVector])
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

        desc = "QgsProcessingParameterVectorLayer|in_vector|Input Vector|QgsProcessing.TypeVector"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "vector")
        self.assertEqual(param.name(), "in_vector")
        self.assertEqual(param.description(), "Input Vector")
        self.assertListEqual(param.dataTypes(), [QgsProcessing.SourceType.TypeVector])
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

        desc = "QgsProcessingParameterVectorLayer|in_vector|Input Vector|1;2"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "vector")
        self.assertEqual(param.name(), "in_vector")
        self.assertEqual(param.description(), "Input Vector")
        self.assertListEqual(
            param.dataTypes(),
            [
                QgsProcessing.SourceType.TypeVectorLine,
                QgsProcessing.SourceType.TypeVectorPolygon,
            ],
        )
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

        desc = "QgsProcessingParameterVectorLayer|in_vector|Input Vector|QgsProcessing.TypeVectorLine;QgsProcessing.TypeVectorPolygon"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "vector")
        self.assertEqual(param.name(), "in_vector")
        self.assertEqual(param.description(), "Input Vector")
        self.assertListEqual(
            param.dataTypes(),
            [
                QgsProcessing.SourceType.TypeVectorLine,
                QgsProcessing.SourceType.TypeVectorPolygon,
            ],
        )
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

        desc = "QgsProcessingParameterVectorLayer|in_vector|Input Vector|-1|None|True"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "vector")
        self.assertEqual(param.name(), "in_vector")
        self.assertEqual(param.description(), "Input Vector")
        self.assertListEqual(
            param.dataTypes(), [QgsProcessing.SourceType.TypeVectorAnyGeometry]
        )
        self.assertIsNone(param.defaultValue())
        self.assertTrue(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

    def testParameterRasterLayerDesc(self):
        desc = "QgsProcessingParameterRasterLayer|in_raster|Input Raster"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "raster")
        self.assertEqual(param.name(), "in_raster")
        self.assertEqual(param.description(), "Input Raster")
        self.assertIsNone(param.defaultValue())
        self.assertFalse(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )

        desc = "QgsProcessingParameterRasterLayer|in_raster|Input Raster|None|True"
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), "raster")
        self.assertEqual(param.name(), "in_raster")
        self.assertEqual(param.description(), "Input Raster")
        self.assertIsNone(param.defaultValue())
        self.assertTrue(
            param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
        )


if __name__ == "__main__":
    unittest.main()
