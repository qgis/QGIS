"""
***************************************************************************
    test_qgssymbollayer.py
    ---------------------
    Date                 : October 2012
    Copyright            : (C) 2012 by Massimo Endrighi
    Email                : massimo dot endrighi at geopartner dot it
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************

From build dir, run: ctest -R PyQgsSymbolLayer -V

"""

__author__ = "Massimo Endrighi"
__date__ = "October 2012"
__copyright__ = "(C) 2012, Massimo Endrighi"

import os

import qgis.core
from osgeo import ogr
from qgis.PyQt.QtCore import (
    QDir,
    QFile,
    QIODevice,
    QObject,
    QPointF,
    QSize,
    Qt,
    QTemporaryDir,
)
from qgis.PyQt.QtGui import QColor, QImage, QPainter
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    Qgis,
    QgsArrowSymbolLayer,
    QgsCategorizedSymbolRenderer,
    QgsCentroidFillSymbolLayer,
    QgsEllipseSymbolLayer,
    QgsFeature,
    QgsFilledMarkerSymbolLayer,
    QgsFillSymbol,
    QgsFillSymbolLayer,
    QgsFontMarkerSymbolLayer,
    QgsGeometry,
    QgsGradientFillSymbolLayer,
    QgsImageFillSymbolLayer,
    QgsLinePatternFillSymbolLayer,
    QgsLineSymbol,
    QgsLineSymbolLayer,
    QgsMapSettings,
    QgsMarkerLineSymbolLayer,
    QgsMarkerSymbol,
    QgsMarkerSymbolLayer,
    QgsPointPatternFillSymbolLayer,
    QgsProject,
    QgsProperty,
    QgsRasterFillSymbolLayer,
    QgsReadWriteContext,
    QgsRectangle,
    QgsRenderContext,
    QgsRendererCategory,
    QgsShapeburstFillSymbolLayer,
    QgsSimpleFillSymbolLayer,
    QgsSimpleLineSymbolLayer,
    QgsSimpleMarkerSymbolLayer,
    QgsSimpleMarkerSymbolLayerBase,
    QgsSingleSymbolRenderer,
    QgsSVGFillSymbolLayer,
    QgsSvgMarkerSymbolLayer,
    QgsSymbolLayer,
    QgsSymbolLayerUtils,
    QgsUnitTypes,
    QgsVectorFieldSymbolLayer,
    QgsVectorLayer,
    QgsSymbolRenderContext,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

# Convenience instances in case you may need them
# not used in this test
start_app()

TEST_DATA_DIR = unitTestDataPath()

EXPECTED_TYPE = type(QObject)


