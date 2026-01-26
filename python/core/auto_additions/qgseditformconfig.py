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
QgsEditFormConfig.DataDefinedProperty.__doc__ = """Data defined properties.
Form data defined overrides are stored in a property collection
and they can be retrieved using the indexes specified in this
enum.

.. versionadded:: 3.14

* ``NoProperty``: No property
* ``AllProperties``: All properties for item
* ``Alias``: Alias
* ``Editable``: Editable state

  .. versionadded:: 3.30


"""
# --
try:
    QgsEditFormConfig.__group__ = ['editform']
except (NameError, AttributeError):
    pass
try:
    QgsEditFormConfig.GroupData.__group__ = ['editform']
except (NameError, AttributeError):
    pass
try:
    QgsEditFormConfig.TabData.__group__ = ['editform']
except (NameError, AttributeError):
    pass
