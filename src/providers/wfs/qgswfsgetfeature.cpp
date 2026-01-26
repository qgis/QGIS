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
#include "qgswfsconstants.h"

#include <QUrlQuery>

#include "moc_qgswfsgetfeature.cpp"

QgsWFSGetFeature::QgsWFSGetFeature( QgsWFSDataSourceURI &uri )
  : QgsWfsRequest( uri )
{
}

bool QgsWFSGetFeature::request( bool synchronous, const QString &WFSVersion, const QString &typeName, const QString &filter, bool hitsOnly, const QgsWfsCapabilities &caps )
{
  QUrl url( mUri.requestUrl( u"GetFeature"_s, mUri.httpMethod() ) );

  switch ( mUri.httpMethod() )
  {
    case Qgis::HttpMethod::Get:
    {
      QUrlQuery query( url );
      query.addQueryItem( u"VERSION"_s, WFSVersion );

      const QString namespaceValue( caps.getNamespaceParameterValue( WFSVersion, typeName ) );

      if ( WFSVersion.startsWith( "2.0"_L1 ) )
      {
        query.addQueryItem( u"TYPENAMES"_s, typeName );
        if ( !namespaceValue.isEmpty() )
        {
          query.addQueryItem( u"NAMESPACES"_s, namespaceValue );
        }
      }
      else
      {
        query.addQueryItem( u"TYPENAME"_s, typeName );
        if ( !namespaceValue.isEmpty() )
        {
          query.addQueryItem( u"NAMESPACE"_s, namespaceValue );
        }
      }

      if ( !filter.isEmpty() )
      {
        query.addQueryItem( u"FILTER"_s, filter );
      }

      if ( hitsOnly )
      {
        query.addQueryItem( u"RESULTTYPE"_s, "hits" );
      }
      url.setQuery( query );
      return sendGET( url, QString(), synchronous, /*forceRefresh=*/true, /* cache=*/false );
    }
    case Qgis::HttpMethod::Post:
    {
      QDomDocument postDocument = createPostDocument();
      QDomElement getFeatureElement = createRootPostElement( caps, WFSVersion, postDocument, u"wfs:GetFeature"_s, { typeName } );

      const bool useVersion2 = !WFSVersion.startsWith( "1."_L1 );

      QDomElement queryElement = postDocument.createElement( u"wfs:Query"_s );
      if ( useVersion2 )
      {
        queryElement.setAttribute( u"typeNames"_s, typeName );
      }
      else
      {
        queryElement.setAttribute( u"typeName"_s, typeName );
      }

      if ( !filter.isEmpty() )
      {
        QDomDocument filterDoc;
        QString cleanedFilter = filter;
        cleanedFilter = cleanedFilter.replace( "<fes:Filter xmlns:fes=\"http://www.opengis.net/fes/2.0\">"_L1, "<fes:Filter>"_L1 );
        if ( filterDoc.setContent( cleanedFilter ) )
        {
          queryElement.appendChild( filterDoc.documentElement() );
        }
      }
      getFeatureElement.appendChild( queryElement );

      if ( hitsOnly )
      {
        getFeatureElement.setAttribute( u"resultType"_s, u"hits"_s );
      }

      return sendPOST( url, u"application/xml; charset=utf-8"_s, postDocument.toByteArray(), synchronous, { QNetworkReply::RawHeaderPair { "Accept", "application/xml" } } );
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
