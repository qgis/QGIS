/***************************************************************************
  qgstiledmeshprovidermetadata.cpp
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

#include "qgstiledmeshprovidermetadata.h"
#include "qgstiledmeshconnection.h"
#include "qgsapplication.h"
#include "qgstiledmeshdataitems.h"

#include <QIcon>

///@cond PRIVATE

#define PROVIDER_KEY QStringLiteral( "tiledmesh" )
#define PROVIDER_DESCRIPTION QStringLiteral( "Tiled mesh provider" )

QgsTiledMeshProviderMetadata::QgsTiledMeshProviderMetadata()
  : QgsProviderMetadata( PROVIDER_KEY, PROVIDER_DESCRIPTION )
{
}

QIcon QgsTiledMeshProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconTiledMeshLayer.svg" ) );
}

QList<QgsDataItemProvider *> QgsTiledMeshProviderMetadata::dataItemProviders() const
{
  return
  {
    new QgsTiledMeshDataItemProvider()
  };
}

QMap<QString, QgsAbstractProviderConnection *> QgsTiledMeshProviderMetadata::connections( bool cached )
{
  return connectionsProtected<QgsTiledMeshProviderConnection, QgsTiledMeshProviderConnection>( cached );
}

QgsAbstractProviderConnection *QgsTiledMeshProviderMetadata::createConnection( const QString &name )
{
  return new QgsTiledMeshProviderConnection( name );
}

void QgsTiledMeshProviderMetadata::deleteConnection( const QString &name )
{
  deleteConnectionProtected<QgsTiledMeshProviderConnection>( name );
}

void QgsTiledMeshProviderMetadata::saveConnection( const QgsAbstractProviderConnection *connection, const QString &name )
{
  saveConnectionProtected( connection, name );
}

QgsProviderMetadata::ProviderCapabilities QgsTiledMeshProviderMetadata::providerCapabilities() const
{
  return QgsProviderMetadata::ProviderCapabilities();
}


///@endcond
