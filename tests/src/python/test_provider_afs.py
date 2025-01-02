"""QGIS Unit tests for the AFS provider.

From build dir, run: ctest -R PyQgsAFSProvider -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "2018-02-16"
__copyright__ = "Copyright 2018, Nyall Dawson"

import hashlib
import tempfile

from qgis.PyQt.QtCore import (
    QCoreApplication,
    QDate,
    QDateTime,
    QObject,
    Qt,
    QTime,
)
from qgis.core import (
    Qgis,
    NULL,
    QgsApplication,
    QgsBox3d,
    QgsCategorizedSymbolRenderer,
    QgsCoordinateReferenceSystem,
    QgsFeature,
    QgsFieldConstraints,
    QgsGeometry,
    QgsLayerMetadata,
    QgsProviderRegistry,
    QgsRectangle,
    QgsSettings,
    QgsVectorDataProvider,
    QgsVectorDataProviderTemporalCapabilities,
    QgsVectorLayer,
    QgsWkbTypes,
    QgsGraduatedSymbolRenderer,
    QgsSymbol,
    QgsRendererRange,
    QgsSingleSymbolRenderer,
    QgsFillSymbol,
    QgsSymbolLayer,
    QgsColorRampTransformer,
    QgsGradientColorRamp,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from providertestbase import ProviderTestCase


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


class TestPyQgsAFSProvider(QgisTestCase, ProviderTestCase):

    def treat_date_as_datetime(self):
        return True

    def treat_time_as_string(self):
        return True

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()

        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("TestPyQgsAFSProvider.com")
        QCoreApplication.setApplicationName("TestPyQgsAFSProvider")
        QgsSettings().clear()
        start_app()

        # On Windows we must make sure that any backslash in the path is
        # replaced by a forward slash so that QUrl can process it
        cls.basetestpath = tempfile.mkdtemp().replace("\\", "/")
        endpoint = cls.basetestpath + "/fake_qgis_http_endpoint"
        with open(sanitize(endpoint, "?f=json"), "wb") as f:
            f.write(
                b"""
{"currentVersion":10.22,"id":1,"name":"QGIS Test","type":"Feature Layer","description":
"QGIS Provider Test Layer.\n","geometryType":"esriGeometryPoint","copyrightText":"","parentLayer":{"id":0,"name":"QGIS Tests"},"subLayers":[],
"minScale":72225,"maxScale":0,
"defaultVisibility":true,
"extent":{"xmin":-71.123,"ymin":66.33,"xmax":-65.32,"ymax":78.3,
"spatialReference":{"wkid":4326,"latestWkid":4326}},
"hasAttachments":false,"htmlPopupType":"esriServerHTMLPopupTypeAsHTMLText",
"displayField":"LABEL","typeIdField":null,
"fields":[{"name":"OBJECTID","type":"esriFieldTypeOID","alias":"OBJECTID","domain":null},
{"name":"pk","type":"esriFieldTypeInteger","alias":"pk","domain":null},
{"name":"cnt","type":"esriFieldTypeInteger","alias":"cnt","domain":null},
{"name":"name","type":"esriFieldTypeString","alias":"name","length":100,"domain":null},
{"name":"name2","type":"esriFieldTypeString","alias":"name2","length":100,"domain":null},
{"name":"num_char","type":"esriFieldTypeString","alias":"num_char","length":100,"domain":null},
{"name":"dt","type":"esriFieldTypeDate","alias":"num_char","length":100,"domain":null},
{"name":"date","type":"esriFieldTypeDate","alias":"num_char","length":100,"domain":null},
{"name":"time","type":"esriFieldTypeString","alias":"num_char","length":100,"domain":null},
{"name":"Shape","type":"esriFieldTypeGeometry","alias":"Shape","domain":null}],
"relationships":[],"canModifyLayer":false,"canScaleSymbols":false,"hasLabels":false,
"capabilities":"Map,Query,Data","maxRecordCount":1000,"supportsStatistics":true,
"supportsAdvancedQueries":true,"supportedQueryFormats":"JSON, AMF",
"ownershipBasedAccessControlForFeatures":{"allowOthersToQuery":true},"useStandardizedQueries":true}"""
            )

        with open(
            sanitize(endpoint, "/query?f=json_where=1=1&returnIdsOnly=true"), "wb"
        ) as f:
            f.write(
                b"""
{
 "objectIdFieldName": "OBJECTID",
 "objectIds": [
  5,
  3,
  1,
  2,
  4
 ]
}
"""
            )

        with open(
            sanitize(
                endpoint,
                '/query?f=json_where="cnt" > 100 and "cnt" < 410&returnIdsOnly=true',
            ),
            "wb",
        ) as f:
            f.write(
                b"""
        {
         "objectIdFieldName": "OBJECTID",
         "objectIds": [
          3,
          2,
          4
         ]
        }
        """
            )

        with open(
            sanitize(
                endpoint,
                '/query?f=json_where="cnt" > 100 and "cnt" < 400&returnIdsOnly=true',
            ),
            "wb",
        ) as f:
            f.write(
                b"""
        {
         "objectIdFieldName": "OBJECTID",
         "objectIds": [
          3,
          2
         ]
        }
        """
            )

        with open(
            sanitize(
                endpoint, "/query?f=json_where=\"name\"='Apple'&returnIdsOnly=true"
            ),
            "wb",
        ) as f:
            f.write(
                b"""
        {
         "objectIdFieldName": "OBJECTID",
         "objectIds": [
          2
         ]
        }
        """
            )

        with open(
            sanitize(
                endpoint,
                "/query?f=json_where=\"name\"='AppleBearOrangePear'&returnIdsOnly=true",
            ),
            "wb",
        ) as f:
            f.write(
                b"""
        {
         "objectIdFieldName": "OBJECTID",
         "objectIds": [
         ]
        }
        """
            )

        with open(
            sanitize(
                endpoint,
                '/query?f=json&where="cnt" > 100 and "cnt" < 410&returnIdsOnly=true&geometry=-70.000000,70.000000,-60.000000,75.000000&geometryType=esriGeometryEnvelope&spatialRel=esriSpatialRelEnvelopeIntersects',
            ),
            "wb",
        ) as f:
            f.write(
                b"""
        {
         "objectIdFieldName": "OBJECTID",
         "objectIds": [
          2
         ]
        }
        """
            )

        with open(
            sanitize(
                endpoint,
                '/query?f=json&where="cnt" > 100 and "cnt" < 410&returnIdsOnly=true&geometry=-71.000000,65.000000,-60.000000,80.000000&geometryType=esriGeometryEnvelope&spatialRel=esriSpatialRelEnvelopeIntersects',
            ),
            "wb",
        ) as f:
            f.write(
                b"""
        {
         "objectIdFieldName": "OBJECTID",
         "objectIds": [
          2,
          4
         ]
        }
        """
            )

        # Create test layer
        cls.vl = QgsVectorLayer(
            "url='http://" + endpoint + "' crs='epsg:4326'",
            "test",
            "arcgisfeatureserver",
        )
        assert cls.vl.isValid()
        cls.source = cls.vl.dataProvider()

        with open(
            sanitize(
                endpoint,
                "/query?f=json&objectIds=5,3,1,2,4&inSR=4326&outSR=4326&returnGeometry=true&outFields=*&returnM=false&returnZ=false",
            ),
            "wb",
        ) as f:
            f.write(
                (
                    """
        {
         "displayFieldName": "name",
         "fieldAliases": {
          "name": "name"
         },
         "geometryType": "esriGeometryPoint",
         "spatialReference": {
          "wkid": 4326,
          "latestWkid": 4326
         },
         "fields":[{"name":"OBJECTID","type":"esriFieldTypeOID","alias":"OBJECTID","domain":null},
        {"name":"pk","type":"esriFieldTypeInteger","alias":"pk","domain":null},
        {"name":"cnt","type":"esriFieldTypeInteger","alias":"cnt","domain":null},
        {"name":"name","type":"esriFieldTypeString","alias":"name","length":100,"domain":null},
        {"name":"name2","type":"esriFieldTypeString","alias":"name2","length":100,"domain":null},
        {"name":"num_char","type":"esriFieldTypeString","alias":"num_char","length":100,"domain":null},
        {"name":"dt","type":"esriFieldTypeDate","alias":"num_char","length":100,"domain":null},
        {"name":"date","type":"esriFieldTypeDate","alias":"num_char","length":100,"domain":null},
        {"name":"time","type":"esriFieldTypeString","alias":"num_char","length":100,"domain":null},
        {"name":"Shape","type":"esriFieldTypeGeometry","alias":"Shape","domain":null}],
         "features": [
          {
           "attributes": {
            "OBJECTID": 5,
            "pk": 5,
            "cnt": -200,
            "name": null,
            "name2":"NuLl",
            "num_char":"5",
            "dt": """
                    + str(
                        QDateTime(
                            QDate(2020, 5, 4), QTime(12, 13, 14)
                        ).toMSecsSinceEpoch()
                    )
                    + """,
            "date": """
                    + str(
                        QDateTime(QDate(2020, 5, 2), QTime(0, 0, 0)).toMSecsSinceEpoch()
                    )
                    + """,
            "time": "12:13:01"
           },
           "geometry": {
            "x": -71.123,
            "y": 78.23
           }
          },
          {
           "attributes": {
            "OBJECTID": 3,
            "pk": 3,
            "cnt": 300,
            "name": "Pear",
            "name2":"PEaR",
            "num_char":"3",
            "dt": null,
            "date": null,
            "time": null
           },
           "geometry": null
          },
          {
           "attributes": {
            "OBJECTID": 1,
            "pk": 1,
            "cnt": 100,
            "name": "Orange",
            "name2":"oranGe",
            "num_char":"1",
            "dt": """
                    + str(
                        QDateTime(
                            QDate(2020, 5, 3), QTime(12, 13, 14)
                        ).toMSecsSinceEpoch()
                    )
                    + """,
            "date": """
                    + str(
                        QDateTime(QDate(2020, 5, 3), QTime(0, 0, 0)).toMSecsSinceEpoch()
                    )
                    + """,
            "time": "12:13:14"
           },
           "geometry": {
            "x": -70.332,
            "y": 66.33
           }
          },
          {
           "attributes": {
            "OBJECTID": 2,
            "pk": 2,
            "cnt": 200,
            "name": "Apple",
            "name2":"Apple",
            "num_char":"2",
            "dt": """
                    + str(
                        QDateTime(
                            QDate(2020, 5, 4), QTime(12, 14, 14)
                        ).toMSecsSinceEpoch()
                    )
                    + """,
            "date": """
                    + str(
                        QDateTime(QDate(2020, 5, 4), QTime(0, 0, 0)).toMSecsSinceEpoch()
                    )
                    + """,
            "time": "12:14:14"
           },
           "geometry": {
            "x": -68.2,
            "y": 70.8
           }
          },
          {
           "attributes": {
            "OBJECTID": 4,
            "pk": 4,
            "cnt": 400,
            "name": "Honey",
            "name2":"Honey",
            "num_char":"4",
            "dt": """
                    + str(
                        QDateTime(
                            QDate(2021, 5, 4), QTime(13, 13, 14)
                        ).toMSecsSinceEpoch()
                    )
                    + """,
            "date": """
                    + str(
                        QDateTime(QDate(2021, 5, 4), QTime(0, 0, 0)).toMSecsSinceEpoch()
                    )
                    + """,
            "time": "13:13:14"
           },
           "geometry": {
            "x": -65.32,
            "y": 78.3
           }
          }
         ]
        }"""
                ).encode("UTF-8")
            )

        with open(
            sanitize(
                endpoint,
                "/query?f=json&objectIds=3,2,4&inSR=4326&outSR=4326&returnGeometry=true&outFields=*&returnM=false&returnZ=false",
            ),
            "wb",
        ) as f:
            f.write(
                (
                    """
        {
         "displayFieldName": "name",
         "fieldAliases": {
          "name": "name"
         },
         "geometryType": "esriGeometryPoint",
         "spatialReference": {
          "wkid": 4326,
          "latestWkid": 4326
         },
         "fields":[{"name":"OBJECTID","type":"esriFieldTypeOID","alias":"OBJECTID","domain":null},
        {"name":"pk","type":"esriFieldTypeInteger","alias":"pk","domain":null},
        {"name":"cnt","type":"esriFieldTypeInteger","alias":"cnt","domain":null},
        {"name":"name","type":"esriFieldTypeString","alias":"name","length":100,"domain":null},
        {"name":"name2","type":"esriFieldTypeString","alias":"name2","length":100,"domain":null},
        {"name":"num_char","type":"esriFieldTypeString","alias":"num_char","length":100,"domain":null},
        {"name":"dt","type":"esriFieldTypeDate","alias":"num_char","length":100,"domain":null},
        {"name":"date","type":"esriFieldTypeDate","alias":"num_char","length":100,"domain":null},
        {"name":"time","type":"esriFieldTypeString","alias":"num_char","length":100,"domain":null},
        {"name":"Shape","type":"esriFieldTypeGeometry","alias":"Shape","domain":null}],
         "features": [
          {
           "attributes": {
            "OBJECTID": 3,
            "pk": 3,
            "cnt": 300,
            "name": "Pear",
            "name2":"PEaR",
            "num_char":"3",
            "dt": null,
            "date": null,
            "time": null
           },
           "geometry": null
          },
          {
           "attributes": {
            "OBJECTID": 2,
            "pk": 2,
            "cnt": 200,
            "name": "Apple",
            "name2":"Apple",
            "num_char":"2",
            "dt": """
                    + str(
                        QDateTime(
                            QDate(2020, 5, 4), QTime(12, 14, 14)
                        ).toMSecsSinceEpoch()
                    )
                    + """,
            "date": """
                    + str(
                        QDateTime(QDate(2020, 5, 4), QTime(0, 0, 0)).toMSecsSinceEpoch()
                    )
                    + """,
            "time": "12:14:14"
           },
           "geometry": {
            "x": -68.2,
            "y": 70.8
           }
          },
          {
           "attributes": {
            "OBJECTID": 4,
            "pk": 4,
            "cnt": 400,
            "name": "Honey",
            "name2":"Honey",
            "num_char":"4",
            "dt": """
                    + str(
                        QDateTime(
                            QDate(2021, 5, 4), QTime(13, 13, 14)
                        ).toMSecsSinceEpoch()
                    )
                    + """,
            "date": """
                    + str(
                        QDateTime(QDate(2021, 5, 4), QTime(0, 0, 0)).toMSecsSinceEpoch()
                    )
                    + """,
            "time": "13:13:14"
           },
           "geometry": {
            "x": -65.32,
            "y": 78.3
           }
          }
         ]
        }"""
                ).encode("UTF-8")
            )

        with open(
            sanitize(
                endpoint,
                "/query?f=json&objectIds=3,2&inSR=4326&outSR=4326&returnGeometry=true&outFields=*&returnM=false&returnZ=false",
            ),
            "wb",
        ) as f:
            f.write(
                (
                    """
        {
         "displayFieldName": "name",
         "fieldAliases": {
          "name": "name"
         },
         "geometryType": "esriGeometryPoint",
         "spatialReference": {
          "wkid": 4326,
          "latestWkid": 4326
         },
         "fields":[{"name":"OBJECTID","type":"esriFieldTypeOID","alias":"OBJECTID","domain":null},
        {"name":"pk","type":"esriFieldTypeInteger","alias":"pk","domain":null},
        {"name":"cnt","type":"esriFieldTypeInteger","alias":"cnt","domain":null},
        {"name":"name","type":"esriFieldTypeString","alias":"name","length":100,"domain":null},
        {"name":"name2","type":"esriFieldTypeString","alias":"name2","length":100,"domain":null},
        {"name":"num_char","type":"esriFieldTypeString","alias":"num_char","length":100,"domain":null},
        {"name":"dt","type":"esriFieldTypeDate","alias":"num_char","length":100,"domain":null},
        {"name":"date","type":"esriFieldTypeDate","alias":"num_char","length":100,"domain":null},
        {"name":"time","type":"esriFieldTypeString","alias":"num_char","length":100,"domain":null},
        {"name":"Shape","type":"esriFieldTypeGeometry","alias":"Shape","domain":null}],
         "features": [
          {
           "attributes": {
            "OBJECTID": 3,
            "pk": 3,
            "cnt": 300,
            "name": "Pear",
            "name2":"PEaR",
            "num_char":"3",
            "dt": null,
            "date": null,
            "time": null
           },
           "geometry": null
          },
          {
           "attributes": {
            "OBJECTID": 2,
            "pk": 2,
            "cnt": 200,
            "name": "Apple",
            "name2":"Apple",
            "num_char":"2",
            "dt": """
                    + str(
                        QDateTime(
                            QDate(2020, 5, 4), QTime(12, 14, 14)
                        ).toMSecsSinceEpoch()
                    )
                    + """,
            "date": """
                    + str(
                        QDateTime(QDate(2020, 5, 4), QTime(0, 0, 0)).toMSecsSinceEpoch()
                    )
                    + """,
            "time": "12:14:14"
           },
           "geometry": {
            "x": -68.2,
            "y": 70.8
           }
          },
          {
           "attributes": {
            "OBJECTID": 4,
            "pk": 4,
            "cnt": 400,
            "name": "Honey",
            "name2":"Honey",
            "num_char":"4",
            "dt": """
                    + str(
                        QDateTime(
                            QDate(2021, 5, 4), QTime(13, 13, 14)
                        ).toMSecsSinceEpoch()
                    )
                    + """,
            "date": """
                    + str(
                        QDateTime(QDate(2021, 5, 4), QTime(0, 0, 0)).toMSecsSinceEpoch()
                    )
                    + """,
            "time": "13:13:14"
           },
           "geometry": {
            "x": -65.32,
            "y": 78.3
           }
          }
         ]
        }"""
                ).encode("UTF-8")
            )

        with open(
            sanitize(
                endpoint,
                "/query?f=json&objectIds=5,3,1,2,4&inSR=4326&outSR=4326&returnGeometry=true&outFields=*&returnM=false&returnZ=false&geometry=-71.123000,66.330000,-65.320000,78.300000&geometryType=esriGeometryEnvelope&spatialRel=esriSpatialRelEnvelopeIntersects",
            ),
            "wb",
        ) as f:
            f.write(
                b"""
{
 "displayFieldName": "name",
 "fieldAliases": {
  "name": "name"
 },
 "geometryType": "esriGeometryPoint",
 "spatialReference": {
  "wkid": 4326,
  "latestWkid": 4326
 },
 "fields":[{"name":"OBJECTID","type":"esriFieldTypeOID","alias":"OBJECTID","domain":null},
{"name":"pk","type":"esriFieldTypeInteger","alias":"pk","domain":null},
{"name":"cnt","type":"esriFieldTypeInteger","alias":"cnt","domain":null},
{"name":"name","type":"esriFieldTypeString","alias":"name","length":100,"domain":null},
{"name":"name2","type":"esriFieldTypeString","alias":"name2","length":100,"domain":null},
{"name":"num_char","type":"esriFieldTypeString","alias":"num_char","length":100,"domain":null},
{"name":"dt","type":"esriFieldTypeDate","alias":"num_char","length":100,"domain":null},
{"name":"date","type":"esriFieldTypeDate","alias":"num_char","length":100,"domain":null},
{"name":"time","type":"esriFieldTypeString","alias":"num_char","length":100,"domain":null},
{"name":"Shape","type":"esriFieldTypeGeometry","alias":"Shape","domain":null}],
 "features": [
  {
   "attributes": {
    "OBJECTID": 5,
    "pk": 5,
    "cnt": -200,
    "name": null,
    "name2":"NuLl",
    "num_char":"5"
   },
   "geometry": {
    "x": -71.123,
    "y": 78.23
   }
  },
  {
   "attributes": {
    "OBJECTID": 3,
    "pk": 3,
    "cnt": 300,
    "name": "Pear",
    "name2":"PEaR",
    "num_char":"3"
   },
   "geometry": null
  },
  {
   "attributes": {
    "OBJECTID": 1,
    "pk": 1,
    "cnt": 100,
    "name": "Orange",
    "name2":"oranGe",
    "num_char":"1"
   },
   "geometry": {
    "x": -70.332,
    "y": 66.33
   }
  },
  {
   "attributes": {
    "OBJECTID": 2,
    "pk": 2,
    "cnt": 200,
    "name": "Apple",
    "name2":"Apple",
    "num_char":"2"
   },
   "geometry": {
    "x": -68.2,
    "y": 70.8
   }
  },
  {
   "attributes": {
    "OBJECTID": 4,
    "pk": 4,
    "cnt": 400,
    "name": "Honey",
    "name2":"Honey",
    "num_char":"4"
   },
   "geometry": {
    "x": -65.32,
    "y": 78.3
   }
  }
 ]
}"""
            )

        with open(
            sanitize(
                endpoint,
                "/query?f=json&objectIds=2,4&inSR=4326&outSR=4326&returnGeometry=true&outFields=*&returnM=false&returnZ=false",
            ),
            "wb",
        ) as f:
            f.write(
                b"""
        {
         "displayFieldName": "name",
         "fieldAliases": {
          "name": "name"
         },
         "geometryType": "esriGeometryPoint",
         "spatialReference": {
          "wkid": 4326,
          "latestWkid": 4326
         },
         "fields":[{"name":"OBJECTID","type":"esriFieldTypeOID","alias":"OBJECTID","domain":null},
        {"name":"pk","type":"esriFieldTypeInteger","alias":"pk","domain":null},
        {"name":"cnt","type":"esriFieldTypeInteger","alias":"cnt","domain":null},
        {"name":"name","type":"esriFieldTypeString","alias":"name","length":100,"domain":null},
        {"name":"name2","type":"esriFieldTypeString","alias":"name2","length":100,"domain":null},
        {"name":"num_char","type":"esriFieldTypeString","alias":"num_char","length":100,"domain":null},
        {"name":"dt","type":"esriFieldTypeDate","alias":"num_char","length":100,"domain":null},
        {"name":"date","type":"esriFieldTypeDate","alias":"num_char","length":100,"domain":null},
        {"name":"time","type":"esriFieldTypeDate","alias":"num_char","length":100,"domain":null},
        {"name":"Shape","type":"esriFieldTypeGeometry","alias":"Shape","domain":null}],
         "features": [
          {
           "attributes": {
            "OBJECTID": 2,
            "pk": 2,
            "cnt": 200,
            "name": "Apple",
            "name2":"Apple",
            "num_char":"2"
           },
           "geometry": {
            "x": -68.2,
            "y": 70.8
           }
          },
          {
           "attributes": {
            "OBJECTID": 4,
            "pk": 4,
            "cnt": 400,
            "name": "Honey",
            "name2":"Honey",
            "num_char":"4"
           },
           "geometry": {
            "x": -65.32,
            "y": 78.3
           }
          }
         ]
        }"""
            )

        with open(
            sanitize(
                endpoint,
                "/query?f=json&where=1=1&returnIdsOnly=true&geometry=-70.000000,67.000000,-60.000000,80.000000&geometryType=esriGeometryEnvelope&spatialRel=esriSpatialRelEnvelopeIntersects",
            ),
            "wb",
        ) as f:
            f.write(
                b"""
        {
         "objectIdFieldName": "OBJECTID",
         "objectIds": [
          2,
          4
         ]
        }
        """
            )

        with open(
            sanitize(
                endpoint,
                "/query?f=json&where==1=&returnIdsOnly=true&geometry=-73.000000,70.000000,-63.000000,80.000000&geometryType=esriGeometryEnvelope&spatialRel=esriSpatialRelEnvelopeIntersects",
            ),
            "wb",
        ) as f:
            f.write(
                b"""
        {
         "objectIdFieldName": "OBJECTID",
         "objectIds": [
          2,
          4
         ]
        }
        """
            )

        with open(
            sanitize(
                endpoint,
                "/query?f=json&where=1=1&returnIdsOnly=true&geometry=-68.721119,68.177676,-64.678700,79.123755&geometryType=esriGeometryEnvelope&spatialRel=esriSpatialRelEnvelopeIntersects",
            ),
            "wb",
        ) as f:
            f.write(
                b"""
        {
         "objectIdFieldName": "OBJECTID",
         "objectIds": [
          2,
          4
         ]
        }
        """
            )

        with open(
            sanitize(
                endpoint, "/query?f=json&where=\"name\"='Apple'&returnExtentOnly=true"
            ),
            "wb",
        ) as f:
            f.write(
                b"""
        {
         "extent": {
          "xmin": -68.2,
          "xmax": -68.2,
          "ymin":70.8,
          "ymax":70.8
         }
        }
        """
            )

        with open(
            sanitize(
                endpoint,
                "/query?f=json&where=\"name\"='AppleBearOrangePear'&returnExtentOnly=true",
            ),
            "wb",
        ) as f:
            f.write(
                b"""
        {
         "extent": {
         }
        }
        """
            )

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        QgsSettings().clear()
        # shutil.rmtree(cls.basetestpath, True)
        cls.vl = (
            None  # so as to properly close the provider and remove any temporary file
        )
        super().tearDownClass()

    def testGetFeaturesSubsetAttributes2(self):
        """Override and skip this test for AFS provider, as it's actually more efficient for the AFS provider to return
        its features as direct copies (due to implicit sharing of QgsFeature), and the nature of the caching
        used by the AFS provider.
        """
        pass

    def testGetFeaturesNoGeometry(self):
        """Override and skip this test for AFS provider, as it's actually more efficient for the AFS provider to return
        its features as direct copies (due to implicit sharing of QgsFeature), and the nature of the caching
        used by the AFS provider.
        """
        pass

    def providerCompatibleOfSubsetStringWithStableFID(self):
        return False

    def testDecodeUri(self):
        """
        Test decoding an AFS uri
        """
        uri = self.vl.source()
        parts = QgsProviderRegistry.instance().decodeUri(
            self.vl.dataProvider().name(), uri
        )
        self.assertEqual(
            parts,
            {
                "crs": "epsg:4326",
                "url": "http://" + self.basetestpath + "/fake_qgis_http_endpoint",
            },
        )

    def testEncodeUri(self):
        """
        Test encoding an AFS uri
        """
        parts = {
            "url": "http://blah.com",
            "crs": "epsg:4326",
            "referer": "me",
            "bounds": QgsRectangle(1, 2, 3, 4),
        }
        uri = QgsProviderRegistry.instance().encodeUri(
            self.vl.dataProvider().name(), parts
        )
        self.assertEqual(
            uri,
            " bbox='1,2,3,4' crs='epsg:4326' url='http://blah.com' http-header:referer='me' referer='me'",
        )

    def testProviderCapabilities(self):
        # non-editable layer
        self.assertEqual(
            self.vl.dataProvider().capabilities(),
            QgsVectorDataProvider.Capabilities(
                QgsVectorDataProvider.Capability.SelectAtId
                | QgsVectorDataProvider.Capability.ReadLayerMetadata
                | QgsVectorDataProvider.Capability.ReloadData
            ),
        )

        # delete capability
        endpoint = self.basetestpath + "/delete_fake_qgis_http_endpoint"
        with open(sanitize(endpoint, "?f=json"), "wb") as f:
            f.write(
                b"""
                {"currentVersion":10.22,"id":1,"name":"QGIS Test","type":"Feature Layer","description":
                "QGIS Provider Test Layer","geometryType":"esriGeometryPoint","copyrightText":"not copyright","parentLayer":{"id":2,"name":"QGIS Tests"},"subLayers":[],
                "minScale":72225,"maxScale":0,
                "defaultVisibility":true,
                "extent":{"xmin":-71.123,"ymin":66.33,"xmax":-65.32,"ymax":78.3,
                "spatialReference":{"wkid":4326,"latestWkid":4326}},
                "hasAttachments":false,"htmlPopupType":"esriServerHTMLPopupTypeAsHTMLText",
                "displayField":"LABEL","typeIdField":null,
                "fields":[{"name":"OBJECTID","type":"esriFieldTypeOID","alias":"OBJECTID","domain":null}],
                "relationships":[],"canModifyLayer":false,"canScaleSymbols":false,"hasLabels":false,
                "capabilities":"Map,Query,Data,Delete","maxRecordCount":1000,"supportsStatistics":true,
                "supportsAdvancedQueries":true,"supportedQueryFormats":"JSON, AMF",
                "ownershipBasedAccessControlForFeatures":{"allowOthersToQuery":true},"useStandardizedQueries":true}"""
            )

        with open(
            sanitize(endpoint, "/query?f=json_where=1=1&returnIdsOnly=true"), "wb"
        ) as f:
            f.write(
                b"""
                {
                 "objectIdFieldName": "OBJECTID",
                 "objectIds": [
                  1
                 ]
                }
                """
            )

        # Create test layer
        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' crs='epsg:4326'",
            "test",
            "arcgisfeatureserver",
        )
        self.assertTrue(vl.isValid())
        self.assertEqual(
            vl.dataProvider().capabilities(),
            QgsVectorDataProvider.Capabilities(
                QgsVectorDataProvider.Capability.SelectAtId
                | QgsVectorDataProvider.Capability.ReadLayerMetadata
                | QgsVectorDataProvider.Capability.ReloadData
                | QgsVectorDataProvider.Capability.DeleteFeatures
            ),
        )

        # add capability
        endpoint = self.basetestpath + "/delete_fake_qgis_http_endpoint"
        with open(sanitize(endpoint, "?f=json"), "wb") as f:
            f.write(
                b"""
                {"currentVersion":10.22,"id":1,"name":"QGIS Test","type":"Feature Layer","description":
                "QGIS Provider Test Layer","geometryType":"esriGeometryPoint","copyrightText":"not copyright","parentLayer":{"id":2,"name":"QGIS Tests"},"subLayers":[],
                "minScale":72225,"maxScale":0,
                "defaultVisibility":true,
                "extent":{"xmin":-71.123,"ymin":66.33,"xmax":-65.32,"ymax":78.3,
                "spatialReference":{"wkid":4326,"latestWkid":4326}},
                "hasAttachments":false,"htmlPopupType":"esriServerHTMLPopupTypeAsHTMLText",
                "displayField":"LABEL","typeIdField":null,
                "fields":[{"name":"OBJECTID","type":"esriFieldTypeOID","alias":"OBJECTID","domain":null}],
                "relationships":[],"canModifyLayer":false,"canScaleSymbols":false,"hasLabels":false,
                "capabilities":"Map,Query,Data,Create","maxRecordCount":1000,"supportsStatistics":true,
                "supportsAdvancedQueries":true,"supportedQueryFormats":"JSON, AMF",
                "ownershipBasedAccessControlForFeatures":{"allowOthersToQuery":true},"useStandardizedQueries":true}"""
            )

        # Create test layer
        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' crs='epsg:4326'",
            "test",
            "arcgisfeatureserver",
        )
        self.assertTrue(vl.isValid())
        self.assertEqual(
            vl.dataProvider().capabilities(),
            QgsVectorDataProvider.Capabilities(
                QgsVectorDataProvider.Capability.SelectAtId
                | QgsVectorDataProvider.Capability.ReadLayerMetadata
                | QgsVectorDataProvider.Capability.ReloadData
                | QgsVectorDataProvider.Capability.AddFeatures
            ),
        )
        # update capability
        endpoint = self.basetestpath + "/delete_fake_qgis_http_endpoint"
        with open(sanitize(endpoint, "?f=json"), "wb") as f:
            f.write(
                b"""
                    {"currentVersion":10.22,"id":1,"name":"QGIS Test","type":"Feature Layer","description":
                    "QGIS Provider Test Layer","geometryType":"esriGeometryPoint","copyrightText":"not copyright","parentLayer":{"id":2,"name":"QGIS Tests"},"subLayers":[],
                    "minScale":72225,"maxScale":0,
                    "defaultVisibility":true,
                    "extent":{"xmin":-71.123,"ymin":66.33,"xmax":-65.32,"ymax":78.3,
                    "spatialReference":{"wkid":4326,"latestWkid":4326}},
                    "hasAttachments":false,"htmlPopupType":"esriServerHTMLPopupTypeAsHTMLText",
                    "displayField":"LABEL","typeIdField":null,
                    "fields":[{"name":"OBJECTID","type":"esriFieldTypeOID","alias":"OBJECTID","domain":null}],
                    "relationships":[],"canModifyLayer":false,"canScaleSymbols":false,"hasLabels":false,
                    "capabilities":"Map,Query,Data,Update","maxRecordCount":1000,"supportsStatistics":true,
                    "supportsAdvancedQueries":true,"supportedQueryFormats":"JSON, AMF",
                    "ownershipBasedAccessControlForFeatures":{"allowOthersToQuery":true},"useStandardizedQueries":true}"""
            )

        # Create test layer
        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' crs='epsg:4326'",
            "test",
            "arcgisfeatureserver",
        )
        self.assertTrue(vl.isValid())
        self.assertEqual(
            vl.dataProvider().capabilities(),
            QgsVectorDataProvider.Capabilities(
                QgsVectorDataProvider.Capability.SelectAtId
                | QgsVectorDataProvider.Capability.ReadLayerMetadata
                | QgsVectorDataProvider.Capability.ReloadData
                | QgsVectorDataProvider.Capability.ChangeAttributeValues
                | QgsVectorDataProvider.Capability.ChangeFeatures
                | QgsVectorDataProvider.Capability.ChangeGeometries
            ),
        )

        # circular strings
        with open(sanitize(endpoint, "?f=json"), "wb") as f:
            f.write(
                b"""
                    {"currentVersion":10.22,"id":1,"name":"QGIS Test","allowTrueCurvesUpdates":true,"type":"Feature Layer","description":
                    "QGIS Provider Test Layer","geometryType":"esriGeometryPoint","copyrightText":"not copyright","parentLayer":{"id":2,"name":"QGIS Tests"},"subLayers":[],
                    "minScale":72225,"maxScale":0,
                    "defaultVisibility":true,
                    "extent":{"xmin":-71.123,"ymin":66.33,"xmax":-65.32,"ymax":78.3,
                    "spatialReference":{"wkid":4326,"latestWkid":4326}},
                    "hasAttachments":false,"htmlPopupType":"esriServerHTMLPopupTypeAsHTMLText",
                    "displayField":"LABEL","typeIdField":null,
                    "fields":[{"name":"OBJECTID","type":"esriFieldTypeOID","alias":"OBJECTID","domain":null}],
                    "relationships":[],"canModifyLayer":false,"canScaleSymbols":false,"hasLabels":false,
                    "capabilities":"Map,Query,Data,Update","maxRecordCount":1000,"supportsStatistics":true,
                    "supportsAdvancedQueries":true,"supportedQueryFormats":"JSON, AMF",
                    "ownershipBasedAccessControlForFeatures":{"allowOthersToQuery":true},"useStandardizedQueries":true}"""
            )
        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' crs='epsg:4326'",
            "test",
            "arcgisfeatureserver",
        )
        self.assertTrue(vl.isValid())
        self.assertEqual(
            vl.dataProvider().capabilities(),
            QgsVectorDataProvider.Capabilities(
                QgsVectorDataProvider.Capability.SelectAtId
                | QgsVectorDataProvider.Capability.ReadLayerMetadata
                | QgsVectorDataProvider.Capability.ReloadData
                | QgsVectorDataProvider.Capability.ChangeAttributeValues
                | QgsVectorDataProvider.Capability.ChangeFeatures
                | QgsVectorDataProvider.Capability.CircularGeometries
                | QgsVectorDataProvider.Capability.ChangeGeometries
            ),
        )

    def testFieldProperties(self):
        self.assertEqual(self.vl.dataProvider().pkAttributeIndexes(), [0])
        self.assertEqual(
            self.vl.dataProvider().fields()[0].constraints().constraints(),
            QgsFieldConstraints.Constraints(
                QgsFieldConstraints.Constraint.ConstraintNotNull
                | QgsFieldConstraints.Constraint.ConstraintUnique
            ),
        )
        self.assertFalse(self.vl.dataProvider().fields()[1].constraints().constraints())
        self.assertEqual(self.vl.dataProvider().defaultValueClause(0), "Autogenerate")
        self.assertFalse(self.vl.dataProvider().defaultValueClause(1))

        self.assertTrue(
            self.vl.dataProvider().skipConstraintCheck(
                0, QgsFieldConstraints.Constraint.ConstraintUnique, "Autogenerate"
            )
        )
        self.assertFalse(
            self.vl.dataProvider().skipConstraintCheck(
                0, QgsFieldConstraints.Constraint.ConstraintUnique, "aa"
            )
        )
        self.assertFalse(
            self.vl.dataProvider().skipConstraintCheck(
                1, QgsFieldConstraints.Constraint.ConstraintUnique, "aa"
            )
        )

    def testObjectIdDifferentName(self):
        """Test that object id fields not named OBJECTID work correctly"""

        endpoint = self.basetestpath + "/oid_fake_qgis_http_endpoint"
        with open(sanitize(endpoint, "?f=json"), "wb") as f:
            f.write(
                b"""
        {"currentVersion":10.22,"id":1,"name":"QGIS Test","type":"Feature Layer","description":
        "QGIS Provider Test Layer.\n","geometryType":"esriGeometryPoint","copyrightText":"","parentLayer":{"id":0,"name":"QGIS Tests"},"subLayers":[],
        "minScale":72225,"maxScale":0,
        "defaultVisibility":true,
        "extent":{"xmin":-71.123,"ymin":66.33,"xmax":-65.32,"ymax":78.3,
        "spatialReference":{"wkid":4326,"latestWkid":4326}},
        "hasAttachments":false,"htmlPopupType":"esriServerHTMLPopupTypeAsHTMLText",
        "displayField":"LABEL","typeIdField":null,
        "fields":[{"name":"OBJECTID1","type":"esriFieldTypeOID","alias":"OBJECTID","domain":null},
        {"name":"pk","type":"esriFieldTypeInteger","alias":"pk","domain":null},
        {"name":"cnt","type":"esriFieldTypeInteger","alias":"cnt","domain":null}],
        "relationships":[],"canModifyLayer":false,"canScaleSymbols":false,"hasLabels":false,
        "capabilities":"Map,Query,Data","maxRecordCount":1000,"supportsStatistics":true,
        "supportsAdvancedQueries":true,"supportedQueryFormats":"JSON, AMF",
        "ownershipBasedAccessControlForFeatures":{"allowOthersToQuery":true},"useStandardizedQueries":true}"""
            )

        with open(
            sanitize(endpoint, "/query?f=json_where=1=1&returnIdsOnly=true"), "wb"
        ) as f:
            f.write(
                b"""
        {
         "objectIdFieldName": "OBJECTID1",
         "objectIds": [
          5,
          3,
          1,
          2,
          4
         ]
        }
        """
            )

        with open(
            sanitize(
                endpoint,
                "/query?f=json&objectIds=5,3,1,2,4&inSR=4326&outSR=4326&returnGeometry=true&outFields=*&returnM=false&returnZ=false",
            ),
            "wb",
        ) as f:
            f.write(
                b"""
        {
         "displayFieldName": "LABEL",
         "geometryType": "esriGeometryPoint",
         "spatialReference": {
          "wkid": 4326,
          "latestWkid": 4326
         },
         "fields":[{"name":"OBJECTID1","type":"esriFieldTypeOID","alias":"OBJECTID1","domain":null},
          {"name":"pk","type":"esriFieldTypeInteger","alias":"pk","domain":null},
          {"name":"cnt","type":"esriFieldTypeInteger","alias":"cnt","domain":null},
          {"name":"Shape","type":"esriFieldTypeGeometry","alias":"Shape","domain":null}],
         "features": [
          {
           "attributes": {
            "OBJECTID1": 5,
            "pk": 5,
            "cnt": -200,
            "name": null
           },
           "geometry": {
            "x": -71.123,
            "y": 78.23
           }
          }
         ]
        }"""
            )

        # Create test layer
        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' crs='epsg:4326'",
            "test",
            "arcgisfeatureserver",
        )
        self.assertTrue(vl.isValid())

        f = vl.getFeature(0)
        self.assertTrue(f.isValid())

    def testDateTime(self):
        """Test that datetime fields work correctly"""

        endpoint = self.basetestpath + "/oid_fake_qgis_http_endpoint"
        with open(sanitize(endpoint, "?f=json"), "wb") as f:
            f.write(
                b"""
        {"currentVersion":10.22,"id":1,"name":"QGIS Test","type":"Feature Layer","description":
        "QGIS Provider Test Layer.\n","geometryType":"esriGeometryPoint","copyrightText":"","parentLayer":{"id":0,"name":"QGIS Tests"},"subLayers":[],
        "minScale":72225,"maxScale":0,
        "defaultVisibility":true,
        "extent":{"xmin":-71.123,"ymin":66.33,"xmax":-65.32,"ymax":78.3,
        "spatialReference":{"wkid":4326,"latestWkid":4326}},
        "hasAttachments":false,"htmlPopupType":"esriServerHTMLPopupTypeAsHTMLText",
        "displayField":"LABEL","typeIdField":null,
        "fields":[{"name":"OBJECTID","type":"esriFieldTypeOID","alias":"OBJECTID","domain":null},
        {"name":"pk","type":"esriFieldTypeInteger","alias":"pk","domain":null},
        {"name":"dt","type":"esriFieldTypeDate","alias":"dt","length":8,"domain":null}],
        "relationships":[],"canModifyLayer":false,"canScaleSymbols":false,"hasLabels":false,
        "capabilities":"Map,Query,Data","maxRecordCount":1000,"supportsStatistics":true,
        "supportsAdvancedQueries":true,"supportedQueryFormats":"JSON, AMF",
        "ownershipBasedAccessControlForFeatures":{"allowOthersToQuery":true},"useStandardizedQueries":true}"""
            )

        with open(
            sanitize(endpoint, "/query?f=json_where=1=1&returnIdsOnly=true"), "wb"
        ) as f:
            f.write(
                b"""
        {
         "objectIdFieldName": "OBJECTID",
         "objectIds": [
          1,
          2
         ]
        }
        """
            )

        # Create test layer
        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' crs='epsg:4326'",
            "test",
            "arcgisfeatureserver",
        )

        self.assertTrue(vl.isValid())

        self.assertFalse(
            vl.dataProvider().temporalCapabilities().hasTemporalCapabilities()
        )

        with open(
            sanitize(
                endpoint,
                "/query?f=json&objectIds=1,2&inSR=4326&outSR=4326&returnGeometry=true&outFields=*&returnM=false&returnZ=false",
            ),
            "wb",
        ) as f:
            f.write(
                b"""
        {
         "displayFieldName": "name",
         "fieldAliases": {
          "name": "name"
         },
         "geometryType": "esriGeometryPoint",
         "spatialReference": {
          "wkid": 4326,
          "latestWkid": 4326
         },
         "fields":[{"name":"OBJECTID","type":"esriFieldTypeOID","alias":"OBJECTID","domain":null},
        {"name":"pk","type":"esriFieldTypeInteger","alias":"pk","domain":null},
        {"name":"dt","type":"esriFieldTypeDate","alias":"dt","domain":null},
        {"name":"Shape","type":"esriFieldTypeGeometry","alias":"Shape","domain":null}],
         "features": [
          {
           "attributes": {
            "OBJECTID": 1,
            "pk": 1,
            "dt":1493769600000
           },
           "geometry": {
            "x": -70.332,
            "y": 66.33
           }
          },
          {
           "attributes": {
            "OBJECTID": 2,
            "pk": 2,
            "dt":null
           },
           "geometry": {
            "x": -68.2,
            "y": 70.8
           }
          }
         ]
        }"""
            )

        features = [f for f in vl.getFeatures()]
        self.assertEqual(len(features), 2)
        self.assertEqual(
            [f["dt"] for f in features],
            [
                QDateTime(
                    QDate(2017, 5, 3), QTime(0, 0, 0, 0), Qt.TimeSpec.UTC
                ).toLocalTime(),
                NULL,
            ],
        )

    def testMetadata(self):
        """Test that metadata is correctly acquired from provider"""

        endpoint = self.basetestpath + "/metadata_fake_qgis_http_endpoint"
        with open(sanitize(endpoint, "?f=json"), "wb") as f:
            f.write(
                b"""
        {"currentVersion":10.22,"id":1,"name":"QGIS Test","type":"Feature Layer","description":
        "QGIS Provider Test Layer","geometryType":"esriGeometryPoint","copyrightText":"not copyright","parentLayer":{"id":2,"name":"QGIS Tests"},"subLayers":[],
        "minScale":72225,"maxScale":0,
        "defaultVisibility":true,
        "extent":{"xmin":-71.123,"ymin":66.33,"xmax":-65.32,"ymax":78.3,
        "spatialReference":{"wkid":4326,"latestWkid":4326}},
        "hasAttachments":false,"htmlPopupType":"esriServerHTMLPopupTypeAsHTMLText",
        "displayField":"LABEL","typeIdField":null,
        "fields":[{"name":"OBJECTID","type":"esriFieldTypeOID","alias":"OBJECTID","domain":null}],
        "relationships":[],"canModifyLayer":false,"canScaleSymbols":false,"hasLabels":false,
        "capabilities":"Map,Query,Data","maxRecordCount":1000,"supportsStatistics":true,
        "supportsAdvancedQueries":true,"supportedQueryFormats":"JSON, AMF",
        "ownershipBasedAccessControlForFeatures":{"allowOthersToQuery":true},"useStandardizedQueries":true}"""
            )

        with open(
            sanitize(endpoint, "/query?f=json_where=1=1&returnIdsOnly=true"), "wb"
        ) as f:
            f.write(
                b"""
        {
         "objectIdFieldName": "OBJECTID",
         "objectIds": [
          1
         ]
        }
        """
            )

        # Create test layer
        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' crs='epsg:4326'",
            "test",
            "arcgisfeatureserver",
        )
        self.assertTrue(vl.isValid())

        extent = QgsLayerMetadata.Extent()
        extent1 = QgsLayerMetadata.SpatialExtent()
        extent1.extentCrs = QgsCoordinateReferenceSystem.fromEpsgId(4326)
        extent1.bounds = QgsBox3d(QgsRectangle(-71.123, 66.33, -65.32, 78.3))
        extent.setSpatialExtents([extent1])
        md = vl.metadata()
        self.assertEqual(md.extent(), extent)
        self.assertEqual(md.crs(), QgsCoordinateReferenceSystem.fromEpsgId(4326))
        self.assertEqual(md.identifier(), "http://" + sanitize(endpoint, ""))
        self.assertEqual(md.parentIdentifier(), "http://" + self.basetestpath + "/2")
        self.assertEqual(md.type(), "dataset")
        self.assertEqual(md.abstract(), "QGIS Provider Test Layer")
        self.assertEqual(md.title(), "QGIS Test")
        self.assertEqual(md.rights(), ["not copyright"])
        l = QgsLayerMetadata.Link()
        l.name = "Source"
        l.type = "WWW:LINK"
        l.url = "http://" + sanitize(endpoint, "")
        self.assertEqual(md.links(), [l])

    def testFieldAlias(self):
        """Test that field aliases are correctly acquired from provider"""

        endpoint = self.basetestpath + "/alias_fake_qgis_http_endpoint"
        with open(sanitize(endpoint, "?f=json"), "wb") as f:
            f.write(
                b"""
        {"currentVersion":10.22,"id":1,"name":"QGIS Test","type":"Feature Layer","description":
        "QGIS Provider Test Layer","geometryType":"esriGeometryPoint","copyrightText":"not copyright","parentLayer":{"id":2,"name":"QGIS Tests"},"subLayers":[],
        "minScale":72225,"maxScale":0,
        "defaultVisibility":true,
        "extent":{"xmin":-71.123,"ymin":66.33,"xmax":-65.32,"ymax":78.3,
        "spatialReference":{"wkid":4326,"latestWkid":4326}},
        "hasAttachments":false,"htmlPopupType":"esriServerHTMLPopupTypeAsHTMLText",
        "displayField":"LABEL","typeIdField":null,
        "fields":[{"name":"OBJECTID","type":"esriFieldTypeOID","alias":"field id","domain":null},{"name":"second","type":"esriFieldTypeString","domain":null}],
        "relationships":[],"canModifyLayer":false,"canScaleSymbols":false,"hasLabels":false,
        "capabilities":"Map,Query,Data","maxRecordCount":1000,"supportsStatistics":true,
        "supportsAdvancedQueries":true,"supportedQueryFormats":"JSON, AMF",
        "ownershipBasedAccessControlForFeatures":{"allowOthersToQuery":true},"useStandardizedQueries":true}"""
            )

        with open(
            sanitize(endpoint, "/query?f=json_where=1=1&returnIdsOnly=true"), "wb"
        ) as f:
            f.write(
                b"""
        {
         "objectIdFieldName": "OBJECTID",
         "objectIds": [
          1
         ]
        }
        """
            )

        # Create test layer
        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' crs='epsg:4326'",
            "test",
            "arcgisfeatureserver",
        )
        self.assertTrue(vl.isValid())

        self.assertEqual(vl.fields().at(0).name(), "OBJECTID")
        self.assertEqual(vl.fields().at(0).alias(), "field id")
        self.assertEqual(vl.fields().at(1).name(), "second")
        self.assertFalse(vl.fields().at(1).alias())

    def testCategorizedRenderer(self):
        """Test that the categorized renderer is correctly acquired from provider"""

        endpoint = self.basetestpath + "/renderer_fake_qgis_http_endpoint"
        with open(sanitize(endpoint, "?f=json"), "wb") as f:
            f.write(
                b"""
        {"currentVersion":10.22,"id":1,"name":"QGIS Test","type":"Feature Layer","description":
        "QGIS Provider Test Layer","geometryType":"esriGeometryPoint","copyrightText":"not copyright","parentLayer":{"id":2,"name":"QGIS Tests"},"subLayers":[],
        "minScale":72225,"maxScale":0,
        "defaultVisibility":true,
        "extent":{"xmin":-71.123,"ymin":66.33,"xmax":-65.32,"ymax":78.3,
        "spatialReference":{"wkid":4326,"latestWkid":4326}},
        "hasAttachments":false,"htmlPopupType":"esriServerHTMLPopupTypeAsHTMLText",
        "displayField":"LABEL","typeIdField":null,
        "fields":[{"name":"OBJECTID","type":"esriFieldTypeOID","alias":"OBJECTID","domain":null}],
        "relationships":[],"canModifyLayer":false,"canScaleSymbols":false,"hasLabels":false,
        "capabilities":"Map,Query,Data","maxRecordCount":1000,"supportsStatistics":true,
        "supportsAdvancedQueries":true,"supportedQueryFormats":"JSON, AMF",
        "drawingInfo":{"renderer": {
    "type": "uniqueValue",
    "field1": "COUNTRY",
    "uniqueValueInfos": [
      {
        "value": "US",
        "symbol": {
          "color": [
            253,
            127,
            111,
            255
          ],
          "size": 12.75,
          "angle": 0,
          "xoffset": 0,
          "yoffset": 0,
          "type": "esriSMS",
          "style": "esriSMSCircle",
          "outline": {
            "color": [
              26,
              26,
              26,
              255
            ],
            "width": 0.75,
            "type": "esriSLS",
            "style": "esriSLSSolid"
          }
        },
        "label": "US"
      },
      {
        "value": "Canada",
        "symbol": {
          "color": [
            126,
            176,
            213,
            255
          ],
          "size": 12.75,
          "angle": 0,
          "xoffset": 0,
          "yoffset": 0,
          "type": "esriSMS",
          "style": "esriSMSCircle",
          "outline": {
            "color": [
              26,
              26,
              26,
              255
            ],
            "width": 0.75,
            "type": "esriSLS",
            "style": "esriSLSSolid"
          }
        },
        "label": "Canada"
      }]}},
        "ownershipBasedAccessControlForFeatures":{"allowOthersToQuery":true},"useStandardizedQueries":true}"""
            )

        with open(
            sanitize(endpoint, "/query?f=json_where=1=1&returnIdsOnly=true"), "wb"
        ) as f:
            f.write(
                b"""
        {
         "objectIdFieldName": "OBJECTID",
         "objectIds": [
          1
         ]
        }
        """
            )

        # Create test layer
        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' crs='epsg:4326'",
            "test",
            "arcgisfeatureserver",
        )
        self.assertTrue(vl.isValid())
        self.assertIsNotNone(vl.dataProvider().createRenderer())
        self.assertIsInstance(vl.renderer(), QgsCategorizedSymbolRenderer)
        self.assertEqual(len(vl.renderer().categories()), 2)
        self.assertEqual(vl.renderer().categories()[0].value(), "US")
        self.assertEqual(vl.renderer().categories()[1].value(), "Canada")

    def testGraduatedRendererContinuous(self):
        """
        Test that the graduated renderer with continuous coloring
        is correctly acquired from provider
        """

        endpoint = self.basetestpath + "/class_breaks_renderer_fake_qgis_http_endpoint"
        with open(sanitize(endpoint, "?f=json"), "wb") as f:
            f.write(
                b"""{
      "currentVersion": 11.2,
      "id": 0,
      "name": "Test graduated renderer",
      "type": "Feature Layer",
      "useStandardizedQueries": true,
      "geometryType": "esriGeometryPolygon",
      "minScale": 0,
      "maxScale": 1155581,
      "extent": {
        "xmin": -17771274.9623,
        "ymin": 2175061.919500001,
        "xmax": -7521909.497300002,
        "ymax": 9988155.384400003,
        "spatialReference": {
          "wkid": 102100,
          "latestWkid": 3857
        }
      },
      "drawingInfo": {
        "renderer": {
          "visualVariables": [
            {
              "type": "colorInfo",
              "field": "SUM",
              "valueExpression": null,
              "stops": [
                {
                  "value": 10151,
                  "color": [
                    255,
                    196,
                    174,
                    255
                  ],
                  "label": "< 10,151"
                },
                {
                  "value": 632613.25,
                  "color": [
                    249,
                    129,
                    108,
                    255
                  ],
                  "label": null
                },
                {
                  "value": 1255075.5,
                  "color": [
                    236,
                    82,
                    68,
                    255
                  ],
                  "label": "1,255,075"
                },
                {
                  "value": 1877537.75,
                  "color": [
                    194,
                    61,
                    51,
                    255
                  ],
                  "label": null
                },
                {
                  "value": 2500000,
                  "color": [
                    123,
                    66,
                    56,
                    255
                  ],
                  "label": "> 2,500,000"
                }
              ]
            },
            {
              "type": "sizeInfo",
              "target": "outline",
              "expression": "view.scale",
              "valueExpression": "$view.scale",
              "stops": [
                {
                  "size": 1.5,
                  "value": 3468153
                },
                {
                  "size": 0.75,
                  "value": 10837979
                },
                {
                  "size": 0.375,
                  "value": 43351915
                },
                {
                  "size": 0,
                  "value": 86703831
                }
              ]
            }
          ],
          "authoringInfo": {
            "classificationMethod": "esriClassifyEqualInterval",
            "visualVariables": [
              {
                "type": "colorInfo",
                "minSliderValue": 10151,
                "maxSliderValue": 15185477,
                "theme": "high-to-low"
              }
            ]
          },
          "type": "classBreaks",
          "field": "SUM",
          "minValue": -9007199254740991,
          "classBreakInfos": [
            {
              "symbol": {
                "color": [
                  170,
                  170,
                  170,
                  255
                ],
                "outline": {
                  "color": [
                    194,
                    194,
                    194,
                    64
                  ],
                  "width": 0.75,
                  "type": "esriSLS",
                  "style": "esriSLSSolid"
                },
                "type": "esriSFS",
                "style": "esriSFSSolid"
              },
              "classMaxValue": 9007199254740991
            }
          ]
        },
        "transparency": 20
      },
      "allowGeometryUpdates": true
    }"""
            )

        with open(
            sanitize(endpoint, "/query?f=json_where=1=1&returnIdsOnly=true"), "wb"
        ) as f:
            f.write(
                b"""
        {
         "objectIdFieldName": "OBJECTID",
         "objectIds": [
          1
         ]
        }
        """
            )

        # Create test layer
        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' crs='epsg:3857'",
            "test",
            "arcgisfeatureserver",
        )
        self.assertTrue(vl.isValid())
        self.assertIsNotNone(vl.dataProvider().createRenderer())
        self.assertIsInstance(vl.renderer(), QgsSingleSymbolRenderer)
        self.assertIsInstance(vl.renderer().symbol(), QgsFillSymbol)

        prop = (
            vl.renderer()
            .symbol()[0]
            .dataDefinedProperties()
            .property(QgsSymbolLayer.Property.FillColor)
        )
        self.assertEqual(prop.propertyType(), Qgis.PropertyType.Field)
        self.assertEqual(prop.field(), "SUM")
        self.assertIsInstance(prop.transformer(), QgsColorRampTransformer)
        self.assertEqual(prop.transformer().minValue(), 10151)
        self.assertEqual(prop.transformer().maxValue(), 2500000)
        ramp = prop.transformer().colorRamp()
        self.assertIsInstance(ramp, QgsGradientColorRamp)
        self.assertEqual(ramp.color1().name(), "#ffc4ae")
        self.assertEqual(ramp.color2().name(), "#7b4238")
        self.assertEqual([stop.offset for stop in ramp.stops()], [0.25, 0.5, 0.75])
        self.assertEqual(
            [stop.color.name() for stop in ramp.stops()],
            ["#f9816c", "#ec5244", "#c23d33"],
        )

    def testGraduatedRendererClassedColor(self):
        """
        Test that the graduated renderer with classified colors
        is correctly acquired from provider
        """

        endpoint = self.basetestpath + "/class_breaks_renderer_fake_qgis_http_endpoint"
        with open(sanitize(endpoint, "?f=json"), "wb") as f:
            f.write(
                b"""{
  "currentVersion": 11.2,
  "id": 0,
  "name": "Test graduated renderer",
  "type": "Feature Layer",
  "useStandardizedQueries": true,
  "geometryType": "esriGeometryPolygon",
  "minScale": 0,
  "maxScale": 1155581,
  "extent": {
    "xmin": -17771274.9623,
    "ymin": 2175061.919500001,
    "xmax": -7521909.497300002,
    "ymax": 9988155.384400003,
    "spatialReference": {
      "wkid": 102100,
      "latestWkid": 3857
    }
  },
  "drawingInfo": {
    "renderer": {
      "type": "classBreaks",
      "authoringInfo": {
        "type": "classedColor",
        "colorRamp": {
          "type": "multipart",
          "colorRamps": [
            {
              "type": "algorithmic",
              "algorithm": "esriCIELabAlgorithm",
              "fromColor": [
                229,
                237,
                206,
                255
              ],
              "toColor": [
                229,
                237,
                206,
                255
              ]
            },
            {
              "type": "algorithmic",
              "algorithm": "esriCIELabAlgorithm",
              "fromColor": [
                155,
                196,
                194,
                255
              ],
              "toColor": [
                155,
                196,
                194,
                255
              ]
            },
            {
              "type": "algorithmic",
              "algorithm": "esriCIELabAlgorithm",
              "fromColor": [
                105,
                168,
                184,
                255
              ],
              "toColor": [
                105,
                168,
                184,
                255
              ]
            },
            {
              "type": "algorithmic",
              "algorithm": "esriCIELabAlgorithm",
              "fromColor": [
                75,
                127,
                153,
                255
              ],
              "toColor": [
                75,
                127,
                153,
                255
              ]
            },
            {
              "type": "algorithmic",
              "algorithm": "esriCIELabAlgorithm",
              "fromColor": [
                48,
                86,
                122,
                255
              ],
              "toColor": [
                48,
                86,
                122,
                255
              ]
            }
          ]
        },
        "classificationMethod": "esriClassifyNaturalBreaks"
      },
      "field": "Value",
      "classificationMethod": "esriClassifyNaturalBreaks",
      "minValue": 7,
      "classBreakInfos": [
        {
          "symbol": {
            "type": "esriSFS",
            "style": "esriSFSSolid",
            "color": [
              230,
              238,
              207,
              255
            ],
            "outline": {
              "type": "esriSLS",
              "style": "esriSLSSolid",
              "color": [
                110,
                110,
                110,
                255
              ],
              "width": 0.7
            }
          },
          "classMaxValue": 7,
          "label": "7.000000"
        },
        {
          "symbol": {
            "type": "esriSFS",
            "style": "esriSFSSolid",
            "color": [
              155,
              196,
              193,
              255
            ],
            "outline": {
              "type": "esriSLS",
              "style": "esriSLSSolid",
              "color": [
                110,
                110,
                110,
                255
              ],
              "width": 0.7
            }
          },
          "classMaxValue": 8,
          "label": "7.000001 - 8.000000"
        },
        {
          "symbol": {
            "type": "esriSFS",
            "style": "esriSFSSolid",
            "color": [
              105,
              168,
              183,
              255
            ],
            "outline": {
              "type": "esriSLS",
              "style": "esriSLSSolid",
              "color": [
                110,
                110,
                110,
                255
              ],
              "width": 0.7
            }
          },
          "classMaxValue": 11,
          "label": "8.000001 - 11.000000"
        },
        {
          "symbol": {
            "type": "esriSFS",
            "style": "esriSFSSolid",
            "color": [
              75,
              126,
              152,
              255
            ],
            "outline": {
              "type": "esriSLS",
              "style": "esriSLSSolid",
              "color": [
                110,
                110,
                110,
                255
              ],
              "width": 0.7
            }
          },
          "classMaxValue": 13,
          "label": "11.000001 - 13.000000"
        },
        {
          "symbol": {
            "type": "esriSFS",
            "style": "esriSFSSolid",
            "color": [
              46,
              85,
              122,
              255
            ],
            "outline": {
              "type": "esriSLS",
              "style": "esriSLSSolid",
              "color": [
                110,
                110,
                110,
                255
              ],
              "width": 0.7
            }
          },
          "classMaxValue": 20,
          "label": "13.000001 - 20.000000"
        }
      ],
      "legendOptions": {
        "order": "ascendingValues"
      }
    },
    "scaleSymbols": true,
    "transparency": 0,
    "labelingInfo": null
  },
  "allowGeometryUpdates": true
}"""
            )

        with open(
            sanitize(endpoint, "/query?f=json_where=1=1&returnIdsOnly=true"), "wb"
        ) as f:
            f.write(
                b"""
        {
         "objectIdFieldName": "OBJECTID",
         "objectIds": [
          1
         ]
        }
        """
            )

        # Create test layer
        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' crs='epsg:3857'",
            "test",
            "arcgisfeatureserver",
        )
        self.assertTrue(vl.isValid())
        self.assertIsNotNone(vl.dataProvider().createRenderer())
        self.assertIsInstance(vl.renderer(), QgsGraduatedSymbolRenderer)
        self.assertIsInstance(vl.renderer().sourceSymbol(), QgsSymbol)
        self.assertIsInstance(vl.renderer().ranges()[0], QgsRendererRange)
        self.assertEqual(len(vl.renderer().ranges()), 5)
        _range = vl.renderer().ranges()[0]
        self.assertEqual(_range.lowerValue(), 7)
        self.assertEqual(_range.upperValue(), 7)
        self.assertEqual(_range.label(), "7.000000")
        self.assertEqual(_range.symbol().color().name(), "#e6eecf")

        _range = vl.renderer().ranges()[1]
        self.assertEqual(_range.lowerValue(), 7)
        self.assertEqual(_range.upperValue(), 8)
        self.assertEqual(_range.label(), "7.000001 - 8.000000")
        self.assertEqual(_range.symbol().color().name(), "#9bc4c1")

        _range = vl.renderer().ranges()[2]
        self.assertEqual(_range.lowerValue(), 8)
        self.assertEqual(_range.upperValue(), 11)
        self.assertEqual(_range.label(), "8.000001 - 11.000000")
        self.assertEqual(_range.symbol().color().name(), "#69a8b7")

        _range = vl.renderer().ranges()[3]
        self.assertEqual(_range.lowerValue(), 11)
        self.assertEqual(_range.upperValue(), 13)
        self.assertEqual(_range.label(), "11.000001 - 13.000000")
        self.assertEqual(_range.symbol().color().name(), "#4b7e98")

        _range = vl.renderer().ranges()[4]
        self.assertEqual(_range.lowerValue(), 13)
        self.assertEqual(_range.upperValue(), 20)
        self.assertEqual(_range.label(), "13.000001 - 20.000000")
        self.assertEqual(_range.symbol().color().name(), "#2e557a")

    def testBboxRestriction(self):
        """
        Test limiting provider to features within a preset bounding box
        """
        endpoint = self.basetestpath + "/fake_qgis_http_endpoint"
        vl = QgsVectorLayer(
            "url='http://"
            + endpoint
            + "' crs='epsg:4326' bbox='-70.000000,67.000000,-60.000000,80.000000'",
            "test",
            "arcgisfeatureserver",
        )
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.featureCount(), 2)
        self.assertEqual([f["pk"] for f in vl.getFeatures()], [2, 4])

    def testBadMultiPoints(self):
        """
        Test invalid server response where a layer's type is multipoint but single point geometries
        are returned. Thanks Jack. Thack.
        """
        endpoint = self.basetestpath + "/multipoint_fake_qgis_http_endpoint"
        with open(sanitize(endpoint, "?f=json"), "wb") as f:
            f.write(
                b"""
        {"currentVersion":10.22,"id":1,"name":"QGIS Test","type":"Feature Layer","description":
        "QGIS Provider Test Layer.\n","geometryType":"esriGeometryMultipoint","copyrightText":"","parentLayer":{"id":0,"name":"QGIS Tests"},"subLayers":[],
        "minScale":72225,"maxScale":0,
        "defaultVisibility":true,
        "extent":{"xmin":-71.123,"ymin":66.33,"xmax":-65.32,"ymax":78.3,
        "spatialReference":{"wkid":4326,"latestWkid":4326}},
        "hasAttachments":false,"htmlPopupType":"esriServerHTMLPopupTypeAsHTMLText",
        "displayField":"LABEL","typeIdField":null,
        "fields":[{"name":"OBJECTID","type":"esriFieldTypeOID","alias":"OBJECTID","domain":null}],
        "relationships":[],"canModifyLayer":false,"canScaleSymbols":false,"hasLabels":false,
        "capabilities":"Map,Query,Data","maxRecordCount":1000,"supportsStatistics":true,
        "supportsAdvancedQueries":true,"supportedQueryFormats":"JSON, AMF",
        "ownershipBasedAccessControlForFeatures":{"allowOthersToQuery":true},"useStandardizedQueries":true}"""
            )

        with open(
            sanitize(endpoint, "/query?f=json_where=1=1&returnIdsOnly=true"), "wb"
        ) as f:
            f.write(
                b"""
        {
         "objectIdFieldName": "OBJECTID",
         "objectIds": [
          1,
          2,
          3
         ]
        }
        """
            )

        # Create test layer
        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' crs='epsg:4326'",
            "test",
            "arcgisfeatureserver",
        )

        self.assertTrue(vl.isValid())
        with open(
            sanitize(
                endpoint,
                "/query?f=json&objectIds=1,2,3&inSR=4326&outSR=4326&returnGeometry=true&outFields=*&returnM=false&returnZ=false",
            ),
            "wb",
        ) as f:
            f.write(
                b"""
        {
         "displayFieldName": "name",
         "fieldAliases": {
          "name": "name"
         },
         "geometryType": "esriGeometryMultipoint",
         "spatialReference": {
          "wkid": 4326,
          "latestWkid": 4326
         },
         "fields":[{"name":"OBJECTID","type":"esriFieldTypeOID","alias":"OBJECTID","domain":null},
        {"name":"Shape","type":"esriFieldTypeGeometry","alias":"Shape","domain":null}],
         "features": [
          {
           "attributes": {
            "OBJECTID": 1
           },
           "geometry": {
            "x": -70,
            "y": 66
           }
          },
          {
           "attributes": {
            "OBJECTID": 2
           },
           "geometry": null
          },
          {
           "attributes": {
            "OBJECTID": 3
           },
           "geometry":
           {"points" :[[-68,70],
           [-22,21]]
           }
          }
         ]
        }"""
            )

        features = [f for f in vl.getFeatures()]
        self.assertEqual(len(features), 3)
        self.assertEqual(
            [f.geometry().asWkt() for f in features],
            ["MultiPoint ((-70 66))", "", "MultiPoint ((-68 70),(-22 21))"],
        )

    def testDomain(self):
        """
        Test fields with a domain are mapped to value map wrapper, for correct value display
        """
        endpoint = self.basetestpath + "/domain_fake_qgis_http_endpoint"
        with open(sanitize(endpoint, "?f=json"), "wb") as f:
            f.write(
                b"""
        {"currentVersion":10.22,"id":1,"name":"QGIS Test","type":"Feature Layer","description":
        "QGIS Provider Test Layer.\n","geometryType":"esriGeometryPoint","copyrightText":"","parentLayer":{"id":0,"name":"QGIS Tests"},"subLayers":[],
        "minScale":72225,"maxScale":0,
        "defaultVisibility":true,
        "extent":{"xmin":-71.123,"ymin":66.33,"xmax":-65.32,"ymax":78.3,
        "spatialReference":{"wkid":4326,"latestWkid":4326}},
        "hasAttachments":false,"htmlPopupType":"esriServerHTMLPopupTypeAsHTMLText",
        "displayField":"LABEL","typeIdField":null,
        "fields":[{"name":"OBJECTID","type":"esriFieldTypeOID","alias":"OBJECTID","domain":null},
        {"name":"with_domain","type":"esriFieldTypeInteger","alias":"with_domain",
        "domain": {
        "type": "codedValue",
        "name": "Test Domain",
        "description": "",
        "codedValues": [
         {
          "name": "Value 1",
          "code": 1
         },
         {
          "name": "Value 2",
          "code": 2
         },
         {
          "name": "Value 3",
          "code": 3
         }
        ],
        "mergePolicy": "esriMPTDefaultValue",
        "splitPolicy": "esriSPTDefaultValue"
       }
       }],
        "relationships":[],"canModifyLayer":false,"canScaleSymbols":false,"hasLabels":false,
        "capabilities":"Map,Query,Data","maxRecordCount":1000,"supportsStatistics":true,
        "supportsAdvancedQueries":true,"supportedQueryFormats":"JSON, AMF",
        "ownershipBasedAccessControlForFeatures":{"allowOthersToQuery":true},"useStandardizedQueries":true}"""
            )

        with open(
            sanitize(endpoint, "/query?f=json_where=1=1&returnIdsOnly=true"), "wb"
        ) as f:
            f.write(
                b"""
        {
         "objectIdFieldName": "OBJECTID",
         "objectIds": [
          1,
          2,
          3
         ]
        }
        """
            )

        # Create test layer
        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' crs='epsg:4326'",
            "test",
            "arcgisfeatureserver",
        )

        self.assertTrue(vl.isValid())
        self.assertFalse(vl.fields()[0].editorWidgetSetup().type())
        self.assertEqual(vl.fields()[1].editorWidgetSetup().type(), "ValueMap")
        self.assertEqual(
            vl.fields()[1].editorWidgetSetup().config(),
            {"map": [{"Value 1": 1.0}, {"Value 2": 2.0}, {"Value 3": 3.0}]},
        )

    def testTemporal1(self):
        """
        Test timeinfo parsing
        """
        endpoint = self.basetestpath + "/temporal1_fake_qgis_http_endpoint"
        with open(sanitize(endpoint, "?f=json"), "wb") as f:
            f.write(
                b"""
        {"currentVersion":10.22,"id":1,"name":"QGIS Test","type":"Feature Layer","description":
        "QGIS Provider Test Layer.\n","geometryType":"esriGeometryPoint","copyrightText":"","parentLayer":{"id":0,"name":"QGIS Tests"},"subLayers":[],
        "minScale":72225,"maxScale":0,
        "defaultVisibility":true,
        "extent":{"xmin":-71.123,"ymin":66.33,"xmax":-65.32,"ymax":78.3,
        "spatialReference":{"wkid":4326,"latestWkid":4326}},
        "hasAttachments":false,"htmlPopupType":"esriServerHTMLPopupTypeAsHTMLText",
        "displayField":"LABEL","typeIdField":null,
        "fields":[{"name":"OBJECTID","type":"esriFieldTypeOID","alias":"OBJECTID","domain":null}
        ],
        "timeInfo": {
          "startTimeField": "date_start",
          "endTimeField": null,
          "trackIdField": null,
          "timeExtent": [
           1142000000000,
           1487000000000
          ]
         },
        "relationships":[],"canModifyLayer":false,"canScaleSymbols":false,"hasLabels":false,
        "capabilities":"Map,Query,Data","maxRecordCount":1000,"supportsStatistics":true,
        "supportsAdvancedQueries":true,"supportedQueryFormats":"JSON, AMF",
        "ownershipBasedAccessControlForFeatures":{"allowOthersToQuery":true},"useStandardizedQueries":true}"""
            )

        with open(
            sanitize(endpoint, "/query?f=json_where=1=1&returnIdsOnly=true"), "wb"
        ) as f:
            f.write(
                b"""
        {
         "objectIdFieldName": "OBJECTID",
         "objectIds": [
          1,
          2,
          3
         ]
        }
        """
            )

        # Create test layer
        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' crs='epsg:4326'",
            "test",
            "arcgisfeatureserver",
        )

        self.assertTrue(vl.isValid())
        self.assertTrue(
            vl.dataProvider().temporalCapabilities().hasTemporalCapabilities()
        )
        self.assertEqual(
            vl.dataProvider().temporalCapabilities().startField(), "date_start"
        )
        self.assertFalse(vl.dataProvider().temporalCapabilities().endField())
        self.assertEqual(
            vl.dataProvider().temporalCapabilities().mode(),
            QgsVectorDataProviderTemporalCapabilities.TemporalMode.ProviderStoresFeatureDateTimeInstantInField,
        )
        self.assertEqual(
            vl.dataProvider().temporalCapabilities().availableTemporalRange().begin(),
            QDateTime(QDate(2006, 3, 10), QTime(14, 13, 20), Qt.TimeSpec.UTC),
        )
        self.assertEqual(
            vl.dataProvider().temporalCapabilities().availableTemporalRange().end(),
            QDateTime(QDate(2017, 2, 13), QTime(15, 33, 20), Qt.TimeSpec.UTC),
        )

    def testTemporal2(self):
        """
        Test timeinfo parsing
        """
        endpoint = self.basetestpath + "/temporal2_fake_qgis_http_endpoint"
        with open(sanitize(endpoint, "?f=json"), "wb") as f:
            f.write(
                b"""
        {"currentVersion":10.22,"id":1,"name":"QGIS Test","type":"Feature Layer","description":
        "QGIS Provider Test Layer.\n","geometryType":"esriGeometryPoint","copyrightText":"","parentLayer":{"id":0,"name":"QGIS Tests"},"subLayers":[],
        "minScale":72225,"maxScale":0,
        "defaultVisibility":true,
        "extent":{"xmin":-71.123,"ymin":66.33,"xmax":-65.32,"ymax":78.3,
        "spatialReference":{"wkid":4326,"latestWkid":4326}},
        "hasAttachments":false,"htmlPopupType":"esriServerHTMLPopupTypeAsHTMLText",
        "displayField":"LABEL","typeIdField":null,
        "fields":[{"name":"OBJECTID","type":"esriFieldTypeOID","alias":"OBJECTID","domain":null}
        ],
        "timeInfo": {
          "startTimeField": "date_start",
          "endTimeField": "date_end",
          "trackIdField": null,
          "timeExtent": [
           1142000000000,
           1487000000000
          ]
         },
        "relationships":[],"canModifyLayer":false,"canScaleSymbols":false,"hasLabels":false,
        "capabilities":"Map,Query,Data","maxRecordCount":1000,"supportsStatistics":true,
        "supportsAdvancedQueries":true,"supportedQueryFormats":"JSON, AMF",
        "ownershipBasedAccessControlForFeatures":{"allowOthersToQuery":true},"useStandardizedQueries":true}"""
            )

        with open(
            sanitize(endpoint, "/query?f=json_where=1=1&returnIdsOnly=true"), "wb"
        ) as f:
            f.write(
                b"""
        {
         "objectIdFieldName": "OBJECTID",
         "objectIds": [
          1,
          2,
          3
         ]
        }
        """
            )

        # Create test layer
        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' crs='epsg:4326'",
            "test",
            "arcgisfeatureserver",
        )

        self.assertTrue(vl.isValid())
        self.assertTrue(
            vl.dataProvider().temporalCapabilities().hasTemporalCapabilities()
        )
        self.assertEqual(
            vl.dataProvider().temporalCapabilities().startField(), "date_start"
        )
        self.assertEqual(
            vl.dataProvider().temporalCapabilities().endField(), "date_end"
        )
        self.assertEqual(
            vl.dataProvider().temporalCapabilities().mode(),
            QgsVectorDataProviderTemporalCapabilities.TemporalMode.ProviderStoresFeatureDateTimeStartAndEndInSeparateFields,
        )
        self.assertEqual(
            vl.dataProvider().temporalCapabilities().availableTemporalRange().begin(),
            QDateTime(QDate(2006, 3, 10), QTime(14, 13, 20), Qt.TimeSpec.UTC),
        )
        self.assertEqual(
            vl.dataProvider().temporalCapabilities().availableTemporalRange().end(),
            QDateTime(QDate(2017, 2, 13), QTime(15, 33, 20), Qt.TimeSpec.UTC),
        )

    def testImageServer(self):
        """
        Test connecting to a image server endpoints works as a footprint featureserver
        """
        endpoint = self.basetestpath + "/imageserver_fake_qgis_http_endpoint"
        with open(sanitize(endpoint, "?f=json"), "wb") as f:
            f.write(
                b"""
        {
 "currentVersion": 10.51,
 "serviceDescription": "test",
 "name": "test",
 "description": "test",
 "extent": {
  "xmin": 1,
  "ymin": 1,
  "xmax": 2,
  "ymax": 2,
  "spatialReference": {
   "wkid": 102100,
   "latestWkid": 3857
  }
 },
 "initialExtent": {
  "xmin": 1,
  "ymin": 1,
  "xmax": 2,
  "ymax": 2,
  "spatialReference": {
   "wkid": 102100,
   "latestWkid": 3857
  }
 },
 "fullExtent": {
  "xmin": 1,
  "ymin": 1,
  "xmax": 2,
  "ymax": 2,
  "spatialReference": {
   "wkid": 102100,
   "latestWkid": 3857
  }
 },
 "heightModelInfo": {
  "heightModel": "orthometric",
  "heightUnit": "meter"
 },
 "pixelSizeX": 30,
 "pixelSizeY": 30,
 "bandCount": 1,
 "pixelType": "U8",
 "minPixelSize": 38,
 "maxPixelSize": 156543,
 "copyrightText": "",
 "serviceDataType": "esriImageServiceDataTypeGeneric",
 "minValues": [
  0
 ],
 "maxValues": [
  30
 ],
 "meanValues": [
  5
 ],
 "stdvValues": [
  4
 ],
 "objectIdField": "OBJECTID",
 "fields": [
  {
   "name": "OBJECTID",
   "type": "esriFieldTypeOID",
   "alias": "OBJECTID",
   "domain": null
  },
  {
   "name": "Shape",
   "type": "esriFieldTypeGeometry",
   "alias": "Shape",
   "domain": null
  },
  {
   "name": "Name",
   "type": "esriFieldTypeString",
   "alias": "Name",
   "domain": null,
   "length": 50
  },
  {
   "name": "MinPS",
   "type": "esriFieldTypeDouble",
   "alias": "MinPS",
   "domain": null
  },
  {
   "name": "MaxPS",
   "type": "esriFieldTypeDouble",
   "alias": "MaxPS",
   "domain": null
  },
  {
   "name": "LowPS",
   "type": "esriFieldTypeDouble",
   "alias": "LowPS",
   "domain": null
  },
  {
   "name": "HighPS",
   "type": "esriFieldTypeDouble",
   "alias": "HighPS",
   "domain": null
  }
 ],
 "capabilities": "Catalog,Mensuration,Image,Metadata",
 "defaultMosaicMethod": "Northwest",
 "allowedMosaicMethods": "NorthWest,Center,LockRaster,ByAttribute,Nadir,Viewpoint,Seamline,None",
 "sortField": "",
 "sortValue": null,
 "mosaicOperator": "First",
 "maxDownloadSizeLimit": 4096,
 "defaultCompressionQuality": 75,
 "defaultResamplingMethod": "Nearest",
 "maxImageHeight": 4100,
 "maxImageWidth": 15000,
 "maxRecordCount": 2147483647,
 "maxDownloadImageCount": 200,
 "maxMosaicImageCount": 2147483647,
 "singleFusedMapCache": true,
 "tileInfo": {
  "rows": 256,
  "cols": 256,
  "dpi": 96,
  "format": "MIXED",
  "compressionQuality": 75,
  "origin": {
   "x": 2,
   "y": 2
  },
  "spatialReference": {
   "wkid": 102100,
   "latestWkid": 3857
  },
  "lods": [
   {
    "level": 0,
    "resolution": 156543.033928,
    "scale": 5.91657527591555E8
   },
   {
    "level": 1,
    "resolution": 78271.5169639999,
    "scale": 2.95828763795777E8
   }
  ]
 },
 "cacheType": "Map",
 "allowRasterFunction": true,
 "rasterFunctionInfos": [
  {
   "name": "Classified",
   "description": "A raster function template.",
   "help": ""
  },
  {
   "name": "None",
   "description": "",
   "help": ""
  }
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
 "minScale": 5,
 "maxScale": 144447,
 "exportTilesAllowed": false,
 "hasMultidimensions": false,
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
            sanitize(endpoint, "/query?f=json_where=1=1&returnIdsOnly=true"), "wb"
        ) as f:
            f.write(
                b"""
        {
         "objectIdFieldName": "OBJECTID",
         "objectIds": [
          1,
          2,
          3
         ]
        }
        """
            )

        # Create test layer
        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' crs='epsg:4326'",
            "test",
            "arcgisfeatureserver",
        )

        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Type.Polygon)

    def testDelete(self):
        # delete capability
        endpoint = self.basetestpath + "/delete_test_fake_qgis_http_endpoint"
        with open(sanitize(endpoint, "?f=json"), "wb") as f:
            f.write(
                b"""
                {"currentVersion":10.22,"id":1,"name":"QGIS Test","type":"Feature Layer","description":
                "QGIS Provider Test Layer","geometryType":"esriGeometryPoint","copyrightText":"not copyright","parentLayer":{"id":2,"name":"QGIS Tests"},"subLayers":[],
                "minScale":72225,"maxScale":0,
                "defaultVisibility":true,
                "extent":{"xmin":-71.123,"ymin":66.33,"xmax":-65.32,"ymax":78.3,
                "spatialReference":{"wkid":4326,"latestWkid":4326}},
                "hasAttachments":false,"htmlPopupType":"esriServerHTMLPopupTypeAsHTMLText",
                "displayField":"LABEL","typeIdField":null,
                "fields":[{"name":"OBJECTID","type":"esriFieldTypeOID","alias":"OBJECTID","domain":null}],
                "relationships":[],"canModifyLayer":false,"canScaleSymbols":false,"hasLabels":false,
                "capabilities":"Map,Query,Data,Delete","maxRecordCount":1000,"supportsStatistics":true,
                "supportsAdvancedQueries":true,"supportedQueryFormats":"JSON, AMF",
                "ownershipBasedAccessControlForFeatures":{"allowOthersToQuery":true},"useStandardizedQueries":true}"""
            )

        with open(
            sanitize(endpoint, "/query?f=json_where=1=1&returnIdsOnly=true"), "wb"
        ) as f:
            f.write(
                b"""
                {
                 "objectIdFieldName": "OBJECTID",
                 "objectIds": [
                  1
                 ]
                }
                """
            )

        delete_endpoint = sanitize(endpoint, "/deleteFeatures")
        with open(delete_endpoint, "wb") as f:
            f.write(
                b"""{
   "deleteResults": [
   {
    "objectId": 1,
    "success": true
   }
  ]
}"""
            )

        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' crs='epsg:4326'",
            "test",
            "arcgisfeatureserver",
        )
        self.assertTrue(vl.isValid())

        res = vl.dataProvider().deleteFeatures([0])
        self.assertTrue(res)

        with open(delete_endpoint + "_payload") as f:
            res = "\n".join(f.readlines())
            self.assertEqual(res, "f=json&objectIds=1")

    def testAddSuccess(self):
        # add capability
        endpoint = self.basetestpath + "/delete_test_fake_qgis_http_endpoint"
        with open(sanitize(endpoint, "?f=json"), "wb") as f:
            f.write(
                b"""
                {"currentVersion":10.22,"id":1,"name":"QGIS Test","type":"Feature Layer","description":
                "QGIS Provider Test Layer","geometryType":"esriGeometryPoint","copyrightText":"not copyright","parentLayer":{"id":2,"name":"QGIS Tests"},"subLayers":[],
                "minScale":72225,"maxScale":0,
                "defaultVisibility":true,
                "extent":{"xmin":-71.123,"ymin":66.33,"xmax":-65.32,"ymax":78.3,
                "spatialReference":{"wkid":4326,"latestWkid":4326}},
                "hasAttachments":false,"htmlPopupType":"esriServerHTMLPopupTypeAsHTMLText",
                "displayField":"LABEL","typeIdField":null,
                "fields":[{"name":"OBJECTID","type":"esriFieldTypeOID","alias":"OBJECTID","domain":null}],
                "relationships":[],"canModifyLayer":false,"canScaleSymbols":false,"hasLabels":false,
                "capabilities":"Map,Query,Data,Create","maxRecordCount":1000,"supportsStatistics":true,
                "supportsAdvancedQueries":true,"supportedQueryFormats":"JSON, AMF",
                "ownershipBasedAccessControlForFeatures":{"allowOthersToQuery":true},"useStandardizedQueries":true}"""
            )

        with open(
            sanitize(endpoint, "/query?f=json_where=1=1&returnIdsOnly=true"), "wb"
        ) as f:
            f.write(
                b"""
                {
                 "objectIdFieldName": "OBJECTID",
                 "objectIds": [
                  1
                 ]
                }
                """
            )

        add_endpoint = sanitize(endpoint, "/addFeatures")
        with open(add_endpoint, "wb") as f:
            f.write(
                b"""{
  "addResults": [
    {
      "objectId": 617,
      "success": true
    }
  ]
}"""
            )

        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' crs='epsg:4326'",
            "test",
            "arcgisfeatureserver",
        )
        self.assertTrue(vl.isValid())

        f = QgsFeature()
        f.setFields(vl.fields())
        f.setAttributes([11])
        res, f = vl.dataProvider().addFeatures([f])
        self.assertTrue(res)

        with open(add_endpoint + "_payload") as f:
            res = "\n".join(f.readlines())
            self.assertEqual(
                res,
                'f=json&features=[\n\n  {\n\n    "attributes": {\n\n      "OBJECTID": 11\n\n    }\n\n  }\n\n]',
            )

        # add empty list, should return true for consistency
        self.assertTrue(vl.dataProvider().addFeatures([]))

    def testAddFail(self):
        # add capability
        endpoint = self.basetestpath + "/delete_test_fake_qgis_http_endpoint"
        with open(sanitize(endpoint, "?f=json"), "wb") as f:
            f.write(
                b"""
                {"currentVersion":10.22,"id":1,"name":"QGIS Test","type":"Feature Layer","description":
                "QGIS Provider Test Layer","geometryType":"esriGeometryPoint","copyrightText":"not copyright","parentLayer":{"id":2,"name":"QGIS Tests"},"subLayers":[],
                "minScale":72225,"maxScale":0,
                "defaultVisibility":true,
                "extent":{"xmin":-71.123,"ymin":66.33,"xmax":-65.32,"ymax":78.3,
                "spatialReference":{"wkid":4326,"latestWkid":4326}},
                "hasAttachments":false,"htmlPopupType":"esriServerHTMLPopupTypeAsHTMLText",
                "displayField":"LABEL","typeIdField":null,
                "fields":[{"name":"OBJECTID","type":"esriFieldTypeOID","alias":"OBJECTID","domain":null}],
                "relationships":[],"canModifyLayer":false,"canScaleSymbols":false,"hasLabels":false,
                "capabilities":"Map,Query,Data,Create","maxRecordCount":1000,"supportsStatistics":true,
                "supportsAdvancedQueries":true,"supportedQueryFormats":"JSON, AMF",
                "ownershipBasedAccessControlForFeatures":{"allowOthersToQuery":true},"useStandardizedQueries":true}"""
            )

        with open(
            sanitize(endpoint, "/query?f=json_where=1=1&returnIdsOnly=true"), "wb"
        ) as f:
            f.write(
                b"""
                {
                 "objectIdFieldName": "OBJECTID",
                 "objectIds": [
                  1
                 ]
                }
                """
            )

        add_endpoint = sanitize(endpoint, "/addFeatures")
        with open(add_endpoint, "wb") as f:
            f.write(
                b"""{
  "addResults": [
    {
      "success": false,
      "error": {
        "code": -2147217395,
        "description": "Setting of Value for depth failed."
      }
    }
  ]
}"""
            )

        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' crs='epsg:4326'",
            "test",
            "arcgisfeatureserver",
        )
        self.assertTrue(vl.isValid())

        f = QgsFeature()
        f.setFields(vl.fields())
        f.setAttributes([11])
        res, f = vl.dataProvider().addFeatures([f])
        self.assertFalse(res)
        self.assertEqual(
            vl.dataProvider().lastError(),
            "Error while adding features: Setting of Value for depth failed.",
        )

        with open(add_endpoint + "_payload") as f:
            res = "\n".join(f.readlines())
            self.assertEqual(
                res,
                'f=json&features=[\n\n  {\n\n    "attributes": {\n\n      "OBJECTID": 11\n\n    }\n\n  }\n\n]',
            )

    def testChangeAttributeValuesSuccess(self):
        # add capability
        endpoint = self.basetestpath + "/change_attr_test_fake_qgis_http_endpoint"
        with open(sanitize(endpoint, "?f=json"), "wb") as f:
            f.write(
                b"""
                {"currentVersion":10.22,"id":1,"name":"QGIS Test","type":"Feature Layer","description":
                "QGIS Provider Test Layer","geometryType":"esriGeometryPoint","copyrightText":"not copyright","parentLayer":{"id":2,"name":"QGIS Tests"},"subLayers":[],
                "minScale":72225,"maxScale":0,
                "defaultVisibility":true,
                "extent":{"xmin":-71.123,"ymin":66.33,"xmax":-65.32,"ymax":78.3,
                "spatialReference":{"wkid":4326,"latestWkid":4326}},
                "hasAttachments":false,"htmlPopupType":"esriServerHTMLPopupTypeAsHTMLText",
                "displayField":"LABEL","typeIdField":null,
                "fields":[{"name":"OBJECTID","type":"esriFieldTypeOID","alias":"OBJECTID","domain":null},
                          {"name":"name","type":"esriFieldTypeString","alias":"name","length":100,"domain":null},
                          {"name":"name2","type":"esriFieldTypeString","alias":"name2","length":100,"domain":null},
                          {"name":"name3","type":"esriFieldTypeString","alias":"name2","length":100,"domain":null}
                ],
                "relationships":[],"canModifyLayer":false,"canScaleSymbols":false,"hasLabels":false,
                "capabilities":"Map,Query,Data,Create,Update","maxRecordCount":1000,"supportsStatistics":true,
                "supportsAdvancedQueries":true,"supportedQueryFormats":"JSON, AMF",
                "ownershipBasedAccessControlForFeatures":{"allowOthersToQuery":true},"useStandardizedQueries":true}"""
            )

        with open(
            sanitize(endpoint, "/query?f=json_where=1=1&returnIdsOnly=true"), "wb"
        ) as f:
            f.write(
                b"""
                {
                 "objectIdFieldName": "OBJECTID",
                 "objectIds": [
                  1
                 ]
                }
                """
            )

        with open(
            sanitize(
                endpoint,
                "/query?f=json&objectIds=1&inSR=4326&outSR=4326&returnGeometry=true&outFields=*&returnM=false&returnZ=false",
            ),
            "wb",
        ) as f:
            f.write(
                b"""
        {
         "displayFieldName": "name",
         "fieldAliases": {
          "name": "name"
         },
         "geometryType": "esriGeometryPoint",
         "spatialReference": {
          "wkid": 4326,
          "latestWkid": 4326
         },
         "fields":[{"name":"OBJECTID","type":"esriFieldTypeOID","alias":"OBJECTID","domain":null},
        {"name":"name","type":"esriFieldTypeString","alias":"name","length":100,"domain":null},
        {"name":"name3","type":"esriFieldTypeString","alias":"name2","length":100,"domain":null},
        {"name":"name2","type":"esriFieldTypeString","alias":"name2","length":100,"domain":null},
        {"name":"Shape","type":"esriFieldTypeGeometry","alias":"Shape","domain":null}],
         "features": [
          {
           "attributes": {
            "OBJECTID": 1,
            "name": "name1",
            "name2":"name2",
            "name3":"name3"
           },
           "geometry": {
            "x": -71.123,
            "y": 78.23
           }
          }
         ]
        }"""
            )

        add_endpoint = sanitize(endpoint, "/updateFeatures")
        with open(add_endpoint, "wb") as f:
            f.write(
                b"""{
  "addResults": [
    {
      "objectId": 617,
      "success": true
    }
  ]
}"""
            )

        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' crs='epsg:4326'",
            "test",
            "arcgisfeatureserver",
        )
        self.assertTrue(vl.isValid())

        res = vl.dataProvider().changeAttributeValues({0: {1: "xxname", 2: "xxname2"}})
        self.assertTrue(res)

        with open(add_endpoint + "_payload") as f:
            res = "\n".join(f.readlines())
            self.assertEqual(
                res,
                'f=json&features=[\n\n  {\n\n    "attributes": {\n\n      "OBJECTID": 1,\n\n      "name": "xxname",\n\n      "name2": "xxname2",\n\n      "name3": "name3"\n\n    }\n\n  }\n\n]',
            )

    def testChangeGeometriesSuccess(self):
        # add capability
        endpoint = self.basetestpath + "/change_geom_test_fake_qgis_http_endpoint"
        with open(sanitize(endpoint, "?f=json"), "wb") as f:
            f.write(
                b"""
                {"currentVersion":10.22,"id":1,"name":"QGIS Test","type":"Feature Layer","description":
                "QGIS Provider Test Layer","geometryType":"esriGeometryPoint","copyrightText":"not copyright","parentLayer":{"id":2,"name":"QGIS Tests"},"subLayers":[],
                "minScale":72225,"maxScale":0,
                "defaultVisibility":true,
                "extent":{"xmin":-71.123,"ymin":66.33,"xmax":-65.32,"ymax":78.3,
                "spatialReference":{"wkid":4326,"latestWkid":4326}},
                "hasAttachments":false,"htmlPopupType":"esriServerHTMLPopupTypeAsHTMLText",
                "displayField":"LABEL","typeIdField":null,
                "fields":[{"name":"OBJECTID","type":"esriFieldTypeOID","alias":"OBJECTID","domain":null},
                          {"name":"name","type":"esriFieldTypeString","alias":"name","length":100,"domain":null},
                          {"name":"name2","type":"esriFieldTypeString","alias":"name2","length":100,"domain":null},
                          {"name":"name3","type":"esriFieldTypeString","alias":"name2","length":100,"domain":null}
                ],
                "relationships":[],"canModifyLayer":false,"canScaleSymbols":false,"hasLabels":false,
                "capabilities":"Map,Query,Data,Create,Update","maxRecordCount":1000,"supportsStatistics":true,
                "supportsAdvancedQueries":true,"supportedQueryFormats":"JSON, AMF",
                "ownershipBasedAccessControlForFeatures":{"allowOthersToQuery":true},"useStandardizedQueries":true}"""
            )

        with open(
            sanitize(endpoint, "/query?f=json_where=1=1&returnIdsOnly=true"), "wb"
        ) as f:
            f.write(
                b"""
                {
                 "objectIdFieldName": "OBJECTID",
                 "objectIds": [
                  1
                 ]
                }
                """
            )

        with open(
            sanitize(
                endpoint,
                "/query?f=json&objectIds=1&inSR=4326&outSR=4326&returnGeometry=true&outFields=*&returnM=false&returnZ=false",
            ),
            "wb",
        ) as f:
            f.write(
                b"""
        {
         "displayFieldName": "name",
         "fieldAliases": {
          "name": "name"
         },
         "geometryType": "esriGeometryPoint",
         "spatialReference": {
          "wkid": 4326,
          "latestWkid": 4326
         },
         "fields":[{"name":"OBJECTID","type":"esriFieldTypeOID","alias":"OBJECTID","domain":null},
        {"name":"name","type":"esriFieldTypeString","alias":"name","length":100,"domain":null},
        {"name":"name3","type":"esriFieldTypeString","alias":"name2","length":100,"domain":null},
        {"name":"name2","type":"esriFieldTypeString","alias":"name2","length":100,"domain":null},
        {"name":"Shape","type":"esriFieldTypeGeometry","alias":"Shape","domain":null}],
         "features": [
          {
           "attributes": {
            "OBJECTID": 1,
            "name": "name1",
            "name2":"name2",
            "name3":"name3"
           },
           "geometry": {
            "x": -71.123,
            "y": 78.23
           }
          }
         ]
        }"""
            )

        add_endpoint = sanitize(endpoint, "/updateFeatures")
        with open(add_endpoint, "wb") as f:
            f.write(
                b"""{
  "addResults": [
    {
      "objectId": 617,
      "success": true
    }
  ]
}"""
            )

        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' crs='epsg:4326'",
            "test",
            "arcgisfeatureserver",
        )
        self.assertTrue(vl.isValid())

        res = vl.dataProvider().changeGeometryValues(
            {0: QgsGeometry.fromWkt("Point( 111 222)")}
        )
        self.assertTrue(res)

        with open(add_endpoint + "_payload") as f:
            res = "\n".join(f.readlines())
            self.assertEqual(
                res,
                'f=json&features=[\n\n  {\n\n    "attributes": {\n\n      "OBJECTID": 1\n\n    },\n\n    "geometry": {\n\n      "x": 111.0,\n\n      "y": 222.0\n\n    }\n\n  }\n\n]',
            )

    def testChangeFeaturesSuccess(self):
        # add capability
        endpoint = self.basetestpath + "/change_geom_test_fake_qgis_http_endpoint"
        with open(sanitize(endpoint, "?f=json"), "wb") as f:
            f.write(
                b"""
                {"currentVersion":10.22,"id":1,"name":"QGIS Test","type":"Feature Layer","description":
                "QGIS Provider Test Layer","geometryType":"esriGeometryPoint","copyrightText":"not copyright","parentLayer":{"id":2,"name":"QGIS Tests"},"subLayers":[],
                "minScale":72225,"maxScale":0,
                "defaultVisibility":true,
                "extent":{"xmin":-71.123,"ymin":66.33,"xmax":-65.32,"ymax":78.3,
                "spatialReference":{"wkid":4326,"latestWkid":4326}},
                "hasAttachments":false,"htmlPopupType":"esriServerHTMLPopupTypeAsHTMLText",
                "displayField":"LABEL","typeIdField":null,
                "fields":[{"name":"OBJECTID","type":"esriFieldTypeOID","alias":"OBJECTID","domain":null},
                          {"name":"name","type":"esriFieldTypeString","alias":"name","length":100,"domain":null},
                          {"name":"name2","type":"esriFieldTypeString","alias":"name2","length":100,"domain":null},
                          {"name":"name3","type":"esriFieldTypeString","alias":"name2","length":100,"domain":null}
                ],
                "relationships":[],"canModifyLayer":false,"canScaleSymbols":false,"hasLabels":false,
                "capabilities":"Map,Query,Data,Create,Update","maxRecordCount":1000,"supportsStatistics":true,
                "supportsAdvancedQueries":true,"supportedQueryFormats":"JSON, AMF",
                "ownershipBasedAccessControlForFeatures":{"allowOthersToQuery":true},"useStandardizedQueries":true}"""
            )

        with open(
            sanitize(endpoint, "/query?f=json_where=1=1&returnIdsOnly=true"), "wb"
        ) as f:
            f.write(
                b"""
                {
                 "objectIdFieldName": "OBJECTID",
                 "objectIds": [
                  1,
                  2
                 ]
                }
                """
            )

        with open(
            sanitize(
                endpoint,
                "/query?f=json&objectIds=1,2&inSR=4326&outSR=4326&returnGeometry=true&outFields=*&returnM=false&returnZ=false",
            ),
            "wb",
        ) as f:
            f.write(
                b"""
        {
         "displayFieldName": "name",
         "fieldAliases": {
          "name": "name"
         },
         "geometryType": "esriGeometryPoint",
         "spatialReference": {
          "wkid": 4326,
          "latestWkid": 4326
         },
         "fields":[{"name":"OBJECTID","type":"esriFieldTypeOID","alias":"OBJECTID","domain":null},
        {"name":"name","type":"esriFieldTypeString","alias":"name","length":100,"domain":null},
        {"name":"name3","type":"esriFieldTypeString","alias":"name2","length":100,"domain":null},
        {"name":"name2","type":"esriFieldTypeString","alias":"name2","length":100,"domain":null},
        {"name":"Shape","type":"esriFieldTypeGeometry","alias":"Shape","domain":null}],
         "features": [
          {
           "attributes": {
            "OBJECTID": 1,
            "name": "name1",
            "name2":"name2",
            "name3":"name3"
           },
           "geometry": {
            "x": -71.123,
            "y": 78.23
           }},
           {
           "attributes": {
            "OBJECTID": 2,
            "name": "bname1",
            "name2":"bname2",
            "name3":"bname3"
           },
           "geometry": {
            "x": -11.123,
            "y": 18.23
           }
          }
         ]
        }"""
            )

        add_endpoint = sanitize(endpoint, "/updateFeatures")
        with open(add_endpoint, "wb") as f:
            f.write(
                b"""{
  "addResults": [
    {
      "objectId": 617,
      "success": true
    }
  ]
}"""
            )

        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' crs='epsg:4326'",
            "test",
            "arcgisfeatureserver",
        )
        self.assertTrue(vl.isValid())

        res = vl.dataProvider().changeFeatures(
            {1: {1: "bname1_x", 3: "bname3_x"}},
            {0: QgsGeometry.fromWkt("Point( 111 222)")},
        )
        self.assertTrue(res)

        with open(add_endpoint + "_payload") as f:
            res = "\n".join(f.readlines())
            self.assertEqual(
                res,
                'f=json&features=[\n\n  {\n\n    "attributes": {\n\n      "OBJECTID": 1,\n\n      "name": "name1",\n\n      "name2": "name2",\n\n      "name3": "name3"\n\n    },\n\n    "geometry": {\n\n      "x": 111.0,\n\n      "y": 222.0\n\n    }\n\n  },\n\n  {\n\n    "attributes": {\n\n      "OBJECTID": 2,\n\n      "name": "bname1_x",\n\n      "name2": "bname2",\n\n      "name3": "bname3_x"\n\n    },\n\n    "geometry": {\n\n      "x": -11.123,\n\n      "y": 18.23\n\n    }\n\n  }\n\n]',
            )


if __name__ == "__main__":
    unittest.main()
