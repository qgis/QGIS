# The following has been generated automatically from src/core/qgsproviderconnectionmodel.h
QgsProviderConnectionModel.Role = QgsProviderConnectionModel.CustomRole
# monkey patching scoped based enum
QgsProviderConnectionModel.RoleConnectionName = QgsProviderConnectionModel.CustomRole.ConnectionName
QgsProviderConnectionModel.Role.RoleConnectionName = QgsProviderConnectionModel.CustomRole.ConnectionName
QgsProviderConnectionModel.RoleConnectionName.is_monkey_patched = True
QgsProviderConnectionModel.RoleConnectionName.__doc__ = "Connection name"
QgsProviderConnectionModel.RoleUri = QgsProviderConnectionModel.CustomRole.Uri
QgsProviderConnectionModel.Role.RoleUri = QgsProviderConnectionModel.CustomRole.Uri
QgsProviderConnectionModel.RoleUri.is_monkey_patched = True
QgsProviderConnectionModel.RoleUri.__doc__ = "Connection URI string"
QgsProviderConnectionModel.RoleConfiguration = QgsProviderConnectionModel.CustomRole.Configuration
QgsProviderConnectionModel.Role.RoleConfiguration = QgsProviderConnectionModel.CustomRole.Configuration
QgsProviderConnectionModel.RoleConfiguration.is_monkey_patched = True
QgsProviderConnectionModel.RoleConfiguration.__doc__ = "Connection configuration variant map"
QgsProviderConnectionModel.RoleEmpty = QgsProviderConnectionModel.CustomRole.Empty
QgsProviderConnectionModel.Role.RoleEmpty = QgsProviderConnectionModel.CustomRole.Empty
QgsProviderConnectionModel.RoleEmpty.is_monkey_patched = True
QgsProviderConnectionModel.RoleEmpty.__doc__ = "Entry is an empty entry"
QgsProviderConnectionModel.CustomRole.__doc__ = """Custom model roles.

.. note::

   Prior to QGIS 3.36 this was available as QgsProviderConnectionModel.Role

.. versionadded:: 3.36

* ``RoleConnectionName``: Connection name
* ``RoleUri``: Connection URI string
* ``RoleConfiguration``: Connection configuration variant map
* ``RoleEmpty``: Entry is an empty entry

"""
# --
QgsProviderConnectionModel.CustomRole.baseClass = QgsProviderConnectionModel
