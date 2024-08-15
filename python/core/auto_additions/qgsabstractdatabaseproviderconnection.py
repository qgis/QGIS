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
try:
    QgsAbstractDatabaseProviderConnection.__attribute_docs__ = {'sql': 'The SQL expression that defines the SQL (query) layer', 'filter': 'Additional subset string (provider-side filter), not all data providers support this feature: check support with SqlLayerDefinitionCapability.Filters capability', 'layerName': 'Optional name for the new layer', 'primaryKeyColumns': 'List of primary key column names', 'geometryColumn': 'Name of the geometry column', 'disableSelectAtId': 'If SelectAtId is disabled (default is false), not all data providers support this feature: check support with SqlLayerDefinitionCapability.SelectAtId capability', 'geometryColumnName': 'Specifies the name of the geometry column to create the index for'}
except NameError:
    pass
QgsAbstractDatabaseProviderConnection.QueryResult.__doc__ = """The QueryResult class represents the result of a query executed by :py:func:`~QgsAbstractDatabaseProviderConnection.execSql`

It encapsulates an iterator over the result rows and a list of the column names.

Rows can be retrieved by iterating over the result with :py:func:`~QgsAbstractDatabaseProviderConnection.hasNextRow` and :py:func:`~QgsAbstractDatabaseProviderConnection.nextRow`
or by calling :py:func:`~QgsAbstractDatabaseProviderConnection.rows` that will internally iterate over the results and return
the whole result list.

.. versionadded:: 3.18"""
QgsAbstractDatabaseProviderConnection.SqlVectorLayerOptions.__doc__ = """The SqlVectorLayerOptions stores all information required to create a SQL (query) layer.

.. seealso:: :py:func:`createSqlVectorLayer`

.. versionadded:: 3.22"""
QgsAbstractDatabaseProviderConnection.TableProperty.__doc__ = """The TableProperty class represents a database table or view.

In case the table is a vector spatial table and it has multiple
geometry columns, separate entries for each geometry column must
be created.

In case the table is a vector spatial table and the geometry column
can contain multiple geometry types and/or CRSs, a clone of the property
for the individual geometry type/CRS can be retrieved with at(i)"""
QgsAbstractDatabaseProviderConnection.TableProperty.GeometryColumnType.__doc__ = """The GeometryColumnType struct represents the combination
of geometry type and CRS for the table geometry column."""
QgsAbstractDatabaseProviderConnection.SpatialIndexOptions.__doc__ = """The SpatialIndexOptions contains extra options relating to spatial index creation.

.. versionadded:: 3.14"""
try:
    QgsAbstractDatabaseProviderConnection.__group__ = ['providers']
except NameError:
    pass
try:
    QgsAbstractDatabaseProviderConnection.QueryResult.__group__ = ['providers']
except NameError:
    pass
try:
    QgsAbstractDatabaseProviderConnection.SqlVectorLayerOptions.__group__ = ['providers']
except NameError:
    pass
try:
    QgsAbstractDatabaseProviderConnection.TableProperty.__group__ = ['providers']
except NameError:
    pass
try:
    QgsAbstractDatabaseProviderConnection.TableProperty.GeometryColumnType.__group__ = ['providers']
except NameError:
    pass
try:
    QgsAbstractDatabaseProviderConnection.SpatialIndexOptions.__group__ = ['providers']
except NameError:
    pass