class TestQgsSymbolLayer(QgisTestCase):
    """
    This class test the sip binding for QgsSymbolLayer descendants
    Every class is tested using the createFromSld implementation
    An exception is done for:
    - QgsLinePatternFillSymbolLayer where createFromSld implementation
        returns NULL
    - QgsPointPatternFillSymbolLayer where createFromSld implementation
        returns NULL
    - QgsVectorFieldSymbolLayer where createFromSld implementation
        returns NULL
    """

    @classmethod
    def control_path_prefix(cls):
        return "symbol_layer"

    def testBinding(self):
        """Test python bindings existence."""
        mType = type(QgsSymbolLayer)
        mMessage = f'Expected "{EXPECTED_TYPE}" got "{mType}"'
        assert EXPECTED_TYPE == mType, mMessage

        try:
            mType = type(QgsFillSymbolLayer)
        except:
            mType = None
        mMessage = f'Expected "{EXPECTED_TYPE}" got "{mType}"'
        assert EXPECTED_TYPE == mType, mMessage

        try:
            mType = type(QgsGradientFillSymbolLayer)
        except:
            mType = None
        mMessage = f'Expected "{EXPECTED_TYPE}" got "{mType}"'
        assert EXPECTED_TYPE == mType, mMessage

        try:
            mType = type(QgsLinePatternFillSymbolLayer)
        except:
            mType = None
        mMessage = f'Expected "{EXPECTED_TYPE}" got "{mType}"'
        assert EXPECTED_TYPE == mType, mMessage

        try:
            mType = type(QgsPointPatternFillSymbolLayer)
        except:
            mType = None
        mMessage = f'Expected "{EXPECTED_TYPE}" got "{mType}"'
        assert EXPECTED_TYPE == mType, mMessage

        try:
            mType = type(QgsImageFillSymbolLayer)
        except:
            mType = None
        mMessage = f'Expected "{EXPECTED_TYPE}" got "{mType}"'
        assert EXPECTED_TYPE == mType, mMessage

        try:
            mType = type(QgsPointPatternFillSymbolLayer)
        except:
            mType = None
        mMessage = f'Expected "{EXPECTED_TYPE}" got "{mType}"'
        assert EXPECTED_TYPE == mType, mMessage

        try:
            mType = type(QgsGradientFillSymbolLayer)
        except:
            mType = None
        mMessage = f'Expected "{EXPECTED_TYPE}" got "{mType}"'
        assert EXPECTED_TYPE == mType, mMessage

        try:
            mType = type(QgsShapeburstFillSymbolLayer)
        except:
            mType = None
        mMessage = f'Expected "{EXPECTED_TYPE}" got "{mType}"'
        assert EXPECTED_TYPE == mType, mMessage

        try:
            mType = type(QgsSVGFillSymbolLayer)
        except:
            mType = None
        mMessage = f'Expected "{EXPECTED_TYPE}" got "{mType}"'
        assert EXPECTED_TYPE == mType, mMessage

        try:
            mType = type(QgsCentroidFillSymbolLayer)
        except:
            mType = None
        mMessage = f'Expected "{EXPECTED_TYPE}" got "{mType}"'
        assert EXPECTED_TYPE == mType, mMessage

        try:
            mType = type(QgsRasterFillSymbolLayer)
        except:
            mType = None
        mMessage = f'Expected "{EXPECTED_TYPE}" got "{mType}"'
        assert EXPECTED_TYPE == mType, mMessage

        try:
            mType = type(QgsSimpleFillSymbolLayer)
        except:
            mType = None
        mMessage = f'Expected "{EXPECTED_TYPE}" got "{mType}"'
        assert EXPECTED_TYPE == mType, mMessage

        try:
            mType = type(QgsLineSymbolLayer)
        except:
            mType = None
        mMessage = f'Expected "{EXPECTED_TYPE}" got "{mType}"'
        assert EXPECTED_TYPE == mType, mMessage

        try:
            mType = type(QgsMarkerLineSymbolLayer)
        except:
            mType = None
        mMessage = f'Expected "{EXPECTED_TYPE}" got "{mType}"'
        assert EXPECTED_TYPE == mType, mMessage

        try:
            mType = type(QgsArrowSymbolLayer)
        except:
            mType = None
        mMessage = f'Expected "{EXPECTED_TYPE}" got "{mType}"'
        assert EXPECTED_TYPE == mType, mMessage

        try:
            mType = type(QgsSimpleLineSymbolLayer)
        except:
            mType = None
        mMessage = f'Expected "{EXPECTED_TYPE}" got "{mType}"'
        assert EXPECTED_TYPE == mType, mMessage

        try:
            mType = type(QgsMarkerSymbolLayer)
        except:
            mType = None
        mMessage = f'Expected "{EXPECTED_TYPE}" got "{mType}"'
        assert EXPECTED_TYPE == mType, mMessage

        try:
            mType = type(QgsEllipseSymbolLayer)
        except:
            mType = None
        mMessage = f'Expected "{EXPECTED_TYPE}" got "{mType}"'
        assert EXPECTED_TYPE == mType, mMessage

        try:
            mType = type(QgsFontMarkerSymbolLayer)
        except:
            mType = None
        mMessage = f'Expected "{EXPECTED_TYPE}" got "{mType}"'
        assert EXPECTED_TYPE == mType, mMessage

        try:
            mType = type(QgsSimpleMarkerSymbolLayer)
        except:
            mType = None
        mMessage = f'Expected "{EXPECTED_TYPE}" got "{mType}"'
        assert EXPECTED_TYPE == mType, mMessage

        try:
            mType = type(QgsFilledMarkerSymbolLayer)
        except:
            mType = None
        mMessage = f'Expected "{EXPECTED_TYPE}" got "{mType}"'
        assert EXPECTED_TYPE == mType, mMessage

        try:
            mType = type(QgsSvgMarkerSymbolLayer)
        except:
            mType = None
        mMessage = f'Expected "{EXPECTED_TYPE}" got "{mType}"'
        assert EXPECTED_TYPE == mType, mMessage

        try:
            mType = type(QgsVectorFieldSymbolLayer)
        except:
            mType = None
        mMessage = f'Expected "{EXPECTED_TYPE}" got "{mType}"'
        assert EXPECTED_TYPE == mType, mMessage

    def testGettersSetters(self):
        """test base class getters/setters"""
        layer = QgsSimpleFillSymbolLayer()

        layer.setEnabled(False)
        self.assertFalse(layer.enabled())
        layer.setEnabled(True)
        self.assertTrue(layer.enabled())

        layer.setLocked(False)
        self.assertFalse(layer.isLocked())
        layer.setLocked(True)
        self.assertTrue(layer.isLocked())

        layer.setRenderingPass(5)
        self.assertEqual(layer.renderingPass(), 5)

    def testSaveRestore(self):
        """Test saving and restoring base symbol layer properties to xml"""

        layer = QgsSimpleFillSymbolLayer()
        layer.setEnabled(False)
        layer.setLocked(True)
        layer.setRenderingPass(5)
        layer.setUserFlags(Qgis.SymbolLayerUserFlag.DisableSelectionRecoloring)

        symbol = QgsFillSymbol()
        symbol.changeSymbolLayer(0, layer)

        doc = QDomDocument("testdoc")
        elem = QgsSymbolLayerUtils.saveSymbol(
            "test", symbol, doc, QgsReadWriteContext()
        )

        restored_symbol = QgsSymbolLayerUtils.loadSymbol(elem, QgsReadWriteContext())
        restored_layer = restored_symbol.symbolLayer(0)
        self.assertFalse(restored_layer.enabled())
        self.assertTrue(restored_layer.isLocked())
        self.assertEqual(restored_layer.renderingPass(), 5)
        self.assertEqual(
            restored_layer.userFlags(),
            Qgis.SymbolLayerUserFlag.DisableSelectionRecoloring,
        )

    def testClone(self):
        """test that base symbol layer properties are cloned with layer"""

        layer = QgsSimpleFillSymbolLayer()
        layer.setEnabled(False)
        layer.setLocked(True)
        layer.setRenderingPass(5)
        layer.setUserFlags(Qgis.SymbolLayerUserFlag.DisableSelectionRecoloring)

        symbol = QgsFillSymbol()
        symbol.changeSymbolLayer(0, layer)

        cloned_symbol = symbol.clone()
        cloned_layer = cloned_symbol.symbolLayer(0)
        self.assertFalse(cloned_layer.enabled())
        self.assertTrue(cloned_layer.isLocked())
        self.assertEqual(cloned_layer.renderingPass(), 5)
        self.assertEqual(
            cloned_layer.userFlags(),
            Qgis.SymbolLayerUserFlag.DisableSelectionRecoloring,
        )

    def testRenderFillLayerDisabled(self):
        """test that rendering a fill symbol with disabled layer works"""
        layer = QgsSimpleFillSymbolLayer()
        layer.setEnabled(False)

        symbol = QgsFillSymbol()
        symbol.changeSymbolLayer(0, layer)

        image = QImage(200, 200, QImage.Format.Format_RGB32)
        painter = QPainter()
        ms = QgsMapSettings()

        geom = QgsGeometry.fromWkt("Polygon ((0 0, 10 0, 10 10, 0 10, 0 0))")
        f = QgsFeature()
        f.setGeometry(geom)

        extent = geom.constGet().boundingBox()
        # buffer extent by 10%
        extent = extent.buffered((extent.height() + extent.width()) / 20.0)

        ms.setExtent(extent)
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI

        painter.begin(image)
        image.fill(QColor(255, 255, 255))

        symbol.startRender(context)
        symbol.renderFeature(f, context)
        symbol.stopRender(context)
        painter.end()

        self.assertTrue(
            self.image_check("symbollayer_disabled", "symbollayer_disabled", image)
        )

    def testRenderFillLayerDataDefined(self):
        """
        Test that rendering a fill symbol with data defined enabled layer works
        """

        polys_shp = os.path.join(TEST_DATA_DIR, "polys.shp")
        polys_layer = QgsVectorLayer(polys_shp, "Polygons", "ogr")
        QgsProject.instance().addMapLayer(polys_layer)

        layer = QgsSimpleFillSymbolLayer()
        layer.setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyLayerEnabled,
            QgsProperty.fromExpression("Name='Lake'"),
        )
        layer.setStrokeStyle(Qt.PenStyle.NoPen)
        layer.setColor(QColor(100, 150, 150))

        symbol = QgsFillSymbol()
        symbol.changeSymbolLayer(0, layer)
        polys_layer.setRenderer(QgsSingleSymbolRenderer(symbol))

        ms = QgsMapSettings()
        ms.setOutputSize(QSize(400, 400))
        ms.setOutputDpi(96)
        ms.setExtent(QgsRectangle(-133, 22, -70, 52))
        ms.setLayers([polys_layer])

        # Test usedAttributes
        ctx = QgsRenderContext.fromMapSettings(ms)
        ctx.expressionContext().appendScope(polys_layer.createExpressionContextScope())
        # for symbol layer
        self.assertCountEqual(layer.usedAttributes(ctx), {"Name"})
        # for symbol
        self.assertCountEqual(symbol.usedAttributes(ctx), {"Name"})
        # for symbol renderer
        self.assertCountEqual(polys_layer.renderer().usedAttributes(ctx), {"Name"})

        # Test rendering
        self.assertTrue(
            self.render_map_settings_check(
                "filllayer_ddenabled", "filllayer_ddenabled", ms
            )
        )

        QgsProject.instance().removeMapLayer(polys_layer)

    def testRenderLineLayerDisabled(self):
        """
        Test that rendering a line symbol with disabled layer works
        """
        layer = QgsSimpleLineSymbolLayer()
        layer.setEnabled(False)

        symbol = QgsLineSymbol()
        symbol.changeSymbolLayer(0, layer)

        image = QImage(200, 200, QImage.Format.Format_RGB32)
        painter = QPainter()
        ms = QgsMapSettings()

        geom = QgsGeometry.fromWkt("LineString (0 0,3 4,4 3)")
        f = QgsFeature()
        f.setGeometry(geom)

        extent = geom.constGet().boundingBox()
        # buffer extent by 10%
        extent = extent.buffered((extent.height() + extent.width()) / 20.0)

        ms.setExtent(extent)
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI

        painter.begin(image)
        image.fill(QColor(255, 255, 255))

        symbol.startRender(context)
        symbol.renderFeature(f, context)
        symbol.stopRender(context)
        painter.end()

        self.assertTrue(
            self.image_check("symbollayer_disabled", "symbollayer_disabled", image)
        )

    def testRenderLineLayerDataDefined(self):
        """
        Test that rendering a line symbol with data defined enabled layer works
        """

        lines_shp = os.path.join(TEST_DATA_DIR, "lines.shp")
        lines_layer = QgsVectorLayer(lines_shp, "Lines", "ogr")
        QgsProject.instance().addMapLayer(lines_layer)

        layer = QgsSimpleLineSymbolLayer()
        layer.setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyLayerEnabled,
            QgsProperty.fromExpression("Name='Highway'"),
        )
        layer.setColor(QColor(100, 150, 150))
        layer.setWidth(5)

        symbol = QgsLineSymbol()
        symbol.changeSymbolLayer(0, layer)
        lines_layer.setRenderer(QgsSingleSymbolRenderer(symbol))

        ms = QgsMapSettings()
        ms.setOutputSize(QSize(400, 400))
        ms.setOutputDpi(96)
        ms.setExtent(QgsRectangle(-133, 22, -70, 52))
        ms.setLayers([lines_layer])

        # Test usedAttributes
        ctx = QgsRenderContext.fromMapSettings(ms)
        ctx.expressionContext().appendScope(lines_layer.createExpressionContextScope())
        # for symbol layer
        self.assertCountEqual(layer.usedAttributes(ctx), {"Name"})
        # for symbol
        self.assertCountEqual(symbol.usedAttributes(ctx), {"Name"})
        # for symbol renderer
        self.assertCountEqual(lines_layer.renderer().usedAttributes(ctx), {"Name"})

        # Test rendering
        self.assertTrue(
            self.render_map_settings_check(
                "linelayer_ddenabled", "linelayer_ddenabled", ms
            )
        )

        QgsProject.instance().removeMapLayer(lines_layer)

    def testRenderMarkerLayerDisabled(self):
        """
        Test that rendering a marker symbol with disabled layer works
        """
        layer = QgsSimpleMarkerSymbolLayer()
        layer.setEnabled(False)

        symbol = QgsMarkerSymbol()
        symbol.changeSymbolLayer(0, layer)

        image = QImage(200, 200, QImage.Format.Format_RGB32)
        painter = QPainter()
        ms = QgsMapSettings()

        geom = QgsGeometry.fromWkt("Point (1 2)")
        f = QgsFeature()
        f.setGeometry(geom)

        extent = QgsRectangle(0, 0, 4, 4)

        ms.setExtent(extent)
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI

        painter.begin(image)
        image.fill(QColor(255, 255, 255))

        symbol.startRender(context)
        symbol.renderFeature(f, context)
        symbol.stopRender(context)
        painter.end()

        self.assertTrue(
            self.image_check("symbollayer_disabled", "symbollayer_disabled", image)
        )

    def testRenderMarkerLayerDataDefined(self):
        """
        Test that rendering a marker symbol with data defined enabled
        layer works
        """

        points_shp = os.path.join(TEST_DATA_DIR, "points.shp")
        points_layer = QgsVectorLayer(points_shp, "Points", "ogr")
        QgsProject.instance().addMapLayer(points_layer)

        layer = QgsSimpleMarkerSymbolLayer()
        layer.setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyLayerEnabled,
            QgsProperty.fromExpression("Class='Biplane'"),
        )
        layer.setColor(QColor(100, 150, 150))
        layer.setSize(5)
        layer.setStrokeStyle(Qt.PenStyle.NoPen)

        symbol = QgsMarkerSymbol()
        symbol.changeSymbolLayer(0, layer)
        points_layer.setRenderer(QgsSingleSymbolRenderer(symbol))

        ms = QgsMapSettings()
        ms.setOutputSize(QSize(400, 400))
        ms.setOutputDpi(96)
        ms.setExtent(QgsRectangle(-133, 22, -70, 52))
        ms.setLayers([points_layer])

        # Test usedAttributes
        ctx = QgsRenderContext.fromMapSettings(ms)
        ctx.expressionContext().appendScope(points_layer.createExpressionContextScope())
        # for symbol layer
        self.assertCountEqual(layer.usedAttributes(ctx), {"Class"})
        # for symbol
        self.assertCountEqual(symbol.usedAttributes(ctx), {"Class"})
        # for symbol renderer
        self.assertCountEqual(points_layer.renderer().usedAttributes(ctx), {"Class"})

        # Test rendering
        self.assertTrue(
            self.render_map_settings_check(
                "markerlayer_ddenabled", "markerlayer_ddenabled", ms
            )
        )
        QgsProject.instance().removeMapLayer(points_layer)

    def testQgsSimpleFillSymbolLayer(self):
        """
        Create a new style from a .sld file and match test.
        """
        mTestName = "QgsSimpleFillSymbolLayer"
        mFilePath = QDir.toNativeSeparators(
            f"{unitTestDataPath()}/symbol_layer/{mTestName}.sld"
        )

        mDoc = QDomDocument(mTestName)
        mFile = QFile(mFilePath)
        mFile.open(QIODevice.OpenModeFlag.ReadOnly)
        mDoc.setContent(mFile, True)
        mFile.close()
        mSymbolLayer = QgsSimpleFillSymbolLayer.createFromSld(
            mDoc.elementsByTagName("PolygonSymbolizer").item(0).toElement()
        )

        mExpectedValue = type(QgsSimpleFillSymbolLayer())
        mValue = type(mSymbolLayer)
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = Qt.BrushStyle.SolidPattern
        mValue = mSymbolLayer.brushStyle()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = "#ffaa7f"
        mValue = mSymbolLayer.strokeColor().name()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = Qt.PenStyle.DotLine
        mValue = mSymbolLayer.strokeStyle()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 0.26
        mValue = mSymbolLayer.strokeWidth()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        ctx = QgsRenderContext()
        self.assertCountEqual(mSymbolLayer.usedAttributes(ctx), {})

    def testQgsGradientFillSymbolLayer(self):
        """
        Test setting and getting QgsGradientFillSymbolLayer properties.
        """
        mGradientLayer = QgsGradientFillSymbolLayer()

        mExpectedValue = type(QgsGradientFillSymbolLayer())
        mValue = type(mGradientLayer)
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QgsGradientFillSymbolLayer.GradientType.Radial
        mGradientLayer.setGradientType(mExpectedValue)
        mValue = mGradientLayer.gradientType()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QgsGradientFillSymbolLayer.GradientColorType.ColorRamp
        mGradientLayer.setGradientColorType(mExpectedValue)
        mValue = mGradientLayer.gradientColorType()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QColor("#55aaff")
        mGradientLayer.setColor2(mExpectedValue)
        mValue = mGradientLayer.color2()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QgsGradientFillSymbolLayer.GradientCoordinateMode.Viewport
        mGradientLayer.setCoordinateMode(mExpectedValue)
        mValue = mGradientLayer.coordinateMode()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QgsGradientFillSymbolLayer.GradientSpread.Reflect
        mGradientLayer.setGradientSpread(mExpectedValue)
        mValue = mGradientLayer.gradientSpread()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QPointF(0.5, 0.8)
        mGradientLayer.setReferencePoint1(mExpectedValue)
        mValue = mGradientLayer.referencePoint1()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = True
        mGradientLayer.setReferencePoint1IsCentroid(mExpectedValue)
        mValue = mGradientLayer.referencePoint1IsCentroid()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QPointF(0.2, 0.4)
        mGradientLayer.setReferencePoint2(mExpectedValue)
        mValue = mGradientLayer.referencePoint2()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = True
        mGradientLayer.setReferencePoint2IsCentroid(mExpectedValue)
        mValue = mGradientLayer.referencePoint2IsCentroid()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 90
        mGradientLayer.setAngle(mExpectedValue)
        mValue = mGradientLayer.angle()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QPointF(10, 20)
        mGradientLayer.setOffset(mExpectedValue)
        mValue = mGradientLayer.offset()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QgsUnitTypes.RenderUnit.RenderMapUnits
        mGradientLayer.setOffsetUnit(mExpectedValue)
        mValue = mGradientLayer.offsetUnit()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        ctx = QgsRenderContext()
        self.assertCountEqual(mGradientLayer.usedAttributes(ctx), {})

    def testQgsCentroidFillSymbolLayer(self):
        """
        Create a new style from a .sld file and match test
        """
        mTestName = "QgsCentroidFillSymbolLayer"
        mFilePath = QDir.toNativeSeparators(
            f"{unitTestDataPath()}/symbol_layer/{mTestName}.sld"
        )

        mDoc = QDomDocument(mTestName)
        mFile = QFile(mFilePath)
        mFile.open(QIODevice.OpenModeFlag.ReadOnly)
        mDoc.setContent(mFile, True)
        mFile.close()
        mSymbolLayer = QgsCentroidFillSymbolLayer.createFromSld(
            mDoc.elementsByTagName("PointSymbolizer").item(0).toElement()
        )

        mExpectedValue = type(QgsCentroidFillSymbolLayer())
        mValue = type(mSymbolLayer)
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QgsSimpleMarkerSymbolLayerBase.Shape.Star
        mValue = mSymbolLayer.subSymbol().symbolLayer(0).shape()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = "#55aaff"
        mValue = mSymbolLayer.subSymbol().symbolLayer(0).color().name()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = "#00ff00"
        mValue = mSymbolLayer.subSymbol().symbolLayer(0).strokeColor().name()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = False
        mValue = mSymbolLayer.pointOnAllParts()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        ctx = QgsRenderContext()
        self.assertCountEqual(mSymbolLayer.usedAttributes(ctx), {})

        # test colors, need to make sure colors are passed/retrieved from subsymbol
        mSymbolLayer.setColor(QColor(150, 50, 100))
        self.assertEqual(mSymbolLayer.color(), QColor(150, 50, 100))
        self.assertEqual(mSymbolLayer.subSymbol().color(), QColor(150, 50, 100))
        mSymbolLayer.subSymbol().setColor(QColor(250, 150, 200))
        self.assertEqual(mSymbolLayer.subSymbol().color(), QColor(250, 150, 200))
        self.assertEqual(mSymbolLayer.color(), QColor(250, 150, 200))

    def testQgsLinePatternFillSymbolLayer(self):
        """
        Create a new style from a .sld file and match test
        """
        mTestName = "QgsLinePatternFillSymbolLayer"
        mFilePath = QDir.toNativeSeparators(
            f"{unitTestDataPath()}/symbol_layer/{mTestName}.sld"
        )

        mDoc = QDomDocument(mTestName)
        mFile = QFile(mFilePath)
        mFile.open(QIODevice.OpenModeFlag.ReadOnly)
        mDoc.setContent(mFile, True)
        mFile.close()
        mSymbolLayer = QgsLinePatternFillSymbolLayer.createFromSld(
            mDoc.elementsByTagName("PolygonSymbolizer").item(0).toElement()
        )

        mExpectedValue = type(QgsLinePatternFillSymbolLayer())
        mValue = type(mSymbolLayer)
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = "#ff55ff"
        mValue = mSymbolLayer.color().name()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 1.5
        mValue = mSymbolLayer.lineWidth()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 4
        mValue = mSymbolLayer.distance()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 57
        mValue = mSymbolLayer.lineAngle()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        ctx = QgsRenderContext()
        self.assertCountEqual(mSymbolLayer.usedAttributes(ctx), {})

        # test colors, need to make sure colors are passed/retrieved from subsymbol
        mSymbolLayer.setColor(QColor(150, 50, 100))
        self.assertEqual(mSymbolLayer.color(), QColor(150, 50, 100))
        self.assertEqual(mSymbolLayer.subSymbol().color(), QColor(150, 50, 100))
        mSymbolLayer.subSymbol().setColor(QColor(250, 150, 200))
        self.assertEqual(mSymbolLayer.subSymbol().color(), QColor(250, 150, 200))
        self.assertEqual(mSymbolLayer.color(), QColor(250, 150, 200))

    def testQgsPointPatternFillSymbolLayerSld(self):
        """
        Create a new style from a .sld file and match test
        """
        # at the moment there is an empty createFromSld implementation
        # that return nulls
        mTestName = "QgsPointPatternFillSymbolLayer"
        mFilePath = QDir.toNativeSeparators(
            f"{unitTestDataPath()}/symbol_layer/{mTestName}.sld"
        )

        mDoc = QDomDocument(mTestName)
        mFile = QFile(mFilePath)
        mFile.open(QIODevice.OpenModeFlag.ReadOnly)
        mDoc.setContent(mFile, True)
        mFile.close()
        mSymbolLayer = QgsPointPatternFillSymbolLayer.createFromSld(
            mDoc.elementsByTagName("PolygonSymbolizer").item(0).toElement()
        )

        mExpectedValue = type(QgsPointPatternFillSymbolLayer())
        mValue = type(mSymbolLayer)
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QgsSimpleMarkerSymbolLayerBase.Shape.Triangle
        mValue = mSymbolLayer.subSymbol().symbolLayer(0).shape()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = "#ffaa00"
        mValue = mSymbolLayer.subSymbol().symbolLayer(0).color().name()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = "#ff007f"
        mValue = mSymbolLayer.subSymbol().symbolLayer(0).strokeColor().name()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 5
        mValue = mSymbolLayer.subSymbol().symbolLayer(0).angle()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 3
        mValue = mSymbolLayer.subSymbol().symbolLayer(0).size()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        ctx = QgsRenderContext()
        self.assertCountEqual(mSymbolLayer.usedAttributes(ctx), {})

    def testQgsPointPatternFillSymbolLayer(self):
        """
        Test point pattern fill
        """
        mSymbolLayer = QgsPointPatternFillSymbolLayer.create()

        ctx = QgsRenderContext()
        self.assertCountEqual(mSymbolLayer.usedAttributes(ctx), {})

        # test colors, need to make sure colors are passed/retrieved from subsymbol
        mSymbolLayer.setColor(QColor(150, 50, 100))
        self.assertEqual(mSymbolLayer.color(), QColor(150, 50, 100))
        self.assertEqual(mSymbolLayer.subSymbol().color(), QColor(150, 50, 100))
        mSymbolLayer.subSymbol().setColor(QColor(250, 150, 200))
        self.assertEqual(mSymbolLayer.subSymbol().color(), QColor(250, 150, 200))
        self.assertEqual(mSymbolLayer.color(), QColor(250, 150, 200))

    def testQgsSVGFillSymbolLayer(self):
        """
        Create a new style from a .sld file and match test
        """
        mTestName = "QgsSVGFillSymbolLayer"
        mFilePath = QDir.toNativeSeparators(
            f"{unitTestDataPath()}/symbol_layer/{mTestName}.sld"
        )

        mDoc = QDomDocument(mTestName)
        mFile = QFile(mFilePath)
        mFile.open(QIODevice.OpenModeFlag.ReadOnly)
        mDoc.setContent(mFile, True)
        mFile.close()
        mSymbolLayer = QgsSVGFillSymbolLayer.createFromSld(
            mDoc.elementsByTagName("PolygonSymbolizer").item(0).toElement()
        )

        mExpectedValue = type(QgsSVGFillSymbolLayer(""))
        mValue = type(mSymbolLayer)
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = "accommodation_camping.svg"
        mValue = os.path.basename(mSymbolLayer.svgFilePath())
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 6
        mValue = mSymbolLayer.patternWidth()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        ctx = QgsRenderContext()
        self.assertCountEqual(mSymbolLayer.usedAttributes(ctx), {})

    def testQgsMarkerLineSymbolLayer(self):
        """
        Create a new style from a .sld file and match test
        """
        mTestName = "QgsMarkerLineSymbolLayer"
        mFilePath = QDir.toNativeSeparators(
            f"{unitTestDataPath()}/symbol_layer/{mTestName}.sld"
        )

        mDoc = QDomDocument(mTestName)
        mFile = QFile(mFilePath)
        mFile.open(QIODevice.OpenModeFlag.ReadOnly)
        mDoc.setContent(mFile, True)
        mFile.close()
        mSymbolLayer = QgsMarkerLineSymbolLayer.createFromSld(
            mDoc.elementsByTagName("LineSymbolizer").item(0).toElement()
        )

        mExpectedValue = type(QgsMarkerLineSymbolLayer())
        mValue = type(mSymbolLayer)
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QgsMarkerLineSymbolLayer.Placement.CentralPoint
        mValue = mSymbolLayer.placement()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = QgsSimpleMarkerSymbolLayerBase.Shape.Circle
        mValue = mSymbolLayer.subSymbol().symbolLayer(0).shape()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = "#000000"
        mValue = mSymbolLayer.subSymbol().symbolLayer(0).strokeColor().name()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = "#ff0000"
        mValue = mSymbolLayer.subSymbol().symbolLayer(0).color().name()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        ctx = QgsRenderContext()
        self.assertCountEqual(mSymbolLayer.usedAttributes(ctx), {})

        # test colors, need to make sure colors are passed/retrieved from subsymbol
        mSymbolLayer.setColor(QColor(150, 50, 100))
        self.assertEqual(mSymbolLayer.color(), QColor(150, 50, 100))
        self.assertEqual(mSymbolLayer.subSymbol().color(), QColor(150, 50, 100))
        mSymbolLayer.subSymbol().setColor(QColor(250, 150, 200))
        self.assertEqual(mSymbolLayer.subSymbol().color(), QColor(250, 150, 200))
        self.assertEqual(mSymbolLayer.color(), QColor(250, 150, 200))

    def testQgsSimpleLineSymbolLayer(self):
        """
        Create a new style from a .sld file and match test
        """
        mTestName = "QgsSimpleLineSymbolLayer"
        mFilePath = QDir.toNativeSeparators(
            f"{unitTestDataPath()}/symbol_layer/{mTestName}.sld"
        )

        mDoc = QDomDocument(mTestName)
        mFile = QFile(mFilePath)
        mFile.open(QIODevice.OpenModeFlag.ReadOnly)
        mDoc.setContent(mFile, True)
        mFile.close()
        mSymbolLayer = QgsSimpleLineSymbolLayer.createFromSld(
            mDoc.elementsByTagName("LineSymbolizer").item(0).toElement()
        )

        mExpectedValue = type(QgsSimpleLineSymbolLayer())
        mValue = type(mSymbolLayer)
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = "#aa007f"
        mValue = mSymbolLayer.color().name()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 1.26
        mValue = mSymbolLayer.width()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = Qt.PenCapStyle.RoundCap
        mValue = mSymbolLayer.penCapStyle()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = Qt.PenJoinStyle.MiterJoin
        mValue = mSymbolLayer.penJoinStyle()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = True
        mValue = mSymbolLayer.useCustomDashPattern()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = [5.0, 2.0]
        mValue = mSymbolLayer.customDashVector()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        ctx = QgsRenderContext()
        self.assertCountEqual(mSymbolLayer.usedAttributes(ctx), {})

    def testQgsEllipseSymbolLayer(self):
        """
        Create a new style from a .sld file and match test
        """
        mTestName = "QgsEllipseSymbolLayer"
        mFilePath = QDir.toNativeSeparators(
            f"{unitTestDataPath()}/symbol_layer/{mTestName}.sld"
        )

        mDoc = QDomDocument(mTestName)
        mFile = QFile(mFilePath)
        mFile.open(QIODevice.OpenModeFlag.ReadOnly)
        mDoc.setContent(mFile, True)
        mFile.close()
        mSymbolLayer = QgsEllipseSymbolLayer.createFromSld(
            mDoc.elementsByTagName("PointSymbolizer").item(0).toElement()
        )

        mExpectedValue = type(QgsEllipseSymbolLayer())
        mValue = type(mSymbolLayer)
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = "circle"
        mValue = mSymbolLayer.symbolName()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = "#ffff7f"
        mValue = mSymbolLayer.fillColor().name()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = "#aaaaff"
        mValue = mSymbolLayer.strokeColor().name()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 7
        mValue = mSymbolLayer.symbolWidth()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 5
        mValue = mSymbolLayer.symbolHeight()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        ctx = QgsRenderContext()
        self.assertCountEqual(mSymbolLayer.usedAttributes(ctx), {})

    def testQgsFontMarkerSymbolLayer(self):
        """
        Create a new style from a .sld file and match test
        """
        mTestName = "QgsFontMarkerSymbolLayer"
        mFilePath = QDir.toNativeSeparators(
            f"{unitTestDataPath()}/symbol_layer/{mTestName}.sld"
        )

        mDoc = QDomDocument(mTestName)
        mFile = QFile(mFilePath)
        mFile.open(QIODevice.OpenModeFlag.ReadOnly)
        mDoc.setContent(mFile, True)
        mFile.close()
        mSymbolLayer = QgsFontMarkerSymbolLayer.createFromSld(
            mDoc.elementsByTagName("PointSymbolizer").item(0).toElement()
        )

        mExpectedValue = type(QgsFontMarkerSymbolLayer())
        mValue = type(mSymbolLayer)
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = "Arial"
        mValue = mSymbolLayer.fontFamily()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = "M"
        mValue = mSymbolLayer.character()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 6.23
        mValue = mSymbolLayer.size()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 3
        mValue = mSymbolLayer.angle()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        ctx = QgsRenderContext()
        self.assertCountEqual(mSymbolLayer.usedAttributes(ctx), {})

    def testQgsSvgMarkerSymbolLayer(self):
        """
        Create a new style from a .sld file and match test
        """
        mTestName = "QgsSvgMarkerSymbolLayer"
        mFilePath = QDir.toNativeSeparators(
            f"{unitTestDataPath()}/symbol_layer/{mTestName}.sld"
        )

        mDoc = QDomDocument(mTestName)
        mFile = QFile(mFilePath)
        mFile.open(QIODevice.OpenModeFlag.ReadOnly)
        mDoc.setContent(mFile, True)
        mFile.close()
        mSymbolLayer = QgsSvgMarkerSymbolLayer.createFromSld(
            mDoc.elementsByTagName("PointSymbolizer").item(0).toElement()
        )

        mExpectedValue = type(QgsSvgMarkerSymbolLayer(""))
        mValue = type(mSymbolLayer)
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = "skull.svg"
        mValue = os.path.basename(mSymbolLayer.path())
        print(("VALUE", mSymbolLayer.path()))
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 12
        mValue = mSymbolLayer.size()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        mExpectedValue = 45
        mValue = mSymbolLayer.angle()
        mMessage = f'Expected "{mExpectedValue}" got "{mValue}"'
        assert mExpectedValue == mValue, mMessage

        # Check values set from the query string in OnlineResource
        self.assertEqual(mSymbolLayer.strokeWidth(), 2.0)
        self.assertEqual(mSymbolLayer.strokeColor().name(), "#ff0000")
        self.assertEqual(mSymbolLayer.fillColor().name(), "#00ff00")

        ctx = QgsRenderContext()
        self.assertCountEqual(mSymbolLayer.usedAttributes(ctx), {})

    def testQgsFilledMarkerSymbolLayer(self):
        """
        Test QgsFilledMarkerSymbolLayer
        """
        mSymbolLayer = QgsFilledMarkerSymbolLayer.create()

        ctx = QgsRenderContext()
        self.assertCountEqual(mSymbolLayer.usedAttributes(ctx), {})

        # test colors, need to make sure colors are passed/retrieved from subsymbol
        mSymbolLayer.setColor(QColor(150, 50, 100))
        self.assertEqual(mSymbolLayer.color(), QColor(150, 50, 100))
        self.assertEqual(mSymbolLayer.subSymbol().color(), QColor(150, 50, 100))
        mSymbolLayer.subSymbol().setColor(QColor(250, 150, 200))
        self.assertEqual(mSymbolLayer.subSymbol().color(), QColor(250, 150, 200))
        self.assertEqual(mSymbolLayer.color(), QColor(250, 150, 200))

    def testSldOpacityFillExport(self):
        """Test issue GH #33376"""

        vl = QgsVectorLayer("Point?crs=epsg:4326", "test", "memory")

        foo_sym = QgsFillSymbol.createSimple(
            {"color": "#00ff00", "outline_color": "#0000ff"}
        )
        foo_sym.setOpacity(0.66)
        vl.setRenderer(QgsSingleSymbolRenderer(foo_sym))
        doc = QDomDocument()
        vl.exportSldStyle(doc, None)
        self.assertIn(
            '<se:SvgParameter name="fill-opacity">0.66</se:SvgParameter>',
            doc.toString(),
        )
        self.assertIn(
            '<se:SvgParameter name="fill">#00ff00</se:SvgParameter>', doc.toString()
        )
        self.assertIn(
            '<se:SvgParameter name="stroke">#0000ff</se:SvgParameter>', doc.toString()
        )
        self.assertIn(
            '<se:SvgParameter name="stroke-opacity">0.66</se:SvgParameter>',
            doc.toString(),
        )

    def testQgsVectorFieldSymbolLayer(self):
        """
        Test QgsVectorFieldSymbolLayer
        """
        mSymbolLayer = QgsVectorFieldSymbolLayer.create()

        ctx = QgsRenderContext()
        self.assertCountEqual(mSymbolLayer.usedAttributes(ctx), {})

        # test colors, need to make sure colors are passed/retrieved from subsymbol
        mSymbolLayer.setColor(QColor(150, 50, 100))
        self.assertEqual(mSymbolLayer.color(), QColor(150, 50, 100))
        self.assertEqual(mSymbolLayer.subSymbol().color(), QColor(150, 50, 100))
        mSymbolLayer.subSymbol().setColor(QColor(250, 150, 200))
        self.assertEqual(mSymbolLayer.subSymbol().color(), QColor(250, 150, 200))
        self.assertEqual(mSymbolLayer.color(), QColor(250, 150, 200))

    def test_should_render_selection_color(self):
        layer = QgsSimpleFillSymbolLayer.create(
            {"color": "#00ff00", "outline_color": "#0000ff"}
        )
        render_context = QgsRenderContext()
        context = QgsSymbolRenderContext(
            render_context, Qgis.RenderUnit.Millimeters, selected=False
        )
        self.assertFalse(layer.shouldRenderUsingSelectionColor(context))
        layer.setUserFlags(Qgis.SymbolLayerUserFlag.DisableSelectionRecoloring)
        self.assertFalse(layer.shouldRenderUsingSelectionColor(context))

        layer.setUserFlags(Qgis.SymbolLayerUserFlags())
        context = QgsSymbolRenderContext(
            render_context, Qgis.RenderUnit.Millimeters, selected=True
        )
        self.assertTrue(layer.shouldRenderUsingSelectionColor(context))
        layer.setUserFlags(Qgis.SymbolLayerUserFlag.DisableSelectionRecoloring)
        self.assertFalse(layer.shouldRenderUsingSelectionColor(context))

    def test_force_vector_rendering(self):
        render_context = QgsRenderContext()
        context = QgsSymbolRenderContext(
            render_context, Qgis.RenderUnit.Millimeters, selected=False
        )
        self.assertFalse(context.forceVectorRendering())

        # render context flag should force vector rendering
        render_context.setFlag(Qgis.RenderContextFlag.ForceVectorOutput, True)
        context = QgsSymbolRenderContext(
            render_context, Qgis.RenderUnit.Millimeters, selected=False
        )
        self.assertTrue(context.forceVectorRendering())

        # symbol render hint should also force vector rendering
        render_context.setFlag(Qgis.RenderContextFlag.ForceVectorOutput, False)
        context = QgsSymbolRenderContext(
            render_context,
            Qgis.RenderUnit.Millimeters,
            renderHints=Qgis.SymbolRenderHint.ForceVectorRendering,
        )
        self.assertTrue(context.forceVectorRendering())


if __name__ == "__main__":
    unittest.main()
