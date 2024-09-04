# The following has been generated automatically from src/core/qgsdatabaseschemamodel.h
QgsDatabaseSchemaModel.Role = QgsDatabaseSchemaModel.CustomRole
# monkey patching scoped based enum
QgsDatabaseSchemaModel.RoleEmpty = QgsDatabaseSchemaModel.CustomRole.Empty
QgsDatabaseSchemaModel.Role.RoleEmpty = QgsDatabaseSchemaModel.CustomRole.Empty
QgsDatabaseSchemaModel.RoleEmpty.is_monkey_patched = True
QgsDatabaseSchemaModel.RoleEmpty.__doc__ = "Entry is an empty entry"
QgsDatabaseSchemaModel.CustomRole.__doc__ = """Custom model roles.

.. note::

   Prior to QGIS 3.36 this was available as QgsDatabaseSchemaModel.Role

.. versionadded:: 3.36

* ``Empty``: Entry is an empty entry

  Available as ``QgsDatabaseSchemaModel.RoleEmpty`` in older QGIS releases.


"""
# --
QgsDatabaseSchemaModel.CustomRole.baseClass = QgsDatabaseSchemaModel
