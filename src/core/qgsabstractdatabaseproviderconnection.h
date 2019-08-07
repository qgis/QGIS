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
 * The class methods will throw exceptions in case the operation could not be
 * performed.
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsAbstractDatabaseProviderConnection : public QgsAbstractProviderConnection
{

    Q_GADGET

  public:

    /**
     * The TableProperty struct represents a database table
     */
    struct TableProperty
    {
      QList<QgsWkbTypes::Type>      types;
      QString                       schemaName;
      QString                       tableName;
      QString                       geometryColumnName;
      QStringList                   pkColumns;
      QList<int>                    srids;
      unsigned int                  nSpColumns;
      QString                       sql;
      bool                          isAspatial = false;
      bool                          isView = false;
      bool                          isMaterializedView = false;
      bool                          isRaster = false;
      QString                       tableComment;

      int geometryColumnCount() const { Q_ASSERT( types.size() == srids.size() ); return types.size(); }

      QString   defaultName() const
      {
        QString n = tableName;
        if ( nSpColumns > 1 ) n += '.' + geometryColumnName;
        return n;
      }

      TableProperty at( int i ) const
      {
        TableProperty property;

        Q_ASSERT( i >= 0 && i < geometryColumnCount() );

        property.types << types[ i ];
        property.srids << srids[ i ];
        property.schemaName      = schemaName;
        property.tableName       = tableName;
        property.geometryColumnName = geometryColumnName;
        property.pkColumns       = pkColumns;
        property.nSpColumns      = nSpColumns;
        property.sql             = sql;
        property.isView          = isView;
        property.isAspatial      = isAspatial;
        property.isRaster        = isRaster;
        property.isMaterializedView = isMaterializedView;
        property.tableComment    = tableComment;

        return property;
      }
    };

    /**
     * The Capability enum represent the operations supported by the connection
     */
    enum Capability
    {
      CreateVectorTable = 1 << 1,   //!< Can create a vector table/layer
      CreateRasterTable = 1 << 2,   //!< Can create a raster table/layer
      DropTable = 1 << 3,           //!< Can drop a table/layer
      RenameTable = 1 << 4,         //!< Can rename a table/layer
      CreateSchema = 1 << 5,        //!< Can create a schema
      DropSchema = 1 << 6,          //!< Can drop a schema
      RenameSchema = 1 << 7,        //!< Can rename a schema
      ExecuteSql = 1 << 8,          //!< Can execute raw SQL queries
      // TODO Transaction = 1 << 9,   //!< Supports transactions when executing operations
      Vacuum = 1 << 10,             //!< Can run vacuum
      Tables = 1 << 11,             //!< Can list tables
      Schemas = 1 << 12,            //!< Can list schemas
      SqlLayers = 1 << 13,          //!< Can create layers from SQL queries
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
     * \note it is responsability of the caller to handle opened layers and registry entries.
     * \throws QgsProviderConnectionException
     */
    virtual void dropSchema( const QString &name ) SIP_THROW( QgsProviderConnectionException );

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

    // TODO: return table information and not just the name

    /**
     * Returns tables information for the given \a schema (may be empty if not supported by the backend)
     * \throws QgsProviderConnectionException
     */
    virtual QList<QgsAbstractDatabaseProviderConnection::TableProperty> tables( const QString &schema = QString() ) SIP_THROW( QgsProviderConnectionException );

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
