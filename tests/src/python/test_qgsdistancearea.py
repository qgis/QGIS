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
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

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
        crs = QgsCoordinateReferenceSystem(3111, QgsCoordinateReferenceSystem.EpsgCrsId)
        da.setSourceCrs(crs, QgsProject.instance().transformContext())
        self.assertEqual(da.sourceCrs().srsid(), crs.srsid())

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

    def testMeasureLineProjected(self):
        #   +-+
        #   | |
        # +-+ +
        # test setting/getting the source CRS
        da_3068 = QgsDistanceArea()
        da_wsg84 = QgsDistanceArea()

        da_3068.setSourceCrs(QgsCoordinateReferenceSystem.fromOgcWmsCrs('EPSG:3068'), QgsProject.instance().transformContext())
        if (da_3068.sourceCrs().isGeographic()):
            da_3068.setEllipsoid(da_3068.sourceCrs().ellipsoidAcronym())
        print(("setting [{}] srid [{}] description [{}]".format(u'Soldner Berlin', da_3068.sourceCrs().authid(), da_3068.sourceCrs().description())))
        self.assertEqual(da_3068.sourceCrs().authid(), 'EPSG:3068')
        da_wsg84.setSourceCrs(QgsCoordinateReferenceSystem.fromOgcWmsCrs('EPSG:4326'), QgsProject.instance().transformContext())
        if (da_wsg84.sourceCrs().isGeographic()):
            da_wsg84.setEllipsoid(da_wsg84.sourceCrs().ellipsoidAcronym())
        self.assertEqual(da_wsg84.sourceCrs().authid(), 'EPSG:4326')
        print(("setting [{}] srid [{}] description [{}] isGeographic[{}]".format(u'Wsg84', da_wsg84.sourceCrs().authid(), da_wsg84.sourceCrs().description(), da_wsg84.sourceCrs().isGeographic())))
        # print(("-- projectionAcronym[{}] ellipsoidAcronym[{}] toWkt[{}] mapUnits[{}] toProj4[{}]".format(da_wsg84.sourceCrs().projectionAcronym(),da_wsg84.sourceCrs().ellipsoidAcronym(), da_wsg84.sourceCrs().toWkt(),da_wsg84.sourceCrs().mapUnits(),da_wsg84.sourceCrs().toProj4())))
        print(("Testing Position change for[{}] years[{}]".format(u'Ampelanlage - Potsdamer Platz, Verkehrsinsel', u'1924 and 1998')))

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
        length_1998_mapunits, point_soldner_1998_result = da_3068.measureLineProjected(point_soldner_1924, distance_soldner_meters, azimuth_qpoint)
        self.assertEqual(point_soldner_1998_result.toString(6), point_soldner_1998.toString(6))
        # Test degree based projected point from point_1924 1 meter due East
        point_wsg84_meter_result = QgsPointXY(0, 0)
        point_wsg84_1927_meter = QgsPointXY(13.37652180838435, 52.50952361017102)
        length_meter_mapunits, point_wsg84_meter_result = da_wsg84.measureLineProjected(point_wsg84_1924, 1.0, (math.pi / 2))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 7, QgsUnitTypes.DistanceDegrees, True), '0.0000147 deg')
        self.assertEqual(point_wsg84_meter_result.toString(7), point_wsg84_1927_meter.toString(7))

        point_wsg84_1998_result = QgsPointXY(0, 0)
        length_1928_mapunits, point_wsg84_1998_result = da_wsg84.measureLineProjected(point_wsg84_1924, distance_wsg84_meters, azimuth_wsg84_1924)
        self.assertEqual(QgsDistanceArea.formatDistance(length_1928_mapunits, 7, QgsUnitTypes.DistanceDegrees, True), distance_wsg84_mapunits_format)
        self.assertEqual(point_wsg84_1998_result.toString(7), point_wsg84_1998.toString(7))

    def testMeasureLineProjectedWorldPoints(self):
        #   +-+
        #   | |
        # +-+ +
        # checking returned length_mapunits/projected_points of diffferent world points with results from SpatiaLite ST_Project
        da_3068 = QgsDistanceArea()
        da_3068.setSourceCrs(QgsCoordinateReferenceSystem.fromOgcWmsCrs('EPSG:3068'), QgsProject.instance().transformContext())
        if (da_3068.sourceCrs().isGeographic()):
            da_3068.setEllipsoid(da_3068.sourceCrs().ellipsoidAcronym())
        self.assertEqual(da_3068.sourceCrs().authid(), 'EPSG:3068')
        print(("setting [{}] srid [{}] description [{}] isGeographic[{}] lengthUnits[{}] projectionAcronym[{}] ellipsoidAcronym[{}]".format(u'EPSG:3068', da_3068.sourceCrs().authid(), da_3068.sourceCrs().description(), da_3068.sourceCrs().isGeographic(), QgsUnitTypes.toString(da_3068.lengthUnits()), da_3068.sourceCrs().projectionAcronym(), da_3068.sourceCrs().ellipsoidAcronym())))
        da_wsg84 = QgsDistanceArea()
        da_wsg84.setSourceCrs(QgsCoordinateReferenceSystem.fromOgcWmsCrs('EPSG:4326'), QgsProject.instance().transformContext())
        if (da_wsg84.sourceCrs().isGeographic()):
            da_wsg84.setEllipsoid(da_wsg84.sourceCrs().ellipsoidAcronym())
        self.assertEqual(da_wsg84.sourceCrs().authid(), 'EPSG:4326')
        print(("setting [{}] srid [{}] description [{}] isGeographic[{}] lengthUnits[{}] projectionAcronym[{}] ellipsoidAcronym[{}] ellipsoid[{}]".format(u'EPSG:4326', da_wsg84.sourceCrs().authid(), da_wsg84.sourceCrs().description(), da_wsg84.sourceCrs().isGeographic(), QgsUnitTypes.toString(da_wsg84.lengthUnits()), da_wsg84.sourceCrs().projectionAcronym(), da_wsg84.sourceCrs().ellipsoidAcronym(), da_wsg84.ellipsoid())))
        da_4314 = QgsDistanceArea()
        da_4314.setSourceCrs(QgsCoordinateReferenceSystem.fromOgcWmsCrs('EPSG:4314'), QgsProject.instance().transformContext())
        if (da_4314.sourceCrs().isGeographic()):
            da_4314.setEllipsoid(da_4314.sourceCrs().ellipsoidAcronym())
        self.assertEqual(da_4314.sourceCrs().authid(), 'EPSG:4314')
        print(("setting [{}] srid [{}] description [{}] isGeographic[{}] lengthUnits[{}] projectionAcronym[{}] ellipsoidAcronym[{}]".format(u'EPSG:4314', da_4314.sourceCrs().authid(), da_4314.sourceCrs().description(), da_4314.sourceCrs().isGeographic(), QgsUnitTypes.toString(da_4314.lengthUnits()), da_4314.sourceCrs().projectionAcronym(), da_4314.sourceCrs().ellipsoidAcronym())))
        da_4805 = QgsDistanceArea()
        da_4805.setSourceCrs(QgsCoordinateReferenceSystem.fromOgcWmsCrs('EPSG:4805'), QgsProject.instance().transformContext())
        if (da_4805.sourceCrs().isGeographic()):
            da_4805.setEllipsoid(da_4805.sourceCrs().ellipsoidAcronym())
        self.assertEqual(da_4805.sourceCrs().authid(), 'EPSG:4805')
        print(("setting [{}] srid [{}] description [{}] isGeographic[{}] lengthUnits[{}] projectionAcronym[{}] ellipsoidAcronym[{}]".format(u'EPSG:4805', da_4805.sourceCrs().authid(), da_4805.sourceCrs().description(), da_4805.sourceCrs().isGeographic(), QgsUnitTypes.toString(da_4805.lengthUnits()), da_4805.sourceCrs().projectionAcronym(), da_4805.sourceCrs().ellipsoidAcronym())))
        # EPSG:5665 unknown, why?
        da_5665 = QgsDistanceArea()
        da_5665.setSourceCrs(QgsCoordinateReferenceSystem.fromOgcWmsCrs('EPSG:5665'), QgsProject.instance().transformContext())
        if (da_5665.sourceCrs().isGeographic()):
            da_5665.setEllipsoid(da_5665.sourceCrs().ellipsoidAcronym())
        print(("setting [{}] srid [{}] description [{}] isGeographic[{}] lengthUnits[{}] projectionAcronym[{}] ellipsoidAcronym[{}]".format(u'EPSG:5665', da_5665.sourceCrs().authid(), da_5665.sourceCrs().description(), da_5665.sourceCrs().isGeographic(), QgsUnitTypes.toString(da_5665.lengthUnits()), da_5665.sourceCrs().projectionAcronym(), da_5665.sourceCrs().ellipsoidAcronym())))
        #self.assertEqual(da_5665.sourceCrs().authid(), 'EPSG:5665')
        da_25833 = QgsDistanceArea()
        da_25833.setSourceCrs(QgsCoordinateReferenceSystem.fromOgcWmsCrs('EPSG:25833'), QgsProject.instance().transformContext())
        if (da_25833.sourceCrs().isGeographic()):
            da_25833.setEllipsoid(da_25833.sourceCrs().ellipsoidAcronym())
        print(("setting [{}] srid [{}] description [{}] isGeographic[{}] lengthUnits[{}] projectionAcronym[{}] ellipsoidAcronym[{}]".format(u'EPSG:25833', da_25833.sourceCrs().authid(), da_25833.sourceCrs().description(), da_25833.sourceCrs().isGeographic(), QgsUnitTypes.toString(da_25833.lengthUnits()), da_25833.sourceCrs().projectionAcronym(), da_25833.sourceCrs().ellipsoidAcronym())))
        self.assertEqual(da_25833.sourceCrs().authid(), 'EPSG:25833')

        # Berlin - Brandenburg Gate - Quadriga
        point_berlin_3068 = QgsPointXY(23183.38449999984, 21047.3225000017)
        point_berlin_3068_project = point_berlin_3068.project(1, (math.pi / 2))
        point_meter_result = QgsPointXY(0, 0)
        length_meter_mapunits, point_meter_result = da_3068.measureLineProjected(point_berlin_3068, 1.0, (math.pi / 2))
        pprint(point_meter_result)
        print('-I-> Berlin 3068 length_meter_mapunits[{}] point_meter_result[{}]'.format(QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_3068.lengthUnits(), True), point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 1, da_3068.lengthUnits(), True), '1.0 m')
        self.assertEqual(point_meter_result.toString(7), point_berlin_3068_project.toString(7))
        point_berlin_wsg84 = QgsPointXY(13.37770458660236, 52.51627178856762)
        point_berlin_wsg84_project = QgsPointXY(13.37771931736259, 52.51627178856669)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_berlin_wsg84, 1.0, (math.pi / 2))
        print('-I-> Berlin Wsg84 length_meter_mapunits[{}] point_meter_result[{}] ellipsoid[{}]'.format(QgsDistanceArea.formatDistance(length_meter_mapunits, 20, da_wsg84.lengthUnits(), True), point_meter_result.asWkt(), da_wsg84.ellipsoid()))
        # for unknown reasons, this is returning '0.00001473026 m' instead of '0.00001473026 deg' when using da_wsg84.lengthUnits()
        # self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits,11,da_wsg84.lengthUnits(),True), '0.00001473026 deg')
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 11, QgsUnitTypes.DistanceDegrees, True), '0.00001473076 deg')
        self.assertEqual(point_meter_result.toString(7), point_berlin_wsg84_project.toString(7))
        point_berlin_4314 = QgsPointXY(13.37944343021465, 52.51767872437083)
        point_berlin_4314_project = QgsPointXY(13.37945816324759, 52.5176787243699)
        length_meter_mapunits, point_meter_result = da_4314.measureLineProjected(point_berlin_4314, 1.0, (math.pi / 2))
        print('-I-> Berlin 4314 length_meter_mapunits[{}] point_meter_result[{}]'.format(QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_4314.lengthUnits(), True), point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 9, QgsUnitTypes.DistanceDegrees, True), '0.000014733 deg')
        self.assertEqual(point_meter_result.toString(7), point_berlin_4314_project.toString(7))
        point_berlin_4805 = QgsPointXY(31.04960570069176, 52.5174657497405)
        point_berlin_4805_project = QgsPointXY(31.04962043365347, 52.51746574973957)
        length_meter_mapunits, point_meter_result = da_4805.measureLineProjected(point_berlin_4805, 1.0, (math.pi / 2))
        print('-I-> Berlin 4805 length_meter_mapunits[{}] point_meter_result[{}]'.format(QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_4805.lengthUnits(), True), point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 9, QgsUnitTypes.DistanceDegrees, True), '0.000014733 deg')
        self.assertEqual(point_meter_result.toString(7), point_berlin_4805_project.toString(7))
        point_berlin_25833 = QgsPointXY(389918.0748318382, 5819698.772194743)
        point_berlin_25833_project = point_berlin_25833.project(1, (math.pi / 2))
        length_meter_mapunits, point_meter_result = da_25833.measureLineProjected(point_berlin_25833, 1.0, (math.pi / 2))
        print('-I-> Berlin 25833 length_meter_mapunits[{}] point_meter_result[{}]'.format(QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_25833.lengthUnits(), True), point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_25833.lengthUnits(), True), '1.0000000 m')
        self.assertEqual(point_meter_result.toString(7), point_berlin_25833_project.toString(7))
        if da_5665.sourceCrs().authid() != "":
            point_berlin_5665 = QgsPointXY(3389996.871728864, 5822169.719727578)
            point_berlin_5665_project = point_berlin_5665.project(1, (math.pi / 2))
            length_meter_mapunits, point_meter_result = da_5665.measureLineProjected(point_berlin_5665, 1.0, (math.pi / 2))
            print('-I-> Berlin 5665 length_meter_mapunits[{}] point_meter_result[{}]'.format(QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_5665.lengthUnits(), True), point_meter_result.asWkt()))
            self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 1.0, da_5665.lengthUnits(), True), '1.0 m')
            self.assertEqual(point_meter_result.toString(7), point_berlin_5665_project.toString(7))
        print('\n12 points ''above over'' and on the Equator')
        point_wsg84 = QgsPointXY(25.7844, 71.1725)
        point_wsg84_project = QgsPointXY(25.78442775215388, 71.17249999999795)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Nordkap, Norway - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True), point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 7, QgsUnitTypes.DistanceDegrees, True), '0.0000278 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(24.95995, 60.16841)
        point_wsg84_project = QgsPointXY(24.95996801277454, 60.16840999999877)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Helsinki, Finnland - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True), point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True), '0.00001801 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(12.599278, 55.692861)
        point_wsg84_project = QgsPointXY(12.59929390161872, 55.69286099999897)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Copenhagen, Denmark - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True), point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True), '0.00001590 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))

        point_wsg84 = QgsPointXY(-0.001389, 51.477778)
        point_wsg84_project = QgsPointXY(-0.001374606184398, 51.4777779999991)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Royal Greenwich Observatory, United Kingdom - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True), point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True), '0.00001439 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(7.58769, 47.55814)
        point_wsg84_project = QgsPointXY(7.587703287209086, 47.55813999999922)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Basel, Switzerland - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True), point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True), '0.00001329 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(11.255278, 43.775278)
        point_wsg84_project = QgsPointXY(11.25529042107924, 43.77527799999933)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Florenz, Italy - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True), point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True), '0.00001242 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(14.514722, 35.899722)
        point_wsg84_project = QgsPointXY(14.51473307693308, 35.89972199999949)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Valletta, Malta - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True), point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True), '0.00001108 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(-79.933333, 32.783333)
        point_wsg84_project = QgsPointXY(-79.93332232547254, 32.78333299999955)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Charlston, South Carolina - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True), point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True), '0.00001067 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(-17.6666666, 27.733333)
        point_wsg84_project = QgsPointXY(-17.66665645831515, 27.73333299999962)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Ferro, Spain - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True), point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True), '0.00001014 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(-99.133333, 19.433333)
        point_wsg84_project = QgsPointXY(-99.1333234776827, 19.43333299999975)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Mexico City, Mexico - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True), point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True), '0.00000952 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(-79.894444, 9.341667)
        point_wsg84_project = QgsPointXY(-79.89443489691369, 9.341666999999882)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Colón, Panama - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True), point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True), '0.00000910 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(-74.075833, 4.598056)
        point_wsg84_project = QgsPointXY(-74.07582398803629, 4.598055999999943)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Bogotá, Colombia - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True), point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True), '0.00000901 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(0, 0)
        point_wsg84_project = QgsPointXY(0.000008983152841, 0)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Equator, Atlantic Ocean - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True), point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True), '0.00000898 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        print('\n12 points ''down under'' and 1 point that should be considered invalid')
        point_wsg84 = QgsPointXY(-78.509722, -0.218611)
        point_wsg84_project = QgsPointXY(-78.50971301678221, -0.218610999999997)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Quito, Ecuador - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True), point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True), '0.00000898 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(106.816667, -6.2)
        point_wsg84_project = QgsPointXY(106.8166760356519, -6.199999999999922)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Jakarta, Indonesia - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True), point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True), '0.00000904 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(-77.018611, -12.035)
        point_wsg84_project = QgsPointXY(-77.01860181630058, -12.03499999999985)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Lima, Peru - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True), point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True), '0.00000918 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(25.466667, -10.716667)
        point_wsg84_project = QgsPointXY(25.46667614155322, -10.71666699999986)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Kolwezi, Congo - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True), point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True), '0.00000914 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(-70.333333, -18.483333)
        point_wsg84_project = QgsPointXY(-70.3333235314429, -18.48333299999976)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Arica, Chile - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True), point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True), '0.00000947 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(-70.666667, -33.45)
        point_wsg84_project = QgsPointXY(-70.66665624452817, -33.44999999999953)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Santiago, Chile - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True), point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True), '0.00001076 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(144.9604, -37.8191)
        point_wsg84_project = QgsPointXY(144.96041135746983741, -37.81909999999945171)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Melbourne, Australia - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, da_wsg84.lengthUnits(), True), point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True), '0.00001136 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(147.29, -42.88)
        point_wsg84_project = QgsPointXY(147.2900122399815, -42.87999999999934)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Hobart City,Tasmania, Australia - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True), point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True), '0.00001224 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(168.101667, -46.899722)
        point_wsg84_project = QgsPointXY(168.101680123673, -46.89972199999923)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Ryan''s Creek Aerodrome, New Zealand - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True), point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True), '0.00001312 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(-69.216667, -51.633333)
        point_wsg84_project = QgsPointXY(-69.21665255700216, -51.6333329999991)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Río Gallegos, Argentina - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True), point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True), '0.00001444 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(-68.3, -54.8)
        point_wsg84_project = QgsPointXY(-68.29998445081456, -54.79999999999899)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Ushuaia, Tierra del Fuego, Argentina - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True), point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True), '0.00001555 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(-63.494444, -64.825278)
        point_wsg84_project = QgsPointXY(-63.49442294002932, -64.82527799999851)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Port Lockroy, Antarctica - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True), point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True), '0.00002106 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(-180, -84.863272250)
        point_wsg84_project = QgsPointXY(-179.9999000000025, -84.8632722499922)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-I-> Someware, Antarctica - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True), point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True), '0.00010000 deg')
        self.assertEqual(point_meter_result.toString(7), point_wsg84_project.toString(7))
        point_wsg84 = QgsPointXY(-180, -85.0511300)
        point_wsg84_project = QgsPointXY(-179.9998962142197, -85.05112999999191)
        length_meter_mapunits, point_meter_result = da_wsg84.measureLineProjected(point_wsg84, 1.0, (math.pi / 2))
        print('-W-> Mercator''s Last Stop, Antarctica - Wsg84 - length_meter_mapunits[{}] point_meter_result[{}] '.format(QgsDistanceArea.formatDistance(length_meter_mapunits, 7, da_wsg84.lengthUnits(), True), point_meter_result.asWkt()))
        self.assertEqual(QgsDistanceArea.formatDistance(length_meter_mapunits, 8, QgsUnitTypes.DistanceDegrees, True), '0.00010379 deg')
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
                QgsPointXY(0, 0), QgsPointXY(1, 0), QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 2), QgsPointXY(0, 2), QgsPointXY(0, 0),
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
                [[QgsPointXY(0, 0), QgsPointXY(1, 0), QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 2), QgsPointXY(0, 2), QgsPointXY(0, 0), ]],
                [[QgsPointXY(4, 0), QgsPointXY(5, 0), QgsPointXY(5, 2), QgsPointXY(3, 2), QgsPointXY(3, 1), QgsPointXY(4, 1), QgsPointXY(4, 0), ]]
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
        assert ((abs(distance - 2.23606797) < 0.00000001 and units == QgsUnitTypes.DistanceDegrees) or
                (abs(distance - 248.52) < 0.01 and units == QgsUnitTypes.DistanceMeters))

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
                QgsPointXY(0, 0), QgsPointXY(1, 0), QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 2), QgsPointXY(0, 2), QgsPointXY(0, 0),
            ]]
        )

        # We check both the measured area AND the units, in case the logic regarding
        # ellipsoids and units changes in future
        area = da.measureArea(polygon)
        units = da.areaUnits()

        print(("measured {} in {}".format(area, QgsUnitTypes.toString(units))))
        assert ((abs(area - 3.0) < 0.00000001 and units == QgsUnitTypes.AreaSquareDegrees) or
                (abs(area - 37176087091.5) < 0.1 and units == QgsUnitTypes.AreaSquareMeters))

        da.setEllipsoid("WGS84")
        area = da.measureArea(polygon)
        units = da.areaUnits()

        print(("measured {} in {}".format(area, QgsUnitTypes.toString(units))))
        # should always be in Meters Squared
        self.assertAlmostEqual(area, 37416879192.9, delta=0.1)
        self.assertEqual(units, QgsUnitTypes.AreaSquareMeters)

        # test converting the resultant area
        area = da.convertAreaMeasurement(area, QgsUnitTypes.AreaSquareMiles)
        self.assertAlmostEqual(area, 14446.7378, delta=0.001)

        # now try with a source CRS which is in feet
        polygon = QgsGeometry.fromPolygonXY(
            [[
                QgsPointXY(1850000, 4423000), QgsPointXY(1851000, 4423000), QgsPointXY(1851000, 4424000), QgsPointXY(1852000, 4424000), QgsPointXY(1852000, 4425000), QgsPointXY(1851000, 4425000), QgsPointXY(1850000, 4423000)
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
        self.assertAlmostEqual(area, 184149.37, delta=1.0)
        self.assertEqual(units, QgsUnitTypes.AreaSquareMeters)

        # test converting the resultant area
        area = da.convertAreaMeasurement(area, QgsUnitTypes.AreaSquareYards)
        self.assertAlmostEqual(area, 220240.8172549, delta=1.0)

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


if __name__ == '__main__':
    unittest.main()
