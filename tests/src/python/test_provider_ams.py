"""QGIS Unit tests for the AMS provider.

From build dir, run: ctest -R PyQgsAMSProvider -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "2025-02-10"
__copyright__ = "Copyright 2025, Nyall Dawson"

import hashlib
import shutil
import tempfile
import unittest

from qgis.PyQt.QtCore import (
    QCoreApplication,
    QObject,
)
from qgis.core import (
    Qgis,
    QgsCoordinateReferenceSystem,
    QgsApplication,
    QgsSettings,
    QgsRasterLayer,
)
from qgis.testing import start_app, QgisTestCase


def sanitize(endpoint, x):
    if x.startswith("/query"):
        x = x[len("/query") :]
        endpoint = endpoint + "_query"

    if len(endpoint + x) > 150:
        ret = endpoint + hashlib.md5(x.encode()).hexdigest()
        # print('Before: ' + endpoint + x)
        # print('After:  ' + ret)
        return ret
    return endpoint + x.replace("?", "_").replace("&", "_").replace("<", "_").replace(
        ">", "_"
    ).replace('"', "_").replace("'", "_").replace(" ", "_").replace(":", "_").replace(
        "/", "_"
    ).replace(
        "\n", "_"
    )


class MessageLogger(QObject):

    def __init__(self, tag=None):
        QObject.__init__(self)
        self.log = []
        self.tag = tag

    def __enter__(self):
        QgsApplication.messageLog().messageReceived.connect(self.logMessage)
        return self

    def __exit__(self, type, value, traceback):
        QgsApplication.messageLog().messageReceived.disconnect(self.logMessage)

    def logMessage(self, msg, tag, level):
        if tag == self.tag or not self.tag:
            self.log.append(msg.encode("UTF-8"))

    def messages(self):
        return self.log


class TestPyQgsAMSProvider(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()

        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("TestPyQgsAMSProvider.com")
        QCoreApplication.setApplicationName("TestPyQgsAMSProvider")
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

    def test_basic(self):
        """
        Test basic provider capabilities
        """
        endpoint = self.basetestpath + "/basic_test_fake_qgis_http_endpoint"
        with open(sanitize(endpoint, "?f=json"), "wb") as f:
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
            "arcgismapserver",
        )
        self.assertTrue(rl.isValid())
        # basic raster layer properties
        self.assertEqual(rl.width(), 0)
        self.assertEqual(rl.height(), 0)
        self.assertEqual(rl.bandCount(), 1)
        self.assertEqual(rl.bandName(1), "Band 1")
        self.assertIsNone(rl.attributeTable(1))
        self.assertFalse(rl.canCreateRasterAttributeTable())
        self.assertEqual(rl.providerType(), "arcgismapserver")
        self.assertEqual(rl.rasterUnitsPerPixelX(), 1)
        self.assertEqual(rl.rasterUnitsPerPixelY(), 1)
        self.assertFalse(rl.subsetString())
        self.assertEqual(rl.subLayers(), ["1"])
        self.assertEqual(rl.crs(), QgsCoordinateReferenceSystem("EPSG:5514"))
        self.assertAlmostEqual(rl.extent().xMinimum(), -33699630.64526356756687164, 2)
        self.assertAlmostEqual(rl.extent().yMinimum(), -33699674.00237041711807251, 2)
        self.assertAlmostEqual(rl.extent().xMaximum(), 33699630.64526356756687164, 2)
        self.assertAlmostEqual(rl.extent().yMaximum(), 33632673.88108269870281219, 2)

        self.assertEqual(rl.dataProvider().dataType(1), Qgis.DataType.ARGB32)
        self.assertEqual(rl.dataProvider().sourceDataType(1), Qgis.DataType.ARGB32)
        self.assertEqual(rl.dataProvider().bandScale(1), 1)
        self.assertEqual(rl.dataProvider().bandOffset(1), 0)

    def test_basic_layer_1(self):
        """
        Test basic provider capabilities
        """
        endpoint = self.basetestpath + "/basic_layer_1_fake_qgis_http_endpoint"
        with open(sanitize(endpoint, "?f=json"), "wb") as f:
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
        with open(sanitize(endpoint, "/1?f=json"), "wb") as f:
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
   "id": 1,
   "name": "Layer 0",
"parentLayer": {
"id": 0,
"name": "Layer 0"
},
   "defaultVisibility": true,
   "subLayerIds": [],
   "minScale": 0,
   "maxScale": 0,
   "type": "Group Layer",
   "supportsDynamicLegends": false,
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
            "url='http://" + endpoint + "' layer='1'",
            "test",
            "arcgismapserver",
        )
        self.assertTrue(rl.isValid())
        # basic raster layer properties
        self.assertEqual(rl.width(), 0)
        self.assertEqual(rl.height(), 0)
        self.assertEqual(rl.bandCount(), 1)
        self.assertEqual(rl.bandName(1), "Band 1")
        self.assertIsNone(rl.attributeTable(1))
        self.assertFalse(rl.canCreateRasterAttributeTable())
        self.assertEqual(rl.providerType(), "arcgismapserver")
        self.assertEqual(rl.rasterUnitsPerPixelX(), 1)
        self.assertEqual(rl.rasterUnitsPerPixelY(), 1)
        self.assertFalse(rl.subsetString())
        self.assertEqual(rl.subLayers(), [])
        self.assertEqual(rl.crs(), QgsCoordinateReferenceSystem("EPSG:5514"))
        self.assertAlmostEqual(rl.extent().xMinimum(), -33699630.64526356756687164, 2)
        self.assertAlmostEqual(rl.extent().yMinimum(), -33699674.00237041711807251, 2)
        self.assertAlmostEqual(rl.extent().xMaximum(), 33699630.64526356756687164, 2)
        self.assertAlmostEqual(rl.extent().yMaximum(), 33632673.88108269870281219, 2)

        self.assertEqual(rl.dataProvider().dataType(1), Qgis.DataType.ARGB32)
        self.assertEqual(rl.dataProvider().sourceDataType(1), Qgis.DataType.ARGB32)
        self.assertEqual(rl.dataProvider().bandScale(1), 1)
        self.assertEqual(rl.dataProvider().bandOffset(1), 0)

    def test_non_consecutive_layer_ids(self):
        """
        Test non-consecutive layer IDs
        """
        endpoint = self.basetestpath + "/basic_test_fake_qgis_http_endpoint"
        with open(sanitize(endpoint, "?f=json"), "wb") as f:
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
   "id": 1,
   "name": "Layer 1",
   "parentLayerId": -1,
   "defaultVisibility": true,
   "subLayerIds": [
    0
   ],
   "minScale": 0,
   "maxScale": 0,
   "type": "Group Layer",
   "supportsDynamicLegends": false
  },
  {
   "id": 0,
   "name": "Layer 0",
   "parentLayerId": 1,
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
            "arcgismapserver",
        )
        self.assertTrue(rl.isValid())
        # basic raster layer properties
        self.assertEqual(rl.width(), 0)
        self.assertEqual(rl.height(), 0)
        self.assertEqual(rl.bandCount(), 1)
        self.assertEqual(rl.bandName(1), "Band 1")
        self.assertIsNone(rl.attributeTable(1))
        self.assertFalse(rl.canCreateRasterAttributeTable())
        self.assertEqual(rl.providerType(), "arcgismapserver")
        self.assertEqual(rl.rasterUnitsPerPixelX(), 1)
        self.assertEqual(rl.rasterUnitsPerPixelY(), 1)
        self.assertFalse(rl.subsetString())
        self.assertEqual(rl.subLayers(), [])
        self.assertEqual(rl.crs(), QgsCoordinateReferenceSystem("EPSG:5514"))
        self.assertAlmostEqual(rl.extent().xMinimum(), -33699630.64526356756687164, 2)
        self.assertAlmostEqual(rl.extent().yMinimum(), -33699674.00237041711807251, 2)
        self.assertAlmostEqual(rl.extent().xMaximum(), 33699630.64526356756687164, 2)
        self.assertAlmostEqual(rl.extent().yMaximum(), 33632673.88108269870281219, 2)

        self.assertEqual(rl.dataProvider().dataType(1), Qgis.DataType.ARGB32)
        self.assertEqual(rl.dataProvider().sourceDataType(1), Qgis.DataType.ARGB32)
        self.assertEqual(rl.dataProvider().bandScale(1), 1)
        self.assertEqual(rl.dataProvider().bandOffset(1), 0)


if __name__ == "__main__":
    unittest.main()
