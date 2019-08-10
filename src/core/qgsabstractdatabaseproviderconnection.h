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

#include <QObject>

/**
 * The QgsAbstractDatabaseProviderConnection class provides common functionality
 * for DB based connections.
 *
 * This class performs low level DB operations without asking
 * the user for confirmation or handling currently opened layers and the registry
 * entries, it is responsability of the client code to keep layers in sync.
 * The class methods will throw exceptions in case the requested operation
 * is not supported or cannot be performed without errors.
 *
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
    };

    Q_ENUMS( TableFlag )
    Q_DECLARE_FLAGS( TableFlags, TableFlag )
    Q_FLAG( TableFlags )

    /**
     * The TableProperty class represents a database table or view.
     *
     * In case the table is a vector spatial table and it has multiple
     * geometry columns, separate entries for each geometry column must
     * be created.
     *
     * In case the table is a vector spatial table and the geometry column
     * can contain multiple geometry types and/or SRIDs, a clone of the property
     * for the individual geometry type/SRID can be retrieved with at(i)
     */
    struct TableProperty
    {

#ifdef SIP_RUN
        SIP_PYOBJECT __repr__();
        % MethodCode
        QString str = QStringLiteral( "<QgsAbstractDatabaseProviderConnection.TableProperty: '%1'>" ).arg( sipCpp->tableName() );
        sipRes = PyUnicode_FromString( str.toUtf8().constData() );
        % End
#endif

      public:

        /**
         * Returns the table name
         * \see defaultName()
         */
        QString tableName() const;

        /**
         * Sets the table name to \a name
         * \see defaultName()
         */
        void setTableName( const QString &name );

        /**
         * Appends the geometry column \a type with the given \a srid to the geometry column types list
         */
        void addGeometryType( const QgsWkbTypes::Type &type, const int srid );

        /**
         * Returns the list of geometry column types and SRIDs.
         * The method returns a list of tuples (QgsWkbTypes::Type, SRID)
         */
        QList<QPair<int, int>> geometryTypes() const;

        /**
         * Returns the default name for the table entry
         *
         * It is usually the table name but in case there are multiple geometry
         * columns, the geometry column name is appendend to the table name.
         * \see geometryColumnCount()
         */
        QString  defaultName() const;

        /**
         * Returns the table property corresponding to the geometry type a
         * the given indext \a index
         */
        TableProperty at( int index ) const;

        /**
         * Returns the schema or an empty string for backends that do not support a schema
         */
        QString schema() const;

        /**
         * Sets the \a schema
         */
        void setSchema( const QString &schema );

        /**
         * Returns the geometry column name
         */
        QString geometryColumn() const;

        /**
         * Sets the geometry column name to \a geometryColumn
         */
        void setGeometryColumn( const QString &geometryColumn );

        /**
         * Returns the list of primary key column names
         */
        QStringList pkColumns() const;

        /**
         * Sets the primary key column names to \a pkColumns
         */
        void setPkColumns( const QStringList &pkColumns );

        /**
         * Returs the list of SRIDs supported by the geometry column
         */
        QList<int> srids() const;

        /**
         * Returns the table flags
         */
        TableFlags flags() const;

        /**
         * Sets the table \a flags
         */
        void setFlags( const TableFlags &flags );

        /**
         * Returns the table comment
         */
        QString comment() const;

        /**
         * Sets the table \a comment
         */
        void setComment( const QString &comment );

        /**
         * Returns the SQL where condition for the table
         */
        QString sql() const;

        /**
         * Set the table SQL where condition to \a sql
         */
        void setSql( const QString &sql );

        /**
         * Returns additional information about the table
         *
         * Provider classes may use this property
         * to store custom bits of information.
         */
        QList<QVariantMap> info() const;

        /**
         * Sets additional information about the table to \a info
         *
         * Provider classes may use this property
         * to store custom bits of information.
         */
        void setInfo( const QList<QVariantMap> &info );

        /**
         * Returns the number of geometry columns in the original table this entry refers to
         *
         * This information is used internally to build the \see defaultName()
         */
        int geometryColumnCount() const;

        /**
         * Sets the \a geometryColumnCount
         */
        void setGeometryColumnCount( int geometryColumnCount );

        /**
         * Sets a \a flag
         */
        void setFlag( const TableFlag &flag );

      private:

        //! Holds the list of geometry wkb types and srids supported by the table
        QList<QPair<int, int>>        mGeometryTypes;
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
        //! SQL where condition
        QString                       mSql;
        //! Additional unstructured information about the table
        QList<QVariantMap>            mInfo;
    };


    /**
     * The Capability enum represent the operations supported by the connection
     */
    enum Capability
    {
      CreateVectorTable = 1 << 1,   //!< Can CREATE a vector (or aspatial) table/layer
      // CreateRasterTable does not make much sense, skip it for now:
      DropRasterTable = 1 << 2,     //!< Can DROP a raster table/layer
      DropVectorTable = 1 << 3,     //!< Can DROP a vector (or aspatial) table/layer
      RenameTable = 1 << 4,         //!< Can RENAME a table/layer
      CreateSchema = 1 << 5,        //!< Can CREATE a schema
      DropSchema = 1 << 6,          //!< Can DROP a schema
      RenameSchema = 1 << 7,        //!< Can RENAME a schema
      ExecuteSql = 1 << 8,          //!< Can execute raw SQL queries (without returning results)
      // TODO: Transaction = 1 << 9,   //!< Supports transactions when executing operations
      Vacuum = 1 << 10,             //!< Can run vacuum
      Tables = 1 << 11,             //!< Can list tables
      Schemas = 1 << 12,            //!< Can list schemas (if not set, the connection does not support schemas)
      SqlLayers = 1 << 13,          //!< Can create vector layers from SQL SELECT queries
      // TODO: TruncateTable = 1 << 14,      //!< Can TRUNCATE a table
      TableExists = 1 << 15,        //!< Can check if table exists
      Spatial = 1 << 16,            //!< The connection supports spatial tables
    };

    Q_ENUM( Capability )
    Q_DECLARE_FLAGS( Capabilities, Capability )
    Q_FLAG( Capabilities )

    QgsAbstractDatabaseProviderConnection( const QString &name );
    QgsAbstractDatabaseProviderConnection( const QString &name, const QString &uri );

    // Public interface

    /**
     * Returns connection capabilities
     */
    Capabilities capabilities() const;

    // Operations interface

    /**
     * Creates an empty table with \a name in the given \a schema (schema is ignored  if not supported by the backend)
     */
    virtual void createVectorTable( const QString &schema, const QString &name, const QgsFields &fields, QgsWkbTypes::Type wkbType, const QgsCoordinateReferenceSystem &srs, bool overwrite, const QMap<QString, QVariant> *options ) const SIP_THROW( QgsProviderConnectionException );

    /**
     * Checks whether a table \a name exists in the given \a schema
     * \throws QgsProviderConnectionException
     */
    virtual bool tableExists( const QString &schema, const QString &name ) const SIP_THROW( QgsProviderConnectionException );

    /**
     * Drops a vector (or aspatial) table with given \a schema (schema is ignored if not supported by the backend) and \a name
     * \note it is responsability of the caller to handle opened layers and registry entries.
     * \throws QgsProviderConnectionException
     */
    virtual void dropVectorTable( const QString &schema, const QString &name ) const SIP_THROW( QgsProviderConnectionException );

    /**
     * Drops a raster table with given \a schema (schema is ignored  if not supported by the backend) and \a name
     * \note it is responsability of the caller to handle opened layers and registry entries.
     * \throws QgsProviderConnectionException
     */
    virtual void dropRasterTable( const QString &schema, const QString &name ) const SIP_THROW( QgsProviderConnectionException );

    /**
     * Renames a table with given \a schema (schema is ignored  if not supported by the backend) and \a name
     * \note it is responsability of the caller to handle opened layers and registry entries.
     * \throws QgsProviderConnectionException
     */
    virtual void renameTable( const QString &schema, const QString &name, const QString &newName ) const SIP_THROW( QgsProviderConnectionException );

    /**
     * Creates a new schema \a schema
     * \throws QgsProviderConnectionException
     */
    virtual void createSchema( const QString &name ) const SIP_THROW( QgsProviderConnectionException );

    /**
     * Drops an entire \a schema
     * \param force if TRUE, a DROP CASCADE will drop all related objects
     * \note it is responsability of the caller to handle opened layers and registry entries.
     * \throws QgsProviderConnectionException
     */
    virtual void dropSchema( const QString &name, bool force = false ) const SIP_THROW( QgsProviderConnectionException );

    /**
     * Renames a \a schema
     * \note it is responsability of the caller to handle opened layers and registry entries.
     * \throws QgsProviderConnectionException
     */
    virtual void renameSchema( const QString &name, const QString &newName ) const SIP_THROW( QgsProviderConnectionException );

    /**
     * Executes raw \a sql and returns the (possibly empty) list of results in a multi-dimensional array
     * \throws QgsProviderConnectionException
     */
    virtual QList<QList<QVariant>> executeSql( const QString &sql ) const SIP_THROW( QgsProviderConnectionException );

    /**
     * Vacuum the database table with given \a schema (schema is ignored  if not supported by the backend) and \a name
     * \throws QgsProviderConnectionException
     */
    virtual void vacuum( const QString &schema, const QString &name ) const SIP_THROW( QgsProviderConnectionException );

    /**
     * Returns information on the tables in the given \a schema (schema is ignored if not supported by the backend)
     * \param flags filter tables by flags, this option completely overrides search options stored in the connection
     * \throws QgsProviderConnectionException
     * \note Not available in Python bindings
     */
    virtual QList<QgsAbstractDatabaseProviderConnection::TableProperty> tables( const QString &schema = QString(), const QgsAbstractDatabaseProviderConnection::TableFlags &flags = nullptr ) const SIP_SKIP;

    /**
     * Returns information on the tables in the given \a schema (schema is ignored if not supported by the backend)
     * \param flags filter tables by flags, this option completely overrides search options stored in the connection
     * \throws QgsProviderConnectionException
     */
    QList<QgsAbstractDatabaseProviderConnection::TableProperty> tablesInt( const QString &schema = QString(), const int flags = 0 ) const SIP_THROW( QgsProviderConnectionException ) SIP_PYNAME( tables );


    // TODO: return more schema information and not just the name

    /**
     * Returns information about the existing schemas
     * \throws QgsProviderConnectionException
     */
    virtual QStringList schemas( ) const SIP_THROW( QgsProviderConnectionException );

  protected:

///@cond PRIVATE

    /**
     * Checks if \a capability is supported and throws and exception if it's not
     * \throws QgsProviderConnectionException
     */
    void checkCapability( Capability capability ) const;
///@endcond

    Capabilities mCapabilities = nullptr SIP_SKIP;

};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsAbstractDatabaseProviderConnection::Capabilities )

#endif // QGSABSTRACTDATABASEPROVIDERCONNECTION_H
