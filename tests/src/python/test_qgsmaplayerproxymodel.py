# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsMapLayerProxyModel

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '22/08/2018'
__copyright__ = 'Copyright 2018, The QGIS Project'

import qgis  # NOQA

from qgis.core import (
    QgsVectorLayer,
    QgsMeshLayer,
    QgsAnnotationLayer,
    QgsProject,
    QgsMapLayerModel,
    QgsMapLayerProxyModel
)
from qgis.PyQt.QtCore import Qt, QModelIndex

from qgis.testing import start_app, unittest

start_app()


def create_layer(name):
    layer = QgsVectorLayer("Point?crs=EPSG:3111&field=fldtxt:string&field=fldint:integer",
                           name, "memory")
    return layer


def create_mesh_layer(name):
    layer = QgsMeshLayer("1.0, 2.0\n2.0, 2.0\n3.0, 2.0\n---\n0, 1, 3", name, "mesh_memory")
    return layer


class TestQgsMapLayerProxyModel(unittest.TestCase):

    def testGettersSetters(self):
        """ test model getters/setters """
        m = QgsMapLayerProxyModel()
        l1 = create_layer('l1')
        QgsProject.instance().addMapLayer(l1)
        l2 = create_layer('l2')
        QgsProject.instance().addMapLayer(l2)

        m.setFilters(QgsMapLayerProxyModel.LineLayer | QgsMapLayerProxyModel.WritableLayer)
        self.assertEqual(m.filters(), QgsMapLayerProxyModel.LineLayer | QgsMapLayerProxyModel.WritableLayer)

        m.setExceptedLayerIds([l2.id()])
        self.assertEqual(m.exceptedLayerIds(), [l2.id()])

        m.setExceptedLayerList([l2])
        self.assertEqual(m.exceptedLayerList(), [l2])

        m.setLayerAllowlist([l2])
        self.assertEqual(m.layerAllowlist(), [l2])

        m.setExcludedProviders(['a', 'b'])
        self.assertEqual(m.excludedProviders(), ['a', 'b'])

        m.setFilterString('c')
        self.assertEqual(m.filterString(), 'c')

    def testMeshLayer(self):
        m = QgsMapLayerProxyModel()
        l1 = create_mesh_layer("l1")
        QgsProject.instance().addMapLayer(l1)
        l2 = create_layer('l2')
        QgsProject.instance().addMapLayer(l2)

        m.setFilters(QgsMapLayerProxyModel.MeshLayer)
        self.assertEqual(m.filters(), QgsMapLayerProxyModel.MeshLayer)

        self.assertEqual(m.rowCount(), 1)
        self.assertEqual(m.data(m.index(0, 0)), 'l1')

        self.assertTrue(m.acceptsLayer(l1))
        self.assertFalse(m.acceptsLayer(l2))

    def testAnnotationLayer(self):
        """
        Test filtering annotation layers
        """
        QgsProject.instance().clear()

        m = QgsMapLayerProxyModel()
        options = QgsAnnotationLayer.LayerOptions(QgsProject.instance().transformContext())
        l1 = QgsAnnotationLayer('annotation 1', options)
        QgsProject.instance().addMapLayer(l1)
        l2 = create_layer('l2')
        QgsProject.instance().addMapLayer(l2)

        m.setFilters(QgsMapLayerProxyModel.AnnotationLayer)
        self.assertEqual(m.filters(), QgsMapLayerProxyModel.AnnotationLayer)

        self.assertEqual(m.rowCount(), 1)
        self.assertEqual(m.data(m.index(0, 0)), 'annotation 1')

        self.assertTrue(m.acceptsLayer(l1))
        self.assertFalse(m.acceptsLayer(l2))

        m.setFilters(QgsMapLayerProxyModel.VectorLayer)
        self.assertEqual(m.rowCount(), 1)
        self.assertEqual(m.data(m.index(0, 0)), 'l2')

        self.assertFalse(m.acceptsLayer(l1))
        self.assertTrue(m.acceptsLayer(l2))

        m.setFilters(QgsMapLayerProxyModel.All)
        self.assertEqual(m.rowCount(), 2)
        self.assertEqual(m.data(m.index(0, 0)), 'annotation 1')
        self.assertEqual(m.data(m.index(1, 0)), 'l2')

        self.assertTrue(m.acceptsLayer(l1))
        self.assertTrue(m.acceptsLayer(l2))

    def testFilterGeometryType(self):
        """ test filtering by geometry type """
        QgsProject.instance().clear()
        m = QgsMapLayerProxyModel()
        l1 = QgsVectorLayer("Point?crs=EPSG:3111&field=fldtxt:string&field=fldint:integer",
                            'layer 1', "memory")
        QgsProject.instance().addMapLayer(l1)
        l2 = QgsVectorLayer("Polygon?crs=EPSG:3111&field=fldtxt:string&field=fldint:integer",
                            'layer 2', "memory")
        QgsProject.instance().addMapLayer(l2)
        l3 = QgsVectorLayer("None?field=fldtxt:string&field=fldint:integer",
                            'layer 3', "memory")
        QgsProject.instance().addMapLayer(l3)
        l4 = QgsVectorLayer("LineString?crs=EPSG:3111&field=fldtxt:string&field=fldint:integer",
                            'layer 4', "memory")
        QgsProject.instance().addMapLayer(l4)

        m.setFilters(QgsMapLayerProxyModel.PolygonLayer)
        self.assertEqual(m.rowCount(), 1)
        self.assertEqual(m.data(m.index(0, 0)), 'layer 2')

        self.assertFalse(m.acceptsLayer(l1))
        self.assertTrue(m.acceptsLayer(l2))
        self.assertFalse(m.acceptsLayer(l3))
        self.assertFalse(m.acceptsLayer(l4))

        m.setFilters(QgsMapLayerProxyModel.PointLayer)
        self.assertEqual(m.rowCount(), 1)
        self.assertEqual(m.data(m.index(0, 0)), 'layer 1')

        self.assertTrue(m.acceptsLayer(l1))
        self.assertFalse(m.acceptsLayer(l2))
        self.assertFalse(m.acceptsLayer(l3))
        self.assertFalse(m.acceptsLayer(l4))

        m.setFilters(QgsMapLayerProxyModel.LineLayer)
        self.assertEqual(m.rowCount(), 1)
        self.assertEqual(m.data(m.index(0, 0)), 'layer 4')

        self.assertFalse(m.acceptsLayer(l1))
        self.assertFalse(m.acceptsLayer(l2))
        self.assertFalse(m.acceptsLayer(l3))
        self.assertTrue(m.acceptsLayer(l4))

        m.setFilters(QgsMapLayerProxyModel.NoGeometry)
        self.assertEqual(m.rowCount(), 1)
        self.assertEqual(m.data(m.index(0, 0)), 'layer 3')

        self.assertFalse(m.acceptsLayer(l1))
        self.assertFalse(m.acceptsLayer(l2))
        self.assertTrue(m.acceptsLayer(l3))
        self.assertFalse(m.acceptsLayer(l4))

        m.setFilters(QgsMapLayerProxyModel.HasGeometry)
        self.assertEqual(m.rowCount(), 3)
        self.assertEqual(m.data(m.index(0, 0)), 'layer 1')
        self.assertEqual(m.data(m.index(1, 0)), 'layer 2')
        self.assertEqual(m.data(m.index(2, 0)), 'layer 4')

        self.assertTrue(m.acceptsLayer(l1))
        self.assertTrue(m.acceptsLayer(l2))
        self.assertFalse(m.acceptsLayer(l3))
        self.assertTrue(m.acceptsLayer(l4))

        m.setFilters(QgsMapLayerProxyModel.VectorLayer)
        self.assertEqual(m.rowCount(), 4)
        self.assertEqual(m.data(m.index(0, 0)), 'layer 1')
        self.assertEqual(m.data(m.index(1, 0)), 'layer 2')
        self.assertEqual(m.data(m.index(2, 0)), 'layer 3')
        self.assertEqual(m.data(m.index(3, 0)), 'layer 4')

        self.assertTrue(m.acceptsLayer(l1))
        self.assertTrue(m.acceptsLayer(l2))
        self.assertTrue(m.acceptsLayer(l3))
        self.assertTrue(m.acceptsLayer(l4))

        m.setFilters(QgsMapLayerProxyModel.PluginLayer)
        self.assertEqual(m.rowCount(), 0)

        self.assertFalse(m.acceptsLayer(l1))
        self.assertFalse(m.acceptsLayer(l2))
        self.assertFalse(m.acceptsLayer(l3))
        self.assertFalse(m.acceptsLayer(l4))

        m.setFilters(QgsMapLayerProxyModel.RasterLayer)
        self.assertEqual(m.rowCount(), 0)

        self.assertFalse(m.acceptsLayer(l1))
        self.assertFalse(m.acceptsLayer(l2))
        self.assertFalse(m.acceptsLayer(l3))
        self.assertFalse(m.acceptsLayer(l4))

    def testFilterString(self):
        """ test filtering by string"""
        QgsProject.instance().clear()
        m = QgsMapLayerProxyModel()
        l1 = QgsVectorLayer("Point?crs=EPSG:3111&field=fldtxt:string&field=fldint:integer",
                            'layer 1', "memory")
        QgsProject.instance().addMapLayer(l1)
        l2 = QgsVectorLayer("Polygon?crs=EPSG:3111&field=fldtxt:string&field=fldint:integer",
                            'lAyEr 2', "memory")
        QgsProject.instance().addMapLayer(l2)
        l3 = QgsVectorLayer("None?field=fldtxt:string&field=fldint:integer",
                            'another', "memory")
        QgsProject.instance().addMapLayer(l3)
        l4 = QgsVectorLayer("LineString?crs=EPSG:3111&field=fldtxt:string&field=fldint:integer",
                            'final layer', "memory")
        QgsProject.instance().addMapLayer(l4)

        m.setFilterString('layer')
        self.assertEqual(m.rowCount(), 3)
        self.assertEqual(m.data(m.index(0, 0)), 'final layer')
        self.assertEqual(m.data(m.index(1, 0)), 'layer 1')
        self.assertEqual(m.data(m.index(2, 0)), 'lAyEr 2')

        self.assertTrue(m.acceptsLayer(l1))
        self.assertTrue(m.acceptsLayer(l2))
        self.assertFalse(m.acceptsLayer(l3))
        self.assertTrue(m.acceptsLayer(l4))

        m.setFilterString('')
        self.assertEqual(m.rowCount(), 4)

    def testFilterByLayer(self):
        """ test filtering by layer"""
        QgsProject.instance().clear()
        m = QgsMapLayerProxyModel()
        l1 = QgsVectorLayer("Point?crs=EPSG:3111&field=fldtxt:string&field=fldint:integer",
                            'layer 1', "memory")
        QgsProject.instance().addMapLayer(l1)
        l2 = QgsVectorLayer("Polygon?crs=EPSG:3111&field=fldtxt:string&field=fldint:integer",
                            'lAyEr 2', "memory")
        QgsProject.instance().addMapLayer(l2)
        l3 = QgsVectorLayer("None?field=fldtxt:string&field=fldint:integer",
                            'another', "memory")
        QgsProject.instance().addMapLayer(l3)
        l4 = QgsVectorLayer("LineString?crs=EPSG:3111&field=fldtxt:string&field=fldint:integer",
                            'final layer', "memory")
        QgsProject.instance().addMapLayer(l4)

        self.assertEqual(m.rowCount(), 4)
        self.assertEqual(m.data(m.index(0, 0)), 'another')
        self.assertEqual(m.data(m.index(1, 0)), 'final layer')
        self.assertEqual(m.data(m.index(2, 0)), 'layer 1')
        self.assertEqual(m.data(m.index(3, 0)), 'lAyEr 2')

        self.assertTrue(m.acceptsLayer(l1))
        self.assertTrue(m.acceptsLayer(l2))
        self.assertTrue(m.acceptsLayer(l3))
        self.assertTrue(m.acceptsLayer(l4))

        m.setExceptedLayerList([l1, l3])
        self.assertEqual(m.rowCount(), 2)
        self.assertEqual(m.data(m.index(0, 0)), 'final layer')
        self.assertEqual(m.data(m.index(1, 0)), 'lAyEr 2')

        self.assertFalse(m.acceptsLayer(l1))
        self.assertTrue(m.acceptsLayer(l2))
        self.assertFalse(m.acceptsLayer(l3))
        self.assertTrue(m.acceptsLayer(l4))

        m.setExceptedLayerIds([l2.id(), l4.id()])
        self.assertEqual(m.rowCount(), 2)
        self.assertEqual(m.data(m.index(0, 0)), 'another')
        self.assertEqual(m.data(m.index(1, 0)), 'layer 1')

        self.assertTrue(m.acceptsLayer(l1))
        self.assertFalse(m.acceptsLayer(l2))
        self.assertTrue(m.acceptsLayer(l3))
        self.assertFalse(m.acceptsLayer(l4))

        m.setLayerAllowlist([l1])
        self.assertEqual(m.rowCount(), 1)
        self.assertEqual(m.data(m.index(0, 0)), 'layer 1')

        self.assertTrue(m.acceptsLayer(l1))
        self.assertFalse(m.acceptsLayer(l2))
        self.assertFalse(m.acceptsLayer(l3))
        self.assertFalse(m.acceptsLayer(l4))

        m.setExceptedLayerIds([])
        self.assertEqual(m.rowCount(), 1)
        self.assertEqual(m.data(m.index(0, 0)), 'layer 1')

        self.assertTrue(m.acceptsLayer(l1))
        self.assertFalse(m.acceptsLayer(l2))
        self.assertFalse(m.acceptsLayer(l3))
        self.assertFalse(m.acceptsLayer(l4))

        m.setLayerAllowlist([l2, l3])
        self.assertEqual(m.rowCount(), 2)
        self.assertEqual(m.data(m.index(0, 0)), 'another')
        self.assertEqual(m.data(m.index(1, 0)), 'lAyEr 2')

        self.assertFalse(m.acceptsLayer(l1))
        self.assertTrue(m.acceptsLayer(l2))
        self.assertTrue(m.acceptsLayer(l3))
        self.assertFalse(m.acceptsLayer(l4))

        m.setLayerAllowlist([])
        self.assertEqual(m.rowCount(), 4)

        self.assertTrue(m.acceptsLayer(l1))
        self.assertTrue(m.acceptsLayer(l2))
        self.assertTrue(m.acceptsLayer(l3))
        self.assertTrue(m.acceptsLayer(l4))


if __name__ == '__main__':
    unittest.main()
