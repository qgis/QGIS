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

#include <QObject>

/**
 * The QgsAbstractDatabaseProviderConnection class provides common functionality
 * for DB based connections
 */
class CORE_EXPORT QgsAbstractDatabaseProviderConnection : public QgsAbstractProviderConnection
{

    Q_GADGET

  public:

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
      Vacuum = 1 << 10,              //!< Can run vacuum
      Tables = 1 << 11,             //!< Can list tables
      Schemas = 1 << 12,            //!< Can list schemas
      SqlLayers = 1 << 13,          //!< Can create layers from SQL queries
    };

    Q_ENUMS( Capability )
    Q_DECLARE_FLAGS( Capabilities, Capability )
    Q_FLAG( Capabilities )

    QgsAbstractDatabaseProviderConnection( const QString &name );
    QgsAbstractDatabaseProviderConnection( const QString &name, const QgsDataSourceUri &uri );

    // Public interface

    /**
     * Returns connection capabilities
     */
    Capabilities capabilities() const;

    // Operations interface

    /**
     * Creates an empty table with \a name in the given \a schema
     */
    virtual bool createVectorTable( const QString &schema,
                                    const QString &name,
                                    const QgsFields &fields,
                                    QgsWkbTypes::Type wkbType,
                                    const QgsCoordinateReferenceSystem &srs,
                                    bool overwrite,
                                    const QMap<QString, QVariant> *options,
                                    QString &errCause );
    // TODO
    virtual bool createRasterTable( const QString &schema,
                                    const QString &name,
                                    QString &errCause );
    virtual bool dropTable( const QString &schema, const QString &name, QString &errCause );
    virtual bool renameTable( const QString &schema, const QString &name, const QString &newName, QString &errCause );
    virtual bool createSchema( const QString &name, QString &errCause );
    virtual bool dropSchema( const QString &name, QString &errCause );
    virtual bool renameSchema( const QString &name, const QString &newName, QString &errCause );
    virtual bool executeSql( const QString &sql, QString &errCause );
    virtual bool vacuum( const QString &schema, const QString &name, QString &errCause );
    // TODO: return table information and not just the name
    virtual QStringList tables( const QString &schema, QString &errCause );
    // TODO: return schema information and not just the name
    virtual QStringList schemas( QString &errCause );

  protected:

    Capabilities mCapabilities = nullptr;

};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsAbstractDatabaseProviderConnection::Capabilities )

#endif // QGSABSTRACTDATABASEPROVIDERCONNECTION_H
