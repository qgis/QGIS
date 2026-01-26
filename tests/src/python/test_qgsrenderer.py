"""QGIS Unit tests for QgsFeatureRenderer.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "07/06/2016"
__copyright__ = "Copyright 2016, The QGIS Project"


from qgis.core import (
    QgsApplication,
    QgsFeature,
    QgsGeometry,
    QgsPointXY,
    QgsRendererAbstractMetadata,
    QgsVectorLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


def createReferencingLayer():
    layer = QgsVectorLayer(
        "Point?field=fldtxt:string&field=foreignkey:integer",
        "referencinglayer",
        "memory",
    )
    pr = layer.dataProvider()
    f1 = QgsFeature()
    f1.setFields(layer.fields())
    f1.setAttributes(["test1", 123])
    f1.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(100, 200)))
    f2 = QgsFeature()
    f2.setFields(layer.fields())
    f2.setAttributes(["test2", 123])
    f2.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(101, 201)))
    assert pr.addFeatures([f1, f2])
    return layer


class TestRenderer(QgsRendererAbstractMetadata):

    def __init__(self, name, layerTypes=QgsRendererAbstractMetadata.LayerType.All):
        QgsRendererAbstractMetadata.__init__(self, name, "Test Renderer")
        self.types = layerTypes

    def compatibleLayerTypes(self):
        return self.types

    def createRenderer(self, elem):
        return None


def clearRegistry():
    # clear registry to start with
    for r in QgsApplication.rendererRegistry().renderersList():
        if r == "singleSymbol":
            continue
        QgsApplication.rendererRegistry().removeRenderer(r)


class TestQgsRendererV2Registry(QgisTestCase):

    def testInstance(self):
        """test retrieving global instance"""
        self.assertTrue(QgsApplication.rendererRegistry())

        # instance should be initially populated with some default renderers
        self.assertIn("singleSymbol", QgsApplication.rendererRegistry().renderersList())

        # register a renderer to the singleton, to test that the same instance is always returned
        self.assertNotIn("test", QgsApplication.rendererRegistry().renderersList())
        self.assertTrue(
            QgsApplication.rendererRegistry().addRenderer(TestRenderer("test"))
        )
        self.assertIn("test", QgsApplication.rendererRegistry().renderersList())

    def testAddRenderer(self):
        """test adding renderers to registry"""
        clearRegistry()

        # add a renderer
        self.assertTrue(
            QgsApplication.rendererRegistry().addRenderer(TestRenderer("test2"))
        )
        self.assertIn("test2", QgsApplication.rendererRegistry().renderersList())

        # try adding it again - should be rejected due to duplicate name
        self.assertFalse(
            QgsApplication.rendererRegistry().addRenderer(TestRenderer("test2"))
        )
        self.assertIn("test2", QgsApplication.rendererRegistry().renderersList())

    def testRemoveRenderer(self):
        """test removing renderers from registry"""
        clearRegistry()

        # try removing non-existent renderer
        self.assertFalse(QgsApplication.rendererRegistry().removeRenderer("test3"))

        # now add it
        self.assertTrue(
            QgsApplication.rendererRegistry().addRenderer(TestRenderer("test3"))
        )
        self.assertIn("test3", QgsApplication.rendererRegistry().renderersList())

        # try removing it again - should be OK this time
        self.assertTrue(QgsApplication.rendererRegistry().removeRenderer("test3"))
        self.assertNotIn("test3", QgsApplication.rendererRegistry().renderersList())

        # try removing it again - should be false since already removed
        self.assertFalse(QgsApplication.rendererRegistry().removeRenderer("test3"))

    def testRetrieveRenderer(self):
        """test retrieving renderer by name"""
        clearRegistry()

        # try non-existent renderer
        self.assertFalse(QgsApplication.rendererRegistry().rendererMetadata("test4"))

        # now add it
        r = TestRenderer("test4")
        self.assertTrue(QgsApplication.rendererRegistry().addRenderer(r))

        # try retrieving it
        result = QgsApplication.rendererRegistry().rendererMetadata("test4")
        self.assertTrue(result)
        self.assertEqual(result.name(), "test4")

    def testRenderersList(self):
        """test getting list of renderers from registry"""
        clearRegistry()

        self.assertEqual(
            QgsApplication.rendererRegistry().renderersList(), ["singleSymbol"]
        )

        # add some renderers
        r1 = TestRenderer("test1", QgsRendererAbstractMetadata.LayerType.PointLayer)
        self.assertTrue(QgsApplication.rendererRegistry().addRenderer(r1))
        r2 = TestRenderer("test2", QgsRendererAbstractMetadata.LayerType.LineLayer)
        self.assertTrue(QgsApplication.rendererRegistry().addRenderer(r2))
        r3 = TestRenderer("test3", QgsRendererAbstractMetadata.LayerType.PolygonLayer)
        self.assertTrue(QgsApplication.rendererRegistry().addRenderer(r3))
        r4 = TestRenderer(
            "test4",
            QgsRendererAbstractMetadata.LayerType.PointLayer
            | QgsRendererAbstractMetadata.LayerType.LineLayer,
        )
        self.assertTrue(QgsApplication.rendererRegistry().addRenderer(r4))

        self.assertEqual(
            QgsApplication.rendererRegistry().renderersList(),
            ["singleSymbol", "test1", "test2", "test3", "test4"],
        )

        # test subsets
        self.assertEqual(
            QgsApplication.rendererRegistry().renderersList(
                QgsRendererAbstractMetadata.LayerType.PointLayer
            ),
            ["singleSymbol", "test1", "test4"],
        )
        self.assertEqual(
            QgsApplication.rendererRegistry().renderersList(
                QgsRendererAbstractMetadata.LayerType.LineLayer
            ),
            ["singleSymbol", "test2", "test4"],
        )
        self.assertEqual(
            QgsApplication.rendererRegistry().renderersList(
                QgsRendererAbstractMetadata.LayerType.PolygonLayer
            ),
            ["singleSymbol", "test3"],
        )
        self.assertEqual(
            QgsApplication.rendererRegistry().renderersList(
                QgsRendererAbstractMetadata.LayerType.LineLayer
                | QgsRendererAbstractMetadata.LayerType.PolygonLayer
            ),
            ["singleSymbol", "test2", "test3", "test4"],
        )

    def testRenderersByLayerType(self):
        """test retrieving compatible renderers by layer type"""
        clearRegistry()

        # add some renderers
        r1 = TestRenderer("test1", QgsRendererAbstractMetadata.LayerType.PointLayer)
        self.assertTrue(QgsApplication.rendererRegistry().addRenderer(r1))
        r2 = TestRenderer("test2", QgsRendererAbstractMetadata.LayerType.LineLayer)
        self.assertTrue(QgsApplication.rendererRegistry().addRenderer(r2))
        r3 = TestRenderer("test3", QgsRendererAbstractMetadata.LayerType.PolygonLayer)
        self.assertTrue(QgsApplication.rendererRegistry().addRenderer(r3))
        r4 = TestRenderer(
            "test4",
            QgsRendererAbstractMetadata.LayerType.PointLayer
            | QgsRendererAbstractMetadata.LayerType.LineLayer,
        )
        self.assertTrue(QgsApplication.rendererRegistry().addRenderer(r4))

        # make some layers
        point_layer = QgsVectorLayer(
            "Point?field=fldtxt:string", "pointlayer", "memory"
        )
        line_layer = QgsVectorLayer(
            "LineString?field=fldtxt:string", "linelayer", "memory"
        )
        polygon_layer = QgsVectorLayer(
            "Polygon?field=fldtxt:string", "polylayer", "memory"
        )

        # test subsets
        self.assertEqual(
            QgsApplication.rendererRegistry().renderersList(point_layer),
            ["singleSymbol", "test1", "test4"],
        )
        self.assertEqual(
            QgsApplication.rendererRegistry().renderersList(line_layer),
            ["singleSymbol", "test2", "test4"],
        )
        self.assertEqual(
            QgsApplication.rendererRegistry().renderersList(polygon_layer),
            ["singleSymbol", "test3"],
        )


if __name__ == "__main__":
    unittest.main()
