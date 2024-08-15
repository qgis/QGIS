# The following has been generated automatically from src/gui/proj/qgsrecentcoordinatereferencesystemsmodel.h
QgsRecentCoordinateReferenceSystemsModel.Roles = QgsRecentCoordinateReferenceSystemsModel.CustomRole
# monkey patching scoped based enum
QgsRecentCoordinateReferenceSystemsModel.RoleCrs = QgsRecentCoordinateReferenceSystemsModel.CustomRole.Crs
QgsRecentCoordinateReferenceSystemsModel.Roles.RoleCrs = QgsRecentCoordinateReferenceSystemsModel.CustomRole.Crs
QgsRecentCoordinateReferenceSystemsModel.RoleCrs.is_monkey_patched = True
QgsRecentCoordinateReferenceSystemsModel.RoleCrs.__doc__ = "Coordinate reference system"
QgsRecentCoordinateReferenceSystemsModel.RoleAuthId = QgsRecentCoordinateReferenceSystemsModel.CustomRole.AuthId
QgsRecentCoordinateReferenceSystemsModel.Roles.RoleAuthId = QgsRecentCoordinateReferenceSystemsModel.CustomRole.AuthId
QgsRecentCoordinateReferenceSystemsModel.RoleAuthId.is_monkey_patched = True
QgsRecentCoordinateReferenceSystemsModel.RoleAuthId.__doc__ = "CRS authority ID"
QgsRecentCoordinateReferenceSystemsModel.CustomRole.__doc__ = "Custom model roles.\n\n" + '* ``RoleCrs``: ' + QgsRecentCoordinateReferenceSystemsModel.CustomRole.Crs.__doc__ + '\n' + '* ``RoleAuthId``: ' + QgsRecentCoordinateReferenceSystemsModel.CustomRole.AuthId.__doc__
# --
QgsRecentCoordinateReferenceSystemsModel.CustomRole.baseClass = QgsRecentCoordinateReferenceSystemsModel
try:
    QgsRecentCoordinateReferenceSystemsModel.__group__ = ['proj']
except NameError:
    pass
try:
    QgsRecentCoordinateReferenceSystemsProxyModel.__group__ = ['proj']
except NameError:
    pass
