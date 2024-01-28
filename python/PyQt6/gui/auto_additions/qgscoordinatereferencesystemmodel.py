# The following has been generated automatically from src/gui/proj/qgscoordinatereferencesystemmodel.h
QgsCoordinateReferenceSystemModel.RoleNodeType = QgsCoordinateReferenceSystemModel.Roles.RoleNodeType
QgsCoordinateReferenceSystemModel.RoleName = QgsCoordinateReferenceSystemModel.Roles.RoleName
QgsCoordinateReferenceSystemModel.RoleAuthId = QgsCoordinateReferenceSystemModel.Roles.RoleAuthId
QgsCoordinateReferenceSystemModel.RoleDeprecated = QgsCoordinateReferenceSystemModel.Roles.RoleDeprecated
QgsCoordinateReferenceSystemModel.RoleType = QgsCoordinateReferenceSystemModel.Roles.RoleType
QgsCoordinateReferenceSystemModel.RoleGroupId = QgsCoordinateReferenceSystemModel.Roles.RoleGroupId
QgsCoordinateReferenceSystemModel.RoleWkt = QgsCoordinateReferenceSystemModel.Roles.RoleWkt
QgsCoordinateReferenceSystemModel.RoleProj = QgsCoordinateReferenceSystemModel.Roles.RoleProj
QgsCoordinateReferenceSystemProxyModel.FilterHorizontal = QgsCoordinateReferenceSystemProxyModel.Filter.FilterHorizontal
QgsCoordinateReferenceSystemProxyModel.FilterVertical = QgsCoordinateReferenceSystemProxyModel.Filter.FilterVertical
QgsCoordinateReferenceSystemProxyModel.FilterCompound = QgsCoordinateReferenceSystemProxyModel.Filter.FilterCompound
QgsCoordinateReferenceSystemProxyModel.Filters = lambda flags=0: QgsCoordinateReferenceSystemProxyModel.Filter(flags)
QgsCoordinateReferenceSystemProxyModel.Filters.baseClass = QgsCoordinateReferenceSystemProxyModel
Filters = QgsCoordinateReferenceSystemProxyModel  # dirty hack since SIP seems to introduce the flags in module
_force_int = lambda v: v if isinstance(v, int) else int(v.value)


QgsCoordinateReferenceSystemProxyModel.Filter.__bool__ = lambda flag: _force_int(flag)
QgsCoordinateReferenceSystemProxyModel.Filter.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsCoordinateReferenceSystemProxyModel.Filter.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsCoordinateReferenceSystemProxyModel.Filter.__or__ = lambda flag1, flag2: QgsCoordinateReferenceSystemProxyModel.Filter(_force_int(flag1) | _force_int(flag2))
