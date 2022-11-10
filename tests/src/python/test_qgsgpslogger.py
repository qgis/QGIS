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

from qgis.core import (
    QgsGpsLogger,
    QgsVectorLayer
)
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()

TEST_DATA_DIR = unitTestDataPath()


class TestQgsGpsLogger(unittest.TestCase):

    def test_setters(self):
        logger = QgsGpsLogger(None)
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

        logger.setPointTimeField('point_time_field')
        self.assertEqual(logger.pointTimeField(), 'point_time_field')

        logger.setPointDistanceFromPreviousField('point_distance_from_previous_field')
        self.assertEqual(logger.pointDistanceFromPreviousField(), 'point_distance_from_previous_field')

        logger.setPointTimeDeltaFromPreviousField('point_delta_from_previous_field')
        self.assertEqual(logger.pointTimeDeltaFromPreviousField(), 'point_delta_from_previous_field')

        logger.setTrackStartTimeField('track_start_time')
        self.assertEqual(logger.trackStartTimeField(), 'track_start_time')

        logger.setTrackEndTimeField('track_end_time')
        self.assertEqual(logger.trackEndTimeField(), 'track_end_time')

        logger.setTrackLengthField('track_length')
        self.assertEqual(logger.trackLengthField(), 'track_length')


if __name__ == '__main__':
    unittest.main()
