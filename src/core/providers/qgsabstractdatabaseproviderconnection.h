/***************************************************************************
  qgsabstractdatabaseproviderconnection.h - QgsAbstractDatabaseProviderConnection

 ---------------------
 begin                : 2.8.2019
 copyright            : (C) 2019 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSABSTRACTDATABASEPROVIDERCONNECTION_H
#define QGSABSTRACTDATABASEPROVIDERCONNECTION_H

#include "qgsabstractproviderconnection.h"
#include "qgscoordinatereferencesystem.h"
#include "qgis_core.h"
#include "qgsfields.h"
#include "qgsexception.h"
#include "qgsvectordataprovider.h"

#include <QObject>

class QgsFeedback;
class QgsFieldDomain;

/**
 * \brief The QgsAbstractDatabaseProviderConnection class provides common functionality
 * for DB based connections.
 *
 * This class performs low level DB operations without asking
 * the user for confirmation or handling currently opened layers and the registry
 * entries, it is responsibility of the client code to keep layers in sync.
 * The class methods will throw exceptions in case the requested operation
 * is not supported or cannot be performed without errors.
 *
 * \ingroup core
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsAbstractDatabaseProviderConnection : public QgsAbstractProviderConnection
{

    Q_GADGET

  public:

    /**
     * Flags for table properties.
     *
     * Flags can be useful for filtering the tables returned
     * from tables().
     */
    enum TableFlag
    {
      Aspatial = 1 << 1,          //!< Aspatial table (it does not contain any geometry column)
      Vector = 1 << 2,            //!< Vector table (it does contain one geometry column)
      Raster = 1 << 3,            //!< Raster table
      View = 1 << 4,              //!< View table
      MaterializedView = 1 << 5,  //!< Materialized view table
      Foreign = 1 << 6,           //!< Foreign data wrapper
    };

    Q_ENUM( TableFlag )
    Q_DECLARE_FLAGS( TableFlags, TableFlag )
    Q_FLAG( TableFlags )

    /**
     * The QueryResult class represents the result of a query executed by execSql()
     *
     * It encapsulates an iterator over the result rows and a list of the column names.
     *
     * Rows can be retrieved by iterating over the result with hasNextRow() and nextRow()
     * or by calling rows() that will internally iterate over the results and return
     * the whole result list.
     *
     *
     * \since QGIS 3.18
     */
    struct CORE_EXPORT QueryResult
    {

        /**
         * Returns the column names.
         */
        QStringList columns() const;

        /**
         * Returns the result rows by calling the iterator internally and fetching
         * all the rows, an optional \a feedback can be used to interrupt the fetching loop.
         *
         * \note calling this function more than one time is not supported: the second
         * call will always return an empty list.
         */
        QList<QList<QVariant> > rows( QgsFeedback *feedback = nullptr );

        /**
         * Returns TRUE if there are more rows to fetch.
         *
         * \see nextRow()
         * \see rewind()
         */
        bool hasNextRow() const;

        /**
         * Returns the next result row or an empty row if there are no rows left.
         *
         * \see hasNextRow()
         * \see rewind()
         */
        QList<QVariant> nextRow() const;

        /**
         * Returns the number of fetched rows.
         *
         * \see rowCount()
         */
        long long fetchedRowCount( ) const;

        /**
         * Returns the number of rows returned by a SELECT query or Qgis::FeatureCountState::UnknownCount if unknown.
         *
         * \see fetchedRowCount()
         */
        long long rowCount( ) const;


#ifdef SIP_RUN
        // Python iterator
        QueryResult *__iter__();
        % MethodCode
        sipRes = sipCpp;
        % End

        SIP_PYOBJECT __next__();
        % MethodCode
        QList<QVariant> result;
        Py_BEGIN_ALLOW_THREADS
        result = sipCpp->nextRow( );
        Py_END_ALLOW_THREADS
        if ( ! result.isEmpty() )
        {
          const sipTypeDef *qvariantlist_type = sipFindType( "QList<QVariant>" );
          sipRes = sipConvertFromNewType( new QList<QVariant>( result ), qvariantlist_type, Py_None );
        }
        else
        {
          PyErr_SetString( PyExc_StopIteration, "" );
        }
        % End
#endif

///@cond private

        /**
         * The QueryResultIterator struct is an abstract interface for provider query result iterators.
         * Providers must implement their own concrete iterator over query results.
         *
         */
        struct CORE_EXPORT QueryResultIterator SIP_SKIP
        {
            //! Returns the next result row
            QVariantList nextRow();

            //! Returns TRUE if there is another row to fetch
            bool hasNextRow() const;

            //! Returns the number of actually fetched rows
            long long fetchedRowCount();

            //! Returns the total number of rows returned by a SELECT query or Qgis::FeatureCountState::UnknownCount ( -1 ) if this is not supported by the provider.
            long long rowCount();

            virtual ~QueryResultIterator() = default;

          private:

            virtual QVariantList nextRowPrivate() = 0;
            virtual bool hasNextRowPrivate() const = 0;
            virtual long long rowCountPrivate() const = 0;

            mutable qlonglong mFetchedRowCount = 0;
            mutable QMutex mMutex;

        };

        /**
         * Appends \a columnName to the list of column names.
         *
         * \note Not available in Python bindings
         */
        void appendColumn( const QString &columnName ) SIP_SKIP;

        /**
         * Constructs a QueryResult object from an \a iterator.
         *
         * \note Not available in Python bindings
         */
        QueryResult( std::shared_ptr<QueryResultIterator> iterator ) SIP_SKIP;

        /**
         * Default constructor, used to return empty results
         * \note Not available in Python bindings
         */
        QueryResult( ) = default SIP_SKIP;

        /**
         * Returns the query execution time in milliseconds.
         */
        double queryExecutionTime( );

        /**
         * Sets the query execution time to \a queryExecutionTime milliseconds.
         */
        void setQueryExecutionTime( double queryExecutionTime );

///@endcond private

      private:

        mutable std::shared_ptr<QueryResultIterator> mResultIterator;
        QStringList mColumns;
        //! Query execution time in milliseconds
        double mQueryExecutionTime = 0;

    };

    /**
     * \brief The SqlVectorLayerOptions stores all information required to create a SQL (query) layer.
     * \see createSqlVectorLayer()
     *
     * \since QGIS 3.22
     */
    struct CORE_EXPORT SqlVectorLayerOptions
    {
      //! The SQL expression that defines the SQL (query) layer
      QString sql;
      //! Additional subset string (provider-side filter), not all data providers support this feature: check support with SqlLayerDefinitionCapability::Filters capability
      QString filter;
      //! Optional name for the new layer
      QString layerName;
      //! List of primary key column names
      QStringList primaryKeyColumns;
      //! Name of the geometry column
      QString geometryColumn;
      //! If SelectAtId is disabled (default is false), not all data providers support this feature: check support with SqlLayerDefinitionCapability::SelectAtId capability
      bool disableSelectAtId = false;

    };

    /**
     * The TableProperty class represents a database table or view.
     *
     * In case the table is a vector spatial table and it has multiple
     * geometry columns, separate entries for each geometry column must
     * be created.
     *
     * In case the table is a vector spatial table and the geometry column
     * can contain multiple geometry types and/or CRSs, a clone of the property
     * for the individual geometry type/CRS can be retrieved with at(i)
     */
    struct CORE_EXPORT TableProperty
    {

#ifdef SIP_RUN
        SIP_PYOBJECT __repr__();
        % MethodCode
        QString str = QStringLiteral( "<QgsAbstractDatabaseProviderConnection.TableProperty: '%1'>" ).arg( sipCpp->tableName() );
        sipRes = PyUnicode_FromString( str.toUtf8().constData() );
        % End
#endif

        /**
         * The GeometryColumnType struct represents the combination
         * of geometry type and CRS for the table geometry column.
         */
        struct CORE_EXPORT GeometryColumnType
        {
#ifdef SIP_RUN
          SIP_PYOBJECT __repr__();
          % MethodCode
          QString str = QStringLiteral( "<QgsAbstractDatabaseProviderConnection.TableProperty.GeometryColumnType: '%1, %2'>" ).arg( QgsWkbTypes::displayString( sipCpp->wkbType ), sipCpp->crs.authid() );
          sipRes = PyUnicode_FromString( str.toUtf8().constData() );
          % End
#endif
          QgsWkbTypes::Type wkbType;
          QgsCoordinateReferenceSystem crs;

          // TODO c++20 - replace with = default
          inline bool operator==( const GeometryColumnType &other ) const
          {
            return this->crs == other.crs && this->wkbType == other.wkbType;
          }
        };

      public:

        /**
         * Returns the table name.
         *
         * \see defaultName()
         */
        QString tableName() const;

        /**
         * Sets the table name to \a name.
         *
         * \see defaultName()
         */
        void setTableName( const QString &name );

        /**
         * Appends the geometry column \a type with the given \a srid to the geometry column types list.
         */
        void addGeometryColumnType( const QgsWkbTypes::Type &type, const QgsCoordinateReferenceSystem &crs );

        /**
         * Returns the list of geometry column types and CRSs.
         *
         * The method returns a list of GeometryColumnType.
         */
        QList<QgsAbstractDatabaseProviderConnection::TableProperty::GeometryColumnType> geometryColumnTypes() const;

        /**
         * Sets the geometry column types to \a geometryColumnTypes.
         */
        void setGeometryColumnTypes( const QList<QgsAbstractDatabaseProviderConnection::TableProperty::GeometryColumnType> &geometryColumnTypes );

        /**
         * Returns the default name for the table entry.
         *
         * This is usually the table name but in case there are multiple geometry
         * columns, the geometry column name is appended to the table name.
         * \see geometryColumnCount()
         */
        QString defaultName() const;

        /**
         * Returns the table property corresponding to the geometry type at
         * the given \a index.
         */
        TableProperty at( int index ) const;

        /**
         * Returns the schema or an empty string for backends that do not support a schema.
         */
        QString schema() const;

        /**
         * Sets the \a schema.
         */
        void setSchema( const QString &schema );

        /**
         * Returns the geometry column name.
         */
        QString geometryColumn() const;

        /**
         * Sets the geometry column name to \a geometryColumn.
         */
        void setGeometryColumn( const QString &geometryColumn );

        /**
         * Returns the list of primary key column names.
         */
        QStringList primaryKeyColumns() const;

        /**
         * Sets the primary key column names to \a primaryKeyColumns.
         */
        void setPrimaryKeyColumns( const QStringList &primaryKeyColumns );

        /**
         * Returns the list of CRSs supported by the geometry column.
         */
        QList<QgsCoordinateReferenceSystem> crsList() const;

        /**
         * Returns the table flags.
         */
        TableFlags flags() const;

        /**
         * Sets the table \a flags.
         */
        void setFlags( const TableFlags &flags );

        /**
         * Returns the table comment.
         */
        QString comment() const;

        /**
         * Sets the table \a comment.
         */
        void setComment( const QString &comment );

        /**
         * Returns additional information about the table.
         *
         * Provider classes may use this property
         * to store custom bits of information.
         */
        QVariantMap info() const;

        /**
         * Sets additional information about the table to \a info.
         *
         * Provider classes may use this property
         * to store custom bits of information.
         */
        void setInfo( const QVariantMap &info );

        /**
         * Returns the number of geometry columns in the original table this entry refers to.
         *
         * This information is used internally to build the \see defaultName().
         */
        int geometryColumnCount() const;

        /**
         * Sets the \a geometryColumnCount.
         */
        void setGeometryColumnCount( int geometryColumnCount );

        /**
         * Sets a \a flag.
         */
        void setFlag( const TableFlag &flag );

        /**
         * Returns the maximum coordinate dimensions of the geometries of a vector table.
         *
         * This information is calculated from the geometry columns types.
         * \see geometryColumnTypes()
         */
        int maxCoordinateDimensions() const;

        bool operator==( const QgsAbstractDatabaseProviderConnection::TableProperty &other ) const;

      private:

        //! Holds the list of geometry wkb types and srids supported by the table
        QList<GeometryColumnType>     mGeometryColumnTypes;
        //! Table schema
        QString                       mSchema;
        //! Table name
        QString                       mTableName;
        //! Name of the geometry column
        QString                       mGeometryColumn;
        //! The number of geometry columns in the table
        int                           mGeometryColumnCount;
        //! PK columns
        QStringList                   mPkColumns;
        TableFlags                    mFlags;
        QString                       mComment;
        //! Additional unstructured information about the table
        QVariantMap                   mInfo;
    };

    /**
     * \brief The SpatialIndexOptions contains extra options relating to spatial index creation.
     *
     * \since QGIS 3.14
     */
    struct CORE_EXPORT SpatialIndexOptions
    {
      //! Specifies the name of the geometry column to create the index for
      QString geometryColumnName;
    };

    /**
     * The Capability enum represents the operations supported by the connection.
     */
    enum Capability
    {
      CreateVectorTable = 1 << 1,                     //!< Can CREATE a vector (or aspatial) table/layer
      DropRasterTable = 1 << 2,                       //!< Can DROP a raster table/layer
      DropVectorTable = 1 << 3,                       //!< Can DROP a vector (or aspatial) table/layer
      RenameVectorTable = 1 << 4,                     //!< Can RENAME a vector (or aspatial) table/layer
      RenameRasterTable = 1 << 5,                     //!< Can RENAME a raster table/layer
      CreateSchema = 1 << 6,                          //!< Can CREATE a schema
      DropSchema = 1 << 7,                            //!< Can DROP a schema
      RenameSchema = 1 << 8,                          //!< Can RENAME a schema
      ExecuteSql = 1 << 9,                            //!< Can execute raw SQL queries (without returning results)
      Vacuum = 1 << 10,                               //!< Can run vacuum
      Tables = 1 << 11,                               //!< Can list tables
      Schemas = 1 << 12,                              //!< Can list schemas (if not set, the connection does not support schemas)
      SqlLayers = 1 << 13,                            //!< Can create vector layers from SQL SELECT queries
      TableExists = 1 << 14,                          //!< Can check if table exists
      Spatial = 1 << 15,                              //!< The connection supports spatial tables
      CreateSpatialIndex = 1 << 16,                   //!< The connection can create spatial indices
      SpatialIndexExists = 1 << 17,                   //!< The connection can determine if a spatial index exists
      DeleteSpatialIndex = 1 << 18,                   //!< The connection can delete spatial indices for tables
      DeleteField = 1 << 19,                          //!< Can delete an existing field/column
      DeleteFieldCascade = 1 << 20,                   //!< Can delete an existing field/column with cascade
      AddField = 1 << 21,                             //!< Can add a new field/column
      ListFieldDomains = 1 << 22,                     //!< Can return a list of field domain names via fieldDomainNames() (since QGIS 3.26)
      RetrieveFieldDomain = 1 << 23,                  //!< Can retrieve field domain details from provider via fieldDomain() (since QGIS 3.26)
      SetFieldDomain = 1 << 24,                       //!< Can set the domain for an existing field via setFieldDomainName() (since QGIS 3.26)
      AddFieldDomain = 1 << 25,                       //!< Can add new field domains to the database via addFieldDomain() (since QGIS 3.26)
    };
    Q_ENUM( Capability )
    Q_DECLARE_FLAGS( Capabilities, Capability )
    Q_FLAG( Capabilities )

    /**
     * The GeometryColumnCapability enum represents the geomery column features supported by the connection.
     *
     * \since QGIS 3.16
     */
    enum GeometryColumnCapability
    {
      Z = 1 << 1,                    //!< Supports Z dimension
      M = 1 << 2,                    //!< Supports M dimension
      SinglePart = 1 << 3,           //!< Multi and single part types are distinct types
      Curves = 1 << 4                //!< Supports curves
    };

    Q_ENUM( GeometryColumnCapability )
    Q_DECLARE_FLAGS( GeometryColumnCapabilities, GeometryColumnCapability )
    Q_FLAG( GeometryColumnCapabilities )

    /**
     * Creates a new connection with \a name by reading its configuration from the settings.
     *
     * If a connection with this name cannot be found, an empty connection will be returned.
     */
    QgsAbstractDatabaseProviderConnection( const QString &name );

    /**
     * Creates a new connection from the given \a uri and \a configuration.
     *
     * The connection is not automatically stored in the settings.
     * \see store()
     */
    QgsAbstractDatabaseProviderConnection( const QString &uri, const QVariantMap &configuration );


    // Public interface

    /**
     * Returns connection capabilities
     */
    Capabilities capabilities() const;

    /**
     * Returns connection geometry column capabilities (Z, M, SinglePart, Curves).
     *
     * \since QGIS 3.16
     */
    virtual GeometryColumnCapabilities geometryColumnCapabilities();

    /**
     * Returns SQL layer definition capabilities (Filters, GeometryColumn, PrimaryKeys).
     * \since QGIS 3.22
     */
    virtual Qgis::SqlLayerDefinitionCapabilities sqlLayerDefinitionCapabilities();

    // Operations interface

    /**
     * Returns the URI string for the given \a table and \a schema.
     *
     * \throws QgsProviderConnectionException if any errors are encountered.
     * \since QGIS 3.12
     */
    virtual QString tableUri( const QString &schema, const QString &name ) const SIP_THROW( QgsProviderConnectionException );

    /**
     * Creates an empty table with \a name in the given \a schema (schema is ignored if not supported by the backend).
     *
     * \throws QgsProviderConnectionException if any errors are encountered.
     */
    virtual void createVectorTable( const QString &schema, const QString &name, const QgsFields &fields, QgsWkbTypes::Type wkbType, const QgsCoordinateReferenceSystem &srs, bool overwrite, const QMap<QString, QVariant> *options ) const SIP_THROW( QgsProviderConnectionException );

    /**
     * Checks whether a table \a name exists in the given \a schema.
     *
     * \throws QgsProviderConnectionException if any errors are encountered.
     */
    virtual bool tableExists( const QString &schema, const QString &name ) const SIP_THROW( QgsProviderConnectionException );

    /**
     * Drops a vector (or aspatial) table with given \a schema (schema is ignored if not supported by the backend) and \a name.
     *
     * \note It is responsibility of the caller to handle open layers and registry entries.
     * \throws QgsProviderConnectionException if any errors are encountered.
     */
    virtual void dropVectorTable( const QString &schema, const QString &name ) const SIP_THROW( QgsProviderConnectionException );

    /**
     * Drops a raster table with given \a schema (schema is ignored if not supported by the backend) and \a name.
     *
     * \note It is responsibility of the caller to handle open layers and registry entries.
     * \throws QgsProviderConnectionException if any errors are encountered.
     */
    virtual void dropRasterTable( const QString &schema, const QString &name ) const SIP_THROW( QgsProviderConnectionException );

    /**
     * Renames a vector or aspatial table with given \a schema (schema is ignored if not supported by the backend) and \a name.
     *
     * \note It is responsibility of the caller to handle open layers and registry entries.
     * \throws QgsProviderConnectionException if any errors are encountered.
     */
    virtual void renameVectorTable( const QString &schema, const QString &name, const QString &newName ) const SIP_THROW( QgsProviderConnectionException );

    /**
     * Renames a raster table with given \a schema (schema is ignored if not supported by the backend) and \a name.
     *
     * \note It is responsibility of the caller to handle open layers and registry entries.
     * \throws QgsProviderConnectionException if any errors are encountered.
     */
    virtual void renameRasterTable( const QString &schema, const QString &name, const QString &newName ) const SIP_THROW( QgsProviderConnectionException );

    /**
     * Creates a new schema with the specified \a name.
     *
     * \throws QgsProviderConnectionException if any errors are encountered.
     */
    virtual void createSchema( const QString &name ) const SIP_THROW( QgsProviderConnectionException );

    /**
     * Drops an entire schema with the specified name.
     *
     * \param name name of the schema to be dropped
     * \param force if TRUE, a DROP CASCADE will drop all related objects
     * \note It is responsibility of the caller to handle open layers and registry entries.
     * \throws QgsProviderConnectionException if any errors are encountered.
     */
    virtual void dropSchema( const QString &name, bool force = false ) const SIP_THROW( QgsProviderConnectionException );

    /**
     * Deletes the field with the specified name.
     *
     * \param fieldName name of the field to be deleted
     * \param schema name of the schema (schema is ignored if not supported by the backend).
     * \param tableName name of the table
     * \param force if TRUE, a DROP CASCADE will drop all related objects
     * \note it is responsibility of the caller to handle open layers and registry entries.
     * \throws QgsProviderConnectionException if any errors are encountered.
     * \since QGIS 3.16
     */
    virtual void deleteField( const QString &fieldName, const QString &schema, const QString &tableName, bool force = false ) const SIP_THROW( QgsProviderConnectionException );

    /**
     * Adds a field.
     *
     * \param field specification of the new field
     * \param schema name of the schema (schema is ignored if not supported by the backend).
     * \param tableName name of the table
     * \note it is responsibility of the caller to handle open layers and registry entries.
     * \throws QgsProviderConnectionException if any errors are encountered.
     * \since QGIS 3.16
     */
    virtual void addField( const QgsField &field, const QString &schema, const QString &tableName ) const SIP_THROW( QgsProviderConnectionException );

    /**
     * Renames a schema with the specified \a name.
     * Raises a QgsProviderConnectionException if any errors are encountered.
     * \note it is responsibility of the caller to handle open layers and registry entries.
     * \throws QgsProviderConnectionException if any errors are encountered.
     */
    virtual void renameSchema( const QString &name, const QString &newName ) const SIP_THROW( QgsProviderConnectionException );

    /**
     * Executes raw \a sql and returns the (possibly empty) list of results in a multi-dimensional array, optionally \a feedback can be provided.
     *
     * \see execSql()
     * \throws QgsProviderConnectionException if any errors are encountered.
     */
    virtual QList<QList<QVariant>> executeSql( const QString &sql, QgsFeedback *feedback = nullptr ) const SIP_THROW( QgsProviderConnectionException );

    /**
     * Creates and returns a (possibly invalid) vector layer based on the \a sql statement and optional \a options.
     * \throws QgsProviderConnectionException if any errors are encountered or if SQL layer creation is not supported.
     * \since QGIS 3.22
     */
    virtual QgsVectorLayer *createSqlVectorLayer( const SqlVectorLayerOptions &options ) const SIP_THROW( QgsProviderConnectionException ) SIP_FACTORY;

    /**
     * Returns the SQL layer options from a \a layerSource.
     *
     * \note the default implementation returns a default constructed option object.
     * \throws QgsProviderConnectionException if any errors are encountered or if SQL layer creation is not supported.
     * \since QGIS 3.22
     */
    virtual SqlVectorLayerOptions sqlOptions( const QString &layerSource ) SIP_THROW( QgsProviderConnectionException );

    /**
     * Executes raw \a sql and returns the (possibly empty) query results, optionally \a feedback can be provided.
     *
     * \see executeSql()
     * \throws QgsProviderConnectionException if any errors are encountered.
     * \since QGIS 3.18
     */
    virtual QueryResult execSql( const QString &sql, QgsFeedback *feedback = nullptr ) const SIP_THROW( QgsProviderConnectionException );

    /**
     * Vacuum the database table with given \a schema and \a name (schema is ignored if not supported by the backend).
     *
     * \throws QgsProviderConnectionException if any errors are encountered.
     */
    virtual void vacuum( const QString &schema, const QString &name ) const SIP_THROW( QgsProviderConnectionException );

    /**
     * Creates a spatial index for the database table with given \a schema and \a name (schema is ignored if not supported by the backend).
     *
     * The \a options argument can be used to provide extra options controlling the spatial index creation.
     *
     * \throws QgsProviderConnectionException if any errors are encountered.
     * \since QGIS 3.14
     */
    virtual void createSpatialIndex( const QString &schema, const QString &name, const QgsAbstractDatabaseProviderConnection::SpatialIndexOptions &options = QgsAbstractDatabaseProviderConnection::SpatialIndexOptions() ) const SIP_THROW( QgsProviderConnectionException );

    /**
     * Determines whether a spatial index exists for the database table with given \a schema, \a name and \a geometryColumn (\a schema and \a geometryColumn are
     * ignored if not supported by the backend).
     *
     * \throws QgsProviderConnectionException if any errors are encountered.
     * \since QGIS 3.14
     */
    virtual bool spatialIndexExists( const QString &schema, const QString &name, const QString &geometryColumn ) const SIP_THROW( QgsProviderConnectionException );

    /**
     * Deletes the existing spatial index for the database table with given \a schema, \a name and \a geometryColumn (\a schema and \a geometryColumn are
     * ignored if not supported by the backend).
     *
     * \throws QgsProviderConnectionException if any errors are encountered.
     * \since QGIS 3.14
     */
    virtual void deleteSpatialIndex( const QString &schema, const QString &name, const QString &geometryColumn ) const SIP_THROW( QgsProviderConnectionException );

    /**
     * Returns information on the tables in the given schema.
     *
     * \param schema name of the schema (ignored if not supported by the backend)
     * \param flags filter tables by flags, this option completely overrides search options stored in the connection
     * \throws QgsProviderConnectionException if any errors are encountered.
     * \note Not available in Python bindings
     */
    virtual QList<QgsAbstractDatabaseProviderConnection::TableProperty> tables( const QString &schema = QString(), const QgsAbstractDatabaseProviderConnection::TableFlags &flags = QgsAbstractDatabaseProviderConnection::TableFlags() ) const SIP_SKIP;

    /**
     * Returns information on a \a table in the given \a schema.
     *
     * \throws QgsProviderConnectionException if any errors are encountered or if the table does not exist.
     * \note Not available in Python bindings
     * \since QGIS 3.12
     */
    virtual QgsAbstractDatabaseProviderConnection::TableProperty table( const QString &schema, const QString &table ) const SIP_THROW( QgsProviderConnectionException );

    /**
     * Returns information on the tables in the given schema.
     *
     * \param schema name of the schema (ignored if not supported by the backend)
     * \param flags filter tables by flags, this option completely overrides search options stored in the connection
     * \throws QgsProviderConnectionException if any errors are encountered.
     */
    QList<QgsAbstractDatabaseProviderConnection::TableProperty> tablesInt( const QString &schema = QString(), const int flags = 0 ) const SIP_THROW( QgsProviderConnectionException ) SIP_PYNAME( tables );


    // TODO: return more schema information and not just the name

    /**
     * Returns information about the existing schemas.
     *
     * \throws QgsProviderConnectionException if any errors are encountered.
     */
    virtual QStringList schemas() const SIP_THROW( QgsProviderConnectionException );

    /**
     * Returns the fields of a \a table and \a schema.
     *
     * \note the default implementation creates a temporary vector layer, providers may
     * choose to override this method for a greater efficiency of to overcome provider's
     * behavior when the layer does not expose all fields (GPKG for example hides geometry
     * and primary key column).
     * \throws QgsProviderConnectionException if any errors are encountered.
     * \since QGIS 3.16
     */
    virtual QgsFields fields( const QString &schema, const QString &table ) const SIP_THROW( QgsProviderConnectionException );

    /**
     * Returns a list of native types supported by the connection.
     *
     * \throws QgsProviderConnectionException if any errors are encountered.
     * \since QGIS 3.16
     */
    virtual QList< QgsVectorDataProvider::NativeType > nativeTypes() const SIP_THROW( QgsProviderConnectionException ) = 0;

    /**
     * Returns the provider key.
     *
     * \since QGIS 3.16
     */
    QString providerKey() const;

    /**
    * Returns a dictionary of SQL keywords supported by the provider.
    * The default implementation returns an list of common reserved words under the
    * "Keyword" and "Constant" categories.
    *
    * Subclasses should add provider- and/or connection- specific words.
    *
    * \since QGIS 3.22
    */
    virtual QMultiMap<Qgis::SqlKeywordCategory, QStringList> sqlDictionary();

    /**
     * Returns a list of field domain names present on the provider.
     *
     * This is supported on providers with the Capability::ListFieldDomains capability only.
     *
     * \throws QgsProviderConnectionException if any errors are encountered.
     *
     * \see fieldDomain()
     * \since QGIS 3.26
     */
    virtual QStringList fieldDomainNames() const SIP_THROW( QgsProviderConnectionException );

    /**
     * Returns the field domain with the specified \a name from the provider.
     *
     * The caller takes ownership of the return object. Will return NULLPTR if no matching field domain is found.
     *
     * This is supported on providers with the Capability::RetrieveFieldDomain capability only.
     *
     * \throws QgsProviderConnectionException if any errors are encountered.
     *
     * \see fieldDomainNames()
     * \since QGIS 3.26
     */
    virtual QgsFieldDomain *fieldDomain( const QString &name ) const SIP_THROW( QgsProviderConnectionException ) SIP_FACTORY;

    /**
     * Sets the field domain name for the existing field with the specified name.
     *
     * \param fieldName name of the field to be modified
     * \param schema name of the schema (schema is ignored if not supported by the backend).
     * \param tableName name of the table
     * \param domainName name of the domain to set for the field. Must be an existing field domain (see fieldDomainNames()). Set to an empty string to remove a previously set domain.
     *
     * \throws QgsProviderConnectionException if any errors are encountered.
     * \since QGIS 3.26
     */
    virtual void setFieldDomainName( const QString &fieldName, const QString &schema, const QString &tableName, const QString &domainName ) const SIP_THROW( QgsProviderConnectionException );

    /**
     * Adds a new field \a domain to the database.
     *
     * \param domain field domain to add
     * \param schema name of the schema (schema is ignored if not supported by the backend).
     *
     * \throws QgsProviderConnectionException if any errors are encountered.
     * \since QGIS 3.26
     */
    virtual void addFieldDomain( const QgsFieldDomain &domain, const QString &schema ) const SIP_THROW( QgsProviderConnectionException );

  protected:

///@cond PRIVATE

    /**
     * Checks if \a capability is supported.
     *
     * \throws QgsProviderConnectionException if the capability is not supported
     */
    void checkCapability( Capability capability ) const;
///@endcond

    Capabilities mCapabilities = Capabilities() SIP_SKIP;
    GeometryColumnCapabilities mGeometryColumnCapabilities = GeometryColumnCapabilities() SIP_SKIP;
    Qgis::SqlLayerDefinitionCapabilities mSqlLayerDefinitionCapabilities = Qgis::SqlLayerDefinitionCapabilities() SIP_SKIP;
    QString mProviderKey;

};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsAbstractDatabaseProviderConnection::Capabilities )

#endif // QGSABSTRACTDATABASEPROVIDERCONNECTION_H
