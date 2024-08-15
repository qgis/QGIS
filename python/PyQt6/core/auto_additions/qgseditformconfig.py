# The following has been generated automatically from src/core/editform/qgseditformconfig.h
# monkey patching scoped based enum
QgsEditFormConfig.NoProperty = QgsEditFormConfig.DataDefinedProperty.NoProperty
QgsEditFormConfig.NoProperty.is_monkey_patched = True
QgsEditFormConfig.NoProperty.__doc__ = "No property"
QgsEditFormConfig.AllProperties = QgsEditFormConfig.DataDefinedProperty.AllProperties
QgsEditFormConfig.AllProperties.is_monkey_patched = True
QgsEditFormConfig.AllProperties.__doc__ = "All properties for item"
QgsEditFormConfig.Alias = QgsEditFormConfig.DataDefinedProperty.Alias
QgsEditFormConfig.Alias.is_monkey_patched = True
QgsEditFormConfig.Alias.__doc__ = "Alias"
QgsEditFormConfig.Editable = QgsEditFormConfig.DataDefinedProperty.Editable
QgsEditFormConfig.Editable.is_monkey_patched = True
QgsEditFormConfig.Editable.__doc__ = "Editable state \n.. versionadded:: 3.30"
QgsEditFormConfig.DataDefinedProperty.__doc__ = "Data defined properties.\nForm data defined overrides are stored in a property collection\nand they can be retrieved using the indexes specified in this\nenum.\n\n.. versionadded:: 3.14\n\n" + '* ``NoProperty``: ' + QgsEditFormConfig.DataDefinedProperty.NoProperty.__doc__ + '\n' + '* ``AllProperties``: ' + QgsEditFormConfig.DataDefinedProperty.AllProperties.__doc__ + '\n' + '* ``Alias``: ' + QgsEditFormConfig.DataDefinedProperty.Alias.__doc__ + '\n' + '* ``Editable``: ' + QgsEditFormConfig.DataDefinedProperty.Editable.__doc__
# --
try:
    QgsEditFormConfig.__group__ = ['editform']
except NameError:
    pass
try:
    QgsEditFormConfig.GroupData.__group__ = ['editform']
except NameError:
    pass
try:
    QgsEditFormConfig.TabData.__group__ = ['editform']
except NameError:
    pass
