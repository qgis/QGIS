# The following has been generated automatically from src/core/qgsattributetableconfig.h
QgsAttributeTableConfig.Field = QgsAttributeTableConfig.Type.Field
QgsAttributeTableConfig.Action = QgsAttributeTableConfig.Type.Action
QgsAttributeTableConfig.ButtonList = QgsAttributeTableConfig.ActionWidgetStyle.ButtonList
QgsAttributeTableConfig.DropDown = QgsAttributeTableConfig.ActionWidgetStyle.DropDown
# monkey patching scoped based enum
QgsAttributeTableConfig.AddFeatureMethod.Unset.__doc__ = "No method set for current layer"
QgsAttributeTableConfig.AddFeatureMethod.Form.__doc__ = "Opens a new Attributeform-Dialog"
QgsAttributeTableConfig.AddFeatureMethod.Table.__doc__ = "Adds a new row (or a form embedded in the attribute table depending on the view)"
QgsAttributeTableConfig.AddFeatureMethod.__doc__ = """The way how to add features in the attribute table

* ``Unset``: No method set for current layer
* ``Form``: Opens a new Attributeform-Dialog
* ``Table``: Adds a new row (or a form embedded in the attribute table depending on the view)

"""
# --
try:
    QgsAttributeTableConfig.ColumnConfig.__attribute_docs__ = {'type': 'The type of this column.', 'name': 'The name of the attribute if this column represents a field', 'hidden': 'Flag that controls if the column is hidden', 'width': 'Width of column, or -1 for default width'}
    QgsAttributeTableConfig.ColumnConfig.__annotations__ = {'type': 'QgsAttributeTableConfig.Type', 'name': str, 'hidden': bool, 'width': int}
    QgsAttributeTableConfig.ColumnConfig.__doc__ = """Defines the configuration of a column in the attribute table."""
except (NameError, AttributeError):
    pass
