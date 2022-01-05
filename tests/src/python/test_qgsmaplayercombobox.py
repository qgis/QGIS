# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsMapLayerComboBox

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '14/06/2019'
__copyright__ = 'Copyright 2019, The QGIS Project'

import qgis  # NOQA

from qgis.core import QgsVectorLayer, QgsMeshLayer, QgsProject, QgsMapLayerProxyModel
from qgis.gui import QgsMapLayerComboBox
from qgis.PyQt.QtCore import (
    QCoreApplication,
    QEvent
)
from qgis.PyQt.QtTest import QSignalSpy

from qgis.testing import start_app, unittest

start_app()


def create_layer(name):
    layer = QgsVectorLayer("Point?crs=EPSG:3111&field=fldtxt:string&field=fldint:integer",
                           name, "memory")
    return layer


def create_mesh_layer(name):
    layer = QgsMeshLayer("1.0, 2.0\n2.0, 2.0\n3.0, 2.0\n---\n0, 1, 3", name, "mesh_memory")
    return layer


class TestQgsMapLayerComboBox(unittest.TestCase):

    def testGettersSetters(self):
        """ test combo getters/setters """
        m = QgsMapLayerComboBox()
        l1 = create_layer('l1')
        QgsProject.instance().addMapLayer(l1)
        l2 = create_layer('l2')
        QgsProject.instance().addMapLayer(l2)

        m.setFilters(QgsMapLayerProxyModel.LineLayer | QgsMapLayerProxyModel.WritableLayer)
        self.assertEqual(m.filters(), QgsMapLayerProxyModel.LineLayer | QgsMapLayerProxyModel.WritableLayer)

        m.setExceptedLayerList([l2])
        self.assertEqual(m.exceptedLayerList(), [l2])

        m.setExcludedProviders(['a', 'b'])
        self.assertEqual(m.excludedProviders(), ['a', 'b'])

    def testMeshLayer(self):
        m = QgsMapLayerComboBox()
        l1 = create_mesh_layer("l1")
        QgsProject.instance().addMapLayer(l1)
        l2 = create_layer('l2')
        QgsProject.instance().addMapLayer(l2)

        m.setFilters(QgsMapLayerProxyModel.MeshLayer)
        self.assertEqual(m.filters(), QgsMapLayerProxyModel.MeshLayer)

        self.assertEqual(m.count(), 1)
        self.assertEqual(m.itemText(0), 'l1')

    def testFilterGeometryType(self):
        """ test filtering by geometry type """
        QgsProject.instance().clear()
        m = QgsMapLayerComboBox()
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
        self.assertEqual(m.count(), 1)
        self.assertEqual(m.itemText(0), 'layer 2')

        m.setFilters(QgsMapLayerProxyModel.PointLayer)
        self.assertEqual(m.count(), 1)
        self.assertEqual(m.itemText(0), 'layer 1')

        m.setFilters(QgsMapLayerProxyModel.LineLayer)
        self.assertEqual(m.count(), 1)
        self.assertEqual(m.itemText(0), 'layer 4')

        m.setFilters(QgsMapLayerProxyModel.NoGeometry)
        self.assertEqual(m.count(), 1)
        self.assertEqual(m.itemText(0), 'layer 3')

        m.setFilters(QgsMapLayerProxyModel.HasGeometry)
        self.assertEqual(m.count(), 3)
        self.assertEqual(m.itemText(0), 'layer 1')
        self.assertEqual(m.itemText(1), 'layer 2')
        self.assertEqual(m.itemText(2), 'layer 4')

        m.setFilters(QgsMapLayerProxyModel.VectorLayer)
        self.assertEqual(m.count(), 4)
        self.assertEqual(m.itemText(0), 'layer 1')
        self.assertEqual(m.itemText(1), 'layer 2')
        self.assertEqual(m.itemText(2), 'layer 3')
        self.assertEqual(m.itemText(3), 'layer 4')

        m.setFilters(QgsMapLayerProxyModel.PluginLayer)
        self.assertEqual(m.count(), 0)

        m.setFilters(QgsMapLayerProxyModel.RasterLayer)
        self.assertEqual(m.count(), 0)

    def testFilterByLayer(self):
        """ test filtering by layer"""
        QgsProject.instance().clear()
        m = QgsMapLayerComboBox()
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

        self.assertEqual(m.count(), 4)
        self.assertEqual(m.itemText(0), 'another')
        self.assertEqual(m.itemText(1), 'final layer')
        self.assertEqual(m.itemText(2), 'layer 1')
        self.assertEqual(m.itemText(3), 'lAyEr 2')

        m.setExceptedLayerList([l1, l3])
        self.assertEqual(m.count(), 2)
        self.assertEqual(m.itemText(0), 'final layer')
        self.assertEqual(m.itemText(1), 'lAyEr 2')

        m.setExceptedLayerList([l2, l4])
        self.assertEqual(m.count(), 2)
        self.assertEqual(m.itemText(0), 'another')
        self.assertEqual(m.itemText(1), 'layer 1')

    def testSignals(self):
        QgsProject.instance().clear()
        m = QgsMapLayerComboBox()
        l1 = QgsVectorLayer("Point?crs=EPSG:3111&field=fldtxt:string&field=fldint:integer",
                            'layer 1', "memory")
        QgsProject.instance().addMapLayer(l1)
        l2 = QgsVectorLayer("Polygon?crs=EPSG:3111&field=fldtxt:string&field=fldint:integer",
                            'lAyEr 2', "memory")
        QgsProject.instance().addMapLayer(l2)
        spy = QSignalSpy(m.layerChanged)

        m.setLayer(l2)
        self.assertEqual(len(spy), 1)
        self.assertEqual(m.currentLayer(), l2)
        m.setLayer(l1)
        self.assertEqual(len(spy), 2)
        self.assertEqual(m.currentLayer(), l1)
        # no signal if same layer
        m.setLayer(l1)
        self.assertEqual(len(spy), 2)

        m.setAllowEmptyLayer(True)
        m.setLayer(None)
        self.assertEqual(len(spy), 3)
        self.assertIsNone(m.currentLayer())
        self.assertFalse(m.currentText())
        m.setLayer(None)
        self.assertEqual(len(spy), 3)
        self.assertIsNone(m.currentLayer())

        m.setEditable(True)
        m.setCurrentText('aaa')
        self.assertIsNone(m.currentLayer())

        m.setLayer(l1)
        self.assertEqual(len(spy), 4)
        self.assertEqual(m.currentLayer(), l1)

    def testAdditionalLayers(self):
        QgsProject.instance().clear()
        l1 = create_layer('l1')
        l2 = create_layer('l2')
        QgsProject.instance().addMapLayers([l1, l2])
        m = QgsMapLayerComboBox()
        self.assertEqual(m.count(), 2)
        l3 = create_layer('l3')
        l4 = create_layer('l4')
        m.setAdditionalLayers([l3, l4])
        self.assertEqual(m.count(), 4)

        m.setAdditionalItems(['a', 'b'])
        self.assertEqual(m.count(), 6)
        self.assertEqual(m.itemText(0), 'l1')
        self.assertEqual(m.itemText(1), 'l2')
        self.assertEqual(m.itemText(2), 'l3')
        self.assertEqual(m.itemText(3), 'l4')
        self.assertEqual(m.itemText(4), 'a')
        self.assertEqual(m.itemText(5), 'b')

        m.setAllowEmptyLayer(True)
        self.assertEqual(m.count(), 7)
        self.assertFalse(m.itemText(0))
        self.assertEqual(m.itemText(1), 'l1')
        self.assertEqual(m.itemText(2), 'l2')
        self.assertEqual(m.itemText(3), 'l3')
        self.assertEqual(m.itemText(4), 'l4')
        self.assertEqual(m.itemText(5), 'a')
        self.assertEqual(m.itemText(6), 'b')

        l3.deleteLater()
        QCoreApplication.sendPostedEvents(None, QEvent.DeferredDelete)
        self.assertEqual(m.count(), 6)
        self.assertFalse(m.itemText(0))
        self.assertEqual(m.itemText(1), 'l1')
        self.assertEqual(m.itemText(2), 'l2')
        self.assertEqual(m.itemText(3), 'l4')
        self.assertEqual(m.itemText(4), 'a')
        self.assertEqual(m.itemText(5), 'b')

        l5 = create_layer('l5')
        l6 = create_layer('l6')
        m.setAdditionalLayers([l5, l6, l4])
        self.assertEqual(m.count(), 8)
        self.assertFalse(m.itemText(0))
        self.assertEqual(m.itemText(1), 'l1')
        self.assertEqual(m.itemText(2), 'l2')
        self.assertEqual(m.itemText(3), 'l4')
        self.assertEqual(m.itemText(4), 'l5')
        self.assertEqual(m.itemText(5), 'l6')
        self.assertEqual(m.itemText(6), 'a')
        self.assertEqual(m.itemText(7), 'b')

        m.setAdditionalLayers([l5, l4])
        self.assertEqual(m.count(), 7)
        self.assertFalse(m.itemText(0))
        self.assertEqual(m.itemText(1), 'l1')
        self.assertEqual(m.itemText(2), 'l2')
        self.assertEqual(m.itemText(3), 'l4')
        self.assertEqual(m.itemText(4), 'l5')
        self.assertEqual(m.itemText(5), 'a')
        self.assertEqual(m.itemText(6), 'b')

        QgsProject.instance().removeMapLayers([l1.id(), l2.id()])

        self.assertEqual(m.count(), 5)
        self.assertFalse(m.itemText(0))
        self.assertEqual(m.itemText(1), 'l4')
        self.assertEqual(m.itemText(2), 'l5')
        self.assertEqual(m.itemText(3), 'a')
        self.assertEqual(m.itemText(4), 'b')

    def testProject(self):
        QgsProject.instance().clear()
        lA = create_layer('lA')
        lB = create_layer('lB')

        projectA = QgsProject.instance()
        projectB = QgsProject()

        projectA.addMapLayer(lA)
        projectB.addMapLayer(lB)
        cb = QgsMapLayerComboBox()

        self.assertEqual(cb.currentLayer(), lA)

        cb.setProject(projectB)
        self.assertEqual(cb.currentLayer(), lB)

        cb.setProject(projectA)
        self.assertEqual(cb.currentLayer(), lA)

        QgsProject.instance().clear()


if __name__ == '__main__':
    unittest.main()
