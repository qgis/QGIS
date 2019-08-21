/***************************************************************************
  qgsabstractproviderconnection.cpp - QgsAbstractProviderConnection

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
#include "qgsabstractproviderconnection.h"


QgsAbstractProviderConnection::QgsAbstractProviderConnection( const QString &name )
{
  Q_UNUSED( name );
  // Note: concrete classes must implement the logic to read the configuration from the settings
  //       and create mUri
}

QgsAbstractProviderConnection::QgsAbstractProviderConnection( const QString &uri, const QVariantMap &configuration )
  : mUri( uri )
  , mConfiguration( configuration )
{

}

QString QgsAbstractProviderConnection::uri() const
{
  return mUri;
}

void QgsAbstractProviderConnection::setUri( const QString &uri )
{
  mUri = uri;
}

QVariantMap QgsAbstractProviderConnection::configuration() const
{
  return mConfiguration;
}

void QgsAbstractProviderConnection::setConfiguration( const QVariantMap &configuration )
{
  mConfiguration = configuration;
}
