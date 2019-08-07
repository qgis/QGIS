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
 * for DB based connections, it performs low level DB operations without asking
 * the user for confirmation or handling currently opened layers and the registry
 * entries, it is responsability of the client code to keep layers in sync.
 * The class methods will throw exceptions in case the requested operation
 * is not supported or cannot be performed without errors.
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsAbstractDatabaseProviderConnection : public QgsAbstractProviderConnection
{

    Q_GADGET

  public:

    /**
     * Flags for table properties
     */
    enum TableFlag
    {
      None = 0,                   //!< No flags
      Aspatial = 1 << 1,          //!< Aspatial table
      Vector = 1 << 2,            //!< Vector table
      Raster = 1 << 3,            //!< Raster table
      View = 1 << 4,              //!< View table
      MaterializedView = 1 << 5,  //!< Materialized view table
    };
    Q_ENUMS( TableFlag )
    Q_DECLARE_FLAGS( TableFlags, TableFlag )
    Q_FLAG( TableFlags )

    /**
     * The TableProperty struct represents a database table
     */
    struct TableProperty
    {

#ifdef SIP_RUN
      SIP_PYOBJECT __repr__();
      % MethodCode
      QString str = QStringLiteral( "<QgsAbstractDatabaseProviderConnection.TableProperty: '%1'>" ).arg( sipCpp->name );
      sipRes = PyUnicode_FromString( str.toUtf8().constData() );
      % End
#endif

      QList<QgsWkbTypes::Type>      types;
      QString                       schema;
      QString                       name;
      QString                       geometryColumn;
      QStringList                   pkColumns;
      QList<int>                    srids;
      unsigned int                  spatialColumnCount;
      TableFlags                    flags;
      QString                       tableComment;
      QString                       sql;

      int geometryColumnCount() const { Q_ASSERT( types.size() == srids.size() ); return types.size(); }

      /**
       * Returns the default name for the layer.
       * It is usually the table name but in case there are multiple geometry
       * columns, the geometry column name is appendend to the table name.
       * @return
       */
      QString  defaultName() const
      {
        QString n = name;
        if ( spatialColumnCount > 1 ) n += '.' + geometryColumn;
        return n;
      }

      TableProperty at( int i ) const
      {
        TableProperty property;

        Q_ASSERT( i >= 0 && i < geometryColumnCount() );

        property.types << types[ i ];
        property.srids << srids[ i ];
        property.schema = schema;
        property.name = name;
        property.geometryColumn = geometryColumn;
        property.pkColumns = pkColumns;
        property.spatialColumnCount = spatialColumnCount;
        property.sql = sql;
        property.tableComment = tableComment;
        property.flags = flags;
        return property;
      }
    };

    /**
     * The Capability enum represent the operations supported by the connection
     */
    enum Capability
    {
      CreateVectorTable = 1 << 1,   //!< Can CREATE a vector table/layer
      CreateRasterTable = 1 << 2,   //!< Can CREATE a raster table/layer
      DropTable = 1 << 3,           //!< Can DROP a table/layer
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
    };

    Q_ENUMS( Capability )
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
     * Creates an empty table with \a name in the given \a schema (may be empty if not supported by the backend)
     */
    virtual void createVectorTable( const QString &schema,
                                    const QString &name,
                                    const QgsFields &fields,
                                    QgsWkbTypes::Type wkbType,
                                    const QgsCoordinateReferenceSystem &srs,
                                    bool overwrite,
                                    const QMap<QString, QVariant> *options
                                  ) SIP_THROW( QgsProviderConnectionException );

    /**
     * Create a new raster table with given \a schema and \a name (may be empty if not supported by the backend)
     * \throws QgsProviderConnectionException
     */
    virtual void createRasterTable( const QString &schema,
                                    const QString &name ) SIP_THROW( QgsProviderConnectionException );

    /**
     * Drops a table with given \a schema (may be empty if not supported by the backend) and \a name
     * \note it is responsability of the caller to handle opened layers and registry entries.
     * \throws QgsProviderConnectionException
     */
    virtual void dropTable( const QString &schema, const QString &name ) SIP_THROW( QgsProviderConnectionException );

    /**
     * Renames a table with given \a schema (may be empty if not supported by the backend) and \a name
     * \note it is responsability of the caller to handle opened layers and registry entries.
     * \throws QgsProviderConnectionException
     */
    virtual void renameTable( const QString &schema, const QString &name, const QString &newName ) SIP_THROW( QgsProviderConnectionException );

    /**
     * Creates anew schema \a schema
     * \throws QgsProviderConnectionException
     */
    virtual void createSchema( const QString &name ) SIP_THROW( QgsProviderConnectionException );

    /**
     * Drops an entire \a schema
     * \param force if TRUE, a DROP CASCADE will drop all related objects
     * \note it is responsability of the caller to handle opened layers and registry entries.
     * \throws QgsProviderConnectionException
     */
    virtual void dropSchema( const QString &name, bool force = false ) SIP_THROW( QgsProviderConnectionException );

    /**
     * Renames a \a schema
     * \note it is responsability of the caller to handle opened layers and registry entries.
     * \throws QgsProviderConnectionException
     */
    virtual void renameSchema( const QString &name, const QString &newName ) SIP_THROW( QgsProviderConnectionException );

    /**
     * Executes raw \a sql
     * \throws QgsProviderConnectionException
     */
    virtual void executeSql( const QString &sql ) SIP_THROW( QgsProviderConnectionException );

    /**
     * Vacuum the database table with given \a schema (may be empty if not supported by the backend) and \a name
     * \throws QgsProviderConnectionException
     */
    virtual void vacuum( const QString &schema, const QString &name ) SIP_THROW( QgsProviderConnectionException );

    /**
     * Returns information on the tables in the given \a schema (this parameter is ignored if not supported by the backend)
     * \param flags filter tables by flags
     * \throws QgsProviderConnectionException
     */
    virtual QList<QgsAbstractDatabaseProviderConnection::TableProperty> tables( const QString &schema = QString(),
        const QgsAbstractDatabaseProviderConnection::TableFlags &flags = QgsAbstractDatabaseProviderConnection::TableFlag::None ) SIP_THROW( QgsProviderConnectionException );

    // TODO: return schema information and not just the name

    /**
     * Returns information about the existing schemas
     * \throws QgsProviderConnectionException
     */
    virtual QStringList schemas( ) SIP_THROW( QgsProviderConnectionException );

  protected:

    Capabilities mCapabilities = nullptr;

};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsAbstractDatabaseProviderConnection::Capabilities )

#endif // QGSABSTRACTDATABASEPROVIDERCONNECTION_H
