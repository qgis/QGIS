# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsAggregateMapping widget and model.

From build dir, run: ctest -R PyQgsAggregateMappingWidget -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
__author__ = 'Nyall Dawson'
__date__ = '03/06/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from qgis.core import (
    QgsFields,
    QgsField,
    QgsFieldConstraints,
)
from qgis.gui import (
    QgsAggregateMappingWidget,
    QgsAggregateMappingModel,
)
from qgis.PyQt.Qt import Qt
from qgis.PyQt.QtCore import (
    QCoreApplication,
    QVariant,
    QModelIndex,
    QItemSelectionModel,
)
from qgis.PyQt.QtGui import (
    QColor
)
from qgis.testing import start_app, unittest


class TestPyQgsAggregateMappingModel(unittest.TestCase):

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

        self.source_fields = source_fields

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

        model = QgsAggregateMappingModel(self.source_fields)
        self.assertEqual(model.rowCount(QModelIndex()), 2)
        self.assertIsNone(model.data(model.index(9999, 0), Qt.DisplayRole))

        self.assertEqual(model.data(model.index(0, 0), Qt.DisplayRole), '"source_field1"')
        self.assertEqual(model.data(model.index(0, 1), Qt.DisplayRole), 'concatenate')
        self.assertEqual(model.data(model.index(0, 2), Qt.DisplayRole), ',')
        self.assertEqual(model.data(model.index(0, 3), Qt.DisplayRole), 'source_field1')
        self.assertEqual(model.data(model.index(0, 4), Qt.DisplayRole), 'text')
        self.assertEqual(model.data(model.index(0, 5), Qt.DisplayRole), 0)
        self.assertEqual(model.data(model.index(0, 6), Qt.DisplayRole), 0)

        self.assertEqual(model.data(model.index(1, 0), Qt.DisplayRole), '"source_field2"')
        self.assertEqual(model.data(model.index(1, 1), Qt.DisplayRole), 'sum')
        self.assertEqual(model.data(model.index(1, 2), Qt.DisplayRole), ',')
        self.assertEqual(model.data(model.index(1, 3), Qt.DisplayRole), 'source_field2')
        self.assertEqual(model.data(model.index(1, 4), Qt.DisplayRole), 'integer')
        self.assertEqual(model.data(model.index(1, 5), Qt.DisplayRole), 10)
        self.assertEqual(model.data(model.index(1, 6), Qt.DisplayRole), 8)

        # Test expression scope
        ctx = model.contextGenerator().createExpressionContext()
        self.assertTrue('source_field1' in ctx.fields().names())

        # Test add fields
        model.appendField(QgsField('field3', QVariant.String), 'upper("field3")', 'first_value')
        self.assertEqual(model.rowCount(QModelIndex()), 3)
        self.assertEqual(model.data(model.index(2, 0), Qt.DisplayRole), 'upper("field3")')
        self.assertEqual(model.data(model.index(2, 1), Qt.DisplayRole), 'first_value')
        self.assertEqual(model.data(model.index(2, 2), Qt.DisplayRole), ',')
        self.assertEqual(model.data(model.index(2, 3), Qt.DisplayRole), 'field3')
        self.assertEqual(model.data(model.index(2, 4), Qt.DisplayRole), 'text')
        self.assertEqual(model.data(model.index(2, 5), Qt.DisplayRole), 0)
        self.assertEqual(model.data(model.index(2, 6), Qt.DisplayRole), 0)

        # Test remove field
        model.removeField(model.index(1, 0))
        self.assertEqual(model.rowCount(QModelIndex()), 2)
        self.assertEqual(model.data(model.index(0, 0), Qt.DisplayRole), '"source_field1"')
        self.assertEqual(model.data(model.index(1, 0), Qt.DisplayRole), 'upper("field3")')

        # Test edit fields
        mapping = model.mapping()
        self.assertEqual(mapping[0].field.name(), 'source_field1')
        self.assertEqual(mapping[0].aggregate, 'concatenate')
        self.assertEqual(mapping[0].delimiter, ',')
        self.assertEqual(mapping[0].source, '"source_field1"')
        self.assertEqual(mapping[1].field.name(), 'field3')
        self.assertEqual(mapping[1].aggregate, 'first_value')
        self.assertEqual(mapping[1].delimiter, ',')
        self.assertEqual(mapping[1].source, 'upper("field3")')

        # Test move up or down
        self.assertFalse(model.moveUp(model.index(0, 0)))
        self.assertFalse(model.moveUp(model.index(100, 0)))
        self.assertFalse(model.moveDown(model.index(1, 0)))
        self.assertFalse(model.moveDown(model.index(100, 0)))

        self.assertTrue(model.moveDown(model.index(0, 0)))
        mapping = model.mapping()
        self.assertEqual(mapping[0].field.name(), 'field3')
        self.assertEqual(mapping[1].field.name(), 'source_field1')

        self.assertTrue(model.moveUp(model.index(1, 0)))
        mapping = model.mapping()
        self.assertEqual(mapping[0].field.name(), 'source_field1')
        self.assertEqual(mapping[1].field.name(), 'field3')

    def testSetSourceFields(self):
        """Test that changing source fields also empty expressions are updated"""

        model = QgsAggregateMappingModel(self.source_fields)
        self.assertEqual(model.data(model.index(0, 0), Qt.DisplayRole), '"source_field1"')
        self.assertEqual(model.data(model.index(0, 3), Qt.DisplayRole), 'source_field1')
        self.assertEqual(model.data(model.index(1, 0), Qt.DisplayRole), '"source_field2"')
        self.assertEqual(model.data(model.index(1, 3), Qt.DisplayRole), 'source_field2')

        f = QgsField('source_field3', QVariant.String)
        fields = self.source_fields
        fields.append(f)
        model.setSourceFields(fields)
        self.assertEqual(model.rowCount(), 3)
        self.assertEqual(model.data(model.index(0, 0), Qt.DisplayRole), '"source_field1"')
        self.assertEqual(model.data(model.index(0, 3), Qt.DisplayRole), 'source_field1')
        self.assertEqual(model.data(model.index(1, 0), Qt.DisplayRole), '"source_field2"')
        self.assertEqual(model.data(model.index(1, 3), Qt.DisplayRole), 'source_field2')
        self.assertEqual(model.data(model.index(2, 0), Qt.DisplayRole), '"source_field3"')
        self.assertEqual(model.data(model.index(2, 3), Qt.DisplayRole), 'source_field3')

    def testProperties(self):
        model = QgsAggregateMappingModel(self.source_fields)

        mapping = model.mapping()
        self.assertEqual(mapping[0].field.name(), 'source_field1')
        self.assertEqual(mapping[0].source, '"source_field1"')
        self.assertEqual(mapping[0].aggregate, 'concatenate')
        self.assertEqual(mapping[0].delimiter, ',')
        self.assertEqual(mapping[1].field.name(), 'source_field2')
        self.assertEqual(mapping[1].source, '"source_field2"')
        self.assertEqual(mapping[1].aggregate, 'sum')
        self.assertEqual(mapping[1].delimiter, ',')

        mapping[0].source = 'upper("source_field2")'
        mapping[0].aggregate = 'first_value'
        mapping[0].delimiter = '|'
        new_aggregate = QgsAggregateMappingModel.Aggregate()
        new_aggregate.field = QgsField('output_field3', QVariant.Double, len=4, prec=2)
        new_aggregate.source = 'randf(1,2)'
        new_aggregate.aggregate = 'mean'
        new_aggregate.delimiter = '*'
        mapping.append(new_aggregate)

        model.setMapping(mapping)

        self.assertEqual(model.rowCount(), 3)
        self.assertEqual(model.data(model.index(0, 0), Qt.DisplayRole), 'upper("source_field2")')
        self.assertEqual(model.data(model.index(0, 1), Qt.DisplayRole), 'first_value')
        self.assertEqual(model.data(model.index(0, 2), Qt.DisplayRole), '|')
        self.assertEqual(model.data(model.index(0, 3), Qt.DisplayRole), 'source_field1')
        self.assertEqual(model.data(model.index(0, 4), Qt.DisplayRole), 'text')
        self.assertEqual(model.data(model.index(0, 5), Qt.DisplayRole), 0)
        self.assertEqual(model.data(model.index(0, 6), Qt.DisplayRole), 0)
        self.assertEqual(model.data(model.index(1, 0), Qt.DisplayRole), '"source_field2"')
        self.assertEqual(model.data(model.index(1, 1), Qt.DisplayRole), 'sum')
        self.assertEqual(model.data(model.index(1, 2), Qt.DisplayRole), ',')
        self.assertEqual(model.data(model.index(1, 3), Qt.DisplayRole), 'source_field2')
        self.assertEqual(model.data(model.index(1, 4), Qt.DisplayRole), 'integer')
        self.assertEqual(model.data(model.index(1, 5), Qt.DisplayRole), 10)
        self.assertEqual(model.data(model.index(1, 6), Qt.DisplayRole), 8)
        self.assertEqual(model.data(model.index(2, 0), Qt.DisplayRole), 'randf(1,2)')
        self.assertEqual(model.data(model.index(2, 1), Qt.DisplayRole), 'mean')
        self.assertEqual(model.data(model.index(2, 2), Qt.DisplayRole), '*')
        self.assertEqual(model.data(model.index(2, 3), Qt.DisplayRole), 'output_field3')
        self.assertEqual(model.data(model.index(2, 4), Qt.DisplayRole), 'double precision')
        self.assertEqual(model.data(model.index(2, 5), Qt.DisplayRole), 4)
        self.assertEqual(model.data(model.index(2, 6), Qt.DisplayRole), 2)

    def testWidget(self):
        """Test widget operations"""

        widget = QgsAggregateMappingWidget()
        for i in range(10):
            widget.appendField(QgsField(str(i)), source=str(i))
        self.assertTrue(widget.model().rowCount(QModelIndex()), 10)

        def _compare(widget, expected):
            actual = []
            for aggregate in widget.mapping():
                actual.append(int(aggregate.source))
            self.assertEqual(actual, expected)

        _compare(widget, [0, 1, 2, 3, 4, 5, 6, 7, 8, 9])

        selection_model = widget.selectionModel()
        selection_model.clear()
        for i in range(0, 10, 2):
            selection_model.select(widget.model().index(i, 0), QItemSelectionModel.Select)

        self.assertTrue(widget.moveSelectedFieldsDown())
        _compare(widget, [1, 0, 3, 2, 5, 4, 7, 6, 9, 8])

        selection_model.clear()
        for i in range(1, 10, 2):
            selection_model.select(widget.model().index(i, 0), QItemSelectionModel.Select)

        self.assertTrue(widget.moveSelectedFieldsUp())
        _compare(widget, [0, 1, 2, 3, 4, 5, 6, 7, 8, 9])

        selection_model.clear()
        for i in range(0, 10, 2):
            selection_model.select(widget.model().index(i, 0), QItemSelectionModel.Select)

        self.assertTrue(widget.removeSelectedFields())
        _compare(widget, [1, 3, 5, 7, 9])

        widget.setSourceFields(self.source_fields)
        mapping = widget.mapping()

        self.assertEqual(mapping[0].field.name(), 'source_field1')
        self.assertEqual(mapping[0].source, '"source_field1"')
        self.assertEqual(mapping[0].aggregate, 'concatenate')
        self.assertEqual(mapping[0].delimiter, ',')
        self.assertEqual(mapping[1].field.name(), 'source_field2')
        self.assertEqual(mapping[1].source, '"source_field2"')
        self.assertEqual(mapping[1].aggregate, 'sum')
        self.assertEqual(mapping[1].delimiter, ',')

        mapping[0].source = 'upper("source_field2")'
        mapping[0].aggregate = 'first_value'
        mapping[0].delimiter = '|'
        new_aggregate = QgsAggregateMappingModel.Aggregate()
        new_aggregate.field = QgsField('output_field3', QVariant.Double, len=4, prec=2)
        new_aggregate.source = 'randf(1,2)'
        new_aggregate.aggregate = 'mean'
        new_aggregate.delimiter = '*'
        mapping.append(new_aggregate)

        widget.setMapping(mapping)

        mapping = widget.mapping()

        self.assertEqual(mapping[0].field.name(), 'source_field1')
        self.assertEqual(mapping[0].source, 'upper("source_field2")')
        self.assertEqual(mapping[0].aggregate, 'first_value')
        self.assertEqual(mapping[0].delimiter, '|')
        self.assertEqual(mapping[1].field.name(), 'source_field2')
        self.assertEqual(mapping[1].source, '"source_field2"')
        self.assertEqual(mapping[1].aggregate, 'sum')
        self.assertEqual(mapping[1].delimiter, ',')
        self.assertEqual(mapping[2].field.name(), 'output_field3')
        self.assertEqual(mapping[2].source, 'randf(1,2)')
        self.assertEqual(mapping[2].aggregate, 'mean')
        self.assertEqual(mapping[2].delimiter, '*')


if __name__ == '__main__':
    unittest.main()
