"""QGIS Unit tests for QgsProcessingUtils

From build dir, run: ctest -R PyQgsProcessingUtils -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Germán Carrillo'
__date__ = '7.3.2022'
__copyright__ = 'Copyright 2022, The QGIS Project'

from qgis.testing import unittest, start_app
from qgis.core import QgsFields, QgsField, QgsProcessingUtils


class TestQgsProcessingUtils(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        start_app()

    def test_combineFields_no_name_conflict(self):
        field1A = QgsField('ID')
        field1B = QgsField('NAME')
        fields1 = QgsFields()
        fields1.append(field1A)
        fields1.append(field1B)

        field2A = QgsField('FID')
        field2B = QgsField('AREA')
        field2C = QgsField('COUNT')
        fields2 = QgsFields()
        fields2.append(field2A)
        fields2.append(field2B)
        fields2.append(field2C)

        combined = QgsProcessingUtils.combineFields(fields1, fields2)
        self.assertEqual(len(combined), 5)

        expected_names = ['ID', 'NAME', 'FID', 'AREA', 'COUNT']
        obtained_names = [field.name() for field in combined]
        self.assertEqual(expected_names, obtained_names)

    def test_combineFields_no_name_conflict_prefix(self):
        prefix = 'joined_'

        field1A = QgsField('ID')
        field1B = QgsField('NAME')
        fields1 = QgsFields()
        fields1.append(field1A)
        fields1.append(field1B)

        field2A = QgsField('FID')
        field2B = QgsField('AREA')
        field2C = QgsField('COUNT')
        fields2 = QgsFields()
        fields2.append(field2A)
        fields2.append(field2B)
        fields2.append(field2C)

        combined = QgsProcessingUtils.combineFields(fields1, fields2, prefix)
        self.assertEqual(len(combined), 5)

        expected_names = ['ID', 'NAME', 'joined_FID', 'joined_AREA', 'joined_COUNT']
        obtained_names = [field.name() for field in combined]
        self.assertEqual(expected_names, obtained_names)

    def test_combineFields_name_conflict(self):
        field1A = QgsField('ID')
        field1B = QgsField('NAME')
        fields1 = QgsFields()
        fields1.append(field1A)
        fields1.append(field1B)

        field2A = QgsField('FID')
        field2B = QgsField('NAME')
        field2C = QgsField('COUNT')
        fields2 = QgsFields()
        fields2.append(field2A)
        fields2.append(field2B)
        fields2.append(field2C)

        combined = QgsProcessingUtils.combineFields(fields1, fields2)
        self.assertEqual(len(combined), 5)

        expected_names = ['ID', 'NAME', 'FID', 'NAME_2', 'COUNT']
        obtained_names = [field.name() for field in combined]
        self.assertEqual(expected_names, obtained_names)

    def test_combineFields_name_conflict_prefix(self):
        prefix = 'joined_'

        field1A = QgsField('ID')
        field1B = QgsField('NAME')
        fields1 = QgsFields()
        fields1.append(field1A)
        fields1.append(field1B)

        field2A = QgsField('FID')
        field2B = QgsField('NAME')
        field2C = QgsField('COUNT')
        fields2 = QgsFields()
        fields2.append(field2A)
        fields2.append(field2B)
        fields2.append(field2C)

        combined = QgsProcessingUtils.combineFields(fields1, fields2, prefix)
        self.assertEqual(len(combined), 5)

        expected_names = ['ID', 'NAME', 'joined_FID', 'joined_NAME', 'joined_COUNT']
        obtained_names = [field.name() for field in combined]
        self.assertEqual(expected_names, obtained_names)

    def test_combineFields_issue_47651(self):
        field1A = QgsField('ID')
        field1B = QgsField('FK')
        fields1 = QgsFields()
        fields1.append(field1A)
        fields1.append(field1B)

        field2A = QgsField('ID')
        field2B = QgsField('ID_2')
        field2C = QgsField('COUNT')
        fields2 = QgsFields()
        fields2.append(field2A)
        fields2.append(field2B)
        fields2.append(field2C)

        combined = QgsProcessingUtils.combineFields(fields1, fields2)
        self.assertEqual(len(combined), 5)

        expected_names = ['ID', 'FK', 'ID_3', 'ID_2', 'COUNT']
        obtained_names = [field.name() for field in combined]
        self.assertEqual(expected_names, obtained_names)

    def test_combineFields_issue_47651_prefix(self):
        prefix = 'joined_'
        field1A = QgsField('ID')
        field1B = QgsField('FK')
        fields1 = QgsFields()
        fields1.append(field1A)
        fields1.append(field1B)

        field2A = QgsField('ID')
        field2B = QgsField('ID_2')
        field2C = QgsField('COUNT')
        fields2 = QgsFields()
        fields2.append(field2A)
        fields2.append(field2B)
        fields2.append(field2C)

        combined = QgsProcessingUtils.combineFields(fields1, fields2, prefix)
        self.assertEqual(len(combined), 5)

        expected_names = ['ID', 'FK', 'joined_ID', 'joined_ID_2', 'joined_COUNT']
        obtained_names = [field.name() for field in combined]
        self.assertEqual(expected_names, obtained_names)


if __name__ == "__main__":
    unittest.main()
