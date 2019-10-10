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
    const QString &typeName, const QgsWfsCapabilities::Capabilities &caps )
{
  QUrl url( mUri.requestUrl( QStringLiteral( "DescribeFeatureType" ) ) );
  url.addQueryItem( QStringLiteral( "VERSION" ), WFSVersion );

  QString namespaceValue( caps.getNamespaceParameterValue( WFSVersion, typeName ) );

  if ( WFSVersion.startsWith( QLatin1String( "2.0" ) ) )
  {
    url.addQueryItem( QStringLiteral( "TYPENAMES" ), typeName );
    if ( !namespaceValue.isEmpty() )
    {
      url.addQueryItem( QStringLiteral( "NAMESPACES" ), namespaceValue );
    }
  }

  url.addQueryItem( QStringLiteral( "TYPENAME" ), typeName );
  if ( !namespaceValue.isEmpty() )
  {
    url.addQueryItem( QStringLiteral( "NAMESPACE" ), namespaceValue );
  }

  return sendGET( url, QString(), true, false );
}

QString QgsWFSDescribeFeatureType::errorMessageWithReason( const QString &reason )
{
  return tr( "Download of feature type failed: %1" ).arg( reason );
}
