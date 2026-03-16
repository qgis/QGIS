"""QGIS Unit tests for QgsVideoExporter

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import os
import tempfile
import unittest

from qgis.core import QgsFeedback, QgsVideoExporter
from qgis.PyQt.QtCore import QT_VERSION, QSize, QUrl
from qgis.PyQt.QtMultimedia import (
    QMediaFormat,
    QMediaPlayer,
    QMediaRecorder,
    QVideoSink,
)
from qgis.PyQt.QtTest import QSignalSpy
from qgis.testing import QgisTestCase, start_app

start_app()


class TestQgsVideoExporter(QgisTestCase):
    @classmethod
    def control_path_prefix(cls):
        return "video"

    @unittest.skipIf(QT_VERSION < 0x060800, "Requires Qt 6.8+")
    def testSimple(self):
        exporter = QgsVideoExporter("test.avi", QSize(1080, 600), 30)
        exporter.setInputFiles(["frame1.png", "frame2.png"])
        self.assertEqual(exporter.inputFiles(), ["frame1.png", "frame2.png"])

        exporter.setFileFormat(QMediaFormat.FileFormat.AVI)
        self.assertEqual(exporter.fileFormat(), QMediaFormat.FileFormat.AVI)

        exporter.setVideoCodec(QMediaFormat.VideoCodec.MPEG4)
        self.assertEqual(exporter.videoCodec(), QMediaFormat.VideoCodec.MPEG4)

    def test_set_input_files_by_pattern(self):
        exporter = QgsVideoExporter("test.mp4", QSize(1280, 720), 1)
        exporter.setInputFilesByPattern(
            self.get_test_data_path("raster/osm_tiles/6/34/").as_posix(), "*.png"
        )
        self.assertEqual(
            exporter.inputFiles(),
            [
                self.get_test_data_path("raster/osm_tiles/6/34/19.png").as_posix(),
                self.get_test_data_path("raster/osm_tiles/6/34/20.png").as_posix(),
                self.get_test_data_path("raster/osm_tiles/6/34/21.png").as_posix(),
                self.get_test_data_path("raster/osm_tiles/6/34/22.png").as_posix(),
                self.get_test_data_path("raster/osm_tiles/6/34/23.png").as_posix(),
            ],
        )

        exporter.setInputFilesByPattern(
            self.get_test_data_path("raster/osm_tiles/6/34/").as_posix(), "2*.png"
        )
        self.assertEqual(
            exporter.inputFiles(),
            [
                self.get_test_data_path("raster/osm_tiles/6/34/20.png").as_posix(),
                self.get_test_data_path("raster/osm_tiles/6/34/21.png").as_posix(),
                self.get_test_data_path("raster/osm_tiles/6/34/22.png").as_posix(),
                self.get_test_data_path("raster/osm_tiles/6/34/23.png").as_posix(),
            ],
        )

    @unittest.skipIf(QT_VERSION < 0x060800, "Requires Qt 6.8+")
    def test_write(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            export_file = os.path.join(temp_dir, "my_video.mp4")

            exporter = QgsVideoExporter(export_file, QSize(1280, 720), 1)
            exporter.setInputFiles(
                [
                    self.get_test_data_path(
                        "control_images/video/expected_frame1/expected_frame1.png"
                    ).as_posix(),
                    self.get_test_data_path(
                        "control_images/video/expected_frame2/expected_frame2.png"
                    ).as_posix(),
                    self.get_test_data_path(
                        "control_images/video/expected_frame3/expected_frame3.png"
                    ).as_posix(),
                ]
            )
            # use an old format/codec so we don't have to worry about patents and widespread
            # availability of encoder
            exporter.setFileFormat(QMediaFormat.FileFormat.MPEG4)
            exporter.setVideoCodec(QMediaFormat.VideoCodec.H264)

            feedback = QgsFeedback()
            exporter.setFeedback(feedback)

            spy = QSignalSpy(exporter.finished)
            exporter.writeVideo()
            spy.wait()
            self.assertFalse(exporter.errorString())
            self.assertEqual(exporter.error(), QMediaRecorder.Error.NoError)

            self.assertEqual(len(spy), 1)

            player = QMediaPlayer()
            sink = QVideoSink()
            player.setVideoOutput(sink)
            player.setSource(QUrl.fromLocalFile(export_file))

            media_status_spy = QSignalSpy(player.mediaStatusChanged)

            if player.mediaStatus() != QMediaPlayer.MediaStatus.LoadedMedia:
                media_status_spy.wait()

            self.assertEqual(player.mediaStatus(), QMediaPlayer.MediaStatus.LoadedMedia)

            self.assertFalse(player.errorString())
            self.assertEqual(player.error(), QMediaPlayer.Error.NoError)
            self.assertEqual(player.duration(), 3000)
            self.assertTrue(player.hasVideo())
            self.assertEqual(feedback.progress(), 100)

            frame_images = []

            def process_frame(frame):
                nonlocal frame_images
                frame_images.append(frame.toImage())

            sink.videoFrameChanged.connect(process_frame)
            player.play()
            while player.mediaStatus() != QMediaPlayer.MediaStatus.EndOfMedia:
                spy.wait()

            self.assertGreaterEqual(len(frame_images), 3)

            self.assertTrue(self.image_check("frame1", "frame1", frame_images[0]))

            self.assertTrue(self.image_check("frame2", "frame2", frame_images[1]))

            self.assertTrue(self.image_check("frame3", "frame3", frame_images[2]))

    @unittest.skipIf(QT_VERSION < 0x060800, "Requires Qt 6.8+")
    def test_failure(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            export_file = os.path.join(temp_dir, "my_video.mp4")

            # use an invalid size to trigger an error
            exporter = QgsVideoExporter(export_file, QSize(0, 0), 1)
            exporter.setInputFiles(
                [
                    self.get_test_data_path(
                        "control_images/video/expected_frame1/expected_frame1.png"
                    ).as_posix(),
                    self.get_test_data_path(
                        "control_images/video/expected_frame2/expected_frame2.png"
                    ).as_posix(),
                    self.get_test_data_path(
                        "control_images/video/expected_frame3/expected_frame3.png"
                    ).as_posix(),
                ]
            )
            # use an old format/codec so we don't have to worry about patents and widespread
            # availability of encoder
            exporter.setFileFormat(QMediaFormat.FileFormat.MPEG4)
            exporter.setVideoCodec(QMediaFormat.VideoCodec.H264)

            spy = QSignalSpy(exporter.finished)
            exporter.writeVideo()
            spy.wait()
            self.assertEqual(exporter.errorString(), "Could not initialize encoder")
            self.assertEqual(exporter.error(), QMediaRecorder.Error.ResourceError)


if __name__ == "__main__":
    unittest.main()
