# The following has been generated automatically from src/core/qgsdatabaseschemamodel.h
QgsDatabaseSchemaModel.Role = QgsDatabaseSchemaModel.CustomRole
# monkey patching scoped based enum
QgsDatabaseSchemaModel.RoleEmpty = QgsDatabaseSchemaModel.CustomRole.Empty
QgsDatabaseSchemaModel.Role.RoleEmpty = QgsDatabaseSchemaModel.CustomRole.Empty
QgsDatabaseSchemaModel.RoleEmpty.is_monkey_patched = True
QgsDatabaseSchemaModel.RoleEmpty.__doc__ = "Entry is an empty entry"
QgsDatabaseSchemaModel.CustomRole.__doc__ = "Custom model roles.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as QgsDatabaseSchemaModel.Role\n\n.. versionadded:: 3.36\n\n" + '* ``RoleEmpty``: ' + QgsDatabaseSchemaModel.CustomRole.Empty.__doc__
# --
QgsDatabaseSchemaModel.CustomRole.baseClass = QgsDatabaseSchemaModel
