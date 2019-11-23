# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsFieldModel

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '14/11/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'

import qgis  # NOQA

from qgis.core import (QgsField,
                       QgsFields,
                       QgsVectorLayer,
                       QgsFieldModel,
                       QgsFieldProxyModel,
                       QgsEditorWidgetSetup,
                       QgsProject,
                       QgsVectorLayerJoinInfo)
from qgis.PyQt.QtCore import QVariant, Qt

from qgis.testing import start_app, unittest

start_app()


def create_layer():
    layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer",
                           "addfeat", "memory")
    layer.setEditorWidgetSetup(0, QgsEditorWidgetSetup('Hidden', {}))
    layer.setEditorWidgetSetup(1, QgsEditorWidgetSetup('ValueMap', {}))
    assert layer.isValid()
    return layer


def create_model():
    l = create_layer()
    m = QgsFieldModel()
    m.setLayer(l)
    return l, m


class TestQgsFieldModel(unittest.TestCase):

    def testGettersSetters(self):
        """ test model getters/setters """
        l = create_layer()
        m = QgsFieldModel()

        self.assertFalse(m.layer())
        m.setLayer(l)
        self.assertEqual(m.layer(), l)

        m.setAllowExpression(True)
        self.assertTrue(m.allowExpression())
        m.setAllowExpression(False)
        self.assertFalse(m.allowExpression())

        m.setAllowEmptyFieldName(True)
        self.assertTrue(m.allowEmptyFieldName())
        m.setAllowEmptyFieldName(False)
        self.assertFalse(m.allowEmptyFieldName())

    def testIndexFromName(self):
        l, m = create_model()
        i = m.indexFromName('fldtxt')
        self.assertTrue(i.isValid())
        self.assertEqual(i.row(), 0)
        i = m.indexFromName('fldint')
        self.assertTrue(i.isValid())
        self.assertEqual(i.row(), 1)
        i = m.indexFromName('not a field')
        self.assertFalse(i.isValid())

        # test with alias
        i = m.indexFromName('text field')
        self.assertFalse(i.isValid())
        l.setFieldAlias(0, 'text field')
        i = m.indexFromName('text field')
        self.assertTrue(i.isValid())
        self.assertEqual(i.row(), 0)
        i = m.indexFromName('int field')
        self.assertFalse(i.isValid())
        l.setFieldAlias(1, 'int field')
        i = m.indexFromName('int field')
        self.assertTrue(i.isValid())
        self.assertEqual(i.row(), 1)

        # should be case insensitive
        i = m.indexFromName('FLDTXT')
        self.assertTrue(i.isValid())
        self.assertEqual(i.row(), 0)
        i = m.indexFromName('FLDINT')
        self.assertTrue(i.isValid())
        self.assertEqual(i.row(), 1)

        # try with expression
        m.setAllowExpression(True)
        i = m.indexFromName('not a field')
        # still not valid - needs expression set first
        self.assertFalse(i.isValid())
        m.setExpression('not a field')
        i = m.indexFromName('not a field')
        self.assertTrue(i.isValid())
        self.assertEqual(i.row(), 2)

        # try with null
        i = m.indexFromName(None)
        self.assertFalse(i.isValid())
        m.setAllowEmptyFieldName(True)
        i = m.indexFromName(None)
        self.assertTrue(i.isValid())
        self.assertEqual(i.row(), 0)
        # when null is shown, all other rows should be offset
        self.assertEqual(m.indexFromName('fldtxt').row(), 1)
        self.assertEqual(m.indexFromName('fldint').row(), 2)
        self.assertEqual(m.indexFromName('not a field').row(), 3)
        self.assertEqual(m.indexFromName('FLDTXT').row(), 1)
        self.assertEqual(m.indexFromName('FLDINT').row(), 2)

    def testIsField(self):
        l, m = create_model()
        self.assertTrue(m.isField('fldtxt'))
        self.assertTrue(m.isField('fldint'))
        self.assertFalse(m.isField(None))
        self.assertFalse(m.isField('an expression'))

    def testRowCount(self):
        l, m = create_model()
        self.assertEqual(m.rowCount(), 2)
        m.setAllowEmptyFieldName(True)
        self.assertEqual(m.rowCount(), 3)
        m.setAllowExpression(True)
        m.setExpression('not a field')
        self.assertEqual(m.rowCount(), 4)
        m.setExpression('not a field')
        self.assertEqual(m.rowCount(), 4)
        m.setExpression('not a field 2')
        self.assertEqual(m.rowCount(), 4)
        m.removeExpression()
        self.assertEqual(m.rowCount(), 3)

    def testFieldNameRole(self):
        l, m = create_model()
        self.assertEqual(m.data(m.indexFromName('fldtxt'), QgsFieldModel.FieldNameRole), 'fldtxt')
        self.assertEqual(m.data(m.indexFromName('fldint'), QgsFieldModel.FieldNameRole), 'fldint')
        self.assertFalse(m.data(m.indexFromName('an expression'), QgsFieldModel.FieldNameRole))
        self.assertFalse(m.data(m.indexFromName(None), QgsFieldModel.FieldNameRole))
        m.setAllowExpression(True)
        m.setExpression('an expression')
        self.assertFalse(m.data(m.indexFromName('an expression'), QgsFieldModel.FieldNameRole))
        m.setAllowEmptyFieldName(True)
        self.assertFalse(m.data(m.indexFromName(None), QgsFieldModel.FieldNameRole))

    def testExpressionRole(self):
        l, m = create_model()
        self.assertEqual(m.data(m.indexFromName('fldtxt'), QgsFieldModel.ExpressionRole), 'fldtxt')
        self.assertEqual(m.data(m.indexFromName('fldint'), QgsFieldModel.ExpressionRole), 'fldint')
        self.assertFalse(m.data(m.indexFromName('an expression'), QgsFieldModel.ExpressionRole))
        self.assertFalse(m.data(m.indexFromName(None), QgsFieldModel.ExpressionRole))
        m.setAllowExpression(True)
        m.setExpression('an expression')
        self.assertEqual(m.data(m.indexFromName('an expression'), QgsFieldModel.ExpressionRole), 'an expression')
        m.setAllowEmptyFieldName(True)
        self.assertFalse(m.data(m.indexFromName(None), QgsFieldModel.ExpressionRole))

    def testFieldIndexRole(self):
        l, m = create_model()
        self.assertEqual(m.data(m.indexFromName('fldtxt'), QgsFieldModel.FieldIndexRole), 0)
        self.assertEqual(m.data(m.indexFromName('fldint'), QgsFieldModel.FieldIndexRole), 1)
        self.assertFalse(m.data(m.indexFromName('an expression'), QgsFieldModel.FieldIndexRole))
        self.assertFalse(m.data(m.indexFromName(None), QgsFieldModel.FieldIndexRole))
        m.setAllowExpression(True)
        m.setExpression('an expression')
        self.assertFalse(m.data(m.indexFromName('an expression'), QgsFieldModel.FieldIndexRole))
        m.setAllowEmptyFieldName(True)
        self.assertFalse(m.data(m.indexFromName(None), QgsFieldModel.FieldIndexRole))

    def testIsExpressionRole(self):
        l, m = create_model()
        self.assertFalse(m.data(m.indexFromName('fldtxt'), QgsFieldModel.IsExpressionRole))
        self.assertFalse(m.data(m.indexFromName('fldint'), QgsFieldModel.IsExpressionRole))
        self.assertFalse(m.data(m.indexFromName('an expression'), QgsFieldModel.IsExpressionRole))
        self.assertFalse(m.data(m.indexFromName(None), QgsFieldModel.IsExpressionRole))
        m.setAllowExpression(True)
        m.setExpression('an expression')
        self.assertTrue(m.data(m.indexFromName('an expression'), QgsFieldModel.IsExpressionRole))
        m.setAllowEmptyFieldName(True)
        self.assertFalse(m.data(m.indexFromName(None), QgsFieldModel.IsExpressionRole))

    def testExpressionValidityRole(self):
        l, m = create_model()
        self.assertTrue(m.data(m.indexFromName('fldtxt'), QgsFieldModel.ExpressionValidityRole))
        self.assertTrue(m.data(m.indexFromName('fldint'), QgsFieldModel.ExpressionValidityRole))
        self.assertFalse(m.data(m.indexFromName('an expression'), QgsFieldModel.ExpressionValidityRole))
        self.assertFalse(m.data(m.indexFromName(None), QgsFieldModel.ExpressionValidityRole))
        m.setAllowExpression(True)
        m.setExpression('an expression')
        self.assertFalse(m.data(m.indexFromName('an expression'), QgsFieldModel.ExpressionValidityRole))
        m.setAllowEmptyFieldName(True)
        self.assertTrue(m.data(m.indexFromName(None), QgsFieldModel.ExpressionValidityRole))

    def testFieldTypeRole(self):
        l, m = create_model()
        self.assertEqual(m.data(m.indexFromName('fldtxt'), QgsFieldModel.FieldTypeRole), QVariant.String)
        self.assertEqual(m.data(m.indexFromName('fldint'), QgsFieldModel.FieldTypeRole), QVariant.Int)
        self.assertFalse(m.data(m.indexFromName('an expression'), QgsFieldModel.FieldTypeRole))
        self.assertFalse(m.data(m.indexFromName(None), QgsFieldModel.FieldTypeRole))
        m.setAllowExpression(True)
        m.setExpression('an expression')
        self.assertFalse(m.data(m.indexFromName('an expression'), QgsFieldModel.FieldTypeRole))
        m.setAllowEmptyFieldName(True)
        self.assertFalse(m.data(m.indexFromName(None), QgsFieldModel.FieldTypeRole))

    def testFieldOriginRole(self):
        l, m = create_model()
        self.assertEqual(m.data(m.indexFromName('fldtxt'), QgsFieldModel.FieldOriginRole), QgsFields.OriginProvider)
        self.assertEqual(m.data(m.indexFromName('fldint'), QgsFieldModel.FieldOriginRole), QgsFields.OriginProvider)
        self.assertFalse(m.data(m.indexFromName('an expression'), QgsFieldModel.FieldOriginRole))
        self.assertFalse(m.data(m.indexFromName(None), QgsFieldModel.FieldOriginRole))
        m.setAllowExpression(True)
        m.setExpression('an expression')
        self.assertFalse(m.data(m.indexFromName('an expression'), QgsFieldModel.FieldOriginRole))
        m.setAllowEmptyFieldName(True)
        self.assertFalse(m.data(m.indexFromName(None), QgsFieldModel.FieldOriginRole))

    def testIsEmptyRole(self):
        l, m = create_model()
        self.assertFalse(m.data(m.indexFromName('fldtxt'), QgsFieldModel.IsEmptyRole), QgsFields.OriginProvider)
        self.assertFalse(m.data(m.indexFromName('fldint'), QgsFieldModel.IsEmptyRole), QgsFields.OriginProvider)
        self.assertFalse(m.data(m.indexFromName('an expression'), QgsFieldModel.IsEmptyRole))
        self.assertFalse(m.data(m.indexFromName(None), QgsFieldModel.IsEmptyRole))
        m.setAllowExpression(True)
        m.setExpression('an expression')
        self.assertFalse(m.data(m.indexFromName('an expression'), QgsFieldModel.IsEmptyRole))
        m.setAllowEmptyFieldName(True)
        self.assertTrue(m.data(m.indexFromName(None), QgsFieldModel.IsEmptyRole))

    def testDisplayRole(self):
        l, m = create_model()
        self.assertEqual(m.data(m.indexFromName('fldtxt'), Qt.DisplayRole), 'fldtxt')
        self.assertEqual(m.data(m.indexFromName('fldint'), Qt.DisplayRole), 'fldint')
        self.assertFalse(m.data(m.indexFromName('an expression'), Qt.DisplayRole))
        self.assertFalse(m.data(m.indexFromName(None), Qt.DisplayRole))
        m.setAllowExpression(True)
        m.setExpression('an expression')
        self.assertEqual(m.data(m.indexFromName('an expression'), Qt.DisplayRole), 'an expression')
        m.setAllowEmptyFieldName(True)
        self.assertFalse(m.data(m.indexFromName(None), Qt.DisplayRole))

    def testEditorWidgetTypeRole(self):
        l, m = create_model()
        self.assertEqual(m.data(m.indexFromName('fldtxt'), QgsFieldModel.EditorWidgetType), 'Hidden')
        self.assertEqual(m.data(m.indexFromName('fldint'), QgsFieldModel.EditorWidgetType), 'ValueMap')
        self.assertIsNone(m.data(m.indexFromName('an expression'), QgsFieldModel.EditorWidgetType))
        self.assertIsNone(m.data(m.indexFromName(None), QgsFieldModel.EditorWidgetType))
        m.setAllowExpression(True)
        m.setExpression('an expression')
        self.assertIsNone(m.data(m.indexFromName('an expression'), QgsFieldModel.EditorWidgetType))
        m.setAllowEmptyFieldName(True)
        self.assertIsNone(m.data(m.indexFromName(None), QgsFieldModel.EditorWidgetType))

    def testJoinedFieldIsEditableRole(self):
        layer = QgsVectorLayer("Point?field=id_a:integer",
                               "addfeat", "memory")
        layer2 = QgsVectorLayer("Point?field=id_b:integer&field=value_b",
                                "addfeat", "memory")
        QgsProject.instance().addMapLayers([layer, layer2])

        # editable join
        join_info = QgsVectorLayerJoinInfo()
        join_info.setTargetFieldName("id_a")
        join_info.setJoinLayer(layer2)
        join_info.setJoinFieldName("id_b")
        join_info.setPrefix("B_")
        join_info.setEditable(True)
        join_info.setUpsertOnEdit(True)
        layer.addJoin(join_info)

        m = QgsFieldModel()
        m.setLayer(layer)

        self.assertIsNone(m.data(m.indexFromName('id_a'), QgsFieldModel.JoinedFieldIsEditable))
        self.assertTrue(m.data(m.indexFromName('B_value_b'), QgsFieldModel.JoinedFieldIsEditable))
        self.assertIsNone(m.data(m.indexFromName('an expression'), QgsFieldModel.JoinedFieldIsEditable))
        self.assertIsNone(m.data(m.indexFromName(None), QgsFieldModel.JoinedFieldIsEditable))
        m.setAllowExpression(True)
        m.setExpression('an expression')
        self.assertIsNone(m.data(m.indexFromName('an expression'), QgsFieldModel.JoinedFieldIsEditable))
        m.setAllowEmptyFieldName(True)
        self.assertIsNone(m.data(m.indexFromName(None), QgsFieldModel.JoinedFieldIsEditable))

        proxy_m = QgsFieldProxyModel()
        proxy_m.setFilters(QgsFieldProxyModel.AllTypes | QgsFieldProxyModel.HideReadOnly)
        proxy_m.sourceFieldModel().setLayer(layer)
        self.assertEqual(proxy_m.rowCount(), 2)
        self.assertEqual(proxy_m.data(proxy_m.index(0, 0)), 'id_a')
        self.assertEqual(proxy_m.data(proxy_m.index(1, 0)), 'B_value_b')

        # not editable join
        layer3 = QgsVectorLayer("Point?field=id_a:integer",
                                "addfeat", "memory")
        QgsProject.instance().addMapLayers([layer3])
        join_info = QgsVectorLayerJoinInfo()
        join_info.setTargetFieldName("id_a")
        join_info.setJoinLayer(layer2)
        join_info.setJoinFieldName("id_b")
        join_info.setPrefix("B_")
        join_info.setEditable(False)

        layer3.addJoin(join_info)
        m = QgsFieldModel()
        m.setLayer(layer3)

        self.assertIsNone(m.data(m.indexFromName('id_a'), QgsFieldModel.JoinedFieldIsEditable))
        self.assertFalse(m.data(m.indexFromName('B_value_b'), QgsFieldModel.JoinedFieldIsEditable))
        self.assertIsNone(m.data(m.indexFromName('an expression'), QgsFieldModel.JoinedFieldIsEditable))
        self.assertIsNone(m.data(m.indexFromName(None), QgsFieldModel.JoinedFieldIsEditable))
        m.setAllowExpression(True)
        m.setExpression('an expression')
        self.assertIsNone(m.data(m.indexFromName('an expression'), QgsFieldModel.JoinedFieldIsEditable))
        m.setAllowEmptyFieldName(True)
        self.assertIsNone(m.data(m.indexFromName(None), QgsFieldModel.JoinedFieldIsEditable))

        proxy_m = QgsFieldProxyModel()
        proxy_m.sourceFieldModel().setLayer(layer3)
        proxy_m.setFilters(QgsFieldProxyModel.AllTypes | QgsFieldProxyModel.HideReadOnly)
        self.assertEqual(proxy_m.rowCount(), 1)
        self.assertEqual(proxy_m.data(proxy_m.index(0, 0)), 'id_a')

    def testFieldTooltip(self):
        f = QgsField('my_string', QVariant.String, 'string')
        self.assertEqual(QgsFieldModel.fieldToolTip(f), '<b>my_string</b><p>string</p>')
        f.setAlias('my alias')
        self.assertEqual(QgsFieldModel.fieldToolTip(f), '<b>my alias</b> (my_string)<p>string</p>')
        f.setLength(20)
        self.assertEqual(QgsFieldModel.fieldToolTip(f), '<b>my alias</b> (my_string)<p>string (20)</p>')
        f = QgsField('my_real', QVariant.Double, 'real', 8, 3)
        self.assertEqual(QgsFieldModel.fieldToolTip(f), '<b>my_real</b><p>real (8, 3)</p>')


if __name__ == '__main__':
    unittest.main()
