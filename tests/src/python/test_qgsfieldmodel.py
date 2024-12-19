"""QGIS Unit tests for QgsFieldModel

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '14/11/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'

from qgis.PyQt.QtCore import QModelIndex, Qt, QVariant
from qgis.core import (
    QgsEditorWidgetSetup,
    QgsField,
    QgsFieldConstraints,
    QgsFieldModel,
    QgsFieldProxyModel,
    QgsFields,
    QgsProject,
    QgsVectorLayer,
    QgsVectorLayerJoinInfo,
)
import unittest
from qgis.testing import start_app, QgisTestCase

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


class TestQgsFieldModel(QgisTestCase):

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

        fields = QgsFields()
        fields.append(QgsField('test1', QVariant.String))
        fields.append(QgsField('test2', QVariant.String))
        m.setFields(fields)
        self.assertIsNone(m.layer())
        self.assertEqual(m.fields(), fields)

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
        self.assertEqual(m.data(m.indexFromName('fldtxt'), QgsFieldModel.FieldRoles.FieldNameRole), 'fldtxt')
        self.assertEqual(m.data(m.indexFromName('fldint'), QgsFieldModel.FieldRoles.FieldNameRole), 'fldint')
        self.assertFalse(m.data(m.indexFromName('an expression'), QgsFieldModel.FieldRoles.FieldNameRole))
        self.assertFalse(m.data(m.indexFromName(None), QgsFieldModel.FieldRoles.FieldNameRole))
        m.setAllowExpression(True)
        m.setExpression('an expression')
        self.assertFalse(m.data(m.indexFromName('an expression'), QgsFieldModel.FieldRoles.FieldNameRole))
        m.setAllowEmptyFieldName(True)
        self.assertFalse(m.data(m.indexFromName(None), QgsFieldModel.FieldRoles.FieldNameRole))

    def testExpressionRole(self):
        l, m = create_model()
        self.assertEqual(m.data(m.indexFromName('fldtxt'), QgsFieldModel.FieldRoles.ExpressionRole), 'fldtxt')
        self.assertEqual(m.data(m.indexFromName('fldint'), QgsFieldModel.FieldRoles.ExpressionRole), 'fldint')
        self.assertFalse(m.data(m.indexFromName('an expression'), QgsFieldModel.FieldRoles.ExpressionRole))
        self.assertFalse(m.data(m.indexFromName(None), QgsFieldModel.FieldRoles.ExpressionRole))
        m.setAllowExpression(True)
        m.setExpression('an expression')
        self.assertEqual(m.data(m.indexFromName('an expression'), QgsFieldModel.FieldRoles.ExpressionRole), 'an expression')
        m.setAllowEmptyFieldName(True)
        self.assertFalse(m.data(m.indexFromName(None), QgsFieldModel.FieldRoles.ExpressionRole))

    def testFieldIndexRole(self):
        l, m = create_model()
        self.assertEqual(m.data(m.indexFromName('fldtxt'), QgsFieldModel.FieldRoles.FieldIndexRole), 0)
        self.assertEqual(m.data(m.indexFromName('fldint'), QgsFieldModel.FieldRoles.FieldIndexRole), 1)
        self.assertFalse(m.data(m.indexFromName('an expression'), QgsFieldModel.FieldRoles.FieldIndexRole))
        self.assertFalse(m.data(m.indexFromName(None), QgsFieldModel.FieldRoles.FieldIndexRole))
        m.setAllowExpression(True)
        m.setExpression('an expression')
        self.assertFalse(m.data(m.indexFromName('an expression'), QgsFieldModel.FieldRoles.FieldIndexRole))
        m.setAllowEmptyFieldName(True)
        self.assertFalse(m.data(m.indexFromName(None), QgsFieldModel.FieldRoles.FieldIndexRole))

    def testIsExpressionRole(self):
        l, m = create_model()
        self.assertFalse(m.data(m.indexFromName('fldtxt'), QgsFieldModel.FieldRoles.IsExpressionRole))
        self.assertFalse(m.data(m.indexFromName('fldint'), QgsFieldModel.FieldRoles.IsExpressionRole))
        self.assertFalse(m.data(m.indexFromName('an expression'), QgsFieldModel.FieldRoles.IsExpressionRole))
        self.assertFalse(m.data(m.indexFromName(None), QgsFieldModel.FieldRoles.IsExpressionRole))
        m.setAllowExpression(True)
        m.setExpression('an expression')
        self.assertTrue(m.data(m.indexFromName('an expression'), QgsFieldModel.FieldRoles.IsExpressionRole))
        m.setAllowEmptyFieldName(True)
        self.assertFalse(m.data(m.indexFromName(None), QgsFieldModel.FieldRoles.IsExpressionRole))

    def testExpressionValidityRole(self):
        l, m = create_model()
        self.assertTrue(m.data(m.indexFromName('fldtxt'), QgsFieldModel.FieldRoles.ExpressionValidityRole))
        self.assertTrue(m.data(m.indexFromName('fldint'), QgsFieldModel.FieldRoles.ExpressionValidityRole))
        self.assertFalse(m.data(m.indexFromName('an expression'), QgsFieldModel.FieldRoles.ExpressionValidityRole))
        self.assertFalse(m.data(m.indexFromName(None), QgsFieldModel.FieldRoles.ExpressionValidityRole))
        m.setAllowExpression(True)
        m.setExpression('an expression')
        self.assertFalse(m.data(m.indexFromName('an expression'), QgsFieldModel.FieldRoles.ExpressionValidityRole))
        m.setAllowEmptyFieldName(True)
        self.assertTrue(m.data(m.indexFromName(None), QgsFieldModel.FieldRoles.ExpressionValidityRole))

    def testFieldTypeRole(self):
        l, m = create_model()
        self.assertEqual(m.data(m.indexFromName('fldtxt'), QgsFieldModel.FieldRoles.FieldTypeRole), QVariant.String)
        self.assertEqual(m.data(m.indexFromName('fldint'), QgsFieldModel.FieldRoles.FieldTypeRole), QVariant.Int)
        self.assertFalse(m.data(m.indexFromName('an expression'), QgsFieldModel.FieldRoles.FieldTypeRole))
        self.assertFalse(m.data(m.indexFromName(None), QgsFieldModel.FieldRoles.FieldTypeRole))
        m.setAllowExpression(True)
        m.setExpression('an expression')
        self.assertFalse(m.data(m.indexFromName('an expression'), QgsFieldModel.FieldRoles.FieldTypeRole))
        m.setAllowEmptyFieldName(True)
        self.assertFalse(m.data(m.indexFromName(None), QgsFieldModel.FieldRoles.FieldTypeRole))

    def testFieldOriginRole(self):
        l, m = create_model()
        self.assertEqual(m.data(m.indexFromName('fldtxt'), QgsFieldModel.FieldRoles.FieldOriginRole), QgsFields.FieldOrigin.OriginProvider)
        self.assertEqual(m.data(m.indexFromName('fldint'), QgsFieldModel.FieldRoles.FieldOriginRole), QgsFields.FieldOrigin.OriginProvider)
        self.assertFalse(m.data(m.indexFromName('an expression'), QgsFieldModel.FieldRoles.FieldOriginRole))
        self.assertFalse(m.data(m.indexFromName(None), QgsFieldModel.FieldRoles.FieldOriginRole))
        m.setAllowExpression(True)
        m.setExpression('an expression')
        self.assertFalse(m.data(m.indexFromName('an expression'), QgsFieldModel.FieldRoles.FieldOriginRole))
        m.setAllowEmptyFieldName(True)
        self.assertFalse(m.data(m.indexFromName(None), QgsFieldModel.FieldRoles.FieldOriginRole))

    def testIsEmptyRole(self):
        l, m = create_model()
        self.assertFalse(m.data(m.indexFromName('fldtxt'), QgsFieldModel.FieldRoles.IsEmptyRole), QgsFields.FieldOrigin.OriginProvider)
        self.assertFalse(m.data(m.indexFromName('fldint'), QgsFieldModel.FieldRoles.IsEmptyRole), QgsFields.FieldOrigin.OriginProvider)
        self.assertFalse(m.data(m.indexFromName('an expression'), QgsFieldModel.FieldRoles.IsEmptyRole))
        self.assertFalse(m.data(m.indexFromName(None), QgsFieldModel.FieldRoles.IsEmptyRole))
        m.setAllowExpression(True)
        m.setExpression('an expression')
        self.assertFalse(m.data(m.indexFromName('an expression'), QgsFieldModel.FieldRoles.IsEmptyRole))
        m.setAllowEmptyFieldName(True)
        self.assertTrue(m.data(m.indexFromName(None), QgsFieldModel.FieldRoles.IsEmptyRole))

    def testDisplayRole(self):
        l, m = create_model()
        self.assertEqual(m.data(m.indexFromName('fldtxt'), Qt.ItemDataRole.DisplayRole), 'fldtxt')
        self.assertEqual(m.data(m.indexFromName('fldint'), Qt.ItemDataRole.DisplayRole), 'fldint')
        self.assertFalse(m.data(m.indexFromName('an expression'), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(m.data(m.indexFromName(None), Qt.ItemDataRole.DisplayRole))
        m.setAllowExpression(True)
        m.setExpression('an expression')
        self.assertEqual(m.data(m.indexFromName('an expression'), Qt.ItemDataRole.DisplayRole), 'an expression')
        m.setAllowEmptyFieldName(True)
        self.assertFalse(m.data(m.indexFromName(None), Qt.ItemDataRole.DisplayRole))

    def testManualFields(self):
        _, m = create_model()
        fields = QgsFields()
        fields.append(QgsField('f1', QVariant.String))
        fields.append(QgsField('f2', QVariant.String))
        m.setFields(fields)
        self.assertEqual(m.rowCount(), 2)
        self.assertEqual(m.data(m.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole), 'f1')
        self.assertEqual(m.data(m.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole), 'f2')

    def testEditorWidgetTypeRole(self):
        l, m = create_model()
        self.assertEqual(m.data(m.indexFromName('fldtxt'), QgsFieldModel.FieldRoles.EditorWidgetType), 'Hidden')
        self.assertEqual(m.data(m.indexFromName('fldint'), QgsFieldModel.FieldRoles.EditorWidgetType), 'ValueMap')
        self.assertIsNone(m.data(m.indexFromName('an expression'), QgsFieldModel.FieldRoles.EditorWidgetType))
        self.assertIsNone(m.data(m.indexFromName(None), QgsFieldModel.FieldRoles.EditorWidgetType))
        m.setAllowExpression(True)
        m.setExpression('an expression')
        self.assertIsNone(m.data(m.indexFromName('an expression'), QgsFieldModel.FieldRoles.EditorWidgetType))
        m.setAllowEmptyFieldName(True)
        self.assertIsNone(m.data(m.indexFromName(None), QgsFieldModel.FieldRoles.EditorWidgetType))

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

        self.assertIsNone(m.data(m.indexFromName('id_a'), QgsFieldModel.FieldRoles.JoinedFieldIsEditable))
        self.assertTrue(m.data(m.indexFromName('B_value_b'), QgsFieldModel.FieldRoles.JoinedFieldIsEditable))
        self.assertIsNone(m.data(m.indexFromName('an expression'), QgsFieldModel.FieldRoles.JoinedFieldIsEditable))
        self.assertIsNone(m.data(m.indexFromName(None), QgsFieldModel.FieldRoles.JoinedFieldIsEditable))
        m.setAllowExpression(True)
        m.setExpression('an expression')
        self.assertIsNone(m.data(m.indexFromName('an expression'), QgsFieldModel.FieldRoles.JoinedFieldIsEditable))
        m.setAllowEmptyFieldName(True)
        self.assertIsNone(m.data(m.indexFromName(None), QgsFieldModel.FieldRoles.JoinedFieldIsEditable))

        proxy_m = QgsFieldProxyModel()
        proxy_m.setFilters(QgsFieldProxyModel.Filter.AllTypes | QgsFieldProxyModel.Filter.HideReadOnly)
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

        self.assertIsNone(m.data(m.indexFromName('id_a'), QgsFieldModel.FieldRoles.JoinedFieldIsEditable))
        self.assertFalse(m.data(m.indexFromName('B_value_b'), QgsFieldModel.FieldRoles.JoinedFieldIsEditable))
        self.assertIsNone(m.data(m.indexFromName('an expression'), QgsFieldModel.FieldRoles.JoinedFieldIsEditable))
        self.assertIsNone(m.data(m.indexFromName(None), QgsFieldModel.FieldRoles.JoinedFieldIsEditable))
        m.setAllowExpression(True)
        m.setExpression('an expression')
        self.assertIsNone(m.data(m.indexFromName('an expression'), QgsFieldModel.FieldRoles.JoinedFieldIsEditable))
        m.setAllowEmptyFieldName(True)
        self.assertIsNone(m.data(m.indexFromName(None), QgsFieldModel.FieldRoles.JoinedFieldIsEditable))

        proxy_m = QgsFieldProxyModel()
        proxy_m.sourceFieldModel().setLayer(layer3)
        proxy_m.setFilters(QgsFieldProxyModel.Filter.AllTypes | QgsFieldProxyModel.Filter.HideReadOnly)
        self.assertEqual(proxy_m.rowCount(), 1)
        self.assertEqual(proxy_m.data(proxy_m.index(0, 0)), 'id_a')

        proxy_m.setFilters(QgsFieldProxyModel.Filter.AllTypes | QgsFieldProxyModel.Filter.OriginProvider)
        proxy_m.sourceFieldModel().setLayer(layer)
        self.assertEqual(proxy_m.rowCount(), 1)
        self.assertEqual(proxy_m.data(proxy_m.index(0, 0)), 'id_a')
        proxy_m.sourceFieldModel().setLayer(layer3)
        self.assertEqual(proxy_m.rowCount(), 1)
        self.assertEqual(proxy_m.data(proxy_m.index(0, 0)), 'id_a')

    def testFieldIsWidgetEditableRole(self):
        l, m = create_model()
        self.assertTrue(m.data(m.indexFromName('fldtxt'), QgsFieldModel.FieldRoles.FieldIsWidgetEditable))
        self.assertTrue(m.data(m.indexFromName('fldint'), QgsFieldModel.FieldRoles.FieldIsWidgetEditable))
        self.assertFalse(m.data(m.indexFromName('an expression'), QgsFieldModel.FieldRoles.FieldIsWidgetEditable))
        self.assertFalse(m.data(m.indexFromName(None), QgsFieldModel.FieldRoles.FieldIsWidgetEditable))
        m.setAllowExpression(True)
        m.setExpression('an expression')
        self.assertTrue(m.data(m.indexFromName('an expression'), QgsFieldModel.FieldRoles.FieldIsWidgetEditable))
        m.setAllowEmptyFieldName(True)
        self.assertTrue(m.data(m.indexFromName(None), QgsFieldModel.FieldRoles.FieldIsWidgetEditable))

        editFormConfig = l.editFormConfig()
        idx = l.fields().indexOf('fldtxt')
        # Make fldtxt readOnly
        editFormConfig.setReadOnly(idx, True)
        l.setEditFormConfig(editFormConfig)
        # It's read only, so the widget is NOT editable
        self.assertFalse(m.data(m.indexFromName('fldtxt'), QgsFieldModel.FieldRoles.FieldIsWidgetEditable))

    def testFieldTooltip(self):
        f = QgsField('my_string', QVariant.String, 'string')
        self.assertEqual(QgsFieldModel.fieldToolTip(f), "<b>my_string</b><br><font style='font-family:monospace; white-space: nowrap;'>string NULL</font>")
        f.setAlias('my alias')
        self.assertEqual(QgsFieldModel.fieldToolTip(f), "<b>my alias</b> (my_string)<br><font style='font-family:monospace; white-space: nowrap;'>string NULL</font>")
        f.setLength(20)
        self.assertEqual(QgsFieldModel.fieldToolTip(f), "<b>my alias</b> (my_string)<br><font style='font-family:monospace; white-space: nowrap;'>string(20) NULL</font>")
        f = QgsField('my_real', QVariant.Double, 'real', 8, 3)
        self.assertEqual(QgsFieldModel.fieldToolTip(f), "<b>my_real</b><br><font style='font-family:monospace; white-space: nowrap;'>real(8, 3) NULL</font>")
        f.setComment('Comment text')
        self.assertEqual(QgsFieldModel.fieldToolTip(f), "<b>my_real</b><br><font style='font-family:monospace; white-space: nowrap;'>real(8, 3) NULL</font><br><em>Comment text</em>")

    def testFieldTooltipExtended(self):
        layer = QgsVectorLayer("Point?", "tooltip", "memory")
        f = QgsField('my_real', QVariant.Double, 'real', 8, 3, 'Comment text')
        layer.addExpressionField('1+1', f)
        layer.updateFields()
        self.assertEqual(QgsFieldModel.fieldToolTipExtended(QgsField('my_string', QVariant.String, 'string'), layer), '')
        self.assertEqual(QgsFieldModel.fieldToolTipExtended(f, layer), "<b>my_real</b><br><font style='font-family:monospace; white-space: nowrap;'>real(8, 3) NULL</font><br><em>Comment text</em><br><font style='font-family:monospace;'>1+1</font>")
        f.setAlias('my alias')
        constraints = f.constraints()
        constraints.setConstraint(QgsFieldConstraints.Constraint.ConstraintUnique)
        f.setConstraints(constraints)
        self.assertEqual(QgsFieldModel.fieldToolTipExtended(f, layer), "<b>my alias</b> (my_real)<br><font style='font-family:monospace; white-space: nowrap;'>real(8, 3) NULL UNIQUE</font><br><em>Comment text</em><br><font style='font-family:monospace;'>1+1</font>")


if __name__ == '__main__':
    unittest.main()
