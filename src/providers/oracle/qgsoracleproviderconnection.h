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

    void store( const QString &name ) const override;
    void remove( const QString &name ) const override;
    QList<QgsVectorDataProvider::NativeType> nativeTypes() const override;

  private:

    void setDefaultCapabilities();
};


#endif // QGSORACLEPROVIDERCONNECTION_H
