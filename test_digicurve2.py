from itertools import count

setup = {
    'Linestring': {
        'LINESTRING(0 0, 10 0, 10 10, 0 10, 0 0)': 'LINESTRING(0 0, 10 0, 10 10, 0 10, 0 0)',
        'COMPOUNDCURVE(CIRCULARSTRING(0 0, 10 0, 10 10, 0 10, 0 0))': 'COMPOUNDCURVE(CIRCULARSTRING(0 0, 10 0, 10 10, 0 10, 0 0))',
        'COMPOUNDCURVE((0 0, 10 0), CIRCULARSTRING(10 0, 10 10, 0 10), (0 10, 0 0))': 'COMPOUNDCURVE((0 0, 10 0), CIRCULARSTRING(10 0, 10 10, 0 10), (0 10, 0 0))',
    },
    'CompoundCurve': {
        'LINESTRING(0 0, 10 0, 10 10, 0 10, 0 0)': 'LINESTRING(0 0, 10 0, 10 10, 0 10, 0 0)',
        'COMPOUNDCURVE(CIRCULARSTRING(0 0, 10 0, 10 10, 0 10, 0 0))': 'COMPOUNDCURVE(CIRCULARSTRING(0 0, 10 0, 10 10, 0 10, 0 0))',
        'COMPOUNDCURVE((0 0, 10 0), CIRCULARSTRING(10 0, 10 10, 0 10), (0 10, 0 0))': 'COMPOUNDCURVE((0 0, 10 0), CIRCULARSTRING(10 0, 10 10, 0 10), (0 10, 0 0))',
    },
    'MultiCurve': {
        'MULTICURVE(LINESTRING(0 0, 10 0, 10 10, 0 10, 0 0), LINESTRING(5 15, 10 20, 0 20, 5 15))': 'MULTICURVE(LINESTRING(0 0, 10 0, 10 10, 0 10, 0 0), LINESTRING(5 15, 10 20, 0 20, 5 15))',
        'MULTICURVE(COMPOUNDCURVE(CIRCULARSTRING(0 0, 10 0, 10 10, 0 10, 0 0)), LINESTRING(5 15, 10 20, 0 20, 5 15))': 'MULTICURVE(COMPOUNDCURVE(CIRCULARSTRING(0 0, 10 0, 10 10, 0 10, 0 0)), LINESTRING(5 15, 10 20, 0 20, 5 15))',
        'MULTICURVE(COMPOUNDCURVE((0 0, 10 0), CIRCULARSTRING(10 0, 10 10, 0 10), (0 10, 0 0)), LINESTRING(5 15, 10 20, 0 20, 5 15))': 'MULTICURVE(COMPOUNDCURVE((0 0, 10 0), CIRCULARSTRING(10 0, 10 10, 0 10), (0 10, 0 0)), LINESTRING(5 15, 10 20, 0 20, 5 15))',
    },
    'Polygon': {
        'CURVEPOLYGON(LINESTRING(0 0, 10 0, 10 10, 0 10, 0 0), LINESTRING(3 3, 7 3, 7 7, 3 7, 3 3))': 'CURVEPOLYGON(LINESTRING(0 0, 10 0, 10 10, 0 10, 0 0), LINESTRING(3 3, 7 3, 7 7, 3 7, 3 3))',
        'CURVEPOLYGON(COMPOUNDCURVE(CIRCULARSTRING(0 0, 10 0, 10 10, 0 10, 0 0)), COMPOUNDCURVE(CIRCULARSTRING(3 3, 7 3, 7 7, 3 7, 3 3)))': 'CURVEPOLYGON(COMPOUNDCURVE(CIRCULARSTRING(0 0, 10 0, 10 10, 0 10, 0 0)), COMPOUNDCURVE(CIRCULARSTRING(3 3, 7 3, 7 7, 3 7, 3 3)))',
        'CURVEPOLYGON(COMPOUNDCURVE((0 0, 10 0), CIRCULARSTRING(10 0, 10 10, 0 10), (0 10, 0 0)), COMPOUNDCURVE((3 3, 7 3), CIRCULARSTRING(7 3, 7 7, 3 7), (3 7, 3 3)))': 'CURVEPOLYGON(COMPOUNDCURVE((0 0, 10 0), CIRCULARSTRING(10 0, 10 10, 0 10), (0 10, 0 0)), COMPOUNDCURVE((3 3, 7 3), CIRCULARSTRING(7 3, 7 7, 3 7), (3 7, 3 3)))',
    },
    'CurvePolygon': {
        'CURVEPOLYGON(LINESTRING(0 0, 10 0, 10 10, 0 10, 0 0), LINESTRING(3 3, 7 3, 7 7, 3 7, 3 3))': 'CURVEPOLYGON(LINESTRING(0 0, 10 0, 10 10, 0 10, 0 0), LINESTRING(3 3, 7 3, 7 7, 3 7, 3 3))',
        'CURVEPOLYGON(COMPOUNDCURVE(CIRCULARSTRING(0 0, 10 0, 10 10, 0 10, 0 0)), COMPOUNDCURVE(CIRCULARSTRING(3 3, 7 3, 7 7, 3 7, 3 3)))': 'CURVEPOLYGON(COMPOUNDCURVE(CIRCULARSTRING(0 0, 10 0, 10 10, 0 10, 0 0)), COMPOUNDCURVE(CIRCULARSTRING(3 3, 7 3, 7 7, 3 7, 3 3)))',
        'CURVEPOLYGON(COMPOUNDCURVE((0 0, 10 0), CIRCULARSTRING(10 0, 10 10, 0 10), (0 10, 0 0)), COMPOUNDCURVE((3 3, 7 3), CIRCULARSTRING(7 3, 7 7, 3 7), (3 7, 3 3)))': 'CURVEPOLYGON(COMPOUNDCURVE((0 0, 10 0), CIRCULARSTRING(10 0, 10 10, 0 10), (0 10, 0 0)), COMPOUNDCURVE((3 3, 7 3), CIRCULARSTRING(7 3, 7 7, 3 7), (3 7, 3 3)))',
    },
    'MultiSurface': {
        'MULTISURFACE(CURVEPOLYGON(LINESTRING(0 0, 10 0, 10 10, 0 10, 0 0), LINESTRING(3 3, 7 3, 7 7, 3 7, 3 3)), CURVEPOLYGON(LINESTRING(5 15, 10 20, 0 20, 5 15)))': 'MULTISURFACE(CURVEPOLYGON(LINESTRING(0 0, 10 0, 10 10, 0 10, 0 0), LINESTRING(3 3, 7 3, 7 7, 3 7, 3 3)), CURVEPOLYGON(LINESTRING(5 15, 10 20, 0 20, 5 15)))',
        'MULTISURFACE(CURVEPOLYGON(COMPOUNDCURVE(CIRCULARSTRING(0 0, 10 0, 10 10, 0 10, 0 0)), COMPOUNDCURVE(CIRCULARSTRING(3 3, 7 3, 7 7, 3 7, 3 3))), CURVEPOLYGON(LINESTRING(5 15, 10 20, 0 20, 5 15)))': 'MULTISURFACE(CURVEPOLYGON(COMPOUNDCURVE(CIRCULARSTRING(0 0, 10 0, 10 10, 0 10, 0 0)), COMPOUNDCURVE(CIRCULARSTRING(3 3, 7 3, 7 7, 3 7, 3 3))), CURVEPOLYGON(LINESTRING(5 15, 10 20, 0 20, 5 15)))',
        'MULTISURFACE(CURVEPOLYGON(COMPOUNDCURVE((0 0, 10 0), CIRCULARSTRING(10 0, 10 10, 0 10), (0 10, 0 0)), COMPOUNDCURVE((3 3, 7 3), CIRCULARSTRING(7 3, 7 7, 3 7), (3 7, 3 3))), CURVEPOLYGON(LINESTRING(5 15, 10 20, 0 20, 5 15)))': 'MULTISURFACE(CURVEPOLYGON(COMPOUNDCURVE((0 0, 10 0), CIRCULARSTRING(10 0, 10 10, 0 10), (0 10, 0 0)), COMPOUNDCURVE((3 3, 7 3), CIRCULARSTRING(7 3, 7 7, 3 7), (3 7, 3 3))), CURVEPOLYGON(LINESTRING(5 15, 10 20, 0 20, 5 15)))',
    },
}

