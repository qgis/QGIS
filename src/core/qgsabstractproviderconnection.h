/***************************************************************************
  qgsabstractproviderconnection.h - QgsAbstractProviderConnection

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
#ifndef QGSABSTRACTPROVIDERCONNECTION_H
#define QGSABSTRACTPROVIDERCONNECTION_H

#include <QObject>
#include "qgis_core.h"


/**
 * The QgsAbstractProviderConnection provides an interface for data provider connections
 */
class CORE_EXPORT QgsAbstractProviderConnection
{
    Q_GADGET

  public:

    /**
     * The Capability enum represent the operations supported by the connection
     */
    enum Capability
    {
      CreateTable = 1 << 1,   //!< Can create a table/layer
      DropTable = 1 << 2,     //!< Can drop a table/layer
      RenameTable = 1 << 3,   //!< Can rename a table/layer
      CreateSchema = 1 << 4,  //!< Can create a schema
      DropSchema = 1 << 5,    //!< Can drop a schema
      RenameSchema = 1 << 6,  //!< Can rename a schema
      ExecuteSQL = 1 << 7,    //!< Can execute raw SQL queries
      // TODO Transaction = 1 << 8,   //!< Supports transactions when executing operations
      Vacuum = 1 << 9,        //!< Can run vacuum
      Tables = 1 << 10,       //!< Can list tables
      Schemas = 1 << 11,      //!< Can list schemas
    };

    Q_ENUMS( Capability )
    Q_DECLARE_FLAGS( Capabilities, Capability )
    Q_FLAGS( Capabilities )

    QgsAbstractProviderConnection();

    virtual ~QgsAbstractProviderConnection() = default;

    // Public interface

    /**
     * Returns connection capabilities
     */
    Capabilities capabilities() const;

    // Operations interface

    virtual bool createTable( const QString &name, const QString schema, QString &errCause );
    virtual bool dropTable( const QString &name, const QString schema, QString &errCause );
    virtual bool renameTable( const QString &name, const QString schema, const QString &newName, QString &errCause );
    virtual bool createSchema( const QString &name, QString &errCause );
    virtual bool dropSchema( const QString &name, QString &errCause );
    virtual bool renameSchema( const QString &name, const QString &newName, QString &errCause );
    virtual QVariant executeSql( const QString &sql, QString &errCause );
    virtual bool vacuum( const QString &name, QString &errCause );
    virtual QStringList tables( const QString schema, QString &errCause );
    virtual QStringList schemas( QString &errCause );

  protected:

    Capabilities mCapabilities = nullptr;


};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsAbstractProviderConnection::Capabilities )

#endif // QGSABSTRACTPROVIDERCONNECTION_H
