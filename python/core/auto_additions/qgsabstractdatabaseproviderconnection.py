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
QgsAbstractDatabaseProviderConnection.TableFlags.baseClass = QgsAbstractDatabaseProviderConnection
TableFlags = QgsAbstractDatabaseProviderConnection  # dirty hack since SIP seems to introduce the flags in module
QgsAbstractDatabaseProviderConnection.Capability.baseClass = QgsAbstractDatabaseProviderConnection
QgsAbstractDatabaseProviderConnection.Capabilities.baseClass = QgsAbstractDatabaseProviderConnection
Capabilities = QgsAbstractDatabaseProviderConnection  # dirty hack since SIP seems to introduce the flags in module
QgsAbstractDatabaseProviderConnection.GeometryColumnCapability.baseClass = QgsAbstractDatabaseProviderConnection
QgsAbstractDatabaseProviderConnection.GeometryColumnCapabilities.baseClass = QgsAbstractDatabaseProviderConnection
GeometryColumnCapabilities = QgsAbstractDatabaseProviderConnection  # dirty hack since SIP seems to introduce the flags in module
QgsAbstractDatabaseProviderConnection.__attribute_docs__ = {'sql': 'The SQL expression that defines the SQL (query) layer', 'filter': 'Additional subset string (provider-side filter), not all data providers support this feature: check support with SqlLayerDefinitionCapability.Filters capability', 'layerName': 'Optional name for the new layer', 'primaryKeyColumns': 'List of primary key column names', 'geometryColumn': 'Name of the geometry column', 'disableSelectAtId': 'If SelectAtId is disabled (default is false), not all data providers support this feature: check support with SqlLayerDefinitionCapability.SelectAtId capability', 'geometryColumnName': 'Specifies the name of the geometry column to create the index for'}
