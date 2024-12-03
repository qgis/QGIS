"""
test_versioncompare.py
                     --------------------------------------
               Date                 : September 2016
               Copyright            : (C) 2016 Alexander Bruy
               email                : alexander dot bruy at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
"""

from pyplugin_installer.version_compare import compareVersions
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestVersionCompare(QgisTestCase):

    def setUp(self):
        """Run before each test."""
        pass

    def tearDown(self):
        """Run after each test."""
        pass

    def testCompareVersions(self):
        a = "1.0.0"
        # a == b
        b = "1.0.0"
        self.assertEqual(compareVersions(a, b), 0)
        # a > b
        b = "0.1.0"
        self.assertEqual(compareVersions(a, b), 1)
        # b > a
        b = "1.1.0"
        self.assertEqual(compareVersions(a, b), 2)

        # test that prefix stripped correctly
        a = "ver. 1.0.0"
        b = "ver. 0.1.0"
        self.assertEqual(compareVersions(a, b), 1)

        # test versions with build numbers
        a = "1.0.0-1"
        b = "1.0.0-2"
        self.assertEqual(compareVersions(a, b), 2)

        # test versions with suffixes
        a = "1.0.0a"
        b = "1.0.0b"
        self.assertEqual(compareVersions(a, b), 2)

        # test versions with suffixes in different cases
        a = "1.0.0-201609011405-2690BD9"
        b = "1.0.0-201609011405-2690bd9"
        self.assertEqual(compareVersions(a, b), 0)


if __name__ == "__main__":
    unittest.main()
