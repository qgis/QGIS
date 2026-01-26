"""QGIS Unit tests for QgsProperty

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "11.04.2017"
__copyright__ = "Copyright 2017, The QGIS Project"

from qgis.PyQt.QtCore import QDate
from qgis.core import QgsProperty
from qgis.testing import unittest


class TestQgsProperty(unittest.TestCase):

    def test_bool_operator(self):
        property = QgsProperty()
        self.assertFalse(property)
        property = QgsProperty.fromValue(5)
        self.assertTrue(property)
        property = QgsProperty.fromField("field")
        self.assertTrue(property)
        property = QgsProperty.fromExpression("1+2")
        self.assertTrue(property)


if __name__ == "__main__":
    unittest.main()
