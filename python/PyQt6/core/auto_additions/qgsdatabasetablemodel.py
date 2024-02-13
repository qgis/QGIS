# The following has been generated automatically from src/core/qgsdatabasetablemodel.h
QgsDatabaseTableModel.Role = QgsDatabaseTableModel.CustomRole
# monkey patching scoped based enum
QgsDatabaseTableModel.RoleTableName = QgsDatabaseTableModel.CustomRole.TableName
QgsDatabaseTableModel.Role.RoleTableName = QgsDatabaseTableModel.CustomRole.TableName
QgsDatabaseTableModel.RoleTableName.is_monkey_patched = True
QgsDatabaseTableModel.RoleTableName.__doc__ = "Table name"
QgsDatabaseTableModel.RoleSchema = QgsDatabaseTableModel.CustomRole.Schema
QgsDatabaseTableModel.Role.RoleSchema = QgsDatabaseTableModel.CustomRole.Schema
QgsDatabaseTableModel.RoleSchema.is_monkey_patched = True
QgsDatabaseTableModel.RoleSchema.__doc__ = "Table schema"
QgsDatabaseTableModel.RoleTableFlags = QgsDatabaseTableModel.CustomRole.TableFlags
QgsDatabaseTableModel.Role.RoleTableFlags = QgsDatabaseTableModel.CustomRole.TableFlags
QgsDatabaseTableModel.RoleTableFlags.is_monkey_patched = True
QgsDatabaseTableModel.RoleTableFlags.__doc__ = "Table flags role"
QgsDatabaseTableModel.RoleComment = QgsDatabaseTableModel.CustomRole.Comment
QgsDatabaseTableModel.Role.RoleComment = QgsDatabaseTableModel.CustomRole.Comment
QgsDatabaseTableModel.RoleComment.is_monkey_patched = True
QgsDatabaseTableModel.RoleComment.__doc__ = "Comment role"
QgsDatabaseTableModel.RoleCustomInfo = QgsDatabaseTableModel.CustomRole.CustomInfo
QgsDatabaseTableModel.Role.RoleCustomInfo = QgsDatabaseTableModel.CustomRole.CustomInfo
QgsDatabaseTableModel.RoleCustomInfo.is_monkey_patched = True
QgsDatabaseTableModel.RoleCustomInfo.__doc__ = "Custom info variant map role"
QgsDatabaseTableModel.RoleWkbType = QgsDatabaseTableModel.CustomRole.WkbType
QgsDatabaseTableModel.Role.RoleWkbType = QgsDatabaseTableModel.CustomRole.WkbType
QgsDatabaseTableModel.RoleWkbType.is_monkey_patched = True
QgsDatabaseTableModel.RoleWkbType.__doc__ = "WKB type for primary (first) geometry column in table"
QgsDatabaseTableModel.RoleCrs = QgsDatabaseTableModel.CustomRole.Crs
QgsDatabaseTableModel.Role.RoleCrs = QgsDatabaseTableModel.CustomRole.Crs
QgsDatabaseTableModel.RoleCrs.is_monkey_patched = True
QgsDatabaseTableModel.RoleCrs.__doc__ = "CRS for primary (first) geometry column in table"
QgsDatabaseTableModel.RoleEmpty = QgsDatabaseTableModel.CustomRole.Empty
QgsDatabaseTableModel.Role.RoleEmpty = QgsDatabaseTableModel.CustomRole.Empty
QgsDatabaseTableModel.RoleEmpty.is_monkey_patched = True
QgsDatabaseTableModel.RoleEmpty.__doc__ = "Entry is an empty entry"
QgsDatabaseTableModel.CustomRole.__doc__ = "Custom model roles.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as QgsDatabaseTableModel.Role\n\n.. versionadded:: 3.36\n\n" + '* ``RoleTableName``: ' + QgsDatabaseTableModel.CustomRole.TableName.__doc__ + '\n' + '* ``RoleSchema``: ' + QgsDatabaseTableModel.CustomRole.Schema.__doc__ + '\n' + '* ``RoleTableFlags``: ' + QgsDatabaseTableModel.CustomRole.TableFlags.__doc__ + '\n' + '* ``RoleComment``: ' + QgsDatabaseTableModel.CustomRole.Comment.__doc__ + '\n' + '* ``RoleCustomInfo``: ' + QgsDatabaseTableModel.CustomRole.CustomInfo.__doc__ + '\n' + '* ``RoleWkbType``: ' + QgsDatabaseTableModel.CustomRole.WkbType.__doc__ + '\n' + '* ``RoleCrs``: ' + QgsDatabaseTableModel.CustomRole.Crs.__doc__ + '\n' + '* ``RoleEmpty``: ' + QgsDatabaseTableModel.CustomRole.Empty.__doc__
# --
QgsDatabaseTableModel.CustomRole.baseClass = QgsDatabaseTableModel