for geomtype, wkts in setup.items():
    layer = QgsVectorLayer(f"{geomtype}?crs=epsg:4326", geomtype, "memory")
    i = 0
    for wkt_before, wkt_expected in wkts.items():
        feature = QgsFeature()
        geom = QgsGeometry.fromWkt(wkt_before)
        geom.translate(i * 15, 0)
        feature.setGeometry(geom)
        layer.dataProvider().addFeature(feature)
        i += 1
    QgsProject.instance().addMapLayer(layer)


for geomtype, wkts in setup.items():
    layer = QgsVectorLayer(f"{geomtype}?crs=epsg:4326", geomtype, "memory")
    i = 0
    for wkt_before, wkt_expected in wkts.items():
        feature = QgsFeature()

        geom = QgsGeometry.fromWkt(wkt_before)

        # Ensure convert has the expected result
        geom.convertVertex(geom.closestVertex(QgsPointXY(10, 10))[1])
        assert geom.asWkt() == QgsGeometry.fromWkt(wkt_expected).asWkt(), 'NOT AS EXPECTED'

        # Ensure clicking again revers to previous
        geom.convertVertex(geom.closestVertex(QgsPointXY(10, 10))[1])
        assert geom.asWkt() == QgsGeometry.fromWkt(wkt_before).asWkt(), 'NOT IDEMPOTENT'

    QgsProject.instance().addMapLayer(layer)
