/***************************************************************************
  qgsvectortileprovidermetadata.cpp
  --------------------------------------
  Date                 : March 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectortileprovidermetadata.h"

#include "qgsvectortileconnection.h"
#include "qgsvectortiledataitems.h"

///@cond PRIVATE

#define PROVIDER_KEY QStringLiteral( "vectortile" )
#define PROVIDER_DESCRIPTION QStringLiteral( "Vector tile provider" )

QgsVectorTileProviderMetadata::QgsVectorTileProviderMetadata()
  : QgsProviderMetadata( PROVIDER_KEY, PROVIDER_DESCRIPTION )
{
}

QList<QgsDataItemProvider *> QgsVectorTileProviderMetadata::dataItemProviders() const
{
  QList< QgsDataItemProvider * > providers;
  providers << new QgsVectorTileDataItemProvider;
  return providers;
}

QMap<QString, QgsAbstractProviderConnection *> QgsVectorTileProviderMetadata::connections( bool cached )
{
  return connectionsProtected<QgsVectorTileProviderConnection, QgsVectorTileProviderConnection>( cached );
}

QgsAbstractProviderConnection *QgsVectorTileProviderMetadata::createConnection( const QString &name )
{
  return new QgsVectorTileProviderConnection( name );
}

void QgsVectorTileProviderMetadata::deleteConnection( const QString &name )
{
  deleteConnectionProtected<QgsVectorTileProviderConnection>( name );
}

void QgsVectorTileProviderMetadata::saveConnection( const QgsAbstractProviderConnection *connection, const QString &name )
{
  saveConnectionProtected( connection, name );
}

///@endcond
