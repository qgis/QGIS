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
QgsAbstractDatabaseProviderConnection.IncludeSystemTables.__doc__ = "Include system tables \n.. versionadded:: 3.30"
QgsAbstractDatabaseProviderConnection.TableFlag.__doc__ = """Flags for table properties.

Flags can be useful for filtering the tables returned
from :py:func:`~QgsAbstractDatabaseProviderConnection.tables`.

* ``Aspatial``: Aspatial table (it does not contain any geometry column)
* ``Vector``: Vector table (it does contain one geometry column)
* ``Raster``: Raster table
* ``View``: View table
* ``MaterializedView``: Materialized view table
* ``Foreign``: Foreign data wrapper
* ``IncludeSystemTables``: Include system tables

  .. versionadded:: 3.30


"""
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
try:
    QgsAbstractDatabaseProviderConnection.SqlVectorLayerOptions.__attribute_docs__ = {'sql': 'The SQL expression that defines the SQL (query) layer', 'filter': 'Additional subset string (provider-side filter), not all data providers support this feature: check support with SqlLayerDefinitionCapability.Filters capability', 'layerName': 'Optional name for the new layer', 'primaryKeyColumns': 'List of primary key column names', 'geometryColumn': 'Name of the geometry column', 'disableSelectAtId': 'If SelectAtId is disabled (default is false), not all data providers support this feature: check support with SqlLayerDefinitionCapability.SelectAtId capability'}
    QgsAbstractDatabaseProviderConnection.SqlVectorLayerOptions.__doc__ = """The SqlVectorLayerOptions stores all information required to create a SQL (query) layer.

.. seealso:: :py:func:`createSqlVectorLayer`

.. versionadded:: 3.22"""
    QgsAbstractDatabaseProviderConnection.SqlVectorLayerOptions.__group__ = ['providers']
except (NameError, AttributeError):
    pass
try:
    QgsAbstractDatabaseProviderConnection.SpatialIndexOptions.__attribute_docs__ = {'geometryColumnName': 'Specifies the name of the geometry column to create the index for'}
    QgsAbstractDatabaseProviderConnection.SpatialIndexOptions.__doc__ = """The SpatialIndexOptions contains extra options relating to spatial index creation.

.. versionadded:: 3.14"""
    QgsAbstractDatabaseProviderConnection.SpatialIndexOptions.__group__ = ['providers']
except (NameError, AttributeError):
    pass
try:
    QgsAbstractDatabaseProviderConnection.QueryResult.__doc__ = """The QueryResult class represents the result of a query executed by :py:func:`~QgsAbstractDatabaseProviderConnection.execSql`

It encapsulates an iterator over the result rows and a list of the column names.

Rows can be retrieved by iterating over the result with :py:func:`~QgsAbstractDatabaseProviderConnection.hasNextRow` and :py:func:`~QgsAbstractDatabaseProviderConnection.nextRow`
or by calling :py:func:`~QgsAbstractDatabaseProviderConnection.rows` that will internally iterate over the results and return
the whole result list.

.. versionadded:: 3.18"""
    QgsAbstractDatabaseProviderConnection.QueryResult.__group__ = ['providers']
except (NameError, AttributeError):
    pass
try:
    QgsAbstractDatabaseProviderConnection.TableProperty.__doc__ = """The TableProperty class represents a database table or view.

In case the table is a vector spatial table and it has multiple
geometry columns, separate entries for each geometry column must
be created.

In case the table is a vector spatial table and the geometry column
can contain multiple geometry types and/or CRSs, a clone of the property
for the individual geometry type/CRS can be retrieved with at(i)"""
    QgsAbstractDatabaseProviderConnection.TableProperty.__group__ = ['providers']
except (NameError, AttributeError):
    pass
try:
    QgsAbstractDatabaseProviderConnection.TableProperty.GeometryColumnType.__doc__ = """The GeometryColumnType struct represents the combination
of geometry type and CRS for the table geometry column."""
    QgsAbstractDatabaseProviderConnection.TableProperty.GeometryColumnType.__group__ = ['providers']
except (NameError, AttributeError):
    pass
try:
    QgsAbstractDatabaseProviderConnection.__group__ = ['providers']
except (NameError, AttributeError):
    pass
