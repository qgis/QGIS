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

QgsWFSDescribeFeatureType::QgsWFSDescribeFeatureType( QgsWFSDataSourceURI &uri )
  : QgsWfsRequest( uri )
{
}

bool QgsWFSDescribeFeatureType::requestFeatureType( const QString &WFSVersion,
    const QString &typeName, bool bUsePlural )
{
  QUrl url( mUri.requestUrl( QStringLiteral( "DescribeFeatureType" ) ) );
  url.addQueryItem( QStringLiteral( "VERSION" ), WFSVersion );
  url.addQueryItem( bUsePlural ?
                    QStringLiteral( "TYPENAMES" ) : QStringLiteral( "TYPENAME" ), typeName );

  return sendGET( url, true, false );
}

QString QgsWFSDescribeFeatureType::errorMessageWithReason( const QString &reason )
{
  return tr( "Download of feature type failed: %1" ).arg( reason );
}
