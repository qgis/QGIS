/***************************************************************************
                              qgswmsgetstyle.h
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
#include "qgswmsgetstyle.h"

namespace QgsWms
{
  //GetStyle for compatibility with earlier QGIS versions
  void writeGetStyle( QgsServerInterface* serverIface, const QString& version,
                      const QgsServerRequest& request, QgsServerResponse& response )
  {
    QDomDocument doc = getStyle( serverIface, version, request );
    response.setHeader( QStringLiteral( "Content-Type" ), QStringLiteral( "text/xml; charset=utf-8" ) );
    response.write( doc.toByteArray() );
  }

  QDomDocument getStyle( QgsServerInterface* serverIface, const QString& version,
                         const QgsServerRequest& request )
  {
    Q_UNUSED( version );

    QgsWmsConfigParser* configParser = getConfigParser( serverIface );
    QgsServerRequest::Parameters parameters = request.parameters();

    QDomDocument doc;

    QString styleName = parameters.value( QStringLiteral( "STYLE" ) );
    QString layerName = parameters.value( QStringLiteral( "LAYER" ) );

    if ( styleName.isEmpty() )
    {
      throw QgsServiceException( QStringLiteral( "StyleNotSpecified" ),
                                 QStringLiteral( "Style is mandatory for GetStyle operation" ), 400 );
    }

    if ( layerName.isEmpty() )
    {
      throw QgsServiceException( QStringLiteral( "LayerNotSpecified" ),
                                 QStringLiteral( "Layer is mandatory for GetStyle operation" ), 400 );
    }

    return configParser->getStyle( styleName, layerName );
  }


} // samespace QgsWms




