/***************************************************************************
                              qgswfsgecapabilities.cpp
                              -------------------------
  begin                : December 20 , 2016
  copyright            : (C) 2007 by Marco Hugentobler  (original code)
                         (C) 2014 by Alessandro Pasotti (original code)
                         (C) 2017 by David Marteau
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
                         a dot pasotti at itopen dot it
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
#include "qgswfsutils.h"
#include "qgswfsgetcapabilities.h"

namespace QgsWfs
{

  /**
   * Output WFS  GetCapabilities response
   */
  void writeGetCapabilities( QgsServerInterface* serverIface, const QgsProject* project, const QString& version,
                             const QgsServerRequest& request, QgsServerResponse& response )
  {
    QDomDocument doc = createGetCapabilitiesDocument( serverIface, project, version, request );

    response.setHeader( "Content-Type", "text/xml; charset=utf-8" );
    response.write( doc.toByteArray() );
  }


  QDomDocument createGetCapabilitiesDocument( QgsServerInterface* serverIface, const QgsProject* project, const QString& version,
      const QgsServerRequest& request )
  {
    Q_UNUSED( version );

    QDomDocument doc;

    QgsWfsProjectParser* configParser = getConfigParser( serverIface );

    //wfs:WFS_Capabilities element
    QDomElement wfsCapabilitiesElement = doc.createElement( QStringLiteral( "WFS_Capabilities" )/*wms:WFS_Capabilities*/ );
    wfsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns" ), WFS_NAMESPACE );
    wfsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:xsi" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema-instance" ) );
    wfsCapabilitiesElement.setAttribute( QStringLiteral( "xsi:schemaLocation" ), WFS_NAMESPACE + " http://schemas.opengis.net/wfs/1.0.0/WFS-capabilities.xsd" );
    wfsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:ogc" ), OGC_NAMESPACE );
    wfsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:gml" ), GML_NAMESPACE );
    wfsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:ows" ), QStringLiteral( "http://www.opengis.net/ows" ) );
    wfsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
    wfsCapabilitiesElement.setAttribute( QStringLiteral( "version" ), implementationVersion() );
    wfsCapabilitiesElement.setAttribute( QStringLiteral( "updateSequence" ), QStringLiteral( "0" ) );
    doc.appendChild( wfsCapabilitiesElement );

    configParser->serviceCapabilities( wfsCapabilitiesElement, doc );

    //wfs:Capability element
    QDomElement capabilityElement = doc.createElement( QStringLiteral( "Capability" )/*wfs:Capability*/ );
    wfsCapabilitiesElement.appendChild( capabilityElement );

    //wfs:Request element
    QDomElement requestElement = doc.createElement( QStringLiteral( "Request" )/*wfs:Request*/ );
    capabilityElement.appendChild( requestElement );
    //wfs:GetCapabilities
    QDomElement getCapabilitiesElement = doc.createElement( QStringLiteral( "GetCapabilities" )/*wfs:GetCapabilities*/ );
    requestElement.appendChild( getCapabilitiesElement );

    QDomElement dcpTypeElement = doc.createElement( QStringLiteral( "DCPType" )/*wfs:DCPType*/ );
    getCapabilitiesElement.appendChild( dcpTypeElement );
    QDomElement httpElement = doc.createElement( QStringLiteral( "HTTP" )/*wfs:HTTP*/ );
    dcpTypeElement.appendChild( httpElement );

    //Prepare url
    QString hrefString = serviceUrl( request, project );

    //only Get supported for the moment
    QDomElement getElement = doc.createElement( QStringLiteral( "Get" )/*wfs:Get*/ );
    httpElement.appendChild( getElement );
    getElement.setAttribute( QStringLiteral( "onlineResource" ), hrefString );
    QDomElement getCapabilitiesDhcTypePostElement = dcpTypeElement.cloneNode().toElement();//this is the same as for 'GetCapabilities'
    getCapabilitiesDhcTypePostElement.firstChild().firstChild().toElement().setTagName( QStringLiteral( "Post" ) );
    getCapabilitiesElement.appendChild( getCapabilitiesDhcTypePostElement );

    //wfs:DescribeFeatureType
    QDomElement describeFeatureTypeElement = doc.createElement( QStringLiteral( "DescribeFeatureType" )/*wfs:DescribeFeatureType*/ );
    requestElement.appendChild( describeFeatureTypeElement );
    QDomElement schemaDescriptionLanguageElement = doc.createElement( QStringLiteral( "SchemaDescriptionLanguage" )/*wfs:SchemaDescriptionLanguage*/ );
    describeFeatureTypeElement.appendChild( schemaDescriptionLanguageElement );
    QDomElement xmlSchemaElement = doc.createElement( QStringLiteral( "XMLSCHEMA" )/*wfs:XMLSCHEMA*/ );
    schemaDescriptionLanguageElement.appendChild( xmlSchemaElement );
    QDomElement describeFeatureTypeDhcTypeElement = dcpTypeElement.cloneNode().toElement();//this is the same as for 'GetCapabilities'
    describeFeatureTypeElement.appendChild( describeFeatureTypeDhcTypeElement );
    QDomElement describeFeatureTypeDhcTypePostElement = dcpTypeElement.cloneNode().toElement();//this is the same as for 'GetCapabilities'
    describeFeatureTypeDhcTypePostElement.firstChild().firstChild().toElement().setTagName( QStringLiteral( "Post" ) );
    describeFeatureTypeElement.appendChild( describeFeatureTypeDhcTypePostElement );

    //wfs:GetFeature
    QDomElement getFeatureElement = doc.createElement( QStringLiteral( "GetFeature" )/*wfs:GetFeature*/ );
    requestElement.appendChild( getFeatureElement );
    QDomElement getFeatureFormatElement = doc.createElement( QStringLiteral( "ResultFormat" ) );/*wfs:ResultFormat*/
    getFeatureElement.appendChild( getFeatureFormatElement );
    QDomElement gmlFormatElement = doc.createElement( QStringLiteral( "GML2" ) );/*wfs:GML2*/
    getFeatureFormatElement.appendChild( gmlFormatElement );
    QDomElement gml3FormatElement = doc.createElement( QStringLiteral( "GML3" ) );/*wfs:GML3*/
    getFeatureFormatElement.appendChild( gml3FormatElement );
    QDomElement geojsonFormatElement = doc.createElement( QStringLiteral( "GeoJSON" ) );/*wfs:GeoJSON*/
    getFeatureFormatElement.appendChild( geojsonFormatElement );
    QDomElement getFeatureDhcTypeGetElement = dcpTypeElement.cloneNode().toElement();//this is the same as for 'GetCapabilities'
    getFeatureElement.appendChild( getFeatureDhcTypeGetElement );
    QDomElement getFeatureDhcTypePostElement = dcpTypeElement.cloneNode().toElement();//this is the same as for 'GetCapabilities'
    getFeatureDhcTypePostElement.firstChild().firstChild().toElement().setTagName( QStringLiteral( "Post" ) );
    getFeatureElement.appendChild( getFeatureDhcTypePostElement );

    //wfs:Transaction
    QDomElement transactionElement = doc.createElement( QStringLiteral( "Transaction" )/*wfs:Transaction*/ );
    requestElement.appendChild( transactionElement );
    QDomElement transactionDhcTypeElement = dcpTypeElement.cloneNode().toElement();//this is the same as for 'GetCapabilities'
    transactionDhcTypeElement.firstChild().firstChild().toElement().setTagName( QStringLiteral( "Post" ) );
    transactionElement.appendChild( transactionDhcTypeElement );

    //wfs:FeatureTypeList element
    QDomElement featureTypeListElement = doc.createElement( QStringLiteral( "FeatureTypeList" )/*wfs:FeatureTypeList*/ );
    wfsCapabilitiesElement.appendChild( featureTypeListElement );
    //wfs:Operations element
    QDomElement operationsElement = doc.createElement( QStringLiteral( "Operations" )/*wfs:Operations*/ );
    featureTypeListElement.appendChild( operationsElement );
    //wfs:Query element
    QDomElement queryElement = doc.createElement( QStringLiteral( "Query" )/*wfs:Query*/ );
    operationsElement.appendChild( queryElement );
    /*
     * Adding layer liste in featureTypeListElement
     */
    configParser->featureTypeList( featureTypeListElement, doc );

    /*
     * Adding ogc:Filter_Capabilities in capabilityElement
     */
    //ogc:Filter_Capabilities element
    QDomElement filterCapabilitiesElement = doc.createElement( QStringLiteral( "ogc:Filter_Capabilities" )/*ogc:Filter_Capabilities*/ );
    wfsCapabilitiesElement.appendChild( filterCapabilitiesElement );
    QDomElement spatialCapabilitiesElement = doc.createElement( QStringLiteral( "ogc:Spatial_Capabilities" )/*ogc:Spatial_Capabilities*/ );
    filterCapabilitiesElement.appendChild( spatialCapabilitiesElement );
    QDomElement spatialOperatorsElement = doc.createElement( QStringLiteral( "ogc:Spatial_Operators" )/*ogc:Spatial_Operators*/ );
    spatialCapabilitiesElement.appendChild( spatialOperatorsElement );
    spatialOperatorsElement.appendChild( doc.createElement( QStringLiteral( "ogc:BBOX" )/*ogc:BBOX*/ ) );
    spatialOperatorsElement.appendChild( doc.createElement( QStringLiteral( "ogc:Disjoint" )/*ogc:Disjoint*/ ) );
    spatialOperatorsElement.appendChild( doc.createElement( QStringLiteral( "ogc:Intersect" )/*ogc:Intersects*/ ) );
    spatialOperatorsElement.appendChild( doc.createElement( QStringLiteral( "ogc:Touches" )/*ogc:Touches*/ ) );
    spatialOperatorsElement.appendChild( doc.createElement( QStringLiteral( "ogc:Crosses" )/*ogc:Crosses*/ ) );
    spatialOperatorsElement.appendChild( doc.createElement( QStringLiteral( "ogc:Contains" )/*ogc:Contains*/ ) );
    spatialOperatorsElement.appendChild( doc.createElement( QStringLiteral( "ogc:Overlaps" )/*ogc:Overlaps*/ ) );
    spatialOperatorsElement.appendChild( doc.createElement( QStringLiteral( "ogc:Within" )/*ogc:Within*/ ) );
    QDomElement scalarCapabilitiesElement = doc.createElement( QStringLiteral( "ogc:Scalar_Capabilities" )/*ogc:Scalar_Capabilities*/ );
    filterCapabilitiesElement.appendChild( scalarCapabilitiesElement );
    QDomElement comparisonOperatorsElement = doc.createElement( QStringLiteral( "ogc:Comparison_Operators" )/*ogc:Comparison_Operators*/ );
    scalarCapabilitiesElement.appendChild( comparisonOperatorsElement );
    comparisonOperatorsElement.appendChild( doc.createElement( QStringLiteral( "ogc:Simple_Comparisons" )/*ogc:Simple_Comparisons*/ ) );
    comparisonOperatorsElement.appendChild( doc.createElement( QStringLiteral( "ogc:Between" )/*ogc:Between*/ ) );
    comparisonOperatorsElement.appendChild( doc.createElement( QStringLiteral( "ogc:Like" )/*ogc:Like*/ ) );

    return doc;

  }

} // samespace QgsWfs



