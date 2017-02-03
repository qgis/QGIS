/***************************************************************************
                              qgswcsdescribecoverage.cpp
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
#include "qgswcsdescribecoverage.h"

namespace QgsWcs
{

  /**
   * Output WCS DescribeCoverage response
   */
  void writeDescribeCoverage( QgsServerInterface* serverIface, const QString& version,
                              const QgsServerRequest& request, QgsServerResponse& response )
  {
    QDomDocument doc = createDescribeCoverageDocument( serverIface, version, request );

    response.setHeader( "Content-Type", "text/xml; charset=utf-8" );
    response.write( doc.toByteArray() );
  }


  QDomDocument createDescribeCoverageDocument( QgsServerInterface* serverIface, const QString& version,
      const QgsServerRequest& request )
  {
    Q_UNUSED( version );

    QDomDocument doc;

    QgsServerRequest::Parameters parameters = request.parameters();

    QgsWCSProjectParser* configParser = getConfigParser( serverIface );

    //wcs:WCS_Capabilities element
    QDomElement coveDescElement = doc.createElement( QStringLiteral( "CoverageDescription" )/*wcs:CoverageDescription*/ );
    coveDescElement.setAttribute( QStringLiteral( "xmlns" ), WCS_NAMESPACE );
    coveDescElement.setAttribute( QStringLiteral( "xmlns:xsi" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema-instance" ) );
    coveDescElement.setAttribute( QStringLiteral( "xsi:schemaLocation" ), WCS_NAMESPACE + " http://schemas.opengis.net/wcs/1.0.0/describeCoverage.xsd" );
    coveDescElement.setAttribute( QStringLiteral( "xmlns:gml" ), GML_NAMESPACE );
    coveDescElement.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
    coveDescElement.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.0.0" ) );
    coveDescElement.setAttribute( QStringLiteral( "updateSequence" ), QStringLiteral( "0" ) );
    doc.appendChild( coveDescElement );

    //defining coverage name
    QString coveName;
    //read COVERAGE
    QMap<QString, QString>::const_iterator cove_name_it = parameters.constFind( QStringLiteral( "COVERAGE" ) );
    if ( cove_name_it != parameters.constEnd() )
    {
      coveName = cove_name_it.value();
    }
    if ( coveName.isEmpty() )
    {
      QMap<QString, QString>::const_iterator cove_name_it = parameters.constFind( QStringLiteral( "IDENTIFIER" ) );
      if ( cove_name_it != parameters.constEnd() )
      {
        coveName = cove_name_it.value();
      }
    }
    configParser->describeCoverage( coveName, coveDescElement, doc );
    return doc;
  }

} // namespace QgsWcs



