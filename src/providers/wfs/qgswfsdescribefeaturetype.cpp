/***************************************************************************
    qgswfsdescribefeaturetype.cpp
    ---------------------
    begin                : February 2016
    copyright            : (C) 2016 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswfsdescribefeaturetype.h"
#include "qgsmessagelog.h"

QgsWFSDescribeFeatureType::QgsWFSDescribeFeatureType( QgsWFSDataSourceURI &uri )
  : QgsWfsRequest( uri )
{
}

bool QgsWFSDescribeFeatureType::requestFeatureType( const QString &WFSVersion,
    const QString &typeName, const QString &namespaces, bool bUsePlural )
{
  QUrl url( mUri.requestUrl( QStringLiteral( "DescribeFeatureType" ) ) );
  url.addQueryItem( QStringLiteral( "VERSION" ), WFSVersion );

  bool tryNameSpacing;
  QString prefixOfTypename;
  if ( ( tryNameSpacing = ( !namespaces.isEmpty() && typeName.contains( ':' ) ) ) )
    prefixOfTypename = typeName.section( ':', 0, 0 );

  bool success;
  if ( bUsePlural )
  {
    url.addQueryItem( QStringLiteral( "TYPENAMES" ),  typeName );
    if ( tryNameSpacing )
    {
      QString namespacesQueryValue = "xmlns(" + prefixOfTypename + "," + namespaces + ")";
      url.addQueryItem( QStringLiteral( "NAMESPACES" ), namespacesQueryValue );
    }
    success = sendGET( url, true, false );
  }
  else
  {
    url.addQueryItem( QStringLiteral( "TYPENAME" ),  typeName );
    if ( tryNameSpacing )
    {
      QString namespacesQueryValue = "xmlns(" + prefixOfTypename + "=" + namespaces + ")";
      url.addQueryItem( QStringLiteral( "NAMESPACE" ), namespacesQueryValue );
    }
    success = sendGET( url, true, false );
  }
  return success;
}

bool QgsWFSDescribeFeatureType::requestFeatureType( const QString &WFSVersion,
    const QString &typeName, const QString &namespaces )
{
  bool success = requestFeatureType( WFSVersion, typeName, namespaces, false );
  if ( !success )
  {
    if ( WFSVersion.startsWith( QLatin1String( "2.0" ) ) )
    {
      success = requestFeatureType( WFSVersion, typeName, namespaces, true );
    }
  }
  else
  {
    if ( response().indexOf( "ExceptionReport" ) >= 0 )
    {
      success = requestFeatureType( WFSVersion, typeName, namespaces, true );
    }
  }
  return success;
}

QString QgsWFSDescribeFeatureType::errorMessageWithReason( const QString &reason )
{
  return tr( "Download of feature type failed: %1" ).arg( reason );
}
