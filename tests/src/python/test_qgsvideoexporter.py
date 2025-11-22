"""QGIS Unit tests for QgsVideoExporter

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import os
import tempfile

from qgis.PyQt.QtCore import QSize, QUrl
from qgis.PyQt.QtMultimedia import QMediaFormat, QMediaPlayer, QVideoSink
from qgis.PyQt.QtTest import QSignalSpy
from qgis.core import QgsVideoExporter
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsVideoExporter(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "video"

    def testSimple(self):
        exporter = QgsVideoExporter("test.avi", QSize(1080, 600), 30)
        exporter.setInputFiles(["frame1.png", "frame2.png"])
        self.assertEqual(exporter.inputFiles(), ["frame1.png", "frame2.png"])

        exporter.setFileFormat(QMediaFormat.FileFormat.AVI)
        self.assertEqual(exporter.fileFormat(), QMediaFormat.FileFormat.AVI)

        exporter.setVideoCodec(QMediaFormat.VideoCodec.MPEG4)
        self.assertEqual(exporter.videoCodec(), QMediaFormat.VideoCodec.MPEG4)

    def test_write(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            export_file = os.path.join(temp_dir, "my_video.mp4")
            export_file = "/home/nyall/test.mp4"

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

            spy = QSignalSpy(exporter.finished)
            exporter.writeVideo()
            spy.wait()

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


if __name__ == "__main__":
    unittest.main()
