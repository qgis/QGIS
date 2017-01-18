/***************************************************************************
                              qgswcsgecapabilities.cpp
                              -------------------------
  begin                : January 16 , 2017
  copyright            : (C) 2013 by René-Luc D'Hont  ( parts from qgswcsserver )
                         (C) 2017 by David Marteau
  email                : rldhont at 3liz dot com
                         david dot marteau at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgswcsutils.h"
#include "qgswcsgetcapabilities.h"

namespace QgsWcs
{

  /**
   * Output WCS  GetCapabilities response
   */
  void writeGetCapabilities( QgsServerInterface* serverIface, const QString& version,
                             const QgsServerRequest& request, QgsServerResponse& response )
  {
    QDomDocument doc = createGetCapabilitiesDocument( serverIface, version, request );

    response.setHeader( "Content-Type", "text/xml; charset=utf-8" );
    response.write( doc.toByteArray() );
  }


  QDomDocument createGetCapabilitiesDocument( QgsServerInterface* serverIface, const QString& version,
      const QgsServerRequest& request )
  {
    Q_UNUSED( version );

    QDomDocument doc;

    QgsWCSProjectParser* configParser = getConfigParser( serverIface );

    //wcs:WCS_Capabilities element
    QDomElement wcsCapabilitiesElement = doc.createElement( QStringLiteral( "WCS_Capabilities" )/*wcs:WCS_Capabilities*/ );
    wcsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns" ), WCS_NAMESPACE );
    wcsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:xsi" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema-instance" ) );
    wcsCapabilitiesElement.setAttribute( QStringLiteral( "xsi:schemaLocation" ), WCS_NAMESPACE + " http://schemas.opengis.net/wcs/1.0.0/wcsCapabilities.xsd" );
    wcsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:gml" ), GML_NAMESPACE );
    wcsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
    wcsCapabilitiesElement.setAttribute( QStringLiteral( "version" ), implementationVersion() );
    wcsCapabilitiesElement.setAttribute( QStringLiteral( "updateSequence" ), QStringLiteral( "0" ) );
    doc.appendChild( wcsCapabilitiesElement );

    configParser->serviceCapabilities( wcsCapabilitiesElement, doc );

    //INSERT Service

    //wcs:Capability element
    QDomElement capabilityElement = doc.createElement( QStringLiteral( "Capability" )/*wcs:Capability*/ );
    wcsCapabilitiesElement.appendChild( capabilityElement );

    //wcs:Request element
    QDomElement requestElement = doc.createElement( QStringLiteral( "Request" )/*wcs:Request*/ );
    capabilityElement.appendChild( requestElement );

    //wcs:GetCapabilities
    QDomElement getCapabilitiesElement = doc.createElement( QStringLiteral( "GetCapabilities" )/*wcs:GetCapabilities*/ );
    requestElement.appendChild( getCapabilitiesElement );

    QDomElement dcpTypeElement = doc.createElement( QStringLiteral( "DCPType" )/*wcs:DCPType*/ );
    getCapabilitiesElement.appendChild( dcpTypeElement );
    QDomElement httpElement = doc.createElement( QStringLiteral( "HTTP" )/*wcs:HTTP*/ );
    dcpTypeElement.appendChild( httpElement );

    //Prepare url
    QString hrefString = serviceUrl( request, configParser );

    QDomElement getElement = doc.createElement( QStringLiteral( "Get" )/*wcs:Get*/ );
    httpElement.appendChild( getElement );
    QDomElement onlineResourceElement = doc.createElement( QStringLiteral( "OnlineResource" )/*wcs:OnlineResource*/ );
    onlineResourceElement.setAttribute( QStringLiteral( "xlink:type" ), QStringLiteral( "simple" ) );
    onlineResourceElement.setAttribute( QStringLiteral( "xlink:href" ), hrefString );
    getElement.appendChild( onlineResourceElement );

    QDomElement getCapabilitiesDhcTypePostElement = dcpTypeElement.cloneNode().toElement();//this is the same as for 'GetCapabilities'
    getCapabilitiesDhcTypePostElement.firstChild().firstChild().toElement().setTagName( QStringLiteral( "Post" ) );
    getCapabilitiesElement.appendChild( getCapabilitiesDhcTypePostElement );

    QDomElement describeCoverageElement = getCapabilitiesElement.cloneNode().toElement();//this is the same as 'GetCapabilities'
    describeCoverageElement.setTagName( QStringLiteral( "DescribeCoverage" ) );
    requestElement.appendChild( describeCoverageElement );

    QDomElement getCoverageElement = getCapabilitiesElement.cloneNode().toElement();//this is the same as 'GetCapabilities'
    getCoverageElement.setTagName( QStringLiteral( "GetCoverage" ) );
    requestElement.appendChild( getCoverageElement );

    /*
     * Adding layer list in ContentMetadata
     */
    QDomElement contentMetadataElement = doc.createElement( QStringLiteral( "ContentMetadata" )/*wcs:ContentMetadata*/ );
    wcsCapabilitiesElement.appendChild( contentMetadataElement );
    /*
     * Adding layer list in contentMetadataElement
     */
    if ( configParser )
    {
      configParser->wcsContentMetadata( contentMetadataElement, doc );
    }

    return doc;

  }

} // namespace QgsWcs



