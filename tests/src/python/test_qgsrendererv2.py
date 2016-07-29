# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsFeatureRendererV2.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '07/06/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.core import (QgsFeatureRendererV2,
                       QgsRendererV2AbstractMetadata,
                       QgsRendererV2Registry,
                       QgsVectorLayer
                       )
from qgis.testing import start_app, unittest

start_app()


def createReferencingLayer():
    layer = QgsVectorLayer("Point?field=fldtxt:string&field=foreignkey:integer",
                           "referencinglayer", "memory")
    pr = layer.dataProvider()
    f1 = QgsFeature()
    f1.setFields(layer.pendingFields())
    f1.setAttributes(["test1", 123])
    f1.setGeometry(QgsGeometry.fromPoint(QgsPoint(100, 200)))
    f2 = QgsFeature()
    f2.setFields(layer.pendingFields())
    f2.setAttributes(["test2", 123])
    f2.setGeometry(QgsGeometry.fromPoint(QgsPoint(101, 201)))
    assert pr.addFeatures([f1, f2])
    return layer


class TestRenderer(QgsRendererV2AbstractMetadata):

    def __init__(self, name, layerTypes=QgsRendererV2AbstractMetadata.All):
        QgsRendererV2AbstractMetadata.__init__(self, name, "Test Renderer")
        self.types = layerTypes

    def compatibleLayerTypes(self):
        return self.types

    def createRenderer(self, elem):
        return None


def clearRegistry():
    # clear registry to start with
    for r in QgsRendererV2Registry.instance().renderersList():
        if r == 'singleSymbol':
            continue
        QgsRendererV2Registry.instance().removeRenderer(r)


