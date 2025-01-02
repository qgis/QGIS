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
QgsCoordinateReferenceSystemModel.Group = QgsCoordinateReferenceSystemModel.CustomRole.Group
QgsCoordinateReferenceSystemModel.Group.is_monkey_patched = True
QgsCoordinateReferenceSystemModel.Group.__doc__ = "Group name. \n.. versionadded:: 3.42"
QgsCoordinateReferenceSystemModel.Projection = QgsCoordinateReferenceSystemModel.CustomRole.Projection
QgsCoordinateReferenceSystemModel.Projection.is_monkey_patched = True
QgsCoordinateReferenceSystemModel.Projection.__doc__ = "Projection name. \n.. versionadded:: 3.42"
QgsCoordinateReferenceSystemModel.CustomRole.__doc__ = """Custom model roles.

.. note::

   Prior to QGIS 3.36 this was available as QgsCoordinateReferenceSystemModel.Roles

.. versionadded:: 3.36

* ``NodeType``: Corresponds to the node's type

  Available as ``QgsCoordinateReferenceSystemModel.RoleNodeType`` in older QGIS releases.

* ``Name``: The coordinate reference system name

  Available as ``QgsCoordinateReferenceSystemModel.RoleName`` in older QGIS releases.

* ``AuthId``: The coordinate reference system authority name and id

  Available as ``QgsCoordinateReferenceSystemModel.RoleAuthId`` in older QGIS releases.

* ``Deprecated``: ``True`` if the CRS is deprecated

  Available as ``QgsCoordinateReferenceSystemModel.RoleDeprecated`` in older QGIS releases.

* ``Type``: The coordinate reference system type

  Available as ``QgsCoordinateReferenceSystemModel.RoleType`` in older QGIS releases.

* ``GroupId``: The node ID (for group nodes)

  Available as ``QgsCoordinateReferenceSystemModel.RoleGroupId`` in older QGIS releases.

* ``Wkt``: The coordinate reference system's WKT representation. This is only used for non-standard CRS (i.e. those not present in the database).

  Available as ``QgsCoordinateReferenceSystemModel.RoleWkt`` in older QGIS releases.

* ``Proj``: The coordinate reference system's PROJ representation. This is only used for non-standard CRS (i.e. those not present in the database).

  Available as ``QgsCoordinateReferenceSystemModel.RoleProj`` in older QGIS releases.

* ``Group``: Group name.

  .. versionadded:: 3.42

* ``Projection``: Projection name.

  .. versionadded:: 3.42


"""
# --
QgsCoordinateReferenceSystemModel.CustomRole.baseClass = QgsCoordinateReferenceSystemModel
QgsCoordinateReferenceSystemProxyModel.Filters.baseClass = QgsCoordinateReferenceSystemProxyModel
Filters = QgsCoordinateReferenceSystemProxyModel  # dirty hack since SIP seems to introduce the flags in module
try:
    QgsCoordinateReferenceSystemModel.__group__ = ['proj']
except (NameError, AttributeError):
    pass
try:
    QgsCoordinateReferenceSystemProxyModel.__group__ = ['proj']
except (NameError, AttributeError):
    pass
