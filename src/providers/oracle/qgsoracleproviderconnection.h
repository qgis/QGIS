/***************************************************************************
  qgsOracleproviderconnection.h - QgsOracleProviderConnection

 ---------------------
 begin                : 28.12.2020
 copyright            : (C) 2020 by Julien Cabieces
 email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSORACLEPROVIDERCONNECTION_H
#define QGSORACLEPROVIDERCONNECTION_H
#include "qgsabstractdatabaseproviderconnection.h"


class QgsOracleProviderConnection : public QgsAbstractDatabaseProviderConnection

{
  public:

    QgsOracleProviderConnection( const QString &name );
    QgsOracleProviderConnection( const QString &uri, const QVariantMap &configuration );

    // QgsAbstractProviderConnection interface

    void createVectorTable( const QString &schema,
                            const QString &name,
                            const QgsFields &fields,
                            QgsWkbTypes::Type wkbType,
                            const QgsCoordinateReferenceSystem &srs, bool overwrite,
                            const QMap<QString, QVariant> *options ) const override;

    QString tableUri( const QString &schema, const QString &name ) const override;
    void dropVectorTable( const QString &schema, const QString &name ) const override;
    void createSchema( const QString &name ) const override;
    void dropSchema( const QString &name, bool force = false ) const override;

    QList<QgsAbstractDatabaseProviderConnection::TableProperty> tables( const QString &schema,
        const TableFlags &flags = TableFlags() ) const override;
    QStringList schemas( ) const override;
    void store( const QString &name ) const override;
    void remove( const QString &name ) const override;
    QList<QgsVectorDataProvider::NativeType> nativeTypes() const override;

  private:

    QList<QVariantList> executeSqlPrivate( const QString &sql, QgsFeedback *feedback = nullptr ) const;
    void setDefaultCapabilities();
};


#endif // QGSORACLEPROVIDERCONNECTION_H
