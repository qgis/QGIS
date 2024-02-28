"""QGIS Unit tests for QgsJsonUtils.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '3/05/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'

from qgis.PyQt.QtCore import QT_VERSION_STR, QLocale, Qt, QVariant
from qgis.core import (
    NULL,
    QgsCoordinateReferenceSystem,
    QgsEditorWidgetSetup,
    QgsFeature,
    QgsField,
    QgsFields,
    QgsGeometry,
    QgsJsonExporter,
    QgsJsonUtils,
    QgsLineString,
    QgsPoint,
    QgsProject,
    QgsRelation,
    QgsVectorLayer,
    QgsWkbTypes,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()

if int(QT_VERSION_STR.split('.')[0]) < 6:
    from qgis.PyQt.QtCore import QTextCodec
    codec = QTextCodec.codecForName("System")


class TestQgsJsonUtils(QgisTestCase):
    def testStringToFeatureList(self):
        """Test converting json string to features"""

        fields = QgsFields()
        fields.append(QgsField("name", QVariant.String))

        # empty string
        if int(QT_VERSION_STR.split('.')[0]) >= 6:
            features = QgsJsonUtils.stringToFeatureList("", fields)
        else:
            features = QgsJsonUtils.stringToFeatureList("", fields, codec)
        self.assertEqual(features, [])

        # bad string
        if int(QT_VERSION_STR.split('.')[0]) >= 6:
            features = QgsJsonUtils.stringToFeatureList("asdasdas", fields)
        else:
            features = QgsJsonUtils.stringToFeatureList("asdasdas", fields, codec)
        self.assertEqual(features, [])

        # geojson string with 1 feature
        s = '{\n"type": "Feature","geometry": {"type": "Point","coordinates": [125, 10]},"properties": {"name": "Dinagat Islands"}}'
        if int(QT_VERSION_STR.split('.')[0]) >= 6:
            features = QgsJsonUtils.stringToFeatureList(s, fields)
        else:
            features = QgsJsonUtils.stringToFeatureList(s, fields, codec)
        self.assertEqual(len(features), 1)
        self.assertFalse(features[0].geometry().isNull())
        self.assertEqual(features[0].geometry().wkbType(), QgsWkbTypes.Type.Point)
        point = features[0].geometry().constGet()
        self.assertEqual(point.x(), 125.0)
        self.assertEqual(point.y(), 10.0)
        self.assertEqual(features[0]['name'], "Dinagat Islands")

        # geojson string with 2 features
        s = '{ "type": "FeatureCollection","features":[{\n"type": "Feature","geometry": {"type": "Point","coordinates": [125, 10]},"properties": {"name": "Dinagat Islands"}}, {\n"type": "Feature","geometry": {"type": "Point","coordinates": [110, 20]},"properties": {"name": "Henry Gale Island"}}]}'
        if int(QT_VERSION_STR.split('.')[0]) >= 6:
            features = QgsJsonUtils.stringToFeatureList(s, fields)
        else:
            features = QgsJsonUtils.stringToFeatureList(s, fields, codec)
        self.assertEqual(len(features), 2)
        self.assertFalse(features[0].geometry().isNull())
        self.assertEqual(features[0].geometry().wkbType(), QgsWkbTypes.Type.Point)
        point = features[0].geometry().constGet()
        self.assertEqual(point.x(), 125.0)
        self.assertEqual(point.y(), 10.0)
        self.assertEqual(features[0]['name'], "Dinagat Islands")
        self.assertFalse(features[1].geometry().isNull())
        self.assertEqual(features[1].geometry().wkbType(), QgsWkbTypes.Type.Point)
        point = features[1].geometry().constGet()
        self.assertEqual(point.x(), 110.0)
        self.assertEqual(point.y(), 20.0)
        self.assertEqual(features[1]['name'], "Henry Gale Island")

    def testStringToFeatureListWithDatetimeProperty_regression44160(self):
        """Test that milliseconds and time zone information is parsed from datetime properties"""
        fields = QgsFields()
        fields.append(QgsField("some_time_field", QVariant.DateTime))

        date = "2020-01-01T"

        def geojson_with_time(timepart):
            return '{"type": "Feature","geometry": {"type": "Point","coordinates": [0,0]},"properties": {"some_time_field": "' + date + timepart + '"}}'

        # No milliseconds
        features = QgsJsonUtils.stringToFeatureList(geojson_with_time('22:00:10'), fields)
        self.assertEqual(len(features), 1)
        self.assertEqual(features[0]['some_time_field'].toString(Qt.DateFormat.ISODateWithMs), f"{date}22:00:10.000")

        # milliseconds
        features = QgsJsonUtils.stringToFeatureList(geojson_with_time('22:00:10.123'), fields)
        self.assertEqual(len(features), 1)
        self.assertEqual(features[0]['some_time_field'].toString(Qt.DateFormat.ISODateWithMs), f"{date}22:00:10.123")

    def testStringToFeatureListWithTimeProperty_regression44160(self):
        """Test that milliseconds and time zone information is parsed from time properties"""
        fields = QgsFields()
        fields.append(QgsField("some_time_field", QVariant.Time))

        def geojson_with_time(timepart):
            return '{"type": "Feature","geometry": {"type": "Point","coordinates": [0,0]},"properties": {"some_time_field": "' + timepart + '"}}'

        # No milliseconds
        features = QgsJsonUtils.stringToFeatureList(geojson_with_time('22:00:10'), fields)
        self.assertEqual(len(features), 1)
        self.assertEqual(features[0]['some_time_field'].toString(Qt.DateFormat.ISODateWithMs), "22:00:10.000")

        # milliseconds
        features = QgsJsonUtils.stringToFeatureList(geojson_with_time('22:00:10.123'), fields)
        self.assertEqual(len(features), 1)
        self.assertEqual(features[0]['some_time_field'].toString(Qt.DateFormat.ISODateWithMs), "22:00:10.123")

    def testStringToFields(self):
        """test retrieving fields from GeoJSON strings"""

        # empty string
        if int(QT_VERSION_STR.split('.')[0]) >= 6:
            fields = QgsJsonUtils.stringToFields("")
        else:
            fields = QgsJsonUtils.stringToFields("", codec)
        self.assertEqual(fields.count(), 0)

        # bad string
        if int(QT_VERSION_STR.split('.')[0]) >= 6:
            fields = QgsJsonUtils.stringToFields("asdasdas")
        else:
            fields = QgsJsonUtils.stringToFields("asdasdas", codec)
        self.assertEqual(fields.count(), 0)

        # geojson string
        s = '{\n"type": "Feature","geometry": {"type": "Point","coordinates": [125, 10]},"properties": {"name": "Dinagat Islands","height":5.5}}'
        if int(QT_VERSION_STR.split('.')[0]) >= 6:
            fields = QgsJsonUtils.stringToFields(s)
        else:
            fields = QgsJsonUtils.stringToFields(s, codec)
        self.assertEqual(fields.count(), 2)
        self.assertEqual(fields[0].name(), "name")
        self.assertEqual(fields[0].type(), QVariant.String)
        self.assertEqual(fields[1].name(), "height")
        self.assertEqual(fields[1].type(), QVariant.Double)

        # geojson string with 2 features
        s = '{ "type": "FeatureCollection","features":[{\n"type": "Feature","geometry": {"type": "Point","coordinates": [125, 10]},"properties": {"name": "Dinagat Islands","height":5.5}}, {\n"type": "Feature","geometry": {"type": "Point","coordinates": [110, 20]},"properties": {"name": "Henry Gale Island","height":6.5}}]}'
        if int(QT_VERSION_STR.split('.')[0]) >= 6:
            fields = QgsJsonUtils.stringToFields(s)
        else:
            fields = QgsJsonUtils.stringToFields(s, codec)
        self.assertEqual(fields.count(), 2)
        self.assertEqual(fields[0].name(), "name")
        self.assertEqual(fields[0].type(), QVariant.String)
        self.assertEqual(fields[1].name(), "height")
        self.assertEqual(fields[1].type(), QVariant.Double)

    def testEncodeValue(self):
        """ test encoding various values for use in GeoJSON strings """
        self.assertEqual(QgsJsonUtils.encodeValue(NULL), 'null')
        self.assertEqual(QgsJsonUtils.encodeValue(5), '5')
        self.assertEqual(QgsJsonUtils.encodeValue(5.9), '5.9')
        self.assertEqual(QgsJsonUtils.encodeValue(5999999999), '5999999999')
        self.assertEqual(QgsJsonUtils.encodeValue('string'), '"string"')
        self.assertEqual(QgsJsonUtils.encodeValue('str\ning'), '"str\\ning"')
        self.assertEqual(QgsJsonUtils.encodeValue('str\ring'), '"str\\ring"')
        self.assertEqual(QgsJsonUtils.encodeValue('str\bing'), '"str\\bing"')
        self.assertEqual(QgsJsonUtils.encodeValue('str\ting'), '"str\\ting"')
        self.assertEqual(QgsJsonUtils.encodeValue('str\\ing'), '"str\\\\ing"')
        self.assertEqual(QgsJsonUtils.encodeValue('str\\ning'), '"str\\\\ning"')
        self.assertEqual(QgsJsonUtils.encodeValue('str\n\\\\ing'), '"str\\n\\\\\\\\ing"')
        self.assertEqual(QgsJsonUtils.encodeValue('str/ing'), '"str\\/ing"')
        self.assertEqual(QgsJsonUtils.encodeValue([5, 6]), '[5,6]')
        self.assertEqual(QgsJsonUtils.encodeValue(['a', 'b', 'c']), '["a","b","c"]')
        self.assertEqual(QgsJsonUtils.encodeValue(['a', 3, 'c']), '["a",3,"c"]')
        self.assertEqual(QgsJsonUtils.encodeValue(['a', 'c\nd']), '["a","c\\nd"]')
        # handle differences due to Qt5 version, where compact output now lacks \n
        enc_str = QgsJsonUtils.encodeValue({'key': 'value', 'key2': 5})
        self.assertTrue(enc_str == '{"key":"value",\n"key2":5}' or enc_str == '{"key":"value","key2":5}')
        enc_str = QgsJsonUtils.encodeValue({'key': [1, 2, 3], 'key2': {'nested': 'nested\\result'}})
        self.assertTrue(
            enc_str == '{"key":[1,2,3],\n"key2":{"nested":"nested\\\\result"}}' or enc_str == '{"key":[1,2,3],"key2":{"nested":"nested\\\\result"}}')

    def testExportAttributes(self):
        """ test exporting feature's attributes to JSON object """
        fields = QgsFields()

        # test empty attributes
        feature = QgsFeature(fields, 5)
        expected = "{}"
        self.assertEqual(QgsJsonUtils.exportAttributes(feature), expected)

        # test feature with attributes
        fields.append(QgsField("name", QVariant.String))
        fields.append(QgsField("cost", QVariant.Double))
        fields.append(QgsField("population", QVariant.Int))

        feature = QgsFeature(fields, 5)
        feature.setGeometry(QgsGeometry(QgsPoint(5, 6)))
        feature.setAttributes(['Valsier Peninsula', 6.8, 198])

        expected = """{"name":"Valsier Peninsula",
"cost":6.8,
"population":198}"""
        self.assertEqual(QgsJsonUtils.exportAttributes(feature), expected)

        # test using field formatters
        source = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer",
                                "parent", "memory")
        pf1 = QgsFeature()
        pf1.setFields(source.fields())
        pf1.setAttributes(["test1", 1])

        setup = QgsEditorWidgetSetup('ValueMap', {"map": {"one": 1, "two": 2, "three": 3}})
        source.setEditorWidgetSetup(1, setup)

        expected = """{"fldtxt":"test1",
"fldint":"one"}"""
        self.assertEqual(QgsJsonUtils.exportAttributes(pf1, source), expected)

    def testJSONExporter(self):
        """ test converting features to GeoJSON """
        fields = QgsFields()
        fields.append(QgsField("name", QVariant.String))
        fields.append(QgsField("cost", QVariant.Double))
        fields.append(QgsField("population", QVariant.Int))

        feature = QgsFeature(fields, 5)
        feature.setGeometry(QgsGeometry(QgsPoint(5, 6)))
        feature.setAttributes(['Valsier Peninsula', 6.8, 198])

        exporter = QgsJsonExporter()

        expected = """{
  "geometry": {
    "coordinates": [
      5.0,
      6.0
    ],
    "type": "Point"
  },
  "id": 5,
  "properties": {
    "cost": 6.8,
    "name": "Valsier Peninsula",
    "population": 198
  },
  "type": "Feature"
}"""
        self.assertEqual(exporter.exportFeature(feature, indent=2), expected)

        # test with linestring for bbox inclusion
        l = QgsLineString()
        l.setPoints([QgsPoint(5, 6), QgsPoint(15, 16)])
        feature.setGeometry(QgsGeometry(QgsLineString(l)))

        expected = """{
  "bbox": [
    5.0,
    6.0,
    15.0,
    16.0
  ],
  "geometry": {
    "coordinates": [
      [
        5.0,
        6.0
      ],
      [
        15.0,
        16.0
      ]
    ],
    "type": "LineString"
  },
  "id": 5,
  "properties": {
    "cost": 6.8,
    "name": "Valsier Peninsula",
    "population": 198
  },
  "type": "Feature"
}"""
        self.assertEqual(exporter.exportFeature(feature, indent=2), expected)

        # test that precision is respected
        feature.setGeometry(QgsGeometry(QgsPoint(5.444444444, 6.333333333)))
        exporter.setPrecision(3)
        self.assertEqual(exporter.precision(), 3)
        expected = """{
  "geometry": {
    "coordinates": [
      5.444,
      6.333
    ],
    "type": "Point"
  },
  "id": 5,
  "properties": {
    "cost": 6.8,
    "name": "Valsier Peninsula",
    "population": 198
  },
  "type": "Feature"
}"""
        self.assertEqual(exporter.exportFeature(feature, indent=2), expected)
        feature.setGeometry(QgsGeometry(QgsPoint(5, 6)))
        exporter.setPrecision(17)

        # test that attribute subset is respected
        exporter.setAttributes([0, 2])
        self.assertEqual(exporter.attributes(), [0, 2])
        expected = """{
  "geometry": {
    "coordinates": [
      5.0,
      6.0
    ],
    "type": "Point"
  },
  "id": 5,
  "properties": {
    "name": "Valsier Peninsula",
    "population": 198
  },
  "type": "Feature"
}"""
        self.assertEqual(exporter.exportFeature(feature, indent=2), expected)

        exporter.setAttributes([1])
        self.assertEqual(exporter.attributes(), [1])
        expected = """{
  "geometry": {
    "coordinates": [
      5.0,
      6.0
    ],
    "type": "Point"
  },
  "id": 5,
  "properties": {
    "cost": 6.8
  },
  "type": "Feature"
}"""
        self.assertEqual(exporter.exportFeature(feature, indent=2), expected)
        exporter.setAttributes([])

        # text excluding attributes

        exporter.setExcludedAttributes([1])
        self.assertEqual(exporter.excludedAttributes(), [1])
        expected = """{
  "geometry": {
    "coordinates": [
      5.0,
      6.0
    ],
    "type": "Point"
  },
  "id": 5,
  "properties": {
    "name": "Valsier Peninsula",
    "population": 198
  },
  "type": "Feature"
}"""
        self.assertEqual(exporter.exportFeature(feature, indent=2), expected)

        exporter.setExcludedAttributes([1, 2])
        self.assertEqual(exporter.excludedAttributes(), [1, 2])
        expected = """{
  "geometry": {
    "coordinates": [
      5.0,
      6.0
    ],
    "type": "Point"
  },
  "id": 5,
  "properties": {
    "name": "Valsier Peninsula"
  },
  "type": "Feature"
}"""
        self.assertEqual(exporter.exportFeature(feature, indent=2), expected)

        exporter.setExcludedAttributes([0, 1, 2])
        self.assertEqual(exporter.excludedAttributes(), [0, 1, 2])
        expected = """{
  "geometry": {
    "coordinates": [
      5.0,
      6.0
    ],
    "type": "Point"
  },
  "id": 5,
  "properties": null,
  "type": "Feature"
}"""
        self.assertEqual(exporter.exportFeature(feature, indent=2), expected)

        # test that excluded attributes take precedence over included

        exporter.setAttributes([1, 2])
        exporter.setExcludedAttributes([0, 1])
        expected = """{
  "geometry": {
    "coordinates": [
      5.0,
      6.0
    ],
    "type": "Point"
  },
  "id": 5,
  "properties": {
    "population": 198
  },
  "type": "Feature"
}"""
        self.assertEqual(exporter.exportFeature(feature, indent=2), expected)

        exporter.setAttributes([])
        exporter.setExcludedAttributes([])

        # test excluding geometry
        exporter.setIncludeGeometry(False)
        self.assertEqual(exporter.includeGeometry(), False)
        feature.setGeometry(QgsGeometry(QgsLineString(l)))

        expected = """{
  "geometry": null,
  "id": 5,
  "properties": {
    "cost": 6.8,
    "name": "Valsier Peninsula",
    "population": 198
  },
  "type": "Feature"
}"""
        self.assertEqual(exporter.exportFeature(feature, indent=2), expected)
        exporter.setIncludeGeometry(True)

        feature.setGeometry(QgsGeometry(QgsPoint(5, 6)))

        # test excluding attributes
        exporter.setIncludeAttributes(False)
        self.assertEqual(exporter.includeAttributes(), False)
        expected = """{
  "geometry": {
    "coordinates": [
      5.0,
      6.0
    ],
    "type": "Point"
  },
  "id": 5,
  "properties": null,
  "type": "Feature"
}"""
        self.assertEqual(exporter.exportFeature(feature, indent=2), expected)

        exporter.setIncludeGeometry(False)
        expected = """{
  "geometry": null,
  "id": 5,
  "properties": null,
  "type": "Feature"
}"""
        self.assertEqual(exporter.exportFeature(feature, indent=2), expected)
        exporter.setIncludeAttributes(True)

        # test overriding ID
        expected = """{
  "geometry": null,
  "id": 29,
  "properties": {
    "cost": 6.8,
    "name": "Valsier Peninsula",
    "population": 198
  },
  "type": "Feature"
}"""
        self.assertEqual(exporter.exportFeature(feature, id=29, indent=2), expected)

        expected = """{
  "geometry": null,
  "id": "mylayer.29",
  "properties": {
    "cost": 6.8,
    "name": "Valsier Peninsula",
    "population": 198
  },
  "type": "Feature"
}"""
        self.assertEqual(exporter.exportFeature(feature, id="mylayer.29", indent=2), expected)

        # test injecting extra attributes
        expected = """{
  "geometry": null,
  "id": 5,
  "properties": {
    "cost": 6.8,
    "extra": "val1",
    "extra2": 2,
    "name": "Valsier Peninsula",
    "population": 198
  },
  "type": "Feature"
}"""
        self.assertEqual(exporter.exportFeature(feature, extraProperties={"extra": "val1", "extra2": 2}, indent=2),
                         expected)

        exporter.setIncludeAttributes(False)
        expected = """{
  "geometry": null,
  "id": 5,
  "properties": {
    "extra": "val1",
    "extra2": {
      "nested_map": 5,
      "nested_map2": "val"
    },
    "extra3": [
      1,
      2,
      3
    ]
  },
  "type": "Feature"
}"""

        exp_f = exporter.exportFeature(feature, extraProperties={"extra": "val1",
                                                                 "extra2": {"nested_map": 5, "nested_map2": "val"},
                                                                 "extra3": [1, 2, 3]}, indent=2)
        self.assertEqual(exp_f, expected)
        exporter.setIncludeGeometry(True)

    def testExportFeatureFieldFormatter(self):
        """ Test exporting a feature with formatting fields """

        # source layer
        source = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer",
                                "parent", "memory")
        pr = source.dataProvider()
        pf1 = QgsFeature(123)
        pf1.setFields(source.fields())
        pf1.setAttributes(["test1", 1])
        pf2 = QgsFeature()
        pf2.setFields(source.fields())
        pf2.setAttributes(["test2", 2])
        assert pr.addFeatures([pf1, pf2])

        setup = QgsEditorWidgetSetup('ValueMap', {"map": {"one": 1, "two": 2, "three": 3}})
        source.setEditorWidgetSetup(1, setup)

        exporter = QgsJsonExporter()
        exporter.setVectorLayer(source)

        expected = """{
  "geometry": null,
  "id": 123,
  "properties": {
    "fldint": "one",
    "fldtxt": "test1"
  },
  "type": "Feature"
}"""
        self.assertEqual(exporter.exportFeature(pf1, indent=2), expected)

    def testExportFeatureCrs(self):
        """ Test CRS transform when exporting features """

        exporter = QgsJsonExporter()
        self.assertFalse(exporter.sourceCrs().isValid())

        # test layer
        layer = QgsVectorLayer("Point?crs=epsg:3111&field=fldtxt:string",
                               "parent", "memory")
        exporter = QgsJsonExporter(layer)
        self.assertTrue(exporter.sourceCrs().isValid())
        self.assertEqual(exporter.sourceCrs().authid(), 'EPSG:3111')

        exporter.setSourceCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        self.assertTrue(exporter.sourceCrs().isValid())
        self.assertEqual(exporter.sourceCrs().authid(), 'EPSG:3857')

        # vector layer CRS should override
        exporter.setVectorLayer(layer)
        self.assertEqual(exporter.sourceCrs().authid(), 'EPSG:3111')

        # test that exported feature is reprojected
        feature = QgsFeature(layer.fields(), 5)
        feature.setGeometry(QgsGeometry(QgsPoint(2502577, 2403869)))
        feature.setAttributes(['test point'])

        # low precision, only need rough coordinate to check and don't want to deal with rounding errors
        exporter.setPrecision(1)
        expected = """{
  "geometry": {
    "coordinates": [
      145.0,
      -37.9
    ],
    "type": "Point"
  },
  "id": 5,
  "properties": {
    "fldtxt": "test point"
  },
  "type": "Feature"
}"""
        self.assertEqual(exporter.exportFeature(feature, indent=2), expected)

    def testExportFeatureRelations(self):
        """ Test exporting a feature with relations """

        # parent layer
        parent = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer&field=foreignkey:integer",
                                "parent", "memory")
        pr = parent.dataProvider()
        pf1 = QgsFeature(432)
        pf1.setFields(parent.fields())
        pf1.setAttributes(["test1", 67, 123])
        pf2 = QgsFeature(876)
        pf2.setFields(parent.fields())
        pf2.setAttributes(["test2", 68, 124])
        assert pr.addFeatures([pf1, pf2])

        # child layer
        child = QgsVectorLayer(
            "Point?field=x:string&field=y:integer&field=z:integer",
            "referencedlayer", "memory")
        pr = child.dataProvider()
        f1 = QgsFeature()
        f1.setFields(child.fields())
        f1.setAttributes(["foo", 123, 321])
        f2 = QgsFeature()
        f2.setFields(child.fields())
        f2.setAttributes(["bar", 123, 654])
        f3 = QgsFeature()
        f3.setFields(child.fields())
        f3.setAttributes(["foobar", 124, 554])
        assert pr.addFeatures([f1, f2, f3])

        QgsProject.instance().addMapLayers([child, parent])

        rel = QgsRelation()
        rel.setId('rel1')
        rel.setName('relation one')
        rel.setReferencingLayer(child.id())
        rel.setReferencedLayer(parent.id())
        rel.addFieldPair('y', 'foreignkey')

        QgsProject.instance().relationManager().addRelation(rel)

        exporter = QgsJsonExporter()

        exporter.setVectorLayer(parent)
        self.assertEqual(exporter.vectorLayer(), parent)
        exporter.setIncludeRelated(True)
        self.assertEqual(exporter.includeRelated(), True)

        expected = """{
  "geometry": null,
  "id": 432,
  "properties": {
    "fldint": 67,
    "fldtxt": "test1",
    "foreignkey": 123,
    "relation one": [
      {
        "x": "foo",
        "y": 123,
        "z": 321
      },
      {
        "x": "bar",
        "y": 123,
        "z": 654
      }
    ]
  },
  "type": "Feature"
}"""
        self.assertEqual(exporter.exportFeature(pf1, indent=2), expected)

        expected = """{
  "geometry": null,
  "id": 876,
  "properties": {
    "fldint": 68,
    "fldtxt": "test2",
    "foreignkey": 124,
    "relation one": [
      {
        "x": "foobar",
        "y": 124,
        "z": 554
      }
    ]
  },
  "type": "Feature"
}"""

        self.assertEqual(exporter.exportFeature(pf2, indent=2), expected)

        # with field formatter
        setup = QgsEditorWidgetSetup('ValueMap', {"map": {"apples": 123, "bananas": 124}})
        child.setEditorWidgetSetup(1, setup)
        expected = """{
  "geometry": null,
  "id": 432,
  "properties": {
    "fldint": 67,
    "fldtxt": "test1",
    "foreignkey": 123,
    "relation one": [
      {
        "x": "foo",
        "y": "apples",
        "z": 321
      },
      {
        "x": "bar",
        "y": "apples",
        "z": 654
      }
    ]
  },
  "type": "Feature"
}"""
        self.assertEqual(exporter.exportFeature(pf1, indent=2), expected)

        # test excluding related attributes
        exporter.setIncludeRelated(False)
        self.assertEqual(exporter.includeRelated(), False)

        expected = """{
  "geometry": null,
  "id": 876,
  "properties": {
    "fldint": 68,
    "fldtxt": "test2",
    "foreignkey": 124
  },
  "type": "Feature"
}"""
        self.assertEqual(exporter.exportFeature(pf2, indent=2), expected)

        # test without vector layer set
        exporter.setIncludeRelated(True)
        exporter.setVectorLayer(None)

        expected = """{
  "geometry": null,
  "id": 876,
  "properties": {
    "fldint": 68,
    "fldtxt": "test2",
    "foreignkey": 124
  },
  "type": "Feature"
}"""
        self.assertEqual(exporter.exportFeature(pf2, indent=2), expected)

    def testExportFeatures(self):
        """ Test exporting feature collections """

        fields = QgsFields()
        fields.append(QgsField("name", QVariant.String))
        fields.append(QgsField("cost", QVariant.Double))
        fields.append(QgsField("population", QVariant.Int))

        feature = QgsFeature(fields, 5)
        feature.setGeometry(QgsGeometry(QgsPoint(5, 6)))
        feature.setAttributes(['Valsier Peninsula', 6.8, 198])

        exporter = QgsJsonExporter()

        # single feature
        expected = """{
  "features": [
    {
      "geometry": {
        "coordinates": [
          5.0,
          6.0
        ],
        "type": "Point"
      },
      "id": 5,
      "properties": {
        "cost": 6.8,
        "name": "Valsier Peninsula",
        "population": 198
      },
      "type": "Feature"
    }
  ],
  "type": "FeatureCollection"
}"""
        self.assertEqual(exporter.exportFeatures([feature], 2), expected)

        # multiple features
        feature2 = QgsFeature(fields, 6)
        feature2.setGeometry(QgsGeometry(QgsPoint(7, 8)))
        feature2.setAttributes(['Henry Gale Island', 9.7, 38])

        expected = """{
  "features": [
    {
      "geometry": {
        "coordinates": [
          5.0,
          6.0
        ],
        "type": "Point"
      },
      "id": 5,
      "properties": {
        "cost": 6.8,
        "name": "Valsier Peninsula",
        "population": 198
      },
      "type": "Feature"
    },
    {
      "geometry": {
        "coordinates": [
          7.0,
          8.0
        ],
        "type": "Point"
      },
      "id": 6,
      "properties": {
        "cost": 9.7,
        "name": "Henry Gale Island",
        "population": 38
      },
      "type": "Feature"
    }
  ],
  "type": "FeatureCollection"
}"""
        self.assertEqual(exporter.exportFeatures([feature, feature2], 2), expected)

    def testExportFeaturesWithLocale_regression20053(self):
        """ Test exporting feature export with range widgets and locale different than C
        Regression: https://github.com/qgis/QGIS/issues/27875 - decimal separator in csv files
        """

        source = QgsVectorLayer("Point?field=name:string&field=cost:double&field=population:int&field=date:date",
                                "parent", "memory")
        self.assertTrue(source.isValid())
        fields = source.fields()

        feature = QgsFeature(fields, 5)
        feature.setGeometry(QgsGeometry(QgsPoint(5, 6)))
        feature.setAttributes(['Valsier Peninsula', 6.8, 198000, '2018-09-10'])

        exporter = QgsJsonExporter()

        # single feature
        expected = """{
  "features": [
    {
      "geometry": {
        "coordinates": [
          5.0,
          6.0
        ],
        "type": "Point"
      },
      "id": 5,
      "properties": {
        "cost": 6.8,
        "date": "2018-09-10",
        "name": "Valsier Peninsula",
        "population": 198000
      },
      "type": "Feature"
    }
  ],
  "type": "FeatureCollection"
}"""
        self.assertEqual(exporter.exportFeatures([feature], 2), expected)

        setup = QgsEditorWidgetSetup('Range', {
            'AllowNull': True,
            'Max': 2147483647,
            'Min': -2147483648,
            'Precision': 4,
            'Step': 1,
            'Style': 'SpinBox'
        }
        )
        source.setEditorWidgetSetup(1, setup)
        source.setEditorWidgetSetup(2, setup)

        QLocale.setDefault(QLocale('it'))
        exporter.setVectorLayer(source)
        self.assertEqual(exporter.exportFeatures([feature], 2), expected)

    def testExportFieldAlias(self):
        """ Test exporting a feature with fields' alias """

        # source layer
        source = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer",
                                "parent", "memory")
        pr = source.dataProvider()
        pf1 = QgsFeature()
        pf1.setFields(source.fields())
        pf1.setAttributes(["test1", 1])
        pf2 = QgsFeature()
        pf2.setFields(source.fields())
        pf2.setAttributes(["test2", 2])
        result, features = pr.addFeatures([pf1, pf2])
        self.assertTrue(result)

        source.setFieldAlias(0, "alias_fldtxt")
        source.setFieldAlias(1, "alias_fldint")

        exporter = QgsJsonExporter()
        exporter.setAttributeDisplayName(True)
        exporter.setVectorLayer(source)

        expected = """{
  "geometry": null,
  "id": 1,
  "properties": {
    "alias_fldint": 1,
    "alias_fldtxt": "test1"
  },
  "type": "Feature"
}"""
        self.assertEqual(exporter.exportFeature(features[0], indent=2), expected)

    def test_geojson_invalid(self):
        res = QgsJsonUtils.geometryFromGeoJson("")
        self.assertTrue(res.isNull())
        res = QgsJsonUtils.geometryFromGeoJson("xxxx")
        self.assertTrue(res.isNull())

    def test_geojson_point(self):
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
            "type": "Point",
            "coordinates": [30.0, 10.0]
        }"""
        )
        self.assertEqual(res.asWkt(), "Point (30 10)")
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
                    "type": "Point",
                    "coordinates": [30.0, 10.0, 15.5]
                }"""
        )
        self.assertEqual(res.asWkt(), "PointZ (30 10 15.5)")
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
                    "type": "Point",
                    "coordinates": [30.0]
                }"""
        )
        self.assertTrue(res.isNull())
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
                    "type": "Point",
                    "coordinates": 3
                }"""
        )
        self.assertTrue(res.isNull())
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
                    "type": "Point",
                    "coordinates": [1,2,3,4]
                }"""
        )
        self.assertTrue(res.isNull())
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
                    "type": "Point"
                }"""
        )
        self.assertTrue(res.isNull())

    def test_geojson_multipoint(self):
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
            "type": "MultiPoint",
            "coordinates": [[10.0, 40.0],
            [40.0, 30.0],
            [20.0, 20.0],
            [30.0, 10.0]]
        }"""
        )
        self.assertEqual(res.asWkt(), "MultiPoint ((10 40),(40 30),(20 20),(30 10))")
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
            "type": "MultiPoint",
            "coordinates": [[10.0, 40.0, 15.5],
            [40.0, 30.0, 12.5],
            [20.0, 20.0, 1.1],
            [30.0, 10.0, 2.2]]
        }"""
        )
        self.assertEqual(
            res.asWkt(1),
            "MultiPointZ ((10 40 15.5),(40 30 12.5),(20 20 1.1),(30 10 2.2))",
        )
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
                    "type": "MultiPoint",
                    "coordinates": [30.0]
                }"""
        )
        self.assertTrue(res.isNull())
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
                    "type": "MultiPoint",
                    "coordinates": [[30.0]]
                }"""
        )
        self.assertTrue(res.isNull())
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
                    "type": "MultiPoint",
                    "coordinates": 3
                }"""
        )
        self.assertTrue(res.isNull())
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
                    "type": "MultiPoint",
                    "coordinates": [[1,2,3,4]]
                }"""
        )
        self.assertTrue(res.isNull())
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
                    "type": "MultiPoint"
                }"""
        )
        self.assertTrue(res.isNull())

    def test_geojson_linestring(self):
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
    "type": "LineString",
    "coordinates": [
        [30.0, 10.0],
        [10.0, 30.0],
        [40.0, 40.0]
    ]
}"""
        )
        self.assertEqual(res.asWkt(), "LineString (30 10, 10 30, 40 40)")
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
    "type": "LineString",
    "coordinates": [
        [30.0, 10.0, 12.2],
        [10.0, 30.0, 12.3],
        [40.0, 40.0, 12.4]
    ]
}"""
        )
        self.assertEqual(
            res.asWkt(1),
            "LineStringZ (30 10 12.2, 10 30 12.3, 40 40 12.4)",
        )
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
                    "type": "LineString",
                    "coordinates": [30.0]
                }"""
        )
        self.assertTrue(res.isNull())
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
                    "type": "LineString",
                    "coordinates": [[30.0]]
                }"""
        )
        self.assertTrue(res.isNull())
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
                    "type": "LineString",
                    "coordinates": 3
                }"""
        )
        self.assertTrue(res.isNull())
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
                    "type": "LineString",
                    "coordinates": [[1,2,3,4]]
                }"""
        )
        self.assertTrue(res.isNull())
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
                    "type": "LineString"
                }"""
        )
        self.assertTrue(res.isNull())

    def test_geojson_multilinestring(self):
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
    "type": "MultiLineString",
    "coordinates": [
        [
            [10.0, 10.0],
            [20.0, 20.0],
            [10.0, 40.0]
        ],
        [
            [40.0, 40.0],
            [30.0, 30.0],
            [40.0, 20.0],
            [30.0, 10.0]
        ]
    ]
}"""
        )
        self.assertEqual(
            res.asWkt(),
            "MultiLineString ((10 10, 20 20, 10 40),(40 40, 30 30, 40 20, 30 10))",
        )
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
    "type": "MultiLineString",
    "coordinates": [
        [
            [10.0, 10.0, 1.2],
            [20.0, 20.0, 1.3],
            [10.0, 40.0, 1.4]
        ],
        [
            [40.0, 40.0, 2],
            [30.0, 30.0, 3],
            [40.0, 20.0, 4],
            [30.0, 10.0, 5]
        ]
    ]
}"""
        )
        self.assertEqual(
            res.asWkt(1),
            "MultiLineStringZ ((10 10 1.2, 20 20 1.3, 10 40 1.4),(40 40 2, 30 30 3, 40 20 4, 30 10 5))",
        )
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
                    "type": "MultiLineString",
                    "coordinates": [30.0]
                }"""
        )
        self.assertTrue(res.isNull())
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
                    "type": "MultiLineString",
                    "coordinates": [[30.0]]
                }"""
        )
        self.assertTrue(res.isNull())
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
                    "type": "MultiLineString",
                    "coordinates": [[[30.0]]]
                }"""
        )
        self.assertTrue(res.isNull())
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
                    "type": "MultiLineString",
                    "coordinates": 3
                }"""
        )
        self.assertTrue(res.isNull())
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
                    "type": "MultiLineString",
                    "coordinates": [[[1,2,3,4]]]
                }"""
        )
        self.assertTrue(res.isNull())
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
                    "type": "MultiLineString"
                }"""
        )
        self.assertTrue(res.isNull())

    def test_geojson_polygon(self):
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
    "type": "Polygon",
    "coordinates": [
        [
            [35.0, 10.0],
            [45.0, 45.0],
            [15.0, 40.0],
            [10.0, 20.0],
            [35.0, 10.0]
        ],
        [
            [20.0, 30.0],
            [35.0, 35.0],
            [30.0, 20.0],
            [20.0, 30.0]
        ]
    ]
}"""
        )
        self.assertEqual(
            res.asWkt(),
            "Polygon ((35 10, 45 45, 15 40, 10 20, 35 10),(20 30, 35 35, 30 20, 20 30))",
        )
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
    "type": "Polygon",
    "coordinates": [
        [
            [35.0, 10.0],
            [45.0, 45.0],
            [15.0, 40.0],
            [10.0, 20.0],
            [35.0, 10.0]
        ]
    ]
}"""
        )
        self.assertEqual(
            res.asWkt(),
            "Polygon ((35 10, 45 45, 15 40, 10 20, 35 10))",
        )
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
    "type": "Polygon",
    "coordinates": [
        [
            [35.0, 10.0, 1.1],
            [45.0, 45.0, 1.2],
            [15.0, 40.0, 1.3],
            [10.0, 20.0, 1.4],
            [35.0, 10.0, 1.1]
        ],
        [
            [20.0, 30.0, 2],
            [35.0, 35.0, 3],
            [30.0, 20.0, 4],
            [20.0, 30.0, 2]
        ]
    ]
}"""
        )
        self.assertEqual(
            res.asWkt(1),
            "PolygonZ ((35 10 1.1, 45 45 1.2, 15 40 1.3, 10 20 1.4, 35 10 1.1),(20 30 2, 35 35 3, 30 20 4, 20 30 2))",
        )
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
                    "type": "Polygon",
                    "coordinates": [30.0]
                }"""
        )
        self.assertTrue(res.isNull())
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
                    "type": "Polygon",
                    "coordinates": [[30.0]]
                }"""
        )
        self.assertTrue(res.isNull())
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
                    "type": "Polygon",
                    "coordinates": [[[30.0]]]
                }"""
        )
        self.assertTrue(res.isNull())
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
                    "type": "Polygon",
                    "coordinates": 3
                }"""
        )
        self.assertTrue(res.isNull())
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
                    "type": "Polygon",
                    "coordinates": [[[1,2,3,4]]]
                }"""
        )
        self.assertTrue(res.isNull())
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
                    "type": "Polygon"
                }"""
        )
        self.assertTrue(res.isNull())

    def test_geojson_multipolygon(self):
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
    "type": "MultiPolygon",
    "coordinates": [
        [
            [
                [30.0, 20.0],
                [45.0, 40.0],
                [10.0, 40.0],
                [30.0, 20.0]
            ]
        ],
        [
            [
                [15.0, 5.0],
                [40.0, 10.0],
                [10.0, 20.0],
                [5.0, 10.0],
                [15.0, 5.0]
            ]
        ]
    ]
}"""
        )
        self.assertEqual(
            res.asWkt(),
            "MultiPolygon (((30 20, 45 40, 10 40, 30 20)),((15 5, 40 10, 10 20, 5 10, 15 5)))",
        )
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
    "type": "MultiPolygon",
    "coordinates": [
        [
            [
                [40.0, 40.0],
                [20.0, 45.0],
                [45.0, 30.0],
                [40.0, 40.0]
            ]
        ],
        [
            [
                [20.0, 35.0],
                [10.0, 30.0],
                [10.0, 10.0],
                [30.0, 5.0],
                [45.0, 20.0],
                [20.0, 35.0]
            ],
            [
                [30.0, 20.0],
                [20.0, 15.0],
                [20.0, 25.0],
                [30.0, 20.0]
            ]
        ]
    ]
}"""
        )
        self.assertEqual(
            res.asWkt(),
            "MultiPolygon (((40 40, 20 45, 45 30, 40 40)),((20 35, 10 30, 10 10, 30 5, 45 20, 20 35),(30 20, 20 15, 20 25, 30 20)))",
        )
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
    "type": "MultiPolygon",
    "coordinates": [
        [
            [
                [30.0, 20.0, 1],
                [45.0, 40.0, 2],
                [10.0, 40.0, 3],
                [30.0, 20.0, 1]
            ]
        ],
        [
            [
                [15.0, 5.0, 11],
                [40.0, 10.0, 12],
                [10.0, 20.0, 13],
                [5.0, 10.0, 14],
                [15.0, 5.0, 11]
            ]
        ]
    ]
}"""
        )
        self.assertEqual(
            res.asWkt(1),
            "MultiPolygonZ (((30 20 1, 45 40 2, 10 40 3, 30 20 1)),((15 5 11, 40 10 12, 10 20 13, 5 10 14, 15 5 11)))",
        )
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
                    "type": "MultiPolygon",
                    "coordinates": [30.0]
                }"""
        )
        self.assertTrue(res.isNull())
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
                    "type": "MultiPolygon",
                    "coordinates": [[30.0]]
                }"""
        )
        self.assertTrue(res.isNull())
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
                    "type": "MultiPolygon",
                    "coordinates": [[[30.0]]]
                }"""
        )
        self.assertTrue(res.isNull())
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
                    "type": "MultiPolygon",
                    "coordinates": [[[[30.0]]]]
                }"""
        )
        self.assertTrue(res.isNull())
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
                    "type": "MultiPolygon",
                    "coordinates": 3
                }"""
        )
        self.assertTrue(res.isNull())
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
                    "type": "MultiPolygon",
                    "coordinates": [[[1,2,3,4]]]
                }"""
        )
        self.assertTrue(res.isNull())
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
                    "type": "MultiPolygon"
                }"""
        )
        self.assertTrue(res.isNull())

    def test_geojson_collection(self):
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
    "type": "GeometryCollection",
    "geometries": [
        {
            "type": "Point",
            "coordinates": [40.0, 10.0]
        },
        {
            "type": "LineString",
            "coordinates": [
                [10.0, 10.0],
                [20.0, 20.0],
                [10.0, 40.0]
            ]
        },
        {
            "type": "Polygon",
            "coordinates": [
                [
                    [40.0, 40.0],
                    [20.0, 45.0],
                    [45.0, 30.0],
                    [40.0, 40.0]
                ]
            ]
        }
    ]
}"""
        )
        self.assertEqual(
            res.asWkt(),
            "GeometryCollection (Point (40 10),LineString (10 10, 20 20, 10 40),Polygon ((40 40, 20 45, 45 30, 40 40)))",
        )

        res = QgsJsonUtils.geometryFromGeoJson(
            """{
                    "type": "GeometryCollection",
                    "geometries": [30.0]
                }"""
        )
        self.assertTrue(res.isNull())
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
                    "type": "GeometryCollection",
                    "geometries": [[30.0]]
                }"""
        )
        self.assertTrue(res.isNull())
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
                    "type": "GeometryCollection",
                    "geometries": 3
                }"""
        )
        self.assertTrue(res.isNull())
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
                    "type": "GeometryCollection",
                    "geometries": [[[1,2,3,4]]]
                }"""
        )
        self.assertTrue(res.isNull())
        res = QgsJsonUtils.geometryFromGeoJson(
            """{
                    "type": "GeometryCollection"
                }"""
        )
        self.assertTrue(res.isNull())


if __name__ == "__main__":
    unittest.main()
