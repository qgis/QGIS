# -*- coding: utf-8 -*-

"""
***************************************************************************
    test_qgssymbol.py
    ---------------------
    Date                 : January 2016
    Copyright            : (C) 2016 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Nyall Dawson'
__date__ = 'January 2016'
__copyright__ = '(C) 2016, Nyall Dawson'

import qgis  # NOQA

from utilities import unitTestDataPath

from qgis.PyQt.QtCore import QDir, Qt, QSize
from qgis.PyQt.QtGui import QImage, QColor, QPainter
from qgis.PyQt.QtXml import QDomDocument

from qgis.core import (QgsGeometry,
                       QgsRectangle,
                       QgsCoordinateTransform,
                       QgsCoordinateReferenceSystem,
                       QgsMapUnitScale,
                       QgsMarkerSymbol,
                       QgsMultiPolygon,
                       QgsPolygon,
                       QgsLineString,
                       QgsFillSymbol,
                       QgsLineSymbol,
                       QgsRenderContext,
                       QgsFeature,
                       QgsMapSettings,
                       QgsRenderChecker,
                       QgsSimpleMarkerSymbolLayer,
                       QgsSimpleMarkerSymbolLayerBase,
                       QgsSimpleLineSymbolLayer,
                       QgsSimpleFillSymbolLayer,
                       QgsUnitTypes,
                       QgsWkbTypes,
                       QgsProject,
                       QgsReadWriteContext,
                       QgsSymbolLayerUtils,
                       QgsMarkerLineSymbolLayer,
                       QgsArrowSymbolLayer,
                       QgsGeometryGeneratorSymbolLayer,
                       QgsSymbol,
                       Qgis,
                       QgsSymbolLayer,
                       QgsProperty,
                       QgsRasterFillSymbolLayer
                       )

from qgis.testing import unittest, start_app

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsSymbol(unittest.TestCase):

    def setUp(self):
        # Create some simple symbols
        self.fill_symbol = QgsFillSymbol.createSimple({'color': '#ffffff', 'outline_color': 'black'})
        self.line_symbol = QgsLineSymbol.createSimple({'color': '#ffffff', 'line_width': '3'})
        self.marker_symbol = QgsMarkerSymbol.createSimple({'color': '#ffffff', 'size': '3', 'outline_color': 'black'})
        self.report = "<h1>Python QgsSymbol Tests</h1>\n"

    def tearDown(self):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(self.report)

    def testPythonAdditions(self):
        """
        Test PyQGIS additions to QgsSymbol
        """
        markerSymbol = QgsMarkerSymbol()
        markerSymbol.symbolLayer(0).setColor(QColor(255, 255, 0))
        self.assertEqual(len(markerSymbol), 1)
        layers = [l.color().name() for l in markerSymbol]
        self.assertEqual(layers, ['#ffff00'])
        self.assertEqual(markerSymbol[0].color().name(), '#ffff00')
        self.assertEqual(markerSymbol[-1].color().name(), '#ffff00')
        with self.assertRaises(IndexError):
            _ = markerSymbol[1]
        with self.assertRaises(IndexError):
            _ = markerSymbol[-2]
        with self.assertRaises(IndexError):
            _ = markerSymbol.symbolLayer(1)
        with self.assertRaises(IndexError):
            _ = markerSymbol.symbolLayer(-1)  # negative index only supported by []

        with self.assertRaises(IndexError):
            del markerSymbol[1]
        with self.assertRaises(IndexError):
            del markerSymbol[-2]

        del markerSymbol[0]
        self.assertEqual(len(markerSymbol), 0)
        self.assertTrue(markerSymbol)

        with self.assertRaises(IndexError):
            _ = markerSymbol[0]
        with self.assertRaises(IndexError):
            _ = markerSymbol[-1]

        with self.assertRaises(IndexError):
            del markerSymbol[0]
        with self.assertRaises(IndexError):
            del markerSymbol[-1]

        markerSymbol.appendSymbolLayer(
            QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayerBase.Star, color=QColor(255, 0, 0),
                                       strokeColor=QColor(0, 255, 0), size=10))
        markerSymbol.appendSymbolLayer(
            QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayerBase.Star, color=QColor(255, 255, 0),
                                       strokeColor=QColor(0, 255, 255), size=10))

        self.assertEqual(len(markerSymbol), 2)
        layers = [l.color().name() for l in markerSymbol]
        self.assertEqual(layers, ['#ff0000', '#ffff00'])
        self.assertEqual(markerSymbol[0].color().name(), '#ff0000')
        self.assertEqual(markerSymbol[-1].color().name(), '#ffff00')
        self.assertEqual(markerSymbol[1].color().name(), '#ffff00')
        self.assertEqual(markerSymbol[-2].color().name(), '#ff0000')
        with self.assertRaises(IndexError):
            _ = markerSymbol[2]
        with self.assertRaises(IndexError):
            _ = markerSymbol[-3]
        with self.assertRaises(IndexError):
            _ = markerSymbol.symbolLayer(2)
        with self.assertRaises(IndexError):
            _ = markerSymbol.symbolLayer(-1)  # negative index only supported by []

        with self.assertRaises(IndexError):
            del markerSymbol[2]
        with self.assertRaises(IndexError):
            del markerSymbol[-3]

        del markerSymbol[1]
        layers = [l.color().name() for l in markerSymbol]
        self.assertEqual(layers, ['#ff0000'])

    def testSymbolTypeToString(self):
        """
        Test QgsSymbol.symbolTypeToString
        """
        self.assertEqual(QgsSymbol.symbolTypeToString(QgsSymbol.Marker), 'Marker')
        self.assertEqual(QgsSymbol.symbolTypeToString(QgsSymbol.Line), 'Line')
        self.assertEqual(QgsSymbol.symbolTypeToString(QgsSymbol.Fill), 'Fill')
        self.assertEqual(QgsSymbol.symbolTypeToString(QgsSymbol.Hybrid), 'Hybrid')

    def testSymbolTypeForGeometryType(self):
        """
        Test QgsSymbol.symbolTypeForGeometryType
        """
        self.assertEqual(QgsSymbol.symbolTypeForGeometryType(QgsWkbTypes.PointGeometry), QgsSymbol.Marker)
        self.assertEqual(QgsSymbol.symbolTypeForGeometryType(QgsWkbTypes.LineGeometry), QgsSymbol.Line)
        self.assertEqual(QgsSymbol.symbolTypeForGeometryType(QgsWkbTypes.PolygonGeometry), QgsSymbol.Fill)
        self.assertEqual(QgsSymbol.symbolTypeForGeometryType(QgsWkbTypes.NullGeometry), QgsSymbol.Hybrid)
        self.assertEqual(QgsSymbol.symbolTypeForGeometryType(QgsWkbTypes.UnknownGeometry), QgsSymbol.Hybrid)

    def testColor(self):
        """
        Test QgsSymbol.color() logic
        """
        symbol = QgsFillSymbol.createSimple({'color': '#ff00ff', 'outline_color': '#ffffff'})
        self.assertEqual(symbol.color().name(), '#ff00ff')

        # insert a new first layer, symbol color should be taken from that layer
        second_fill = QgsSimpleFillSymbolLayer(QColor(0, 255, 0))
        symbol.insertSymbolLayer(0, second_fill)
        self.assertEqual(symbol.color().name(), '#00ff00')

        # lock the first layer -- locked color layers are ignored, so symbol color should come from second layer
        second_fill.setLocked(True)
        self.assertEqual(symbol.color().name(), '#ff00ff')

        # add a symbol layer which does not have colors (raster fill)
        raster_fill = QgsRasterFillSymbolLayer()
        symbol.insertSymbolLayer(0, raster_fill)
        self.assertFalse(raster_fill.color().isValid())
        # raster fill does not have a valid color, so should be ignored and the 3rd symbol layer color will be returned
        self.assertEqual(symbol.color().name(), '#ff00ff')

    def testFlags(self):
        """
        Test symbol flags
        """
        s = QgsLineSymbol.createSimple({})
        self.assertEqual(s.flags(), Qgis.SymbolFlags())

        s.setFlags(Qgis.SymbolFlag.RendererShouldUseSymbolLevels)
        self.assertEqual(s.flags(), Qgis.SymbolFlag.RendererShouldUseSymbolLevels)

        s2 = s.clone()
        self.assertEqual(s2.flags(), Qgis.SymbolFlag.RendererShouldUseSymbolLevels)

        # test that flags are saved/restored via XML
        doc = QDomDocument()
        context = QgsReadWriteContext()
        element = QgsSymbolLayerUtils.saveSymbol('test', s, doc, context)

        s2 = QgsSymbolLayerUtils.loadSymbol(element, context)
        self.assertEqual(s2.flags(), Qgis.SymbolFlag.RendererShouldUseSymbolLevels)

    def testCanCauseArtifactsBetweenAdjacentTiles(self):
        """
        Test canCauseArtifactsBetweenAdjacentTiles()
        """

        # start with a symbol which won't cause artifacts -- a simple line symbol
        symbol = QgsLineSymbol.createSimple({})
        self.assertFalse(symbol.canCauseArtifactsBetweenAdjacentTiles())
        # add a second layer which CAN cause artifacts
        symbol.appendSymbolLayer(QgsArrowSymbolLayer())
        self.assertTrue(symbol.canCauseArtifactsBetweenAdjacentTiles())
        symbol.deleteSymbolLayer(0)
        self.assertTrue(symbol.canCauseArtifactsBetweenAdjacentTiles())

    def testGeometryRendering(self):
        '''Tests rendering a bunch of different geometries, including bad/odd geometries.'''

        empty_multipolygon = QgsMultiPolygon()
        empty_multipolygon.addGeometry(QgsPolygon())
        empty_polygon = QgsPolygon()
        empty_linestring = QgsLineString()

        tests = [{'name': 'Point',
                  'wkt': 'Point (1 2)',
                  'reference_image': 'point'},
                 {'name': 'MultiPoint',
                  'wkt': 'MultiPoint ((10 30),(40 20),(30 10),(20 10))',
                  'reference_image': 'multipoint'},
                 {'name': 'LineString',
                  'wkt': 'LineString (0 0,3 4,4 3)',
                  'reference_image': 'linestring'},
                 {'name': 'Empty LineString',
                  'geom': QgsGeometry(empty_linestring),
                  'reference_image': 'empty'},
                 {'name': 'MultiLineString',
                  'wkt': 'MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 0), (3 1, 5 1, 5 0, 6 0))',
                  'reference_image': 'multilinestring'},
                 {'name': 'Polygon',
                  'wkt': 'Polygon ((0 0, 10 0, 10 10, 0 10, 0 0),(5 5, 7 5, 7 7 , 5 7, 5 5))',
                  'reference_image': 'polygon'},
                 {'name': 'Empty Polygon',
                  'geom': QgsGeometry(empty_polygon),
                  'reference_image': 'empty'},
                 {'name': 'MultiPolygon',
                  'wkt': 'MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),((4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0)))',
                  'reference_image': 'multipolygon'},
                 {'name': 'Empty MultiPolygon',
                  'geom': QgsGeometry(empty_multipolygon),
                  'reference_image': 'empty'},
                 {'name': 'CircularString',
                  'wkt': 'CIRCULARSTRING(268 415,227 505,227 406)',
                  'reference_image': 'circular_string',
                  'clip_size': 30},
                 {'name': 'CompoundCurve',
                  'wkt': 'COMPOUNDCURVE((5 3, 5 13), CIRCULARSTRING(5 13, 7 15, 9 13), (9 13, 9 3), CIRCULARSTRING(9 3, 7 1, 5 3))',
                  'reference_image': 'compound_curve',
                  'clip_size': 30},
                 {'name': 'CurvePolygon',
                  'wkt': 'CURVEPOLYGON(CIRCULARSTRING(1 3, 3 5, 4 7, 7 3, 1 3))',
                  'reference_image': 'curve_polygon'},
                 {'name': 'MultiCurve',
                  'wkt': 'MultiCurve((5 5,3 5,3 3,0 3),CIRCULARSTRING(0 0, 2 1,2 2))',
                  'reference_image': 'multicurve'},
                 {'name': 'CurvePolygon_no_arc',  # refs #14028
                  'wkt': 'CURVEPOLYGON(LINESTRING(1 3, 3 5, 4 7, 7 3, 1 3))',
                  'reference_image': 'curve_polygon_no_arc'}]

        for test in tests:

            def get_geom():
                if 'geom' not in test:
                    geom = QgsGeometry.fromWkt(test['wkt'])
                    assert geom and not geom.isNull(), 'Could not create geometry {}'.format(test['wkt'])
                else:
                    geom = test['geom']
                return geom

            geom = get_geom()
            rendered_image = self.renderGeometry(geom)
            self.assertTrue(self.imageCheck(test['name'], test['reference_image'], rendered_image), test['name'])

            # Note - each test is repeated with the same geometry and reference image, but with added
            # z and m dimensions. This tests that presence of the dimensions does not affect rendering

            # test with Z
            geom_z = get_geom()
            geom_z.get().addZValue(5)
            rendered_image = self.renderGeometry(geom_z)
            assert self.imageCheck(test['name'] + 'Z', test['reference_image'], rendered_image)

            # test with ZM
            geom_z.get().addMValue(15)
            rendered_image = self.renderGeometry(geom_z)
            assert self.imageCheck(test['name'] + 'ZM', test['reference_image'], rendered_image)

            # test with M
            geom_m = get_geom()
            geom_m.get().addMValue(15)
            rendered_image = self.renderGeometry(geom_m)
            assert self.imageCheck(test['name'] + 'M', test['reference_image'], rendered_image)

            # test with clipping

            geom = get_geom()
            rendered_image = self.renderGeometry(geom, clipped_geometry=True, clip_size=test.get('clip_size', 10))
            self.assertTrue(self.imageCheck(test['name'], test['reference_image'] + '_clipped', rendered_image), test['name'])

            # test with Z
            geom_z = get_geom()
            geom_z.get().addZValue(5)
            rendered_image = self.renderGeometry(geom_z, clipped_geometry=True, clip_size=test.get('clip_size', 10))
            assert self.imageCheck(test['name'] + 'Z', test['reference_image'] + '_clipped', rendered_image)

            # test with ZM
            geom_z.get().addMValue(15)
            rendered_image = self.renderGeometry(geom_z, clipped_geometry=True, clip_size=test.get('clip_size', 10))
            assert self.imageCheck(test['name'] + 'ZM', test['reference_image'] + '_clipped', rendered_image)

            # test with M
            geom_m = get_geom()
            geom_m.get().addMValue(15)
            rendered_image = self.renderGeometry(geom_m, clipped_geometry=True, clip_size=test.get('clip_size', 10))
            assert self.imageCheck(test['name'] + 'M', test['reference_image'] + '_clipped', rendered_image)

    def renderGeometry(self, geom, clipped_geometry=False, clip_size=10):
        f = QgsFeature()
        f.setGeometry(geom)

        image = QImage(200, 200, QImage.Format_RGB32)

        painter = QPainter()
        ms = QgsMapSettings()
        extent = geom.get().boundingBox()
        # buffer extent by 10%
        if extent.width() > 0:
            if clipped_geometry:
                extent = extent.buffered(-(extent.height() + extent.width()) / clip_size)
            else:
                extent = extent.buffered((extent.height() + extent.width()) / 20.0)
        else:
            extent = extent.buffered(10)

        ms.setExtent(extent)
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI

        painter.begin(image)
        try:
            image.fill(QColor(0, 0, 0))

            if geom.type() == QgsWkbTypes.PolygonGeometry:
                self.fill_symbol.startRender(context)
                self.fill_symbol.renderFeature(f, context)
                self.fill_symbol.stopRender(context)

            elif geom.type() == QgsWkbTypes.LineGeometry:
                self.line_symbol.startRender(context)
                self.line_symbol.renderFeature(f, context)
                self.line_symbol.stopRender(context)

            elif geom.type() == QgsWkbTypes.PointGeometry:
                self.marker_symbol.startRender(context)
                self.marker_symbol.renderFeature(f, context)
                self.marker_symbol.stopRender(context)
            else:
                self.fail("Unknown type: " + geom.type())
        finally:
            painter.end()

        return image

    def testReprojectionErrorsWhileRendering(self):
        # WKT of a polygon which causes reprojection errors while rendering
        # (apologies for the ridiculously complex wkt, but I can't find a way to reproduce with simplifiction)
        wkt = 'MultiPolygon (((16.93392988400009358 42.77094147300012139, 16.88493899800005238 42.72939687700012712, ' \
              '16.80298912900011032 42.76349518400014915, 16.85816491000014139 42.78400299700011544, ' \
              '16.93392988400009358 42.77094147300012139)),((17.38200931100010393 42.79783763200002511, ' \
              '17.65894616000011297 42.74298737200008702, 17.74887129000009622 42.69456614800010641, ' \
              '17.32374108200008322 42.79083893400003547, 17.38200931100010393 42.79783763200002511)),' \
              '((16.768565300000148 42.97223541900014254, 17.03207441500009622 42.98261139500014849, ' \
              '17.13184655000009116 42.96954987200014386, 17.20020592500009116 42.92177969000012183, ' \
              '16.85141035200010151 42.90070221600008438, 16.65544681100004709 42.92625560099999404, ' \
              '16.70679772200014668 42.96954987200014386, 16.63168379000003938 42.98261139500014849, ' \
              '16.768565300000148 42.97223541900014254)),((17.05567467500011958 43.02895742400001211, ' \
              '17.24024498800011429 43.02277252800014651, 17.74146569100011561 42.83926015800001608, ' \
              '17.70736738400009358 42.88703034100014122, 17.65334906206413734 42.8909283361407887, ' \
              '17.70158573400010482 42.91950022500007833, 17.81175988700005064 42.909862570000044, ' \
              '17.85847538200005147 42.81697418200012351, 18.22413781700009849 42.62807098500009317, ' \
              '18.43735477700010961 42.55921213800017711, 18.4371480710000526 42.4934022020000981, ' \
              '18.49642988400009358 42.41632721600008438, 18.23894290500010129 42.55906810100005089, ' \
              '18.21753991000014139 42.6201032570001388, 18.07601972700010151 42.65131256700003348, ' \
              '18.0432235040000819 42.70205312700007028, 17.90162194100014403 42.75189850500014188, ' \
              '17.8928328790000819 42.79083893400003547, 17.72095787900005348 42.8262393250000315, ' \
              '17.7618921230000808 42.77871328300012976, 17.74870853000004445 42.77204010600017625, ' \
              '17.21387780000011958 42.98261139500014849, 17.04615319100011561 42.9950625670000619, ' \
              '17.00163821700004974 43.05149974200010377, 17.05567467500011958 43.02895742400001211)),' \
              '((16.19467207100007045 43.07440827000000638, 16.254893425000148 43.06854889500006323, ' \
              '16.08716881600014403 43.01146067900008063, 16.04883873800011429 43.06517161700004692, ' \
              '16.19467207100007045 43.07440827000000638)),((16.56275475400011032 43.22898997600010773, ' \
              '16.65951582100009887 43.21596914300012315, 16.72771243600001867 43.16461823100003414, ' \
              '17.19336998800014271 43.12726471600016964, 16.67017662900013875 43.12547435099999404, ' \
              '16.37159264400014536 43.19550202000006323, 16.49642988400006516 43.21808502800014651, ' \
              '16.58326256600014403 43.18866608300005794, 16.52051842500006273 43.22898997600010773, ' \
              '16.56275475400011032 43.22898997600010773)),((16.80681399800010922 43.34247467700005529, ' \
              '16.89234459700011826 43.31220123900006058, 16.84620201900008851 43.27338288000005662, ' \
              '16.62826582100012729 43.26373932500008834, 16.50074303500014139 43.28424713700003679, ' \
              '16.42188561300008587 43.31757233300011478, 16.40577233200011165 43.33270905200011214, ' \
              '16.45346113400009358 43.35317617400009738, 16.42628014400008851 43.39411041900011412, ' \
              '16.44703209700008983 43.39484284100014122, 16.80681399800010922 43.34247467700005529)),' \
              '((16.29818769600012729 43.40363190300011809, 16.30274498800008587 43.38727448100009099, ' \
              '16.39144941500012465 43.34638092700005529, 16.348643425000148 43.33869049700003018, ' \
              '16.20045006600014403 43.40704987200003018, 16.29818769600012729 43.40363190300011809)),' \
              '((16.33415774800010922 43.50153229400014254, 16.3752547540000819 43.49017975500008504, ' \
              '16.21143639400008851 43.49005768400009231, 16.26441491000014139 43.51288483300011478, ' \
              '16.33415774800010922 43.50153229400014254)),((15.67888431100004709 43.64801666900014254, ' \
              '15.74040774800010922 43.62750885600009099, 15.67204837300002396 43.63743724200010377, ' \
              '15.60377037900013875 43.67470937700007028, 15.67888431100004709 43.64801666900014254)),' \
              '((15.36736087300005238 43.79010651200015047, 15.39568118600007551 43.7724063170000619, ' \
              '15.22779381600014403 43.87445709800014981, 15.24073326900014536 43.88076406500009341, ' \
              '15.36736087300005238 43.79010651200015047)),((15.44271894600009887 43.89907461100013109, ' \
              '15.35865319100014403 43.91937897300014981, 15.26124108200011165 44.01105377800003282, ' \
              '15.38404381600008719 43.9701602230000077, 15.44271894600009887 43.89907461100013109)),' \
              '((15.22575931100010393 44.06622955900014915, 15.25440514400008851 44.01788971600014122, ' \
              '15.12183678500014139 44.09223053600005926, 15.06251061300008587 44.16193268400012073, ' \
              '15.22575931100010393 44.06622955900014915)),((14.83545983200014007 44.15102773600013109, ' \
              '14.85726972700010151 44.15204498900000374, 14.86915123800014271 44.14052969000006499, ' \
              '14.83521569100008719 44.14166901200009363, 14.81983483200014007 44.15302155199999845, ' \
              '14.82243899800005238 44.16868724200004692, 14.83545983200014007 44.15102773600013109)),' \
              '((14.98511803500011297 44.09096914300012315, 15.21680748800008587 43.91278717700008372, ' \
              '15.13331139400011693 43.92121002800003282, 15.19450931100004709 43.87262604400017096, ' \
              '15.10661868600007551 43.92544179900015422, 14.84961998800014271 44.17560455900014915, ' \
              '14.98511803500011297 44.09096914300012315)),((14.765961134000122 44.26504140800015819, ' \
              '14.74854576900014536 44.26166413000014188, 14.73959394600012729 44.28017812700015554, ' \
              '14.79167728000007287 44.27252838700003679, 14.765961134000122 44.26504140800015819)),' \
              '((14.66138756600011561 44.30866120000014519, 14.6407983730000808 44.31183502800003282, ' \
              '14.59506269600007045 44.34711334800006455, 14.643565300000148 44.32575104400011412, ' \
              '14.66138756600011561 44.30866120000014519)),((14.81120853000004445 44.35004303600000242, ' \
              '14.75619550900009358 44.36399974200004692, 14.76343834700008983 44.41535065300017493, ' \
              '14.80323326900008851 44.40550364800004957, 14.81120853000004445 44.35004303600000242)),' \
              '((14.27116946700002131 44.61253489800004957, 14.23259524800005238 44.62604401200012205, ' \
              '14.2657983730000808 44.67951080900003547, 14.28044681100007551 44.67755768400009231, ' \
              '14.27116946700002131 44.61253489800004957)),((14.84522545700008322 44.60053131700011875, ' \
              '14.93824303500014139 44.59414297100001079, 15.07553144600007045 44.48407623900006058, ' \
              '14.91114342500011958 44.54547760600014783, 15.04802493600004709 44.43943919500001982, ' \
              '15.09669030000009116 44.41518789300000947, 15.04151451900014536 44.47662995000008834, ' \
              '15.25440514400008851 44.34003327000000638, 15.165049675000148 44.36737702000006323, ' \
              '15.22022545700008322 44.3127302100001117, 15.13086998800008587 44.33258698100003414, ' \
              '15.17237389400014536 44.29913971600016964, 15.12875410200007309 44.31199778900018771, ' \
              '15.08920332100009887 44.37421295800000109, 15.11719811300014271 44.38719310099999404, ' \
              '15.04900149800010922 44.39468008000015686, 14.89747155000009116 44.49091217699999845, ' \
              '14.91863040500010129 44.50454336100013109, 14.87696373800011429 44.55975983300005794, ' \
              '14.73365319100008719 44.70319245000014519, 14.84522545700008322 44.60053131700011875)),' \
              '((14.41000410200010151 44.60097890800001608, 14.52662194100011561 44.50372955900012073, ' \
              '14.53435306100010393 44.48407623900006058, 14.42261803500008455 44.57387929900009738, ' \
              '14.36304772200014668 44.57343170800000109, 14.38257897200014668 44.60325755399999537, ' \
              '14.33578535200007309 44.71678294500010509, 14.39747155000009116 44.6856143250000315, ' \
              '14.41000410200010151 44.60097890800001608)),((14.75326582100007045 44.84585195500012844, ' \
              '14.74048912900011032 44.82050202000000638, 14.82243899800005238 44.77142975500005662, ' \
              '14.84961998800014271 44.70319245000014519, 14.65788821700004974 44.79877350500014188, ' \
              '14.7268172540000819 44.79877350500014188, 14.6858016290000819 44.8471540390000456, ' \
              '14.75326582100007045 44.84585195500012844)),((14.47103925900006516 44.95392487200003018, ' \
              '14.45191491000008455 44.79877350500014188, 14.47217858200011165 44.7079531920000619, ' \
              '14.53435306100010393 44.63426341400010244, 14.51335696700007816 44.618841864000089, ' \
              '14.42790774800005238 44.65656159100014122, 14.29420006600008719 44.9086367860001161, ' \
              '14.30152428500011297 44.94342682500014519, 14.38738040500004445 44.90900299700003018, ' \
              '14.39031009200004974 44.96039459800012139, 14.41138756600008719 44.95636627800014651, ' \
              '14.27849368600004709 45.1133487000000315, 14.29957116000014139 45.16233958499999801, ' \
              '14.35621178500014139 45.16925690300008966, 14.387705925000148 45.03904857000013351, ' \
              '14.47103925900006516 44.95392487200003018)),((14.56332441500012465 45.24974192900008063, ' \
              '14.62378991000011297 45.17548248900006058, 14.59742272200011826 45.16644928600005926, ' \
              '14.66529381600011561 45.16181061400011743, 14.66529381600011561 45.08734772300006455, ' \
              '14.74048912900011032 45.07306549700014386, 14.81495201900008851 44.97748444200009033, ' \
              '14.70639082100009887 44.9467227230000077, 14.62891686300014271 44.97817617400004053, ' \
              '14.62086022200008983 45.04559967700011214, 14.61695397200008983 45.02464427300007799, ' \
              '14.51050866000014139 45.03217194200011875, 14.43873131600014403 45.07050202000006323, ' \
              '14.4670516290000819 45.12409088700015047, 14.53012129000009622 45.13483307500014519, ' \
              '14.53435306100010393 45.23753489800002114, 14.56332441500012465 45.24974192900008063)),' \
              '((16.36947066200013978 46.54057118800012915, 16.63767134600004738 46.47447703100009164, ' \
              '16.75508020000012266 46.38187286400001597, 16.83765913900006694 46.38187286400001597, ' \
              '16.88923221800007468 46.29216257800014489, 17.05294315600005461 46.15346303300005104, ' \
              '17.20859257000006437 46.11656606000003933, 17.27587528500004055 46.01202463800002818, ' \
              '17.31680301900004793 45.99765859000002877, 17.29013798000011093 45.98463612900009423, ' \
              '17.40620324700006449 45.94365671800015605, 17.59110152100009827 45.93621531200012953, ' \
              '17.65652388500006964 45.84541982000014571, 17.80917606600013414 45.81441396100005647, ' \
              '17.85806197100004056 45.77172922800004073, 18.21121870900006456 45.78537180600012846, ' \
              '18.40438521300006869 45.74180857400001798, 18.57347049900010916 45.81668772400014689, ' \
              '18.6556360270001278 45.90758656800015558, 18.7755253500000947 45.88283355700004051, ' \
              '18.90130578600007993 45.93120269800006383, 18.87288374800004931 45.89523590100002082, ' \
              '18.90699019400011593 45.86795074500018643, 18.85531376100007606 45.85735707600009903, ' \
              '18.84497847500006174 45.8157058720000947, 18.96848514800012708 45.66873809800016204, ' \
              '18.90357954900008508 45.57308502200005762, 18.94171675700005153 45.53892689999999277, ' \
              '19.01809452300011571 45.56740061400002162, 19.10625451700005328 45.51164174500017623, ' \
              '19.00961958800010621 45.49867095900005154, 19.00300500400010151 45.45536611000007099, ' \
              '19.03742150900006891 45.42229319300010104, 18.97592655400006834 45.39495636000008005, ' \
              '19.09199182100007874 45.34999786400005917, 19.12475467900009107 45.29811472600006539, ' \
              '19.36308638500014467 45.24824696900010679, 19.40783817500010855 45.20313344400013023, ' \
              '19.39068160000005037 45.16933705700016333, 19.22593713300008744 45.16194732700016345, ' \
              '19.12186079900010327 45.195795390000157, 19.13767378700009658 45.14603098600004216, ' \
              '19.04486291500009543 45.13724599300006446, 19.08227665200013234 45.08494944300004192, ' \
              '19.0872375890000967 44.97710072800013847, 19.13167932100006396 44.95317454000003465, ' \
              '19.06667036900009293 44.90568389900012392, 18.99142948400006503 44.9149339800001286, ' \
              '19.01582076000008215 44.86563466400004074, 18.88962691200009658 44.86119049100013001, ' \
              '18.78338016700013213 44.91374542300012251, 18.79175174900009893 45.00154368100008639, ' \
              '18.73831831900008638 45.0159097290000858, 18.68405806500004473 45.08479441400000098, ' \
              '18.64871138500012648 45.06267689999999959, 18.61667199700013953 45.09766184500010411, ' \
              '18.54959598800010667 45.09476796500011631, 18.51703983500007666 45.05585561200003042, ' \
              '18.23788374800011525 45.15745147700012296, 18.15365116400005263 45.0975584930001645, ' \
              '18.00347945100011771 45.1493382780000303, 17.83573775200005684 45.0644338990000648, ' \
              '17.68473921700012852 45.1639627080000281, 17.48185754400009273 45.11440500900012296, ' \
              '17.49622359200009214 45.1416901650001563, 17.44775109900012922 45.13430043600014585, ' \
              '17.44330692500011537 45.16205068000009248, 17.38243208800008688 45.1396231090000839, ' \
              '17.26895064300006766 45.18954254200015441, 17.24548954300007608 45.15538442000017483, ' \
              '17.18709517400012032 45.14856313100001728, 17.0363033440001459 45.23047027600007652, ' \
              '17.00829471800011561 45.21615590500009318, 17.00829471800011561 45.24416453100009505, ' \
              '16.94731652900014751 45.23568959600000028, 16.9243721930001243 45.28452382500016427, ' \
              '16.81171757000004163 45.18122263700009, 16.52894413300009546 45.22225372400005483, ' \
              '16.38921106000003647 45.11683380099999852, 16.31624393700010955 45.00123362300008978, ' \
              '16.12152714000009723 45.09616322900008356, 16.02044803900011516 45.213933818000001, ' \
              '15.79234826700013627 45.18980092400012438, 15.76361617000014803 44.97555043600003444, ' \
              '15.7308533120001357 44.92723297200008403, 15.77343469200010873 44.84501576800015243, ' \
              '15.71607385200013596 44.80320953400008932, 15.72847619600008784 44.76910308800002269, ' \
              '15.80568078600006743 44.69665273000013883, 15.88877648900006534 44.72424794500012979, ' \
              '15.96897831200004703 44.63924021400013942, 16.02830285600006732 44.62471913700009907, ' \
              '16.04473596200011798 44.58937245700018082, 16.00608199000004106 44.54100331600012908, ' \
              '16.11646285000011858 44.52146962500013672, 16.15966434700004584 44.41610138000002905, ' \
              '16.13827030500004867 44.37760243800015303, 16.20286584400008678 44.35977406800010669, ' \
              '16.18756962000011868 44.28241444999999032, 16.21578495300011014 44.20815541600011045, ' \
              '16.32688928200008149 44.08237498000012522, 16.50103885900011846 43.99271637000008184, ' \
              '16.67859908100004418 43.8406843060001421, 16.71260217300007866 43.77151540100005889, ' \
              '17.03051558500007445 43.54847991900005866, 17.27050093600007585 43.46321380700000248, ' \
              '17.28993127500007176 43.3034302780000786, 17.44206669100009321 43.15243174300015028, ' \
              '17.6284119050001209 43.04657257100008394, 17.66272505700004558 42.96569895500012137, ' \
              '17.63450972400008254 42.950402731000068, 17.51563561300008587 42.95888906500012183, ' \
              '17.47087649800005238 43.01341380400010905, 17.50196373800014271 43.03099192900005221, ' \
              '17.43360436300014271 43.01740143400009231, 17.46021569100011561 43.03099192900005221, ' \
              '17.42611738400009358 43.06517161700004692, 17.4045516290000819 43.05149974200010377, ' \
              '17.31625410200012993 43.12726471600016964, 17.11394290500004445 43.21320221600008438, ' \
              '16.88062584700011826 43.40595123900006058, 16.62582441500009622 43.44904205900009231, ' \
              '16.52466881600011561 43.51080963700009363, 16.39144941500012465 43.51080963700009363, ' \
              '16.47339928500008455 43.5381533870001789, 16.43384850400013875 43.54975006700000506, ' \
              '16.11768639400008851 43.52448151200003679, 16.17237389400014536 43.4896914730000077, ' \
              '16.11312910200004467 43.47890859600009605, 15.95948326900011693 43.50397370000008834, ' \
              '15.987315300000148 43.54490794500010509, 15.92530358200011165 43.55857982000004824, ' \
              '15.91895592500009116 43.62872955900012073, 15.96631920700011165 43.64118073100003414, ' \
              '15.90479576900014536 43.64801666900014254, 15.95297285200010151 43.65086497599999404, ' \
              '15.95045006600008719 43.68854401200015047, 15.70630944100008719 43.76341380400005221, ' \
              '15.6174422540000819 43.82550690300017493, 15.66309655000009116 43.81297435099999404, ' \
              '15.67888431100004709 43.81928131700011875, 15.45508873800014271 43.92804596600014122, ' \
              '15.14454186300011429 44.19546133000015686, 15.15219160200012993 44.23529694200014717, ' \
              '15.11036217500011958 44.26434967700011214, 15.14063561300011429 44.28245677300013483, ' \
              '15.17660566500009622 44.24994538000005662, 15.20777428500008455 44.27277252800014651, ' \
              '15.19809004000012465 44.30166250200007028, 15.295258009000122 44.25067780199999845, ' \
              '15.30274498800008587 44.29913971600016964, 15.26124108200011165 44.33258698100003414, ' \
              '15.42448978000001603 44.26797109600006763, 15.52865644600009887 44.27179596600008438, ' \
              '15.30795332100009887 44.35439687700007028, 15.00733483200014007 44.56972890800012976, ' \
              '14.883799675000148 44.7236188820001388, 14.883799675000148 44.86147695500012844, 14.92164147200008983 ' \
              '44.95880768400009231, 14.85279381600011561 45.09365469000000815, 14.65788821700004974 ' \
              '45.19660065300017493, 14.57081139400008851 45.29364655200011214, 14.31153405000009116 ' \
              '45.34398021000005485, 14.23259524800005238 45.14935944200000506, 14.17937259200007816 ' \
              '45.13450755400005221, 14.19312584700008983 45.10561758000012844, 14.14389082100007045 ' \
              '45.05939362200003018, 14.151377800000148 44.97748444200009033, 14.06885826900014536 ' \
              '44.94953034100014122, 14.08383222700007309 44.9863955750000315, 14.04029381600014403 ' \
              '45.03896719000015025, 14.0756942070000548 44.98371002800003282, 14.02051842500011958 ' \
              '44.90110911700004692, 13.97266686300011429 44.90110911700004692, 13.99301191500009622 ' \
              '44.88129303600014453, 13.97266686300011429 44.82664622599999404, 14.00001061300008587 ' \
              '44.81305573100003414, 13.89014733200011165 44.83348216400010244, 13.91797936300014271 ' \
              '44.77826569200009033, 13.90316816500009622 44.77240631700014717, 13.89698326900011693 ' \
              '44.81305573100003414, 13.78711998800014271 44.87506745000008834, 13.84229576900008851 ' \
              '44.88812897300006455, 13.79460696700010658 44.89496491100008768, 13.77409915500007287 ' \
              '44.96381256700014717, 13.6232202480000808 45.07306549700014386, 13.61255944100014403 ' \
              '45.11786530199999845, 13.72624759200004974 45.13450755400005221, 13.5959578790000819 ' \
              '45.14541250200001343, 13.57545006600011561 45.26487864800007799, 13.60271243600001867 ' \
              '45.28534577000012007, 13.57545006600011561 45.30646393400006389, 13.60954837300005238 ' \
              '45.32013580900017757, 13.54127037900013875 45.34613678600005926, 13.50709069100014403 ' \
              '45.51190827000000638, 13.62901778100007277 45.45898346000016943, 13.75929406800014476 ' \
              '45.46316925100011019, 13.88900191200011136 45.42363678000005223, 13.98263960800005634 ' \
              '45.47531321200001742, 13.97189091000012695 45.5142255660000643, 14.09291711400010172 ' \
              '45.47391794800002174, 14.21869755100007637 45.49717234400004884, 14.37279667100006009 ' \
              '45.47784535800009564, 14.4689148350000778 45.52559438100014688, 14.49857710800012001 ' \
              '45.59618438800005435, 14.58094934100009255 45.66780792200010808, 14.66848921700008646 ' \
              '45.53396596300005683, 14.79716353300005949 45.46518463200006011, 14.88160282300009385 ' \
              '45.46978383400001178, 14.9226339110000481 45.51494903600017494, 15.13926151500010064 ' \
              '45.43004465799999991, 15.32519331800011742 45.45283396399999276, 15.36136682100004691 ' \
              '45.48203114900003641, 15.29666792800006192 45.52295888300012905, 15.2685559480001416 ' \
              '45.60166208900012919, 15.37376916500011248 45.64021270800010655, 15.25501672300006817 ' \
              '45.72346344000011698, 15.42906294700014769 45.77529490200011253, 15.45128381300008868 ' \
              '45.81513743100013869, 15.67607629400006886 45.84169911700014666, 15.65943648300003588 ' \
              '45.88882802400014782, 15.69798710100010908 46.0362092080000167, 15.58988000500005455 ' \
              '46.11351715100001059, 15.62284956800010605 46.19170359400006021, 16.01920780400010358 ' \
              '46.29882883700007312, 16.05961877400008575 46.33231516600015709, 16.0579651280001201 ' \
              '46.37753204400003426, 16.2756262620000598 46.37316538500006402, 16.23490523300009158 ' \
              '46.4933389280001137, 16.36947066200013978 46.54057118800012915))) '
        geom = QgsGeometry.fromWkt(wkt)
        f = QgsFeature()
        f.setGeometry(geom)

        image = QImage(200, 200, QImage.Format_RGB32)

        painter = QPainter()
        ms = QgsMapSettings()
        crs = QgsCoordinateReferenceSystem.fromProj(
            '+proj=ortho +lat_0=36.5 +lon_0=-118.8 +x_0=0 +y_0=0 +a=6371000 +b=6371000 +units=m +no_defs')
        self.assertTrue(crs.isValid())
        ms.setDestinationCrs(crs)
        ms.setExtent(QgsRectangle(1374999.8, 3912610.7, 4724462.5, 6505499.6))
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI
        ct = QgsCoordinateTransform(QgsCoordinateReferenceSystem('epsg:4326'),
                                    crs, QgsProject.instance())
        self.assertTrue(ct.isValid())
        context.setCoordinateTransform(ct)
        context.setExtent(ct.transformBoundingBox(ms.extent(), QgsCoordinateTransform.ReverseTransform))

        fill_symbol = QgsFillSymbol.createSimple(
            {'color': '#ffffff', 'outline_color': '#ffffff', 'outline_width': '10'})

        painter.begin(image)
        try:
            image.fill(QColor(0, 0, 0))
            fill_symbol.startRender(context)
            fill_symbol.renderFeature(f, context)
            fill_symbol.stopRender(context)
        finally:
            painter.end()

        assert self.imageCheck('Reprojection errors polygon', 'reprojection_errors_polygon', image)

        # also test linestring
        linestring = QgsGeometry(geom.constGet().boundary())
        f.setGeometry(linestring)
        line_symbol = QgsLineSymbol.createSimple({'color': '#ffffff', 'outline_width': '10'})

        image = QImage(200, 200, QImage.Format_RGB32)
        painter.begin(image)
        try:
            image.fill(QColor(0, 0, 0))
            line_symbol.startRender(context)
            line_symbol.renderFeature(f, context)
            line_symbol.stopRender(context)
        finally:
            painter.end()

        assert self.imageCheck('Reprojection errors linestring', 'reprojection_errors_linestring', image)

    def test_animation_settings(self):
        s = QgsFillSymbol()
        self.assertFalse(s.animationSettings().isAnimated())
        s.animationSettings().setIsAnimated(True)
        self.assertTrue(s.animationSettings().isAnimated())

        s.animationSettings().setFrameRate(30)
        self.assertEqual(s.animationSettings().frameRate(), 30)

        s.setForceRHR(True)
        doc = QDomDocument()
        context = QgsReadWriteContext()
        element = QgsSymbolLayerUtils.saveSymbol('test', s, doc, context)

        s2 = QgsSymbolLayerUtils.loadSymbol(element, context)
        self.assertTrue(s2.animationSettings().isAnimated())
        self.assertEqual(s2.animationSettings().frameRate(), 30)

        s3 = s2.clone()
        self.assertTrue(s3.animationSettings().isAnimated())
        self.assertEqual(s3.animationSettings().frameRate(), 30)

    def renderCollection(self, geom, symbol):
        f = QgsFeature()
        f.setGeometry(geom)

        image = QImage(200, 200, QImage.Format_RGB32)

        painter = QPainter()
        ms = QgsMapSettings()
        extent = geom.get().boundingBox()
        # buffer extent by 10%
        if extent.width() > 0:
            extent = extent.buffered((extent.height() + extent.width()) / 20.0)
        else:
            extent = extent.buffered(10)

        ms.setExtent(extent)
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI

        painter.begin(image)
        try:
            image.fill(QColor(0, 0, 0))
            symbol.startRender(context)
            symbol.renderFeature(f, context)
            symbol.stopRender(context)
        finally:
            painter.end()

        return image

    def testGeometryCollectionRender(self):
        tests = [{'name': 'Marker',
                  'wkt': 'GeometryCollection (Point(1 2))',
                  'symbol': self.marker_symbol,
                  'reference_image': 'point'},
                 {'name': 'MultiPoint',
                  'wkt': 'GeometryCollection (Point(10 30),Point(40 20),Point(30 10),Point(20 10))',
                  'symbol': self.marker_symbol,
                  'reference_image': 'multipoint'},
                 {'name': 'LineString',
                  'wkt': 'GeometryCollection( LineString (0 0,3 4,4 3) )',
                  'symbol': self.line_symbol,
                  'reference_image': 'linestring'},
                 {'name': 'MultiLineString',
                  'wkt': 'GeometryCollection (LineString(0 0, 1 0, 1 1, 2 1, 2 0), LineString(3 1, 5 1, 5 0, 6 0))',
                  'symbol': self.line_symbol,
                  'reference_image': 'multilinestring'},
                 {'name': 'Polygon',
                  'wkt': 'GeometryCollection(Polygon ((0 0, 10 0, 10 10, 0 10, 0 0),(5 5, 7 5, 7 7 , 5 7, 5 5)))',
                  'symbol': self.fill_symbol,
                  'reference_image': 'polygon'},
                 {'name': 'MultiPolygon',
                  'wkt': 'GeometryCollection( Polygon((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),Polygon((4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0)))',
                  'symbol': self.fill_symbol,
                  'reference_image': 'multipolygon'},
                 {'name': 'CircularString',
                  'wkt': 'GeometryCollection(CIRCULARSTRING(268 415,227 505,227 406))',
                  'symbol': self.line_symbol,
                  'reference_image': 'circular_string'},
                 {'name': 'CompoundCurve',
                  'wkt': 'GeometryCollection(COMPOUNDCURVE((5 3, 5 13), CIRCULARSTRING(5 13, 7 15, 9 13), (9 13, 9 3), CIRCULARSTRING(9 3, 7 1, 5 3)))',
                  'symbol': self.line_symbol,
                  'reference_image': 'compound_curve'},
                 {'name': 'CurvePolygon',
                  'wkt': 'GeometryCollection(CURVEPOLYGON(CIRCULARSTRING(1 3, 3 5, 4 7, 7 3, 1 3)))',
                  'symbol': self.fill_symbol,
                  'reference_image': 'curve_polygon'},
                 {'name': 'CurvePolygon_no_arc',  # refs #14028
                  'wkt': 'GeometryCollection(CURVEPOLYGON(LINESTRING(1 3, 3 5, 4 7, 7 3, 1 3)))',
                  'symbol': self.fill_symbol,
                  'reference_image': 'curve_polygon_no_arc'},
                 {'name': 'Mixed line symbol',
                  'wkt': 'GeometryCollection(Point(1 2), MultiPoint(3 3, 2 3), LineString (0 0,3 4,4 3), MultiLineString((3 1, 3 2, 4 2)), Polygon((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)), MultiPolygon(((4 0, 5 0, 5 1, 6 1, 6 2, 4 2, 4 0)),(( 1 4, 2 4, 1 5, 1 4))))',
                  'symbol': self.line_symbol,
                  'reference_image': 'collection_line_symbol'},
                 {'name': 'Mixed fill symbol',
                  'wkt': 'GeometryCollection(Point(1 2), MultiPoint(3 3, 2 3), LineString (0 0,3 4,4 3), MultiLineString((3 1, 3 2, 4 2)), Polygon((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)), MultiPolygon(((4 0, 5 0, 5 1, 6 1, 6 2, 4 2, 4 0)),(( 1 4, 2 4, 1 5, 1 4))))',
                  'symbol': self.fill_symbol,
                  'reference_image': 'collection_fill_symbol'},
                 {'name': 'Mixed marker symbol',
                  'wkt': 'GeometryCollection(Point(1 2), MultiPoint(3 3, 2 3), LineString (0 0,3 4,4 3), MultiLineString((3 1, 3 2, 4 2)), Polygon((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)), MultiPolygon(((4 0, 5 0, 5 1, 6 1, 6 2, 4 2, 4 0)),(( 1 4, 2 4, 1 5, 1 4))))',
                  'symbol': self.marker_symbol,
                  'reference_image': 'collection_marker_symbol'},
                 ]

        for test in tests:
            geom = QgsGeometry.fromWkt(test['wkt'])
            rendered_image = self.renderCollection(geom, test['symbol'])
            self.assertTrue(self.imageCheck(test['name'], test['reference_image'], rendered_image, '_collection'), test['name'])

    def imageCheck(self, name, reference_image, image, extra=''):
        self.report += "<h2>Render {}</h2>\n".format(name)
        temp_dir = QDir.tempPath() + '/'
        file_name = temp_dir + 'symbol_' + name + extra + ".png"
        image.save(file_name, "PNG")
        checker = QgsRenderChecker()
        checker.setControlPathPrefix("symbol")
        checker.setControlName("expected_" + reference_image)
        checker.setRenderedImage(file_name)
        checker.setColorTolerance(2)
        result = checker.compareImages(name, 20)
        self.report += checker.report()
        print((self.report))
        return result


class TestQgsMarkerSymbol(unittest.TestCase):

    def setUp(self):
        self.report = "<h1>Python QgsMarkerSymbol Tests</h1>\n"

    def tearDown(self):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(self.report)

    def testSize(self):
        # test size and setSize
        ms = QgsMapSettings()
        extent = QgsRectangle(100, 200, 100, 200)
        ms.setExtent(extent)
        ms.setOutputSize(QSize(400, 400))
        context = QgsRenderContext.fromMapSettings(ms)
        context.setScaleFactor(96 / 25.4)  # 96 DPI
        ms.setExtent(QgsRectangle(100, 150, 100, 150))
        ms.setOutputDpi(ms.outputDpi() * 2)
        context2 = QgsRenderContext.fromMapSettings(ms)
        context2.setScaleFactor(300 / 25.4)

        # create a marker symbol with a single layer
        markerSymbol = QgsMarkerSymbol()
        markerSymbol.deleteSymbolLayer(0)
        markerSymbol.appendSymbolLayer(
            QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayerBase.Star, color=QColor(255, 0, 0),
                                       strokeColor=QColor(0, 255, 0), size=10))
        self.assertEqual(markerSymbol.size(), 10)
        self.assertAlmostEqual(markerSymbol.size(context), 37.795275590551185, 3)
        self.assertAlmostEqual(markerSymbol.size(context2), 118.11023622047244, 3)
        markerSymbol.setSize(20)
        self.assertEqual(markerSymbol.size(), 20)
        self.assertEqual(markerSymbol.symbolLayer(0).size(), 20)
        self.assertAlmostEqual(markerSymbol.size(context), 75.59055118, 3)
        self.assertAlmostEqual(markerSymbol.size(context2), 236.2204724409449, 3)

        # add additional layers
        markerSymbol.appendSymbolLayer(
            QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayerBase.Star, color=QColor(255, 0, 0),
                                       strokeColor=QColor(0, 255, 0), size=10))
        markerSymbol.appendSymbolLayer(
            QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayerBase.Star, color=QColor(255, 0, 0),
                                       strokeColor=QColor(0, 255, 0), size=30))
        self.assertEqual(markerSymbol.size(), 30)
        self.assertAlmostEqual(markerSymbol.size(context), 113.38582677165356, 3)
        self.assertAlmostEqual(markerSymbol.size(context2), 354.33070866141736, 3)

        markerSymbol.setSize(3)
        self.assertEqual(markerSymbol.size(), 3)
        # layer sizes should maintain relative size
        self.assertEqual(markerSymbol.symbolLayer(0).size(), 2)
        self.assertEqual(markerSymbol.symbolLayer(1).size(), 1)
        self.assertEqual(markerSymbol.symbolLayer(2).size(), 3)

        # symbol layer in different size
        markerSymbol.symbolLayer(1).setSize(15)
        self.assertAlmostEqual(markerSymbol.size(context), 56.69291338582678, 3)
        self.assertAlmostEqual(markerSymbol.size(context2), 177.16535433070868, 3)
        markerSymbol.symbolLayer(1).setSizeUnit(QgsUnitTypes.RenderPixels)
        self.assertAlmostEqual(markerSymbol.size(context), 15, 3)
        self.assertAlmostEqual(markerSymbol.size(context2), 35.43307086614173, 3)
        markerSymbol.symbolLayer(1).setSize(45)
        self.assertAlmostEqual(markerSymbol.size(context), 45, 3)
        self.assertAlmostEqual(markerSymbol.size(context2), 45, 3)

    def testGeometryGeneratorSize(self):
        # test marker symbol size propagation to geometry generated sub marker symbols
        geomGeneratorSymbolLayer = QgsGeometryGeneratorSymbolLayer.create({'geometryModifier': '$geometry'})
        geomGeneratorSymbolLayer.setSymbolType(QgsSymbol.Marker)
        geomGeneratorSymbolLayer.subSymbol().setSize(2.5)

        markerSymbol = QgsMarkerSymbol()
        markerSymbol.deleteSymbolLayer(0)
        markerSymbol.appendSymbolLayer(geomGeneratorSymbolLayer)
        self.assertEqual(markerSymbol.size(), 2.5)

        markerSymbol.setSize(10.5)
        self.assertEqual(markerSymbol.size(), 10.5)

    def testGeometryGeneratorWidth(self):
        # test line symbol width propagation to geometry generated sub line symbols
        geomGeneratorSymbolLayer = QgsGeometryGeneratorSymbolLayer.create({'geometryModifier': '$geometry'})
        geomGeneratorSymbolLayer.setSymbolType(QgsSymbol.Line)
        geomGeneratorSymbolLayer.subSymbol().setWidth(2.5)

        lineSymbol = QgsLineSymbol()
        lineSymbol.deleteSymbolLayer(0)
        lineSymbol.appendSymbolLayer(geomGeneratorSymbolLayer)
        self.assertEqual(lineSymbol.width(), 2.5)

        lineSymbol.setWidth(10.5)
        self.assertEqual(lineSymbol.width(), 10.5)

    def testAngle(self):
        # test angle and setAngle

        # create a marker symbol with a single layer
        markerSymbol = QgsMarkerSymbol()
        markerSymbol.deleteSymbolLayer(0)
        markerSymbol.appendSymbolLayer(
            QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayerBase.Star, color=QColor(255, 0, 0),
                                       strokeColor=QColor(0, 255, 0), size=10, angle=90))
        self.assertEqual(markerSymbol.angle(), 90)
        markerSymbol.setAngle(100)
        self.assertEqual(markerSymbol.angle(), 100)
        self.assertEqual(markerSymbol.symbolLayer(0).angle(), 100)

        # add additional layers
        markerSymbol.appendSymbolLayer(
            QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayerBase.Star, color=QColor(255, 0, 0),
                                       strokeColor=QColor(0, 255, 0), size=10, angle=130))
        markerSymbol.appendSymbolLayer(
            QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayerBase.Star, color=QColor(255, 0, 0),
                                       strokeColor=QColor(0, 255, 0), size=10, angle=150))
        # should take first layer's angle
        self.assertEqual(markerSymbol.angle(), 100)
        markerSymbol.setAngle(10)
        self.assertEqual(markerSymbol.angle(), 10)
        # layer angles should maintain relative angle
        self.assertEqual(markerSymbol.symbolLayer(0).angle(), 10)
        self.assertEqual(markerSymbol.symbolLayer(1).angle(), 40)
        self.assertEqual(markerSymbol.symbolLayer(2).angle(), 60)

    def testSizeUnit(self):
        # test sizeUnit and setSizeUnit

        # create a marker symbol with a single layer
        markerSymbol = QgsMarkerSymbol()
        markerSymbol.deleteSymbolLayer(0)
        markerSymbol.appendSymbolLayer(
            QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayerBase.Star, color=QColor(255, 0, 0),
                                       strokeColor=QColor(0, 255, 0), size=10))
        self.assertEqual(markerSymbol.sizeUnit(), QgsUnitTypes.RenderMillimeters)
        markerSymbol.setSizeUnit(QgsUnitTypes.RenderMapUnits)
        self.assertEqual(markerSymbol.sizeUnit(), QgsUnitTypes.RenderMapUnits)
        self.assertEqual(markerSymbol.symbolLayer(0).sizeUnit(), QgsUnitTypes.RenderMapUnits)

        # add additional layers
        markerSymbol.appendSymbolLayer(
            QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayerBase.Star, color=QColor(255, 0, 0),
                                       strokeColor=QColor(0, 255, 0), size=10))
        markerSymbol.appendSymbolLayer(
            QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayerBase.Star, color=QColor(255, 0, 0),
                                       strokeColor=QColor(0, 255, 0), size=30))
        # should now be mixed size units
        self.assertEqual(markerSymbol.sizeUnit(), QgsUnitTypes.RenderUnknownUnit)
        markerSymbol.setSizeUnit(QgsUnitTypes.RenderPixels)
        self.assertEqual(markerSymbol.sizeUnit(), QgsUnitTypes.RenderPixels)
        # all layers should have size unit set
        self.assertEqual(markerSymbol.symbolLayer(0).sizeUnit(), QgsUnitTypes.RenderPixels)
        self.assertEqual(markerSymbol.symbolLayer(1).sizeUnit(), QgsUnitTypes.RenderPixels)
        self.assertEqual(markerSymbol.symbolLayer(2).sizeUnit(), QgsUnitTypes.RenderPixels)

    def testSizeMapUnitScale(self):
        # test sizeMapUnitScale and setSizeMapUnitScale

        # create a marker symbol with a single layer
        markerSymbol = QgsMarkerSymbol()
        markerSymbol.deleteSymbolLayer(0)
        markerSymbol.appendSymbolLayer(
            QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayerBase.Star, color=QColor(255, 0, 0),
                                       strokeColor=QColor(0, 255, 0), size=10))
        markerSymbol.symbolLayer(0).setSizeMapUnitScale(QgsMapUnitScale(10000, 20000))
        self.assertEqual(markerSymbol.sizeMapUnitScale(), QgsMapUnitScale(10000, 20000))
        markerSymbol.setSizeMapUnitScale(QgsMapUnitScale(1000, 2000))
        self.assertEqual(markerSymbol.sizeMapUnitScale(), QgsMapUnitScale(1000, 2000))
        self.assertEqual(markerSymbol.symbolLayer(0).sizeMapUnitScale(), QgsMapUnitScale(1000, 2000))

        # add additional layers
        markerSymbol.appendSymbolLayer(
            QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayerBase.Star, color=QColor(255, 0, 0),
                                       strokeColor=QColor(0, 255, 0), size=10))
        markerSymbol.appendSymbolLayer(
            QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayerBase.Star, color=QColor(255, 0, 0),
                                       strokeColor=QColor(0, 255, 0), size=30))
        # should take first layer's map unit scale
        self.assertEqual(markerSymbol.sizeMapUnitScale(), QgsMapUnitScale(1000, 2000))
        markerSymbol.setSizeMapUnitScale(QgsMapUnitScale(3000, 4000))
        self.assertEqual(markerSymbol.sizeMapUnitScale(), QgsMapUnitScale(3000, 4000))
        # all layers should have size unit set
        self.assertEqual(markerSymbol.symbolLayer(0).sizeMapUnitScale(), QgsMapUnitScale(3000, 4000))
        self.assertEqual(markerSymbol.symbolLayer(1).sizeMapUnitScale(), QgsMapUnitScale(3000, 4000))
        self.assertEqual(markerSymbol.symbolLayer(2).sizeMapUnitScale(), QgsMapUnitScale(3000, 4000))

    def testBoundDisabledSymbolLayer(self):
        # test calculating symbol bounds with a disabled symbol layer
        s = QgsMarkerSymbol()
        s.deleteSymbolLayer(0)
        s.appendSymbolLayer(
            QgsSimpleMarkerSymbolLayer(size=10, color=QColor(255, 255, 0)))
        s[0].setStrokeStyle(Qt.NoPen)

        # larger layer, but disabled. Should not be considered in the bounds
        s.appendSymbolLayer(
            QgsSimpleMarkerSymbolLayer(size=20, color=QColor(255, 255, 0)))
        s[1].setStrokeStyle(Qt.NoPen)
        s[1].setEnabled(False)

        g = QgsGeometry.fromWkt('Point(1 1)')
        rendered_image = self.renderGeometry(s, g, QgsMapSettings.DrawSymbolBounds)
        self.assertTrue(self.imageCheck('marker_bounds_layer_disabled', 'marker_bounds_layer_disabled', rendered_image))

        # with data defined visibility
        s[1].setEnabled(True)
        s[1].setDataDefinedProperty(QgsSymbolLayer.PropertyLayerEnabled, QgsProperty.fromExpression('false'))
        rendered_image = self.renderGeometry(s, g, QgsMapSettings.DrawSymbolBounds)
        self.assertTrue(self.imageCheck('marker_bounds_layer_disabled', 'marker_bounds_layer_disabled', rendered_image))

    def test_animation(self):
        markerSymbol = QgsMarkerSymbol()
        markerSymbol.deleteSymbolLayer(0)
        markerSymbol.appendSymbolLayer(
            QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayerBase.Triangle, color=QColor(255, 0, 0),
                                       strokeColor=QColor(0, 255, 0), size=10, angle=0))
        markerSymbol[0].setStrokeStyle(Qt.NoPen)

        markerSymbol.animationSettings().setIsAnimated(True)

        markerSymbol[0].setDataDefinedProperty(QgsSymbolLayer.PropertyAngle, QgsProperty.fromExpression('@symbol_frame * 90'))
        g = QgsGeometry.fromWkt('Point(1 1)')
        rendered_image = self.renderGeometry(markerSymbol, g, frame=0)
        self.assertTrue(self.imageCheck('animated_frame1', 'animated_frame1', rendered_image))
        rendered_image = self.renderGeometry(markerSymbol, g, frame=1)
        self.assertTrue(self.imageCheck('animated_frame2', 'animated_frame2', rendered_image))

    def renderGeometry(self, symbol, geom, flags=QgsMapSettings.Flags(), frame=None):
        f = QgsFeature()
        f.setGeometry(geom)

        image = QImage(200, 200, QImage.Format_RGB32)

        painter = QPainter()
        ms = QgsMapSettings()
        if flags:
            ms.setFlags(flags)
        extent = geom.get().boundingBox()
        # buffer extent by 10%
        if extent.width() > 0:
            extent = extent.buffered((extent.height() + extent.width()) / 20.0)
        else:
            extent = extent.buffered(10)

        ms.setExtent(extent)
        ms.setOutputSize(image.size())
        if frame is not None:
            ms.setFrameRate(10)
            ms.setCurrentFrame(frame)
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI

        painter.begin(image)
        try:
            image.fill(QColor(0, 0, 0))
            symbol.startRender(context)
            symbol.renderFeature(f, context)
            symbol.stopRender(context)
        finally:
            painter.end()

        return image

    def imageCheck(self, name, reference_image, image):
        self.report += "<h2>Render {}</h2>\n".format(name)
        temp_dir = QDir.tempPath() + '/'
        file_name = temp_dir + 'symbol_' + name + ".png"
        image.save(file_name, "PNG")
        checker = QgsRenderChecker()
        checker.setControlPathPrefix("symbol")
        checker.setControlName("expected_" + reference_image)
        checker.setRenderedImage(file_name)
        checker.setColorTolerance(2)
        result = checker.compareImages(name, 20)
        self.report += checker.report()
        print((self.report))
        return result


class TestQgsLineSymbol(unittest.TestCase):

    def setUp(self):
        self.report = "<h1>Python QgsLineSymbol Tests</h1>\n"

    def tearDown(self):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(self.report)

    def testWidth(self):
        # test width and setWidth
        ms = QgsMapSettings()
        extent = QgsRectangle(100, 200, 100, 200)
        ms.setExtent(extent)
        ms.setOutputSize(QSize(400, 400))
        context = QgsRenderContext.fromMapSettings(ms)
        context.setScaleFactor(96 / 25.4)  # 96 DPI
        ms.setExtent(QgsRectangle(100, 150, 100, 150))
        ms.setOutputDpi(ms.outputDpi() * 2)
        context2 = QgsRenderContext.fromMapSettings(ms)
        context2.setScaleFactor(300 / 25.4)

        # create a line symbol with a single layer
        line_symbol = QgsLineSymbol()
        line_symbol.deleteSymbolLayer(0)
        line_symbol.appendSymbolLayer(
            QgsSimpleLineSymbolLayer(color=QColor(255, 0, 0), width=10))
        self.assertEqual(line_symbol.width(), 10)
        self.assertAlmostEqual(line_symbol.width(context), 37.795275590551185, 3)
        self.assertAlmostEqual(line_symbol.width(context2), 118.11023622047244, 3)
        line_symbol.setWidth(20)
        self.assertEqual(line_symbol.width(), 20)
        self.assertEqual(line_symbol.symbolLayer(0).width(), 20)
        self.assertAlmostEqual(line_symbol.width(context), 75.59055118, 3)
        self.assertAlmostEqual(line_symbol.width(context2), 236.2204724409449, 3)

        # add additional layers
        line_symbol.appendSymbolLayer(
            QgsSimpleLineSymbolLayer(color=QColor(255, 0, 0), width=10))
        line_symbol.appendSymbolLayer(
            QgsSimpleLineSymbolLayer(color=QColor(255, 0, 0), width=30))
        self.assertEqual(line_symbol.width(), 30)
        self.assertAlmostEqual(line_symbol.width(context), 113.38582677165356, 3)
        self.assertAlmostEqual(line_symbol.width(context2), 354.33070866141736, 3)

        line_symbol.setWidth(3)
        self.assertEqual(line_symbol.width(), 3)
        # layer widths should maintain relative size
        self.assertEqual(line_symbol.symbolLayer(0).width(), 2)
        self.assertEqual(line_symbol.symbolLayer(1).width(), 1)
        self.assertEqual(line_symbol.symbolLayer(2).width(), 3)

        # symbol layer in different size
        line_symbol.symbolLayer(1).setWidth(15)
        self.assertAlmostEqual(line_symbol.width(context), 56.69291338582678, 3)
        self.assertAlmostEqual(line_symbol.width(context2), 177.16535433070868, 3)
        line_symbol.symbolLayer(1).setWidthUnit(QgsUnitTypes.RenderPixels)
        self.assertAlmostEqual(line_symbol.width(context), 15, 3)
        self.assertAlmostEqual(line_symbol.width(context2), 35.43307086614173, 3)
        line_symbol.symbolLayer(1).setWidth(45)
        self.assertAlmostEqual(line_symbol.width(context), 45, 3)
        self.assertAlmostEqual(line_symbol.width(context2), 45, 3)


class TestQgsFillSymbol(unittest.TestCase):

    def setUp(self):
        self.report = "<h1>Python QgsFillSymbol Tests</h1>\n"

    def tearDown(self):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(self.report)

    def testForceRHR(self):
        # test forcing right hand rule during rendering

        s = QgsFillSymbol()
        s.deleteSymbolLayer(0)
        s.appendSymbolLayer(
            QgsSimpleFillSymbolLayer(color=QColor(255, 0, 0), strokeColor=QColor(0, 255, 0)))
        self.assertFalse(s.forceRHR())
        s.setForceRHR(True)
        self.assertTrue(s.forceRHR())
        s.setForceRHR(False)
        self.assertFalse(s.forceRHR())

        s.setForceRHR(True)
        doc = QDomDocument()
        context = QgsReadWriteContext()
        element = QgsSymbolLayerUtils.saveSymbol('test', s, doc, context)

        s2 = QgsSymbolLayerUtils.loadSymbol(element, context)
        self.assertTrue(s2.forceRHR())

        # rendering test
        s3 = QgsFillSymbol()
        s3.deleteSymbolLayer(0)
        s3.appendSymbolLayer(
            QgsSimpleFillSymbolLayer(color=QColor(255, 200, 200), strokeColor=QColor(0, 255, 0), strokeWidth=2))
        marker_line = QgsMarkerLineSymbolLayer(True)
        marker_line.setPlacement(QgsMarkerLineSymbolLayer.FirstVertex)
        marker = QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayer.Triangle, 4)
        marker.setColor(QColor(255, 0, 0))
        marker.setStrokeStyle(Qt.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line.setSubSymbol(marker_symbol)
        s3.appendSymbolLayer(marker_line)

        g = QgsGeometry.fromWkt(
            'Polygon((0 0, 10 0, 10 10, 0 10, 0 0),(1 1, 1 2, 2 2, 2 1, 1 1),(8 8, 9 8, 9 9, 8 9, 8 8))')
        rendered_image = self.renderGeometry(s3, g)
        assert self.imageCheck('force_rhr_off', 'polygon_forcerhr_off', rendered_image)

        s3.setForceRHR(True)
        rendered_image = self.renderGeometry(s3, g)
        assert self.imageCheck('force_rhr_on', 'polygon_forcerhr_on', rendered_image)

    def renderGeometry(self, symbol, geom):
        f = QgsFeature()
        f.setGeometry(geom)

        image = QImage(200, 200, QImage.Format_RGB32)

        painter = QPainter()
        ms = QgsMapSettings()
        extent = geom.get().boundingBox()
        # buffer extent by 10%
        if extent.width() > 0:
            extent = extent.buffered((extent.height() + extent.width()) / 20.0)
        else:
            extent = extent.buffered(10)

        ms.setExtent(extent)
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI

        painter.begin(image)
        try:
            image.fill(QColor(0, 0, 0))
            symbol.startRender(context)
            symbol.renderFeature(f, context)
            symbol.stopRender(context)
        finally:
            painter.end()

        return image

    def imageCheck(self, name, reference_image, image):
        self.report += "<h2>Render {}</h2>\n".format(name)
        temp_dir = QDir.tempPath() + '/'
        file_name = temp_dir + 'symbol_' + name + ".png"
        image.save(file_name, "PNG")
        checker = QgsRenderChecker()
        checker.setControlPathPrefix("symbol")
        checker.setControlName("expected_" + reference_image)
        checker.setRenderedImage(file_name)
        checker.setColorTolerance(2)
        result = checker.compareImages(name, 20)
        self.report += checker.report()
        print((self.report))
        return result


if __name__ == '__main__':
    unittest.main()
