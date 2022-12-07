# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsGpsLogger.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '10/11/2022'
__copyright__ = 'Copyright 2022, The QGIS Project'

import qgis  # NOQA
from qgis.PyQt.QtCore import (
    Qt,
    QBuffer,
    QDateTime,
    QCoreApplication
)
from qgis.PyQt.QtTest import QSignalSpy
from qgis.core import (
    Qgis,
    QgsGpsLogger,
    QgsVectorLayerGpsLogger,
    QgsVectorLayer,
    QgsNmeaConnection,
    QgsSettings,
    QgsWkbTypes,
    NULL
)
from qgis.testing import start_app, unittest

from utilities import unitTestDataPath

start_app()

TEST_DATA_DIR = unitTestDataPath()


class GpsReplay(QgsNmeaConnection):

    def __init__(self):
        self.buffer = QBuffer()
        self.buffer.open(QBuffer.ReadWrite)

        super().__init__(self.buffer)
        assert self.connect()

    def send_message(self, message):
        msg = (message + '\r\n').encode()

        spy = QSignalSpy(self.stateChanged)

        pos = self.buffer.pos()
        self.buffer.write(msg)
        self.buffer.seek(pos)

        spy.wait()


class TestQgsGpsLogger(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""

        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("TestQgsGpsLogger.com")
        QCoreApplication.setApplicationName("TestQgsGpsLogger")
        QgsSettings().clear()

        start_app()

        settings = QgsSettings()
        settings.setValue('/gps/leap-seconds', 48)
        settings.setValue('/gps/timestamp-offset-from-utc', 3000)
        settings.setValue('/gps/timestamp-time-spec', 'OffsetFromUTC')

    def test_setters(self):
        logger = QgsVectorLayerGpsLogger(None)
        self.assertIsNone(logger.tracksLayer())
        self.assertIsNone(logger.pointsLayer())

        tracks_layer = QgsVectorLayer('LineString', 'tracks', 'memory')
        self.assertTrue(tracks_layer.isValid())
        logger.setTracksLayer(tracks_layer)

        points_layer = QgsVectorLayer('Point', 'points', 'memory')
        self.assertTrue(points_layer.isValid())
        logger.setPointsLayer(points_layer)

        self.assertEqual(logger.pointsLayer(), points_layer)
        self.assertEqual(logger.tracksLayer(), tracks_layer)

        logger.setDestinationField(Qgis.GpsInformationComponent.Timestamp, 'point_time_field')
        self.assertEqual(logger.destinationField(Qgis.GpsInformationComponent.Timestamp), 'point_time_field')

        logger.setDestinationField(Qgis.GpsInformationComponent.TrackDistanceSinceLastPoint, 'point_distance_from_previous_field')
        self.assertEqual(logger.destinationField(Qgis.GpsInformationComponent.TrackDistanceSinceLastPoint), 'point_distance_from_previous_field')

        logger.setDestinationField(Qgis.GpsInformationComponent.TrackTimeSinceLastPoint, 'point_delta_from_previous_field')
        self.assertEqual(logger.destinationField(Qgis.GpsInformationComponent.TrackTimeSinceLastPoint), 'point_delta_from_previous_field')

        logger.setDestinationField(Qgis.GpsInformationComponent.TrackStartTime, 'track_start_time')
        self.assertEqual(logger.destinationField(Qgis.GpsInformationComponent.TrackStartTime), 'track_start_time')

        logger.setDestinationField(Qgis.GpsInformationComponent.TrackStartTime, 'track_end_time')
        self.assertEqual(logger.destinationField(Qgis.GpsInformationComponent.TrackStartTime), 'track_end_time')

        logger.setDestinationField(Qgis.GpsInformationComponent.TotalTrackLength, 'track_length')
        self.assertEqual(logger.destinationField(Qgis.GpsInformationComponent.TotalTrackLength), 'track_length')

    def test_current_geometry(self):
        gps_connection = GpsReplay()

        logger = QgsGpsLogger(gps_connection)
        spy = QSignalSpy(logger.stateChanged)

        gps_connection.send_message('$GPRMC,084111.185,A,6938.6531,N,01856.8527,E,0.16,2.00,220120,,,A*6E')
        self.assertEqual(len(spy), 1)

        # not enough vertices for these types yet:
        res, err = logger.currentGeometry(QgsWkbTypes.LineString)
        self.assertTrue(err)
        res, err = logger.currentGeometry(QgsWkbTypes.LineStringZ)
        self.assertTrue(err)
        res, err = logger.currentGeometry(QgsWkbTypes.Polygon)
        self.assertTrue(err)
        res, err = logger.currentGeometry(QgsWkbTypes.PolygonZ)
        self.assertTrue(err)

        # can create points
        res, err = logger.currentGeometry(QgsWkbTypes.Point)
        self.assertFalse(err)
        self.assertEqual(res.asWkt(4), 'Point (18.9475 69.6442)')
        res, err = logger.currentGeometry(QgsWkbTypes.PointZ)
        self.assertFalse(err)
        self.assertEqual(res.asWkt(4), 'PointZ (18.9475 69.6442 0)')

        # make elevation available
        gps_connection.send_message("$GPGGA,084112.185,6939.6532,N,01856.8526,E,1,04,1.4,35.0,M,29.4,M,,0000*63")
        res, err = logger.currentGeometry(QgsWkbTypes.PointZ)
        self.assertFalse(err)
        self.assertEqual(res.asWkt(4), 'PointZ (18.9475 69.6609 35)')
        res, err = logger.currentGeometry(QgsWkbTypes.MultiPointZ)
        self.assertFalse(err)
        self.assertEqual(res.asWkt(4), 'MultiPointZ ((18.9475 69.6609 35))')

        res, err = logger.currentGeometry(QgsWkbTypes.LineString)
        self.assertFalse(err)
        self.assertEqual(res.asWkt(4), 'LineString (18.9475 69.6442, 18.9475 69.6609)')
        res, err = logger.currentGeometry(QgsWkbTypes.LineStringZ)
        self.assertFalse(err)
        self.assertEqual(res.asWkt(4), 'LineStringZ (18.9475 69.6442 0, 18.9475 69.6609 35)')
        res, err = logger.currentGeometry(QgsWkbTypes.MultiLineStringZ)
        self.assertFalse(err)
        self.assertEqual(res.asWkt(4), 'MultiLineStringZ ((18.9475 69.6442 0, 18.9475 69.6609 35))')

        # note enough vertices for polygons yet
        res, err = logger.currentGeometry(QgsWkbTypes.Polygon)
        self.assertTrue(err)
        res, err = logger.currentGeometry(QgsWkbTypes.PolygonZ)
        self.assertTrue(err)

        gps_connection.send_message("$GPGGA,084112.185,6939.6532,N,01866.8526,E,1,04,1.4,35.0,M,29.4,M,,0000*63")

        res, err = logger.currentGeometry(QgsWkbTypes.PointZ)
        self.assertFalse(err)
        self.assertEqual(res.asWkt(4), 'PointZ (19.1142 69.6609 35)')
        res, err = logger.currentGeometry(QgsWkbTypes.MultiPointZ)
        self.assertFalse(err)
        self.assertEqual(res.asWkt(4), 'MultiPointZ ((19.1142 69.6609 35))')

        res, err = logger.currentGeometry(QgsWkbTypes.LineString)
        self.assertFalse(err)
        self.assertEqual(res.asWkt(4), 'LineString (18.9475 69.6442, 18.9475 69.6609, 19.1142 69.6609)')
        res, err = logger.currentGeometry(QgsWkbTypes.LineStringZ)
        self.assertFalse(err)
        self.assertEqual(res.asWkt(4), 'LineStringZ (18.9475 69.6442 0, 18.9475 69.6609 35, 19.1142 69.6609 35)')
        res, err = logger.currentGeometry(QgsWkbTypes.MultiLineStringZ)
        self.assertFalse(err)
        self.assertEqual(res.asWkt(4), 'MultiLineStringZ ((18.9475 69.6442 0, 18.9475 69.6609 35, 19.1142 69.6609 35))')

        res, err = logger.currentGeometry(QgsWkbTypes.Polygon)
        self.assertFalse(err)
        self.assertEqual(res.asWkt(4), 'Polygon ((18.9475 69.6442, 18.9475 69.6609, 19.1142 69.6609, 18.9475 69.6442))')
        res, err = logger.currentGeometry(QgsWkbTypes.PolygonZ)
        self.assertFalse(err)
        self.assertEqual(res.asWkt(4), 'PolygonZ ((18.9475 69.6442 0, 18.9475 69.6609 35, 19.1142 69.6609 35, 18.9475 69.6442 0))')
        res, err = logger.currentGeometry(QgsWkbTypes.MultiPolygonZ)
        self.assertFalse(err)
        self.assertEqual(res.asWkt(4), 'MultiPolygonZ (((18.9475 69.6442 0, 18.9475 69.6609 35, 19.1142 69.6609 35, 18.9475 69.6442 0)))')

    def test_point_recording(self):
        points_layer = QgsVectorLayer(
            'PointZ?crs=EPSG:28355&field=timestamp:datetime&field=distance:double&field=seconds:double', 'points',
            'memory')
        self.assertTrue(points_layer.isValid())
        self.assertEqual(points_layer.crs().authid(), 'EPSG:28355')

        gps_connection = GpsReplay()

        logger = QgsVectorLayerGpsLogger(gps_connection)
        logger.setPointsLayer(points_layer)
        spy = QSignalSpy(logger.stateChanged)

        logger.setDestinationField(Qgis.GpsInformationComponent.Timestamp, 'timestamp')
        logger.setDestinationField(Qgis.GpsInformationComponent.TrackDistanceSinceLastPoint, 'distance')
        logger.setDestinationField(Qgis.GpsInformationComponent.TrackTimeSinceLastPoint, 'seconds')

        points_layer.startEditing()

        gps_connection.send_message('$GPRMC,084111.185,A,6938.6531,N,01856.8527,E,0.16,2.00,220120,,,A*6E')
        self.assertEqual(len(spy), 1)

        self.assertEqual(points_layer.featureCount(), 1)
        f = next(points_layer.getFeatures())
        exp = QDateTime(2020, 1, 22, 9, 31, 59, 185)
        exp.setOffsetFromUtc(3000)
        self.assertEqual(f.attributes(), [exp, None, None])
        self.assertEqual(f.geometry().asWkt(-3), 'PointZ (-1297000 21436000 0)')

        gps_connection.send_message(
            '$GPRMC,084113.185,A,6938.9152,N,01856.8526,E,0.05,2.00,220120,,,A*6C')
        self.assertEqual(len(spy), 2)

        self.assertEqual(points_layer.featureCount(), 2)
        features = points_layer.getFeatures()
        f = next(features)
        exp = QDateTime(2020, 1, 22, 9, 31, 59, 185)
        exp.setOffsetFromUtc(3000)
        self.assertEqual(f.attributes(), [exp, None, None])
        f = next(features)
        exp = QDateTime(2020, 1, 22, 9, 32, 1, 185)
        exp.setOffsetFromUtc(3000)
        self.assertEqual(f.attributes(), [exp, 0.004368333651276768, 2.0])
        self.assertEqual(f.geometry().asWkt(-3), 'PointZ (-1297000 21435000 0)')

        gps_connection.send_message(
            '$GPRMC,084117.185,A,6939.3152,N,01856.8526,E,0.05,2.00,220120,,,A*6C')
        self.assertEqual(len(spy), 3)

        self.assertEqual(points_layer.featureCount(), 3)
        features = points_layer.getFeatures()
        f = next(features)
        exp = QDateTime(2020, 1, 22, 9, 31, 59, 185)
        exp.setOffsetFromUtc(3000)
        self.assertEqual(f.attributes(), [exp, None, None])
        f = next(features)
        exp = QDateTime(2020, 1, 22, 9, 32, 1, 185)
        exp.setOffsetFromUtc(3000)
        self.assertEqual(f.attributes(), [exp, 0.004368333651276768, 2.0])
        f = next(features)
        exp = QDateTime(2020, 1, 22, 9, 32, 5, 185)
        exp.setOffsetFromUtc(3000)
        self.assertEqual(f.attributes(), [exp, 0.006666666666660603, 4.0])
        self.assertEqual(f.geometry().asWkt(-3), 'PointZ (-1296000 21435000 0)')

        # stop recording distance
        logger.setDestinationField(Qgis.GpsInformationComponent.TrackDistanceSinceLastPoint, None)

        gps_connection.send_message(
            '$GPRMC,084118.185,A,6939.1152,N,01856.8526,E,0.05,2.00,220120,,,A*6C')
        self.assertEqual(len(spy), 4)

        self.assertEqual(points_layer.featureCount(), 4)
        features = points_layer.getFeatures()
        f = next(features)
        exp = QDateTime(2020, 1, 22, 9, 31, 59, 185)
        exp.setOffsetFromUtc(3000)
        self.assertEqual(f.attributes(), [exp, None, None])
        f = next(features)
        exp = QDateTime(2020, 1, 22, 9, 32, 1, 185)
        exp.setOffsetFromUtc(3000)
        self.assertEqual(f.attributes(), [exp, 0.004368333651276768, 2.0])
        f = next(features)
        exp = QDateTime(2020, 1, 22, 9, 32, 5, 185)
        exp.setOffsetFromUtc(3000)
        self.assertEqual(f.attributes(), [exp, 0.006666666666660603, 4.0])
        f = next(features)
        exp = QDateTime(2020, 1, 22, 9, 32, 6, 185)
        exp.setOffsetFromUtc(3000)
        self.assertEqual(f.attributes(), [exp, NULL, 1.0])
        self.assertEqual(f.geometry().asWkt(-3), 'PointZ (-1297000 21435000 0)')

        # stop recording time since previous
        logger.setDestinationField(Qgis.GpsInformationComponent.TrackTimeSinceLastPoint, None)

        gps_connection.send_message(
            '$GPRMC,084119.185,A,6939.3152,N,01856.8526,E,0.05,2.00,220120,,,A*6C')
        self.assertEqual(len(spy), 5)

        self.assertEqual(points_layer.featureCount(), 5)
        features = points_layer.getFeatures()
        f = next(features)
        exp = QDateTime(2020, 1, 22, 9, 31, 59, 185)
        exp.setOffsetFromUtc(3000)
        self.assertEqual(f.attributes(), [exp, None, None])
        f = next(features)
        exp = QDateTime(2020, 1, 22, 9, 32, 1, 185)
        exp.setOffsetFromUtc(3000)
        self.assertEqual(f.attributes(), [exp, 0.004368333651276768, 2.0])
        f = next(features)
        exp = QDateTime(2020, 1, 22, 9, 32, 5, 185)
        exp.setOffsetFromUtc(3000)
        self.assertEqual(f.attributes(), [exp, 0.006666666666660603, 4.0])
        f = next(features)
        exp = QDateTime(2020, 1, 22, 9, 32, 6, 185)
        exp.setOffsetFromUtc(3000)
        self.assertEqual(f.attributes(), [exp, NULL, 1.0])
        f = next(features)
        exp = QDateTime(2020, 1, 22, 9, 32, 7, 185)
        exp.setOffsetFromUtc(3000)
        self.assertEqual(f.attributes(), [exp, NULL, NULL])
        self.assertEqual(f.geometry().asWkt(-3), 'PointZ (-1296000 21435000 0)')

        # stop recording timestamp
        logger.setDestinationField(Qgis.GpsInformationComponent.Timestamp, None)

        gps_connection.send_message(
            '$GPRMC,084120.185,A,6939.4152,N,01856.8526,E,0.05,2.00,220120,,,A*6C')
        self.assertEqual(len(spy), 6)

        self.assertEqual(points_layer.featureCount(), 6)
        features = points_layer.getFeatures()
        f = next(features)
        exp = QDateTime(2020, 1, 22, 9, 31, 59, 185)
        exp.setOffsetFromUtc(3000)
        self.assertEqual(f.attributes(), [exp, None, None])
        f = next(features)
        exp = QDateTime(2020, 1, 22, 9, 32, 1, 185)
        exp.setOffsetFromUtc(3000)
        self.assertEqual(f.attributes(), [exp, 0.004368333651276768, 2.0])
        f = next(features)
        exp = QDateTime(2020, 1, 22, 9, 32, 5, 185)
        exp.setOffsetFromUtc(3000)
        self.assertEqual(f.attributes(), [exp, 0.006666666666660603, 4.0])
        f = next(features)
        exp = QDateTime(2020, 1, 22, 9, 32, 6, 185)
        exp.setOffsetFromUtc(3000)
        self.assertEqual(f.attributes(), [exp, NULL, 1.0])
        f = next(features)
        exp = QDateTime(2020, 1, 22, 9, 32, 7, 185)
        exp.setOffsetFromUtc(3000)
        self.assertEqual(f.attributes(), [exp, NULL, NULL])
        f = next(features)
        self.assertEqual(f.attributes(), [NULL, NULL, NULL])
        self.assertEqual(f.geometry().asWkt(-3), 'PointZ (-1296000 21435000 0)')

        # points should be written to layer's edit buffer
        self.assertTrue(logger.writeToEditBuffer())
        self.assertEqual(points_layer.dataProvider().featureCount(), 0)

        # write direct to data provider
        logger.setWriteToEditBuffer(False)
        self.assertFalse(logger.writeToEditBuffer())

        gps_connection.send_message(
            '$GPRMC,084129.185,A,6939.4152,N,01856.8526,E,0.05,2.00,220120,,,A*6C')
        self.assertEqual(points_layer.dataProvider().featureCount(), 1)

    def test_point_recording_no_z(self):
        points_layer = QgsVectorLayer(
            'Point?crs=EPSG:28355&field=timestamp:datetime&field=distance:double&field=seconds:double', 'points',
            'memory')
        self.assertTrue(points_layer.isValid())
        self.assertEqual(points_layer.crs().authid(), 'EPSG:28355')

        gps_connection = GpsReplay()

        logger = QgsVectorLayerGpsLogger(gps_connection)
        logger.setPointsLayer(points_layer)
        spy = QSignalSpy(logger.stateChanged)

        logger.setDestinationField(Qgis.GpsInformationComponent.Timestamp, 'timestamp')
        logger.setDestinationField(Qgis.GpsInformationComponent.TrackDistanceSinceLastPoint, 'distance')
        logger.setDestinationField(Qgis.GpsInformationComponent.TrackTimeSinceLastPoint, 'seconds')

        points_layer.startEditing()

        gps_connection.send_message('$GPRMC,084111.185,A,6938.6531,N,01856.8527,E,0.16,2.00,220120,,,A*6E')
        self.assertEqual(len(spy), 1)

        self.assertEqual(points_layer.featureCount(), 1)
        f = next(points_layer.getFeatures())
        exp = QDateTime(2020, 1, 22, 9, 31, 59, 185)
        exp.setOffsetFromUtc(3000)
        self.assertEqual(f.attributes(), [exp, None, None])
        # should be no z value in geometry, to match layer dimensionality
        self.assertEqual(f.geometry().asWkt(-3), 'Point (-1297000 21436000)')

    def test_point_recording_m_value_epoch(self):
        points_layer = QgsVectorLayer(
            'PointM?crs=EPSG:28355&field=timestamp:datetime&field=distance:double&field=seconds:double', 'points',
            'memory')
        self.assertTrue(points_layer.isValid())
        self.assertEqual(points_layer.crs().authid(), 'EPSG:28355')

        QgsSettings().setValue('gps/store-attribute-in-m-values', True)
        QgsSettings().setValue('gps/m-value-attribute', 'Timestamp')

        gps_connection = GpsReplay()

        logger = QgsVectorLayerGpsLogger(gps_connection)
        logger.setPointsLayer(points_layer)
        spy = QSignalSpy(logger.stateChanged)
        points_layer.startEditing()

        gps_connection.send_message('$GPRMC,084111.185,A,6938.6531,N,01856.8527,E,0.16,2.00,220120,,,A*6E')
        self.assertEqual(len(spy), 1)

        self.assertEqual(logger.lastMValue(), 1579682471185.0)

        self.assertEqual(points_layer.featureCount(), 1)
        f = next(points_layer.getFeatures())
        exp = QDateTime(2020, 1, 22, 9, 31, 59, 185)
        exp.setOffsetFromUtc(3000)
        self.assertEqual(f.geometry().asWkt(-3), 'PointM (-1297000 21436000 1579682471000)')

        gps_connection.send_message(
            '$GPRMC,084113.185,A,6938.9152,N,01856.8526,E,0.05,2.00,220120,,,A*6C')
        self.assertEqual(len(spy), 2)
        self.assertEqual(logger.lastMValue(), 1579682473185.)

        self.assertEqual(points_layer.featureCount(), 2)
        features = points_layer.getFeatures()
        next(features)
        f = next(features)
        self.assertEqual(f.geometry().asWkt(-3), 'PointM (-1297000 21435000 1579682473000)')

        gps_connection.send_message(
            '$GPRMC,084117.185,A,6939.3152,N,01856.8526,E,0.05,Z2.00,220120,,,A*6C')
        self.assertEqual(len(spy), 3)
        self.assertEqual(logger.lastMValue(), 1579682477185.0)

        self.assertEqual(points_layer.featureCount(), 3)
        features = points_layer.getFeatures()
        next(features)
        next(features)
        f = next(features)
        self.assertEqual(f.geometry().asWkt(-3), 'PointM (-1296000 21435000 1579682477000)')
        QgsSettings().setValue('gps/store-attribute-in-m-values', False)

    def test_point_recording_m_value_altitude(self):
        points_layer = QgsVectorLayer(
            'PointZM?crs=EPSG:28355&field=timestamp:datetime&field=distance:double&field=seconds:double',
            'points',
            'memory')
        self.assertTrue(points_layer.isValid())
        self.assertEqual(points_layer.crs().authid(), 'EPSG:28355')
        self.assertEqual(points_layer.wkbType(), QgsWkbTypes.PointZM)

        QgsSettings().setValue('gps/store-attribute-in-m-values', True)
        QgsSettings().setValue('gps/m-value-attribute', 'Altitude')

        gps_connection = GpsReplay()

        logger = QgsVectorLayerGpsLogger(gps_connection)
        logger.setPointsLayer(points_layer)
        spy = QSignalSpy(logger.stateChanged)
        points_layer.startEditing()

        gps_connection.send_message('$GPRMC,084111.185,A,6938.6531,N,01856.8527,E,0.16,2.00,220120,,,A*6E')
        self.assertEqual(len(spy), 1)
        self.assertEqual(logger.lastMValue(), 0)

        self.assertEqual(points_layer.featureCount(), 1)
        f = next(points_layer.getFeatures())
        exp = QDateTime(2020, 1, 22, 9, 31, 59, 185)
        exp.setOffsetFromUtc(3000)
        self.assertEqual(f.geometry().asWkt(-3), 'PointZM (-1297000 21436000 0 0)')

        gps_connection.send_message("$GPGGA,084112.185,6938.9152,N,01856.8526,E,1,04,1.4,3335.0,M,29.4,M,,0000*63")
        self.assertEqual(len(spy), 2)
        self.assertEqual(logger.lastMValue(), 3335.0)

        self.assertEqual(points_layer.featureCount(), 2)
        features = points_layer.getFeatures()
        f = next(features)
        self.assertEqual(f.geometry().asWkt(-3), 'PointZM (-1297000 21436000 0 0)')
        f = next(features)
        self.assertEqual(f.geometry().asWkt(-3), 'PointZM (-1297000 21435000 3000 3000)')
        QgsSettings().setValue('gps/store-attribute-in-m-values', False)

    def test_track_recording(self):
        line_layer = QgsVectorLayer(
            'LineStringZ?crs=EPSG:28355&field=start:datetime&field=end:string&field=length:double', 'lines', 'memory')
        self.assertTrue(line_layer.isValid())
        self.assertEqual(line_layer.crs().authid(), 'EPSG:28355')

        gps_connection = GpsReplay()

        logger = QgsVectorLayerGpsLogger(gps_connection)
        logger.setTracksLayer(line_layer)
        spy = QSignalSpy(logger.stateChanged)

        logger.setDestinationField(Qgis.GpsInformationComponent.TotalTrackLength, 'length')
        logger.setDestinationField(Qgis.GpsInformationComponent.TrackStartTime, 'start')
        logger.setDestinationField(Qgis.GpsInformationComponent.TrackEndTime, 'end')

        line_layer.startEditing()

        gps_connection.send_message('$GPRMC,084111.185,A,6938.6531,N,01856.8527,E,0.16,2.00,220120,,,A*6E')
        self.assertEqual(len(spy), 1)

        # should be no features until track is ended
        self.assertEqual(line_layer.featureCount(), 0)

        gps_connection.send_message(
            '$GPRMC,084113.185,A,6938.9152,N,01856.8526,E,0.05,2.00,220120,,,A*6C')
        self.assertEqual(len(spy), 2)

        gps_connection.send_message(
            '$GPRMC,084118.185,A,6938.9152,N,01857.8526,E,0.05,2.00,220120,,,A*6C')
        self.assertEqual(len(spy), 3)

        self.assertEqual(line_layer.featureCount(), 0)

        logger.endCurrentTrack()

        self.assertEqual(line_layer.featureCount(), 1)
        f = next(line_layer.getFeatures())
        exp = QDateTime(2020, 1, 22, 9, 31, 59, 185)
        exp.setOffsetFromUtc(3000)
        self.assertEqual(f.attributes(),
                         [exp, '2020-01-22T09:32:06+00:50',
                          0.021035000317942486])
        self.assertEqual(f.geometry().asWkt(-2),
                         'LineStringZ (-1297400 21435500 0, -1297000 21435200 0, -1297400 21434700 0)')

        self.assertFalse(logger.currentTrack())

        # points should be written to layer's edit buffer
        self.assertEqual(line_layer.dataProvider().featureCount(), 0)

        # write direct to data provider
        logger.setWriteToEditBuffer(False)
        self.assertFalse(logger.writeToEditBuffer())

        gps_connection.send_message(
            '$GPRMC,084129.185,A,6939.4152,N,01856.8526,E,0.05,2.00,220120,,,A*6C')
        gps_connection.send_message(
            '$GPRMC,084129.185,A,6939.4152,N,01956.8526,E,0.05,2.00,220120,,,A*6C')
        logger.endCurrentTrack()
        self.assertEqual(line_layer.dataProvider().featureCount(), 1)

    def test_track_recording_no_z(self):
        line_layer = QgsVectorLayer(
            'LineString?crs=EPSG:28355&field=start:datetime&field=end:string&field=length:double', 'lines', 'memory')
        self.assertTrue(line_layer.isValid())
        self.assertEqual(line_layer.crs().authid(), 'EPSG:28355')

        gps_connection = GpsReplay()

        logger = QgsVectorLayerGpsLogger(gps_connection)
        logger.setTracksLayer(line_layer)
        spy = QSignalSpy(logger.stateChanged)

        logger.setDestinationField(Qgis.GpsInformationComponent.TotalTrackLength, 'length')
        logger.setDestinationField(Qgis.GpsInformationComponent.TrackStartTime, 'start')
        logger.setDestinationField(Qgis.GpsInformationComponent.TrackEndTime, 'end')

        line_layer.startEditing()

        gps_connection.send_message('$GPRMC,084111.185,A,6938.6531,N,01856.8527,E,0.16,2.00,220120,,,A*6E')
        self.assertEqual(len(spy), 1)

        # should be no features until track is ended
        self.assertEqual(line_layer.featureCount(), 0)

        gps_connection.send_message(
            '$GPRMC,084113.185,A,6938.9152,N,01856.8526,E,0.05,2.00,220120,,,A*6C')
        self.assertEqual(len(spy), 2)

        gps_connection.send_message(
            '$GPRMC,084118.185,A,6938.9152,N,01857.8526,E,0.05,2.00,220120,,,A*6C')
        self.assertEqual(len(spy), 3)

        self.assertEqual(line_layer.featureCount(), 0)

        logger.endCurrentTrack()

        self.assertEqual(line_layer.featureCount(), 1)
        f = next(line_layer.getFeatures())
        exp = QDateTime(2020, 1, 22, 9, 31, 59, 185)
        exp.setOffsetFromUtc(3000)
        self.assertEqual(f.attributes(),
                         [exp, '2020-01-22T09:32:06+00:50',
                          0.021035000317942486])
        self.assertEqual(f.geometry().asWkt(-2),
                         'LineString (-1297400 21435500, -1297000 21435200, -1297400 21434700)')

    def test_track_recording_z_m_timestamp(self):
        line_layer = QgsVectorLayer(
            'LineStringZM?crs=EPSG:28355&field=start:datetime&field=end:string&field=length:double', 'lines', 'memory')
        self.assertTrue(line_layer.isValid())
        self.assertEqual(line_layer.crs().authid(), 'EPSG:28355')

        QgsSettings().setValue('gps/store-attribute-in-m-values', True)
        QgsSettings().setValue('gps/m-value-attribute', 'Timestamp')

        gps_connection = GpsReplay()

        logger = QgsVectorLayerGpsLogger(gps_connection)
        logger.setTracksLayer(line_layer)
        spy = QSignalSpy(logger.stateChanged)

        logger.setDestinationField(Qgis.GpsInformationComponent.TotalTrackLength, 'length')
        logger.setDestinationField(Qgis.GpsInformationComponent.TrackStartTime, 'start')
        logger.setDestinationField(Qgis.GpsInformationComponent.TrackEndTime, 'end')

        line_layer.startEditing()

        gps_connection.send_message('$GPRMC,084111.185,A,6938.6531,N,01856.8527,E,0.16,2.00,220120,,,A*6E')
        self.assertEqual(len(spy), 1)

        # should be no features until track is ended
        self.assertEqual(line_layer.featureCount(), 0)

        gps_connection.send_message(
            '$GPRMC,084113.185,A,6938.9152,N,01856.8526,E,0.05,2.00,220120,,,A*6C')
        self.assertEqual(len(spy), 2)

        gps_connection.send_message(
            '$GPRMC,084118.185,A,6938.9152,N,01857.8526,E,0.05,2.00,220120,,,A*6C')
        self.assertEqual(len(spy), 3)

        self.assertEqual(line_layer.featureCount(), 0)

        logger.endCurrentTrack()

        self.assertEqual(line_layer.featureCount(), 1)
        f = next(line_layer.getFeatures())
        exp = QDateTime(2020, 1, 22, 9, 31, 59, 185)
        exp.setOffsetFromUtc(3000)
        self.assertEqual(f.attributes(),
                         [exp, '2020-01-22T09:32:06+00:50',
                          0.021035000317942486])
        self.assertEqual(f.geometry().asWkt(-2),
                         'LineStringZM (-1297400 21435500 0 1579682471200, -1297000 21435200 0 1579682473200, -1297400 21434700 0 1579682478200)')


if __name__ == '__main__':
    unittest.main()
