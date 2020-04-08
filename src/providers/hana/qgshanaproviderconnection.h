/***************************************************************************
   qgshanaproviderconnection.h  -  QgsHanaProviderConnection
   --------------------------------------
   Date      : 07-04-2020
   Copyright : (C) SAP SE
   Author    : Maksim Rylov
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#ifndef QGSHANAPROVIDERCONNECTION_H
#define QGSHANAPROVIDERCONNECTION_H

#include "qgsabstractdatabaseproviderconnection.h"

class QgsHanaProviderConnection : public QgsAbstractDatabaseProviderConnection
{
  public:

    QgsHanaProviderConnection( const QString &name );
    QgsHanaProviderConnection( const QString &uri, const QVariantMap &configuration );

    // QgsAbstractProviderConnection interface

  public:

    void createVectorTable( const QString &schema,
                            const QString &name,
                            const QgsFields &fields,
                            QgsWkbTypes::Type wkbType,
                            const QgsCoordinateReferenceSystem &srs, bool overwrite,
                            const QMap<QString, QVariant> *options ) const override;

    QString tableUri( const QString &schema, const QString &name ) const override;
    void dropVectorTable( const QString &schema, const QString &name ) const override;
    void renameVectorTable( const QString &schema, const QString &name, const QString &newName ) const override;
    void createSchema( const QString &name ) const override;
    void dropSchema( const QString &name, bool force = false ) const override;
    QList<QVariantList> executeSql( const QString &sql ) const override;
    QList<QgsAbstractDatabaseProviderConnection::TableProperty> tables( const QString &schema,
        const TableFlags &flags = nullptr ) const override;
    QStringList schemas( ) const override;
    void store( const QString &name ) const override;
    void remove( const QString &name ) const override;
    QIcon icon() const override;

  private:

    QList<QVariantList> executeSqlQuery( const QString &sql ) const;
    void executeSqlStatement( const QString &sql ) const;
    void setCapabilities();
    void dropTable( const QString &schema, const QString &name ) const;
    void renameTable( const QString &schema, const QString &name, const QString &newName ) const;
};

#endif // QGSHANAPROVIDERCONNECTION_H
