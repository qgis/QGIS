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
QgsProviderConnectionModel.CustomRole.__doc__ = "Custom model roles.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as QgsProviderConnectionModel.Role\n\n.. versionadded:: 3.36\n\n" + '* ``RoleConnectionName``: ' + QgsProviderConnectionModel.CustomRole.ConnectionName.__doc__ + '\n' + '* ``RoleUri``: ' + QgsProviderConnectionModel.CustomRole.Uri.__doc__ + '\n' + '* ``RoleConfiguration``: ' + QgsProviderConnectionModel.CustomRole.Configuration.__doc__ + '\n' + '* ``RoleEmpty``: ' + QgsProviderConnectionModel.CustomRole.Empty.__doc__
# --
QgsProviderConnectionModel.CustomRole.baseClass = QgsProviderConnectionModel
