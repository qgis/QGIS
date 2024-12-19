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
QgsRecentCoordinateReferenceSystemsModel.CustomRole.__doc__ = """Custom model roles.

* ``Crs``: Coordinate reference system

  Available as ``QgsRecentCoordinateReferenceSystemsModel.RoleCrs`` in older QGIS releases.

* ``AuthId``: CRS authority ID

  Available as ``QgsRecentCoordinateReferenceSystemsModel.RoleAuthId`` in older QGIS releases.


"""
# --
QgsRecentCoordinateReferenceSystemsModel.CustomRole.baseClass = QgsRecentCoordinateReferenceSystemsModel
try:
    QgsRecentCoordinateReferenceSystemsModel.__group__ = ['proj']
except (NameError, AttributeError):
    pass
try:
    QgsRecentCoordinateReferenceSystemsProxyModel.__group__ = ['proj']
except (NameError, AttributeError):
    pass
