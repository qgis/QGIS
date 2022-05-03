# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsVectorLayerElevationProperties

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '09/11/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA

from qgis.core import (
    Qgis,
    QgsVectorLayer,
    QgsVectorLayerElevationProperties,
    QgsReadWriteContext,
    QgsLineSymbol,
    QgsMarkerSymbol,
    QgsFillSymbol,
    QgsProperty,
    QgsMapLayerElevationProperties,
    QgsPropertyCollection
)
from utilities import unitTestDataPath

from qgis.PyQt.QtXml import QDomDocument

from qgis.testing import start_app, unittest

start_app()


class TestQgsVectorLayerElevationProperties(unittest.TestCase):

    def testBasic(self):
        props = QgsVectorLayerElevationProperties(None)
        self.assertEqual(props.zScale(), 1)
        self.assertEqual(props.zOffset(), 0)
        self.assertFalse(props.extrusionEnabled())
        self.assertEqual(props.extrusionHeight(), 0)
        self.assertTrue(props.hasElevation())
        self.assertEqual(props.clamping(), Qgis.AltitudeClamping.Terrain)
        self.assertEqual(props.binding(), Qgis.AltitudeBinding.Centroid)
        self.assertTrue(props.respectLayerSymbology())
        self.assertEqual(props.type(), Qgis.VectorProfileType.IndividualFeatures)
        self.assertEqual(props.profileSymbology(), Qgis.ProfileSurfaceSymbology.Line)
        self.assertFalse(props.showMarkerSymbolInSurfacePlots())

        props.setZOffset(0.5)
        props.setZScale(2)
        props.setClamping(Qgis.AltitudeClamping.Relative)
        props.setBinding(Qgis.AltitudeBinding.Vertex)
        props.setExtrusionHeight(10)
        props.setExtrusionEnabled(True)
        props.setRespectLayerSymbology(False)
        props.setType(Qgis.VectorProfileType.ContinuousSurface)
        props.setProfileSymbology(Qgis.ProfileSurfaceSymbology.FillBelow)
        props.setShowMarkerSymbolInSurfacePlots(True)

        self.assertEqual(props.zScale(), 2)
        self.assertEqual(props.zOffset(), 0.5)
        self.assertEqual(props.extrusionHeight(), 10)
        self.assertTrue(props.hasElevation())
        self.assertTrue(props.extrusionEnabled())
        self.assertEqual(props.clamping(), Qgis.AltitudeClamping.Relative)
        self.assertEqual(props.binding(), Qgis.AltitudeBinding.Vertex)
        self.assertFalse(props.respectLayerSymbology())
        self.assertEqual(props.type(), Qgis.VectorProfileType.ContinuousSurface)
        self.assertEqual(props.profileSymbology(), Qgis.ProfileSurfaceSymbology.FillBelow)
        self.assertTrue(props.showMarkerSymbolInSurfacePlots())

        props.dataDefinedProperties().setProperty(QgsMapLayerElevationProperties.ExtrusionHeight, QgsProperty.fromExpression('1*5'))
        self.assertEqual(props.dataDefinedProperties().property(QgsMapLayerElevationProperties.ExtrusionHeight).asExpression(), '1*5')
        properties = QgsPropertyCollection()
        properties.setProperty(QgsMapLayerElevationProperties.ZOffset, QgsProperty.fromExpression('9'))
        props.setDataDefinedProperties(properties)
        self.assertFalse(
            props.dataDefinedProperties().isActive(QgsMapLayerElevationProperties.ExtrusionHeight))
        self.assertEqual(
            props.dataDefinedProperties().property(QgsMapLayerElevationProperties.ZOffset).asExpression(),
            '9')

        sym = QgsLineSymbol.createSimple({'outline_color': '#ff4433', 'outline_width': 0.5})
        props.setProfileLineSymbol(sym)
        self.assertEqual(props.profileLineSymbol().color().name(), '#ff4433')

        sym = QgsFillSymbol.createSimple({'color': '#ff4455', 'outline_width': 0.5})
        props.setProfileFillSymbol(sym)
        self.assertEqual(props.profileFillSymbol().color().name(), '#ff4455')

        sym = QgsMarkerSymbol.createSimple({'color': '#ff1122', 'outline_width': 0.5})
        props.setProfileMarkerSymbol(sym)
        self.assertEqual(props.profileMarkerSymbol().color().name(), '#ff1122')

        doc = QDomDocument("testdoc")
        elem = doc.createElement('test')
        props.writeXml(elem, doc, QgsReadWriteContext())

        props2 = QgsVectorLayerElevationProperties(None)
        props2.readXml(elem, QgsReadWriteContext())
        self.assertEqual(props2.zScale(), 2)
        self.assertEqual(props2.zOffset(), 0.5)
        self.assertEqual(props2.clamping(), Qgis.AltitudeClamping.Relative)
        self.assertEqual(props2.binding(), Qgis.AltitudeBinding.Vertex)
        self.assertEqual(props2.extrusionHeight(), 10)
        self.assertTrue(props2.extrusionEnabled())
        self.assertFalse(props2.respectLayerSymbology())
        self.assertEqual(props2.type(), Qgis.VectorProfileType.ContinuousSurface)
        self.assertEqual(props2.profileSymbology(), Qgis.ProfileSurfaceSymbology.FillBelow)
        self.assertTrue(props2.showMarkerSymbolInSurfacePlots())

        self.assertEqual(props2.profileLineSymbol().color().name(), '#ff4433')
        self.assertEqual(props2.profileFillSymbol().color().name(), '#ff4455')
        self.assertEqual(props2.profileMarkerSymbol().color().name(), '#ff1122')

        self.assertEqual(
            props2.dataDefinedProperties().property(QgsMapLayerElevationProperties.ZOffset).asExpression(),
            '9')

        props_clone = props.clone()
        self.assertEqual(props_clone.zScale(), 2)
        self.assertEqual(props_clone.zOffset(), 0.5)
        self.assertEqual(props_clone.clamping(), Qgis.AltitudeClamping.Relative)
        self.assertEqual(props_clone.binding(), Qgis.AltitudeBinding.Vertex)
        self.assertEqual(props_clone.extrusionHeight(), 10)
        self.assertTrue(props_clone.extrusionEnabled())
        self.assertFalse(props_clone.respectLayerSymbology())
        self.assertEqual(props_clone.type(), Qgis.VectorProfileType.ContinuousSurface)
        self.assertEqual(props_clone.profileSymbology(), Qgis.ProfileSurfaceSymbology.FillBelow)
        self.assertTrue(props_clone.showMarkerSymbolInSurfacePlots())

        self.assertEqual(props_clone.profileLineSymbol().color().name(), '#ff4433')
        self.assertEqual(props_clone.profileFillSymbol().color().name(), '#ff4455')
        self.assertEqual(props_clone.profileMarkerSymbol().color().name(), '#ff1122')

        self.assertEqual(
            props_clone.dataDefinedProperties().property(QgsMapLayerElevationProperties.ZOffset).asExpression(),
            '9')

    def test_defaults(self):
        # a layer without z values present should default to terrain clamping mode
        vl = QgsVectorLayer(unitTestDataPath() + '/3d/trees.shp')
        self.assertTrue(vl.isValid())

        self.assertEqual(vl.elevationProperties().clamping(), Qgis.AltitudeClamping.Terrain)

        # a layer WITH z values present should default to relative mode
        vl = QgsVectorLayer(unitTestDataPath() + '/3d/points_with_z.shp')
        self.assertTrue(vl.isValid())

        self.assertEqual(vl.elevationProperties().clamping(), Qgis.AltitudeClamping.Relative)

    def test_show_by_default(self):
        props = QgsVectorLayerElevationProperties(None)
        self.assertFalse(props.showByDefaultInElevationProfilePlots())

        props = QgsVectorLayerElevationProperties(None)
        props.setZOffset(1)
        self.assertTrue(props.showByDefaultInElevationProfilePlots())

        props = QgsVectorLayerElevationProperties(None)
        props.setZScale(2)
        self.assertTrue(props.showByDefaultInElevationProfilePlots())

        props = QgsVectorLayerElevationProperties(None)
        props.setExtrusionEnabled(True)
        self.assertTrue(props.showByDefaultInElevationProfilePlots())

        props = QgsVectorLayerElevationProperties(None)
        props.setClamping(Qgis.AltitudeClamping.Absolute)
        self.assertTrue(props.showByDefaultInElevationProfilePlots())


if __name__ == '__main__':
    unittest.main()
