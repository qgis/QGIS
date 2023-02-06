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
from qgis.core import (
    QgsFields,
    QgsField,
    QgsFieldConstraints,
    QgsProperty
)
from qgis.gui import (
    QgsFieldMappingWidget,
    QgsFieldMappingModel,
)
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
        self.assertEqual(model.rowCount(QModelIndex()), 3)
        self.assertIsNone(model.data(model.index(9999, 0), Qt.DisplayRole))
        # We now have this default mapping:
        # source exp        | destination fld
        # -------------------------------------------
        # source_field2     | destination_field1
        # source_field1     | destination_field2
        # NOT SET (NULL)    | destination_field3
        self.assertEqual(model.data(model.index(0, 0), Qt.DisplayRole), '"source_field2"')
        self.assertEqual(model.data(model.index(0, 1), Qt.DisplayRole), 'destination_field1')
        self.assertEqual(model.data(model.index(0, 3), Qt.DisplayRole), 10)
        self.assertEqual(model.data(model.index(0, 4), Qt.DisplayRole), 8)

        self.assertEqual(model.data(model.index(1, 0), Qt.DisplayRole), '"source_field1"')
        self.assertEqual(model.data(model.index(1, 1), Qt.DisplayRole), 'destination_field2')

        self.assertEqual(model.data(model.index(2, 0), Qt.DisplayRole), QVariant())
        self.assertEqual(model.data(model.index(2, 1), Qt.DisplayRole), 'destination_field3')

        # Test expression scope
        ctx = model.contextGenerator().createExpressionContext()
        self.assertTrue('source_field1' in ctx.fields().names())

        # Test add fields
        model.appendField(QgsField('destination_field4', QVariant.String))
        self.assertEqual(model.rowCount(QModelIndex()), 4)
        self.assertEqual(model.data(model.index(3, 1), Qt.DisplayRole), 'destination_field4')

        # Test remove field
        model.removeField(model.index(3, 0))
        self.assertEqual(model.rowCount(QModelIndex()), 3)
        self.assertEqual(model.data(model.index(2, 1), Qt.DisplayRole), 'destination_field3')

        # Test edit fields
        mapping = model.mapping()
        self.assertEqual(mapping[0].field.name(), 'destination_field1')
        self.assertEqual(mapping[1].field.name(), 'destination_field2')
        self.assertEqual(mapping[2].field.name(), 'destination_field3')
        self.assertEqual(mapping[0].originalName, 'destination_field1')
        self.assertEqual(mapping[1].originalName, 'destination_field2')
        self.assertEqual(mapping[2].originalName, 'destination_field3')

        # Test move up or down
        self.assertFalse(model.moveUp(model.index(0, 0)))
        self.assertFalse(model.moveUp(model.index(100, 0)))
        self.assertFalse(model.moveDown(model.index(2, 0)))
        self.assertFalse(model.moveDown(model.index(100, 0)))

        self.assertTrue(model.moveDown(model.index(0, 0)))
        mapping = model.mapping()
        self.assertEqual(mapping[1].field.name(), 'destination_field1')
        self.assertEqual(mapping[0].field.name(), 'destination_field2')
        self.assertEqual(mapping[2].field.name(), 'destination_field3')
        self.assertEqual(mapping[1].originalName, 'destination_field1')
        self.assertEqual(mapping[0].originalName, 'destination_field2')
        self.assertEqual(mapping[2].originalName, 'destination_field3')

        self.assertTrue(model.moveUp(model.index(1, 0)))
        mapping = model.mapping()
        self.assertEqual(mapping[0].field.name(), 'destination_field1')
        self.assertEqual(mapping[1].field.name(), 'destination_field2')
        self.assertEqual(mapping[2].field.name(), 'destination_field3')
        self.assertEqual(mapping[0].originalName, 'destination_field1')
        self.assertEqual(mapping[1].originalName, 'destination_field2')
        self.assertEqual(mapping[2].originalName, 'destination_field3')

        self.assertTrue(model.moveUp(model.index(2, 0)))
        mapping = model.mapping()
        self.assertEqual(mapping[0].field.name(), 'destination_field1')
        self.assertEqual(mapping[2].field.name(), 'destination_field2')
        self.assertEqual(mapping[1].field.name(), 'destination_field3')
        self.assertEqual(mapping[0].originalName, 'destination_field1')
        self.assertEqual(mapping[2].originalName, 'destination_field2')
        self.assertEqual(mapping[1].originalName, 'destination_field3')

    def testSetSourceFields(self):
        """Test that changing source fields also empty expressions are updated"""

        model = QgsFieldMappingModel(self.source_fields, self.destination_fields)
        self.assertEqual(model.data(model.index(2, 0), Qt.DisplayRole), QVariant())
        self.assertEqual(model.data(model.index(2, 1), Qt.DisplayRole), 'destination_field3')

        f = QgsField('source_field3', QVariant.String)
        fields = self.source_fields
        fields.append(f)
        model.setSourceFields(fields)
        self.assertEqual(model.data(model.index(0, 0), Qt.DisplayRole), '"source_field2"')
        self.assertEqual(model.data(model.index(0, 1), Qt.DisplayRole), 'destination_field1')
        self.assertEqual(model.data(model.index(1, 0), Qt.DisplayRole), '"source_field1"')
        self.assertEqual(model.data(model.index(1, 1), Qt.DisplayRole), 'destination_field2')
        self.assertEqual(model.data(model.index(2, 0), Qt.DisplayRole), '"source_field3"')
        self.assertEqual(model.data(model.index(2, 1), Qt.DisplayRole), 'destination_field3')

    def testProperties(self):
        model = QgsFieldMappingModel(self.source_fields, self.destination_fields)
        model.setDestinationFields(self.destination_fields, {'destination_field1': '5',
                                                             'destination_field2': 'source_field2',
                                                             'destination_field3': 'source_field2 * @myvar'})

        mapping = model.mapping()
        self.assertEqual(mapping[0].field.name(), 'destination_field1')
        self.assertEqual(mapping[1].field.name(), 'destination_field2')
        self.assertEqual(mapping[2].field.name(), 'destination_field3')
        self.assertEqual(mapping[0].expression, '5')
        self.assertEqual(mapping[1].expression, 'source_field2')
        self.assertEqual(mapping[2].expression, 'source_field2 * @myvar')

        self.assertEqual(model.fieldPropertyMap(), {'destination_field1': QgsProperty.fromExpression('5'),
                                                    'destination_field2': QgsProperty.fromField('source_field2'),
                                                    'destination_field3': QgsProperty.fromExpression(
                                                        'source_field2 * @myvar'),
                                                    })

        model = QgsFieldMappingModel(self.source_fields, self.destination_fields)
        self.assertEqual(model.fieldPropertyMap(), {'destination_field1': QgsProperty.fromField('source_field2'),
                                                    'destination_field2': QgsProperty.fromField('source_field1'),
                                                    'destination_field3': QgsProperty.fromExpression(''),
                                                    })

        model.setFieldPropertyMap({
            'destination_field1': QgsProperty.fromField('source_field1'),
            'destination_field2': QgsProperty.fromExpression('55*6'),
            'destination_field3': QgsProperty.fromValue(6),
        })
        self.assertEqual(model.fieldPropertyMap(), {
            'destination_field1': QgsProperty.fromField('source_field1'),
            'destination_field2': QgsProperty.fromExpression('55*6'),
            'destination_field3': QgsProperty.fromExpression('6'),
        })

    def testWidget(self):
        """Test widget operations"""

        widget = QgsFieldMappingWidget()
        for i in range(10):
            widget.appendField(QgsField(str(i)))
        self.assertTrue(widget.model().rowCount(QModelIndex()), 10)

        def _compare(widget, expected):
            actual = []
            for field in widget.mapping():
                actual.append(int(field.originalName))
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

        # Test set destination fields
        widget.setSourceFields(self.source_fields)
        widget.setDestinationFields(self.destination_fields)
        mapping = widget.mapping()
        self.assertEqual(mapping[0].field.name(), 'destination_field1')
        self.assertEqual(mapping[1].field.name(), 'destination_field2')
        self.assertEqual(mapping[2].field.name(), 'destination_field3')
        self.assertEqual(mapping[0].originalName, 'destination_field1')
        self.assertEqual(mapping[1].originalName, 'destination_field2')
        self.assertEqual(mapping[2].originalName, 'destination_field3')

        # Test constraints
        f = QgsField('constraint_field', QVariant.Int)
        constraints = QgsFieldConstraints()
        constraints.setConstraint(QgsFieldConstraints.ConstraintNotNull, QgsFieldConstraints.ConstraintOriginProvider)
        constraints.setConstraint(QgsFieldConstraints.ConstraintExpression,
                                  QgsFieldConstraints.ConstraintOriginProvider)
        constraints.setConstraint(QgsFieldConstraints.ConstraintUnique, QgsFieldConstraints.ConstraintOriginProvider)
        f.setConstraints(constraints)
        fields = QgsFields()
        fields.append(f)
        widget.setDestinationFields(fields)
        self.assertEqual(widget.model().data(widget.model().index(0, 5, QModelIndex()), Qt.DisplayRole),
                         "Constraints active")
        self.assertEqual(widget.model().data(widget.model().index(0, 5, QModelIndex()), Qt.ToolTipRole),
                         "Unique<br>Not null<br>Expression")
        self.assertEqual(widget.model().data(widget.model().index(0, 5, QModelIndex()), Qt.BackgroundColorRole),
                         QColor(255, 224, 178))

        # self._showDialog(widget)


if __name__ == '__main__':
    unittest.main()
