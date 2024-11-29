"""
***************************************************************************
    test_qgsgeometrygeneratorsymbollayer.py
    ---------------------
    Date                 : December 2015
    Copyright            : (C) 2015 by Matthias Kuhn
    Email                : matthias at opengis dot ch
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Matthias Kuhn"
__date__ = "December 2015"
__copyright__ = "(C) 2015, Matthias Kuhn"

import os

from qgis.PyQt.QtCore import QPointF, QSize
from qgis.PyQt.QtGui import QColor, QImage, QPainter, QPolygonF
from qgis.core import (
    Qgis,
    QgsArrowSymbolLayer,
    QgsCoordinateReferenceSystem,
    QgsCoordinateTransform,
    QgsFeature,
    QgsFillSymbol,
    QgsFontMarkerSymbolLayer,
    QgsFontUtils,
    QgsGeometry,
    QgsGeometryGeneratorSymbolLayer,
    QgsLineSymbol,
    QgsMapRendererSequentialJob,
    QgsMapSettings,
    QgsMarkerSymbol,
    QgsProject,
    QgsProperty,
    QgsRectangle,
    QgsReferencedGeometry,
    QgsRenderContext,
    QgsSingleSymbolRenderer,
    QgsSymbol,
    QgsSymbolLayer,
    QgsUnitTypes,
    QgsVectorLayer,
    QgsVectorLayerUtils,
)
import unittest
from qgis.testing import start_app, QgisTestCase
from qgis.testing.mocked import get_iface

from utilities import unitTestDataPath

# Convenience instances in case you may need them
# not used in this test
start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsGeometryGeneratorSymbolLayerV2(QgisTestCase):

    def setUp(self):
        self.iface = get_iface()

        polys_shp = os.path.join(TEST_DATA_DIR, "polys.shp")
        points_shp = os.path.join(TEST_DATA_DIR, "points.shp")
        lines_shp = os.path.join(TEST_DATA_DIR, "lines.shp")
        self.polys_layer = QgsVectorLayer(polys_shp, "Polygons", "ogr")
        self.points_layer = QgsVectorLayer(points_shp, "Points", "ogr")
        self.lines_layer = QgsVectorLayer(lines_shp, "Lines", "ogr")
        QgsProject.instance().addMapLayer(self.polys_layer)
        QgsProject.instance().addMapLayer(self.lines_layer)
        QgsProject.instance().addMapLayer(self.points_layer)

        # Create style
        sym1 = QgsFillSymbol.createSimple(
            {"color": "#fdbf6f", "outline_color": "black"}
        )
        sym2 = QgsLineSymbol.createSimple({"color": "#fdbf6f"})
        sym3 = QgsMarkerSymbol.createSimple(
            {"color": "#fdbf6f", "outline_color": "black"}
        )

        self.polys_layer.setRenderer(QgsSingleSymbolRenderer(sym1))
        self.lines_layer.setRenderer(QgsSingleSymbolRenderer(sym2))
        self.points_layer.setRenderer(QgsSingleSymbolRenderer(sym3))

        self.mapsettings = self.iface.mapCanvas().mapSettings()
        self.mapsettings.setOutputSize(QSize(400, 400))
        self.mapsettings.setOutputDpi(96)
        self.mapsettings.setExtent(QgsRectangle(-133, 22, -70, 52))

    def tearDown(self):
        QgsProject.instance().removeAllMapLayers()

    def test_basic(self):
        """
        Test getters/setters
        """
        sym_layer = QgsGeometryGeneratorSymbolLayer.create(
            {"geometryModifier": "centroid($geometry)"}
        )
        self.assertEqual(sym_layer.geometryExpression(), "centroid($geometry)")
        sym_layer.setGeometryExpression("project($geometry, 4, 5)")
        self.assertEqual(sym_layer.geometryExpression(), "project($geometry, 4, 5)")

        sym_layer.setSymbolType(Qgis.SymbolType.Marker)
        self.assertEqual(sym_layer.symbolType(), Qgis.SymbolType.Marker)

        sym_layer.setUnits(QgsUnitTypes.RenderUnit.RenderMillimeters)
        self.assertEqual(sym_layer.units(), QgsUnitTypes.RenderUnit.RenderMillimeters)

    def test_clone(self):
        """
        Test cloning layer
        """
        sym_layer = QgsGeometryGeneratorSymbolLayer.create(
            {"geometryModifier": "centroid($geometry)"}
        )
        sym_layer.setSymbolType(Qgis.SymbolType.Marker)
        sym_layer.setUnits(QgsUnitTypes.RenderUnit.RenderMillimeters)
        sym_layer.subSymbol().symbolLayer(0).setStrokeColor(QColor(0, 255, 255))

        layer2 = sym_layer.clone()
        self.assertEqual(layer2.symbolType(), Qgis.SymbolType.Marker)
        self.assertEqual(layer2.units(), QgsUnitTypes.RenderUnit.RenderMillimeters)
        self.assertEqual(layer2.geometryExpression(), "centroid($geometry)")
        self.assertEqual(layer2.subSymbol()[0].strokeColor(), QColor(0, 255, 255))

    def test_properties_create(self):
        """
        Test round trip through properties and create
        """
        sym_layer = QgsGeometryGeneratorSymbolLayer.create(
            {"geometryModifier": "centroid($geometry)"}
        )
        sym_layer.setSymbolType(Qgis.SymbolType.Marker)
        sym_layer.setUnits(QgsUnitTypes.RenderUnit.RenderMillimeters)

        layer2 = QgsGeometryGeneratorSymbolLayer.create(sym_layer.properties())
        self.assertEqual(layer2.symbolType(), Qgis.SymbolType.Marker)
        self.assertEqual(layer2.units(), QgsUnitTypes.RenderUnit.RenderMillimeters)
        self.assertEqual(layer2.geometryExpression(), "centroid($geometry)")

    def test_color(self):
        """
        Test that subsymbol color is returned for symbol layer
        """
        sym_layer = QgsGeometryGeneratorSymbolLayer.create(
            {"geometryModifier": "buffer($geometry, 2)"}
        )
        sym_layer.setSymbolType(Qgis.SymbolType.Fill)
        sym_layer.setUnits(QgsUnitTypes.RenderUnit.RenderMillimeters)
        sym_layer.subSymbol().symbolLayer(0).setColor(QColor(0, 255, 255))

        self.assertEqual(sym_layer.color(), QColor(0, 255, 255))

    def test_marker(self):
        sym = self.polys_layer.renderer().symbol()
        sym_layer = QgsGeometryGeneratorSymbolLayer.create(
            {"geometryModifier": "centroid($geometry)"}
        )
        sym_layer.setSymbolType(QgsSymbol.SymbolType.Marker)
        sym_layer.subSymbol().symbolLayer(0).setStrokeColor(QColor(0, 0, 0))
        sym.changeSymbolLayer(0, sym_layer)

        rendered_layers = [self.polys_layer]
        self.mapsettings.setLayers(rendered_layers)

        self.assertTrue(
            self.render_map_settings_check(
                "geometrygenerator_marker", "geometrygenerator_marker", self.mapsettings
            )
        )

    def test_mixed(self):
        sym = self.polys_layer.renderer().symbol()

        buffer_layer = QgsGeometryGeneratorSymbolLayer.create(
            {
                "geometryModifier": 'buffer($geometry, "value"/15)',
                "outline_color": "black",
            }
        )
        buffer_layer.setSymbolType(QgsSymbol.SymbolType.Fill)
        buffer_layer.subSymbol().symbolLayer(0).setStrokeColor(QColor(0, 0, 0))
        self.assertIsNotNone(buffer_layer.subSymbol())
        sym.appendSymbolLayer(buffer_layer)
        marker_layer = QgsGeometryGeneratorSymbolLayer.create(
            {"geometryModifier": "centroid($geometry)", "outline_color": "black"}
        )
        marker_layer.setSymbolType(QgsSymbol.SymbolType.Marker)
        marker_layer.subSymbol().symbolLayer(0).setStrokeColor(QColor(0, 0, 0))
        sym.appendSymbolLayer(marker_layer)

        rendered_layers = [self.polys_layer]
        self.mapsettings.setLayers(rendered_layers)

        self.assertTrue(
            self.render_map_settings_check(
                "geometrygenerator_mixed", "geometrygenerator_mixed", self.mapsettings
            )
        )

    def test_buffer_lines(self):
        sym = self.lines_layer.renderer().symbol()

        buffer_layer = QgsGeometryGeneratorSymbolLayer.create(
            {
                "geometryModifier": 'buffer($geometry, "value"/15)',
                "outline_color": "black",
            }
        )
        buffer_layer.setSymbolType(QgsSymbol.SymbolType.Fill)
        self.assertIsNotNone(buffer_layer.subSymbol())
        sym.appendSymbolLayer(buffer_layer)

        rendered_layers = [self.lines_layer]
        self.mapsettings.setLayers(rendered_layers)

        self.assertTrue(
            self.render_map_settings_check(
                "geometrygenerator_buffer_lines",
                "geometrygenerator_buffer_lines",
                self.mapsettings,
            )
        )

    def test_buffer_points(self):
        sym = self.points_layer.renderer().symbol()

        buffer_layer = QgsGeometryGeneratorSymbolLayer.create(
            {
                "geometryModifier": 'buffer($geometry, "staff"/15)',
                "outline_color": "black",
            }
        )
        buffer_layer.setSymbolType(QgsSymbol.SymbolType.Fill)
        self.assertIsNotNone(buffer_layer.subSymbol())
        sym.appendSymbolLayer(buffer_layer)

        rendered_layers = [self.points_layer]
        self.mapsettings.setLayers(rendered_layers)

        self.assertTrue(
            self.render_map_settings_check(
                "geometrygenerator_buffer_points",
                "geometrygenerator_buffer_points",
                self.mapsettings,
            )
        )

    def test_units_millimeters(self):
        sym = self.points_layer.renderer().symbol()

        buffer_layer = QgsGeometryGeneratorSymbolLayer.create(
            {"geometryModifier": 'buffer($geometry, "staff")', "outline_color": "black"}
        )
        buffer_layer.setSymbolType(QgsSymbol.SymbolType.Fill)
        buffer_layer.setUnits(QgsUnitTypes.RenderUnit.RenderMillimeters)
        self.assertIsNotNone(buffer_layer.subSymbol())
        sym.appendSymbolLayer(buffer_layer)

        rendered_layers = [self.points_layer]
        self.mapsettings.setLayers(rendered_layers)

        self.assertTrue(
            self.render_map_settings_check(
                "geometrygenerator_millimeters",
                "geometrygenerator_millimeters",
                self.mapsettings,
            )
        )

    def test_multi_poly_opacity(self):
        # test that multi-type features are only rendered once

        multipoly = QgsVectorLayer(
            os.path.join(TEST_DATA_DIR, "multipatch.shp"), "Polygons", "ogr"
        )

        sym = QgsFillSymbol.createSimple(
            {"color": "#77fdbf6f", "outline_color": "black"}
        )

        buffer_layer = QgsGeometryGeneratorSymbolLayer.create(
            {"geometryModifier": "buffer($geometry, -0.01)", "outline_color": "black"}
        )
        buffer_layer.setSymbolType(QgsSymbol.SymbolType.Fill)
        buffer_layer.setSubSymbol(sym)
        geom_symbol = QgsFillSymbol()
        geom_symbol.changeSymbolLayer(0, buffer_layer)
        multipoly.renderer().setSymbol(geom_symbol)

        mapsettings = QgsMapSettings(self.mapsettings)
        mapsettings.setExtent(multipoly.extent())
        mapsettings.setLayers([multipoly])

        self.assertTrue(
            self.render_map_settings_check(
                "geometrygenerator_opacity", "geometrygenerator_opacity", mapsettings
            )
        )

    def test_generator_with_multipart_result_with_generator_subsymbol(self):
        """
        Test that generator subsymbol of generator renders all parts of multipart geometry results
        """
        lines = QgsVectorLayer("MultiLineString?crs=epsg:4326", "Lines", "memory")
        self.assertTrue(lines.isValid())
        f = QgsFeature()
        f.setGeometry(
            QgsGeometry.fromWkt("MultiLineString((1 1, 2 1, 2 2),(3 1, 3 2, 4 2))")
        )
        lines.dataProvider().addFeature(f)

        sym = QgsLineSymbol.createSimple({"color": "#fffdbf6f", "outline_width": 1})

        parent_generator = QgsGeometryGeneratorSymbolLayer.create(
            {"geometryModifier": "segments_to_lines($geometry)"}
        )
        parent_generator.setSymbolType(QgsSymbol.SymbolType.Line)

        child_generator = QgsGeometryGeneratorSymbolLayer.create(
            {
                "geometryModifier": "collect_geometries(offset_curve($geometry, -2), offset_curve($geometry,2))"
            }
        )
        child_generator.setUnits(QgsUnitTypes.RenderUnit.RenderMillimeters)
        child_generator.setSymbolType(QgsSymbol.SymbolType.Line)
        child_generator.setSubSymbol(sym)

        child_symbol = QgsLineSymbol()
        child_symbol.changeSymbolLayer(0, child_generator)
        parent_generator.setSubSymbol(child_symbol)

        geom_symbol = QgsLineSymbol()
        geom_symbol.changeSymbolLayer(0, parent_generator)
        lines.renderer().setSymbol(geom_symbol)

        mapsettings = QgsMapSettings(self.mapsettings)
        mapsettings.setExtent(lines.extent())
        mapsettings.setLayers([lines])

        self.assertTrue(
            self.render_map_settings_check(
                "geometrygenerator_multipart_subsymbol",
                "geometrygenerator_multipart_subsymbol",
                mapsettings,
            )
        )

    def test_no_feature(self):
        """
        Test rendering as a pure symbol, no feature associated
        """
        buffer_layer = QgsGeometryGeneratorSymbolLayer.create(
            {"geometryModifier": "buffer($geometry, 5)"}
        )
        buffer_layer.setSymbolType(QgsSymbol.SymbolType.Fill)
        buffer_layer.setUnits(QgsUnitTypes.RenderUnit.RenderMillimeters)
        self.assertIsNotNone(buffer_layer.subSymbol())

        symbol = QgsLineSymbol()
        symbol.changeSymbolLayer(0, buffer_layer)

        image = QImage(400, 400, QImage.Format.Format_RGB32)
        image.fill(QColor(255, 255, 255))
        image.setDotsPerMeterX(int(96 / 25.4 * 1000))
        image.setDotsPerMeterY(int(96 / 25.4 * 1000))
        painter = QPainter(image)

        context = QgsRenderContext.fromQPainter(painter)

        symbol.startRender(context)

        symbol.renderPolyline(
            QPolygonF([QPointF(50, 200), QPointF(100, 170), QPointF(350, 270)]),
            None,
            context,
        )

        symbol.stopRender(context)
        painter.end()

        self.assertTrue(
            self.image_check(
                "geometrygenerator_nofeature",
                "geometrygenerator_nofeature",
                image,
                allowed_mismatch=0,
            )
        )

    def test_no_feature_coordinate_transform(self):
        """
        Test rendering as a pure symbol, no feature associated, with coordinate transform
        """
        buffer_layer = QgsGeometryGeneratorSymbolLayer.create(
            {"geometryModifier": "buffer($geometry, 5)"}
        )
        buffer_layer.setSymbolType(QgsSymbol.SymbolType.Fill)
        buffer_layer.setUnits(QgsUnitTypes.RenderUnit.RenderMillimeters)
        self.assertIsNotNone(buffer_layer.subSymbol())

        symbol = QgsLineSymbol()
        symbol.changeSymbolLayer(0, buffer_layer)

        image = QImage(400, 400, QImage.Format.Format_RGB32)
        image.setDotsPerMeterX(int(96 / 25.4 * 1000))
        image.setDotsPerMeterY(int(96 / 25.4 * 1000))
        image.fill(QColor(255, 255, 255))
        painter = QPainter(image)

        context = QgsRenderContext.fromQPainter(painter)
        context.setCoordinateTransform(
            QgsCoordinateTransform(
                QgsCoordinateReferenceSystem("EPSG:4326"),
                QgsCoordinateReferenceSystem("EPSG:3857"),
                QgsProject.instance().transformContext(),
            )
        )

        symbol.startRender(context)

        symbol.renderPolyline(
            QPolygonF([QPointF(50, 200), QPointF(100, 170), QPointF(350, 270)]),
            None,
            context,
        )

        symbol.stopRender(context)
        painter.end()

        self.assertTrue(
            self.image_check(
                "geometrygenerator_nofeature",
                "geometrygenerator_nofeature",
                image,
                allowed_mismatch=0,
            )
        )

    def test_subsymbol(self):
        """
        Test rendering a generator in a subsymbol of another symbol
        """
        sym = QgsLineSymbol()
        arrow = QgsArrowSymbolLayer()
        arrow.setIsRepeated(False)
        arrow.setArrowStartWidth(10)
        arrow.setArrowWidth(5)
        arrow.setHeadLength(20)
        arrow.setHeadThickness(10)

        sym.changeSymbolLayer(0, arrow)

        self.lines_layer.renderer().setSymbol(sym)

        # here "$geometry" must refer to the created ARROW shape, NOT the original feature line geometry!
        generator_layer = QgsGeometryGeneratorSymbolLayer.create(
            {"geometryModifier": "buffer($geometry, 3)"}
        )
        generator_layer.setSymbolType(QgsSymbol.SymbolType.Fill)
        generator_layer.setUnits(QgsUnitTypes.RenderUnit.RenderMillimeters)
        self.assertIsNotNone(generator_layer.subSymbol())

        generator_layer.subSymbol().symbolLayer(0).setColor(QColor(255, 255, 255))
        generator_layer.subSymbol().symbolLayer(0).setStrokeColor(QColor(0, 0, 0))
        generator_layer.subSymbol().symbolLayer(0).setStrokeWidth(2)

        sub_symbol = QgsFillSymbol()
        sub_symbol.changeSymbolLayer(0, generator_layer)
        arrow.setSubSymbol(sub_symbol)

        rendered_layers = [self.lines_layer]
        self.mapsettings.setLayers(rendered_layers)

        self.assertTrue(
            self.render_map_settings_check(
                "geometrygenerator_subsymbol",
                "geometrygenerator_subsymbol",
                self.mapsettings,
            )
        )

    def test_geometry_function(self):
        """
        The $geometry function used in a subsymbol should refer to the generated geometry
        """
        points = QgsVectorLayer("Point?crs=epsg:4326", "Points", "memory")
        self.assertTrue(points.isValid())
        f = QgsFeature()
        f.setGeometry(QgsGeometry.fromWkt("Point(1 2)"))
        points.dataProvider().addFeature(f)

        font = QgsFontUtils.getStandardTestFont("Bold")
        font_marker = QgsFontMarkerSymbolLayer(font.family(), "x", 16)
        font_marker.setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyCharacter,
            QgsProperty.fromExpression("geom_to_wkt($geometry)"),
        )
        subsymbol = QgsMarkerSymbol()
        subsymbol.changeSymbolLayer(0, font_marker)

        parent_generator = QgsGeometryGeneratorSymbolLayer.create(
            {"geometryModifier": "translate($geometry, 1, 2)"}
        )
        parent_generator.setSymbolType(QgsSymbol.SymbolType.Marker)

        parent_generator.setSubSymbol(subsymbol)

        geom_symbol = QgsMarkerSymbol()
        geom_symbol.changeSymbolLayer(0, parent_generator)
        points.renderer().setSymbol(geom_symbol)

        mapsettings = QgsMapSettings(self.mapsettings)
        mapsettings.setExtent(QgsRectangle(0, 0, 5, 5))
        mapsettings.setLayers([points])

        self.assertTrue(
            self.render_map_settings_check(
                "geometrygenerator_function_geometry",
                "geometrygenerator_function_geometry",
                mapsettings,
            )
        )

    def test_field_geometry(self):
        """
        Use a geometry field
        """

        points = QgsVectorLayer(
            "Point?crs=epsg:2154&field=other_geom:geometry(0,0)", "Points", "memory"
        )
        f = QgsVectorLayerUtils.createFeature(
            points,
            QgsGeometry.fromWkt("Point(5 4)"),
            {
                0: QgsReferencedGeometry(
                    QgsGeometry.fromWkt("LineString(5 6, 7 8)"),
                    QgsCoordinateReferenceSystem("EPSG:4326"),
                )
            },
        )
        points.dataProvider().addFeature(f)
        other_layer = QgsGeometryGeneratorSymbolLayer.create(
            {
                "geometryModifier": '"other_geom"',
                "outline_color": "black",
                "SymbolType": "Line",
                "line_width": 2,
            }
        )
        points.renderer().symbol().changeSymbolLayer(0, other_layer)

        mapsettings = QgsMapSettings(self.mapsettings)
        mapsettings.setExtent(QgsRectangle(0, 0, 10, 10))
        mapsettings.setLayers([points])

        self.assertTrue(
            self.render_map_settings_check(
                "geometrygenerator_field_geometry",
                "geometrygenerator_field_geometry",
                mapsettings,
            )
        )

    def test_feature_geometry(self):
        """
        The geometry($currentfeature) expression used in a subsymbol should refer to the original FEATURE geometry
        """
        points = QgsVectorLayer("Point?crs=epsg:4326", "Points", "memory")
        self.assertTrue(points.isValid())
        f = QgsFeature()
        f.setGeometry(QgsGeometry.fromWkt("Point(1 2)"))
        points.dataProvider().addFeature(f)

        font = QgsFontUtils.getStandardTestFont("Bold")
        font_marker = QgsFontMarkerSymbolLayer(font.family(), "x", 16)
        font_marker.setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyCharacter,
            QgsProperty.fromExpression("geom_to_wkt(geometry($currentfeature))"),
        )
        subsymbol = QgsMarkerSymbol()
        subsymbol.changeSymbolLayer(0, font_marker)

        parent_generator = QgsGeometryGeneratorSymbolLayer.create(
            {"geometryModifier": "translate($geometry, 1, 2)"}
        )
        parent_generator.setSymbolType(QgsSymbol.SymbolType.Marker)

        parent_generator.setSubSymbol(subsymbol)

        geom_symbol = QgsMarkerSymbol()
        geom_symbol.changeSymbolLayer(0, parent_generator)
        points.renderer().setSymbol(geom_symbol)

        mapsettings = QgsMapSettings(self.mapsettings)
        mapsettings.setExtent(QgsRectangle(0, 0, 5, 5))
        mapsettings.setLayers([points])

        self.assertTrue(
            self.render_map_settings_check(
                "geometrygenerator_feature_geometry",
                "geometrygenerator_feature_geometry",
                mapsettings,
            )
        )

    def test_clipped_results_with_z(self):
        """
        See https://github.com/qgis/QGIS/issues/51796
        """
        lines = QgsVectorLayer("LineString?crs=epsg:2154", "Lines", "memory")
        self.assertTrue(lines.isValid())
        f = QgsFeature()
        f.setGeometry(
            QgsGeometry.fromWkt(
                "LineStringZ (704425.82266868802253157 7060014.33574043028056622 19.51000000000000156, 704439.59844558802433312 7060023.7300771102309227 19.69000000000000128, 704441.67482289997860789 7060020.65665366966277361 19.62999999999999901, 704428.333267995971255 7060011.65915509033948183 19.42000000000000171)"
            )
        )
        lines.dataProvider().addFeature(f)

        subsymbol = QgsFillSymbol.createSimple({"color": "#0000ff", "line_style": "no"})

        parent_generator = QgsGeometryGeneratorSymbolLayer.create(
            {"geometryModifier": "single_sided_buffer($geometry,-0.32, 1, 2)"}
        )
        parent_generator.setSymbolType(QgsSymbol.SymbolType.Fill)

        parent_generator.setSubSymbol(subsymbol)

        geom_symbol = QgsLineSymbol()
        geom_symbol.changeSymbolLayer(0, parent_generator)
        lines.renderer().setSymbol(geom_symbol)

        mapsettings = QgsMapSettings(self.mapsettings)
        mapsettings.setDestinationCrs(lines.crs())
        mapsettings.setExtent(
            QgsRectangle(704433.77, 7060006.64, 704454.78, 7060027.95)
        )
        mapsettings.setLayers([lines])

        self.assertTrue(
            self.render_map_settings_check(
                "geometrygenerator_z_clipping",
                "geometrygenerator_z_clipping",
                mapsettings,
            )
        )


if __name__ == "__main__":
    unittest.main()
