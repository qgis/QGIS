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

#include "qgslogger.h"
#include "qgswfsconstants.h"

#include "moc_qgswfsrequest.cpp"

QgsWfsRequest::QgsWfsRequest( const QgsWFSDataSourceURI &uri )
  : QgsBaseNetworkRequest( uri.auth(), tr( "WFS" ) )
  , mUri( uri )
{
  QgsDebugMsgLevel( u"theUri = "_s + uri.uri(), 4 );
}

QUrl QgsWfsRequest::requestUrl( const QString &request ) const
{
  return mUri.requestUrl( request );
}

QDomDocument QgsWfsRequest::createPostDocument() const
{
  QDomDocument postDocument;
  postDocument.appendChild( postDocument.createProcessingInstruction( u"xml"_s, u"version=\"1.0\" encoding=\"UTF-8\""_s ) );
  return postDocument;
}

QDomElement QgsWfsRequest::createRootPostElement( const QgsWfsCapabilities &capabilities, const QString &wfsVersion, QDomDocument &postDocument, const QString &name, const QStringList &typeNamesForNamespaces ) const
{
  QDomElement rootElement = postDocument.createElement( name );
  rootElement.setAttribute( u"service"_s, u"WFS"_s );
  rootElement.setAttribute( u"version"_s, wfsVersion );

  if ( wfsVersion.startsWith( "1.0"_L1 ) )
  {
    rootElement.setAttribute( u"xmlns:wfs"_s, QgsWFSConstants::WFS_NAMESPACE );
    rootElement.setAttribute( u"xmlns:ogc"_s, QgsWFSConstants::OGC_NAMESPACE );
    rootElement.setAttribute( u"xmlns:gml"_s, QgsWFSConstants::GML_NAMESPACE );
    rootElement.setAttribute( u"xmlns:xsi"_s, u"http://www.w3.org/2001/XMLSchema-instance"_s );
    rootElement.setAttribute( u"xsi:schemaLocation"_s, u"http://www.opengis.net/wfs http://schemas.opengis.net/wfs/1.0.0/wfs.xsd"_s );
  }
  else if ( wfsVersion.startsWith( "1.1"_L1 ) )
  {
    rootElement.setAttribute( u"xmlns:wfs"_s, QgsWFSConstants::WFS_NAMESPACE );
    rootElement.setAttribute( u"xmlns:ogc"_s, QgsWFSConstants::OGC_NAMESPACE );
    rootElement.setAttribute( u"xmlns:gml"_s, QgsWFSConstants::GML_NAMESPACE );
    rootElement.setAttribute( u"xmlns:xsi"_s, u"http://www.w3.org/2001/XMLSchema-instance"_s );
    rootElement.setAttribute( u"xsi:schemaLocation"_s, u"http://www.opengis.net/wfs http://schemas.opengis.net/wfs/1.1.0/wfs.xsd"_s );
  }
  else // 2.0
  {
    rootElement.setAttribute( u"xmlns:wfs"_s, u"http://www.opengis.net/wfs/2.0"_s );
    rootElement.setAttribute( u"xmlns:fes"_s, u"http://www.opengis.net/fes/2.0"_s );
    rootElement.setAttribute( u"xmlns:gml"_s, u"http://www.opengis.net/gml/3.2"_s );
    rootElement.setAttribute( u"xmlns:xsi"_s, u"http://www.w3.org/2001/XMLSchema-instance"_s );
    rootElement.setAttribute( u"xsi:schemaLocation"_s, u"http://www.opengis.net/wfs/2.0 http://schemas.opengis.net/wfs/2.0/wfs.xsd"_s );
  }

  for ( const QString &typeName : typeNamesForNamespaces )
  {
    const QString lNamespace = capabilities.getNamespaceForTypename( typeName );
    if ( !lNamespace.isEmpty() )
    {
      const QStringList typeNameParts = typeName.split( ':' );
      rootElement.setAttribute( u"xmlns:%1"_s.arg( typeNameParts.at( 0 ) ), lNamespace );
    }
  }

  postDocument.appendChild( rootElement );
  return rootElement;
}
