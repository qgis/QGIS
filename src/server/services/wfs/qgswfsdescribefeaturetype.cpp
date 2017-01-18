/***************************************************************************
                              qgswfsdescribefeaturetype.cpp
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
#include "qgswfsdescribefeaturetype.h"

namespace QgsWfs
{

  void writeDescribeFeatureType( QgsServerInterface* serverIface, const QString& version,
                                 const QgsServerRequest& request, QgsServerResponse& response )
  {
    QDomDocument doc = createDescribeFeatureTypeDocument( serverIface, version, request );

    response.setHeader( "Content-Type", "text/xml; charset=utf-8" );
    response.write( doc.toByteArray() );
  }


  QDomDocument createDescribeFeatureTypeDocument( QgsServerInterface* serverIface, const QString& version,
      const QgsServerRequest& request )
  {
    Q_UNUSED( version );

    QDomDocument doc;

    QgsWfsProjectParser* configParser = getConfigParser( serverIface );
    QgsServerRequest::Parameters parameters = request.parameters();

    //xsd:schema
    QDomElement schemaElement = doc.createElement( QStringLiteral( "schema" )/*xsd:schema*/ );
    schemaElement.setAttribute( QStringLiteral( "xmlns" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema" ) );
    schemaElement.setAttribute( QStringLiteral( "xmlns:xsd" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema" ) );
    schemaElement.setAttribute( QStringLiteral( "xmlns:ogc" ), OGC_NAMESPACE );
    schemaElement.setAttribute( QStringLiteral( "xmlns:gml" ), GML_NAMESPACE );
    schemaElement.setAttribute( QStringLiteral( "xmlns:qgs" ), QGS_NAMESPACE );
    schemaElement.setAttribute( QStringLiteral( "targetNamespace" ), QGS_NAMESPACE );
    schemaElement.setAttribute( QStringLiteral( "elementFormDefault" ), QStringLiteral( "qualified" ) );
    schemaElement.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.0" ) );
    doc.appendChild( schemaElement );

    //xsd:import
    QDomElement importElement = doc.createElement( QStringLiteral( "import" )/*xsd:import*/ );
    importElement.setAttribute( QStringLiteral( "namespace" ),  GML_NAMESPACE );
    importElement.setAttribute( QStringLiteral( "schemaLocation" ), QStringLiteral( "http://schemas.opengis.net/gml/2.1.2/feature.xsd" ) );
    schemaElement.appendChild( importElement );

    //defining typename
    QString typeName = QLatin1String( "" );

    QDomDocument queryDoc;
    QString errorMsg;
    if ( queryDoc.setContent( parameters.value( QStringLiteral( "REQUEST_BODY" ) ), true, &errorMsg ) )
    {
      //read doc
      QDomElement queryDocElem = queryDoc.documentElement();
      QDomNodeList docChildNodes = queryDocElem.childNodes();
      if ( docChildNodes.size() )
      {
        for ( int i = 0; i < docChildNodes.size(); i++ )
        {
          QDomElement docChildElem = docChildNodes.at( i ).toElement();
          if ( docChildElem.tagName() == QLatin1String( "TypeName" ) )
          {
            if ( typeName == QLatin1String( "" ) )
              typeName = docChildElem.text();
            else
              typeName += "," + docChildElem.text();
          }
        }
      }
    }
    else
    {
      typeName = request.getParameter( QStringLiteral( "TYPENAME" ) );
    }

    configParser->describeFeatureType( typeName, schemaElement, doc );
    return doc;
  }

} // samespace QgsWfs



