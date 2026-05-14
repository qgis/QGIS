# The following has been generated automatically from src/core/project/qgsproject.h
# monkey patching scoped based enum
QgsProject.NoProperty = QgsProject.DataDefinedServerProperty.NoProperty
QgsProject.NoProperty.is_monkey_patched = True
QgsProject.NoProperty.__doc__ = "No property"
QgsProject.AllProperties = QgsProject.DataDefinedServerProperty.AllProperties
QgsProject.AllProperties.is_monkey_patched = True
QgsProject.AllProperties.__doc__ = "All properties for item"
QgsProject.WMSOnlineResource = QgsProject.DataDefinedServerProperty.WMSOnlineResource
QgsProject.WMSOnlineResource.is_monkey_patched = True
QgsProject.WMSOnlineResource.__doc__ = "Alias"
QgsProject.DataDefinedServerProperty.__doc__ = """Data defined properties.
Overrides of user defined server parameters are stored in a
property collection and they can be retrieved using the
indexes specified in this enum.

.. versionadded:: 3.14

* ``NoProperty``: No property
* ``AllProperties``: All properties for item
* ``WMSOnlineResource``: Alias

"""
# --
try:
    QgsProject.instance = staticmethod(QgsProject.instance)
    QgsProject.setInstance = staticmethod(QgsProject.setInstance)
    QgsProject.__overridden_methods__ = ['createExpressionContext', 'createExpressionContextScope', 'translate']
    QgsProject.__group__ = ['project']
except (NameError, AttributeError):
    pass
try:
    QgsProjectDirtyBlocker.__group__ = ['project']
except (NameError, AttributeError):
    pass
