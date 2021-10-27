# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsDistanceArea.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Jürgen E. Fischer'
__date__ = '19/01/2014'
__copyright__ = 'Copyright 2014, The QGIS Project'

import qgis  # NOQA
import math

from qgis.core import (QgsGeometry,
                       QgsPointXY,
                       QgsDistanceArea,
                       QgsCoordinateReferenceSystem,
                       QgsUnitTypes,
                       QgsProject)

from qgis.testing import start_app, unittest
from qgis.PyQt.QtCore import QLocale
from pprint import pprint

# Convenience instances in case you may need them
# not used in this test

start_app()


class TestQgsDistanceArea(unittest.TestCase):

    def testCrs(self):
        # test setting/getting the source CRS
        da = QgsDistanceArea()

        # try setting using a CRS object
        crs = QgsCoordinateReferenceSystem('EPSG:3111')
        da.setSourceCrs(crs, QgsProject.instance().transformContext())
        self.assertEqual(da.sourceCrs().srsid(), crs.srsid())

        self.assertFalse(da.ellipsoidCrs().isValid())
        da.setEllipsoid("GRS80")
        # depends on proj version
        self.assertIn(da.ellipsoidCrs().toProj(), (
            '+proj=longlat +ellps=GRS80 +no_defs', '+proj=longlat +a=6378137 +rf=298.25722210100002 +no_defs'))
        da.setEllipsoid("WGS84")
        self.assertIn(da.ellipsoidCrs().toProj(), (
            '+proj=longlat +ellps=WGS84 +no_defs', '+proj=longlat +a=6378137 +rf=298.25722356300003 +no_defs'))

    def testMeasureLine(self):
        #   +-+
        #   | |
        # +-+ +
        linestring = QgsGeometry.fromPolylineXY(
            [QgsPointXY(0, 0), QgsPointXY(1, 0), QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 0), ]
        )
        da = QgsDistanceArea()
        length = da.measureLength(linestring)
        myMessage = ('Expected:\n%f\nGot:\n%f\n' %
                     (4, length))
        assert length == 4, myMessage

    def testBearing(self):
        """
        Test bearing calculation
        """
        da = QgsDistanceArea()
        self.assertAlmostEqual(da.bearing(QgsPointXY(145.047, -37.578), QgsPointXY(168.38, -16.95)), 0.84685, 5)
        self.assertAlmostEqual(da.bearing(QgsPointXY(-19.57, 65.12), QgsPointXY(-2.63, 54.97)), 2.11060792, 5)

        da.setSourceCrs(QgsCoordinateReferenceSystem('EPSG:3857'), QgsProject.instance().transformContext())
        da.setEllipsoid(da.sourceCrs().ellipsoidAcronym())
        self.assertTrue(da.willUseEllipsoid())
        self.assertAlmostEqual(da.bearing(QgsPointXY(16198544, -4534850), QgsPointXY(18736872, -1877769)), 0.8723168079, 5)
        self.assertAlmostEqual(da.bearing(QgsPointXY(-2074453, 9559553), QgsPointXY(-55665, 6828252)), 2.35691008, 5)

    def testMeasureLineProjected(self):
        #   +-+
        #   | |
        # +-+ +
        # test setting/getting the source CRS
        da_3068 = QgsDistanceArea()
        da_wsg84 = QgsDistanceArea()

        da_3068.setSourceCrs(QgsCoordinateReferenceSystem.fromOgcWmsCrs('EPSG:3068'),
                             QgsProject.instance().transformContext())
        if (da_3068.sourceCrs().isGeographic()):
            da_3068.setEllipsoid(da_3068.sourceCrs().ellipsoidAcronym())
        print(("setting [{}] srid [{}] description [{}]".format(u'Soldner Berlin', da_3068.sourceCrs().authid(),
                                                                da_3068.sourceCrs().description())))
        self.assertEqual(da_3068.sourceCrs().authid(), 'EPSG:3068')
        da_wsg84.setSourceCrs(QgsCoordinateReferenceSystem.fromOgcWmsCrs('EPSG:4326'),
                              QgsProject.instance().transformContext())
        if (da_wsg84.sourceCrs().isGeographic()):
            da_wsg84.setEllipsoid(da_wsg84.sourceCrs().ellipsoidAcronym())
        self.assertEqual(da_wsg84.sourceCrs().authid(), 'EPSG:4326')
        print(("setting [{}] srid [{}] description [{}] isGeographic[{}]".format(u'Wsg84',
                                                                                 da_wsg84.sourceCrs().authid(),
                                                                                 da_wsg84.sourceCrs().description(),
                                                                                 da_wsg84.sourceCrs().isGeographic())))
        # print(("-- projectionAcronym[{}] ellipsoidAcronym[{}] toWkt[{}] mapUnits[{}] toProj4[{}]".format(da_wsg84.sourceCrs().projectionAcronym(),da_wsg84.sourceCrs().ellipsoidAcronym(), da_wsg84.sourceCrs().toWkt(),da_wsg84.sourceCrs().mapUnits(),da_wsg84.sourceCrs().toProj())))
        print(("Testing Position change for[{}] years[{}]".format(u'Ampelanlage - Potsdamer Platz, Verkehrsinsel',
                                                                  u'1924 and 1998')))

        # 1924-10-24 SRID=3068;POINT(23099.49 20296.69)
        # 1924-10-24 SRID=4326;POINT(13.37650707988041 52.50952361017194)
        # 1998-10-02 SRID=3068;POINT(23082.30 20267.80)
        # 1998-10-02 SRID=4326;POINT(13.37625537334001 52.50926345498337)
        # values returned by SpatiaLite
        point_soldner_1924 = QgsPointXY(23099.49, 20296.69)
        point_soldner_1998 = QgsPointXY(23082.30, 20267.80)
        distance_soldner_meters = 33.617379
        azimuth_soldner_1924 = 3.678339
        # ST_Transform(point_soldner_1924,point_soldner_1998,4326)
        point_wsg84_1924 = QgsPointXY(13.37650707988041, 52.50952361017194)
        point_wsg84_1998 = QgsPointXY(13.37625537334001, 52.50926345498337)
        # ST_Distance(point_wsg84_1924,point_wsg84_1998,1)
        distance_wsg84_meters = 33.617302
        # ST_Distance(point_wsg84_1924,point_wsg84_1998)
        # distance_wsg84_mapunits=0.000362
        distance_wsg84_mapunits_format = QgsDistanceArea.formatDistance(0.000362, 7, QgsUnitTypes.DistanceDegrees, True)
        # ST_Azimuth(point_wsg84_1924,point_wsg84_1998)
        azimuth_wsg84_1924 = 3.674878
        # ST_Azimuth(point_wsg84_1998,point_wsg84_1998)
        azimuth_wsg84_1998 = 0.533282
        # ST_Project(point_wsg84_1924,33.617302,3.674878)
        # SRID=4326;POINT(13.37625537318728 52.50926345503591)
        point_soldner_1998_project = QgsPointXY(13.37625537318728, 52.50926345503591)
        # ST_Project(point_wsg84_1998,33.617302,0.533282)
        # SRID=4326;POINT(13.37650708009255 52.50952361009799)
        point_soldner_1924_project = QgsPointXY(13.37650708009255, 52.50952361009799)

        distance_qpoint = point_soldner_1924.distance(point_soldner_1998)
        azimuth_qpoint = point_soldner_1924.azimuth(point_soldner_1998)
        point_soldner_1998_result = point_soldner_1924.project(distance_qpoint, azimuth_qpoint)

        point_soldner_1924_result = QgsPointXY(0, 0)
        point_soldner_1998_result = QgsPointXY(0, 0)
        # Test meter based projected point from point_1924 to point_1998
        length_1998_mapunits, point_soldner_1998_result = da_3068.measureLineProjected(point_soldner_1924,
                                                                                       distance_soldner_meters,
                                                                                       azimuth_qpoint)
        self.assertEqual(point_soldner_1998_result.toString(6), point_soldner_1998.toString(6))
        # Test degree based projected point from point_1924 1 meter due East
        point_wsg84_meter_result = QgsPointXY(0, 0)
        point_wsg84_1927_meter = QgsPointXY(13.37652180838435, 52.50952361017102)
        length_meter_mapunits, point_wsg84_meter_result = da_wsg84.measureLineProjected(point_wsg84_1924, 1.0,
                                                                                        (math.pi / 2))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 7, QgsUnitTypes.DistanceDegrees, True),
                         '0.0000147 deg')
        self.assertEqual(point_wsg84_meter_result.toString(7), point_wsg84_1927_meter.toString(7))

        point_wsg84_1998_result = QgsPointXY(0, 0)
        length_1928_mapunits, point_wsg84_1998_result = da_wsg84.measureLineProjected(point_wsg84_1924,
                                                                                      distance_wsg84_meters,
                                                                                      azimuth_wsg84_1924)
        self.assertEqual(QgsDistanceArea.formatDistance(length_1928_mapunits, 7, QgsUnitTypes.DistanceDegrees, True),
                         distance_wsg84_mapunits_format)
        self.assertEqual(point_wsg84_1998_result.toString(7), point_wsg84_1998.toString(7))

    def testMeasureLineProjectedWorldPoints(self):
        #   +-+
        #   | |
        # +-+ +
        # checking returned length_mapunits/projected_points of different world points with results from SpatiaLite ST_Project
        da_3068 = QgsDistanceArea()
        da_3068.setSourceCrs(QgsCoordinateReferenceSystem.fromOgcWmsCrs('EPSG:3068'),
                             QgsProject.instance().transformContext())
        if (da_3068.sourceCrs().isGeographic()):
            da_3068.setEllipsoid(da_3068.sourceCrs().ellipsoidAcronym())
        self.assertEqual(da_3068.sourceCrs().authid(), 'EPSG:3068')
        print((
            "setting [{}] srid [{}] description [{}] isGeographic[{}] lengthUnits[{}] projectionAcronym[{}] ellipsoidAcronym[{}]".format(
                u'EPSG:3068', da_3068.sourceCrs().authid(), da_3068.sourceCrs().description(),
                da_3068.sourceCrs().isGeographic(), QgsUnitTypes.toString(da_3068.lengthUnits()),
                da_3068.sourceCrs().projectionAcronym(), da_3068.sourceCrs().ellipsoidAcronym())))
        da_wsg84 = QgsDistanceArea()
        da_wsg84.setSourceCrs(QgsCoordinateReferenceSystem.fromOgcWmsCrs('EPSG:4326'),
                              QgsProject.instance().transformContext())
        if (da_wsg84.sourceCrs().isGeographic()):
            da_wsg84.setEllipsoid(da_wsg84.sourceCrs().ellipsoidAcronym())
        self.assertEqual(da_wsg84.sourceCrs().authid(), 'EPSG:4326')
        print((
            "setting [{}] srid [{}] description [{}] isGeographic[{}] lengthUnits[{}] projectionAcronym[{}] ellipsoidAcronym[{}] ellipsoid[{}]".format(
                u'EPSG:4326', da_wsg84.sourceCrs().authid(), da_wsg84.sourceCrs().description(),
                da_wsg84.sourceCrs().isGeographic(), QgsUnitTypes.toString(da_wsg84.lengthUnits()),
                da_wsg84.sourceCrs().projectionAcronym(), da_wsg84.sourceCrs().ellipsoidAcronym(),
                da_wsg84.ellipsoid())))
        da_4314 = QgsDistanceArea()
        da_4314.setSourceCrs(QgsCoordinateReferenceSystem.fromOgcWmsCrs('EPSG:4314'),
                             QgsProject.instance().transformContext())
        if (da_4314.sourceCrs().isGeographic()):
            da_4314.setEllipsoid(da_4314.sourceCrs().ellipsoidAcronym())
        self.assertEqual(da_4314.sourceCrs().authid(), 'EPSG:4314')
        print((
            "setting [{}] srid [{}] description [{}] isGeographic[{}] lengthUnits[{}] projectionAcronym[{}] ellipsoidAcronym[{}]".format(
                u'EPSG:4314', da_4314.sourceCrs().authid(), da_4314.sourceCrs().description(),
                da_4314.sourceCrs().isGeographic(), QgsUnitTypes.toString(da_4314.lengthUnits()),
                da_4314.sourceCrs().projectionAcronym(), da_4314.sourceCrs().ellipsoidAcronym())))
        da_4805 = QgsDistanceArea()
        da_4805.setSourceCrs(QgsCoordinateReferenceSystem.fromOgcWmsCrs('EPSG:4805'),
                             QgsProject.instance().transformContext())
        if (da_4805.sourceCrs().isGeographic()):
            da_4805.setEllipsoid(da_4805.sourceCrs().ellipsoidAcronym())
        self.assertEqual(da_4805.sourceCrs().authid(), 'EPSG:4805')
        print((
            "setting [{}] srid [{}] description [{}] isGeographic[{}] lengthUnits[{}] projectionAcronym[{}] ellipsoidAcronym[{}]".format(
                u'EPSG:4805', da_4805.sourceCrs().authid(), da_4805.sourceCrs().description(),
                da_4805.sourceCrs().isGeographic(), QgsUnitTypes.toString(da_4805.lengthUnits()),
                da_4805.sourceCrs().projectionAcronym(), da_4805.sourceCrs().ellipsoidAcronym())))
        # EPSG:5665 unknown, why?
        da_5665 = QgsDistanceArea()
        da_5665.setSourceCrs(QgsCoordinateReferenceSystem.fromOgcWmsCrs('EPSG:5665'),
                             QgsProject.instance().transformContext())
        if (da_5665.sourceCrs().isGeographic()):
            da_5665.setEllipsoid(da_5665.sourceCrs().ellipsoidAcronym())
        print((
            "setting [{}] srid [{}] description [{}] isGeographic[{}] lengthUnits[{}] projectionAcronym[{}] ellipsoidAcronym[{}]".format(
                u'EPSG:5665', da_5665.sourceCrs().authid(), da_5665.sourceCrs().description(),
                da_5665.sourceCrs().isGeographic(), QgsUnitTypes.toString(da_5665.lengthUnits()),
                da_5665.sourceCrs().projectionAcronym(), da_5665.sourceCrs().ellipsoidAcronym())))
        # self.assertEqual(da_5665.sourceCrs().authid(), 'EPSG:5665')
        da_25833 = QgsDistanceArea()
        da_25833.setSourceCrs(QgsCoordinateReferenceSystem.fromOgcWmsCrs('EPSG:25833'),
                              QgsProject.instance().transformContext())
        if (da_25833.sourceCrs().isGeographic()):
            da_25833.setEllipsoid(da_25833.sourceCrs().ellipsoidAcronym())
        print((
            "setting [{}] srid [{}] description [{}] isGeographic[{}] lengthUnits[{}] projectionAcronym[{}] ellipsoidAcronym[{}]".format(
                u'EPSG:25833', da_25833.sourceCrs().authid(), da_25833.sourceCrs().description(),
                da_25833.sourceCrs().isGeographic(), QgsUnitTypes.toString(da_25833.lengthUnits()),
                da_25833.sourceCrs().projectionAcronym(), da_25833.sourceCrs().ellipsoidAcronym())))
        self.assertEqual(da_25833.sourceCrs().authid(), 'EPSG:25833')

        # Berlin - Brandenburg Gate - Quadriga
        point_berlin_3068 = QgsPointXY(23183.38449999984, 21047.3225000017)
        point_berlin_3068_project = point_berlin_3068.project(1, (math.pi / 2))
        point_meter_result = QgsPointXY(0, 0)
        length_meter_mapunits, point_meter_result = da_3068.measureLineProjected(point_berlin_3068, 1.0, (math.pi / 2))
        pprint(point_meter_result)
        print('-I-> Berlin 3068 length_meter_mapunits[{}] point_meter_result[{}]'.format(
            QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_3068.lengthUnits(), True),
            point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 1, da_3068.lengthUnits(), True), '1.0 m')
        self.assertEqual(point_meter_result.toString(7), point_berlin_3068_project.toString(7))
        point_berlin_wsg84 = QgsPointXY(13.37770458660236, 52.51627178856762)
        point_berlin_wsg84_project = QgsPointXY(13.37771931736259, 52.51627178856669)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_berlin_wsg84, 1.0,
                                                                                  (math.pi / 2))
        print('-I-> Berlin Wsg84 length_meter_mapunits[{}] point_meter_result[{}] ellipsoid[{}]'.format(
            QgsDistanceArea.formatDistance(length_meter_mapunits, 20, da_wsg84.lengthUnits(), True),
            point_meter_result.asWkt(), da_wsg84.ellipsoid()))
        # for unknown reasons, this is returning '0.00001473026 m' instead of '0.00001473026 deg' when using da_wsg84.lengthUnits()
        # self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits,11,da_wsg84.lengthUnits(),True), '0.00001473026 deg')
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 11, QgsUnitTypes.DistanceDegrees, True),
                         '0.00001473076 deg')
        self.assertEqual(point_meter_result.toString(7), point_berlin_wsg84_project.toString(7))
        point_berlin_4314 = QgsPointXY(13.37944343021465, 52.51767872437083)
        point_berlin_4314_project = QgsPointXY(13.37945816324759, 52.5176787243699)
        length_meter_mapunits, point_meter_result = da_4314.measureLineProjected(point_berlin_4314, 1.0, (math.pi / 2))
        print('-I-> Berlin 4314 length_meter_mapunits[{}] point_meter_result[{}]'.format(
            QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_4314.lengthUnits(), True),
            point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 9, QgsUnitTypes.DistanceDegrees, True),
                         '0.000014733 deg')
        self.assertEqual(point_meter_result.toString(7), point_berlin_4314_project.toString(7))
        point_berlin_4805 = QgsPointXY(31.04960570069176, 52.5174657497405)
        point_berlin_4805_project = QgsPointXY(31.04962043365347, 52.51746574973957)
        length_meter_mapunits, point_meter_result = da_4805.measureLineProjected(point_berlin_4805, 1.0, (math.pi / 2))
        print('-I-> Berlin 4805 length_meter_mapunits[{}] point_meter_result[{}]'.format(
            QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_4805.lengthUnits(), True),
            point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 9, QgsUnitTypes.DistanceDegrees, True),
                         '0.000014733 deg')
        self.assertEqual(point_meter_result.toString(7), point_berlin_4805_project.toString(7))
        point_berlin_25833 = QgsPointXY(389918.0748318382, 5819698.772194743)
        point_berlin_25833_project = point_berlin_25833.project(1, (math.pi / 2))
        length_meter_mapunits, point_meter_result = da_25833.measureLineProjected(point_berlin_25833, 1.0,
                                                                                  (math.pi / 2))
        print('-I-> Berlin 25833 length_meter_mapunits[{}] point_meter_result[{}]'.format(
            QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_25833.lengthUnits(), True),
            point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_25833.lengthUnits(), True),
                         '1.0000000 m')
        self.assertEqual(point_meter_result.toString(7), point_berlin_25833_project.toString(7))
        if da_5665.sourceCrs().authid() != "":
            point_berlin_5665 = QgsPointXY(3389996.871728864, 5822169.719727578)
            point_berlin_5665_project = point_berlin_5665.project(1, (math.pi / 2))
            length_meter_mapunits, point_meter_result = da_5665.measureLineProjected(point_berlin_5665, 1.0,
                                                                                     (math.pi / 2))
            print('-I-> Berlin 5665 length_meter_mapunits[{}] point_meter_result[{}]'.format(
                QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_5665.lengthUnits(), True),
                point_meter_result.asWkt()))
            self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 1, da_5665.lengthUnits(), True),
                             '1.0 m')
            self.assertEqual(point_meter_result.toString(7), point_berlin_5665_project.toString(7))
        print('\n12 points ''above over'' and on the Equator')
        point_wsg84 = QgsPointXY(25.7844, 71.1725)
        point_wsg84_project = QgsPointXY(25.78442775215388, 71.17249999999795)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Nordkap, Norway - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(
            QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True),
            point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 7, QgsUnitTypes.DistanceDegrees, True),
                         '0.0000278 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(24.95995, 60.16841)
        point_wsg84_project = QgsPointXY(24.95996801277454, 60.16840999999877)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Helsinki, Finnland - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(
            QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True),
            point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True),
                         '0.00001801 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(12.599278, 55.692861)
        point_wsg84_project = QgsPointXY(12.59929390161872, 55.69286099999897)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Copenhagen, Denmark - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(
            QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True),
            point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True),
                         '0.00001590 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))

        point_wsg84 = QgsPointXY(-0.001389, 51.477778)
        point_wsg84_project = QgsPointXY(-0.001374606184398, 51.4777779999991)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print(
            '-I-> Royal Greenwich Observatory, United Kingdom - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(
                QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True),
                point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True),
                         '0.00001439 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(7.58769, 47.55814)
        point_wsg84_project = QgsPointXY(7.587703287209086, 47.55813999999922)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Basel, Switzerland - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(
            QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True),
            point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True),
                         '0.00001329 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(11.255278, 43.775278)
        point_wsg84_project = QgsPointXY(11.25529042107924, 43.77527799999933)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Florenz, Italy - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(
            QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True),
            point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True),
                         '0.00001242 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(14.514722, 35.899722)
        point_wsg84_project = QgsPointXY(14.51473307693308, 35.89972199999949)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Valletta, Malta - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(
            QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True),
            point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True),
                         '0.00001108 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(-79.933333, 32.783333)
        point_wsg84_project = QgsPointXY(-79.93332232547254, 32.78333299999955)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Charlston, South Carolina - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(
            QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True),
            point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True),
                         '0.00001067 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(-17.6666666, 27.733333)
        point_wsg84_project = QgsPointXY(-17.66665645831515, 27.73333299999962)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Ferro, Spain - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(
            QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True),
            point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True),
                         '0.00001014 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(-99.133333, 19.433333)
        point_wsg84_project = QgsPointXY(-99.1333234776827, 19.43333299999975)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Mexico City, Mexico - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(
            QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True),
            point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True),
                         '0.00000952 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(-79.894444, 9.341667)
        point_wsg84_project = QgsPointXY(-79.89443489691369, 9.341666999999882)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Colón, Panama - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(
            QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True),
            point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True),
                         '0.00000910 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(-74.075833, 4.598056)
        point_wsg84_project = QgsPointXY(-74.07582398803629, 4.598055999999943)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Bogotá, Colombia - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(
            QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True),
            point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True),
                         '0.00000901 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(0, 0)
        point_wsg84_project = QgsPointXY(0.000008983152841, 0)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Equator, Atlantic Ocean - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(
            QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True),
            point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True),
                         '0.00000898 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        print('\n12 points ''down under'' and 1 point that should be considered invalid')
        point_wsg84 = QgsPointXY(-78.509722, -0.218611)
        point_wsg84_project = QgsPointXY(-78.50971301678221, -0.218610999999997)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Quito, Ecuador - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(
            QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True),
            point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True),
                         '0.00000898 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(106.816667, -6.2)
        point_wsg84_project = QgsPointXY(106.8166760356519, -6.199999999999922)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Jakarta, Indonesia - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(
            QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True),
            point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True),
                         '0.00000904 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(-77.018611, -12.035)
        point_wsg84_project = QgsPointXY(-77.01860181630058, -12.03499999999985)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Lima, Peru - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(
            QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True),
            point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True),
                         '0.00000918 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(25.466667, -10.716667)
        point_wsg84_project = QgsPointXY(25.46667614155322, -10.71666699999986)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Kolwezi, Congo - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(
            QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True),
            point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True),
                         '0.00000914 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(-70.333333, -18.483333)
        point_wsg84_project = QgsPointXY(-70.3333235314429, -18.48333299999976)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Arica, Chile - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(
            QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True),
            point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True),
                         '0.00000947 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(-70.666667, -33.45)
        point_wsg84_project = QgsPointXY(-70.66665624452817, -33.44999999999953)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Santiago, Chile - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(
            QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True),
            point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True),
                         '0.00001076 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(144.9604, -37.8191)
        point_wsg84_project = QgsPointXY(144.96041135746983741, -37.81909999999945171)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Melbourne, Australia - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(
            QgsDistanceArea.formatDistance(length_meter_mapunits, 8, da_wsg84.lengthUnits(), True),
            point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True),
                         '0.00001136 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(147.29, -42.88)
        point_wsg84_project = QgsPointXY(147.2900122399815, -42.87999999999934)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Hobart City,Tasmania, Australia - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(
            QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True),
            point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True),
                         '0.00001224 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(168.101667, -46.899722)
        point_wsg84_project = QgsPointXY(168.101680123673, -46.89972199999923)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print(
            '-I-> Ryan''s Creek Aerodrome, New Zealand - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(
                QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True),
                point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True),
                         '0.00001312 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(-69.216667, -51.633333)
        point_wsg84_project = QgsPointXY(-69.21665255700216, -51.6333329999991)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Río Gallegos, Argentina - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(
            QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True),
            point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True),
                         '0.00001444 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(-68.3, -54.8)
        point_wsg84_project = QgsPointXY(-68.29998445081456, -54.79999999999899)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print(
            '-I-> Ushuaia, Tierra del Fuego, Argentina - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(
                QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True),
                point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True),
                         '0.00001555 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(-63.494444, -64.825278)
        point_wsg84_project = QgsPointXY(-63.49442294002932, -64.82527799999851)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Port Lockroy, Antarctica - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(
            QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True),
            point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True),
                         '0.00002106 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(-180, -84.863272250)
        point_wsg84_project = QgsPointXY(-179.9999000000025, -84.8632722499922)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Someware, Antarctica - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(
            QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True),
            point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True),
                         '0.00010000 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(-180, -85.0511300)
        point_wsg84_project = QgsPointXY(-179.9998962142197, -85.05112999999191)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print(
            '-W-> Mercator''s Last Stop, Antarctica - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(
                QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True),
                point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True),
                         '0.00010379 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))

    def testMeasureMultiLine(self):
        #   +-+ +-+-+
        #   | | |   |
        # +-+ + +   +-+
        linestring = QgsGeometry.fromMultiPolylineXY(
            [
                [QgsPointXY(0, 0), QgsPointXY(1, 0), QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 0), ],
                [QgsPointXY(3, 0), QgsPointXY(3, 1), QgsPointXY(5, 1), QgsPointXY(5, 0), QgsPointXY(6, 0), ]
            ]
        )
        da = QgsDistanceArea()
        length = da.measureLength(linestring)
        myMessage = ('Expected:\n%f\nGot:\n%f\n' %
                     (9, length))
        assert length == 9, myMessage

    def testMeasurePolygon(self):
        # +-+-+
        # |   |
        # + +-+
        # | |
        # +-+
        polygon = QgsGeometry.fromPolygonXY(
            [[
                QgsPointXY(0, 0), QgsPointXY(1, 0), QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 2),
                QgsPointXY(0, 2), QgsPointXY(0, 0),
            ]]
        )

        da = QgsDistanceArea()
        area = da.measureArea(polygon)
        assert area == 3, 'Expected:\n%f\nGot:\n%f\n' % (3, area)

        perimeter = da.measurePerimeter(polygon)
        assert perimeter == 8, 'Expected:\n%f\nGot:\n%f\n' % (8, perimeter)

    def testMeasurePolygonWithHole(self):
        # +-+-+-+
        # |     |
        # + +-+ +
        # | | | |
        # + +-+ +
        # |     |
        # +-+-+-+
        polygon = QgsGeometry.fromPolygonXY(
            [
                [QgsPointXY(0, 0), QgsPointXY(3, 0), QgsPointXY(3, 3), QgsPointXY(0, 3), QgsPointXY(0, 0)],
                [QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 2), QgsPointXY(1, 2), QgsPointXY(1, 1)],
            ]
        )
        da = QgsDistanceArea()
        area = da.measureArea(polygon)
        assert area == 8, "Expected:\n%f\nGot:\n%f\n" % (8, area)

        # MH150729: Changed behavior to consider inner rings for perimeter calculation. Therefore, expected result is 16.
        perimeter = da.measurePerimeter(polygon)
        assert perimeter == 16, "Expected:\n%f\nGot:\n%f\n" % (16, perimeter)

    def testMeasureMultiPolygon(self):
        # +-+-+ +-+-+
        # |   | |   |
        # + +-+ +-+ +
        # | |     | |
        # +-+     +-+
        polygon = QgsGeometry.fromMultiPolygonXY(
            [
                [[QgsPointXY(0, 0), QgsPointXY(1, 0), QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 2),
                  QgsPointXY(0, 2), QgsPointXY(0, 0), ]],
                [[QgsPointXY(4, 0), QgsPointXY(5, 0), QgsPointXY(5, 2), QgsPointXY(3, 2), QgsPointXY(3, 1),
                  QgsPointXY(4, 1), QgsPointXY(4, 0), ]]
            ]
        )

        da = QgsDistanceArea()
        area = da.measureArea(polygon)
        assert area == 6, 'Expected:\n%f\nGot:\n%f\n' % (6, area)

        perimeter = da.measurePerimeter(polygon)
        assert perimeter == 16, "Expected:\n%f\nGot:\n%f\n" % (16, perimeter)

    def testWillUseEllipsoid(self):
        """test QgsDistanceArea::willUseEllipsoid """

        da = QgsDistanceArea()
        da.setEllipsoid("NONE")
        self.assertFalse(da.willUseEllipsoid())

        da.setEllipsoid("WGS84")
        self.assertTrue(da.willUseEllipsoid())

    def testLengthMeasureAndUnits(self):
        """Test a variety of length measurements in different CRS and ellipsoid modes, to check that the
           calculated lengths and units are always consistent
        """

        da = QgsDistanceArea()
        da.setSourceCrs(QgsCoordinateReferenceSystem.fromSrsId(3452), QgsProject.instance().transformContext())
        da.setEllipsoid("NONE")

        # We check both the measured length AND the units, in case the logic regarding
        # ellipsoids and units changes in future
        distance = da.measureLine(QgsPointXY(1, 1), QgsPointXY(2, 3))
        units = da.lengthUnits()

        print(("measured {} in {}".format(distance, QgsUnitTypes.toString(units))))
        assert ((abs(distance - 2.23606797) < 0.00000001 and units == QgsUnitTypes.DistanceDegrees)
                or (abs(distance - 248.52) < 0.01 and units == QgsUnitTypes.DistanceMeters))

        da.setEllipsoid("WGS84")
        distance = da.measureLine(QgsPointXY(1, 1), QgsPointXY(2, 3))
        units = da.lengthUnits()

        print(("measured {} in {}".format(distance, QgsUnitTypes.toString(units))))
        # should always be in Meters
        self.assertAlmostEqual(distance, 247555.57, delta=0.01)
        self.assertEqual(units, QgsUnitTypes.DistanceMeters)

        # test converting the resultant length
        distance = da.convertLengthMeasurement(distance, QgsUnitTypes.DistanceNauticalMiles)
        self.assertAlmostEqual(distance, 133.669, delta=0.01)

        # now try with a source CRS which is in feet
        da.setSourceCrs(QgsCoordinateReferenceSystem.fromSrsId(27469), QgsProject.instance().transformContext())
        da.setEllipsoid("NONE")
        # measurement should be in feet
        distance = da.measureLine(QgsPointXY(1, 1), QgsPointXY(2, 3))
        units = da.lengthUnits()
        print(("measured {} in {}".format(distance, QgsUnitTypes.toString(units))))
        self.assertAlmostEqual(distance, 2.23606797, delta=0.000001)
        self.assertEqual(units, QgsUnitTypes.DistanceFeet)

        # test converting the resultant length
        distance = da.convertLengthMeasurement(distance, QgsUnitTypes.DistanceMeters)
        self.assertAlmostEqual(distance, 0.6815, delta=0.001)

        da.setEllipsoid("WGS84")
        # now should be in Meters again
        distance = da.measureLine(QgsPointXY(1, 1), QgsPointXY(2, 3))
        units = da.lengthUnits()
        print(("measured {} in {}".format(distance, QgsUnitTypes.toString(units))))
        self.assertAlmostEqual(distance, 0.67953772, delta=0.000001)
        self.assertEqual(units, QgsUnitTypes.DistanceMeters)

        # test converting the resultant length
        distance = da.convertLengthMeasurement(distance, QgsUnitTypes.DistanceFeet)
        self.assertAlmostEqual(distance, 2.2294, delta=0.001)

    def testAreaMeasureAndUnits(self):
        """Test a variety of area measurements in different CRS and ellipsoid modes, to check that the
           calculated areas and units are always consistent
        """

        da = QgsDistanceArea()
        da.setSourceCrs(QgsCoordinateReferenceSystem.fromSrsId(3452), QgsProject.instance().transformContext())
        da.setEllipsoid("NONE")

        polygon = QgsGeometry.fromPolygonXY(
            [[
                QgsPointXY(0, 0), QgsPointXY(1, 0), QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 2),
                QgsPointXY(0, 2), QgsPointXY(0, 0),
            ]]
        )

        # We check both the measured area AND the units, in case the logic regarding
        # ellipsoids and units changes in future
        area = da.measureArea(polygon)
        units = da.areaUnits()

        print(("measured {} in {}".format(area, QgsUnitTypes.toString(units))))
        assert ((abs(area - 3.0) < 0.00000001 and units == QgsUnitTypes.AreaSquareDegrees)
                or (abs(area - 37176087091.5) < 0.1 and units == QgsUnitTypes.AreaSquareMeters))

        da.setEllipsoid("WGS84")
        area = da.measureArea(polygon)
        units = da.areaUnits()

        print(("measured {} in {}".format(area, QgsUnitTypes.toString(units))))
        # should always be in Meters Squared
        self.assertAlmostEqual(area, 36922805935.96157, delta=0.1)
        self.assertEqual(units, QgsUnitTypes.AreaSquareMeters)

        # test converting the resultant area
        area = da.convertAreaMeasurement(area, QgsUnitTypes.AreaSquareMiles)
        self.assertAlmostEqual(area, 14255.975071318593, delta=0.001)

        # now try with a source CRS which is in feet
        polygon = QgsGeometry.fromPolygonXY(
            [[
                QgsPointXY(1850000, 4423000), QgsPointXY(1851000, 4423000), QgsPointXY(1851000, 4424000),
                QgsPointXY(1852000, 4424000), QgsPointXY(1852000, 4425000), QgsPointXY(1851000, 4425000),
                QgsPointXY(1850000, 4423000)
            ]]
        )
        da.setSourceCrs(QgsCoordinateReferenceSystem.fromSrsId(27469), QgsProject.instance().transformContext())
        da.setEllipsoid("NONE")
        # measurement should be in square feet
        area = da.measureArea(polygon)
        units = da.areaUnits()
        print(("measured {} in {}".format(area, QgsUnitTypes.toString(units))))
        self.assertAlmostEqual(area, 2000000, delta=0.001)
        self.assertEqual(units, QgsUnitTypes.AreaSquareFeet)

        # test converting the resultant area
        area = da.convertAreaMeasurement(area, QgsUnitTypes.AreaSquareYards)
        self.assertAlmostEqual(area, 222222.2222, delta=0.001)

        da.setEllipsoid("WGS84")
        # now should be in Square Meters again
        area = da.measureArea(polygon)
        units = da.areaUnits()
        print(("measured {} in {}".format(area, QgsUnitTypes.toString(units))))
        self.assertAlmostEqual(area, 185825.2069028169, delta=1.0)
        self.assertEqual(units, QgsUnitTypes.AreaSquareMeters)

        # test converting the resultant area
        area = da.convertAreaMeasurement(area, QgsUnitTypes.AreaSquareYards)
        self.assertAlmostEqual(area, 222245.0978076078, delta=1.0)

    def testFormatDistance(self):
        """Test formatting distances"""
        QLocale.setDefault(QLocale.c())
        self.assertEqual(QgsDistanceArea.formatDistance(45, 3, QgsUnitTypes.DistanceMeters), '45.000 m')
        self.assertEqual(QgsDistanceArea.formatDistance(1300, 1, QgsUnitTypes.DistanceMeters, False), '1.3 km')
        self.assertEqual(QgsDistanceArea.formatDistance(.005, 1, QgsUnitTypes.DistanceMeters, False), '5.0 mm')
        self.assertEqual(QgsDistanceArea.formatDistance(.05, 1, QgsUnitTypes.DistanceMeters, False), '5.0 cm')
        self.assertEqual(QgsDistanceArea.formatDistance(1.5, 3, QgsUnitTypes.DistanceKilometers, True), '1.500 km')
        self.assertEqual(QgsDistanceArea.formatDistance(1.5, 3, QgsUnitTypes.DistanceKilometers, False), '1.500 km')
        self.assertEqual(QgsDistanceArea.formatDistance(0.5, 3, QgsUnitTypes.DistanceKilometers, True), '0.500 km')
        self.assertEqual(QgsDistanceArea.formatDistance(0.5, 3, QgsUnitTypes.DistanceKilometers, False), '500.000 m')
        self.assertEqual(QgsDistanceArea.formatDistance(6000, 0, QgsUnitTypes.DistanceFeet, True), '6000 ft')
        self.assertEqual(QgsDistanceArea.formatDistance(6000, 3, QgsUnitTypes.DistanceFeet, False), '1.136 mi')
        self.assertEqual(QgsDistanceArea.formatDistance(300, 0, QgsUnitTypes.DistanceFeet, True), '300 ft')
        self.assertEqual(QgsDistanceArea.formatDistance(300, 0, QgsUnitTypes.DistanceFeet, False), '300 ft')
        self.assertEqual(QgsDistanceArea.formatDistance(3000, 0, QgsUnitTypes.DistanceYards, True), '3000 yd')
        self.assertEqual(QgsDistanceArea.formatDistance(3000, 3, QgsUnitTypes.DistanceYards, False), '1.705 mi')
        self.assertEqual(QgsDistanceArea.formatDistance(300, 0, QgsUnitTypes.DistanceYards, True), '300 yd')
        self.assertEqual(QgsDistanceArea.formatDistance(300, 0, QgsUnitTypes.DistanceYards, False), '300 yd')
        self.assertEqual(QgsDistanceArea.formatDistance(1.5, 3, QgsUnitTypes.DistanceMiles, True), '1.500 mi')
        self.assertEqual(QgsDistanceArea.formatDistance(1.5, 3, QgsUnitTypes.DistanceMiles, False), '1.500 mi')
        self.assertEqual(QgsDistanceArea.formatDistance(0.5, 3, QgsUnitTypes.DistanceMiles, True), '0.500 mi')
        self.assertEqual(QgsDistanceArea.formatDistance(0.5, 0, QgsUnitTypes.DistanceMiles, False), '2640 ft')
        self.assertEqual(QgsDistanceArea.formatDistance(0.5, 1, QgsUnitTypes.DistanceNauticalMiles, True), '0.5 NM')
        self.assertEqual(QgsDistanceArea.formatDistance(0.5, 1, QgsUnitTypes.DistanceNauticalMiles, False), '0.5 NM')
        self.assertEqual(QgsDistanceArea.formatDistance(1.5, 1, QgsUnitTypes.DistanceNauticalMiles, True), '1.5 NM')
        self.assertEqual(QgsDistanceArea.formatDistance(1.5, 1, QgsUnitTypes.DistanceNauticalMiles, False), '1.5 NM')
        self.assertEqual(QgsDistanceArea.formatDistance(1.5, 1, QgsUnitTypes.DistanceDegrees, True), '1.5 deg')
        self.assertEqual(QgsDistanceArea.formatDistance(1.0, 1, QgsUnitTypes.DistanceDegrees, False), '1.0 deg')
        self.assertEqual(QgsDistanceArea.formatDistance(1.0, 1, QgsUnitTypes.DistanceUnknownUnit, False), '1.0')
        QLocale.setDefault(QLocale.system())

    def testGeodesicIntersectionAtAntimeridian(self):
        da = QgsDistanceArea()
        crs = QgsCoordinateReferenceSystem('EPSG:4326')
        da.setSourceCrs(crs, QgsProject.instance().transformContext())
        da.setEllipsoid("WGS84")

        lat, fract = da.latitudeGeodesicCrossesAntimeridian(QgsPointXY(0, 0), QgsPointXY(-170, 0))
        self.assertAlmostEqual(lat, 0, 5)
        self.assertAlmostEqual(fract, 0, 5)
        lat, fract = da.latitudeGeodesicCrossesAntimeridian(QgsPointXY(-170, 0), QgsPointXY(170, 0))
        self.assertAlmostEqual(lat, 0, 5)
        self.assertAlmostEqual(fract, 0.5, 5)
        lat, fract = da.latitudeGeodesicCrossesAntimeridian(QgsPointXY(179, 0), QgsPointXY(181, 0))
        self.assertAlmostEqual(lat, 0, 5)
        self.assertAlmostEqual(fract, 0.5, 5)
        lat, fract = da.latitudeGeodesicCrossesAntimeridian(QgsPointXY(-170, 0), QgsPointXY(170, 0))
        self.assertAlmostEqual(lat, 0, 5)
        self.assertAlmostEqual(fract, 0.5, 5)
        lat, fract = da.latitudeGeodesicCrossesAntimeridian(QgsPointXY(180, 0), QgsPointXY(180, 0))
        self.assertAlmostEqual(lat, 0, 5)
        lat, fract = da.latitudeGeodesicCrossesAntimeridian(QgsPointXY(180, -10), QgsPointXY(180, -10))
        self.assertAlmostEqual(lat, -10, 5)
        lat, fract = da.latitudeGeodesicCrossesAntimeridian(QgsPointXY(171, 0), QgsPointXY(181, 0))
        self.assertAlmostEqual(lat, 0, 5)
        self.assertAlmostEqual(fract, 0.9, 5)
        lat, fract = da.latitudeGeodesicCrossesAntimeridian(QgsPointXY(181, 0), QgsPointXY(171, 0))
        self.assertAlmostEqual(lat, 0, 5)
        self.assertAlmostEqual(fract, 0.1, 5)

        lat, fract = da.latitudeGeodesicCrossesAntimeridian(QgsPointXY(138.26237, -20.314687),
                                                            QgsPointXY(-151.6, -77.8))
        self.assertAlmostEqual(lat, -73.89148222666744914, 5)
        self.assertAlmostEqual(fract, 0.007113545719515548, 5)
        lat, fract = da.latitudeGeodesicCrossesAntimeridian(QgsPointXY(138.26237, -20.314687),
                                                            QgsPointXY(-151.6 + 360, -77.8))
        self.assertAlmostEqual(lat, -73.89148222666744914, 5)
        self.assertAlmostEqual(fract, 0.007113545719515548, 5)
        lat, fract = da.latitudeGeodesicCrossesAntimeridian(QgsPointXY(-151.6, -77.8),
                                                            QgsPointXY(138.26237, -20.314687))
        self.assertAlmostEqual(lat, -73.89148222666744914, 5)
        self.assertAlmostEqual(fract, 0.9928864542804845, 5)
        lat, fract = da.latitudeGeodesicCrossesAntimeridian(QgsPointXY(170.60188754234980024, -70.81368329001529105),
                                                            QgsPointXY(-164.61259948055175073, -76.66761193248410677))
        self.assertAlmostEqual(lat, -73.89148222666744914, 5)
        self.assertAlmostEqual(fract, 0.0879577697523441, 5)
        lat, fract = da.latitudeGeodesicCrossesAntimeridian(QgsPointXY(-164.61259948055175073, -76.66761193248410677),
                                                            QgsPointXY(170.60188754234980024,
                                                                       -70.81368329001529105))
        self.assertAlmostEqual(lat, -73.89148222666744914, 5)
        self.assertAlmostEqual(fract, 0.9120422302476558, 5)
        lat, fract = da.latitudeGeodesicCrossesAntimeridian(QgsPointXY(178.44469761238570982, -73.47820480021761114),
                                                            QgsPointXY(-179.21026002627399976, -74.08952948682963324))
        self.assertAlmostEqual(lat, -73.89148222666744914, 5)
        self.assertAlmostEqual(fract, 0.6713541474159178, 5)
        lat, fract = da.latitudeGeodesicCrossesAntimeridian(QgsPointXY(-179.21026002627399976, -74.08952948682963324),
                                                            QgsPointXY(178.44469761238570982,
                                                                       -73.47820480021761114))
        self.assertAlmostEqual(lat, -73.89148222666744914, 5)
        self.assertAlmostEqual(fract, 0.3286458525840822, 5)
        lat, fract = da.latitudeGeodesicCrossesAntimeridian(QgsPointXY(179.83103440731269984, -73.8481044794813215),
                                                            QgsPointXY(-179.93191793815378787, -73.90885909527753483))
        self.assertAlmostEqual(lat, -73.89148222666744914, 5)
        self.assertAlmostEqual(fract, 0.7135414998986486, 5)
        lat, fract = da.latitudeGeodesicCrossesAntimeridian(QgsPointXY(-179.93191793815378787, -73.90885909527753483),
                                                            QgsPointXY(179.83103440731269984, -73.8481044794813215))
        self.assertAlmostEqual(lat, -73.89148222666744914, 5)
        self.assertAlmostEqual(fract, 0.28645850010135143, 5)

        lat, fract = da.latitudeGeodesicCrossesAntimeridian(QgsPointXY(179.92498611649580198, 7.24703528617311754),
                                                            QgsPointXY(-178.20070563806575592, 16.09649962419504732))
        self.assertAlmostEqual(lat, 7.6112109902580265, 5)
        self.assertAlmostEqual(fract, 0.04111771567489498, 5)
        lat, fract = da.latitudeGeodesicCrossesAntimeridian(QgsPointXY(-178.20070563806575592, 16.09649962419504732),
                                                            QgsPointXY(179.92498611649580198, 7.24703528617311754))
        self.assertAlmostEqual(lat, 7.6112109902580265, 5)
        self.assertAlmostEqual(fract, 0.958882284325105, 5)
        lat, fract = da.latitudeGeodesicCrossesAntimeridian(
            QgsPointXY(360 - 178.20070563806575592, 16.09649962419504732),
            QgsPointXY(179.92498611649580198, 7.24703528617311754))
        self.assertAlmostEqual(lat, 7.6112109902580265, 5)
        self.assertAlmostEqual(fract, 0.95888228432510, 5)

        lat, fract = da.latitudeGeodesicCrossesAntimeridian(QgsPointXY(175.76717768974583578, 8.93749416467257873),
                                                            QgsPointXY(-175.15030911497356669, 8.59851183021221033))
        self.assertAlmostEqual(lat, 8.80683758146703966, 5)
        self.assertAlmostEqual(fract, 0.46581637044475815, 5)
        lat, fract = da.latitudeGeodesicCrossesAntimeridian(QgsPointXY(-175.15030911497356669, 8.59851183021221033),
                                                            QgsPointXY(175.76717768974583578,
                                                                       8.93749416467257873))
        self.assertAlmostEqual(lat, 8.80683758146703966, 5)
        self.assertAlmostEqual(fract, 0.5341836295552418, 5)

        # calculation should be ellipsoid dependent!
        da.setEllipsoid('PARAMETER:6370997:6370997')
        lat, fract = da.latitudeGeodesicCrossesAntimeridian(QgsPointXY(-175.15030911497356669, 8.59851183021221033),
                                                            QgsPointXY(175.76717768974583578,
                                                                       8.93749416467257873))
        self.assertAlmostEqual(lat, 8.806658717133244, 5)
        self.assertAlmostEqual(fract, 0.5341851152000393, 5)

        # no ellipsoid
        da.setEllipsoid("NONE")
        lat, fract = da.latitudeGeodesicCrossesAntimeridian(QgsPointXY(-175, 8),
                                                            QgsPointXY(175,
                                                                       9))
        self.assertAlmostEqual(lat, 8.5, 5)
        self.assertAlmostEqual(fract, 0.5, 5)
        lat, fract = da.latitudeGeodesicCrossesAntimeridian(QgsPointXY(165, 8),
                                                            QgsPointXY(-175,
                                                                       9))
        self.assertAlmostEqual(lat, 8.75, 5)
        self.assertAlmostEqual(fract, 0.75, 5)
        lat, fract = da.latitudeGeodesicCrossesAntimeridian(QgsPointXY(-175, 8),
                                                            QgsPointXY(165,
                                                                       9))
        self.assertAlmostEqual(lat, 8.25, 5)
        self.assertAlmostEqual(fract, 0.25, 5)

    def testGeodesicLine(self):
        da = QgsDistanceArea()
        crs = QgsCoordinateReferenceSystem('EPSG:4326')
        da.setSourceCrs(crs, QgsProject.instance().transformContext())
        da.setEllipsoid("WGS84")
        g = QgsGeometry.fromMultiPolylineXY(da.geodesicLine(QgsPointXY(105.4, 66.4), QgsPointXY(208.4, -77.8),
                                                            1000000, True))
        self.assertEqual(g.asWkt(5),
                         'MultiLineString ((105.4 66.4, 114.11119 58.36882, 119.52376 49.95732, 123.30625 41.35664, 126.19835 32.6479, 128.57411 23.87234, 130.647 15.05482, 132.55465 6.21309, 134.39916 -2.63822, 136.27014 -11.48549, 138.26237 -20.31469, 140.4956 -29.10966, 143.14591 -37.84912, 146.5073 -46.50015, 151.13295 -55.00229, 158.2045 -63.2234, 170.60189 -70.81368, 180 -73.89148),(-180 -73.89148, -164.6126 -76.66761, -151.6 -77.8))')
        g = QgsGeometry.fromMultiPolylineXY(da.geodesicLine(QgsPointXY(105.4, 66.4), QgsPointXY(208.4, -77.8),
                                                            1000000, False))
        self.assertEqual(g.asWkt(5),
                         'MultiLineString ((105.4 66.4, 114.11119 58.36882, 119.52376 49.95732, 123.30625 41.35664, 126.19835 32.6479, 128.57411 23.87234, 130.647 15.05482, 132.55465 6.21309, 134.39916 -2.63822, 136.27014 -11.48549, 138.26237 -20.31469, 140.4956 -29.10966, 143.14591 -37.84912, 146.5073 -46.50015, 151.13295 -55.00229, 158.2045 -63.2234, 170.60189 -70.81368, -164.6126 -76.66761, -151.6 -77.8))')
        g = QgsGeometry.fromMultiPolylineXY(da.geodesicLine(QgsPointXY(105.4, 66.4), QgsPointXY(208.4, -77.8),
                                                            100000, True))
        self.assertEqual(g.asWkt(5),
                         'MultiLineString ((105.4 66.4, 106.50684 65.62452, 107.54925 64.84137, 108.53251 64.05125, 109.46143 63.25477, 110.34036 62.45245, 111.17326 61.6448, 111.96369 60.83224, 112.7149 60.01516, 113.42984 59.19392, 114.11119 58.36882, 114.76141 57.54016, 115.38271 56.70818, 115.97713 55.87314, 116.54653 55.03522, 117.09261 54.19464, 117.61695 53.35156, 118.12096 52.50615, 118.60597 51.65855, 119.0732 50.8089, 119.52376 49.95732, 119.95868 49.10392, 120.37892 48.24881, 120.78537 47.39209, 121.17884 46.53385, 121.5601 45.67416, 121.92984 44.81311, 122.28874 43.95077, 122.6374 43.08721, 122.97639 42.22247, 123.30625 41.35664, 123.62747 40.48975, 123.94053 39.62186, 124.24585 38.75302, 124.54386 37.88326, 124.83494 37.01265, 125.11945 36.14121, 125.39773 35.26898, 125.67011 34.39599, 125.93688 33.52229, 126.19835 32.6479, 126.45477 31.77286, 126.7064 30.89719, 126.9535 30.02092, 127.19628 29.14407, 127.43498 28.26668, 127.66979 27.38877, 127.90093 26.51035, 128.12857 25.63146, 128.35291 24.75212, 128.57411 23.87234, 128.79234 22.99214, 129.00775 22.11156, 129.22051 21.23059, 129.43076 20.34927, 129.63863 19.46761, 129.84427 18.58563, 130.0478 17.70334, 130.24935 16.82077, 130.44904 15.93792, 130.647 15.05482, 130.84333 14.17148, 131.03815 13.28792, 131.23156 12.40414, 131.42367 11.52017, 131.61458 10.63603, 131.8044 9.75171, 131.99322 8.86725, 132.18114 7.98265, 132.36825 7.09792, 132.55465 6.21309, 132.74043 5.32816, 132.92567 4.44315, 133.11048 3.55808, 133.29493 2.67295, 133.47912 1.78778, 133.66313 0.90258, 133.84706 0.01736, 134.03098 -0.86785, 134.21498 -1.75305, 134.39916 -2.63822, 134.5836 -3.52335, 134.76839 -4.40843, 134.95362 -5.29344, 135.13937 -6.17837, 135.32575 -7.06321, 135.51283 -7.94794, 135.70071 -8.83255, 135.8895 -9.71702, 136.07928 -10.60134, 136.27014 -11.48549, 136.46221 -12.36947, 136.65557 -13.25325, 136.85033 -14.13682, 137.04659 -15.02017, 137.24448 -15.90328, 137.44411 -16.78614, 137.64558 -17.66872, 137.84904 -18.55102, 138.05459 -19.43301, 138.26237 -20.31469, 138.47252 -21.19602, 138.68518 -22.077, 138.90049 -22.9576, 139.1186 -23.83781, 139.33968 -24.71761, 139.56389 -25.59697, 139.7914 -26.47588, 140.0224 -27.35431, 140.25706 -28.23225, 140.4956 -29.10966, 140.73822 -29.98653, 140.98515 -30.86282, 141.2366 -31.73851, 141.49283 -32.61358, 141.75409 -33.488, 142.02065 -34.36173, 142.2928 -35.23474, 142.57084 -36.107, 142.8551 -36.97847, 143.14591 -37.84912, 143.44364 -38.71891, 143.74868 -39.58779, 144.06142 -40.45572, 144.38232 -41.32265, 144.71183 -42.18852, 145.05045 -43.0533, 145.39872 -43.91692, 145.7572 -44.77931, 146.12651 -45.64041, 146.5073 -46.50015, 146.90028 -47.35845, 147.3062 -48.21523, 147.72589 -49.0704, 148.16022 -49.92387, 148.61014 -50.77552, 149.0767 -51.62525, 149.56099 -52.47294, 150.06423 -53.31844, 150.58774 -54.16161, 151.13295 -55.00229, 151.7014 -55.84031, 152.29481 -56.67548, 152.91501 -57.50758, 153.56405 -58.33637, 154.24414 -59.16162, 154.95771 -59.98302, 155.70745 -60.80027, 156.49628 -61.61301, 157.32744 -62.42086, 158.2045 -63.2234, 159.13138 -64.02012, 160.11242 -64.8105, 161.15241 -65.59393, 162.25662 -66.36973, 163.43088 -67.13713, 164.68163 -67.89527, 166.01595 -68.64317, 167.44162 -69.37973, 168.96717 -70.10371, 170.60189 -70.81368, 172.35586 -71.50806, 174.2399 -72.18501, 176.26551 -72.8425, 178.4447 -73.4782, 180 -73.89148),(-180 -73.89148, -179.21026 -74.08953, -176.68721 -74.67356, -173.97467 -75.22708, -171.06257 -75.74654, -167.94325 -76.22808, -164.6126 -76.66761, -161.07142 -77.06086, -157.32679 -77.40349, -153.39333 -77.69129, -151.6 -77.8))')
        g = QgsGeometry.fromMultiPolylineXY(da.geodesicLine(QgsPointXY(105.4, 66.4), QgsPointXY(208.4, -77.8),
                                                            100000, False))
        self.assertEqual(g.asWkt(5),
                         'MultiLineString ((105.4 66.4, 106.50684 65.62452, 107.54925 64.84137, 108.53251 64.05125, 109.46143 63.25477, 110.34036 62.45245, 111.17326 61.6448, 111.96369 60.83224, 112.7149 60.01516, 113.42984 59.19392, 114.11119 58.36882, 114.76141 57.54016, 115.38271 56.70818, 115.97713 55.87314, 116.54653 55.03522, 117.09261 54.19464, 117.61695 53.35156, 118.12096 52.50615, 118.60597 51.65855, 119.0732 50.8089, 119.52376 49.95732, 119.95868 49.10392, 120.37892 48.24881, 120.78537 47.39209, 121.17884 46.53385, 121.5601 45.67416, 121.92984 44.81311, 122.28874 43.95077, 122.6374 43.08721, 122.97639 42.22247, 123.30625 41.35664, 123.62747 40.48975, 123.94053 39.62186, 124.24585 38.75302, 124.54386 37.88326, 124.83494 37.01265, 125.11945 36.14121, 125.39773 35.26898, 125.67011 34.39599, 125.93688 33.52229, 126.19835 32.6479, 126.45477 31.77286, 126.7064 30.89719, 126.9535 30.02092, 127.19628 29.14407, 127.43498 28.26668, 127.66979 27.38877, 127.90093 26.51035, 128.12857 25.63146, 128.35291 24.75212, 128.57411 23.87234, 128.79234 22.99214, 129.00775 22.11156, 129.22051 21.23059, 129.43076 20.34927, 129.63863 19.46761, 129.84427 18.58563, 130.0478 17.70334, 130.24935 16.82077, 130.44904 15.93792, 130.647 15.05482, 130.84333 14.17148, 131.03815 13.28792, 131.23156 12.40414, 131.42367 11.52017, 131.61458 10.63603, 131.8044 9.75171, 131.99322 8.86725, 132.18114 7.98265, 132.36825 7.09792, 132.55465 6.21309, 132.74043 5.32816, 132.92567 4.44315, 133.11048 3.55808, 133.29493 2.67295, 133.47912 1.78778, 133.66313 0.90258, 133.84706 0.01736, 134.03098 -0.86785, 134.21498 -1.75305, 134.39916 -2.63822, 134.5836 -3.52335, 134.76839 -4.40843, 134.95362 -5.29344, 135.13937 -6.17837, 135.32575 -7.06321, 135.51283 -7.94794, 135.70071 -8.83255, 135.8895 -9.71702, 136.07928 -10.60134, 136.27014 -11.48549, 136.46221 -12.36947, 136.65557 -13.25325, 136.85033 -14.13682, 137.04659 -15.02017, 137.24448 -15.90328, 137.44411 -16.78614, 137.64558 -17.66872, 137.84904 -18.55102, 138.05459 -19.43301, 138.26237 -20.31469, 138.47252 -21.19602, 138.68518 -22.077, 138.90049 -22.9576, 139.1186 -23.83781, 139.33968 -24.71761, 139.56389 -25.59697, 139.7914 -26.47588, 140.0224 -27.35431, 140.25706 -28.23225, 140.4956 -29.10966, 140.73822 -29.98653, 140.98515 -30.86282, 141.2366 -31.73851, 141.49283 -32.61358, 141.75409 -33.488, 142.02065 -34.36173, 142.2928 -35.23474, 142.57084 -36.107, 142.8551 -36.97847, 143.14591 -37.84912, 143.44364 -38.71891, 143.74868 -39.58779, 144.06142 -40.45572, 144.38232 -41.32265, 144.71183 -42.18852, 145.05045 -43.0533, 145.39872 -43.91692, 145.7572 -44.77931, 146.12651 -45.64041, 146.5073 -46.50015, 146.90028 -47.35845, 147.3062 -48.21523, 147.72589 -49.0704, 148.16022 -49.92387, 148.61014 -50.77552, 149.0767 -51.62525, 149.56099 -52.47294, 150.06423 -53.31844, 150.58774 -54.16161, 151.13295 -55.00229, 151.7014 -55.84031, 152.29481 -56.67548, 152.91501 -57.50758, 153.56405 -58.33637, 154.24414 -59.16162, 154.95771 -59.98302, 155.70745 -60.80027, 156.49628 -61.61301, 157.32744 -62.42086, 158.2045 -63.2234, 159.13138 -64.02012, 160.11242 -64.8105, 161.15241 -65.59393, 162.25662 -66.36973, 163.43088 -67.13713, 164.68163 -67.89527, 166.01595 -68.64317, 167.44162 -69.37973, 168.96717 -70.10371, 170.60189 -70.81368, 172.35586 -71.50806, 174.2399 -72.18501, 176.26551 -72.8425, 178.4447 -73.4782, -179.21026 -74.08953, -176.68721 -74.67356, -173.97467 -75.22708, -171.06257 -75.74654, -167.94325 -76.22808, -164.6126 -76.66761, -161.07142 -77.06086, -157.32679 -77.40349, -153.39333 -77.69129, -151.6 -77.8))')

        g = QgsGeometry.fromMultiPolylineXY(da.geodesicLine(QgsPointXY(121.4, -76.4), QgsPointXY(-121.6, 76.8),
                                                            1000000, True))
        self.assertEqual(g.asWkt(5),
                         'MultiLineString ((121.4 -76.4, 144.24671 -70.15471, 155.62067 -62.40689, 162.18341 -54.11007, 166.52394 -45.56426, 169.70696 -36.88448, 172.23571 -28.12423, 174.38053 -19.31324, 176.30526 -10.47118, 178.12301 -1.61326, 179.92499 7.24704, 180 7.61121),(-180 7.61121, -178.20071 16.0965, -176.15151 24.92076, -173.78654 33.70222, -170.88367 42.4158, -167.0472 51.01916, -161.47936 59.42708, -152.33578 67.43298, -134.83075 74.42214, -121.6 76.8))')
        g = QgsGeometry.fromMultiPolylineXY(da.geodesicLine(QgsPointXY(121.4, -76.4), QgsPointXY(-121.6, 76.8),
                                                            1000000, False))
        self.assertEqual(g.asWkt(5),
                         'MultiLineString ((121.4 -76.4, 144.24671 -70.15471, 155.62067 -62.40689, 162.18341 -54.11007, 166.52394 -45.56426, 169.70696 -36.88448, 172.23571 -28.12423, 174.38053 -19.31324, 176.30526 -10.47118, 178.12301 -1.61326, 179.92499 7.24704, -178.20071 16.0965, -176.15151 24.92076, -173.78654 33.70222, -170.88367 42.4158, -167.0472 51.01916, -161.47936 59.42708, -152.33578 67.43298, -134.83075 74.42214, -121.6 76.8))')
        g = QgsGeometry.fromMultiPolylineXY(da.geodesicLine(QgsPointXY(121.4, 6.4), QgsPointXY(-121.6, 2.8),
                                                            1000000, True))
        self.assertEqual(g.asWkt(5),
                         'MultiLineString ((121.4 6.4, 130.40033 7.32484, 139.43407 8.06935, 148.49637 8.61432, 157.57946 8.9455, 166.67342 9.05419, 175.76718 8.93749, 180 8.80684),(-180 8.80684, -175.15031 8.59851, -166.08891 8.04617, -157.05629 7.29488, -148.0572 6.36403, -139.09301 5.2773, -130.16168 4.06195, -121.6 2.8))')
        g = QgsGeometry.fromMultiPolylineXY(da.geodesicLine(QgsPointXY(121.4, 6.4), QgsPointXY(-121.6, 2.8),
                                                            1000000, False))
        self.assertEqual(g.asWkt(5),
                         'MultiLineString ((121.4 6.4, 130.40033 7.32484, 139.43407 8.06935, 148.49637 8.61432, 157.57946 8.9455, 166.67342 9.05419, 175.76718 8.93749, -175.15031 8.59851, -166.08891 8.04617, -157.05629 7.29488, -148.0572 6.36403, -139.09301 5.2773, -130.16168 4.06195, -121.6 2.8))')

        # different ellipsoid, should be respected
        da.setEllipsoid('PARAMETER:6370997:6370997')
        g = QgsGeometry.fromMultiPolylineXY(da.geodesicLine(QgsPointXY(121.4, 6.4), QgsPointXY(-121.6, 2.8),
                                                            1000000, False))
        self.assertEqual(g.asWkt(5),
                         'MultiLineString ((121.4 6.4, 130.41144 7.31297, 139.45604 8.04667, 148.52889 8.58224, 157.62221 8.90571, 166.72609 9.0086, 175.82954 8.88819, -175.07842 8.54766, -166.00754 7.99595, -156.96541 7.24741, -147.95669 6.32128, -138.98271 5.24103, -130.04141 4.03364, -121.6 2.8))')

        da.setEllipsoid("WGS84")
        # with reprojection
        da.setSourceCrs(QgsCoordinateReferenceSystem('EPSG:3857'), QgsProject.instance().transformContext())
        g = QgsGeometry.fromMultiPolylineXY(
            da.geodesicLine(QgsPointXY(-13536427, 14138932), QgsPointXY(13760912, -13248201),
                            1000000, False))
        self.assertEqual(g.asWkt(0),
                         'MultiLineString ((-13536427 14138932, -16514348 11691516, -17948849 9406595, -18744235 7552985, -19255354 6014890, -19622372 4688888, -19909239 3505045, 19925702 2415579, 19712755 1385803, 19513769 388441, 19318507 -600065, 19117459 -1602293, 18899973 -2642347, 18651869 -3748726, 18351356 -4958346, 17960498 -6322823, 17404561 -7918366, 16514601 -9855937, 14851845 -12232940, 13760912 -13248201))')
        g = QgsGeometry.fromMultiPolylineXY(
            da.geodesicLine(QgsPointXY(-13536427, 14138932), QgsPointXY(13760912, -13248201),
                            1000000, True))
        self.assertEqual(g.asWkt(0),
                         'MultiLineString ((-13536427 14138932, -16514348 11691516, -17948849 9406595, -18744235 7552985, -19255354 6014890, -19622372 4688888, -19909239 3505045, -20037508 2933522),(20037508 2933522, 19925702 2415579, 19712755 1385803, 19513769 388441, 19318507 -600065, 19117459 -1602293, 18899973 -2642347, 18651869 -3748726, 18351356 -4958346, 17960498 -6322823, 17404561 -7918366, 16514601 -9855937, 14851845 -12232940, 13760912 -13248201))')
        g = QgsGeometry.fromMultiPolylineXY(
            da.geodesicLine(QgsPointXY(18933544, -5448034), QgsPointXY(-11638480, 3962206),
                            1000000, True))
        self.assertEqual(g.asWkt(0),
                         'MultiLineString ((18933544 -5448034, 20037508 -4772933),(-20037508 -4772933, -20002064 -4748323, -19015781 -3988451, -18153035 -3204936, -17383137 -2416816, -16678635 -1632067, -16015884 -852355, -15374147 -76043, -14734258 699941, -14077193 1478790, -13382634 2262546, -12627598 3050380, -11785404 3835868, -11638480 3962206))')

    def testSplitGeometryAtAntimeridian(self):
        da = QgsDistanceArea()
        crs = QgsCoordinateReferenceSystem('EPSG:4326')
        da.setSourceCrs(crs, QgsProject.instance().transformContext())
        da.setEllipsoid("WGS84")

        # noops
        g = da.splitGeometryAtAntimeridian(QgsGeometry())
        self.assertTrue(g.isNull())
        g = da.splitGeometryAtAntimeridian(QgsGeometry.fromWkt('Point(1 2)'))
        self.assertEqual(g.asWkt(), 'Point (1 2)')
        g = da.splitGeometryAtAntimeridian(QgsGeometry.fromWkt('MultiPoint(1 2, 3 4)'))
        self.assertEqual(g.asWkt(), 'MultiPoint ((1 2),(3 4))')
        g = da.splitGeometryAtAntimeridian(QgsGeometry.fromWkt('PointZ(1 2 3)'))
        self.assertEqual(g.asWkt(), 'PointZ (1 2 3)')
        g = da.splitGeometryAtAntimeridian(QgsGeometry.fromWkt('PointM(1 2 3)'))
        self.assertEqual(g.asWkt(), 'PointM (1 2 3)')
        g = da.splitGeometryAtAntimeridian(QgsGeometry.fromWkt('LineString EMPTY'))
        self.assertEqual(g.asWkt(), 'MultiLineString EMPTY')

        # lines
        g = da.splitGeometryAtAntimeridian(QgsGeometry.fromWkt('LineString(0 0, -170 0)'))
        self.assertEqual(g.asWkt(), 'MultiLineString ((0 0, -170 0))')
        g = da.splitGeometryAtAntimeridian(QgsGeometry.fromWkt('LineString(-170 0, 0 0)'))
        self.assertEqual(g.asWkt(), 'MultiLineString ((-170 0, 0 0))')
        g = da.splitGeometryAtAntimeridian(QgsGeometry.fromWkt('LineString(179 0, -179 0)'))
        self.assertEqual(g.asWkt(), 'MultiLineString ((179 0, 180 0),(-180 0, -179 0))')
        g = da.splitGeometryAtAntimeridian(QgsGeometry.fromWkt('LineString(179 0, 181 0)'))
        self.assertEqual(g.asWkt(), 'MultiLineString ((179 0, 180 0),(-180 0, -179 0))')
        g = da.splitGeometryAtAntimeridian(QgsGeometry.fromWkt('LineString(-179 0, 179 0)'))
        self.assertEqual(g.asWkt(), 'MultiLineString ((-179 0, -180 0),(180 0, 179 0))')
        g = da.splitGeometryAtAntimeridian(QgsGeometry.fromWkt('LineString(181 0, 179 0)'))
        self.assertEqual(g.asWkt(), 'MultiLineString ((-179 0, -180 0),(180 0, 179 0))')
        g = da.splitGeometryAtAntimeridian(QgsGeometry.fromWkt('LineString(179 10, -179 -20)'))
        self.assertEqual(g.asWkt(3), 'MultiLineString ((179 10, 180 -5.362),(-180 -5.362, -179 -20))')
        g = da.splitGeometryAtAntimeridian(QgsGeometry.fromWkt('LineString(179 -80, -179 70)'))
        self.assertEqual(g.asWkt(3), 'MultiLineString ((179 -80, 180 -55.685),(-180 -55.685, -179 70))')

        # multiline input
        g = da.splitGeometryAtAntimeridian(QgsGeometry.fromWkt('MultiLineString((1 10, 50 30),(179 -80, -179 70))'))
        self.assertEqual(g.asWkt(3), 'MultiLineString ((1 10, 50 30),(179 -80, 180 -55.685),(-180 -55.685, -179 70))')
        g = da.splitGeometryAtAntimeridian(QgsGeometry.fromWkt('MultiLineString((1 10, 50 30),(179 -80, 179.99 70))'))
        self.assertEqual(g.asWkt(3), 'MultiLineString ((1 10, 50 30),(179 -80, 179.99 70))')

        # with z/m
        g = da.splitGeometryAtAntimeridian(QgsGeometry.fromWkt('LineStringZ(179 -80 1, -179 70 10)'))
        self.assertEqual(g.asWkt(3),
                         'MultiLineStringZ ((179 -80 1, 180 -55.685 2.466),(-180 -55.685 2.466, -179 70 10))')
        g = da.splitGeometryAtAntimeridian(QgsGeometry.fromWkt('LineStringM(179 -80 1, -179 70 10)'))
        self.assertEqual(g.asWkt(3),
                         'MultiLineStringM ((179 -80 1, 180 -55.685 2.466),(-180 -55.685 2.466, -179 70 10))')
        g = da.splitGeometryAtAntimeridian(QgsGeometry.fromWkt('LineStringZM(179 -80 1 -4, -179 70 10 -30)'))
        self.assertEqual(g.asWkt(3),
                         'MultiLineStringZM ((179 -80 1 -4, 180 -55.685 2.466 -8.234),(-180 -55.685 2.466 -8.234, -179 70 10 -30))')
        g = da.splitGeometryAtAntimeridian(
            QgsGeometry.fromWkt('MultiLineStringZ((179 -80 1, -179 70 10),(-170 -5 1, -181 10 5))'))
        self.assertEqual(g.asWkt(3),
                         'MultiLineStringZ ((179 -80 1, 180 -55.685 2.466),(-180 -55.685 2.466, -179 70 10),(-170 -5 1, -181 10 5))')

        # different ellipsoid - should change intersection latitude
        da.setEllipsoid('PARAMETER:6370997:6370997')
        g = da.splitGeometryAtAntimeridian(QgsGeometry.fromWkt('LineString(179 10, -179 -20)'))
        self.assertEqual(g.asWkt(3), 'MultiLineString ((179 10, 180 -5.361),(-180 -5.361, -179 -20))')

        # with reprojection
        da.setEllipsoid("WGS84")
        # with reprojection
        da.setSourceCrs(QgsCoordinateReferenceSystem('EPSG:3857'), QgsProject.instance().transformContext())

        g = da.splitGeometryAtAntimeridian(QgsGeometry.fromWkt('LineString( -13536427 14138932, 13760912 -13248201)'))
        self.assertEqual(g.asWkt(1),
                         'MultiLineString ((-13536427 14138932, -20037508.3 2933521.7),(20037508.3 2933521.7, 13760912 -13248201))')


if __name__ == '__main__':
    unittest.main()