class TestQgsRendererV2Registry(unittest.TestCase):

    def testInstance(self):
        """ test retrieving global instance """
        self.assertTrue(QgsRendererV2Registry.instance())

        # instance should be initially populated with some default renderers
        self.assertTrue('singleSymbol' in QgsRendererV2Registry.instance().renderersList())

        # register a renderer to the singleton, to test that the same instance is always returned
        self.assertFalse('test' in QgsRendererV2Registry.instance().renderersList())
        self.assertTrue(QgsRendererV2Registry.instance().addRenderer(TestRenderer('test')))
        self.assertTrue('test' in QgsRendererV2Registry.instance().renderersList())

    def testAddRenderer(self):
        """ test adding renderers to registry """
        clearRegistry()

        # add a renderer
        self.assertTrue(QgsRendererV2Registry.instance().addRenderer(TestRenderer('test2')))
        self.assertTrue('test2' in QgsRendererV2Registry.instance().renderersList())

        # try adding it again - should be rejected due to duplicate name
        self.assertFalse(QgsRendererV2Registry.instance().addRenderer(TestRenderer('test2')))
        self.assertTrue('test2' in QgsRendererV2Registry.instance().renderersList())

    def testRemoveRenderer(self):
        """ test removing renderers from registry """
        clearRegistry()

        # try removing non-existant renderer
        self.assertFalse(QgsRendererV2Registry.instance().removeRenderer('test3'))

        # now add it
        self.assertTrue(QgsRendererV2Registry.instance().addRenderer(TestRenderer('test3')))
        self.assertTrue('test3' in QgsRendererV2Registry.instance().renderersList())

        # try removing it again - should be ok this time
        self.assertTrue(QgsRendererV2Registry.instance().removeRenderer('test3'))
        self.assertFalse('test3' in QgsRendererV2Registry.instance().renderersList())

        # try removing it again - should be false since already removed
        self.assertFalse(QgsRendererV2Registry.instance().removeRenderer('test3'))

    def testRetrieveRenderer(self):
        """ test retrieving renderer by name """
        clearRegistry()

        # try non-existant renderer
        self.assertFalse(QgsRendererV2Registry.instance().rendererMetadata('test4'))

        # now add it
        r = TestRenderer('test4')
        self.assertTrue(QgsRendererV2Registry.instance().addRenderer(r))

        # try retrieving it
        result = QgsRendererV2Registry.instance().rendererMetadata('test4')
        self.assertTrue(result)
        self.assertEqual(result.name(), 'test4')

    def testRenderersList(self):
        """ test getting list of renderers from registry """
        clearRegistry()

        self.assertEqual(QgsRendererV2Registry.instance().renderersList(), ['singleSymbol'])

        # add some renderers
        r1 = TestRenderer('test1', QgsRendererV2AbstractMetadata.PointLayer)
        self.assertTrue(QgsRendererV2Registry.instance().addRenderer(r1))
        r2 = TestRenderer('test2', QgsRendererV2AbstractMetadata.LineLayer)
        self.assertTrue(QgsRendererV2Registry.instance().addRenderer(r2))
        r3 = TestRenderer('test3', QgsRendererV2AbstractMetadata.PolygonLayer)
        self.assertTrue(QgsRendererV2Registry.instance().addRenderer(r3))
        r4 = TestRenderer('test4', QgsRendererV2AbstractMetadata.PointLayer | QgsRendererV2AbstractMetadata.LineLayer)
        self.assertTrue(QgsRendererV2Registry.instance().addRenderer(r4))

        self.assertEqual(QgsRendererV2Registry.instance().renderersList(), ['singleSymbol', 'test1', 'test2', 'test3', 'test4'])

        # test subsets
        self.assertEqual(QgsRendererV2Registry.instance().renderersList(QgsRendererV2AbstractMetadata.PointLayer), ['singleSymbol', 'test1', 'test4'])
        self.assertEqual(QgsRendererV2Registry.instance().renderersList(QgsRendererV2AbstractMetadata.LineLayer), ['singleSymbol', 'test2', 'test4'])
        self.assertEqual(QgsRendererV2Registry.instance().renderersList(QgsRendererV2AbstractMetadata.PolygonLayer), ['singleSymbol', 'test3'])
        self.assertEqual(QgsRendererV2Registry.instance().renderersList(QgsRendererV2AbstractMetadata.LineLayer | QgsRendererV2AbstractMetadata.PolygonLayer), ['singleSymbol', 'test2', 'test3', 'test4'])

    def testRenderersByLayerType(self):
        """ test retrieving compatible renderers by layer type """
        clearRegistry()

        # add some renderers
        r1 = TestRenderer('test1', QgsRendererV2AbstractMetadata.PointLayer)
        self.assertTrue(QgsRendererV2Registry.instance().addRenderer(r1))
        r2 = TestRenderer('test2', QgsRendererV2AbstractMetadata.LineLayer)
        self.assertTrue(QgsRendererV2Registry.instance().addRenderer(r2))
        r3 = TestRenderer('test3', QgsRendererV2AbstractMetadata.PolygonLayer)
        self.assertTrue(QgsRendererV2Registry.instance().addRenderer(r3))
        r4 = TestRenderer('test4', QgsRendererV2AbstractMetadata.PointLayer | QgsRendererV2AbstractMetadata.LineLayer)
        self.assertTrue(QgsRendererV2Registry.instance().addRenderer(r4))

        # make some layers
        point_layer = QgsVectorLayer("Point?field=fldtxt:string",
                                     "pointlayer", "memory")
        line_layer = QgsVectorLayer("LineString?field=fldtxt:string",
                                    "linelayer", "memory")
        polygon_layer = QgsVectorLayer("Polygon?field=fldtxt:string",
                                       "polylayer", "memory")

        # test subsets
        self.assertEqual(QgsRendererV2Registry.instance().renderersList(point_layer), ['singleSymbol', 'test1', 'test4'])
        self.assertEqual(QgsRendererV2Registry.instance().renderersList(line_layer), ['singleSymbol', 'test2', 'test4'])
        self.assertEqual(QgsRendererV2Registry.instance().renderersList(polygon_layer), ['singleSymbol', 'test3'])

if __name__ == '__main__':
    unittest.main()
