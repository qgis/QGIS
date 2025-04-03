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
#include "moc_qgswfsgetfeature.cpp"
#include "qgsmessagelog.h"
#include "qgswfsconstants.h"
#include <QUrlQuery>

QgsWFSGetFeature::QgsWFSGetFeature( QgsWFSDataSourceURI &uri )
  : QgsWfsRequest( uri )
{
}

bool QgsWFSGetFeature::request( bool synchronous, const QString &WFSVersion, const QString &typeName, const QString &filter, bool hitsOnly, const QgsWfsCapabilities &caps )
{
  QUrl url( mUri.requestUrl( QStringLiteral( "GetFeature" ), mUri.httpMethod() ) );

  switch ( mUri.httpMethod() )
  {
    case Qgis::HttpMethod::Get:
    {
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
      return sendGET( url, QString(), synchronous, /*forceRefresh=*/true, /* cache=*/false );
    }
    case Qgis::HttpMethod::Post:
    {
      QDomDocument postDocument = createPostDocument();
      QDomElement getFeatureElement = createRootPostElement( caps, WFSVersion, postDocument, QStringLiteral( "wfs:GetFeature" ), { typeName } );

      const bool useVersion2 = !WFSVersion.startsWith( QLatin1String( "1." ) );

      QDomElement queryElement = postDocument.createElement( QStringLiteral( "wfs:Query" ) );
      if ( useVersion2 )
      {
        queryElement.setAttribute( QStringLiteral( "typeNames" ), typeName );
      }
      else
      {
        queryElement.setAttribute( QStringLiteral( "typeName" ), typeName );
      }

      if ( !filter.isEmpty() )
      {
        QDomDocument filterDoc;
        QString cleanedFilter = filter;
        cleanedFilter = cleanedFilter.replace( QLatin1String( "<fes:Filter xmlns:fes=\"http://www.opengis.net/fes/2.0\">" ), QLatin1String( "<fes:Filter>" ) );
        if ( filterDoc.setContent( cleanedFilter ) )
        {
          queryElement.appendChild( filterDoc.documentElement() );
        }
      }
      getFeatureElement.appendChild( queryElement );

      if ( hitsOnly )
      {
        getFeatureElement.setAttribute( QStringLiteral( "resultType" ), QStringLiteral( "hits" ) );
      }

      return sendPOST( url, QStringLiteral( "application/xml; charset=utf-8" ), postDocument.toByteArray(), synchronous, { QNetworkReply::RawHeaderPair { "Accept", "application/xml" } } );
    }

    case Qgis::HttpMethod::Head:
    case Qgis::HttpMethod::Put:
    case Qgis::HttpMethod::Delete:
      // not supported, impossible to hit
      break;
  }
  return false;
}

QString QgsWFSGetFeature::errorMessageWithReason( const QString &reason )
{
  return tr( "Download of feature type failed: %1" ).arg( reason );
}
