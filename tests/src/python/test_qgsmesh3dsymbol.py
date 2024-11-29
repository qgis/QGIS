"""QGIS Unit tests for QgsMesh3DSymbol

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtGui import QColor
from qgis.core import Qgis, QgsProperty, QgsAbstract3DSymbol
from qgis._3d import (
    QgsMesh3DSymbol,
    Qgs3DTypes,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsQgsMesh3DSymbol(QgisTestCase):

    def test_getters_and_setters(self):
        symbol = QgsMesh3DSymbol()

        # Test enabled
        self.assertTrue(symbol.isEnabled())
        symbol.setEnabled(False)
        self.assertFalse(symbol.isEnabled())

        # Test culling mode
        self.assertEqual(symbol.cullingMode(), Qgs3DTypes.NoCulling)
        symbol.setCullingMode(Qgs3DTypes.Front)
        self.assertEqual(symbol.cullingMode(), Qgs3DTypes.Front)

        # Test altitude clamping
        self.assertEqual(symbol.altitudeClamping(), Qgis.AltitudeClamping.Relative)
        symbol.setAltitudeClamping(Qgis.AltitudeClamping.Absolute)
        self.assertEqual(symbol.altitudeClamping(), Qgis.AltitudeClamping.Absolute)

        # Test height
        self.assertEqual(symbol.height(), 0.0)
        symbol.setHeight(10.5)
        self.assertEqual(symbol.height(), 10.5)

        # Test add back faces
        self.assertFalse(symbol.addBackFaces())
        symbol.setAddBackFaces(True)
        self.assertTrue(symbol.addBackFaces())

        # Test smoothed triangles
        self.assertFalse(symbol.smoothedTriangles())
        symbol.setSmoothedTriangles(True)
        self.assertTrue(symbol.smoothedTriangles())

        # Test wireframe settings
        self.assertFalse(symbol.wireframeEnabled())
        symbol.setWireframeEnabled(True)
        self.assertTrue(symbol.wireframeEnabled())

        self.assertEqual(symbol.wireframeLineWidth(), 1.0)
        symbol.setWireframeLineWidth(2.5)
        self.assertEqual(symbol.wireframeLineWidth(), 2.5)

        default_color = QColor(Qt.GlobalColor.darkGray)
        self.assertEqual(symbol.wireframeLineColor(), default_color)
        new_color = QColor(255, 0, 0)
        symbol.setWireframeLineColor(new_color)
        self.assertEqual(symbol.wireframeLineColor(), new_color)

        # Test vertical settings
        self.assertEqual(symbol.verticalScale(), 1.0)
        symbol.setVerticalScale(2.0)
        self.assertEqual(symbol.verticalScale(), 2.0)

        self.assertEqual(symbol.verticalDatasetGroupIndex(), -1)
        symbol.setVerticalDatasetGroupIndex(1)
        self.assertEqual(symbol.verticalDatasetGroupIndex(), 1)

        self.assertFalse(symbol.isVerticalMagnitudeRelative())
        symbol.setIsVerticalMagnitudeRelative(True)
        self.assertTrue(symbol.isVerticalMagnitudeRelative())

        # Test rendering style
        self.assertEqual(
            symbol.renderingStyle(), QgsMesh3DSymbol.RenderingStyle.SingleColor
        )
        symbol.setRenderingStyle(QgsMesh3DSymbol.RenderingStyle.ColorRamp)
        self.assertEqual(
            symbol.renderingStyle(), QgsMesh3DSymbol.RenderingStyle.ColorRamp
        )

        default_color = QColor(Qt.GlobalColor.darkGreen)
        self.assertEqual(symbol.singleMeshColor(), default_color)
        new_color = QColor(0, 255, 0)
        symbol.setSingleMeshColor(new_color)
        self.assertEqual(symbol.singleMeshColor(), new_color)

        # Test arrows
        self.assertFalse(symbol.arrowsEnabled())
        symbol.setArrowsEnabled(True)
        self.assertTrue(symbol.arrowsEnabled())

        self.assertEqual(symbol.arrowsSpacing(), 25)
        symbol.setArrowsSpacing(50)
        self.assertEqual(symbol.arrowsSpacing(), 50)

        self.assertFalse(symbol.arrowsFixedSize())
        symbol.setArrowsFixedSize(True)
        self.assertTrue(symbol.arrowsFixedSize())

        # Test texture size
        self.assertEqual(symbol.maximumTextureSize(), 1024)
        symbol.setMaximumTextureSize(2048)
        self.assertEqual(symbol.maximumTextureSize(), 2048)

        # Test level of detail
        self.assertEqual(symbol.levelOfDetailIndex(), 0)
        symbol.setLevelOfDetailIndex(1)
        self.assertEqual(symbol.levelOfDetailIndex(), 1)

        symbol.dataDefinedProperties().setProperty(
            QgsAbstract3DSymbol.Property.Height, QgsProperty.fromExpression("1+2")
        )
        self.assertEqual(
            symbol.dataDefinedProperties()
            .property(QgsAbstract3DSymbol.Property.Height)
            .asExpression(),
            "1+2",
        )

    def test_equality(self):
        symbol1 = QgsMesh3DSymbol()
        symbol2 = QgsMesh3DSymbol()

        self.assertEqual(symbol1, symbol2)

        # Test enabled
        symbol2.setEnabled(False)
        self.assertNotEqual(symbol1, symbol2)
        symbol2.setEnabled(True)
        self.assertEqual(symbol1, symbol2)

        # Test culling mode
        symbol2.setCullingMode(Qgs3DTypes.Front)
        self.assertNotEqual(symbol1, symbol2)
        symbol2.setCullingMode(Qgs3DTypes.NoCulling)
        self.assertEqual(symbol1, symbol2)

        # Test altitude clamping
        symbol2.setAltitudeClamping(Qgis.AltitudeClamping.Absolute)
        self.assertNotEqual(symbol1, symbol2)
        symbol2.setAltitudeClamping(Qgis.AltitudeClamping.Relative)
        self.assertEqual(symbol1, symbol2)

        # Test height
        symbol2.setHeight(10.5)
        self.assertNotEqual(symbol1, symbol2)
        symbol2.setHeight(0.0)
        self.assertEqual(symbol1, symbol2)

        # Test back faces
        symbol2.setAddBackFaces(True)
        self.assertNotEqual(symbol1, symbol2)
        symbol2.setAddBackFaces(False)
        self.assertEqual(symbol1, symbol2)

        # Test smoothed triangles
        symbol2.setSmoothedTriangles(True)
        self.assertNotEqual(symbol1, symbol2)
        symbol2.setSmoothedTriangles(False)
        self.assertEqual(symbol1, symbol2)

        # Test wireframe enabled
        symbol2.setWireframeEnabled(True)
        self.assertNotEqual(symbol1, symbol2)
        symbol2.setWireframeEnabled(False)
        self.assertEqual(symbol1, symbol2)

        # Test wireframe width
        symbol2.setWireframeLineWidth(2.5)
        self.assertNotEqual(symbol1, symbol2)
        symbol2.setWireframeLineWidth(1.0)
        self.assertEqual(symbol1, symbol2)

        # Test wireframe color
        symbol2.setWireframeLineColor(QColor(255, 0, 0))
        self.assertNotEqual(symbol1, symbol2)
        symbol2.setWireframeLineColor(QColor(Qt.GlobalColor.darkGray))
        self.assertEqual(symbol1, symbol2)

        # Test vertical scale
        symbol2.setVerticalScale(2.0)
        self.assertNotEqual(symbol1, symbol2)
        symbol2.setVerticalScale(1.0)
        self.assertEqual(symbol1, symbol2)

        # Test vertical dataset group index
        symbol2.setVerticalDatasetGroupIndex(1)
        self.assertNotEqual(symbol1, symbol2)
        symbol2.setVerticalDatasetGroupIndex(-1)
        self.assertEqual(symbol1, symbol2)

        # Test vertical magnitude relative
        symbol2.setIsVerticalMagnitudeRelative(True)
        self.assertNotEqual(symbol1, symbol2)
        symbol2.setIsVerticalMagnitudeRelative(False)
        self.assertEqual(symbol1, symbol2)

        # Test rendering style
        symbol2.setRenderingStyle(QgsMesh3DSymbol.RenderingStyle.ColorRamp)
        self.assertNotEqual(symbol1, symbol2)
        symbol2.setRenderingStyle(QgsMesh3DSymbol.RenderingStyle.SingleColor)
        self.assertEqual(symbol1, symbol2)

        # Test single mesh color
        symbol2.setSingleMeshColor(QColor(0, 255, 0))
        self.assertNotEqual(symbol1, symbol2)
        symbol2.setSingleMeshColor(QColor(Qt.GlobalColor.darkGreen))
        self.assertEqual(symbol1, symbol2)

        # Test arrows enabled
        symbol2.setArrowsEnabled(True)
        self.assertNotEqual(symbol1, symbol2)
        symbol2.setArrowsEnabled(False)
        self.assertEqual(symbol1, symbol2)

        # Test arrows spacing
        symbol2.setArrowsSpacing(50)
        self.assertNotEqual(symbol1, symbol2)
        symbol2.setArrowsSpacing(25)
        self.assertEqual(symbol1, symbol2)

        # Test arrows fixed size
        symbol2.setArrowsFixedSize(True)
        self.assertNotEqual(symbol1, symbol2)
        symbol2.setArrowsFixedSize(False)
        self.assertEqual(symbol1, symbol2)

        # Test maximum texture size
        symbol2.setMaximumTextureSize(2048)
        self.assertNotEqual(symbol1, symbol2)
        symbol2.setMaximumTextureSize(1024)
        self.assertEqual(symbol1, symbol2)

        # Test level of detail index
        symbol2.setLevelOfDetailIndex(1)
        self.assertNotEqual(symbol1, symbol2)
        symbol2.setLevelOfDetailIndex(0)
        self.assertEqual(symbol1, symbol2)

        symbol2.dataDefinedProperties().setProperty(
            QgsAbstract3DSymbol.Property.Height, QgsProperty.fromExpression("1+2")
        )
        self.assertNotEqual(symbol1, symbol2)


if __name__ == "__main__":
    unittest.main()
