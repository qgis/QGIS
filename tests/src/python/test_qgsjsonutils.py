# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsJSONUtils.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '3/05/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.testing import unittest, start_app
from qgis.core import QgsJSONUtils, QgsFeature, QgsField, QgsFields, QgsWKBTypes, QgsGeometry, QgsPointV2, QgsLineStringV2, NULL
from qgis.PyQt.QtCore import QVariant, QTextCodec

start_app()
codec = QTextCodec.codecForName("System")


class TestQgsJSONUtils(unittest.TestCase):

    def testStringToFeatureList(self):
        """Test converting json string to features"""

        fields = QgsFields()
        fields.append(QgsField("name", QVariant.String))

        # empty string
        features = QgsJSONUtils.stringToFeatureList("", fields, codec)
        self.assertEqual(features, [])

        # bad string
        features = QgsJSONUtils.stringToFeatureList("asdasdas", fields, codec)
        self.assertEqual(features, [])

        # geojson string with 1 feature
        features = QgsJSONUtils.stringToFeatureList('{\n"type": "Feature","geometry": {"type": "Point","coordinates": [125, 10]},"properties": {"name": "Dinagat Islands"}}', fields, codec)
        self.assertEqual(len(features), 1)
        self.assertFalse(features[0].constGeometry().isEmpty())
        self.assertEqual(features[0].constGeometry().wkbType(), QgsWKBTypes.Point)
        point = features[0].constGeometry().geometry()
        self.assertEqual(point.x(), 125.0)
        self.assertEqual(point.y(), 10.0)
        self.assertEqual(features[0]['name'], "Dinagat Islands")

        # geojson string with 2 features
        features = QgsJSONUtils.stringToFeatureList('{ "type": "FeatureCollection","features":[{\n"type": "Feature","geometry": {"type": "Point","coordinates": [125, 10]},"properties": {"name": "Dinagat Islands"}}, {\n"type": "Feature","geometry": {"type": "Point","coordinates": [110, 20]},"properties": {"name": "Henry Gale Island"}}]}', fields, codec)
        self.assertEqual(len(features), 2)
        self.assertFalse(features[0].constGeometry().isEmpty())
        self.assertEqual(features[0].constGeometry().wkbType(), QgsWKBTypes.Point)
        point = features[0].constGeometry().geometry()
        self.assertEqual(point.x(), 125.0)
        self.assertEqual(point.y(), 10.0)
        self.assertEqual(features[0]['name'], "Dinagat Islands")
        self.assertFalse(features[1].constGeometry().isEmpty())
        self.assertEqual(features[1].constGeometry().wkbType(), QgsWKBTypes.Point)
        point = features[1].constGeometry().geometry()
        self.assertEqual(point.x(), 110.0)
        self.assertEqual(point.y(), 20.0)
        self.assertEqual(features[1]['name'], "Henry Gale Island")

    def testStringToFields(self):
        """test retrieving fields from GeoJSON strings"""

        # empty string
        fields = QgsJSONUtils.stringToFields("", codec)
        self.assertEqual(fields.count(), 0)

        # bad string
        fields = QgsJSONUtils.stringToFields("asdasdas", codec)
        self.assertEqual(fields.count(), 0)

        # geojson string
        fields = QgsJSONUtils.stringToFields('{\n"type": "Feature","geometry": {"type": "Point","coordinates": [125, 10]},"properties": {"name": "Dinagat Islands","height":5.5}}', codec)
        self.assertEqual(fields.count(), 2)
        self.assertEqual(fields[0].name(), "name")
        self.assertEqual(fields[0].type(), QVariant.String)
        self.assertEqual(fields[1].name(), "height")
        self.assertEqual(fields[1].type(), QVariant.Double)

    def testEncodeValue(self):
        """ test encoding various values for use in GeoJSON strings """
        self.assertEqual(QgsJSONUtils.encodeValue(NULL), 'null')
        self.assertEqual(QgsJSONUtils.encodeValue(5), '5')
        self.assertEqual(QgsJSONUtils.encodeValue(5.9), '5.9')
        self.assertEqual(QgsJSONUtils.encodeValue(5999999999), '5999999999')
        self.assertEqual(QgsJSONUtils.encodeValue('string'), '"string"')
        self.assertEqual(QgsJSONUtils.encodeValue('str\ning'), '"str\\ning"')
        self.assertEqual(QgsJSONUtils.encodeValue('str\ring'), '"str\\ring"')
        self.assertEqual(QgsJSONUtils.encodeValue('str"ing'), '"str\\"ing"')

    def testFeatureToGeoJSON(self):
        """ test converting features to GeoJSON """
        fields = QgsFields()
        fields.append(QgsField("name", QVariant.String))
        fields.append(QgsField("cost", QVariant.Double))
        fields.append(QgsField("population", QVariant.Int))

        feature = QgsFeature(fields, 5)
        feature.setGeometry(QgsGeometry(QgsPointV2(5, 6)))
        feature.setAttributes(['Valsier Peninsula', 6.8, 198])

        expected = """{
   "type":"Feature",
   "id":5,
   "geometry":
   {"type": "Point", "coordinates": [5, 6]},
   "properties":{
      "name":"Valsier Peninsula",
      "cost":6.8,
      "population":198
   }
}"""
        self.assertEqual(QgsJSONUtils.featureToGeoJSON(feature), expected)

        # test with linestring for bbox inclusion
        l = QgsLineStringV2()
        l.setPoints([QgsPointV2(5, 6), QgsPointV2(15, 16)])
        feature.setGeometry(QgsGeometry(QgsLineStringV2(l)))

        expected = """{
   "type":"Feature",
   "id":5,
   "bbox":[5, 6, 15, 16],
   "geometry":
   {"type": "LineString", "coordinates": [ [5, 6], [15, 16]]},
   "properties":{
      "name":"Valsier Peninsula",
      "cost":6.8,
      "population":198
   }
}"""
        self.assertEqual(QgsJSONUtils.featureToGeoJSON(feature), expected)

        # test that precision is respected
        feature.setGeometry(QgsGeometry(QgsPointV2(5.444444444, 6.333333333)))
        expected = """{
   "type":"Feature",
   "id":5,
   "geometry":
   {"type": "Point", "coordinates": [5.444, 6.333]},
   "properties":{
      "name":"Valsier Peninsula",
      "cost":6.8,
      "population":198
   }
}"""
        self.assertEqual(QgsJSONUtils.featureToGeoJSON(feature, 3), expected)
        feature.setGeometry(QgsGeometry(QgsPointV2(5, 6)))

        # test that attribute subset is respected
        expected = """{
   "type":"Feature",
   "id":5,
   "geometry":
   {"type": "Point", "coordinates": [5, 6]},
   "properties":{
      "name":"Valsier Peninsula",
      "population":198
   }
}"""
        self.assertEqual(QgsJSONUtils.featureToGeoJSON(feature, attrIndexes=[0, 2]), expected)

        expected = """{
   "type":"Feature",
   "id":5,
   "geometry":
   {"type": "Point", "coordinates": [5, 6]},
   "properties":{
      "cost":6.8
   }
}"""
        self.assertEqual(QgsJSONUtils.featureToGeoJSON(feature, attrIndexes=[1]), expected)

        # test excluding geometry
        feature.setGeometry(QgsGeometry(QgsLineStringV2(l)))

        expected = """{
   "type":"Feature",
   "id":5,
   "properties":{
      "name":"Valsier Peninsula",
      "cost":6.8,
      "population":198
   }
}"""
        self.assertEqual(QgsJSONUtils.featureToGeoJSON(feature, includeGeom=False), expected)
        feature.setGeometry(QgsGeometry(QgsPointV2(5, 6)))

        # test excluding attributes
        expected = """{
   "type":"Feature",
   "id":5,
   "geometry":
   {"type": "Point", "coordinates": [5, 6]}
}"""
        self.assertEqual(QgsJSONUtils.featureToGeoJSON(feature, includeAttributes=False), expected)

        expected = """{
   "type":"Feature",
   "id":5
}"""
        self.assertEqual(QgsJSONUtils.featureToGeoJSON(feature, includeGeom=False, includeAttributes=False), expected)

        # test overriding ID
        expected = """{
   "type":"Feature",
   "id":29,
   "properties":{
      "name":"Valsier Peninsula",
      "cost":6.8,
      "population":198
   }
}"""
        self.assertEqual(QgsJSONUtils.featureToGeoJSON(feature, includeGeom=False, id=29), expected)

if __name__ == "__main__":
    unittest.main()
