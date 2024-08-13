# The following has been generated automatically from src/core/providers/qgsabstractdatabaseproviderconnection.h
# monkey patching scoped based enum
QgsAbstractDatabaseProviderConnection.Aspatial = QgsAbstractDatabaseProviderConnection.TableFlag.Aspatial
QgsAbstractDatabaseProviderConnection.Aspatial.is_monkey_patched = True
QgsAbstractDatabaseProviderConnection.Aspatial.__doc__ = "Aspatial table (it does not contain any geometry column)"
QgsAbstractDatabaseProviderConnection.Vector = QgsAbstractDatabaseProviderConnection.TableFlag.Vector
QgsAbstractDatabaseProviderConnection.Vector.is_monkey_patched = True
QgsAbstractDatabaseProviderConnection.Vector.__doc__ = "Vector table (it does contain one geometry column)"
QgsAbstractDatabaseProviderConnection.Raster = QgsAbstractDatabaseProviderConnection.TableFlag.Raster
QgsAbstractDatabaseProviderConnection.Raster.is_monkey_patched = True
QgsAbstractDatabaseProviderConnection.Raster.__doc__ = "Raster table"
QgsAbstractDatabaseProviderConnection.View = QgsAbstractDatabaseProviderConnection.TableFlag.View
QgsAbstractDatabaseProviderConnection.View.is_monkey_patched = True
QgsAbstractDatabaseProviderConnection.View.__doc__ = "View table"
QgsAbstractDatabaseProviderConnection.MaterializedView = QgsAbstractDatabaseProviderConnection.TableFlag.MaterializedView
QgsAbstractDatabaseProviderConnection.MaterializedView.is_monkey_patched = True
QgsAbstractDatabaseProviderConnection.MaterializedView.__doc__ = "Materialized view table"
QgsAbstractDatabaseProviderConnection.Foreign = QgsAbstractDatabaseProviderConnection.TableFlag.Foreign
QgsAbstractDatabaseProviderConnection.Foreign.is_monkey_patched = True
QgsAbstractDatabaseProviderConnection.Foreign.__doc__ = "Foreign data wrapper"
QgsAbstractDatabaseProviderConnection.IncludeSystemTables = QgsAbstractDatabaseProviderConnection.TableFlag.IncludeSystemTables
QgsAbstractDatabaseProviderConnection.IncludeSystemTables.is_monkey_patched = True
QgsAbstractDatabaseProviderConnection.IncludeSystemTables.__doc__ = "Include system tables (since QGIS 3.30)"
QgsAbstractDatabaseProviderConnection.TableFlag.__doc__ = "Flags for table properties.\n\nFlags can be useful for filtering the tables returned\nfrom :py:func:`~QgsAbstractDatabaseProviderConnection.tables`.\n\n" + '* ``Aspatial``: ' + QgsAbstractDatabaseProviderConnection.TableFlag.Aspatial.__doc__ + '\n' + '* ``Vector``: ' + QgsAbstractDatabaseProviderConnection.TableFlag.Vector.__doc__ + '\n' + '* ``Raster``: ' + QgsAbstractDatabaseProviderConnection.TableFlag.Raster.__doc__ + '\n' + '* ``View``: ' + QgsAbstractDatabaseProviderConnection.TableFlag.View.__doc__ + '\n' + '* ``MaterializedView``: ' + QgsAbstractDatabaseProviderConnection.TableFlag.MaterializedView.__doc__ + '\n' + '* ``Foreign``: ' + QgsAbstractDatabaseProviderConnection.TableFlag.Foreign.__doc__ + '\n' + '* ``IncludeSystemTables``: ' + QgsAbstractDatabaseProviderConnection.TableFlag.IncludeSystemTables.__doc__
# --
QgsAbstractDatabaseProviderConnection.TableFlag.baseClass = QgsAbstractDatabaseProviderConnection
QgsAbstractDatabaseProviderConnection.TableFlags = lambda flags=0: QgsAbstractDatabaseProviderConnection.TableFlag(flags)
QgsAbstractDatabaseProviderConnection.TableFlags.baseClass = QgsAbstractDatabaseProviderConnection
TableFlags = QgsAbstractDatabaseProviderConnection  # dirty hack since SIP seems to introduce the flags in module
QgsAbstractDatabaseProviderConnection.CreateVectorTable = QgsAbstractDatabaseProviderConnection.Capability.CreateVectorTable
QgsAbstractDatabaseProviderConnection.DropRasterTable = QgsAbstractDatabaseProviderConnection.Capability.DropRasterTable
QgsAbstractDatabaseProviderConnection.DropVectorTable = QgsAbstractDatabaseProviderConnection.Capability.DropVectorTable
QgsAbstractDatabaseProviderConnection.RenameVectorTable = QgsAbstractDatabaseProviderConnection.Capability.RenameVectorTable
QgsAbstractDatabaseProviderConnection.RenameRasterTable = QgsAbstractDatabaseProviderConnection.Capability.RenameRasterTable
QgsAbstractDatabaseProviderConnection.CreateSchema = QgsAbstractDatabaseProviderConnection.Capability.CreateSchema
QgsAbstractDatabaseProviderConnection.DropSchema = QgsAbstractDatabaseProviderConnection.Capability.DropSchema
QgsAbstractDatabaseProviderConnection.RenameSchema = QgsAbstractDatabaseProviderConnection.Capability.RenameSchema
QgsAbstractDatabaseProviderConnection.ExecuteSql = QgsAbstractDatabaseProviderConnection.Capability.ExecuteSql
QgsAbstractDatabaseProviderConnection.Vacuum = QgsAbstractDatabaseProviderConnection.Capability.Vacuum
QgsAbstractDatabaseProviderConnection.Tables = QgsAbstractDatabaseProviderConnection.Capability.Tables
QgsAbstractDatabaseProviderConnection.Schemas = QgsAbstractDatabaseProviderConnection.Capability.Schemas
QgsAbstractDatabaseProviderConnection.SqlLayers = QgsAbstractDatabaseProviderConnection.Capability.SqlLayers
QgsAbstractDatabaseProviderConnection.TableExists = QgsAbstractDatabaseProviderConnection.Capability.TableExists
QgsAbstractDatabaseProviderConnection.Spatial = QgsAbstractDatabaseProviderConnection.Capability.Spatial
QgsAbstractDatabaseProviderConnection.CreateSpatialIndex = QgsAbstractDatabaseProviderConnection.Capability.CreateSpatialIndex
QgsAbstractDatabaseProviderConnection.SpatialIndexExists = QgsAbstractDatabaseProviderConnection.Capability.SpatialIndexExists
QgsAbstractDatabaseProviderConnection.DeleteSpatialIndex = QgsAbstractDatabaseProviderConnection.Capability.DeleteSpatialIndex
QgsAbstractDatabaseProviderConnection.DeleteField = QgsAbstractDatabaseProviderConnection.Capability.DeleteField
QgsAbstractDatabaseProviderConnection.DeleteFieldCascade = QgsAbstractDatabaseProviderConnection.Capability.DeleteFieldCascade
QgsAbstractDatabaseProviderConnection.AddField = QgsAbstractDatabaseProviderConnection.Capability.AddField
QgsAbstractDatabaseProviderConnection.ListFieldDomains = QgsAbstractDatabaseProviderConnection.Capability.ListFieldDomains
QgsAbstractDatabaseProviderConnection.RetrieveFieldDomain = QgsAbstractDatabaseProviderConnection.Capability.RetrieveFieldDomain
QgsAbstractDatabaseProviderConnection.SetFieldDomain = QgsAbstractDatabaseProviderConnection.Capability.SetFieldDomain
QgsAbstractDatabaseProviderConnection.AddFieldDomain = QgsAbstractDatabaseProviderConnection.Capability.AddFieldDomain
QgsAbstractDatabaseProviderConnection.RenameField = QgsAbstractDatabaseProviderConnection.Capability.RenameField
QgsAbstractDatabaseProviderConnection.RetrieveRelationships = QgsAbstractDatabaseProviderConnection.Capability.RetrieveRelationships
QgsAbstractDatabaseProviderConnection.AddRelationship = QgsAbstractDatabaseProviderConnection.Capability.AddRelationship
QgsAbstractDatabaseProviderConnection.UpdateRelationship = QgsAbstractDatabaseProviderConnection.Capability.UpdateRelationship
QgsAbstractDatabaseProviderConnection.DeleteRelationship = QgsAbstractDatabaseProviderConnection.Capability.DeleteRelationship
QgsAbstractDatabaseProviderConnection.Capability.baseClass = QgsAbstractDatabaseProviderConnection
QgsAbstractDatabaseProviderConnection.Capabilities = lambda flags=0: QgsAbstractDatabaseProviderConnection.Capability(flags)
QgsAbstractDatabaseProviderConnection.Capabilities.baseClass = QgsAbstractDatabaseProviderConnection
Capabilities = QgsAbstractDatabaseProviderConnection  # dirty hack since SIP seems to introduce the flags in module
QgsAbstractDatabaseProviderConnection.Z = QgsAbstractDatabaseProviderConnection.GeometryColumnCapability.Z
QgsAbstractDatabaseProviderConnection.M = QgsAbstractDatabaseProviderConnection.GeometryColumnCapability.M
QgsAbstractDatabaseProviderConnection.SinglePart = QgsAbstractDatabaseProviderConnection.GeometryColumnCapability.SinglePart
QgsAbstractDatabaseProviderConnection.Curves = QgsAbstractDatabaseProviderConnection.GeometryColumnCapability.Curves
QgsAbstractDatabaseProviderConnection.SinglePoint = QgsAbstractDatabaseProviderConnection.GeometryColumnCapability.SinglePoint
QgsAbstractDatabaseProviderConnection.SingleLineString = QgsAbstractDatabaseProviderConnection.GeometryColumnCapability.SingleLineString
QgsAbstractDatabaseProviderConnection.SinglePolygon = QgsAbstractDatabaseProviderConnection.GeometryColumnCapability.SinglePolygon
QgsAbstractDatabaseProviderConnection.GeometryColumnCapability.baseClass = QgsAbstractDatabaseProviderConnection
QgsAbstractDatabaseProviderConnection.GeometryColumnCapabilities = lambda flags=0: QgsAbstractDatabaseProviderConnection.GeometryColumnCapability(flags)
QgsAbstractDatabaseProviderConnection.GeometryColumnCapabilities.baseClass = QgsAbstractDatabaseProviderConnection
GeometryColumnCapabilities = QgsAbstractDatabaseProviderConnection  # dirty hack since SIP seems to introduce the flags in module
from enum import Enum


def _force_int(v): return int(v.value) if isinstance(v, Enum) else v


QgsAbstractDatabaseProviderConnection.Capability.__bool__ = lambda flag: bool(_force_int(flag))
QgsAbstractDatabaseProviderConnection.Capability.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsAbstractDatabaseProviderConnection.Capability.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsAbstractDatabaseProviderConnection.Capability.__or__ = lambda flag1, flag2: QgsAbstractDatabaseProviderConnection.Capability(_force_int(flag1) | _force_int(flag2))
try:
    QgsAbstractDatabaseProviderConnection.__attribute_docs__ = {'sql': 'The SQL expression that defines the SQL (query) layer', 'filter': 'Additional subset string (provider-side filter), not all data providers support this feature: check support with SqlLayerDefinitionCapability.Filters capability', 'layerName': 'Optional name for the new layer', 'primaryKeyColumns': 'List of primary key column names', 'geometryColumn': 'Name of the geometry column', 'disableSelectAtId': 'If SelectAtId is disabled (default is false), not all data providers support this feature: check support with SqlLayerDefinitionCapability.SelectAtId capability', 'geometryColumnName': 'Specifies the name of the geometry column to create the index for'}
except NameError:
    pass
