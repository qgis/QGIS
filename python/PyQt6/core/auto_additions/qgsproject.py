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
QgsProject.DataDefinedServerProperty.__doc__ = "Data defined properties.\nOverrides of user defined server parameters are stored in a\nproperty collection and they can be retrieved using the\nindexes specified in this enum.\n\n.. versionadded:: 3.14\n\n" + '* ``NoProperty``: ' + QgsProject.DataDefinedServerProperty.NoProperty.__doc__ + '\n' + '* ``AllProperties``: ' + QgsProject.DataDefinedServerProperty.AllProperties.__doc__ + '\n' + '* ``WMSOnlineResource``: ' + QgsProject.DataDefinedServerProperty.WMSOnlineResource.__doc__
# --
