"""QGIS Unit tests for the ImageServer provider.

From build dir, run: ctest -R PyQgsImageServerProvider -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import shutil
import struct
import tempfile
import unittest

from osgeo import gdal
from qgis.core import (
    Qgis,
    QgsCoordinateReferenceSystem,
    QgsGdalUtils,
    QgsPointXY,
    QgsRasterLayer,
    QgsRectangle,
    QgsSettings,
)
from qgis.PyQt.QtCore import QCoreApplication, QMetaType
from qgis.testing import QgisTestCase, start_app
from raster_provider_test_base import RasterProviderTestCase


class TestPyQgsImageServerProvider(QgisTestCase, RasterProviderTestCase):
    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()

        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("TestPyQgsIMSProvider.com")
        QCoreApplication.setApplicationName("TestPyQgsIMSProvider")
        QgsSettings().clear()
        start_app()

        cls.basetestpath = tempfile.mkdtemp().replace("\\", "/")

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        QgsSettings().clear()
        if cls.basetestpath:
            shutil.rmtree(cls.basetestpath, True)
        cls.vl = (
            None
            # so as to properly close the provider and remove any temporary file
        )
        super().tearDownClass()

    def get_layer(self, test_id: str) -> QgsRasterLayer:
        endpoint = self.basetestpath + f"/{test_id}_fake_qgis_http_endpoint"
        with open(self.sanitize_local_url(endpoint, "?f=json"), "wb") as f:
            f.write(
                b"""
                {
 "currentVersion": 10.91,
 "name": "CharlotteLAS",
 "extent": {
  "xmin": 1440000,
  "ymin": 535000,
  "xmax": 1455000,
  "ymax": 550000,
  "spatialReference": {
   "wkid": 102719,
   "latestWkid": 2264
  }
 },
 "initialExtent": {
  "xmin": 1440000,
  "ymin": 535000,
  "xmax": 1440000.001927593273148176,
  "ymax": 550000,
  "spatialReference": {
   "wkid": 102719,
   "latestWkid": 2264
  }
 },
 "fullExtent": {
  "xmin": 1440000,
  "ymin": 535000,
  "xmax": 1440000.001927593273148176,
  "ymax": 535000.0019346599399128195,
  "spatialReference": {
   "wkid": 102719,
   "latestWkid": 2264
  }
 },
 "hasMultidimensions": false,
 "pixelSizeX": 0.000642531091049392,
 "pixelSizeY": 0.00048366498497820487,
 "datasetFormat": "AMD",
 "uncompressedSize": 9000000,
 "blockWidth": 2048,
 "blockHeight": 256,
 "compressionType": "None",
 "bandNames": [
  "Band_1"
 ],
 "allowCopy": true,
 "allowAnalysis": true,
 "bandCount": 1,
 "pixelType": "F32",
 "minPixelSize": 0,
 "maxPixelSize": 0,
 "serviceDataType": "esriImageServiceDataTypeElevation",
 "serviceSourceType": "esriImageServiceSourceTypeMosaicDataset",
 "minValues": [
  515.6699829101562
 ],
 "maxValues": [
  1611.125
 ],
 "meanValues": [
  712.371337411377
 ],
 "stdvValues": [
  45.56883666949917
 ],
 "objectIdField": "OBJECTID",
 "fields": [
  {
   "name": "OBJECTID",
   "type": "esriFieldTypeOID",
   "alias": "OBJECTID",
   "domain": null
  }
 ],
 "capabilities": "Image,Metadata,Catalog,Mensuration",
 "defaultMosaicMethod": "Northwest",
 "allowedMosaicMethods": "NorthWest,Center,LockRaster,ByAttribute,Nadir,Viewpoint,Seamline,None",
 "sortField": "",
 "sortValue": null,
 "sortAscending": true,
 "mosaicOperator": "First",
 "maxDownloadSizeLimit": 0,
 "defaultCompressionQuality": 10000,
 "defaultResamplingMethod": "Bilinear",
 "maxImageHeight": 4100,
 "maxImageWidth": 15000,
 "maxRecordCount": 1000,
 "maxDownloadImageCount": 0,
 "maxMosaicImageCount": 20,
 "allowRasterFunction": true,
 "rasterFunctionInfos": [
 ],
 "rasterTypeInfos": [
  {
   "name": "Raster Dataset",
   "description": "Supports all ArcGIS Raster Datasets",
   "help": ""
  }
 ],
 "mensurationCapabilities": "Basic",
 "hasHistograms": true,
 "hasColormap": false,
 "hasRasterAttributeTable": false,
 "minScale": 0,
 "maxScale": 0,
 "exportTilesAllowed": false,
 "supportsStatistics": true,
 "supportsAdvancedQueries": true,
 "editFieldsInfo": null,
 "ownershipBasedAccessControlForRasters": null,
 "allowComputeTiePoints": false,
 "useStandardizedQueries": true,
 "advancedQueryCapabilities": {
  "useStandardizedQueries": true,
  "supportsStatistics": true,
  "supportsOrderBy": true,
  "supportsDistinct": true,
  "supportsPagination": true
 },
 "spatialReference": {
  "wkid": 102719,
  "latestWkid": 2264
 }
}"""
            )
        return QgsRasterLayer(
            "url='http://" + endpoint + "'",
            "test",
            "arcgisimageserver",
        )

    def test_invalid(self):
        """
        Test connecting to a non-imageserver endpoint
        """
        endpoint = self.basetestpath + "/notims_test_fake_qgis_http_endpoint"
        with open(self.sanitize_local_url(endpoint, "?f=json"), "wb") as f:
            f.write(
                b"""
                {
 "currentVersion": 11.1,
 "cimVersion": "3.1.0",
 "serviceDescription": "",
 "mapName": "Layers",
 "description": "",
 "copyrightText": "",
 "supportsDynamicLayers": true,
 "layers": [
  {
   "id": 0,
   "name": "Layer 0",
   "parentLayerId": -1,
   "defaultVisibility": true,
   "subLayerIds": [
    1
   ],
   "minScale": 0,
   "maxScale": 0,
   "type": "Group Layer",
   "supportsDynamicLegends": false
  },
  {
   "id": 1,
   "name": "Layer 1",
   "parentLayerId": 0,
   "defaultVisibility": true,
   "subLayerIds": null,
   "minScale": 0,
   "maxScale": 0,
   "type": "Feature Layer",
   "geometryType": "esriGeometryPolyline",
   "supportsDynamicLegends": true
  }
 ],
 "tables": [],
 "spatialReference": {
  "wkid": 102067,
  "latestWkid": 5514,
  "xyTolerance": 0.001,
  "zTolerance": 0.001,
  "mTolerance": 0.001,
  "falseX": -33699800,
  "falseY": -33699800,
  "xyUnits": 1.3363876424698351E8,
  "falseZ": -100000,
  "zUnits": 10000,
  "falseM": -100000,
  "mUnits": 10000
 },
 "singleFusedMapCache": false,
 "initialExtent": {
  "xmin": -598610.4,
  "ymin": -1161402.1,
  "xmax": -597723.3,
  "ymax": -1160580.8,
  "spatialReference": {
   "wkid": 102067,
   "latestWkid": 5514,
   "xyTolerance": 0.001,
   "zTolerance": 0.001,
   "mTolerance": 0.001,
   "falseX": -33699800,
   "falseY": -33699800,
   "xyUnits": 1.3363876424698351E8,
   "falseZ": -100000,
   "zUnits": 10000,
   "falseM": -100000,
   "mUnits": 10000
  }
 },
 "fullExtent": {
  "xmin": -3.369963064526357E7,
  "ymin": -3.369967400237042E7,
  "xmax": 3.369963064526357E7,
  "ymax": 3.36326738810827E7,
  "spatialReference": {
   "wkid": 102067,
   "latestWkid": 5514,
   "xyTolerance": 0.001,
   "zTolerance": 0.001,
   "mTolerance": 0.001,
   "falseX": -33699800,
   "falseY": -33699800,
   "xyUnits": 1.3363876424698351E8,
   "falseZ": -100000,
   "zUnits": 10000,
   "falseM": -100000,
   "mUnits": 10000
  }
 },
 "datesInUnknownTimezone": false,
 "minScale": 0,
 "maxScale": 0,
 "units": "esriMeters",
 "supportedImageFormatTypes": "PNG32,PNG24,PNG,JPG,DIB,TIFF,EMF,PS,PDF,GIF,SVG,SVGZ,BMP",
 "documentInfo": {
  "Title": "Untitled.aprx",
  "Author": "",
  "Comments": "",
  "Subject": "",
  "Category": "",
  "Version": "3.1.0",
  "AntialiasingMode": "Normal",
  "TextAntialiasingMode": "Force",
  "Keywords": ""
 },
 "capabilities": "Map,Query,Data",
 "supportedQueryFormats": "JSON, geoJSON, PBF",
 "exportTilesAllowed": false,
 "referenceScale": 0.0,
 "datumTransformations": [],
 "supportsDatumTransformation": true,
 "archivingInfo": {"supportsHistoricMoment": false},
 "supportsClipping": true,
 "supportsSpatialFilter": true,
 "supportsTimeRelation": true,
 "supportsQueryDataElements": true,
 "mapUnits": {"uwkid": 9001},
 "maxRecordCount": 1000,
 "maxImageHeight": 4096,
 "maxImageWidth": 4096,
 "supportedExtensions": "WMSServer"
}"""
            )

        rl = QgsRasterLayer(
            "url='http://" + endpoint + "'",
            "test",
            "arcgisimageserver",
        )
        self.assertFalse(rl.isValid())
        self.assertEqual(
            rl.dataProvider().error().summary(),
            "Service does not have Image capability -- it is likely not an ESRI ImageService",
        )

    def test_basic(self):
        """
        Test basic provider capabilities
        """
        endpoint = self.basetestpath + "/basic_test_fake_qgis_http_endpoint"
        with open(self.sanitize_local_url(endpoint, "?f=json"), "wb") as f:
            f.write(
                b"""
                {
 "currentVersion": 10.91,
 "name": "CharlotteLAS",
 "description": "My service description",
 "copyrightText": "My copyright string",
 "extent": {
  "xmin": 1440000,
  "ymin": 535000,
  "xmax": 1455000,
  "ymax": 550000,
  "spatialReference": {
   "wkid": 102719,
   "latestWkid": 2264
  }
 },
 "initialExtent": {
  "xmin": 1440000,
  "ymin": 535000,
  "xmax": 1455000,
  "ymax": 550000,
  "spatialReference": {
   "wkid": 102719,
   "latestWkid": 2264
  }
 },
 "fullExtent": {
  "xmin": 1440000,
  "ymin": 535000,
  "xmax": 1455000,
  "ymax": 550000,
  "spatialReference": {
   "wkid": 102719,
   "latestWkid": 2264
  }
 },
 "hasMultidimensions": false,
 "pixelSizeX": 10,
 "pixelSizeY": 10,
 "datasetFormat": "AMD",
 "uncompressedSize": 9000000,
 "blockWidth": 2048,
 "blockHeight": 256,
 "compressionType": "None",
 "bandNames": [
  "Band_1"
 ],
 "allowCopy": true,
 "allowAnalysis": true,
 "bandCount": 1,
 "pixelType": "F32",
 "minPixelSize": 0,
 "maxPixelSize": 0,
 "serviceDataType": "esriImageServiceDataTypeElevation",
 "serviceSourceType": "esriImageServiceSourceTypeMosaicDataset",
 "minValues": [
  515.6699829101562
 ],
 "maxValues": [
  1611.125
 ],
 "meanValues": [
  712.371337411377
 ],
 "stdvValues": [
  45.56883666949917
 ],
 "objectIdField": "OBJECTID",
 "fields": [
  {
   "name": "OBJECTID",
   "type": "esriFieldTypeOID",
   "alias": "OBJECTID",
   "domain": null
  }
 ],
 "capabilities": "Image,Metadata,Catalog,Mensuration",
 "defaultMosaicMethod": "Northwest",
 "allowedMosaicMethods": "NorthWest,Center,LockRaster,ByAttribute,Nadir,Viewpoint,Seamline,None",
 "sortField": "",
 "sortValue": null,
 "sortAscending": true,
 "mosaicOperator": "First",
 "maxDownloadSizeLimit": 0,
 "defaultCompressionQuality": 10000,
 "defaultResamplingMethod": "Bilinear",
 "maxImageHeight": 4100,
 "maxImageWidth": 15000,
 "maxRecordCount": 1000,
 "maxDownloadImageCount": 0,
 "maxMosaicImageCount": 20,
 "allowRasterFunction": true,
 "rasterFunctionInfos": [
 ],
 "rasterTypeInfos": [
  {
   "name": "Raster Dataset",
   "description": "Supports all ArcGIS Raster Datasets",
   "help": ""
  }
 ],
 "mensurationCapabilities": "Basic",
 "hasHistograms": true,
 "hasColormap": false,
 "hasRasterAttributeTable": false,
 "minScale": 0,
 "maxScale": 0,
 "exportTilesAllowed": false,
 "supportsStatistics": true,
 "supportsAdvancedQueries": true,
 "editFieldsInfo": null,
 "ownershipBasedAccessControlForRasters": null,
 "allowComputeTiePoints": false,
 "useStandardizedQueries": true,
 "advancedQueryCapabilities": {
  "useStandardizedQueries": true,
  "supportsStatistics": true,
  "supportsOrderBy": true,
  "supportsDistinct": true,
  "supportsPagination": true
 },
 "spatialReference": {
  "wkid": 102719,
  "latestWkid": 2264
 }
}"""
            )

        with open(
            self.sanitize_local_url(
                endpoint,
                "/identify?f=json&geometryType=esriGeometryPoint&geometry=1455000.000000,542500.000000",
            ),
            "wb",
        ) as f:
            f.write(
                b"""
                {
  "objectId": 0,
  "name": "Pixel",
  "value": "1088.04",
  "location": {
    "x": 1455000.000000,
    "y": 542500.000000,
    "spatialReference": {
      "wkid": 102719,
      "latestWkid": 2264
    }
  },
  "properties": {
    "Values": [
      "1088.03"
    ]
  },
  "catalogItems": {
    "objectIdFieldName": "OBJECTID",
    "geometryType": "esriGeometryPolygon",
    "spatialReference": {
      "wkid": 102719,
      "latestWkid": 2264
    },
    "features": [
      {
        "attributes": {
          "OBJECTID": 6,
          "Name": "4544-04",
          "MinPS": 0,
          "MaxPS": 100,
          "LowPS": 10,
          "HighPS": 10,
          "Category": 1,
          "Tag": "Dataset",
          "GroupName": "",
          "ProductName": "",
          "CenterX": 1447499.9999391,
          "CenterY": 542500.000002436,
          "ZOrder": null,
          "Version": "1.0",
          "PointCount": 1141410,
          "PointSpacing": 4.67996083909501,
          "ZMin": 50.83,
          "ZMax": 1640,
          "Shape_Length": 20000.0006823242,
          "Shape_Area": 25000001.7058105
        },
        "geometry": {
          "rings": [
            [
              [1444999.9998538, 539999.99991715],
              [1444999.9998538, 545000.000087723],
              [1450000.00002439, 545000.000087723],
              [1450000.00002439, 539999.99991715],
              [1444999.9998538, 539999.99991715]
            ]
          ],
          "spatialReference": {
            "wkid": 102719,
            "latestWkid": 2264
          }
        }
      }
    ]
  },
  "catalogItemVisibilities": [1]
}"""
            )
        rl = QgsRasterLayer(
            "url='http://" + endpoint + "'",
            "test",
            "arcgisimageserver",
        )
        self.assertTrue(rl.isValid())
        # basic raster layer properties
        self.assertEqual(rl.width(), 1500)
        self.assertEqual(rl.height(), 1500)
        self.assertEqual(rl.bandCount(), 1)
        self.assertEqual(rl.bandName(1), "Band_1")
        self.assertEqual(
            rl.dataProvider().colorInterpretation(1),
            Qgis.RasterColorInterpretation.Undefined,
        )
        self.assertIsNone(rl.attributeTable(1))
        self.assertFalse(rl.canCreateRasterAttributeTable())
        self.assertEqual(rl.providerType(), "arcgisimageserver")
        self.assertEqual(rl.rasterUnitsPerPixelX(), 10)
        self.assertEqual(rl.rasterUnitsPerPixelY(), 10)
        self.assertFalse(rl.subsetString())
        self.assertEqual(rl.crs(), QgsCoordinateReferenceSystem("EPSG:2264"))
        self.assertAlmostEqual(rl.extent().xMinimum(), 1440000.0, 2)
        self.assertAlmostEqual(rl.extent().yMinimum(), 535000.0, 2)
        self.assertAlmostEqual(rl.extent().xMaximum(), 1455000.0, 2)
        self.assertAlmostEqual(rl.extent().yMaximum(), 550000.0, 2)

        self.assertEqual(rl.dataProvider().dataType(1), Qgis.DataType.Float32)
        self.assertEqual(rl.dataProvider().sourceDataType(1), Qgis.DataType.Float32)
        self.assertEqual(rl.dataProvider().bandScale(1), 1)
        self.assertEqual(rl.dataProvider().bandOffset(1), 0)

        self.assertFalse(rl.dataProvider().sourceHasNoDataValue(1))
        self.assertFalse(rl.dataProvider().useSourceNoDataValue(1))

        self.assertEqual(rl.metadata().title(), "CharlotteLAS")
        self.assertEqual(rl.metadata().abstract(), "My service description")
        self.assertEqual(rl.metadata().rights(), ["My copyright string"])

        self.assertTrue(rl.elevationProperties().isEnabled())

        # test identify

        # not supported format
        self.assertFalse(
            rl.dataProvider()
            .identify(
                QgsPointXY(1455000, 542500.0),
                Qgis.RasterIdentifyFormat.Html,
                QgsRectangle(),
                0,
                0,
            )
            .isValid()
        )
        # pixel outside of extent
        res = rl.dataProvider().identify(
            QgsPointXY(1, 2), Qgis.RasterIdentifyFormat.Value, QgsRectangle(), 0, 0
        )
        self.assertTrue(res.isValid())
        self.assertEqual(res.results(), {1: None})

        # valid identify request
        res = rl.dataProvider().identify(
            QgsPointXY(1455000, 542500.0),
            Qgis.RasterIdentifyFormat.Value,
            QgsRectangle(),
            0,
            0,
        )
        self.assertTrue(res.isValid())
        self.assertAlmostEqual(res.results()[1], 1088.04, places=3)

    def test_multibands(self):
        """
        Test basic provider capabilities for multi-band layer
        """
        endpoint = self.basetestpath + "/basic_test_multiband_fake_qgis_http_endpoint"
        with open(self.sanitize_local_url(endpoint, "?f=json"), "wb") as f:
            f.write(
                b"""
                {
 "currentVersion": 10.91,
 "name": "Toronto",
 "extent": {
  "xmin": -8844874.0651,
  "ymin": 5401062.402699997,
  "xmax": -8828990.0651,
  "ymax": 5420947.402699997,
  "spatialReference": {
   "wkid": 102100,
   "latestWkid": 3857
  }
 },
 "initialExtent": {
  "xmin": -8844874.0651,
  "ymin": 5401062.402699997,
  "xmax": -8828990.0651,
  "ymax": 5420947.402699997,
  "spatialReference": {
   "wkid": 102100,
   "latestWkid": 3857
  }
 },
 "fullExtent": {
  "xmin": -8844874.0651,
  "ymin": 5401062.402699997,
  "xmax": -8828990.0651,
  "ymax": 5420947.402699997,
  "spatialReference": {
   "wkid": 102100,
   "latestWkid": 3857
  }
 },
 "hasMultidimensions": false,
 "pixelSizeX": 1,
 "pixelSizeY": 1,
 "datasetFormat": "AMD",
 "uncompressedSize": 2526826720,
 "blockWidth": 2048,
 "blockHeight": 256,
 "compressionType": "None",
 "bandNames": [
  "Blue",
  "Green",
  "Red",
  "NearInfrared"
 ],
 "allowCopy": true,
 "allowAnalysis": true,
 "bandCount": 4,
 "pixelType": "U16",
 "minPixelSize": 0.5971642835598172,
 "maxPixelSize": 305.74811314055756,
 "serviceDataType": "esriImageServiceDataTypeGeneric",
 "serviceSourceType": "esriImageServiceSourceTypeMosaicDataset",
 "minValues": [
  195,
  168,
  1,
  1
 ],
 "maxValues": [
  2047,
  2047,
  2047,
  2047
 ],
 "meanValues": [
  407.6350849285231,
  424.3829909000868,
  306.3585015423408,
  451.4521619746197
 ],
 "stdvValues": [
  103.3454476143749,
  161.37726805365864,
  177.16151769901987,
  330.19574780626806
 ],
 "objectIdField": "OBJECTID",
 "fields": [
 ],
 "capabilities": "Catalog,Mensuration,Image,Metadata",
 "defaultMosaicMethod": "Northwest",
 "allowedMosaicMethods": "NorthWest,Center,LockRaster,ByAttribute,Nadir,Viewpoint,Seamline,None",
 "sortField": "",
 "sortValue": null,
 "sortAscending": true,
 "mosaicOperator": "First",
 "maxDownloadSizeLimit": 2048,
 "defaultCompressionQuality": 75,
 "defaultResamplingMethod": "Bilinear",
 "maxImageHeight": 4100,
 "maxImageWidth": 15000,
 "maxRecordCount": 1000,
 "maxDownloadImageCount": 20,
 "maxMosaicImageCount": 20,
 "singleFusedMapCache": false,
 "tileInfo": {
  "rows": 256,
  "cols": 256,
  "dpi": 96,
  "format": "Mixed",
  "compressionQuality": 75,
  "origin": {
   "x": -2.0037508342787E7,
   "y": 2.0037508342787E7
  },
  "spatialReference": {
   "wkid": 102100,
   "latestWkid": 3857
  },
  "lods": [
  ]
 },
 "storageInfo": {
  "storageFormat": "esriMapCacheStorageModeCompact",
  "packetSize": 128
 },
 "cacheType": "Map",
 "allowRasterFunction": true,
 "rasterFunctionInfos": [
 ],
 "mensurationCapabilities": "Basic,Base-Top Height,Top-Top Shadow Height,Base-Top Shadow Height",
 "hasHistograms": true,
 "hasColormap": false,
 "hasRasterAttributeTable": false,
 "minScale": 1155581.108577,
 "maxScale": 2256.994353,
 "exportTilesAllowed": false,
 "supportsStatistics": true,
 "supportsAdvancedQueries": true,
 "editFieldsInfo": null,
 "ownershipBasedAccessControlForRasters": null,
 "allowComputeTiePoints": false,
 "useStandardizedQueries": true,
 "advancedQueryCapabilities": {
  "useStandardizedQueries": true,
  "supportsStatistics": true,
  "supportsOrderBy": true,
  "supportsDistinct": true,
  "supportsPagination": true
 },
 "spatialReference": {
  "wkid": 102100,
  "latestWkid": 3857
 }
}"""
            )

            with open(
                self.sanitize_local_url(
                    endpoint,
                    "/identify?f=json&geometryType=esriGeometryPoint&geometry=-8836932.000000,5411004.000000",
                ),
                "wb",
            ) as f:
                f.write(
                    b"""
                    {
  "objectId": 0,
  "name": "Pixel",
  "value": "879, 1114, 1032, 975",
  "location": {
    "x": -8836932.000000,
    "y": 5411004.000000,
    "spatialReference": {
      "wkid": 102100,
      "latestWkid": 3857
    }
  },
  "properties": {
    "Values": [
      "NoData NoData NoData NoData",
      "835 1070 988 931"
    ]
  },
  "catalogItems": {
    "objectIdFieldName": "OBJECTID",
    "geometryType": "esriGeometryPolygon",
    "spatialReference": {
      "wkid": 102100,
      "latestWkid": 3857
    },
    "features": [
      {
        "attributes": {
          "OBJECTID": 1,
          "Name": "po_578117_metadata.txt:0000000;po_578117_metadata.",
          "MinPS": 0,
          "MaxPS": 2,
          "LowPS": 1,
          "HighPS": 4,
          "Category": 1,
          "Tag": "Pansharpened",
          "GroupName": "578117_0000000",
          "ProductName": "Geo",
          "CenterX": -8836937.95994665,
          "CenterY": 5411005.39592532,
          "ZOrder": null,
          "StereoID": "2007070316182540000011619942",
          "BlockName": "578117",
          "SensorName": "IKONOS-2",
          "AcquisitionDate": 1183479480000,
          "SunAzimuth": 142.0678,
          "SunElevation": 65.59146,
          "SatAzimuth": 144.9171,
          "SatElevation": 79.46645,
          "CloudCover": 0,
          "SrcImgID": "2007070316182540000011619942",
          "OffNadir": 10.53355,
          "Shape_Length": 70917.3671667544,
          "Shape_Area": 310247601.691659
        },
        "geometry": {
          "rings": [
            [
              [-8837399.716, 5420783.1034],
              [-8836575.0219, 5420766.9695],
              [-8836574.8132, 5420777.691],
              [-8835835.4695, 5420763.1639],
              [-8835096.1293, 5420748.5774],
              [-8834356.7928, 5420733.9315],
              [-8834356.5815, 5420744.6528],
              [-8834244.9369, 5420742.436],
              [-8834245.1483, 5420731.7147],
              [-8834006.9738, 5420726.981],
              [-8834006.762, 5420737.7024],
              [-8833170.6721, 5420721.0364],
              [-8832334.5868, 5420704.2944],
              [-8831498.5062, 5420687.4766],
              [-8830662.4303, 5420670.5827],
              [-8829826.3591, 5420653.613],
              [-8828990.2927, 5420636.5673],
              [-8828992.1179, 5419745.9511],
              [-8828993.9408, 5418855.42],
              [-8828995.7615, 5417964.9739],
              [-8828997.5799, 5417074.6128],
              [-8828999.3961, 5416184.3366],
              [-8829001.21, 5415294.1454],
              [-8829003.0217, 5414404.0391],
              [-8829004.8311, 5413514.0177],
              [-8829006.6382, 5412624.0811],
              [-8829008.4431, 5411734.2294],
              [-8829010.2458, 5410844.4625],
              [-8829012.0461, 5409954.7804],
              [-8829013.8443, 5409065.1831],
              [-8829015.6402, 5408175.6705],
              [-8829017.4338, 5407286.2426],
              [-8829019.2252, 5406396.8994],
              [-8829021.0143, 5405507.6409],
              [-8829022.8012, 5404618.467],
              [-8829024.5858, 5403729.3777],
              [-8829026.3682, 5402840.373],
              [-8829028.1483, 5401951.4529],
              [-8829029.9262, 5401062.6173],
              [-8829909.6175, 5401080.5051],
              [-8830789.314, 5401098.309],
              [-8831669.0158, 5401116.0291],
              [-8832548.7227, 5401133.6653],
              [-8833428.4349, 5401151.2176],
              [-8834308.1522, 5401168.686],
              [-8835187.8746, 5401186.0705],
              [-8836067.6022, 5401203.3712],
              [-8836947.3348, 5401220.588],
              [-8837827.0725, 5401237.7209],
              [-8838706.8152, 5401254.7699],
              [-8839586.5629, 5401271.735],
              [-8840466.3156, 5401288.6162],
              [-8841346.0733, 5401305.4136],
              [-8842225.8359, 5401322.127],
              [-8843105.6034, 5401338.7566],
              [-8843985.3758, 5401355.3023],
              [-8844865.1531, 5401371.764],
              [-8844865.5756, 5402260.1897],
              [-8844865.9966, 5403148.6999],
              [-8844866.4158, 5404037.2946],
              [-8844866.8334, 5404925.9738],
              [-8844867.2493, 5405814.7376],
              [-8844867.6636, 5406703.5859],
              [-8844868.0762, 5407592.5188],
              [-8844868.4871, 5408481.5363],
              [-8844868.8964, 5409370.6385],
              [-8844869.304, 5410259.8253],
              [-8844869.7099, 5411149.0969],
              [-8844870.1142, 5412038.4531],
              [-8844870.5168, 5412927.8941],
              [-8844870.9177, 5413817.4199],
              [-8844871.317, 5414707.0304],
              [-8844871.7146, 5415596.7258],
              [-8844872.1105, 5416486.506],
              [-8844872.5048, 5417376.3711],
              [-8844872.8973, 5418266.3211],
              [-8844873.2883, 5419156.356],
              [-8844873.6775, 5420046.4758],
              [-8844874.0651, 5420936.6807],
              [-8844821.9608, 5420935.7059],
              [-8844822.16, 5420924.9838],
              [-8843997.4267, 5420909.5149],
              [-8843172.6977, 5420893.9721],
              [-8842347.973, 5420878.3554],
              [-8841523.2526, 5420862.6648],
              [-8840698.5365, 5420846.9003],
              [-8839873.8248, 5420831.0619],
              [-8839049.1175, 5420815.1496],
              [-8838224.4145, 5420799.1635],
              [-8837399.716, 5420783.1034]
            ]
          ],
          "spatialReference": {
            "wkid": 102100,
            "latestWkid": 3857
          }
        }
      },
      {
        "attributes": {
          "OBJECTID": 2,
          "Name": "po_578117_metadata.txt:0000000",
          "MinPS": 2,
          "MaxPS": 200,
          "LowPS": 4,
          "HighPS": 4,
          "Category": 1,
          "Tag": "MS",
          "GroupName": "578117_0000000",
          "ProductName": "Geo",
          "CenterX": -8836939.93022586,
          "CenterY": 5411011.83316092,
          "ZOrder": null,
          "StereoID": "2007070316182540000011619942",
          "BlockName": "578117",
          "SensorName": "IKONOS-2",
          "AcquisitionDate": 1183479480000,
          "SunAzimuth": 142.0678,
          "SunElevation": 65.59146,
          "SatAzimuth": 144.9171,
          "SatElevation": 79.46645,
          "CloudCover": 0,
          "SrcImgID": "2007070316182540000011619942",
          "OffNadir": 10.53355,
          "Shape_Length": 70874.4933548685,
          "Shape_Area": 310449405.817243
        },
        "geometry": {
          "rings": [
            [
              [-8829028.1483, 5401951.4529],
              [-8829029.9262, 5401062.6173],
              [-8829909.6175, 5401080.5051],
              [-8830789.314, 5401098.309],
              [-8831669.0158, 5401116.0291],
              [-8832548.7227, 5401133.6653],
              [-8833428.4349, 5401151.2176],
              [-8834308.1522, 5401168.686],
              [-8835187.8746, 5401186.0705],
              [-8836067.6022, 5401203.3712],
              [-8836947.3348, 5401220.588],
              [-8837827.0725, 5401237.7209],
              [-8838706.8152, 5401254.7699],
              [-8839586.5629, 5401271.735],
              [-8840466.3156, 5401288.6162],
              [-8841346.0733, 5401305.4136],
              [-8842225.8359, 5401322.127],
              [-8843105.6034, 5401338.7566],
              [-8843985.3758, 5401355.3023],
              [-8844865.1531, 5401371.764],
              [-8844865.5666, 5402260.6761],
              [-8844865.9785, 5403149.6728],
              [-8844866.3888, 5404038.7541],
              [-8844866.7973, 5404927.92],
              [-8844867.2042, 5405817.1705],
              [-8844867.6095, 5406706.5056],
              [-8844868.013, 5407595.9255],
              [-8844868.4149, 5408485.4301],
              [-8844868.8151, 5409375.0194],
              [-8844869.2137, 5410264.6934],
              [-8844869.6106, 5411154.4523],
              [-8844870.0058, 5412044.2959],
              [-8844870.3993, 5412934.2244],
              [-8844870.7912, 5413824.2378],
              [-8844871.1814, 5414714.336],
              [-8844871.5699, 5415604.5192],
              [-8844871.9568, 5416494.7873],
              [-8844872.3419, 5417385.1404],
              [-8844872.7254, 5418275.5784],
              [-8844873.1073, 5419166.1015],
              [-8844873.4874, 5420056.7096],
              [-8844873.8659, 5420947.4027],
              [-8843991.4024, 5420930.8532],
              [-8843108.9438, 5420914.219],
              [-8842226.4902, 5420897.5002],
              [-8841344.0414, 5420880.6969],
              [-8840461.5977, 5420863.8089],
              [-8839579.1589, 5420846.8363],
              [-8838696.7252, 5420829.7791],
              [-8837814.2965, 5420812.6374],
              [-8836931.8729, 5420795.411],
              [-8836049.4544, 5420778.1001],
              [-8835167.041, 5420760.7045],
              [-8834284.6327, 5420743.2244],
              [-8833402.2296, 5420725.6597],
              [-8832519.8318, 5420708.0104],
              [-8831637.4391, 5420690.2765],
              [-8830755.0517, 5420672.458],
              [-8829872.6695, 5420654.555],
              [-8828990.2927, 5420636.5673],
              [-8828992.1179, 5419745.9511],
              [-8828993.9408, 5418855.42],
              [-8828995.7615, 5417964.9739],
              [-8828997.5799, 5417074.6128],
              [-8828999.3961, 5416184.3366],
              [-8829001.21, 5415294.1454],
              [-8829003.0217, 5414404.0391],
              [-8829004.8311, 5413514.0177],
              [-8829006.6382, 5412624.0811],
              [-8829008.4431, 5411734.2294],
              [-8829010.2458, 5410844.4625],
              [-8829012.0461, 5409954.7804],
              [-8829013.8443, 5409065.1831],
              [-8829015.6402, 5408175.6705],
              [-8829017.4338, 5407286.2426],
              [-8829019.2252, 5406396.8994],
              [-8829021.0143, 5405507.6409],
              [-8829022.8012, 5404618.467],
              [-8829024.5858, 5403729.3777],
              [-8829026.3682, 5402840.373],
              [-8829028.1483, 5401951.4529]
            ]
          ],
          "spatialReference": {
            "wkid": 102100,
            "latestWkid": 3857
          }
        }
      }
    ]
  },
  "catalogItemVisibilities": [1, 0]
}"""
                )

        rl = QgsRasterLayer(
            "url='http://" + endpoint + "'",
            "test",
            "arcgisimageserver",
        )
        self.assertTrue(rl.isValid())
        self.assertEqual(rl.bandCount(), 4)
        self.assertEqual(rl.bandName(1), "Blue")
        self.assertEqual(rl.bandName(2), "Green")
        self.assertEqual(rl.bandName(3), "Red")
        self.assertEqual(rl.bandName(4), "NearInfrared")
        self.assertEqual(
            rl.dataProvider().colorInterpretation(1),
            Qgis.RasterColorInterpretation.BlueBand,
        )
        self.assertEqual(
            rl.dataProvider().colorInterpretation(2),
            Qgis.RasterColorInterpretation.GreenBand,
        )
        self.assertEqual(
            rl.dataProvider().colorInterpretation(3),
            Qgis.RasterColorInterpretation.RedBand,
        )
        self.assertEqual(
            rl.dataProvider().colorInterpretation(4),
            Qgis.RasterColorInterpretation.NIRBand,
        )
        self.assertEqual(rl.dataProvider().dataType(1), Qgis.DataType.UInt16)
        self.assertEqual(rl.dataProvider().dataType(2), Qgis.DataType.UInt16)
        self.assertEqual(rl.dataProvider().dataType(3), Qgis.DataType.UInt16)
        self.assertEqual(rl.dataProvider().dataType(4), Qgis.DataType.UInt16)

        # we always populate nodata values for integers, so that we can
        # safely pad raster blocks which were requested outside of the
        # providers extent
        for i in range(1, 5):
            self.assertTrue(rl.dataProvider().sourceHasNoDataValue(i))
            self.assertTrue(rl.dataProvider().useSourceNoDataValue(i))
            self.assertEqual(rl.dataProvider().sourceNoDataValue(i), 65535)

        self.assertFalse(rl.elevationProperties().isEnabled())

        self.assertTrue(
            rl.dataProvider().hasStatistics(
                1,
                Qgis.RasterBandStatistic.Min
                | Qgis.RasterBandStatistic.Max
                | Qgis.RasterBandStatistic.Range
                | Qgis.RasterBandStatistic.Mean
                | Qgis.RasterBandStatistic.StdDev,
            )
        )
        # sum of squares not available:
        self.assertFalse(
            rl.dataProvider().hasStatistics(
                1,
                Qgis.RasterBandStatistic.Min
                | Qgis.RasterBandStatistic.Max
                | Qgis.RasterBandStatistic.Range
                | Qgis.RasterBandStatistic.Mean
                | Qgis.RasterBandStatistic.StdDev
                | Qgis.RasterBandStatistic.SumOfSquares,
            )
        )

        band_stats = rl.dataProvider().bandStatistics(
            1,
            Qgis.RasterBandStatistic.Min
            | Qgis.RasterBandStatistic.Max
            | Qgis.RasterBandStatistic.Range
            | Qgis.RasterBandStatistic.Mean
            | Qgis.RasterBandStatistic.StdDev,
        )
        self.assertEqual(band_stats.bandNumber, 1)
        self.assertEqual(band_stats.maximumValue, 2047.0)
        self.assertEqual(band_stats.minimumValue, 195.0)
        self.assertEqual(band_stats.mean, 407.6350849285231)
        self.assertEqual(band_stats.range, 1852.0)
        self.assertEqual(band_stats.stdDev, 103.3454476143749)
        self.assertEqual(
            band_stats.statsGathered,
            Qgis.RasterBandStatistic.Min
            | Qgis.RasterBandStatistic.Max
            | Qgis.RasterBandStatistic.Range
            | Qgis.RasterBandStatistic.Mean
            | Qgis.RasterBandStatistic.StdDev,
        )

        band_stats = rl.dataProvider().bandStatistics(
            2,
            Qgis.RasterBandStatistic.Min
            | Qgis.RasterBandStatistic.Max
            | Qgis.RasterBandStatistic.Range
            | Qgis.RasterBandStatistic.Mean
            | Qgis.RasterBandStatistic.StdDev,
        )
        self.assertEqual(band_stats.bandNumber, 2)
        self.assertEqual(band_stats.maximumValue, 2047.0)
        self.assertEqual(band_stats.minimumValue, 168.0)
        self.assertEqual(band_stats.mean, 424.3829909000868)
        self.assertEqual(band_stats.range, 1879.0)
        self.assertEqual(band_stats.stdDev, 161.37726805365864)

        band_stats = rl.dataProvider().bandStatistics(
            3,
            Qgis.RasterBandStatistic.Min
            | Qgis.RasterBandStatistic.Max
            | Qgis.RasterBandStatistic.Range
            | Qgis.RasterBandStatistic.Mean
            | Qgis.RasterBandStatistic.StdDev,
        )
        self.assertEqual(band_stats.bandNumber, 3)
        self.assertEqual(band_stats.maximumValue, 2047.0)
        self.assertEqual(band_stats.minimumValue, 1.0)
        self.assertEqual(band_stats.mean, 306.3585015423408)
        self.assertEqual(band_stats.range, 2046.0)
        self.assertEqual(band_stats.stdDev, 177.16151769901987)

        band_stats = rl.dataProvider().bandStatistics(
            4,
            Qgis.RasterBandStatistic.Min
            | Qgis.RasterBandStatistic.Max
            | Qgis.RasterBandStatistic.Range
            | Qgis.RasterBandStatistic.Mean
            | Qgis.RasterBandStatistic.StdDev,
        )
        self.assertEqual(band_stats.bandNumber, 4)
        self.assertEqual(band_stats.maximumValue, 2047.0)
        self.assertEqual(band_stats.minimumValue, 1.0)
        self.assertEqual(band_stats.mean, 451.4521619746197)
        self.assertEqual(band_stats.range, 2046.0)
        self.assertEqual(band_stats.stdDev, 330.19574780626806)

        # test identify

        # not supported format
        self.assertFalse(
            rl.dataProvider()
            .identify(
                QgsPointXY(-8836932, 5411004.0),
                Qgis.RasterIdentifyFormat.Html,
                QgsRectangle(),
                0,
                0,
            )
            .isValid()
        )
        # pixel outside of extent
        res = rl.dataProvider().identify(
            QgsPointXY(-118836932, 225411004.0),
            Qgis.RasterIdentifyFormat.Value,
            QgsRectangle(),
            0,
            0,
        )
        self.assertTrue(res.isValid())
        self.assertEqual(res.results(), {1: None, 2: None, 3: None, 4: None})

        # valid identify request
        res = rl.dataProvider().identify(
            QgsPointXY(-8836932, 5411004.0),
            Qgis.RasterIdentifyFormat.Value,
            QgsRectangle(),
            0,
            0,
        )
        self.assertTrue(res.isValid())
        self.assertEqual(res.results(), {1: 879.0, 2: 1114.0, 3: 1032.0, 4: 975.0})

    def test_nodata_values(self):
        """
        Test basic provider capabilities for multi-band layer
        """
        endpoint = self.basetestpath + "/basic_test_multiband_fake_qgis_http_endpoint"
        with open(self.sanitize_local_url(endpoint, "?f=json"), "wb") as f:
            f.write(
                b"""

