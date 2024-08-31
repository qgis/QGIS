# The following has been generated automatically from src/core/qgsbookmarkmodel.h
QgsBookmarkManagerModel.CustomRoles = QgsBookmarkManagerModel.CustomRole
# monkey patching scoped based enum
QgsBookmarkManagerModel.RoleExtent = QgsBookmarkManagerModel.CustomRole.Extent
QgsBookmarkManagerModel.CustomRoles.RoleExtent = QgsBookmarkManagerModel.CustomRole.Extent
QgsBookmarkManagerModel.RoleExtent.is_monkey_patched = True
QgsBookmarkManagerModel.RoleExtent.__doc__ = "Bookmark extent as a QgsReferencedRectangle"
QgsBookmarkManagerModel.RoleName = QgsBookmarkManagerModel.CustomRole.Name
QgsBookmarkManagerModel.CustomRoles.RoleName = QgsBookmarkManagerModel.CustomRole.Name
QgsBookmarkManagerModel.RoleName.is_monkey_patched = True
QgsBookmarkManagerModel.RoleName.__doc__ = "Bookmark name"
QgsBookmarkManagerModel.RoleId = QgsBookmarkManagerModel.CustomRole.Id
QgsBookmarkManagerModel.CustomRoles.RoleId = QgsBookmarkManagerModel.CustomRole.Id
QgsBookmarkManagerModel.RoleId.is_monkey_patched = True
QgsBookmarkManagerModel.RoleId.__doc__ = "Bookmark ID"
QgsBookmarkManagerModel.RoleGroup = QgsBookmarkManagerModel.CustomRole.Group
QgsBookmarkManagerModel.CustomRoles.RoleGroup = QgsBookmarkManagerModel.CustomRole.Group
QgsBookmarkManagerModel.RoleGroup.is_monkey_patched = True
QgsBookmarkManagerModel.RoleGroup.__doc__ = "Bookmark group"
QgsBookmarkManagerModel.RoleRotation = QgsBookmarkManagerModel.CustomRole.Rotation
QgsBookmarkManagerModel.CustomRoles.RoleRotation = QgsBookmarkManagerModel.CustomRole.Rotation
QgsBookmarkManagerModel.RoleRotation.is_monkey_patched = True
QgsBookmarkManagerModel.RoleRotation.__doc__ = "Bookmark map rotation"
QgsBookmarkManagerModel.CustomRole.__doc__ = """Custom model roles.

.. note::

   Prior to QGIS 3.36 this was available as QgsBookmarkManagerModel.CustomRoles

.. versionadded:: 3.36

* ``RoleExtent``: Bookmark extent as a QgsReferencedRectangle
* ``RoleName``: Bookmark name
* ``RoleId``: Bookmark ID
* ``RoleGroup``: Bookmark group
* ``RoleRotation``: Bookmark map rotation

"""
# --
QgsBookmarkManagerModel.CustomRole.baseClass = QgsBookmarkManagerModel
QgsBookmarkManagerModel.ColumnName = QgsBookmarkManagerModel.Columns.ColumnName
QgsBookmarkManagerModel.ColumnGroup = QgsBookmarkManagerModel.Columns.ColumnGroup
QgsBookmarkManagerModel.ColumnXMin = QgsBookmarkManagerModel.Columns.ColumnXMin
QgsBookmarkManagerModel.ColumnYMin = QgsBookmarkManagerModel.Columns.ColumnYMin
QgsBookmarkManagerModel.ColumnXMax = QgsBookmarkManagerModel.Columns.ColumnXMax
QgsBookmarkManagerModel.ColumnYMax = QgsBookmarkManagerModel.Columns.ColumnYMax
QgsBookmarkManagerModel.ColumnRotation = QgsBookmarkManagerModel.Columns.ColumnRotation
QgsBookmarkManagerModel.ColumnCrs = QgsBookmarkManagerModel.Columns.ColumnCrs
QgsBookmarkManagerModel.ColumnStore = QgsBookmarkManagerModel.Columns.ColumnStore
