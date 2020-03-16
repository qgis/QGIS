# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsFieldMapping widget and model.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
__author__ = 'Alessandro Pasotti'
__date__ = '16/03/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from qgis.core import (
    QgsFields,
    QgsField,
)
from qgis.gui import (
    QgsFieldMappingWidget,
    QgsFieldMappingModel,
)
from qgis.PyQt.Qt import Qt
from qgis.PyQt.QtCore import QCoreApplication, QVariant

from qgis.testing import start_app, unittest


class TestPyQgsFieldMappingModel(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""

        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain(cls.__name__)
        QCoreApplication.setApplicationName(cls.__name__)
        start_app()

    def setUp(self):
        """Run before each test"""

        source_fields = QgsFields()
        f = QgsField('source_field1', QVariant.String)
        self.assertTrue(source_fields.append(f))
        f = QgsField('source_field2', QVariant.Int, 'integer', 10, 8)
        self.assertTrue(source_fields.append(f))

        destination_fields = QgsFields()
        f = QgsField('destination_field1', QVariant.Int, 'integer', 10, 8)
        self.assertTrue(destination_fields.append(f))
        f = QgsField('destination_field2', QVariant.String)
        self.assertTrue(destination_fields.append(f))
        f = QgsField('destination_field3', QVariant.String)
        self.assertTrue(destination_fields.append(f))

        self.source_fields = source_fields
        self.destination_fields = destination_fields

    def _showDialog(self, widget):
        """Used during development"""

        from qgis.PyQt.QtWidgets import QDialog, QVBoxLayout
        d = QDialog()
        l = QVBoxLayout()
        l.addWidget(widget)
        d.setLayout(l)
        d.exec()

    def testModel(self):
        """Test the mapping model"""

        model = QgsFieldMappingModel(self.source_fields, self.destination_fields)
        self.assertIsNone(model.data(model.index(9999, 0), Qt.DisplayRole))
        # We now have this default mapping:
        # source exp        | destination fld
        # -------------------------------------------
        # source_field2     | destination_field1
        # source_field1     | destination_field2
        # NOT SET (NULL)    | destination_field3
        self.assertEqual(model.data(model.index(0, 0), Qt.DisplayRole), '"source_field2"')
        self.assertEqual(model.data(model.index(0, 1), Qt.DisplayRole), 'destination_field1')
        self.assertEqual(model.data(model.index(0, 2), Qt.DisplayRole), 10)
        self.assertEqual(model.data(model.index(0, 3), Qt.DisplayRole), 8)

        self.assertEqual(model.data(model.index(1, 0), Qt.DisplayRole), '"source_field1"')
        self.assertEqual(model.data(model.index(1, 1), Qt.DisplayRole), 'destination_field2')

        self.assertEqual(model.data(model.index(2, 0), Qt.DisplayRole), QVariant())
        self.assertEqual(model.data(model.index(2, 1), Qt.DisplayRole), 'destination_field3')

        # Test expression scope
        ctx = model.contextGenerator().createExpressionContext()
        self.assertTrue('source_field1' in ctx.fields().names())

    def testWidget(self):
        """Test the mapping widget"""

        widget = QgsFieldMappingWidget(self.source_fields, self.destination_fields)
        self._showDialog(widget)


if __name__ == '__main__':
    unittest.main()
