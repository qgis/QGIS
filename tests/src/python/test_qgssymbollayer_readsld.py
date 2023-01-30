# -*- coding: utf-8 -*-

"""
***************************************************************************
    test_qgssymbollayer_readsld.py
    ---------------------
    Date                 : January 2017
    Copyright            : (C) 2017, Jorge Gustavo Rocha
    Email                : jgr at di dot uminho dot pt
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Jorge Gustavo Rocha'
__date__ = 'January 2017'
__copyright__ = '(C) 2017, Jorge Gustavo Rocha'

import qgis  # NOQA

import os
from qgis.PyQt.QtXml import QDomDocument
from qgis.PyQt.QtCore import QTemporaryDir
from qgis.testing import start_app, unittest
from qgis.core import (Qgis,
                       QgsVectorLayer,
                       QgsFeature,
                       QgsGeometry,
                       QgsUnitTypes,
                       QgsPointXY,
                       QgsSvgMarkerSymbolLayer,
                       QgsSymbol,
                       QgsEllipseSymbolLayer,
                       QgsSimpleFillSymbolLayer,
                       QgsSVGFillSymbolLayer,
                       QgsSvgMarkerSymbolLayer,
                       QgsLinePatternFillSymbolLayer,
                       QgsSimpleLineSymbolLayer,
                       QgsMarkerLineSymbolLayer,
                       QgsSimpleMarkerSymbolLayer,
                       QgsFontMarkerSymbolLayer
                       )
from qgis.testing.mocked import get_iface
from utilities import unitTestDataPath

start_app()

TEST_DATA_DIR = unitTestDataPath()


def createLayerWithOneLine():
    # create a temporary layer
    # linelayer = iface.addVectorLayer("LineString?crs=epsg:4326&field=gid:int&field=name:string", "simple_line", "memory")
    linelayer = QgsVectorLayer("LineString?crs=epsg:4326&field=gid:int&field=name:string", "simple_line", "memory")
    one = QgsFeature(linelayer.dataProvider().fields(), 0)
    one.setAttributes([1, 'one'])
    one.setGeometry(QgsGeometry.fromPolylineXY([QgsPointXY(-7, 38), QgsPointXY(-8, 42)]))
    linelayer.dataProvider().addFeatures([one])
    return linelayer


def createLayerWithOnePoint():
    layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer",
                           "addfeat", "memory")
    pr = layer.dataProvider()
    f = QgsFeature()
    f.setAttributes(["test", 123])
    f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(100, 200)))
    assert pr.addFeatures([f])
    assert layer.featureCount() == 1
    return layer


def createLayerWithOnePolygon():
    layer = QgsVectorLayer("Polygon?crs=epsg:3111&field=pk:int", "vl", "memory")
    assert layer.isValid()
    f1 = QgsFeature(layer.dataProvider().fields(), 1)
    f1.setAttribute("pk", 1)
    f1.setGeometry(QgsGeometry.fromPolygonXY([[QgsPointXY(2484588, 2425722), QgsPointXY(2482767, 2398853),
                                               QgsPointXY(2520109, 2397715), QgsPointXY(2520792, 2425494),
                                               QgsPointXY(2484588, 2425722)]]))
    assert layer.dataProvider().addFeatures([f1])
    return layer


class TestQgsSymbolLayerReadSld(unittest.TestCase):
    """
    This class checks if SLD styles are properly applied
    """

    def setUp(self):
        self.iface = get_iface()

    # test <CSSParameter>VALUE<CSSParameter/>
    # test <CSSParameter><ogc:Literal>VALUE<ogc:Literal/><CSSParameter/>
    def test_Literal_within_CSSParameter(self):
        layer = createLayerWithOneLine()
        mFilePath = os.path.join(TEST_DATA_DIR, 'symbol_layer/external_sld/simple_streams.sld')
        layer.loadSldStyle(mFilePath)
        props = layer.renderer().symbol().symbolLayers()[0].properties()

        def testLineColor():
            # stroke CSSParameter within ogc:Literal
            # expected color is #003EBA, RGB 0,62,186
            self.assertEqual(layer.renderer().symbol().symbolLayers()[0].color().name(), '#003eba')

        def testLineWidth():
            # stroke-width CSSParameter within ogc:Literal
            self.assertEqual(props['line_width'], '2')

        def testLineOpacity():
            # stroke-opacity CSSParameter NOT within ogc:Literal
            # stroke-opacity=0.1
            self.assertEqual(props['line_color'], '0,62,186,25')

        testLineColor()
        testLineWidth()
        testLineOpacity()

    def testSimpleMarkerRotation(self):
        """
        Test if pointMarker property sld:Rotation value can be read if format is:
        <sld:Rotation>50.0</sld:Rotation>
        or
        <se:Rotation><ogc:Literal>50</ogc:Literal></se:Rotation>
        """
        # technically it's not necessary to use a real shape, but a empty memory
        # layer. In case these tests will upgrade to a rendering where to
        # compare also rendering not only properties
        # myShpFile = os.path.join(unitTestDataPath(), 'points.shp')
        # layer = QgsVectorLayer(myShpFile, 'points', 'ogr')
        layer = QgsVectorLayer("Point", "addfeat", "memory")
        assert (layer.isValid())
        # test if able to read <sld:Rotation>50.0</sld:Rotation>
        mFilePath = os.path.join(unitTestDataPath(),
                                 'symbol_layer/external_sld/testSimpleMarkerRotation-directValue.sld')
        layer.loadSldStyle(mFilePath)
        props = layer.renderer().symbol().symbolLayers()[0].properties()
        self.assertEqual(props['angle'], '50')
        # test if able to read <se:Rotation><ogc:Literal>50</ogc:Literal></se:Rotation>
        mFilePath = os.path.join(unitTestDataPath(),
                                 'symbol_layer/external_sld/testSimpleMarkerRotation-ogcLiteral.sld')
        layer.loadSldStyle(mFilePath)
        props = layer.renderer().symbol().symbolLayers()[0].properties()
        self.assertEqual(props['angle'], '50')

    def testSymbolSizeUom(self):
        # create a layer
        layer = createLayerWithOnePoint()

        # load a sld with marker size without uom attribute (pixels)
        sld = 'symbol_layer/QgsSvgMarkerSymbolLayer.sld'
        mFilePath = os.path.join(TEST_DATA_DIR, sld)
        layer.loadSldStyle(mFilePath)

        sld_size_px = 12

        sl = layer.renderer().symbol().symbolLayers()[0]
        size = sl.size()
        unit = sl.outputUnit()
        self.assertEqual(unit, QgsUnitTypes.RenderPixels)
        self.assertEqual(size, sld_size_px)

        # load a sld with marker size with uom attribute in pixel
        sld = 'symbol_layer/QgsSvgMarkerSymbolLayerUomPixel.sld'
        mFilePath = os.path.join(TEST_DATA_DIR, sld)
        layer.loadSldStyle(mFilePath)

        sld_size_px = 12

        sl = layer.renderer().symbol().symbolLayers()[0]
        size = sl.size()
        unit = sl.outputUnit()
        self.assertEqual(unit, QgsUnitTypes.RenderPixels)
        self.assertEqual(size, sld_size_px)

        # load a sld with marker size with uom attribute in meter
        sld = 'symbol_layer/QgsSvgMarkerSymbolLayerUomMetre.sld'
        mFilePath = os.path.join(TEST_DATA_DIR, sld)
        layer.loadSldStyle(mFilePath)

        sld_size_meters_at_scale = 12

        sl = layer.renderer().symbol().symbolLayers()[0]
        size = sl.size()
        unit = sl.outputUnit()
        self.assertEqual(unit, QgsUnitTypes.RenderMetersInMapUnits)
        self.assertEqual(size, sld_size_meters_at_scale)

        # load a sld with marker size with uom attribute in foot
        sld = 'symbol_layer/QgsSvgMarkerSymbolLayerUomFoot.sld'
        mFilePath = os.path.join(TEST_DATA_DIR, sld)
        layer.loadSldStyle(mFilePath)

        sld_size_meters_at_scale = 12 * 0.3048

        sl = layer.renderer().symbol().symbolLayers()[0]
        size = sl.size()
        unit = sl.outputUnit()
        self.assertEqual(unit, QgsUnitTypes.RenderMetersInMapUnits)
        self.assertAlmostEqual(size, sld_size_meters_at_scale, delta=0.1)

    def testSymbolSize(self):
        # create a layers
        layer = createLayerWithOnePoint()
        player = createLayerWithOnePolygon()

        # size test for QgsEllipseSymbolLayer
        sld = 'symbol_layer/QgsEllipseSymbolLayer.sld'
        mFilePath = os.path.join(TEST_DATA_DIR, sld)
        layer.loadSldStyle(mFilePath)

        sld_size_px = 7
        sld_stroke_width_px = 1

        sl = layer.renderer().symbol().symbolLayers()[0]
        size = sl.symbolWidth()
        stroke_width = sl.strokeWidth()
        unit = sl.outputUnit()
        self.assertTrue(isinstance(sl, QgsEllipseSymbolLayer))
        self.assertEqual(unit, QgsUnitTypes.RenderPixels)
        self.assertEqual(size, sld_size_px)
        self.assertEqual(stroke_width, sld_stroke_width_px)

        # size test for QgsVectorFieldSymbolLayer
        # createFromSld not implemented

        # size test for QgsSimpleFillSymbolLayer
        sld = 'symbol_layer/QgsSimpleFillSymbolLayer.sld'
        mFilePath = os.path.join(TEST_DATA_DIR, sld)
        player.loadSldStyle(mFilePath)

        sld_stroke_width_px = 0.26

        sl = player.renderer().symbol().symbolLayers()[0]
        stroke_width = sl.strokeWidth()
        unit = sl.outputUnit()
        self.assertTrue(isinstance(sl, QgsSimpleFillSymbolLayer))
        self.assertEqual(unit, QgsUnitTypes.RenderPixels)
        self.assertEqual(stroke_width, sld_stroke_width_px)

        # size test for QgsSVGFillSymbolLayer
        sld = 'symbol_layer/QgsSVGFillSymbolLayer.sld'
        mFilePath = os.path.join(TEST_DATA_DIR, sld)
        player.loadSldStyle(mFilePath)

        sld_size_px = 6
        sld_stroke_width_px = 3

        sl = player.renderer().symbol().symbolLayers()[0]
        size = sl.patternWidth()
        stroke_width = sl.svgStrokeWidth()
        unit = sl.outputUnit()
        self.assertTrue(isinstance(sl, QgsSVGFillSymbolLayer))
        self.assertEqual(unit, QgsUnitTypes.RenderPixels)
        self.assertEqual(size, sld_size_px)
        self.assertEqual(stroke_width, sld_stroke_width_px)

        # size test for QgsSvgMarkerSymbolLayer
        sld = 'symbol_layer/QgsSvgMarkerSymbolLayer.sld'
        mFilePath = os.path.join(TEST_DATA_DIR, sld)
        layer.loadSldStyle(mFilePath)

        sld_size_px = 12

        sl = layer.renderer().symbol().symbolLayers()[0]
        size = sl.size()
        unit = sl.outputUnit()
        self.assertTrue(isinstance(sl, QgsSvgMarkerSymbolLayer))
        self.assertEqual(unit, QgsUnitTypes.RenderPixels)
        self.assertEqual(size, sld_size_px)

        # size test for QgsPointPatternFillSymbolLayer
        # createFromSld not implemented

        # size test for QgsLinePatternFillSymbolLayer
        sld = 'symbol_layer/QgsLinePatternFillSymbolLayer.sld'
        mFilePath = os.path.join(TEST_DATA_DIR, sld)
        player.loadSldStyle(mFilePath)

        sld_size_px = 4
        sld_stroke_width_px = 1.5

        sl = player.renderer().symbol().symbolLayers()[0]
        size = sl.distance()
        stroke_width = sl.lineWidth()
        unit = sl.outputUnit()
        self.assertTrue(isinstance(sl, QgsLinePatternFillSymbolLayer))
        self.assertEqual(unit, QgsUnitTypes.RenderPixels)
        self.assertEqual(size, sld_size_px)
        self.assertEqual(stroke_width, sld_stroke_width_px)

        # test size for QgsSimpleLineSymbolLayer
        sld = 'symbol_layer/QgsSimpleLineSymbolLayer.sld'
        mFilePath = os.path.join(TEST_DATA_DIR, sld)
        player.loadSldStyle(mFilePath)

        sld_stroke_width_px = 1.26

        sl = player.renderer().symbol().symbolLayers()[0]
        stroke_width = sl.width()
        unit = sl.outputUnit()
        self.assertTrue(isinstance(sl, QgsSimpleLineSymbolLayer))
        self.assertEqual(unit, QgsUnitTypes.RenderPixels)
        self.assertEqual(stroke_width, sld_stroke_width_px)

        # test size for QgsMarkerLineSymbolLayer
        sld = 'symbol_layer/QgsMarkerLineSymbolLayer.sld'
        mFilePath = os.path.join(TEST_DATA_DIR, sld)
        player.loadSldStyle(mFilePath)

        sld_interval_px = 3.3
        sld_offset_px = 6.6

        sl = player.renderer().symbol().symbolLayers()[0]
        interval = sl.interval()
        offset = sl.offset()
        unit = sl.outputUnit()
        self.assertTrue(isinstance(sl, QgsMarkerLineSymbolLayer))
        self.assertEqual(unit, QgsUnitTypes.RenderPixels)
        self.assertEqual(interval, sld_interval_px)
        self.assertEqual(offset, sld_offset_px)

        # test size for QgsSimpleMarkerSymbolLayer
        sld = 'symbol_layer/QgsSimpleMarkerSymbolLayer.sld'
        mFilePath = os.path.join(TEST_DATA_DIR, sld)
        layer.loadSldStyle(mFilePath)

        sld_size_px = 6
        sld_displacement_x_px = 3.3
        sld_displacement_y_px = 6.6

        sl = layer.renderer().symbol().symbolLayers()[0]
        size = sl.size()
        offset = sl.offset()
        unit = sl.outputUnit()
        self.assertTrue(isinstance(sl, QgsSimpleMarkerSymbolLayer))
        self.assertEqual(unit, QgsUnitTypes.RenderPixels)
        self.assertEqual(size, sld_size_px)
        self.assertEqual(offset.x(), sld_displacement_x_px)
        self.assertEqual(offset.y(), sld_displacement_y_px)

        # test size for QgsSVGMarkerSymbolLayer
        sld = 'symbol_layer/QgsSvgMarkerSymbolLayer.sld'
        mFilePath = os.path.join(TEST_DATA_DIR, sld)
        layer.loadSldStyle(mFilePath)

        sld_size_px = 12

        sl = layer.renderer().symbol().symbolLayers()[0]
        size = sl.size()
        self.assertTrue(isinstance(sl, QgsSvgMarkerSymbolLayer))
        self.assertEqual(unit, QgsUnitTypes.RenderPixels)
        self.assertEqual(size, sld_size_px)

        # test size for QgsFontMarkerSymbolLayer
        sld = 'symbol_layer/QgsFontMarkerSymbolLayer.sld'
        mFilePath = os.path.join(TEST_DATA_DIR, sld)
        layer.loadSldStyle(mFilePath)

        sld_size_px = 6.23

        sl = layer.renderer().symbol().symbolLayers()[0]
        size = sl.size()
        self.assertTrue(isinstance(sl, QgsFontMarkerSymbolLayer))
        self.assertEqual(unit, QgsUnitTypes.RenderPixels)
        self.assertEqual(size, sld_size_px)

        # test percent encoding  for QgsFontMarkerSymbolLayer
        sld = 'symbol_layer/QgsFontMarkerSymbolLayerPercentEncoding.sld'
        mFilePath = os.path.join(TEST_DATA_DIR, sld)
        layer.loadSldStyle(mFilePath)

        sl = layer.renderer().symbol().symbolLayers()[0]
        self.assertTrue(isinstance(sl, QgsFontMarkerSymbolLayer))
        self.assertEqual(sl.fontFamily(), 'MapInfo Miscellaneous')

    def testSymbolSizeAfterReload(self):
        # create a layer
        layer = createLayerWithOnePoint()

        # load a sld with marker size
        sld = 'symbol_layer/QgsSvgMarkerSymbolLayer.sld'
        mFilePath = os.path.join(TEST_DATA_DIR, sld)
        layer.loadSldStyle(mFilePath)

        # get the size and unit of the symbol
        sl = layer.renderer().symbol().symbolLayers()[0]
        first_size = sl.size()
        first_unit = sl.outputUnit()  # in pixels

        # export sld into a qdomdocument with namespace processing activated
        doc = QDomDocument()
        msg = ""
        layer.exportSldStyle(doc, msg)
        doc.setContent(doc.toString(), True)
        self.assertTrue(msg == "")

        # reload the same sld
        root = doc.firstChildElement("StyledLayerDescriptor")
        el = root.firstChildElement("NamedLayer")
        layer.readSld(el, msg)

        # extract the size and unit of symbol
        sl = layer.renderer().symbol().symbolLayers()[0]
        second_size = sl.size()
        second_unit = sl.outputUnit()

        # size and unit should be the same after export and reload the same
        # sld description
        self.assertEqual(first_size, second_size)
        self.assertEqual(first_unit, second_unit)

    def test_Literal_within_CSSParameter_and_Text(self):
        layer = createLayerWithOneLine()
        mFilePath = os.path.join(TEST_DATA_DIR, 'symbol_layer/external_sld/simple_line_with_text.sld')
        layer.loadSldStyle(mFilePath)
        props = layer.renderer().symbol().symbolLayers()[0].properties()

        def testLineColor():
            # stroke SvgParameter within ogc:Literal
            # expected color is #003EBA, RGB 0,62,186
            self.assertEqual(layer.renderer().symbol().symbolLayers()[0].color().name(), '#003eba')

        def testLineWidth():
            # stroke-width SvgParameter within ogc:Literal
            self.assertEqual(props['line_width'], '2')

        def testLineOpacity():
            # stroke-opacity SvgParameter NOT within ogc:Literal
            # stroke-opacity=0.1
            self.assertEqual(props['line_color'], '0,62,186,24')

        testLineColor()
        testLineWidth()
        testLineOpacity()

        from qgis.core import QgsPalLayerSettings

        self.assertTrue(layer.labelsEnabled())
        self.assertEqual(layer.labeling().type(), 'simple')

        settings = layer.labeling().settings()
        self.assertEqual(settings.fieldName, 'name')

        format = settings.format()
        self.assertEqual(format.color().name(), '#ff0000')

        font = format.font()
        self.assertEqual(font.family(), 'QGIS Vera Sans')
        self.assertTrue(font.bold())
        self.assertFalse(font.italic())

        self.assertEqual(format.size(), 18)
        self.assertEqual(format.sizeUnit(), QgsUnitTypes.RenderPixels)

        # the layer contains lines
        # from qgis.core import QgsWkbTypes
        # self.assertEqual(layer.geometryType(), QgsWkbTypes.LineGeometry)
        # the placement should be QgsPalLayerSettings.Line
        self.assertEqual(settings.placement, QgsPalLayerSettings.AroundPoint)

        self.assertEqual(settings.xOffset, 1)
        self.assertEqual(settings.yOffset, 0)
        self.assertEqual(settings.offsetUnits, QgsUnitTypes.RenderPixels)

    def test_read_circle(self):
        """Test wellknown name circle polygon fill"""

        sld = """<?xml version="1.0" encoding="UTF-8"?>
            <StyledLayerDescriptor xmlns="http://www.opengis.net/sld" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:ogc="http://www.opengis.net/ogc" version="1.1.0" xsi:schemaLocation="http://www.opengis.net/sld http://schemas.opengis.net/sld/1.1.0/StyledLayerDescriptor.xsd" xmlns:se="http://www.opengis.net/se">
            <NamedLayer>
                <se:Name>Single symbol fill</se:Name>
                <UserStyle>
                <se:Name>Single symbol fill</se:Name>
                <se:FeatureTypeStyle>
                    <se:Rule>
                    <se:Name>Single symbol</se:Name>
                    <se:PolygonSymbolizer>
                        <se:Fill>
                        <se:GraphicFill>
                            <se:Graphic>
                            <se:Mark>
                                <se:WellKnownName>circle</se:WellKnownName>
                                <se:Fill>
                                <se:SvgParameter name="fill">#db1e2a</se:SvgParameter>
                                </se:Fill>
                                <se:Stroke>
                                <se:SvgParameter name="stroke">#801119</se:SvgParameter>
                                <se:SvgParameter name="stroke-width">1.5</se:SvgParameter>
                                </se:Stroke>
                            </se:Mark>
                            <se:Size>14</se:Size>
                            </se:Graphic>
                        </se:GraphicFill>
                        </se:Fill>
                        <sld:VendorOption name="graphic-margin">{}</sld:VendorOption>
                    </se:PolygonSymbolizer>
                    <se:PolygonSymbolizer>
                        <se:Stroke>
                            <se:SvgParameter name="stroke">#ff0000</se:SvgParameter>
                            <se:SvgParameter name="stroke-width">2</se:SvgParameter>
                            <se:SvgParameter name="stroke-linejoin">bevel</se:SvgParameter>
                        </se:Stroke>
                    </se:PolygonSymbolizer>
                    </se:Rule>
                </se:FeatureTypeStyle>
                </UserStyle>
            </NamedLayer>
            </StyledLayerDescriptor>
        """

        tmp_dir = QTemporaryDir()
        tmp_path = tmp_dir.path()
        sld_path = os.path.join(tmp_path, 'circle_fill.sld')

        layer = createLayerWithOnePolygon()

        def _check_layer(layer, yMargin=10, xMargin=15):
            """
            - QgsFillSymbol
              - layers
                - QgsPointPatternFillSymbolLayer
                  - subSymbol: QgsMarkerSymbol
                    - layers
                       - QgsSimpleMarkerSymbolLayer (shape)
            """
            layer.loadSldStyle(sld_path)
            point_pattern_fill_symbol_layer = layer.renderer().symbol().symbolLayers()[0]
            marker = point_pattern_fill_symbol_layer.subSymbol()
            self.assertEqual(marker.type(), QgsSymbol.SymbolType.Marker)
            marker_symbol = marker.symbolLayers()[0]
            self.assertEqual(marker_symbol.strokeColor().name(), '#801119')
            self.assertEqual(marker_symbol.strokeWidth(), 1.5)
            self.assertEqual(marker_symbol.shape(), Qgis.MarkerShape.Circle)
            self.assertEqual(marker_symbol.size(), 14)
            self.assertEqual(point_pattern_fill_symbol_layer.distanceXUnit(), QgsUnitTypes.RenderUnit.RenderPixels)
            self.assertEqual(point_pattern_fill_symbol_layer.distanceYUnit(), QgsUnitTypes.RenderUnit.RenderPixels)
            self.assertEqual(point_pattern_fill_symbol_layer.distanceX(), xMargin * 2 + marker_symbol.size())
            self.assertEqual(point_pattern_fill_symbol_layer.distanceY(), yMargin * 2 + marker_symbol.size())

        with open(sld_path, 'w+') as f:
            f.write(sld.format('25'))
        _check_layer(layer, 25, 25)

        # From: https://docs.geoserver.org/stable/en/user/styling/sld/extensions/margins.html
        # top,right,bottom,left (one explicit value per margin)
        # top,right-left,bottom (three values, with right and left sharing the same value)
        # top-bottom,right-left (two values, top and bottom sharing the same value)
        # top-right-bottom-left (single value for all four margins)

        for margin in ('10 15', '10 15 10', '10 15 10 15'):
            with open(sld_path, 'w+') as f:
                f.write(sld.format(margin))
            _check_layer(layer)

        # Round trip
        dom = QDomDocument()
        root = dom.createElement("FakeRoot")
        dom.appendChild(root)
        result = layer.saveSldStyle(sld_path)
        self.assertTrue(result)

        _check_layer(layer)


if __name__ == '__main__':
    unittest.main()
