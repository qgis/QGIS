# The following has been generated automatically from src/core/qgsfieldmodel.h
QgsFieldModel.FieldRoles = QgsFieldModel.CustomRole
# monkey patching scoped based enum
QgsFieldModel.FieldNameRole = QgsFieldModel.CustomRole.FieldName
QgsFieldModel.FieldRoles.FieldNameRole = QgsFieldModel.CustomRole.FieldName
QgsFieldModel.FieldNameRole.is_monkey_patched = True
QgsFieldModel.FieldNameRole.__doc__ = "Return field name if index corresponds to a field"
QgsFieldModel.FieldIndexRole = QgsFieldModel.CustomRole.FieldIndex
QgsFieldModel.FieldRoles.FieldIndexRole = QgsFieldModel.CustomRole.FieldIndex
QgsFieldModel.FieldIndexRole.is_monkey_patched = True
QgsFieldModel.FieldIndexRole.__doc__ = "Return field index if index corresponds to a field"
QgsFieldModel.ExpressionRole = QgsFieldModel.CustomRole.Expression
QgsFieldModel.FieldRoles.ExpressionRole = QgsFieldModel.CustomRole.Expression
QgsFieldModel.ExpressionRole.is_monkey_patched = True
QgsFieldModel.ExpressionRole.__doc__ = "Return field name or expression"
QgsFieldModel.IsExpressionRole = QgsFieldModel.CustomRole.IsExpression
QgsFieldModel.FieldRoles.IsExpressionRole = QgsFieldModel.CustomRole.IsExpression
QgsFieldModel.IsExpressionRole.is_monkey_patched = True
QgsFieldModel.IsExpressionRole.__doc__ = "Return if index corresponds to an expression"
QgsFieldModel.ExpressionValidityRole = QgsFieldModel.CustomRole.ExpressionValidity
QgsFieldModel.FieldRoles.ExpressionValidityRole = QgsFieldModel.CustomRole.ExpressionValidity
QgsFieldModel.ExpressionValidityRole.is_monkey_patched = True
QgsFieldModel.ExpressionValidityRole.__doc__ = "Return if expression is valid or not"
QgsFieldModel.FieldTypeRole = QgsFieldModel.CustomRole.FieldType
QgsFieldModel.FieldRoles.FieldTypeRole = QgsFieldModel.CustomRole.FieldType
QgsFieldModel.FieldTypeRole.is_monkey_patched = True
QgsFieldModel.FieldTypeRole.__doc__ = "Return the field type (if a field, return QVariant if expression)"
QgsFieldModel.FieldOriginRole = QgsFieldModel.CustomRole.FieldOrigin
QgsFieldModel.FieldRoles.FieldOriginRole = QgsFieldModel.CustomRole.FieldOrigin
QgsFieldModel.FieldOriginRole.is_monkey_patched = True
QgsFieldModel.FieldOriginRole.__doc__ = "Return the field origin (if a field, returns QVariant if expression)"
QgsFieldModel.IsEmptyRole = QgsFieldModel.CustomRole.IsEmpty
QgsFieldModel.FieldRoles.IsEmptyRole = QgsFieldModel.CustomRole.IsEmpty
QgsFieldModel.IsEmptyRole.is_monkey_patched = True
QgsFieldModel.IsEmptyRole.__doc__ = "Return if the index corresponds to the empty value"
QgsFieldModel.EditorWidgetType = QgsFieldModel.CustomRole.EditorWidgetType
QgsFieldModel.EditorWidgetType.is_monkey_patched = True
QgsFieldModel.EditorWidgetType.__doc__ = "Editor widget type"
QgsFieldModel.JoinedFieldIsEditable = QgsFieldModel.CustomRole.JoinedFieldIsEditable
QgsFieldModel.JoinedFieldIsEditable.is_monkey_patched = True
QgsFieldModel.JoinedFieldIsEditable.__doc__ = "``True`` if a joined field is editable (returns QVariant if not a joined field)"
QgsFieldModel.FieldIsWidgetEditable = QgsFieldModel.CustomRole.FieldIsWidgetEditable
QgsFieldModel.FieldIsWidgetEditable.is_monkey_patched = True
QgsFieldModel.FieldIsWidgetEditable.__doc__ = "``True`` if a is editable from the widget"
QgsFieldModel.CustomRole.__doc__ = """Custom model roles.

.. note::

   Prior to QGIS 3.36 this was available as QgsFieldModel.FieldRoles

.. versionadded:: 3.36

* ``FieldNameRole``: Return field name if index corresponds to a field
* ``FieldIndexRole``: Return field index if index corresponds to a field
* ``ExpressionRole``: Return field name or expression
* ``IsExpressionRole``: Return if index corresponds to an expression
* ``ExpressionValidityRole``: Return if expression is valid or not
* ``FieldTypeRole``: Return the field type (if a field, return QVariant if expression)
* ``FieldOriginRole``: Return the field origin (if a field, returns QVariant if expression)
* ``IsEmptyRole``: Return if the index corresponds to the empty value
* ``EditorWidgetType``: Editor widget type
* ``JoinedFieldIsEditable``: ``True`` if a joined field is editable (returns QVariant if not a joined field)
* ``FieldIsWidgetEditable``: ``True`` if a is editable from the widget

"""
# --
QgsFieldModel.CustomRole.baseClass = QgsFieldModel
QgsFieldModel.fieldToolTip = staticmethod(QgsFieldModel.fieldToolTip)
QgsFieldModel.fieldToolTipExtended = staticmethod(QgsFieldModel.fieldToolTipExtended)
