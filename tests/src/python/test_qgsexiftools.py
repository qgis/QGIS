# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsExifTools.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '09/11/2018'
__copyright__ = 'Copyright 2018, The QGIS Project'

import qgis  # NOQA switch sip api
import os
import shutil
from qgis.PyQt.QtCore import QTemporaryFile
from qgis.core import QgsPointXY
from qgis.analysis import (QgsExifTools)
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

TEST_DATA_DIR = unitTestDataPath()

start_app()


class TestQgsExifUtils(unittest.TestCase):

    def testGeoTags(self):
        photos_folder = os.path.join(TEST_DATA_DIR, 'photos')

        tag, ok = QgsExifTools.getGeoTag('')
        self.assertFalse(ok)

        tag, ok = QgsExifTools.getGeoTag(os.path.join(photos_folder, '0997.JPG'))
        self.assertTrue(ok)
        self.assertEqual(tag.asWkt(6), 'PointZ (149.275167 -37.2305 422.191011)')

        tag, ok = QgsExifTools.getGeoTag(os.path.join(photos_folder, 'geotagged.jpg'))
        self.assertTrue(ok)
        self.assertEqual(tag.asWkt(6), 'PointZ (149.131878 -36.220892 867)')

        tag, ok = QgsExifTools.getGeoTag(os.path.join(photos_folder, 'notags.JPG'))
        self.assertFalse(ok)

        tag, ok = QgsExifTools.getGeoTag(os.path.join(photos_folder, 'not_photo.jpg'))
        self.assertFalse(ok)

    def testTagging(self):
        self.assertFalse(QgsExifTools.geoTagImage('', QgsPointXY(1, 2)))
        self.assertFalse(QgsExifTools.geoTagImage('not a path', QgsPointXY(1, 2)))

        src_photo = os.path.join(TEST_DATA_DIR, 'photos', 'notags.JPG')

        tmpFile = QTemporaryFile()
        tmpFile.open()
        tmpName = tmpFile.fileName()
        tmpFile.close()

        shutil.copy(src_photo, tmpName)
        self.assertTrue(QgsExifTools.geoTagImage(tmpName, QgsPointXY(1.1, 3.3)))
        tag, ok = QgsExifTools.getGeoTag(tmpName)
        self.assertTrue(ok)
        self.assertEqual(tag.asWkt(6), 'Point (1.1 3.3)')
        os.remove(tmpName)

        shutil.copy(src_photo, tmpName)
        self.assertTrue(QgsExifTools.geoTagImage(tmpName, QgsPointXY(-1.1, -3.3)))
        tag, ok = QgsExifTools.getGeoTag(tmpName)
        self.assertTrue(ok)
        self.assertEqual(tag.asWkt(6), 'Point (-1.1 -3.3)')
        os.remove(tmpName)

        shutil.copy(src_photo, tmpName)
        deets = QgsExifTools.GeoTagDetails()
        deets.elevation = 110.1
        self.assertTrue(QgsExifTools.geoTagImage(tmpName, QgsPointXY(1.1, 3.3), deets))
        tag, ok = QgsExifTools.getGeoTag(tmpName)
        self.assertTrue(ok)
        self.assertEqual(tag.asWkt(6), 'PointZ (1.1 3.3 110.1)')
        os.remove(tmpName)

        shutil.copy(src_photo, tmpName)
        deets = QgsExifTools.GeoTagDetails()
        deets.elevation = -110.1
        self.assertTrue(QgsExifTools.geoTagImage(tmpName, QgsPointXY(1.1, 3.3), deets))
        tag, ok = QgsExifTools.getGeoTag(tmpName)
        self.assertTrue(ok)
        self.assertEqual(tag.asWkt(6), 'PointZ (1.1 3.3 -110.1)')
        os.remove(tmpName)


if __name__ == '__main__':
    unittest.main()
