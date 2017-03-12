/***************************************************************************
                              qgswmsgetstyles.h
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
#include "qgswmsgetstyles.h"

namespace QgsWms
{

  void writeGetStyles( QgsServerInterface *serverIface, const QString &version,
                       const QgsServerRequest &request, QgsServerResponse &response )
  {
    QDomDocument doc = getStyles( serverIface, version, request );
    response.setHeader( QStringLiteral( "Content-Type" ), QStringLiteral( "text/xml; charset=utf-8" ) );
    response.write( doc.toByteArray() );
  }

  QDomDocument getStyles( QgsServerInterface *serverIface, const QString &version,
                          const QgsServerRequest &request )
  {
    Q_UNUSED( version );

    QgsWmsConfigParser *configParser = getConfigParser( serverIface );
    QgsServerRequest::Parameters parameters = request.parameters();

    QDomDocument doc;

    QString layersName = parameters.value( "LAYERS" );

    if ( layersName.isEmpty() )
    {
      throw QgsBadRequestException( QStringLiteral( "LayerNotSpecified" ),
                                    QStringLiteral( "Layers is mandatory for GetStyles operation" ) );
    }

    QStringList layersList = layersName.split( QStringLiteral( "," ), QString::SkipEmptyParts );
    if ( layersList.size() < 1 )
    {
      throw QgsBadRequestException( QStringLiteral( "LayerNotSpecified" ),
                                    QStringLiteral( "Layers is mandatory for GetStyles operation" ) );
    }

    return configParser->getStyles( layersList );
  }


} // samespace QgsWms




