/***************************************************************************
  qgsafsproviderconnection.h - QgsAfsProviderConnection

 ---------------------
 begin                : 21.11.2019
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
#ifndef QGSAFSPROVIDERCONNECTION_H
#define QGSAFSPROVIDERCONNECTION_H

#include "qgsabstractwebserviceproviderconnection.h"

class QgsAfsProviderConnection : public QgsAbstractWebServiceProviderConnection
{
  public:

    QgsAfsProviderConnection( const QString &name );
    QgsAfsProviderConnection( const QString &uri, const QVariantMap &configuration );


    // QgsAbstractWebServiceProviderConnection interface
  public:
    QString layerUri( const QString &layerName ) const override;
    QList<QgsAbstractWebServiceProviderConnection::LayerProperty> layers( const QgsAbstractWebServiceProviderConnection::LayerFlags &flags ) const override;

  private:
    void setDefaultCapabilities();
};

#endif // QGSAFSPROVIDERCONNECTION_H
