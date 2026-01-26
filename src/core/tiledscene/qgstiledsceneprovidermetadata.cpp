/***************************************************************************
  qgstiledsceneprovidermetadata.cpp
  --------------------------------------
  Date                 : June 2023
  Copyright            : (C) 2023 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstiledsceneprovidermetadata.h"

#include "qgsapplication.h"
#include "qgstiledsceneconnection.h"
#include "qgstiledscenedataitems.h"

#include <QIcon>

#include "moc_qgstiledsceneprovidermetadata.cpp"

///@cond PRIVATE

#define PROVIDER_KEY u"tiledscene"_s
#define PROVIDER_DESCRIPTION u"Tiled scene provider"_s

QgsTiledSceneProviderMetadata::QgsTiledSceneProviderMetadata()
  : QgsProviderMetadata( PROVIDER_KEY, PROVIDER_DESCRIPTION )
{
}

QIcon QgsTiledSceneProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( u"mIconTiledSceneLayer.svg"_s );
}

QList<QgsDataItemProvider *> QgsTiledSceneProviderMetadata::dataItemProviders() const
{
  return
  {
    new QgsTiledSceneDataItemProvider()
  };
}

QMap<QString, QgsAbstractProviderConnection *> QgsTiledSceneProviderMetadata::connections( bool cached )
{
  return connectionsProtected<QgsTiledSceneProviderConnection, QgsTiledSceneProviderConnection>( cached );
}

QgsAbstractProviderConnection *QgsTiledSceneProviderMetadata::createConnection( const QString &name )
{
  return new QgsTiledSceneProviderConnection( name );
}

void QgsTiledSceneProviderMetadata::deleteConnection( const QString &name )
{
  deleteConnectionProtected<QgsTiledSceneProviderConnection>( name );
}

void QgsTiledSceneProviderMetadata::saveConnection( const QgsAbstractProviderConnection *connection, const QString &name )
{
  saveConnectionProtected( connection, name );
}

QgsProviderMetadata::ProviderCapabilities QgsTiledSceneProviderMetadata::providerCapabilities() const
{
  return QgsProviderMetadata::ProviderCapabilities();
}


///@endcond
