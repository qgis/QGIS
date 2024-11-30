"""QGIS Unit tests for QgsProfileSourceRegistry

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Germán Carrillo"
__date__ = "03/05/2024"
__copyright__ = "Copyright 2024, The QGIS Project"


from qgis.PyQt.QtCore import QRectF, Qt, QPointF
from qgis.PyQt.QtGui import QColor, QPainterPath, QPolygonF
from qgis.PyQt.QtTest import QSignalSpy
from qgis.core import (
    Qgis,
    QgsAbstractProfileGenerator,
    QgsAbstractProfileResults,
    QgsAbstractProfileSource,
    QgsApplication,
    QgsCoordinateReferenceSystem,
    QgsDoubleRange,
    QgsFeedback,
    QgsFillSymbol,
    QgsFontUtils,
    QgsGeometry,
    QgsLayout,
    QgsLayoutItemElevationProfile,
    QgsLineString,
    QgsLineSymbol,
    QgsMarkerSymbol,
    QgsPoint,
    QgsProfileExporter,
    QgsProfileGenerationContext,
    QgsProfileRequest,
    QgsProject,
    QgsTextFormat,
)
from qgis.gui import QgsElevationProfileCanvas

import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class MyProfileResults(QgsAbstractProfileResults):
    def __init__(self):
        QgsAbstractProfileResults.__init__(self)

        self.__profile_curve = None
        self.raw_points = []

        self.distance_to_height = {}
        self.geometries = []
        self.cross_section_geometries = []
        self.min_z = 4500
        self.max_z = -100

        self.marker_symbol = QgsMarkerSymbol.createSimple(
            {"name": "square", "size": 2, "color": "#00ff00", "outline_style": "no"}
        )

    def asFeatures(self, type, feedback):
        result = []

        if type == Qgis.ProfileExportType.Features3D:
            for g in self.geometries:
                feature = QgsAbstractProfileResults.Feature()
                feature.geometry = g
                result.append(feature)

        elif type == Qgis.ProfileExportType.Profile2D:
            for g in self.cross_section_geometries:
                feature = QgsAbstractProfileResults.Feature()
                feature.geometry = g
                result.append(feature)

        elif type == Qgis.ProfileExportType.DistanceVsElevationTable:
            for i, geom in enumerate(self.geometries):
                feature = QgsAbstractProfileResults.Feature()
                feature.geometry = geom
                p = self.cross_section_geometries[i].asPoint()
                feature.attributes = {"distance": p.x(), "elevation": p.y()}
                result.append(feature)

        return result

    def asGeometries(self):
        return self.geometries

    def sampledPoints(self):
        return self.raw_points

    def zRange(self):
        return QgsDoubleRange(self.min_z, self.max_z)

    def type(self):
        return "my-web-service-profile"

    def renderResults(self, context):
        painter = context.renderContext().painter()
        if not painter:
            return

        painter.setBrush(Qt.BrushStyle.NoBrush)
        painter.setPen(Qt.PenStyle.NoPen)

        minDistance = context.distanceRange().lower()
        maxDistance = context.distanceRange().upper()
        minZ = context.elevationRange().lower()
        maxZ = context.elevationRange().upper()

        visibleRegion = QRectF(
            minDistance, minZ, maxDistance - minDistance, maxZ - minZ
        )
        clipPath = QPainterPath()
        clipPath.addPolygon(context.worldTransform().map(QPolygonF(visibleRegion)))
        painter.setClipPath(clipPath, Qt.ClipOperation.IntersectClip)

        self.marker_symbol.startRender(context.renderContext())

        for k, v in self.distance_to_height.items():
            if not v:
                continue

            self.marker_symbol.renderPoint(
                context.worldTransform().map(QPointF(k, v)),
                None,
                context.renderContext(),
            )

        self.marker_symbol.stopRender(context.renderContext())


class MyProfileGenerator(QgsAbstractProfileGenerator):
    def __init__(self, request):
        QgsAbstractProfileGenerator.__init__(self)
        self.__request = request
        self.__profile_curve = (
            request.profileCurve().clone() if request.profileCurve() else None
        )
        self.__results = None
        self.__feedback = QgsFeedback()

    def sourceId(self):
        return "my-profile"

    def feedback(self):
        return self.__feedback

    def generateProfile(self, context):  # QgsProfileGenerationContext
        if self.__profile_curve is None:
            return False

        self.__results = MyProfileResults()

        result = [
            {"z": 454.8, "d": 0, "x": 2584085.816, "y": 1216473.232},
            {"z": 429.3, "d": 2199.9, "x": 2582027.691, "y": 1217250.279},
            {"z": 702.5, "d": 4399.9, "x": 2579969.567, "y": 1218027.326},
            {"z": 857.9, "d": 6430.1, "x": 2578394.472, "y": 1219308.404},
            {"z": 1282.7, "d": 8460.4, "x": 2576819.377, "y": 1220589.481},
        ]

        for point in result:
            if self.__feedback.isCanceled():
                return False

            x, y, z, d = point["x"], point["y"], point["z"], point["d"]
            p = QgsPoint(x, y, z)
            self.__results.raw_points.append(p)
            self.__results.distance_to_height[d] = z
            if z < self.__results.min_z:
                self.__results.min_z = z

            if z > self.__results.max_z:
                self.__results.max_z = z

            self.__results.geometries.append(QgsGeometry(p))
            self.__results.cross_section_geometries.append(QgsGeometry(QgsPoint(d, z)))

        return not self.__feedback.isCanceled()

    def takeResults(self):
        return self.__results


class MyProfileSource(QgsAbstractProfileSource):
    def __init__(self):
        QgsAbstractProfileSource.__init__(self)

    def createProfileGenerator(self, request):
        return MyProfileGenerator(request)


class TestQgsProfileSourceRegistry(QgisTestCase):

    def test_register_unregister_source(self):
        initial_sources = QgsApplication.profileSourceRegistry().profileSources()

        source = MyProfileSource()
        QgsApplication.profileSourceRegistry().registerProfileSource(source)
        self.assertEqual(
            len(QgsApplication.profileSourceRegistry().profileSources()),
            len(initial_sources) + 1,
        )
        self.assertEqual(
            QgsApplication.profileSourceRegistry().profileSources()[-1], source
        )
        QgsApplication.profileSourceRegistry().unregisterProfileSource(source)
        self.assertEqual(
            QgsApplication.profileSourceRegistry().profileSources(), initial_sources
        )

    def test_generate_profile_from_custom_source(self):
        curve = QgsLineString()
        curve.fromWkt(
            "LINESTRING (2584085.816 1216473.232, 2579969.567 1218027.326, 2576819.377 1220589.481)"
        )
        req = QgsProfileRequest(curve)
        req.setCrs(QgsCoordinateReferenceSystem("EPSG:2056"))

        source = MyProfileSource()
        generator = source.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile(QgsProfileGenerationContext()))
        results = generator.takeResults()
        self.assertEqual(results.type(), "my-web-service-profile")
        self.assertEqual(results.zRange(), QgsDoubleRange(429.3, 1282.7))

        self.assertTrue(len(results.asGeometries()), 5)
        expected_geoms = [
            QgsGeometry(QgsPoint(2584085.816, 1216473.232, 454.8)),
            QgsGeometry(QgsPoint(2582027.691, 1217250.279, 429.3)),
            QgsGeometry(QgsPoint(2579969.567, 1218027.326, 702.5)),
            QgsGeometry(QgsPoint(2578394.472, 1219308.404, 857.9)),
            QgsGeometry(QgsPoint(2576819.377, 1220589.481, 1282.7)),
        ]

        for i, geom in enumerate(results.asGeometries()):
            self.checkGeometriesEqual(geom, expected_geoms[i], 0, 0)

        features = results.asFeatures(
            Qgis.ProfileExportType.DistanceVsElevationTable, QgsFeedback()
        )
        self.assertEqual(len(features), len(results.distance_to_height))
        for feature in features:
            self.assertEqual(feature.geometry.wkbType(), Qgis.WkbType.PointZ)
            self.assertTrue(not feature.geometry.isEmpty())
            d = feature.attributes["distance"]
            self.assertIn(d, results.distance_to_height)
            self.assertEqual(
                feature.attributes["elevation"], results.distance_to_height[d]
            )

    def test_export_3d_from_custom_source(self):
        source = MyProfileSource()
        curve = QgsLineString()
        curve.fromWkt(
            "LINESTRING (2584085.816 1216473.232, 2579969.567 1218027.326, 2576819.377 1220589.481)"
        )
        req = QgsProfileRequest(curve)
        req.setCrs(QgsCoordinateReferenceSystem("EPSG:2056"))

        exporter = QgsProfileExporter([source], req, Qgis.ProfileExportType.Features3D)
        exporter.run(QgsFeedback())
        layers = exporter.toLayers()
        self.assertEqual(len(layers), 1)
        layer = layers[0]
        self.assertEqual(layer.wkbType(), Qgis.WkbType.PointZ)
        self.assertEqual(layer.crs(), QgsCoordinateReferenceSystem("EPSG:2056"))
        self.assertEqual(layer.featureCount(), 5)

        expected_z = [454.8, 429.3, 702.5, 857.9, 1282.7]
        for i, feature in enumerate(layer.getFeatures()):
            z = feature.geometry().constGet().z()
            self.assertEqual(z, expected_z[i])

    def test_export_2d_from_custom_source(self):
        source = MyProfileSource()
        curve = QgsLineString()
        curve.fromWkt(
            "LINESTRING (2584085.816 1216473.232, 2579969.567 1218027.326, 2576819.377 1220589.481)"
        )
        req = QgsProfileRequest(curve)
        req.setCrs(QgsCoordinateReferenceSystem("EPSG:2056"))

        exporter = QgsProfileExporter([source], req, Qgis.ProfileExportType.Profile2D)
        exporter.run(QgsFeedback())
        layers = exporter.toLayers()
        self.assertEqual(len(layers), 1)
        layer = layers[0]
        self.assertEqual(layer.wkbType(), Qgis.WkbType.Point)
        self.assertEqual(layer.featureCount(), 5)

        expected_values = {
            0: 454.8,
            2199.9: 429.3,
            4399.9: 702.5,
            6430.1: 857.9,
            8460.4: 1282.7,
        }
        for i, feature in enumerate(layer.getFeatures()):
            geom = feature.geometry().constGet()
            d = geom.x()
            z = geom.y()
            self.assertEqual(z, expected_values[d])

    def test_export_distance_elevation_from_custom_source(self):
        source = MyProfileSource()
        curve = QgsLineString()
        curve.fromWkt(
            "LINESTRING (2584085.816 1216473.232, 2579969.567 1218027.326, 2576819.377 1220589.481)"
        )
        req = QgsProfileRequest(curve)
        req.setCrs(QgsCoordinateReferenceSystem("EPSG:2056"))

        exporter = QgsProfileExporter(
            [source], req, Qgis.ProfileExportType.DistanceVsElevationTable
        )
        exporter.run(QgsFeedback())
        layers = exporter.toLayers()
        self.assertEqual(len(layers), 1)
        layer = layers[0]
        self.assertEqual(layer.wkbType(), Qgis.WkbType.PointZ)
        self.assertEqual(layer.crs(), QgsCoordinateReferenceSystem("EPSG:2056"))
        self.assertEqual(layer.featureCount(), 5)

        expected_values = {
            0: 454.8,
            2199.9: 429.3,
            4399.9: 702.5,
            6430.1: 857.9,
            8460.4: 1282.7,
        }
        expected_z = list(expected_values.values())
        for i, feature in enumerate(layer.getFeatures()):
            z = feature.geometry().constGet().z()
            self.assertEqual(z, expected_z[i])
            self.assertEqual(z, feature["elevation"])
            self.assertIn(feature["distance"], expected_values)
            self.assertEqual(expected_values[feature["distance"]], feature["elevation"])

    def test_profile_canvas_custom_source(self):
        canvas = QgsElevationProfileCanvas()
        canvas.setProject(QgsProject.instance())
        canvas.setCrs(QgsCoordinateReferenceSystem("EPSG:2056"))
        curve = QgsLineString()
        curve.fromWkt(
            "LINESTRING (2584085.816 1216473.232, 2579969.567 1218027.326, 2576819.377 1220589.481)"
        )
        canvas.setProfileCurve(curve.clone())
        spy = QSignalSpy(canvas.activeJobCountChanged)
        self.assertTrue(spy.isValid())

        source = MyProfileSource()
        QgsApplication.profileSourceRegistry().registerProfileSource(source)
        canvas.refresh()
        spy.wait()

        distance_range = canvas.visibleDistanceRange()
        self.assertTrue(
            distance_range.contains(0),
            f"Distance 0 (min) not included in range ({distance_range})",
        )
        self.assertTrue(
            distance_range.contains(8460.4),
            f"Distance 8460.4 (max) not included in range ({distance_range})",
        )

        elevation_range = canvas.visibleElevationRange()
        self.assertTrue(
            elevation_range.contains(429.3),
            f"Elevation 429.3 (min) not included in range ({elevation_range})",
        )
        self.assertTrue(
            elevation_range.contains(1282.7),
            f"Elevation 1282.7 (max) not included in range ({elevation_range})",
        )
        QgsApplication.profileSourceRegistry().unregisterProfileSource(source)

    def test_layout_item_profile_custom_source(self):
        """
        Test getting a custom profile in a layout item
        """
        source = MyProfileSource()
        QgsApplication.profileSourceRegistry().registerProfileSource(source)

        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()

        profile_item = QgsLayoutItemElevationProfile(layout)
        layout.addLayoutItem(profile_item)
        profile_item.attemptSetSceneRect(QRectF(10, 10, 180, 180))

        curve = QgsLineString()
        curve.fromWkt(
            "LINESTRING (2584085.816 1216473.232, 2579969.567 1218027.326, 2576819.377 1220589.481)"
        )

        profile_item.setProfileCurve(curve)
        profile_item.setCrs(QgsCoordinateReferenceSystem("EPSG:2056"))

        profile_item.plot().setXMinimum(-100)
        profile_item.plot().setXMaximum(curve.length() + 100)
        profile_item.plot().setYMaximum(1300)

        profile_item.plot().xAxis().setGridIntervalMajor(1000)
        profile_item.plot().xAxis().setGridIntervalMinor(500)
        profile_item.plot().xAxis().setGridMajorSymbol(
            QgsLineSymbol.createSimple({"color": "#ffaaff", "width": 2})
        )
        profile_item.plot().xAxis().setGridMinorSymbol(
            QgsLineSymbol.createSimple({"color": "#ffffaa", "width": 2})
        )

        format = QgsTextFormat()
        format.setFont(QgsFontUtils.getStandardTestFont("Bold"))
        format.setSize(20)
        format.setNamedStyle("Bold")
        format.setColor(QColor(0, 0, 0))
        profile_item.plot().xAxis().setTextFormat(format)
        profile_item.plot().xAxis().setLabelInterval(2000)

        profile_item.plot().yAxis().setGridIntervalMajor(1000)
        profile_item.plot().yAxis().setGridIntervalMinor(500)
        profile_item.plot().yAxis().setGridMajorSymbol(
            QgsLineSymbol.createSimple({"color": "#ffffaa", "width": 2})
        )
        profile_item.plot().yAxis().setGridMinorSymbol(
            QgsLineSymbol.createSimple({"color": "#aaffaa", "width": 2})
        )

        profile_item.plot().yAxis().setTextFormat(format)
        profile_item.plot().yAxis().setLabelInterval(500)

        profile_item.plot().setChartBorderSymbol(
            QgsFillSymbol.createSimple(
                {"style": "no", "color": "#aaffaa", "width_border": 2}
            )
        )

        self.assertTrue(self.render_layout_check("custom_profile", layout))
        QgsApplication.profileSourceRegistry().unregisterProfileSource(source)


if __name__ == "__main__":
    unittest.main()
