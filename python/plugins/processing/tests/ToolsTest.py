"""
***************************************************************************
    ToolsTest
    ---------------------
    Date                 : July 2016
    Copyright            : (C) 2016 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Nyall Dawson"
__date__ = "July 2016"
__copyright__ = "(C) 2016, Nyall Dawson"

import os
import shutil

from qgis.core import NULL, QgsVectorLayer
import unittest
from qgis.testing import start_app, QgisTestCase

from processing.tests.TestData import points
from processing.tools import vector

testDataPath = os.path.join(os.path.dirname(__file__), "testdata")

start_app()


class VectorTest(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        cls.cleanup_paths = []

    @classmethod
    def tearDownClass(cls):
        for path in cls.cleanup_paths:
            shutil.rmtree(path)

    def testValues(self):
        test_data = points()
        test_layer = QgsVectorLayer(test_data, "test", "ogr")

        # field by index
        res = vector.values(test_layer, 1)
        self.assertEqual(res[1], [1, 2, 3, 4, 5, 6, 7, 8, 9])

        # field by name
        res = vector.values(test_layer, "id")
        self.assertEqual(res["id"], [1, 2, 3, 4, 5, 6, 7, 8, 9])

        # two fields
        res = vector.values(test_layer, 1, 2)
        self.assertEqual(res[1], [1, 2, 3, 4, 5, 6, 7, 8, 9])
        self.assertEqual(res[2], [2, 1, 0, 2, 1, 0, 0, 0, 0])

        # two fields by name
        res = vector.values(test_layer, "id", "id2")
        self.assertEqual(res["id"], [1, 2, 3, 4, 5, 6, 7, 8, 9])
        self.assertEqual(res["id2"], [2, 1, 0, 2, 1, 0, 0, 0, 0])

        # two fields by name and index
        res = vector.values(test_layer, "id", 2)
        self.assertEqual(res["id"], [1, 2, 3, 4, 5, 6, 7, 8, 9])
        self.assertEqual(res[2], [2, 1, 0, 2, 1, 0, 0, 0, 0])

    def testLoadField(self):
        test_data = points()
        test_layer = QgsVectorLayer(test_data, "test", "ogr")

        # basic load
        res = vector.load_field(test_layer, "id")
        self.assertEqual(res, [1, 2, 3, 4, 5, 6, 7, 8, 9])

        # test replacement
        res = vector.load_field(test_layer, "id2", replacements={0: "X"})
        self.assertEqual(res, [2, 1, "X", 2, 1, "X", "X", "X", "X"])
        res = vector.load_field(test_layer, "id2", replacements={0: None})
        self.assertEqual(res, [2, 1, None, 2, 1, None, None, None, None])


if __name__ == "__main__":
    unittest.main()
