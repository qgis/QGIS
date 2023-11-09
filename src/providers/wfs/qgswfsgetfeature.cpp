/***************************************************************************
    qgswfsgetfeature.cpp
    ---------------------
    begin                : November 2022
    copyright            : (C) 2022 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswfsgetfeature.h"
#include "qgsmessagelog.h"
#include <QUrlQuery>

QgsWFSGetFeature::QgsWFSGetFeature( QgsWFSDataSourceURI &uri )
  : QgsWfsRequest( uri )
{
}

bool QgsWFSGetFeature::request( bool synchronous, const QString &WFSVersion,
                                const QString &typeName, const QString &filter, bool hitsOnly, const QgsWfsCapabilities::Capabilities &caps )
{
  QUrl url( mUri.requestUrl( QStringLiteral( "GetFeature" ) ) );
  QUrlQuery query( url );
  query.addQueryItem( QStringLiteral( "VERSION" ), WFSVersion );

  const QString namespaceValue( caps.getNamespaceParameterValue( WFSVersion, typeName ) );

  if ( WFSVersion.startsWith( QLatin1String( "2.0" ) ) )
  {
    query.addQueryItem( QStringLiteral( "TYPENAMES" ), typeName );
    if ( !namespaceValue.isEmpty() )
    {
      query.addQueryItem( QStringLiteral( "NAMESPACES" ), namespaceValue );
    }
  }
  else
  {
    query.addQueryItem( QStringLiteral( "TYPENAME" ), typeName );
  }

  if ( !namespaceValue.isEmpty() )
  {
    query.addQueryItem( QStringLiteral( "NAMESPACE" ), namespaceValue );
  }

  if ( !filter.isEmpty() )
  {
    query.addQueryItem( QStringLiteral( "FILTER" ), filter );
  }

  if ( hitsOnly )
  {
    query.addQueryItem( QStringLiteral( "RESULTTYPE" ), "hits" );
  }

  url.setQuery( query );
  return sendGET( url, QString(), synchronous, /*forceRefresh=*/ true, /* cache=*/ false );
}

QString QgsWFSGetFeature::errorMessageWithReason( const QString &reason )
{
  return tr( "Download of feature type failed: %1" ).arg( reason );
}
