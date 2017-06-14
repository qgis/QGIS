# -*- coding: utf-8 -*-

"""
***************************************************************************
    test_qgssymbollayerv2_readsld.py
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
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

import os
from qgis.PyQt.QtXml import QDomDocument
from qgis.testing import start_app, unittest
from qgis.core import (QgsVectorLayer,
                       QgsFeature,
                       QgsGeometry,
                       QgsPoint,
                       QgsUnitTypes,
                       QgsPoint,
                       QgsSvgMarkerSymbolLayerV2,
                       QgsEllipseSymbolLayerV2,
                       QgsSimpleFillSymbolLayerV2,
                       QgsSVGFillSymbolLayer,
                       QgsLinePatternFillSymbolLayer,
                       QgsSimpleLineSymbolLayerV2,
                       QgsMarkerLineSymbolLayerV2,
                       QgsSimpleMarkerSymbolLayerV2,
                       QgsFontMarkerSymbolLayerV2,
                       QgsSymbolV2
                       )
from qgis.testing import unittest
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
    one.setGeometry(QgsGeometry.fromPolyline([QgsPoint(-7, 38), QgsPoint(-8, 42)]))
    linelayer.dataProvider().addFeatures([one])
    return linelayer


def createLayerWithOnePoint():
    layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer",
                           "addfeat", "memory")
    pr = layer.dataProvider()
    f = QgsFeature()
    f.setAttributes(["test", 123])
    f.setGeometry(QgsGeometry.fromPoint(QgsPoint(100, 200)))
    assert pr.addFeatures([f])
    assert layer.pendingFeatureCount() == 1
    return layer


def createLayerWithOnePolygon():
    layer = QgsVectorLayer("Polygon?crs=epsg:3111&field=pk:int", "vl", "memory")
    assert layer.isValid()
    f1 = QgsFeature(layer.dataProvider().fields(), 1)
    f1.setAttribute("pk", 1)
    f1.setGeometry(QgsGeometry.fromPolygon([[QgsPoint(2484588, 2425722), QgsPoint(2482767, 2398853), QgsPoint(2520109, 2397715), QgsPoint(2520792, 2425494), QgsPoint(2484588, 2425722)]]))
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
        props = layer.rendererV2().symbol().symbolLayers()[0].properties()

        def testLineColor():
            # border CSSParameter within ogc:Literal
            # expected color is #003EBA, RGB 0,62,186
            self.assertEqual(layer.rendererV2().symbol().symbolLayers()[0].color().name(), '#003eba')

        def testLineWidth():
            # border-width CSSParameter within ogc:Literal
            self.assertEqual(props['line_width'], '2')

        def testLineOpacity():
            # border-opacity CSSParameter NOT within ogc:Literal
            # border-opacity=0.1
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
        # layer. In case these tests will upgrate to a rendering where to
        # compare also rendering not only properties
        #myShpFile = os.path.join(unitTestDataPath(), 'points.shp')
        #layer = QgsVectorLayer(myShpFile, 'points', 'ogr')
        layer = QgsVectorLayer("Point", "addfeat", "memory")
        assert(layer.isValid())
        # test if able to read <sld:Rotation>50.0</sld:Rotation>
        mFilePath = os.path.join(unitTestDataPath(), 'symbol_layer/external_sld/testSimpleMarkerRotation-directValue.sld')
        layer.loadSldStyle(mFilePath)
        props = layer.rendererV2().symbol().symbolLayers()[0].properties()
        self.assertEqual(props['angle'], '50')
        # test if able to read <se:Rotation><ogc:Literal>50</ogc:Literal></se:Rotation>
        mFilePath = os.path.join(unitTestDataPath(), 'symbol_layer/external_sld/testSimpleMarkerRotation-ogcLiteral.sld')
        layer.loadSldStyle(mFilePath)
        props = layer.rendererV2().symbol().symbolLayers()[0].properties()
        self.assertEqual(props['angle'], '50')

    def testSymbolSizeUom(self):
        # create a layer
        layer = createLayerWithOnePoint()

        # load a sld with marker size without uom attribute (pixels)
        sld = 'symbol_layer/QgsSvgMarkerSymbolLayerV2.sld'
        mFilePath = os.path.join(TEST_DATA_DIR, sld)
        layer.loadSldStyle(mFilePath)

        sld_size_px = 12

        sl = layer.rendererV2().symbol().symbolLayers()[0]
        size = sl.size()
        unit = sl.outputUnit()
        self.assertEqual(unit, QgsSymbolV2.Pixel)
        self.assertEqual(size, sld_size_px)

        # load a sld with marker size with uom attribute in pixel
        sld = 'symbol_layer/QgsSvgMarkerSymbolLayerUomPixel.sld'
        mFilePath = os.path.join(TEST_DATA_DIR, sld)
        layer.loadSldStyle(mFilePath)

        sld_size_px = 12

        sl = layer.rendererV2().symbol().symbolLayers()[0]
        size = sl.size()
        unit = sl.outputUnit()
        self.assertEqual(unit, QgsSymbolV2.Pixel)
        self.assertEqual(size, sld_size_px)

        # load a sld with marker size with uom attribute in meter
        sld = 'symbol_layer/QgsSvgMarkerSymbolLayerUomMetre.sld'
        mFilePath = os.path.join(TEST_DATA_DIR, sld)
        layer.loadSldStyle(mFilePath)

        sld_size_px = 12 / (0.28 * 0.001)

        sl = layer.rendererV2().symbol().symbolLayers()[0]
        size = sl.size()
        unit = sl.outputUnit()
        self.assertEqual(unit, QgsSymbolV2.Pixel)
        self.assertAlmostEqual(size, sld_size_px, delta=0.1)

        # load a sld with marker size with uom attribute in foot
        sld = 'symbol_layer/QgsSvgMarkerSymbolLayerUomFoot.sld'
        mFilePath = os.path.join(TEST_DATA_DIR, sld)
        layer.loadSldStyle(mFilePath)

        sld_size_px = 12 * (304.8 / 0.28)

        sl = layer.rendererV2().symbol().symbolLayers()[0]
        size = sl.size()
        unit = sl.outputUnit()
        self.assertEqual(unit, QgsSymbolV2.Pixel)
        self.assertAlmostEqual(size, sld_size_px, delta=0.1)

    def testSymbolSize(self):
        # create a layers
        layer = createLayerWithOnePoint()
        player = createLayerWithOnePolygon()

        # size test for QgsEllipseSymbolLayer
        sld = 'symbol_layer/QgsEllipseSymbolLayerV2.sld'
        mFilePath = os.path.join(TEST_DATA_DIR, sld)
        layer.loadSldStyle(mFilePath)

        sld_size_px = 7
        sld_border_width_px = 1

        sl = layer.rendererV2().symbol().symbolLayers()[0]
        size = sl.symbolWidth()
        border_width = sl.outlineWidth()
        unit = sl.outputUnit()
        self.assertTrue(isinstance(sl, QgsEllipseSymbolLayerV2))
        self.assertEqual(unit, QgsSymbolV2.Pixel)
        self.assertEqual(size, sld_size_px)
        self.assertEqual(border_width, sld_border_width_px)

        # size test for QgsVectorFieldSymbolLayer
        # createFromSld not implemented

        # size test for QgsSimpleFillSymbolLayer
        sld = 'symbol_layer/QgsSimpleFillSymbolLayerV2.sld'
        mFilePath = os.path.join(TEST_DATA_DIR, sld)
        player.loadSldStyle(mFilePath)

        sld_border_width_px = 0.26

        sl = player.rendererV2().symbol().symbolLayers()[0]
        border_width = sl.borderWidth()
        unit = sl.outputUnit()
        self.assertTrue(isinstance(sl, QgsSimpleFillSymbolLayerV2))
        self.assertEqual(unit, QgsSymbolV2.Pixel)
        self.assertEqual(border_width, sld_border_width_px)

        # size test for QgsSVGFillSymbolLayer
        sld = 'symbol_layer/QgsSVGFillSymbolLayer.sld'
        mFilePath = os.path.join(TEST_DATA_DIR, sld)
        player.loadSldStyle(mFilePath)

        sld_size_px = 6
        sld_border_width_px = 3

        sl = player.rendererV2().symbol().symbolLayers()[0]
        size = sl.patternWidth()
        border_width = sl.svgOutlineWidth()
        unit = sl.outputUnit()
        self.assertTrue(isinstance(sl, QgsSVGFillSymbolLayer))
        self.assertEqual(unit, QgsSymbolV2.Pixel)
        self.assertEqual(size, sld_size_px)
        self.assertEqual(border_width, sld_border_width_px)

        # size test for QgsSvgMarkerSymbolLayer
        sld = 'symbol_layer/QgsSvgMarkerSymbolLayerV2.sld'
        mFilePath = os.path.join(TEST_DATA_DIR, sld)
        layer.loadSldStyle(mFilePath)

        sld_size_px = 12

        sl = layer.rendererV2().symbol().symbolLayers()[0]
        size = sl.size()
        unit = sl.outputUnit()
        self.assertTrue(isinstance(sl, QgsSvgMarkerSymbolLayerV2))
        self.assertEqual(unit, QgsSymbolV2.Pixel)
        self.assertEqual(size, sld_size_px)

        # size test for QgsPointPatternFillSymbolLayer
        # createFromSld not implemented

        # size test for QgsLinePatternFillSymbolLayer
        sld = 'symbol_layer/QgsLinePatternFillSymbolLayer.sld'
        mFilePath = os.path.join(TEST_DATA_DIR, sld)
        player.loadSldStyle(mFilePath)

        sld_size_px = 4
        sld_border_width_px = 1.5

        sl = player.rendererV2().symbol().symbolLayers()[0]
        size = sl.distance()
        border_width = sl.lineWidth()
        unit = sl.outputUnit()
        self.assertTrue(isinstance(sl, QgsLinePatternFillSymbolLayer))
        self.assertEqual(unit, QgsSymbolV2.Pixel)
        self.assertEqual(size, sld_size_px)
        self.assertEqual(border_width, sld_border_width_px)

        # test size for QgsSimpleLineSymbolLayer
        sld = 'symbol_layer/QgsSimpleLineSymbolLayerV2.sld'
        mFilePath = os.path.join(TEST_DATA_DIR, sld)
        player.loadSldStyle(mFilePath)

        sld_border_width_px = 1.26

        sl = player.rendererV2().symbol().symbolLayers()[0]
        border_width = sl.width()
        unit = sl.outputUnit()
        self.assertTrue(isinstance(sl, QgsSimpleLineSymbolLayerV2))
        self.assertEqual(unit, QgsSymbolV2.Pixel)
        self.assertEqual(border_width, sld_border_width_px)

        # test size for QgsMarkerLineSymbolLayer
        sld = 'symbol_layer/QgsMarkerLineSymbolLayerV2.sld'
        mFilePath = os.path.join(TEST_DATA_DIR, sld)
        player.loadSldStyle(mFilePath)

        sld_interval_px = 3.3
        sld_offset_px = 6.6

        sl = player.rendererV2().symbol().symbolLayers()[0]
        interval = sl.interval()
        offset = sl.offset()
        unit = sl.outputUnit()
        self.assertTrue(isinstance(sl, QgsMarkerLineSymbolLayerV2))
        self.assertEqual(unit, QgsSymbolV2.Pixel)
        self.assertEqual(interval, sld_interval_px)
        self.assertEqual(offset, sld_offset_px)

        # test size for QgsSimpleMarkerSymbolLayer
        sld = 'symbol_layer/QgsSimpleMarkerSymbolLayerV2.sld'
        mFilePath = os.path.join(TEST_DATA_DIR, sld)
        layer.loadSldStyle(mFilePath)

        sld_size_px = 6

        sld_displacement_x_px = 3.3
        sld_displacement_y_px = 6.6

        sl = layer.rendererV2().symbol().symbolLayers()[0]
        size = sl.size()
        offset = sl.offset()
        unit = sl.outputUnit()
        self.assertTrue(isinstance(sl, QgsSimpleMarkerSymbolLayerV2))
        self.assertEqual(unit, QgsSymbolV2.Pixel)
        self.assertEqual(size, sld_size_px)
        self.assertEqual(offset.x(), sld_displacement_x_px)
        self.assertEqual(offset.y(), sld_displacement_y_px)

        # test size for QgsSVGMarkerSymbolLayer
        sld = 'symbol_layer/QgsSvgMarkerSymbolLayerV2.sld'
        mFilePath = os.path.join(TEST_DATA_DIR, sld)
        layer.loadSldStyle(mFilePath)

        sld_size_px = 12

        sl = layer.rendererV2().symbol().symbolLayers()[0]
        size = sl.size()
        self.assertTrue(isinstance(sl, QgsSvgMarkerSymbolLayerV2))
        self.assertEqual(unit, QgsSymbolV2.Pixel)
        self.assertEqual(size, sld_size_px)

        # test size for QgsFontMarkerSymbolLayer
        sld = 'symbol_layer/QgsFontMarkerSymbolLayerV2.sld'
        mFilePath = os.path.join(TEST_DATA_DIR, sld)
        layer.loadSldStyle(mFilePath)

        sld_size_px = 6.23

        sl = layer.rendererV2().symbol().symbolLayers()[0]
        size = sl.size()
        self.assertTrue(isinstance(sl, QgsFontMarkerSymbolLayerV2))
        self.assertEqual(unit, QgsSymbolV2.Pixel)
        self.assertEqual(size, sld_size_px)

    def testSymbolSizeAfterReload(self):
        # create a layer
        layer = createLayerWithOnePoint()

        # load a sld with marker size
        sld = 'symbol_layer/QgsSvgMarkerSymbolLayerV2.sld'
        mFilePath = os.path.join(TEST_DATA_DIR, sld)
        layer.loadSldStyle(mFilePath)

        # get the size and unit of the symbol
        sl = layer.rendererV2().symbol().symbolLayers()[0]
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
        sl = layer.rendererV2().symbol().symbolLayers()[0]
        second_size = sl.size()
        second_unit = sl.outputUnit()

        # size and unit should be the same after export and reload the same
        # sld description
        self.assertEqual(first_size, second_size)
        self.assertEqual(first_unit, second_unit)


if __name__ == '__main__':
    unittest.main()
