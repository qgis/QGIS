/***************************************************************************
    qgswfsrequest.cpp
    ---------------------
    begin                : February 2016
    copyright            : (C) 2011 by Martin Dobias
                           (C) 2016 by Even Rouault
    email                : wonder dot sk at gmail dot com
                           even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswfsrequest.h"
#include "qgswfsconstants.h"
#include "moc_qgswfsrequest.cpp"

#include "qgslogger.h"

QgsWfsRequest::QgsWfsRequest( const QgsWFSDataSourceURI &uri )
  : QgsBaseNetworkRequest( uri.auth(), tr( "WFS" ) )
  , mUri( uri )
{
  QgsDebugMsgLevel( QStringLiteral( "theUri = " ) + uri.uri(), 4 );
}

QUrl QgsWfsRequest::requestUrl( const QString &request ) const
{
  return mUri.requestUrl( request );
}

QDomDocument QgsWfsRequest::createPostDocument() const
{
  QDomDocument postDocument;
  postDocument.appendChild( postDocument.createProcessingInstruction( QStringLiteral( "xml" ), QStringLiteral( "version=\"1.0\" encoding=\"UTF-8\"" ) ) );
  return postDocument;
}

QDomElement QgsWfsRequest::createRootPostElement( const QgsWfsCapabilities &capabilities, const QString &wfsVersion, QDomDocument &postDocument, const QString &name, const QStringList &typeNamesForNamespaces ) const
{
  QDomElement rootElement = postDocument.createElement( name );
  rootElement.setAttribute( QStringLiteral( "service" ), QStringLiteral( "WFS" ) );
  rootElement.setAttribute( QStringLiteral( "version" ), wfsVersion );

  if ( wfsVersion.startsWith( QLatin1String( "1.0" ) ) )
  {
    rootElement.setAttribute( QStringLiteral( "xmlns:wfs" ), QgsWFSConstants::WFS_NAMESPACE );
    rootElement.setAttribute( QStringLiteral( "xmlns:ogc" ), QgsWFSConstants::OGC_NAMESPACE );
    rootElement.setAttribute( QStringLiteral( "xmlns:gml" ), QgsWFSConstants::GML_NAMESPACE );
    rootElement.setAttribute( QStringLiteral( "xmlns:xsi" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema-instance" ) );
    rootElement.setAttribute( QStringLiteral( "xsi:schemaLocation" ), QStringLiteral( "http://www.opengis.net/wfs http://schemas.opengis.net/wfs/1.0.0/wfs.xsd" ) );
  }
  else if ( wfsVersion.startsWith( QLatin1String( "1.1" ) ) )
  {
    rootElement.setAttribute( QStringLiteral( "xmlns:wfs" ), QgsWFSConstants::WFS_NAMESPACE );
    rootElement.setAttribute( QStringLiteral( "xmlns:ogc" ), QgsWFSConstants::OGC_NAMESPACE );
    rootElement.setAttribute( QStringLiteral( "xmlns:gml" ), QgsWFSConstants::GML_NAMESPACE );
    rootElement.setAttribute( QStringLiteral( "xmlns:xsi" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema-instance" ) );
    rootElement.setAttribute( QStringLiteral( "xsi:schemaLocation" ), QStringLiteral( "http://www.opengis.net/wfs http://schemas.opengis.net/wfs/1.1.0/wfs.xsd" ) );
  }
  else // 2.0
  {
    rootElement.setAttribute( QStringLiteral( "xmlns:wfs" ), QStringLiteral( "http://www.opengis.net/wfs/2.0" ) );
    rootElement.setAttribute( QStringLiteral( "xmlns:fes" ), QStringLiteral( "http://www.opengis.net/fes/2.0" ) );
    rootElement.setAttribute( QStringLiteral( "xmlns:gml" ), QStringLiteral( "http://www.opengis.net/gml/3.2" ) );
    rootElement.setAttribute( QStringLiteral( "xmlns:xsi" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema-instance" ) );
    rootElement.setAttribute( QStringLiteral( "xsi:schemaLocation" ), QStringLiteral( "http://www.opengis.net/wfs/2.0 http://schemas.opengis.net/wfs/2.0/wfs.xsd" ) );
  }

  for ( const QString &typeName : typeNamesForNamespaces )
  {
    const QString lNamespace = capabilities.getNamespaceForTypename( typeName );
    if ( !lNamespace.isEmpty() )
    {
      const QStringList typeNameParts = typeName.split( ':' );
      rootElement.setAttribute( QStringLiteral( "xmlns:%1" ).arg( typeNameParts.at( 0 ) ), lNamespace );
    }
  }

  postDocument.appendChild( rootElement );
  return rootElement;
}
