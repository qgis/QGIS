# The following has been generated automatically from src/gui/proj/qgscoordinatereferencesystemmodel.h
QgsCoordinateReferenceSystemModel.Roles = QgsCoordinateReferenceSystemModel.CustomRole
# monkey patching scoped based enum
QgsCoordinateReferenceSystemModel.RoleNodeType = QgsCoordinateReferenceSystemModel.CustomRole.NodeType
QgsCoordinateReferenceSystemModel.Roles.RoleNodeType = QgsCoordinateReferenceSystemModel.CustomRole.NodeType
QgsCoordinateReferenceSystemModel.RoleNodeType.is_monkey_patched = True
QgsCoordinateReferenceSystemModel.RoleNodeType.__doc__ = "Corresponds to the node's type"
QgsCoordinateReferenceSystemModel.RoleName = QgsCoordinateReferenceSystemModel.CustomRole.Name
QgsCoordinateReferenceSystemModel.Roles.RoleName = QgsCoordinateReferenceSystemModel.CustomRole.Name
QgsCoordinateReferenceSystemModel.RoleName.is_monkey_patched = True
QgsCoordinateReferenceSystemModel.RoleName.__doc__ = "The coordinate reference system name"
QgsCoordinateReferenceSystemModel.RoleAuthId = QgsCoordinateReferenceSystemModel.CustomRole.AuthId
QgsCoordinateReferenceSystemModel.Roles.RoleAuthId = QgsCoordinateReferenceSystemModel.CustomRole.AuthId
QgsCoordinateReferenceSystemModel.RoleAuthId.is_monkey_patched = True
QgsCoordinateReferenceSystemModel.RoleAuthId.__doc__ = "The coordinate reference system authority name and id"
QgsCoordinateReferenceSystemModel.RoleDeprecated = QgsCoordinateReferenceSystemModel.CustomRole.Deprecated
QgsCoordinateReferenceSystemModel.Roles.RoleDeprecated = QgsCoordinateReferenceSystemModel.CustomRole.Deprecated
QgsCoordinateReferenceSystemModel.RoleDeprecated.is_monkey_patched = True
QgsCoordinateReferenceSystemModel.RoleDeprecated.__doc__ = "``True`` if the CRS is deprecated"
QgsCoordinateReferenceSystemModel.RoleType = QgsCoordinateReferenceSystemModel.CustomRole.Type
QgsCoordinateReferenceSystemModel.Roles.RoleType = QgsCoordinateReferenceSystemModel.CustomRole.Type
QgsCoordinateReferenceSystemModel.RoleType.is_monkey_patched = True
QgsCoordinateReferenceSystemModel.RoleType.__doc__ = "The coordinate reference system type"
QgsCoordinateReferenceSystemModel.RoleGroupId = QgsCoordinateReferenceSystemModel.CustomRole.GroupId
QgsCoordinateReferenceSystemModel.Roles.RoleGroupId = QgsCoordinateReferenceSystemModel.CustomRole.GroupId
QgsCoordinateReferenceSystemModel.RoleGroupId.is_monkey_patched = True
QgsCoordinateReferenceSystemModel.RoleGroupId.__doc__ = "The node ID (for group nodes)"
QgsCoordinateReferenceSystemModel.RoleWkt = QgsCoordinateReferenceSystemModel.CustomRole.Wkt
QgsCoordinateReferenceSystemModel.Roles.RoleWkt = QgsCoordinateReferenceSystemModel.CustomRole.Wkt
QgsCoordinateReferenceSystemModel.RoleWkt.is_monkey_patched = True
QgsCoordinateReferenceSystemModel.RoleWkt.__doc__ = "The coordinate reference system's WKT representation. This is only used for non-standard CRS (i.e. those not present in the database)."
QgsCoordinateReferenceSystemModel.RoleProj = QgsCoordinateReferenceSystemModel.CustomRole.Proj
QgsCoordinateReferenceSystemModel.Roles.RoleProj = QgsCoordinateReferenceSystemModel.CustomRole.Proj
QgsCoordinateReferenceSystemModel.RoleProj.is_monkey_patched = True
QgsCoordinateReferenceSystemModel.RoleProj.__doc__ = "The coordinate reference system's PROJ representation. This is only used for non-standard CRS (i.e. those not present in the database)."
QgsCoordinateReferenceSystemModel.CustomRole.__doc__ = "Custom model roles.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as QgsCoordinateReferenceSystemModel.Roles\n\n.. versionadded:: 3.36\n\n" + '* ``RoleNodeType``: ' + QgsCoordinateReferenceSystemModel.CustomRole.NodeType.__doc__ + '\n' + '* ``RoleName``: ' + QgsCoordinateReferenceSystemModel.CustomRole.Name.__doc__ + '\n' + '* ``RoleAuthId``: ' + QgsCoordinateReferenceSystemModel.CustomRole.AuthId.__doc__ + '\n' + '* ``RoleDeprecated``: ' + QgsCoordinateReferenceSystemModel.CustomRole.Deprecated.__doc__ + '\n' + '* ``RoleType``: ' + QgsCoordinateReferenceSystemModel.CustomRole.Type.__doc__ + '\n' + '* ``RoleGroupId``: ' + QgsCoordinateReferenceSystemModel.CustomRole.GroupId.__doc__ + '\n' + '* ``RoleWkt``: ' + QgsCoordinateReferenceSystemModel.CustomRole.Wkt.__doc__ + '\n' + '* ``RoleProj``: ' + QgsCoordinateReferenceSystemModel.CustomRole.Proj.__doc__
# --
QgsCoordinateReferenceSystemModel.CustomRole.baseClass = QgsCoordinateReferenceSystemModel
QgsCoordinateReferenceSystemProxyModel.Filters.baseClass = QgsCoordinateReferenceSystemProxyModel
Filters = QgsCoordinateReferenceSystemProxyModel  # dirty hack since SIP seems to introduce the flags in module
try:
    QgsCoordinateReferenceSystemModel.__group__ = ['proj']
except NameError:
    pass
try:
    QgsCoordinateReferenceSystemProxyModel.__group__ = ['proj']
except NameError:
    pass
