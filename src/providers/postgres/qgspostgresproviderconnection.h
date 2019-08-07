/***************************************************************************
  qgspostgresproviderconnection.h - QgsPostgresProviderConnection

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
#ifndef QGSPOSTGRESPROVIDERCONNECTION_H
#define QGSPOSTGRESPROVIDERCONNECTION_H
#include "qgsabstractdatabaseproviderconnection.h"


class QgsPostgresProviderConnection : public QgsAbstractDatabaseProviderConnection
{
  public:

    QgsPostgresProviderConnection( const QString &name );
    QgsPostgresProviderConnection( const QString &name, const QString &uri );

    // QgsAbstractProviderConnection interface

  public:

    void createVectorTable( const QString &schema,
                            const QString &name,
                            const QgsFields &fields,
                            QgsWkbTypes::Type wkbType,
                            const QgsCoordinateReferenceSystem &srs, bool overwrite,
                            const QMap<QString, QVariant> *options ) override;

    void dropTable( const QString &schema, const QString &name ) override;
    void renameTable( const QString &schema, const QString &name, const QString &newName ) override;
    void createSchema( const QString &name ) override;
    void dropSchema( const QString &name ) override;
    void renameSchema( const QString &name, const QString &newName ) override;
    void executeSql( const QString &sql ) override;
    void vacuum( const QString &schema, const QString &name ) override;
    QStringList tables( const QString &schema ) override;
    QStringList schemas( ) override;
    void store( QVariantMap guiConfig = QVariantMap() ) override;
    void remove() override;


  private:

    void executeSqlPrivate( const QString &sql );
    void setDefaultCapabilities();
};

#endif // QGSPOSTGRESPROVIDERCONNECTION_H