{
  "currentVersion" : 12,
  "name" : "GEBCO_2021_v2_Depth_Ranges",
  "serviceDescription" : "",
  "description" : "",
  "type" : "ImageServer",
  "capabilities" : "Image",
  "copyrightText" : "",
  "serviceItemId" : "4915fa5b342d412fa5188681751491eb",
  "meanPixelSize" : 463.8312116386403,
  "server" : "P3ePLMYs2RVChkJx",
  "created" : null,
  "modified" : null,
  "status" : "created",
  "access" : "SECURE",
  "allowAnalysis" : true,
  "allowCopy" : false,
  "serviceSourceType" : "esriImageServiceSourceTypeDataset",
  "spatialReference" : {
    "wkid" : 4326,
    "latestWkid" : 4326
  },
  "extent" : {
    "xmin" : -180,
    "ymin" : -90,
    "xmax" : 180,
    "ymax" : 90,
    "spatialReference" : {
      "wkid" : 4326,
      "latestWkid" : 4326
    }
  },
  "initialExtent" : {
    "xmin" : -180,
    "ymin" : -90,
    "xmax" : 180,
    "ymax" : 90,
    "spatialReference" : {
      "wkid" : 4326,
      "latestWkid" : 4326
    }
  },
  "fullExtent" : {
    "xmin" : -180,
    "ymin" : -90,
    "xmax" : 180,
    "ymax" : 90,
    "spatialReference" : {
      "wkid" : 4326,
      "latestWkid" : 4326
    }
  },
  "datasetFormat" : "Cache/LERC2D",
  "compressionType" : "LERC",
  "uncompressedSize" : 7464700802,
  "bsq" : false,
  "pixelSizeX" : 0.00416666666666667,
  "pixelSizeY" : 0.00416666666666667,
  "blockWidth" : 256,
  "blockHeight" : 256,
  "bandCount" : 1,
  "bandNames" : [
    "Band 0"
  ],
  "pixelType" : "S16",
  "minPixelSize" : 0.0041666666666666666,
  "maxPixelSize" : 1.0666666666666667,
  "minScale" : 448280793.02542216,
  "maxScale" : 1751096.8477555553,
  "serviceDataType" : "esriImageServiceDataTypeThematic",
  "noDataValue" : 32767,
  "noDataValues" : [
    32767
  ],
  "minValues" : [
    1
  ],
  "maxValues" : [
    7
  ],
  "meanValues" : [
    5.0427777764381805
  ],
  "stdvValues" : [
    1.741604907714148
  ],
  "objectIdField" : "",
  "fields" : [

  ],
  "defaultMosaicMethod" : "",
  "allowedMosaicMethods" : "",
  "sortField" : "",
  "sortValue" : null,
  "sortAscending" : true,
  "singleFusedMapCache" : false,
  "tileInfo" : {
    "rows" : 256,
    "cols" : 256,
    "dpi" : 96,
    "preciseDpi" : 96,
    "format" : "LERC2D",
    "storageFormat" : "esriMapCacheStorageModeCompactV2",
    "packetSize" : 16,
    "compressionQuality" : 0,
    "antialiasing" : false,
    "origin" : {
      "x" : -180,
      "y" : 90
    },
    "spatialReference" : {
      "wkid" : 4326,
      "latestWkid" : 4326
    },
    "lods" : [{
        "level" : 0,
        "resolution" : 1.0666666666666667,
        "scale" : 448280793.02542216
      },
      {
        "level" : 1,
        "resolution" : 0.53333333333333333,
        "scale" : 224140396.51271108
      },
      {
        "level" : 2,
        "resolution" : 0.26666666666666666,
        "scale" : 112070198.25635554
      },
      {
        "level" : 3,
        "resolution" : 0.13333333333333333,
        "scale" : 56035099.128177769
      },
      {
        "level" : 4,
        "resolution" : 0.066666666666666666,
        "scale" : 28017549.564088885
      },
      {
        "level" : 5,
        "resolution" : 0.033333333333333333,
        "scale" : 14008774.782044442
      },
      {
        "level" : 6,
        "resolution" : 0.016666666666666666,
        "scale" : 7004387.3910222212
      },
      {
        "level" : 7,
        "resolution" : 0.0083333333333333332,
        "scale" : 3502193.6955111106
      },
      {
        "level" : 8,
        "resolution" : 0.0041666666666666666,
        "scale" : 1751096.8477555553
      }
    ]
  },
  "cacheType" : "Raster",
  "allowRasterFunction" : false,
  "hasHistograms" : true,
  "hasColormap" : false,
  "exportTilesAllowed" : false,
  "maxExportTilesCount" : 100000,
  "hasMultidimensions" : false
}"""
            )

        rl = QgsRasterLayer(
            "url='http://" + endpoint + "'",
            "test",
            "arcgisimageserver",
        )
        self.assertTrue(rl.isValid())
        self.assertEqual(rl.bandCount(), 1)
        self.assertTrue(rl.dataProvider().sourceHasNoDataValue(1))
        self.assertTrue(rl.dataProvider().useSourceNoDataValue(1))
        self.assertEqual(rl.dataProvider().sourceNoDataValue(1), 32767.0)

    @unittest.skipIf(
        not QgsGdalUtils.supportsMrfLercCompression(),
        "GDAL build with LERC support required",
    )
    def test_fetch_block_lerc(self):
        """
        Test fetching a block using LERC format for F32 scientific data
        """
        endpoint = self.basetestpath + "/fetch_lerc_fake_qgis_http_endpoint"

        with open(self.sanitize_local_url(endpoint, "?f=json"), "wb") as f:
            f.write(
                b"""{
  "currentVersion": 10.91,
  "pixelType": "F32",
  "capabilities": "Image",
  "fullExtent": {
    "xmin": 0,
    "ymin": 0,
    "xmax": 10,
    "ymax": 10,
    "spatialReference": {
      "wkid": 4326
    }
  },
  "bandCount": 1,
  "type": "ImageServer",
  "serviceSourceType": "esriImageServiceSourceTypeDataset",
  "serviceDataType": "esriImageServiceDataTypeElevation",
  "spatialReference": {
    "wkid": 4326,
    "latestWkid": 4326
  }
}"""
            )

        rl = QgsRasterLayer(
            "url='http://" + endpoint + "'", "test", "arcgisimageserver"
        )
        self.assertTrue(rl.isValid())

        # generate a valid 2x2 LERC blob using GDAL VSI
        mem_ds = gdal.GetDriverByName("MEM").Create("", 2, 2, 1, gdal.GDT_Float32)
        # raster data = 10.5, 20.5, 30.5, 40.5
        mem_ds.GetRasterBand(1).WriteRaster(
            0, 0, 2, 2, struct.pack("f" * 4, 10.5, 20.5, 30.5, 40.5)
        )
        gdal.GetDriverByName("GTiff").CreateCopy(
            "/vsimem/test.tiff", mem_ds, options=["COMPRESS=LERC"]
        )
        vsi_file = gdal.VSIFOpenL("/vsimem/test.tiff", "rb")
        gdal.VSIFSeekL(vsi_file, 0, 2)
        size = gdal.VSIFTellL(vsi_file)
        gdal.VSIFSeekL(vsi_file, 0, 0)
        lerc_blob = gdal.VSIFReadL(1, size, vsi_file)
        gdal.VSIFCloseL(vsi_file)
        gdal.Unlink("/vsimem/test.lrc")
        query = "/exportImage?bbox=0.000000,0.000000,10.000000,10.000000&size=2,2&f=image&bandIds=0&interpretation=RSP_BilinearInterpolation&pixelType=F32&lercVersion=2&compression=LERC&compressionTolerance=0&format=lerc"
        with open(self.sanitize_local_url(endpoint, query), "wb") as f:
            f.write(lerc_blob)

        block = rl.dataProvider().block(1, QgsRectangle(0, 0, 10, 10), 2, 2)
        self.assertTrue(block.isValid())

        self.assertAlmostEqual(block.value(0, 0), 10.5, places=3)
        self.assertAlmostEqual(block.value(0, 1), 20.5, places=3)
        self.assertAlmostEqual(block.value(1, 0), 30.5, places=3)
        self.assertAlmostEqual(block.value(1, 1), 40.5, places=3)

        # fetch block outside raster extent
        block = rl.dataProvider().block(1, QgsRectangle(20, 20, 30, 30), 2, 2)
        self.assertFalse(block.isValid())

        # invalid size
        block = rl.dataProvider().block(1, QgsRectangle(0, 0, 10, 10), 0, 0)
        self.assertFalse(block.isValid())

    def test_fetch_block_tiff(self):
        """
        Test fetching a block using standard TIFF format (e.g. U16 multiband fallback)
        """
        endpoint = self.basetestpath + "/fetch_tiff_fake_qgis_http_endpoint"
        with open(self.sanitize_local_url(endpoint, "?f=json"), "wb") as f:
            f.write(
                b"""{
  "currentVersion": 10.91,
  "pixelType": "U16",
  "capabilities": "Image",
  "fullExtent": {
    "xmin": 0,
    "ymin": 0,
    "xmax": 10,
    "ymax": 10,
    "spatialReference": {
      "wkid": 4326
    }
  },
  "bandCount": 1,
  "type": "ImageServer",
  "serviceSourceType": "esriImageServiceSourceTypeDataset",
  "serviceDataType": "esriImageServiceDataTypeElevation",
  "spatialReference": {
    "wkid": 4326,
    "latestWkid": 4326
  }
}"""
            )

        # 2x2 uncompressed TIFF blob
        mem_ds = gdal.GetDriverByName("MEM").Create("", 2, 2, 1, gdal.GDT_UInt16)
        # raster values = 100, 200, 300, 400
        mem_ds.GetRasterBand(1).WriteRaster(
            0, 0, 2, 2, struct.pack("H" * 4, 100, 200, 300, 400)
        )
        gdal.GetDriverByName("GTiff").CreateCopy("/vsimem/test.tif", mem_ds)

        vsi_file = gdal.VSIFOpenL("/vsimem/test.tif", "rb")
        gdal.VSIFSeekL(vsi_file, 0, 2)
        size = gdal.VSIFTellL(vsi_file)
        gdal.VSIFSeekL(vsi_file, 0, 0)
        tiff_blob = gdal.VSIFReadL(1, size, vsi_file)
        gdal.VSIFCloseL(vsi_file)
        gdal.Unlink("/vsimem/test.tif")

        query = "/exportImage?bbox=0.000000,0.000000,10.000000,10.000000&size=2,2&f=image&bandIds=0&interpretation=RSP_BilinearInterpolation&pixelType=U16&format=tiff"

        with open(self.sanitize_local_url(endpoint, query), "wb") as f:
            f.write(tiff_blob)

        rl = QgsRasterLayer(
            "url='http://" + endpoint + "'", "test", "arcgisimageserver"
        )
        self.assertTrue(rl.isValid())

        block = rl.dataProvider().block(1, QgsRectangle(0, 0, 10, 10), 2, 2)
        self.assertTrue(block.isValid())

        self.assertEqual(block.value(0, 0), 100)
        self.assertEqual(block.value(0, 1), 200)
        self.assertEqual(block.value(1, 0), 300)
        self.assertEqual(block.value(1, 1), 400)

    def test_fetch_block_jpg(self):
        """
        Test fetching a block when the user forces format=jpg via the connection URI
        """
        endpoint = self.basetestpath + "/fetch_jpg_fake_qgis_http_endpoint"

        with open(self.sanitize_local_url(endpoint, "?f=json"), "wb") as f:
            f.write(
                b"""{
  "currentVersion": 10.91,
  "pixelType": "U8",
  "extent": {
    "xmin": 0,
    "ymin": 0,
    "xmax": 10,
    "ymax": 10,
    "spatialReference": {
      "wkid": 4326
    }
  },
  "capabilities": "Image",
  "bandCount": 1,
  "type": "ImageServer",
  "serviceSourceType": "esriImageServiceSourceTypeDataset",
  "serviceDataType": "esriImageServiceDataTypeElevation",
  "spatialReference": {
    "wkid": 4326,
    "latestWkid": 4326
  }
}"""
            )

        # 2x2 JPG blob
        mem_ds = gdal.GetDriverByName("MEM").Create("", 2, 2, 1, gdal.GDT_Byte)
        # raster data = 50, 100, 150, 200
        mem_ds.GetRasterBand(1).WriteRaster(
            0, 0, 2, 2, struct.pack("B" * 4, 50, 100, 150, 200)
        )

        gdal.GetDriverByName("JPEG").CreateCopy("/vsimem/test.jpg", mem_ds)

        vsi_file = gdal.VSIFOpenL("/vsimem/test.jpg", "rb")
        gdal.VSIFSeekL(vsi_file, 0, 2)
        size = gdal.VSIFTellL(vsi_file)
        gdal.VSIFSeekL(vsi_file, 0, 0)
        jpg_blob = gdal.VSIFReadL(1, size, vsi_file)
        gdal.VSIFCloseL(vsi_file)
        gdal.Unlink("/vsimem/test.jpg")

        query = "/exportImage?bbox=0.000000,0.000000,10.000000,10.000000&size=2,2&f=image&bandIds=0&interpretation=RSP_BilinearInterpolation&pixelType=U8&format=jpg"
        with open(self.sanitize_local_url(endpoint, query), "wb") as f:
            f.write(jpg_blob)

        # forcefully request JPG format via the URI
        rl = QgsRasterLayer(
            "url='http://" + endpoint + "' format='jpg'", "test", "arcgisimageserver"
        )
        self.assertTrue(rl.isValid())

        block = rl.dataProvider().block(1, QgsRectangle(0, 0, 10, 10), 2, 2)
        self.assertTrue(block.isValid())

        # since JPEG is lossy, we use a high tolerance when reading back values
        self.assertAlmostEqual(block.value(0, 0), 50, delta=15)
        self.assertAlmostEqual(block.value(0, 1), 100, delta=15)
        self.assertAlmostEqual(block.value(1, 0), 150, delta=15)
        self.assertAlmostEqual(block.value(1, 1), 200, delta=15)

    def test_raster_attribute_table(self):
        """
        Test fetching raster attribute table
        """
        endpoint = self.basetestpath + "/fetch_rat_fake_qgis_http_endpoint"

        with open(self.sanitize_local_url(endpoint, "?f=json"), "wb") as f:
            f.write(
                b"""{
  "currentVersion": 10.91,
  "pixelType": "U8",
  "extent": {
    "xmin": 0,
    "ymin": 0,
    "xmax": 10,
    "ymax": 10,
    "spatialReference": {
      "wkid": 4326
    }
  },
  "capabilities": "Image",
  "bandCount": 1,
  "type": "ImageServer",
  "serviceSourceType": "esriImageServiceSourceTypeDataset",
  "serviceDataType": "esriImageServiceDataTypeElevation",
  "spatialReference": {
    "wkid": 4326,
    "latestWkid": 4326
  },
 "hasRasterAttributeTable": true
}"""
            )

        with open(
            self.sanitize_local_url(
                endpoint,
                "/rasterAttributeTable?f=json",
            ),
            "wb",
        ) as f:
            f.write(
                b"""
                {
 "objectIdFieldName": "OBJECTID",
 "fields": [
  {
   "name": "OID",
   "type": "esriFieldTypeOID",
   "alias": "OID",
   "domain": null
  },
  {
   "name": "Value",
   "type": "esriFieldTypeInteger",
   "alias": "Value",
   "domain": null
  },
  {
   "name": "Count",
   "type": "esriFieldTypeDouble",
   "alias": "Count",
   "domain": null
  },
  {
   "name": "Red",
   "type": "esriFieldTypeSmallInteger",
   "alias": "Red",
   "domain": null
  },
  {
   "name": "Green",
   "type": "esriFieldTypeSmallInteger",
   "alias": "Green",
   "domain": null
  },
  {
   "name": "Blue",
   "type": "esriFieldTypeSmallInteger",
   "alias": "Blue",
   "domain": null
  },
  {
   "name": "ClassName",
   "type": "esriFieldTypeString",
   "alias": "ClassName",
   "domain": null,
   "length": 50
  }
 ],
 "features": [
  {
   "attributes": {
    "OID": 0,
    "Value": 11,
    "Count": 466650898,
    "Red": 71,
    "Green": 107,
    "Blue": 161,
    "ClassName": "Open Water"
   }
  },
  {
   "attributes": {
    "OID": 1,
    "Value": 12,
    "Count": 1731114,
    "Red": 209,
    "Green": 222,
    "Blue": 250,
    "ClassName": "Perennial Snow/Ice"
   }
  },
  {
   "attributes": {
    "OID": 2,
    "Value": 21,
    "Count": 287572411,
    "Red": 222,
    "Green": 202,
    "Blue": 202,
    "ClassName": "Developed, Open Space"
   }
  },
  {
   "attributes": {
    "OID": 3,
    "Value": 22,
    "Count": 124755469,
    "Red": 217,
    "Green": 148,
    "Blue": 130,
    "ClassName": "Developed, Low Intensity"
   }
  },
  {
   "attributes": {
    "OID": 4,
    "Value": 23,
    "Count": 48230587,
    "Red": 238,
    "Green": 0,
    "Blue": 0,
    "ClassName": "Developed, Medium Intensity"
   }
  },
  {
   "attributes": {
    "OID": 5,
    "Value": 24,
    "Count": 16773628,
    "Red": 171,
    "Green": 0,
    "Blue": 0,
    "ClassName": "Developed, High Intensity"
   }
  },
  {
   "attributes": {
    "OID": 6,
    "Value": 31,
    "Count": 106587346,
    "Red": 179,
    "Green": 174,
    "Blue": 163,
    "ClassName": "Barren Land"
   }
  }
 ]
}
"""
            )
        rl = QgsRasterLayer(
            "url='http://" + endpoint + "'", "test", "arcgisimageserver"
        )
        self.assertTrue(rl.isValid())

        rat = rl.dataProvider().attributeTable(1)
        self.assertIsNotNone(rat)

        self.assertTrue(rat.hasColor())

        fields = rat.fields()
        self.assertEqual(
            [f.name for f in fields],
            ["OID", "Value", "Count", "Red", "Green", "Blue", "ClassName"],
        )
        self.assertEqual(
            [f.usage for f in fields],
            [
                Qgis.RasterAttributeTableFieldUsage.Generic,
                Qgis.RasterAttributeTableFieldUsage.MinMax,
                Qgis.RasterAttributeTableFieldUsage.PixelCount,
                Qgis.RasterAttributeTableFieldUsage.Red,
                Qgis.RasterAttributeTableFieldUsage.Green,
                Qgis.RasterAttributeTableFieldUsage.Blue,
                Qgis.RasterAttributeTableFieldUsage.Name,
            ],
        )
        self.assertEqual(
            [f.type for f in fields],
            [
                QMetaType.Type.LongLong,
                QMetaType.Type.LongLong,
                QMetaType.Type.Double,
                QMetaType.Type.Int,
                QMetaType.Type.Int,
                QMetaType.Type.Int,
                QMetaType.Type.QString,
            ],
        )
        self.assertEqual(
            rat.data(),
            [
                [0, 11, 466650898.0, 71, 107, 161, "Open Water"],
                [1, 12, 1731114.0, 209, 222, 250, "Perennial Snow/Ice"],
                [2, 21, 287572411.0, 222, 202, 202, "Developed, Open Space"],
                [3, 22, 124755469.0, 217, 148, 130, "Developed, Low Intensity"],
                [4, 23, 48230587.0, 238, 0, 0, "Developed, Medium Intensity"],
                [5, 24, 16773628.0, 171, 0, 0, "Developed, High Intensity"],
                [6, 31, 106587346.0, 179, 174, 163, "Barren Land"],
            ],
        )

    def test_fetch_block_tiled_one_tile(self):
        """
        Test fetching data from a tiled service, one tile required only
        """
        endpoint = self.basetestpath + "/fetch_tiled_fake_qgis_http_endpoint"

        with open(self.sanitize_local_url(endpoint, "?f=json"), "wb") as f:
            f.write(
                b"""{
  "currentVersion": 10.91,
  "pixelType": "U16",
  "capabilities": "Image",
  "singleFusedMapCache": true,
  "serviceDataType": "esriImageServiceDataTypeElevation",
  "minLOD": 0,
  "maxLOD": 0,
  "fullExtent": {
    "xmin": 0,
    "ymin": 0,
    "xmax": 10,
    "ymax": 10,
    "spatialReference": {"wkid": 4326}
  },
  "bandCount": 1,
  "type": "ImageServer",
  "tileInfo": {
    "rows": 2,
    "cols": 2,
    "format": "TIFF",
    "origin": {"x": 0, "y": 10},
    "spatialReference": {"wkid": 4326},
    "lods": [
      {"level": 0, "resolution": 5.0, "scale": 10000}
    ]
  }
}"""
            )

        # 2x2 TIFF for tile at /tile/0/0/0
        mem_ds = gdal.GetDriverByName("MEM").Create("", 2, 2, 1, gdal.GDT_UInt16)
        mem_ds.GetRasterBand(1).WriteRaster(
            0, 0, 2, 2, struct.pack("H" * 4, 11, 22, 33, 44)
        )
        gdal.GetDriverByName("GTiff").CreateCopy("/vsimem/test_tile.tif", mem_ds)
        vsi_file = gdal.VSIFOpenL("/vsimem/test_tile.tif", "rb")
        gdal.VSIFSeekL(vsi_file, 0, 2)
        size = gdal.VSIFTellL(vsi_file)
        gdal.VSIFSeekL(vsi_file, 0, 0)
        tiff_blob = gdal.VSIFReadL(1, size, vsi_file)
        gdal.VSIFCloseL(vsi_file)
        gdal.Unlink("/vsimem/test_tile.tif")

        with open(self.sanitize_local_url(endpoint, "/tile/0/0/0"), "wb") as f:
            f.write(tiff_blob)

        rl = QgsRasterLayer(
            "url='http://" + endpoint + "'", "test", "arcgisimageserver"
        )
        self.assertTrue(rl.isValid())

        block = rl.dataProvider().block(1, QgsRectangle(0, 0, 10, 10), 2, 2)
        self.assertTrue(block.isValid())

        self.assertEqual(block.value(0, 0), 11)
        self.assertEqual(block.value(0, 1), 22)
        self.assertEqual(block.value(1, 0), 33)
        self.assertEqual(block.value(1, 1), 44)

    def test_fetch_block_tiled_multiple(self):
        """
        Test fetching a block from a tiled service that requires downloading
        and stitching multiple tiles together
        """
        endpoint = self.basetestpath + "/fetch_tiled_multi_fake_qgis_http_endpoint"

        with open(self.sanitize_local_url(endpoint, "?f=json"), "wb") as f:
            f.write(
                b"""{
  "currentVersion": 10.91,
  "pixelType": "U16",
  "capabilities": "Image",
  "singleFusedMapCache": true,
  "serviceDataType": "esriImageServiceDataTypeElevation",
  "minLOD": 0,
  "maxLOD": 0,
  "fullExtent": {
    "xmin": 0,
    "ymin": 0,
    "xmax": 20,
    "ymax": 20,
    "spatialReference": {"wkid": 4326}
  },
  "bandCount": 1,
  "type": "ImageServer",
  "tileInfo": {
    "rows": 2,
    "cols": 2,
    "format": "TIFF",
    "origin": {"x": 0, "y": 20},
    "spatialReference": {"wkid": 4326},
    "lods": [
      {"level": 0, "resolution": 5.0, "scale": 10000}
    ]
  }
}"""
            )

        def create_tile(vals):
            mem_ds = gdal.GetDriverByName("MEM").Create("", 2, 2, 1, gdal.GDT_UInt16)
            mem_ds.GetRasterBand(1).WriteRaster(0, 0, 2, 2, struct.pack("H" * 4, *vals))
            gdal.GetDriverByName("GTiff").CreateCopy("/vsimem/test_tile.tif", mem_ds)
            vsi_file = gdal.VSIFOpenL("/vsimem/test_tile.tif", "rb")
            gdal.VSIFSeekL(vsi_file, 0, 2)
            size = gdal.VSIFTellL(vsi_file)
            gdal.VSIFSeekL(vsi_file, 0, 0)
            blob = gdal.VSIFReadL(1, size, vsi_file)
            gdal.VSIFCloseL(vsi_file)
            gdal.Unlink("/vsimem/test_tile.tif")
            return blob

        # mock the 4 tiles required to build the full 20x20 extent
        with open(
            self.sanitize_local_url(endpoint, "/tile/0/0/0"), "wb"
        ) as f:  # Top-Left
            f.write(create_tile([1, 2, 3, 4]))
        with open(
            self.sanitize_local_url(endpoint, "/tile/0/0/1"), "wb"
        ) as f:  # Top-Right
            f.write(create_tile([5, 6, 7, 8]))
        with open(
            self.sanitize_local_url(endpoint, "/tile/0/1/0"), "wb"
        ) as f:  # Bottom-Left
            f.write(create_tile([9, 10, 11, 12]))
        with open(
            self.sanitize_local_url(endpoint, "/tile/0/1/1"), "wb"
        ) as f:  # Bottom-Right
            f.write(create_tile([13, 14, 15, 16]))

        rl = QgsRasterLayer(
            "url='http://" + endpoint + "'", "test", "arcgisimageserver"
        )
        self.assertTrue(rl.isValid())

        block = rl.dataProvider().block(
            1, QgsRectangle(0.001, 0.001, 19.999, 19.999), 4, 4
        )
        self.assertTrue(block.isValid())
        self.assertEqual(block.value(0, 0), 1)
        self.assertEqual(block.value(0, 1), 2)
        self.assertEqual(block.value(0, 2), 5)
        self.assertEqual(block.value(0, 3), 6)
        self.assertEqual(block.value(1, 0), 3)
        self.assertEqual(block.value(1, 1), 4)
        self.assertEqual(block.value(1, 2), 7)
        self.assertEqual(block.value(1, 3), 8)
        self.assertEqual(block.value(2, 0), 9)
        self.assertEqual(block.value(2, 1), 10)
        self.assertEqual(block.value(2, 2), 13)
        self.assertEqual(block.value(2, 3), 14)
        self.assertEqual(block.value(3, 0), 11)
        self.assertEqual(block.value(3, 1), 12)
        self.assertEqual(block.value(3, 2), 15)
        self.assertEqual(block.value(3, 3), 16)


if __name__ == "__main__":
    unittest.main()
