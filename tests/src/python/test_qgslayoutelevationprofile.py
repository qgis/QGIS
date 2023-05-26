"""QGIS Unit tests for QgsLayoutItemElevationProfile.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2023 by Nyall Dawson'
__date__ = '13/01/2023'
__copyright__ = 'Copyright 2023, The QGIS Project'

import os
import tempfile

from qgis.PyQt.QtCore import (
    Qt,
    QRectF
)
from qgis.PyQt.QtGui import (
    QColor,
    QImage,
    QPainter
)
from qgis.PyQt.QtTest import QSignalSpy

from qgis.core import (
    Qgis,
    QgsCoordinateReferenceSystem,
    QgsExpressionContextUtils,
    QgsFeature,
    QgsFillSymbol,
    QgsFlatTerrainProvider,
    QgsFontUtils,
    QgsGeometry,
    QgsLayout,
    QgsLayoutItemElevationProfile,
    QgsLineString,
    QgsLineSymbol,
    QgsPrintLayout,
    QgsProject,
    QgsRasterLayer,
    QgsTextFormat,
    QgsVectorLayer,
    QgsSimpleFillSymbolLayer,
    QgsLayoutItemShape,
    QgsMarkerSymbol
)
import unittest
from qgis.testing import start_app, QgisTestCase

from test_qgslayoutitem import LayoutItemTestCase
from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsLayoutItemElevationProfile(QgisTestCase, LayoutItemTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "layout_profile"

    @classmethod
    def setUpClass(cls):
        super(TestQgsLayoutItemElevationProfile, cls).setUpClass()
        cls.item_class = QgsLayoutItemElevationProfile

    def test_opacity(self):
        """
        Test rendering the profile with opacity
        """
        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()

        profile_item = QgsLayoutItemElevationProfile(layout)
        layout.addLayoutItem(profile_item)
        profile_item.attemptSetSceneRect(QRectF(10, 10, 180, 180))

        curve = QgsLineString()
        curve.fromWkt(
            'LineString (321897.18831187387695536 129916.86947759155009408, 321942.11597351566888392 129924.94403429214435164)')

        profile_item.setProfileCurve(curve)
        profile_item.setCrs(QgsCoordinateReferenceSystem())

        profile_item.plot().setXMaximum(curve.length())
        profile_item.plot().setYMaximum(14)

        profile_item.plot().xAxis().setGridIntervalMajor(10)
        profile_item.plot().xAxis().setGridIntervalMinor(5)
        profile_item.plot().xAxis().setGridMajorSymbol(QgsLineSymbol.createSimple({'color': '#ffaaff', 'width': 2}))
        profile_item.plot().xAxis().setGridMinorSymbol(
            QgsLineSymbol.createSimple({'color': '#ffffaa', 'width': 2}))

        format = QgsTextFormat()
        format.setFont(QgsFontUtils.getStandardTestFont("Bold"))
        format.setSize(20)
        format.setNamedStyle("Bold")
        format.setColor(QColor(0, 0, 0))
        profile_item.plot().xAxis().setTextFormat(format)
        profile_item.plot().xAxis().setLabelInterval(20)

        profile_item.plot().yAxis().setGridIntervalMajor(10)
        profile_item.plot().yAxis().setGridIntervalMinor(5)
        profile_item.plot().yAxis().setGridMajorSymbol(QgsLineSymbol.createSimple({'color': '#ffffaa', 'width': 2}))
        profile_item.plot().yAxis().setGridMinorSymbol(
            QgsLineSymbol.createSimple({'color': '#aaffaa', 'width': 2}))

        profile_item.plot().yAxis().setTextFormat(format)
        profile_item.plot().yAxis().setLabelInterval(10)

        profile_item.plot().setChartBorderSymbol(
            QgsFillSymbol.createSimple({'style': 'no', 'color': '#aaffaa', 'width_border': 2}))

        profile_item.setItemOpacity(0.3)

        self.assertFalse(
            profile_item.requiresRasterization()
        )
        self.assertTrue(
            profile_item.containsAdvancedEffects()
        )

        self.assertTrue(
            self.render_layout_check('opacity', layout)
        )

    def test_opacity_rendering_designer_preview(self):
        """
        Test rendering of profile opacity while in designer dialogs
        """
        p = QgsProject()
        l = QgsLayout(p)
        self.assertTrue(l.renderContext().isPreviewRender())

        l.initializeDefaults()
        profile_item = QgsLayoutItemElevationProfile(l)
        l.addLayoutItem(profile_item)
        profile_item.attemptSetSceneRect(QRectF(10, 10, 180, 180))

        curve = QgsLineString()
        curve.fromWkt(
            'LineString (321897.18831187387695536 129916.86947759155009408, 321942.11597351566888392 129924.94403429214435164)')

        profile_item.setProfileCurve(curve)
        profile_item.setCrs(QgsCoordinateReferenceSystem())

        profile_item.plot().setXMaximum(curve.length())
        profile_item.plot().setYMaximum(14)

        profile_item.plot().xAxis().setGridIntervalMajor(10)
        profile_item.plot().xAxis().setGridIntervalMinor(5)
        profile_item.plot().xAxis().setGridMajorSymbol(QgsLineSymbol.createSimple({'color': '#ffaaff', 'width': 2}))
        profile_item.plot().xAxis().setGridMinorSymbol(
            QgsLineSymbol.createSimple({'color': '#ffffaa', 'width': 2}))

        format = QgsTextFormat()
        format.setFont(QgsFontUtils.getStandardTestFont("Bold"))
        format.setSize(20)
        format.setNamedStyle("Bold")
        format.setColor(QColor(0, 0, 0))
        profile_item.plot().xAxis().setTextFormat(format)
        profile_item.plot().xAxis().setLabelInterval(20)

        profile_item.plot().yAxis().setGridIntervalMajor(10)
        profile_item.plot().yAxis().setGridIntervalMinor(5)
        profile_item.plot().yAxis().setGridMajorSymbol(QgsLineSymbol.createSimple({'color': '#ffffaa', 'width': 2}))
        profile_item.plot().yAxis().setGridMinorSymbol(
            QgsLineSymbol.createSimple({'color': '#aaffaa', 'width': 2}))

        profile_item.plot().yAxis().setTextFormat(format)
        profile_item.plot().yAxis().setLabelInterval(10)

        profile_item.plot().setChartBorderSymbol(
            QgsFillSymbol.createSimple({'style': 'no', 'color': '#aaffaa', 'width_border': 2}))

        profile_item.setItemOpacity(0.3)

        page_item = l.pageCollection().page(0)
        paper_rect = QRectF(page_item.pos().x(),
                            page_item.pos().y(),
                            page_item.rect().width(),
                            page_item.rect().height())

        im = QImage(1122, 794, QImage.Format.Format_ARGB32)
        im.fill(Qt.GlobalColor.transparent)
        im.setDotsPerMeterX(int(300 / 25.4 * 1000))
        im.setDotsPerMeterY(int(300 / 25.4 * 1000))

        spy = QSignalSpy(profile_item.previewRefreshed)

        painter = QPainter(im)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)

        l.render(painter, QRectF(0, 0, painter.device().width(), painter.device().height()), paper_rect)
        painter.end()

        # we have to wait for the preview image to refresh, then redraw
        # the item to get the actual content which was generated in the
        # background thread
        spy.wait()

        im.fill(Qt.GlobalColor.transparent)
        painter = QPainter(im)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
        l.render(painter, QRectF(0, 0, painter.device().width(), painter.device().height()), paper_rect)
        painter.end()

        self.assertTrue(self.image_check('opacity',
                                         'opacity',
                                         im, allowed_mismatch=0))

    def test_blend_mode(self):
        """
        Test rendering the profile with a blend mode
        """
        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()

        item1 = QgsLayoutItemShape(layout)
        item1.attemptSetSceneRect(QRectF(20, 20, 150, 100))
        item1.setShapeType(QgsLayoutItemShape.Shape.Rectangle)
        simple_fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, simple_fill)
        simple_fill.setColor(QColor(0, 100, 50))
        simple_fill.setStrokeColor(Qt.GlobalColor.black)
        item1.setSymbol(fill_symbol)
        layout.addLayoutItem(item1)

        profile_item = QgsLayoutItemElevationProfile(layout)
        layout.addLayoutItem(profile_item)
        profile_item.attemptSetSceneRect(QRectF(10, 10, 180, 180))

        curve = QgsLineString()
        curve.fromWkt(
            'LineString (321897.18831187387695536 129916.86947759155009408, 321942.11597351566888392 129924.94403429214435164)')

        profile_item.setProfileCurve(curve)
        profile_item.setCrs(QgsCoordinateReferenceSystem())

        profile_item.plot().setXMaximum(curve.length())
        profile_item.plot().setYMaximum(14)

        profile_item.plot().xAxis().setGridIntervalMajor(10)
        profile_item.plot().xAxis().setGridIntervalMinor(5)
        profile_item.plot().xAxis().setGridMajorSymbol(QgsLineSymbol.createSimple({'color': '#ffaaff', 'width': 2}))
        profile_item.plot().xAxis().setGridMinorSymbol(
            QgsLineSymbol.createSimple({'color': '#ffffaa', 'width': 2}))

        format = QgsTextFormat()
        format.setFont(QgsFontUtils.getStandardTestFont("Bold"))
        format.setSize(20)
        format.setNamedStyle("Bold")
        format.setColor(QColor(0, 0, 0))
        profile_item.plot().xAxis().setTextFormat(format)
        profile_item.plot().xAxis().setLabelInterval(20)

        profile_item.plot().yAxis().setGridIntervalMajor(10)
        profile_item.plot().yAxis().setGridIntervalMinor(5)
        profile_item.plot().yAxis().setGridMajorSymbol(QgsLineSymbol.createSimple({'color': '#ffffaa', 'width': 2}))
        profile_item.plot().yAxis().setGridMinorSymbol(
            QgsLineSymbol.createSimple({'color': '#aaffaa', 'width': 2}))

        profile_item.plot().yAxis().setTextFormat(format)
        profile_item.plot().yAxis().setLabelInterval(10)

        profile_item.plot().setChartBorderSymbol(
            QgsFillSymbol.createSimple({'style': 'no', 'color': '#aaffaa', 'width_border': 2}))

        profile_item.setBlendMode(QPainter.CompositionMode.CompositionMode_Darken)

        self.assertTrue(profile_item.requiresRasterization())

        self.assertTrue(self.render_layout_check("blendmode", layout))

    def test_blend_mode_designer_preview(self):
        """
        Test rendering the profile with a blend mode
        """
        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()
        self.assertTrue(layout.renderContext().isPreviewRender())

        item1 = QgsLayoutItemShape(layout)
        item1.attemptSetSceneRect(QRectF(20, 20, 150, 100))
        item1.setShapeType(QgsLayoutItemShape.Shape.Rectangle)
        simple_fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, simple_fill)
        simple_fill.setColor(QColor(0, 100, 50))
        simple_fill.setStrokeColor(Qt.GlobalColor.black)
        item1.setSymbol(fill_symbol)
        layout.addLayoutItem(item1)

        profile_item = QgsLayoutItemElevationProfile(layout)
        layout.addLayoutItem(profile_item)
        profile_item.attemptSetSceneRect(QRectF(10, 10, 180, 180))

        curve = QgsLineString()
        curve.fromWkt(
            'LineString (321897.18831187387695536 129916.86947759155009408, 321942.11597351566888392 129924.94403429214435164)')

        profile_item.setProfileCurve(curve)
        profile_item.setCrs(QgsCoordinateReferenceSystem())

        profile_item.plot().setXMaximum(curve.length())
        profile_item.plot().setYMaximum(14)

        profile_item.plot().xAxis().setGridIntervalMajor(10)
        profile_item.plot().xAxis().setGridIntervalMinor(5)
        profile_item.plot().xAxis().setGridMajorSymbol(QgsLineSymbol.createSimple({'color': '#ffaaff', 'width': 2}))
        profile_item.plot().xAxis().setGridMinorSymbol(
            QgsLineSymbol.createSimple({'color': '#ffffaa', 'width': 2}))

        format = QgsTextFormat()
        format.setFont(QgsFontUtils.getStandardTestFont("Bold"))
        format.setSize(20)
        format.setNamedStyle("Bold")
        format.setColor(QColor(0, 0, 0))
        profile_item.plot().xAxis().setTextFormat(format)
        profile_item.plot().xAxis().setLabelInterval(20)

        profile_item.plot().yAxis().setGridIntervalMajor(10)
        profile_item.plot().yAxis().setGridIntervalMinor(5)
        profile_item.plot().yAxis().setGridMajorSymbol(QgsLineSymbol.createSimple({'color': '#ffffaa', 'width': 2}))
        profile_item.plot().yAxis().setGridMinorSymbol(
            QgsLineSymbol.createSimple({'color': '#aaffaa', 'width': 2}))

        profile_item.plot().yAxis().setTextFormat(format)
        profile_item.plot().yAxis().setLabelInterval(10)

        profile_item.plot().setChartBorderSymbol(
            QgsFillSymbol.createSimple({'style': 'no', 'color': '#aaffaa', 'width_border': 2}))

        profile_item.setBlendMode(QPainter.CompositionMode.CompositionMode_Darken)

        page_item = layout.pageCollection().page(0)
        paper_rect = QRectF(
            page_item.pos().x(),
            page_item.pos().y(),
            page_item.rect().width(),
            page_item.rect().height(),
        )

        im = QImage(1122, 794, QImage.Format.Format_ARGB32)
        im.fill(Qt.GlobalColor.transparent)
        im.setDotsPerMeterX(int(300 / 25.4 * 1000))
        im.setDotsPerMeterY(int(300 / 25.4 * 1000))
        spy = QSignalSpy(profile_item.previewRefreshed)

        painter = QPainter(im)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)

        layout.render(
            painter,
            QRectF(0, 0, painter.device().width(), painter.device().height()),
            paper_rect,
        )
        painter.end()

        # we have to wait for the preview image to refresh, then redraw
        # the item to get the actual content which was generated in the
        # background thread
        spy.wait()

        im.fill(Qt.GlobalColor.transparent)
        painter = QPainter(im)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
        layout.render(painter, QRectF(0, 0, painter.device().width(), painter.device().height()), paper_rect)
        painter.end()

        self.assertTrue(
            self.image_check(
                "blendmode", "blendmode", im, allowed_mismatch=0
            )
        )

    def test_layers(self):
        project = QgsProject()
        layout = QgsPrintLayout(project)
        profile = QgsLayoutItemElevationProfile(layout)
        layout.addLayoutItem(profile)

        self.assertFalse(profile.layers())

        layer1 = QgsVectorLayer(os.path.join(unitTestDataPath(), 'france_parts.shp'), 'france', "ogr")
        self.assertTrue(layer1.isValid())
        project.addMapLayers([layer1])

        layer2 = QgsRasterLayer(os.path.join(unitTestDataPath(), 'landsat.tif'), 'landsat', "gdal")
        self.assertTrue(layer2.isValid())
        project.addMapLayers([layer2])

        layer3 = QgsVectorLayer(os.path.join(unitTestDataPath(), 'lines.shp'), 'lines', "ogr")
        self.assertTrue(layer3.isValid())
        project.addMapLayers([layer3])

        profile.setLayers([layer2, layer3])
        self.assertEqual(profile.layers(), [layer2, layer3])

        project.layoutManager().addLayout(layout)

        # test that layers are written/restored
        with tempfile.TemporaryDirectory() as temp_dir:
            self.assertTrue(project.write(os.path.join(temp_dir, 'p.qgs')))

            p2 = QgsProject()
            self.assertTrue(p2.read(os.path.join(temp_dir, 'p.qgs')))

            layout2 = p2.layoutManager().printLayouts()[0]
            profile2 = [i for i in layout2.items() if isinstance(i, QgsLayoutItemElevationProfile)][0]

            self.assertEqual([m.id() for m in profile2.layers()], [layer2.id(), layer3.id()])

    def test_settings(self):
        project = QgsProject()
        layout = QgsPrintLayout(project)
        profile = QgsLayoutItemElevationProfile(layout)
        layout.addLayoutItem(profile)
        project.layoutManager().addLayout(layout)

        # test that default settings are written/restored
        with tempfile.TemporaryDirectory() as temp_dir:
            self.assertTrue(project.write(os.path.join(temp_dir, 'p.qgs')))

            p2 = QgsProject()
            self.assertTrue(p2.read(os.path.join(temp_dir, 'p.qgs')))

            layout2 = p2.layoutManager().printLayouts()[0]
            profile2 = [i for i in layout2.items() if isinstance(i, QgsLayoutItemElevationProfile)][0]

            self.assertFalse(profile2.crs().isValid())
            self.assertEqual(profile2.tolerance(), 0)
            self.assertIsNone(profile2.profileCurve())

        curve = QgsGeometry.fromWkt('LineString(0 0, 10 10)')
        profile.setProfileCurve(curve.constGet().clone())
        self.assertEqual(profile.profileCurve().asWkt(), 'LineString (0 0, 10 10)')

        profile.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        self.assertEqual(profile.crs(), QgsCoordinateReferenceSystem('EPSG:3857'))
        profile.setTolerance(101)
        self.assertEqual(profile.tolerance(), 101)

        profile.setDistanceUnit(Qgis.DistanceUnit.Kilometers)

        # test that settings are written/restored
        with tempfile.TemporaryDirectory() as temp_dir:
            self.assertTrue(project.write(os.path.join(temp_dir, 'p.qgs')))

            p2 = QgsProject()
            self.assertTrue(p2.read(os.path.join(temp_dir, 'p.qgs')))

            layout2 = p2.layoutManager().printLayouts()[0]
            profile2 = [i for i in layout2.items() if isinstance(i, QgsLayoutItemElevationProfile)][0]

            self.assertEqual(profile2.crs(), QgsCoordinateReferenceSystem('EPSG:3857'))
            self.assertEqual(profile2.tolerance(), 101)
            self.assertEqual(profile2.profileCurve().asWkt(), 'LineString (0 0, 10 10)')
            self.assertEqual(profile2.distanceUnit(), Qgis.DistanceUnit.Kilometers)

    def test_request(self):
        """
        Test generating a request for the item
        """
        project = QgsProject()
        layout = QgsPrintLayout(project)
        profile = QgsLayoutItemElevationProfile(layout)
        layout.addLayoutItem(profile)
        project.layoutManager().addLayout(layout)

        curve = QgsGeometry.fromWkt('LineString(0 0, 10 10)')
        profile.setProfileCurve(curve.constGet().clone())
        profile.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        profile.setTolerance(101)

        QgsExpressionContextUtils.setLayoutItemVariable(profile, 'my_var', 202)

        req = profile.profileRequest()
        self.assertEqual(req.tolerance(), 101)
        self.assertEqual(req.profileCurve().asWkt(), 'LineString (0 0, 10 10)')
        self.assertEqual(req.crs(), QgsCoordinateReferenceSystem('EPSG:3857'))
        self.assertEqual(req.expressionContext().variable('my_var'), '202')

        project.elevationProperties().setTerrainProvider(QgsFlatTerrainProvider())

        req = profile.profileRequest()
        self.assertIsInstance(req.terrainProvider(), QgsFlatTerrainProvider)

    def test_draw(self):
        """
        Test rendering the layout profile item
        """
        vl = QgsVectorLayer('PolygonZ?crs=EPSG:27700', 'lines', 'memory')
        vl.setCrs(QgsCoordinateReferenceSystem())
        self.assertTrue(vl.isValid())

        for line in [
            'PolygonZ ((321829.48893365426920354 129991.38697145861806348 1, 321847.89668515208177269 129996.63588572069420479 1, 321848.97131609614007175 129979.22330882755341008 1, 321830.31725845142500475 129978.07136809575604275 1, 321829.48893365426920354 129991.38697145861806348 1))',
            'PolygonZ ((321920.00953056826256216 129924.58260190498549491 2, 321924.65299345907988027 129908.43546159457764588 2, 321904.78543491888558492 129903.99811821122420952 2, 321900.80605239619035274 129931.39860145389684476 2, 321904.84799937985371798 129931.71552911199978553 2, 321908.93646715773502365 129912.90030360443051904 2, 321914.20495146053144708 129913.67693978428724222 2, 321911.30165811872575432 129923.01272751353099011 2, 321920.00953056826256216 129924.58260190498549491 2))',
            'PolygonZ ((321923.10517279652412981 129919.61521573827485554 3, 321922.23537852568551898 129928.3598982143739704 3, 321928.60423935484141111 129934.22530528216157109 3, 321929.39881197665818036 129923.29054521876969375 3, 321930.55804549407912418 129916.53248518184409477 3, 321923.10517279652412981 129919.61521573827485554 3))',
            'PolygonZ ((321990.47451346553862095 129909.63588680300745182 4, 321995.04325810901354998 129891.84052284323843196 4, 321989.66826330573530868 129890.5092018858413212 4, 321990.78512359503656626 129886.49917887404444627 4, 321987.37291929306229576 129885.64982962771318853 4, 321985.2254804756375961 129893.81317058412241749 4, 321987.63158903241856024 129894.41078495365218259 4, 321984.34022761805681512 129907.57450046355370432 4, 321990.47451346553862095 129909.63588680300745182 4))',
                'PolygonZ ((322103.03910495212767273 129795.91051736124791205 5, 322108.25568856322206557 129804.76113295342656784 5, 322113.29666162584908307 129803.9285887333098799 5, 322117.78645010641776025 129794.48194090687320568 5, 322103.03910495212767273 129795.91051736124791205 5))']:
            f = QgsFeature()
            f.setGeometry(QgsGeometry.fromWkt(line))
            self.assertTrue(vl.dataProvider().addFeature(f))

        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Absolute)
        vl.elevationProperties().setExtrusionEnabled(True)
        vl.elevationProperties().setExtrusionHeight(7)
        fill_symbol = QgsFillSymbol.createSimple({'color': '#ff00ff', 'outline_style': 'no'})
        vl.elevationProperties().setRespectLayerSymbology(False)
        vl.elevationProperties().setProfileFillSymbol(fill_symbol)

        p = QgsProject()
        p.addMapLayer(vl)
        layout = QgsLayout(p)
        layout.initializeDefaults()

        profile_item = QgsLayoutItemElevationProfile(layout)
        layout.addLayoutItem(profile_item)
        profile_item.attemptSetSceneRect(QRectF(10, 10, 180, 180))

        curve = QgsLineString()
        curve.fromWkt(
            'LineString (321897.18831187387695536 129916.86947759155009408, 321942.11597351566888392 129924.94403429214435164)')

        profile_item.setProfileCurve(curve)
        profile_item.setCrs(QgsCoordinateReferenceSystem())

        profile_item.plot().setXMaximum(curve.length())
        profile_item.plot().setYMaximum(14)

        profile_item.plot().xAxis().setGridIntervalMajor(10)
        profile_item.plot().xAxis().setGridIntervalMinor(5)
        profile_item.plot().xAxis().setGridMajorSymbol(QgsLineSymbol.createSimple({'color': '#ffaaff', 'width': 2}))
        profile_item.plot().xAxis().setGridMinorSymbol(
            QgsLineSymbol.createSimple({'color': '#ffffaa', 'width': 2}))

        format = QgsTextFormat()
        format.setFont(QgsFontUtils.getStandardTestFont("Bold"))
        format.setSize(20)
        format.setNamedStyle("Bold")
        format.setColor(QColor(0, 0, 0))
        profile_item.plot().xAxis().setTextFormat(format)
        profile_item.plot().xAxis().setLabelInterval(20)

        profile_item.plot().yAxis().setGridIntervalMajor(10)
        profile_item.plot().yAxis().setGridIntervalMinor(5)
        profile_item.plot().yAxis().setGridMajorSymbol(QgsLineSymbol.createSimple({'color': '#ffffaa', 'width': 2}))
        profile_item.plot().yAxis().setGridMinorSymbol(
            QgsLineSymbol.createSimple({'color': '#aaffaa', 'width': 2}))

        profile_item.plot().yAxis().setTextFormat(format)
        profile_item.plot().yAxis().setLabelInterval(10)

        profile_item.plot().setChartBorderSymbol(
            QgsFillSymbol.createSimple({'style': 'no', 'color': '#aaffaa', 'width_border': 2}))

        profile_item.setLayers([vl])

        self.assertTrue(self.render_layout_check(
            'vector_layer', layout
        ))

    def test_draw_distance_units(self):
        """
        Test rendering the layout profile item with distance unit change
        """
        vl = QgsVectorLayer('PolygonZ?crs=EPSG:27700', 'lines', 'memory')
        self.assertTrue(vl.isValid())

        for line in [
            'PolygonZ ((321829.48893365426920354 129991.38697145861806348 1, 321847.89668515208177269 129996.63588572069420479 1, 321848.97131609614007175 129979.22330882755341008 1, 321830.31725845142500475 129978.07136809575604275 1, 321829.48893365426920354 129991.38697145861806348 1))',
            'PolygonZ ((321920.00953056826256216 129924.58260190498549491 2, 321924.65299345907988027 129908.43546159457764588 2, 321904.78543491888558492 129903.99811821122420952 2, 321900.80605239619035274 129931.39860145389684476 2, 321904.84799937985371798 129931.71552911199978553 2, 321908.93646715773502365 129912.90030360443051904 2, 321914.20495146053144708 129913.67693978428724222 2, 321911.30165811872575432 129923.01272751353099011 2, 321920.00953056826256216 129924.58260190498549491 2))',
            'PolygonZ ((321923.10517279652412981 129919.61521573827485554 3, 321922.23537852568551898 129928.3598982143739704 3, 321928.60423935484141111 129934.22530528216157109 3, 321929.39881197665818036 129923.29054521876969375 3, 321930.55804549407912418 129916.53248518184409477 3, 321923.10517279652412981 129919.61521573827485554 3))',
            'PolygonZ ((321990.47451346553862095 129909.63588680300745182 4, 321995.04325810901354998 129891.84052284323843196 4, 321989.66826330573530868 129890.5092018858413212 4, 321990.78512359503656626 129886.49917887404444627 4, 321987.37291929306229576 129885.64982962771318853 4, 321985.2254804756375961 129893.81317058412241749 4, 321987.63158903241856024 129894.41078495365218259 4, 321984.34022761805681512 129907.57450046355370432 4, 321990.47451346553862095 129909.63588680300745182 4))',
                'PolygonZ ((322103.03910495212767273 129795.91051736124791205 5, 322108.25568856322206557 129804.76113295342656784 5, 322113.29666162584908307 129803.9285887333098799 5, 322117.78645010641776025 129794.48194090687320568 5, 322103.03910495212767273 129795.91051736124791205 5))']:
            f = QgsFeature()
            f.setGeometry(QgsGeometry.fromWkt(line))
            self.assertTrue(vl.dataProvider().addFeature(f))

        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Absolute)
        vl.elevationProperties().setExtrusionEnabled(True)
        vl.elevationProperties().setExtrusionHeight(7)
        fill_symbol = QgsFillSymbol.createSimple({'color': '#ff00ff', 'outline_style': 'no'})
        vl.elevationProperties().setRespectLayerSymbology(False)
        vl.elevationProperties().setProfileFillSymbol(fill_symbol)

        p = QgsProject()
        p.addMapLayer(vl)
        layout = QgsLayout(p)
        layout.initializeDefaults()

        profile_item = QgsLayoutItemElevationProfile(layout)
        layout.addLayoutItem(profile_item)
        profile_item.attemptSetSceneRect(QRectF(10, 10, 180, 180))

        curve = QgsLineString()
        curve.fromWkt(
            'LineString (321897.18831187387695536 129916.86947759155009408, 321942.11597351566888392 129924.94403429214435164)')

        profile_item.setProfileCurve(curve)
        profile_item.setCrs(vl.crs())

        profile_item.plot().setXMaximum(curve.length() / 1000)
        profile_item.plot().setYMaximum(14)

        profile_item.setDistanceUnit(Qgis.DistanceUnit.Kilometers)
        profile_item.plot().xAxis().setLabelSuffixPlacement(Qgis.PlotAxisSuffixPlacement.LastLabel)
        profile_item.plot().xAxis().setGridIntervalMajor(0.010)
        profile_item.plot().xAxis().setGridIntervalMinor(0.005)
        profile_item.plot().xAxis().setGridMajorSymbol(QgsLineSymbol.createSimple({'color': '#ffaaff', 'width': 2}))
        profile_item.plot().xAxis().setGridMinorSymbol(
            QgsLineSymbol.createSimple({'color': '#ffffaa', 'width': 2}))

        format = QgsTextFormat()
        format.setFont(QgsFontUtils.getStandardTestFont("Bold"))
        format.setSize(20)
        format.setNamedStyle("Bold")
        format.setColor(QColor(0, 0, 0))
        profile_item.plot().xAxis().setTextFormat(format)
        profile_item.plot().xAxis().setLabelInterval(0.020)

        profile_item.plot().yAxis().setGridIntervalMajor(10)
        profile_item.plot().yAxis().setGridIntervalMinor(5)
        profile_item.plot().yAxis().setGridMajorSymbol(QgsLineSymbol.createSimple({'color': '#ffffaa', 'width': 2}))
        profile_item.plot().yAxis().setGridMinorSymbol(
            QgsLineSymbol.createSimple({'color': '#aaffaa', 'width': 2}))

        profile_item.plot().yAxis().setTextFormat(format)
        profile_item.plot().yAxis().setLabelInterval(10)

        profile_item.plot().setChartBorderSymbol(
            QgsFillSymbol.createSimple({'style': 'no', 'color': '#aaffaa', 'width_border': 2}))

        profile_item.setLayers([vl])

        self.assertTrue(self.render_layout_check(
            'distance_units', layout
        ))

    def test_draw_map_units(self):
        """
        Test rendering the layout profile item using symbols with map unit sizes
        """
        vl = QgsVectorLayer('LineStringZ?crs=EPSG:27700', 'lines', 'memory')
        vl.setCrs(QgsCoordinateReferenceSystem())
        self.assertTrue(vl.isValid())

        for line in [
            'LineStringZ (321829.48893365426920354 129991.38697145861806348 1, 321847.89668515208177269 129996.63588572069420479 1, 321848.97131609614007175 129979.22330882755341008 1, 321830.31725845142500475 129978.07136809575604275 1, 321829.48893365426920354 129991.38697145861806348 1)',
            'LineStringZ (321920.00953056826256216 129924.58260190498549491 2, 321924.65299345907988027 129908.43546159457764588 2, 321904.78543491888558492 129903.99811821122420952 2, 321900.80605239619035274 129931.39860145389684476 2, 321904.84799937985371798 129931.71552911199978553 2, 321908.93646715773502365 129912.90030360443051904 2, 321914.20495146053144708 129913.67693978428724222 2, 321911.30165811872575432 129923.01272751353099011 2, 321920.00953056826256216 129924.58260190498549491 2)',
            'LineStringZ (321923.10517279652412981 129919.61521573827485554 3, 321922.23537852568551898 129928.3598982143739704 3, 321928.60423935484141111 129934.22530528216157109 3, 321929.39881197665818036 129923.29054521876969375 3, 321930.55804549407912418 129916.53248518184409477 3, 321923.10517279652412981 129919.61521573827485554 3)',
            'LineStringZ (321990.47451346553862095 129909.63588680300745182 4, 321995.04325810901354998 129891.84052284323843196 4, 321989.66826330573530868 129890.5092018858413212 4, 321990.78512359503656626 129886.49917887404444627 4, 321987.37291929306229576 129885.64982962771318853 4, 321985.2254804756375961 129893.81317058412241749 4, 321987.63158903241856024 129894.41078495365218259 4, 321984.34022761805681512 129907.57450046355370432 4, 321990.47451346553862095 129909.63588680300745182 4)',
                'LineStringZ (322103.03910495212767273 129795.91051736124791205 5, 322108.25568856322206557 129804.76113295342656784 5, 322113.29666162584908307 129803.9285887333098799 5, 322117.78645010641776025 129794.48194090687320568 5, 322103.03910495212767273 129795.91051736124791205 5)']:
            f = QgsFeature()
            f.setGeometry(QgsGeometry.fromWkt(line))
            self.assertTrue(vl.dataProvider().addFeature(f))

        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Absolute)
        marker_symbol = QgsMarkerSymbol.createSimple(
            {'name': 'square', 'size': 4, 'color': '#00ff00',
             'outline_style': 'no'})
        marker_symbol.setSizeUnit(Qgis.RenderUnit.MapUnits)
        vl.elevationProperties().setRespectLayerSymbology(False)
        vl.elevationProperties().setProfileMarkerSymbol(marker_symbol)

        p = QgsProject()
        p.addMapLayer(vl)
        layout = QgsLayout(p)
        layout.initializeDefaults()

        profile_item = QgsLayoutItemElevationProfile(layout)
        layout.addLayoutItem(profile_item)
        profile_item.attemptSetSceneRect(QRectF(10, 10, 180, 180))

        curve = QgsLineString()
        curve.fromWkt(
            'LineString (321897.18831187387695536 129916.86947759155009408, 321942.11597351566888392 129924.94403429214435164)')

        profile_item.setProfileCurve(curve)
        profile_item.setCrs(QgsCoordinateReferenceSystem())

        profile_item.plot().setXMaximum(curve.length())
        profile_item.plot().setYMaximum(14)

        profile_item.plot().xAxis().setGridIntervalMajor(10)
        profile_item.plot().xAxis().setGridIntervalMinor(5)
        profile_item.plot().xAxis().setGridMajorSymbol(QgsLineSymbol.createSimple({'color': '#ffaaff', 'width': 2}))
        profile_item.plot().xAxis().setGridMinorSymbol(
            QgsLineSymbol.createSimple({'color': '#ffffaa', 'width': 2}))

        format = QgsTextFormat()
        format.setFont(QgsFontUtils.getStandardTestFont("Bold"))
        format.setSize(20)
        format.setNamedStyle("Bold")
        format.setColor(QColor(0, 0, 0))
        profile_item.plot().xAxis().setTextFormat(format)
        profile_item.plot().xAxis().setLabelInterval(20)

        profile_item.plot().yAxis().setGridIntervalMajor(10)
        profile_item.plot().yAxis().setGridIntervalMinor(5)
        profile_item.plot().yAxis().setGridMajorSymbol(QgsLineSymbol.createSimple({'color': '#ffffaa', 'width': 2}))
        profile_item.plot().yAxis().setGridMinorSymbol(
            QgsLineSymbol.createSimple({'color': '#aaffaa', 'width': 2}))

        profile_item.plot().yAxis().setTextFormat(format)
        profile_item.plot().yAxis().setLabelInterval(10)

        profile_item.plot().setChartBorderSymbol(
            QgsFillSymbol.createSimple({'style': 'no', 'color': '#aaffaa', 'width_border': 2}))

        profile_item.setLayers([vl])

        self.assertTrue(self.render_layout_check(
            'vector_layer_map_units', layout
        ))

    def test_draw_zero_label_interval(self):
        """
        Test rendering the layout profile item with 0 label intervals
        """
        vl = QgsVectorLayer('PolygonZ?crs=EPSG:27700', 'lines', 'memory')
        vl.setCrs(QgsCoordinateReferenceSystem())
        self.assertTrue(vl.isValid())

        for line in [
            'PolygonZ ((321829.48893365426920354 129991.38697145861806348 1, 321847.89668515208177269 129996.63588572069420479 1, 321848.97131609614007175 129979.22330882755341008 1, 321830.31725845142500475 129978.07136809575604275 1, 321829.48893365426920354 129991.38697145861806348 1))',
            'PolygonZ ((321920.00953056826256216 129924.58260190498549491 2, 321924.65299345907988027 129908.43546159457764588 2, 321904.78543491888558492 129903.99811821122420952 2, 321900.80605239619035274 129931.39860145389684476 2, 321904.84799937985371798 129931.71552911199978553 2, 321908.93646715773502365 129912.90030360443051904 2, 321914.20495146053144708 129913.67693978428724222 2, 321911.30165811872575432 129923.01272751353099011 2, 321920.00953056826256216 129924.58260190498549491 2))',
            'PolygonZ ((321923.10517279652412981 129919.61521573827485554 3, 321922.23537852568551898 129928.3598982143739704 3, 321928.60423935484141111 129934.22530528216157109 3, 321929.39881197665818036 129923.29054521876969375 3, 321930.55804549407912418 129916.53248518184409477 3, 321923.10517279652412981 129919.61521573827485554 3))',
            'PolygonZ ((321990.47451346553862095 129909.63588680300745182 4, 321995.04325810901354998 129891.84052284323843196 4, 321989.66826330573530868 129890.5092018858413212 4, 321990.78512359503656626 129886.49917887404444627 4, 321987.37291929306229576 129885.64982962771318853 4, 321985.2254804756375961 129893.81317058412241749 4, 321987.63158903241856024 129894.41078495365218259 4, 321984.34022761805681512 129907.57450046355370432 4, 321990.47451346553862095 129909.63588680300745182 4))',
                'PolygonZ ((322103.03910495212767273 129795.91051736124791205 5, 322108.25568856322206557 129804.76113295342656784 5, 322113.29666162584908307 129803.9285887333098799 5, 322117.78645010641776025 129794.48194090687320568 5, 322103.03910495212767273 129795.91051736124791205 5))']:
            f = QgsFeature()
            f.setGeometry(QgsGeometry.fromWkt(line))
            self.assertTrue(vl.dataProvider().addFeature(f))

        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Absolute)
        vl.elevationProperties().setExtrusionEnabled(True)
        vl.elevationProperties().setExtrusionHeight(7)
        fill_symbol = QgsFillSymbol.createSimple({'color': '#ff00ff', 'outline_style': 'no'})
        vl.elevationProperties().setRespectLayerSymbology(False)
        vl.elevationProperties().setProfileFillSymbol(fill_symbol)

        p = QgsProject()
        p.addMapLayer(vl)
        layout = QgsLayout(p)
        layout.initializeDefaults()

        profile_item = QgsLayoutItemElevationProfile(layout)
        layout.addLayoutItem(profile_item)
        profile_item.attemptSetSceneRect(QRectF(10, 10, 180, 180))

        curve = QgsLineString()
        curve.fromWkt(
            'LineString (321897.18831187387695536 129916.86947759155009408, 321942.11597351566888392 129924.94403429214435164)')

        profile_item.setProfileCurve(curve)
        profile_item.setCrs(QgsCoordinateReferenceSystem())

        profile_item.plot().setXMaximum(curve.length())
        profile_item.plot().setYMaximum(14)

        profile_item.plot().xAxis().setGridIntervalMajor(10)
        profile_item.plot().xAxis().setGridIntervalMinor(5)
        profile_item.plot().xAxis().setGridMajorSymbol(QgsLineSymbol.createSimple({'color': '#ffaaff', 'width': 2}))
        profile_item.plot().xAxis().setGridMinorSymbol(
            QgsLineSymbol.createSimple({'color': '#ffffaa', 'width': 2}))

        format = QgsTextFormat()
        format.setFont(QgsFontUtils.getStandardTestFont("Bold"))
        format.setSize(20)
        format.setNamedStyle("Bold")
        format.setColor(QColor(0, 0, 0))
        profile_item.plot().xAxis().setTextFormat(format)
        profile_item.plot().xAxis().setLabelInterval(0)

        profile_item.plot().yAxis().setGridIntervalMajor(10)
        profile_item.plot().yAxis().setGridIntervalMinor(5)
        profile_item.plot().yAxis().setGridMajorSymbol(QgsLineSymbol.createSimple({'color': '#ffffaa', 'width': 2}))
        profile_item.plot().yAxis().setGridMinorSymbol(
            QgsLineSymbol.createSimple({'color': '#aaffaa', 'width': 2}))

        profile_item.plot().yAxis().setTextFormat(format)
        profile_item.plot().yAxis().setLabelInterval(0)

        profile_item.plot().setChartBorderSymbol(
            QgsFillSymbol.createSimple({'style': 'no', 'color': '#aaffaa', 'width_border': 2}))

        profile_item.setLayers([vl])

        self.assertTrue(self.render_layout_check(
            'zero_label_interval', layout
        ))

    def test_draw_map_units_tolerance(self):
        """
        Test rendering the layout profile item using symbols with map unit sizes
        """
        vl = QgsVectorLayer('LineStringZ?crs=EPSG:27700', 'lines', 'memory')
        vl.setCrs(QgsCoordinateReferenceSystem())
        self.assertTrue(vl.isValid())

        for line in [
            'LineStringZ (321829.48893365426920354 129991.38697145861806348 1, 321847.89668515208177269 129996.63588572069420479 1, 321848.97131609614007175 129979.22330882755341008 1, 321830.31725845142500475 129978.07136809575604275 1, 321829.48893365426920354 129991.38697145861806348 1)',
            'LineStringZ (321920.00953056826256216 129924.58260190498549491 2, 321924.65299345907988027 129908.43546159457764588 2, 321904.78543491888558492 129903.99811821122420952 2, 321900.80605239619035274 129931.39860145389684476 2, 321904.84799937985371798 129931.71552911199978553 2, 321908.93646715773502365 129912.90030360443051904 2, 321914.20495146053144708 129913.67693978428724222 2, 321911.30165811872575432 129923.01272751353099011 2, 321920.00953056826256216 129924.58260190498549491 2)',
            'LineStringZ (321923.10517279652412981 129919.61521573827485554 3, 321922.23537852568551898 129928.3598982143739704 3, 321928.60423935484141111 129934.22530528216157109 3, 321929.39881197665818036 129923.29054521876969375 3, 321930.55804549407912418 129916.53248518184409477 3, 321923.10517279652412981 129919.61521573827485554 3)',
            'LineStringZ (321990.47451346553862095 129909.63588680300745182 4, 321995.04325810901354998 129891.84052284323843196 4, 321989.66826330573530868 129890.5092018858413212 4, 321990.78512359503656626 129886.49917887404444627 4, 321987.37291929306229576 129885.64982962771318853 4, 321985.2254804756375961 129893.81317058412241749 4, 321987.63158903241856024 129894.41078495365218259 4, 321984.34022761805681512 129907.57450046355370432 4, 321990.47451346553862095 129909.63588680300745182 4)',
                'LineStringZ (322103.03910495212767273 129795.91051736124791205 5, 322108.25568856322206557 129804.76113295342656784 5, 322113.29666162584908307 129803.9285887333098799 5, 322117.78645010641776025 129794.48194090687320568 5, 322103.03910495212767273 129795.91051736124791205 5)']:
            f = QgsFeature()
            f.setGeometry(QgsGeometry.fromWkt(line))
            self.assertTrue(vl.dataProvider().addFeature(f))

        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Absolute)
        line_symbol = QgsLineSymbol.createSimple({'color': '#ff00ff', 'width': '0.8'})
        line_symbol.setWidthUnit(Qgis.RenderUnit.MapUnits)
        vl.elevationProperties().setProfileLineSymbol(line_symbol)
        vl.elevationProperties().setRespectLayerSymbology(False)

        p = QgsProject()
        p.addMapLayer(vl)
        layout = QgsLayout(p)
        layout.initializeDefaults()

        profile_item = QgsLayoutItemElevationProfile(layout)
        layout.addLayoutItem(profile_item)
        profile_item.attemptSetSceneRect(QRectF(10, 10, 180, 180))

        curve = QgsLineString()
        curve.fromWkt(
            'LineString (321897.18831187387695536 129916.86947759155009408, 321942.11597351566888392 129924.94403429214435164)')

        profile_item.setProfileCurve(curve)
        profile_item.setCrs(QgsCoordinateReferenceSystem())

        profile_item.plot().setXMaximum(curve.length())
        profile_item.plot().setYMaximum(14)

        profile_item.plot().xAxis().setGridIntervalMajor(10)
        profile_item.plot().xAxis().setGridIntervalMinor(5)
        profile_item.plot().xAxis().setGridMajorSymbol(QgsLineSymbol.createSimple({'color': '#ffaaff', 'width': 2}))
        profile_item.plot().xAxis().setGridMinorSymbol(
            QgsLineSymbol.createSimple({'color': '#ffffaa', 'width': 2}))

        format = QgsTextFormat()
        format.setFont(QgsFontUtils.getStandardTestFont("Bold"))
        format.setSize(20)
        format.setNamedStyle("Bold")
        format.setColor(QColor(0, 0, 0))
        profile_item.plot().xAxis().setTextFormat(format)
        profile_item.plot().xAxis().setLabelInterval(20)

        profile_item.plot().yAxis().setGridIntervalMajor(10)
        profile_item.plot().yAxis().setGridIntervalMinor(5)
        profile_item.plot().yAxis().setGridMajorSymbol(QgsLineSymbol.createSimple({'color': '#ffffaa', 'width': 2}))
        profile_item.plot().yAxis().setGridMinorSymbol(
            QgsLineSymbol.createSimple({'color': '#aaffaa', 'width': 2}))

        profile_item.plot().yAxis().setTextFormat(format)
        profile_item.plot().yAxis().setLabelInterval(10)

        profile_item.plot().setChartBorderSymbol(
            QgsFillSymbol.createSimple({'style': 'no', 'color': '#aaffaa', 'width_border': 2}))

        profile_item.setTolerance(1)
        profile_item.setLayers([vl])

        self.assertTrue(self.render_layout_check(
            'vector_layer_map_units_tolerance', layout
        ))


if __name__ == '__main__':
    unittest.main()
