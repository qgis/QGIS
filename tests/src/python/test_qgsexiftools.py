"""QGIS Unit tests for QgsExifTools.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "09/11/2018"
__copyright__ = "Copyright 2018, The QGIS Project"

import os
import shutil

from qgis.PyQt.QtCore import QDateTime, Qt, QTemporaryFile, QTime, QDate
from qgis.core import QgsExifTools, QgsPointXY
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

TEST_DATA_DIR = unitTestDataPath()

start_app()


class TestQgsExifUtils(QgisTestCase):

    def testReadTags(self):
        photos_folder = os.path.join(TEST_DATA_DIR, "photos")

        # test a converted Exif rational value
        elevation = QgsExifTools.readTag(
            os.path.join(photos_folder, "0997.JPG"), "Exif.GPSInfo.GPSAltitude"
        )
        self.assertEqual(elevation, 422.19101123595505)

        # test a converted Exif datetime value
        dt = QgsExifTools.readTag(
            os.path.join(photos_folder, "0997.JPG"), "Exif.Image.DateTime"
        )
        self.assertEqual(dt, QDateTime(2018, 3, 16, 12, 19, 19))

        # test a converted Xmp datetime value
        dt = QgsExifTools.readTag(
            os.path.join(photos_folder, "0997.JPG"), "Xmp.xmp.MetadataDate"
        )
        self.assertEqual(
            dt, QDateTime(QDate(2023, 2, 5), QTime(10, 16, 5, 0), Qt.TimeSpec(1))
        )

    def testGeoTags(self):
        photos_folder = os.path.join(TEST_DATA_DIR, "photos")

        tag, ok = QgsExifTools.getGeoTag("")
        self.assertFalse(ok)

        tag, ok = QgsExifTools.getGeoTag(os.path.join(photos_folder, "0997.JPG"))
        self.assertTrue(ok)
        self.assertEqual(tag.asWkt(6), "Point Z (149.275167 -37.2305 422.191011)")

        tag, ok = QgsExifTools.getGeoTag(os.path.join(photos_folder, "geotagged.jpg"))
        self.assertTrue(ok)
        self.assertEqual(tag.asWkt(6), "Point Z (149.131878 -36.220892 867)")

        tag, ok = QgsExifTools.getGeoTag(os.path.join(photos_folder, "notags.JPG"))
        self.assertFalse(ok)

        tag, ok = QgsExifTools.getGeoTag(os.path.join(photos_folder, "not_photo.jpg"))
        self.assertFalse(ok)

    def testTagging(self):
        self.assertFalse(QgsExifTools.geoTagImage("", QgsPointXY(1, 2)))
        self.assertFalse(QgsExifTools.geoTagImage("not a path", QgsPointXY(1, 2)))

        src_photo = os.path.join(TEST_DATA_DIR, "photos", "notags.JPG")

        tmpFile = QTemporaryFile()
        tmpFile.open()
        tmpName = tmpFile.fileName()
        tmpFile.close()

        shutil.copy(src_photo, tmpName)
        self.assertTrue(QgsExifTools.geoTagImage(tmpName, QgsPointXY(1.1, 3.3)))
        tag, ok = QgsExifTools.getGeoTag(tmpName)
        self.assertTrue(ok)
        self.assertEqual(tag.asWkt(6), "Point (1.1 3.3)")
        os.remove(tmpName)

        shutil.copy(src_photo, tmpName)
        self.assertTrue(QgsExifTools.geoTagImage(tmpName, QgsPointXY(-1.1, -3.3)))
        tag, ok = QgsExifTools.getGeoTag(tmpName)
        self.assertTrue(ok)
        self.assertEqual(tag.asWkt(6), "Point (-1.1 -3.3)")
        os.remove(tmpName)

        shutil.copy(src_photo, tmpName)
        deets = QgsExifTools.GeoTagDetails()
        deets.elevation = 110.1
        self.assertTrue(QgsExifTools.geoTagImage(tmpName, QgsPointXY(1.1, 3.3), deets))
        tag, ok = QgsExifTools.getGeoTag(tmpName)
        self.assertTrue(ok)
        self.assertEqual(tag.asWkt(6), "Point Z (1.1 3.3 110.1)")
        os.remove(tmpName)

        shutil.copy(src_photo, tmpName)
        deets = QgsExifTools.GeoTagDetails()
        deets.elevation = -110.1
        self.assertTrue(QgsExifTools.geoTagImage(tmpName, QgsPointXY(1.1, 3.3), deets))
        tag, ok = QgsExifTools.getGeoTag(tmpName)
        self.assertTrue(ok)
        self.assertEqual(tag.asWkt(6), "Point Z (1.1 3.3 -110.1)")
        os.remove(tmpName)

        shutil.copy(src_photo, tmpName)
        self.assertTrue(
            QgsExifTools.tagImage(tmpName, "Exif.Photo.ShutterSpeedValue", 5.333)
        )
        self.assertTrue(QgsExifTools.tagImage(tmpName, "Xmp.dc.Format", "image/jpeg"))
        value = QgsExifTools.readTag(tmpName, "Exif.Photo.ShutterSpeedValue")
        self.assertEqual(value, 5.333)
        value = QgsExifTools.readTag(tmpName, "Xmp.dc.Format")
        self.assertEqual(value, "image/jpeg")
        os.remove(tmpName)


if __name__ == "__main__":
    unittest.main()
