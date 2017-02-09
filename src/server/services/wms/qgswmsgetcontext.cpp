/***************************************************************************
                              qgswmsgetcontext.cpp
                              -------------------------
  begin                : December 20 , 2016
  copyright            : (C) 2007 by Marco Hugentobler  (original code)
                         (C) 2014 by Alessandro Pasotti (original code)
                         (C) 2016 by David Marteau
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
#include "qgswmsutils.h"
#include "qgswmsgetcontext.h"

namespace QgsWms
{

  void writeGetContext( QgsServerInterface* serverIface, const QgsProject* project,
                        const QString& version, const QgsServerRequest& request,
                        QgsServerResponse& response )
  {
    QDomDocument doc = getContext( serverIface, project, version, request );

    response.setHeader( QStringLiteral( "Content-Type" ), QStringLiteral( "text/xml; charset=utf-8" ) );
    response.write( doc.toByteArray() );
  }


  QDomDocument getContext( QgsServerInterface* serverIface, const QgsProject* project,
                           const QString& version, const QgsServerRequest& request )
  {
    Q_UNUSED( version );

    QgsWmsConfigParser*  configParser = getConfigParser( serverIface );

    QDomDocument doc;
    QDomProcessingInstruction xmlDeclaration = doc.createProcessingInstruction( QStringLiteral( "xml" ),
        QStringLiteral( "version=\"1.0\" encoding=\"utf-8\"" ) );

    doc.appendChild( xmlDeclaration );

    QDomElement owsContextElem = doc.createElement( QStringLiteral( "OWSContext" ) );
    owsContextElem.setAttribute( QStringLiteral( "xmlns" ), QStringLiteral( "http://www.opengis.net/ows-context" ) );
    owsContextElem.setAttribute( QStringLiteral( "xmlns:ows-context" ), QStringLiteral( "http://www.opengis.net/ows-context" ) );
    owsContextElem.setAttribute( QStringLiteral( "xmlns:context" ), QStringLiteral( "http://www.opengis.net/context" ) );
    owsContextElem.setAttribute( QStringLiteral( "xmlns:ows" ), QStringLiteral( "http://www.opengis.net/ows" ) );
    owsContextElem.setAttribute( QStringLiteral( "xmlns:sld" ), QStringLiteral( "http://www.opengis.net/sld" ) );
    owsContextElem.setAttribute( QStringLiteral( "xmlns:ogc" ), QStringLiteral( "http://www.opengis.net/ogc" ) );
    owsContextElem.setAttribute( QStringLiteral( "xmlns:gml" ), QStringLiteral( "http://www.opengis.net/gml" ) );
    owsContextElem.setAttribute( QStringLiteral( "xmlns:kml" ), QStringLiteral( "http://www.opengis.net/kml/2.2" ) );
    owsContextElem.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
    owsContextElem.setAttribute( QStringLiteral( "xmlns:ns9" ), QStringLiteral( "http://www.w3.org/2005/Atom" ) );
    owsContextElem.setAttribute( QStringLiteral( "xmlns:xal" ), QStringLiteral( "urn:oasis:names:tc:ciq:xsdschema:xAL:2.0" ) );
    owsContextElem.setAttribute( QStringLiteral( "xmlns:ins" ), QStringLiteral( "http://www.inspire.org" ) );
    owsContextElem.setAttribute( QStringLiteral( "version" ), QStringLiteral( "0.3.1" ) );
    doc.appendChild( owsContextElem );

    QString hrefString = serviceUrl( request, project ).toString( QUrl::FullyDecoded );
    configParser->owsGeneralAndResourceList( owsContextElem, doc, hrefString );

    return doc;
  }

} // samespace QgsWms




