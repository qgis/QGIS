"""QGIS Unit tests for QgsFeatureSink.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2017 by Nyall Dawson"
__date__ = "26/04/2017"
__copyright__ = "Copyright 2017, The QGIS Project"

from qgis.PyQt.QtCore import QVariant
from qgis.core import (
    QgsCoordinateReferenceSystem,
    QgsExpressionContext,
    QgsExpressionContextScope,
    QgsFeature,
    QgsFeatureStore,
    QgsField,
    QgsFields,
    QgsGeometry,
    QgsPointXY,
    QgsProject,
    QgsProperty,
    QgsProxyFeatureSink,
    QgsRemappingProxyFeatureSink,
    QgsRemappingSinkDefinition,
    QgsVectorLayer,
    QgsWkbTypes,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


def createLayerWithFivePoints():
    layer = QgsVectorLayer(
        "Point?field=fldtxt:string&field=fldint:integer", "addfeat", "memory"
    )
    pr = layer.dataProvider()
    f = QgsFeature()
    f.setAttributes(["test", 123])
    f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(100, 200)))
    f2 = QgsFeature()
    f2.setAttributes(["test2", 457])
    f2.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(200, 200)))
    f3 = QgsFeature()
    f3.setAttributes(["test2", 888])
    f3.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(300, 200)))
    f4 = QgsFeature()
    f4.setAttributes(["test3", -1])
    f4.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(400, 300)))
    f5 = QgsFeature()
    f5.setAttributes(["test4", 0])
    f5.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(0, 0)))
    assert pr.addFeatures([f, f2, f3, f4, f5])
    assert layer.featureCount() == 5
    return layer


class TestQgsFeatureSink(QgisTestCase):

    def testFromIterator(self):
        """
        Test adding features from an iterator
        :return:
        """
        layer = createLayerWithFivePoints()
        store = QgsFeatureStore(layer.fields(), layer.crs())

        self.assertTrue(store.addFeatures(layer.getFeatures()))
        vals = [f["fldint"] for f in store.features()]
        self.assertEqual(vals, [123, 457, 888, -1, 0])

    def testProxyFeatureSink(self):
        fields = QgsFields()
        fields.append(QgsField("fldtxt", QVariant.String))
        fields.append(QgsField("fldint", QVariant.Int))

        store = QgsFeatureStore(fields, QgsCoordinateReferenceSystem())
        proxy = QgsProxyFeatureSink(store)
        self.assertEqual(proxy.destinationSink(), store)

        self.assertEqual(len(store), 0)

        f = QgsFeature()
        f.setAttributes(["test", 123])
        f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(100, 200)))
        proxy.addFeature(f)
        self.assertEqual(len(store), 1)
        self.assertEqual(store.features()[0]["fldtxt"], "test")

        f2 = QgsFeature()
        f2.setAttributes(["test2", 457])
        f2.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(200, 200)))
        f3 = QgsFeature()
        f3.setAttributes(["test3", 888])
        f3.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(300, 200)))
        proxy.addFeatures([f2, f3])
        self.assertEqual(len(store), 3)
        self.assertEqual(store.features()[1]["fldtxt"], "test2")
        self.assertEqual(store.features()[2]["fldtxt"], "test3")

    def testRemappingSinkDefinition(self):
        """
        Test remapping sink definitions
        """
        fields = QgsFields()
        fields.append(QgsField("fldtxt", QVariant.String))
        fields.append(QgsField("fldint", QVariant.Int))
        fields.append(QgsField("fldtxt2", QVariant.String))

        mapping_def = QgsRemappingSinkDefinition()
        mapping_def.setDestinationWkbType(QgsWkbTypes.Type.Point)
        self.assertEqual(mapping_def.destinationWkbType(), QgsWkbTypes.Type.Point)
        mapping_def.setSourceCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        mapping_def.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        self.assertEqual(mapping_def.sourceCrs().authid(), "EPSG:4326")
        self.assertEqual(mapping_def.destinationCrs().authid(), "EPSG:3857")
        mapping_def.setDestinationFields(fields)
        self.assertEqual(mapping_def.destinationFields(), fields)
        mapping_def.addMappedField("fldtxt2", QgsProperty.fromField("fld1"))
        mapping_def.addMappedField(
            "fldint", QgsProperty.fromExpression("@myval * fldint")
        )

        self.assertEqual(mapping_def.fieldMap()["fldtxt2"].field(), "fld1")
        self.assertEqual(
            mapping_def.fieldMap()["fldint"].expressionString(), "@myval * fldint"
        )

        mapping_def2 = QgsRemappingSinkDefinition(mapping_def)
        self.assertTrue(mapping_def == mapping_def2)
        self.assertFalse(mapping_def != mapping_def2)
        mapping_def2.setDestinationWkbType(QgsWkbTypes.Type.Polygon)
        self.assertFalse(mapping_def == mapping_def2)
        self.assertTrue(mapping_def != mapping_def2)
        mapping_def2.setDestinationWkbType(QgsWkbTypes.Type.Point)
        mapping_def2.setSourceCrs(QgsCoordinateReferenceSystem("EPSG:3111"))
        self.assertFalse(mapping_def == mapping_def2)
        self.assertTrue(mapping_def != mapping_def2)
        mapping_def2.setSourceCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        mapping_def2.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3111"))
        self.assertFalse(mapping_def == mapping_def2)
        self.assertTrue(mapping_def != mapping_def2)
        mapping_def2.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        mapping_def2.setDestinationFields(QgsFields())
        self.assertFalse(mapping_def == mapping_def2)
        self.assertTrue(mapping_def != mapping_def2)
        mapping_def2.setDestinationFields(fields)
        mapping_def2.addMappedField(
            "fldint3", QgsProperty.fromExpression("@myval * fldint")
        )
        self.assertFalse(mapping_def == mapping_def2)
        self.assertTrue(mapping_def != mapping_def2)
        mapping_def2.setFieldMap(mapping_def.fieldMap())
        self.assertTrue(mapping_def == mapping_def2)
        self.assertFalse(mapping_def != mapping_def2)

        # to variant
        var = mapping_def.toVariant()

        def2 = QgsRemappingSinkDefinition()
        def2.loadVariant(var)
        self.assertEqual(def2.destinationWkbType(), QgsWkbTypes.Type.Point)
        self.assertEqual(def2.sourceCrs().authid(), "EPSG:4326")
        self.assertEqual(def2.destinationCrs().authid(), "EPSG:3857")
        self.assertEqual(def2.destinationFields()[0].name(), "fldtxt")
        self.assertEqual(def2.destinationFields()[1].name(), "fldint")
        self.assertEqual(def2.fieldMap()["fldtxt2"].field(), "fld1")
        self.assertEqual(
            def2.fieldMap()["fldint"].expressionString(), "@myval * fldint"
        )

    def testRemappingSink(self):
        """
        Test remapping features
        """
        fields = QgsFields()
        fields.append(QgsField("fldtxt", QVariant.String))
        fields.append(QgsField("fldint", QVariant.Int))
        fields.append(QgsField("fldtxt2", QVariant.String))

        store = QgsFeatureStore(fields, QgsCoordinateReferenceSystem("EPSG:3857"))

        mapping_def = QgsRemappingSinkDefinition()
        mapping_def.setDestinationWkbType(QgsWkbTypes.Type.Point)
        mapping_def.setSourceCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        mapping_def.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        mapping_def.setDestinationFields(fields)
        mapping_def.addMappedField("fldtxt2", QgsProperty.fromField("fld1"))
        mapping_def.addMappedField(
            "fldint", QgsProperty.fromExpression("@myval * fldint")
        )

        proxy = QgsRemappingProxyFeatureSink(mapping_def, store)
        self.assertEqual(proxy.destinationSink(), store)

        self.assertEqual(len(store), 0)

        incoming_fields = QgsFields()
        incoming_fields.append(QgsField("fld1", QVariant.String))
        incoming_fields.append(QgsField("fldint", QVariant.Int))

        context = QgsExpressionContext()
        scope = QgsExpressionContextScope()
        scope.setVariable("myval", 2)
        context.appendScope(scope)
        context.setFields(incoming_fields)
        proxy.setExpressionContext(context)
        proxy.setTransformContext(QgsProject.instance().transformContext())

        f = QgsFeature()
        f.setFields(incoming_fields)
        f.setAttributes(["test", 123])
        f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(1, 2)))
        self.assertTrue(proxy.addFeature(f))
        self.assertEqual(len(store), 1)
        self.assertEqual(
            store.features()[0].geometry().asWkt(1), "Point (111319.5 222684.2)"
        )
        self.assertEqual(store.features()[0].attributes(), [None, 246, "test"])

        f2 = QgsFeature()
        f2.setAttributes(["test2", 457])
        f2.setGeometry(QgsGeometry.fromWkt("LineString( 1 1, 2 2)"))
        f3 = QgsFeature()
        f3.setAttributes(["test3", 888])
        f3.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(3, 4)))
        self.assertTrue(proxy.addFeatures([f2, f3]))
        self.assertEqual(len(store), 4)
        self.assertEqual(store.features()[1].attributes(), [None, 914, "test2"])
        self.assertEqual(store.features()[2].attributes(), [None, 914, "test2"])
        self.assertEqual(store.features()[3].attributes(), [None, 1776, "test3"])
        self.assertEqual(
            store.features()[1].geometry().asWkt(1), "Point (111319.5 111325.1)"
        )
        self.assertEqual(
            store.features()[2].geometry().asWkt(1), "Point (222639 222684.2)"
        )
        self.assertEqual(
            store.features()[3].geometry().asWkt(1), "Point (333958.5 445640.1)"
        )


if __name__ == "__main__":
    unittest.main()
