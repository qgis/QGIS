/***************************************************************************
                              qgswmsdescribelayer.cpp
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
#include "qgswmsdescribelayer.h"

namespace QgsWms
{

  void writeDescribeLayer( QgsServerInterface* serverIface, const QString& version,
                           const QgsServerRequest& request, QgsServerResponse& response )
  {
    QDomDocument doc = describeLayer( serverIface, version, request );
    response.setHeader( QStringLiteral( "Content-Type" ), QStringLiteral( "text/xml; charset=utf-8" ) );
    response.write( doc.toByteArray() );
  }

  // DescribeLayer is defined for WMS1.1.1/SLD1.0 and in WMS 1.3.0 SLD Extension
  QDomDocument describeLayer( QgsServerInterface* serverIface, const QString& version,
                              const QgsServerRequest& request )
  {
    Q_UNUSED( version );

    QgsServerRequest::Parameters parameters = request.parameters();
    QgsWmsConfigParser* configParser = getConfigParser( serverIface );

    if ( !parameters.contains( QStringLiteral( "SLD_VERSION" ) ) )
    {
      throw QgsServiceException( QStringLiteral( "MissingParameterValue" ),
                                 QStringLiteral( "SLD_VERSION is mandatory for DescribeLayer operation" ), 400 );
    }
    if ( parameters[ QStringLiteral( "SLD_VERSION" )] != QLatin1String( "1.1.0" ) )
    {
      throw QgsServiceException( QStringLiteral( "InvalidParameterValue" ),
                                 QStringLiteral( "SLD_VERSION = %1 is not supported" ).arg( parameters[ QStringLiteral( "SLD_VERSION" )] ), 400 );
    }

    if ( !parameters.contains( QStringLiteral( "LAYERS" ) ) )
    {
      throw QgsServiceException( QStringLiteral( "MissingParameterValue" ),
                                 QStringLiteral( "LAYERS is mandatory for DescribeLayer operation" ), 400 );
    }

    QStringList layersList = parameters[ QStringLiteral( "LAYERS" )].split( QStringLiteral( "," ), QString::SkipEmptyParts );
    if ( layersList.size() < 1 )
    {
      throw QgsServiceException( QStringLiteral( "InvalidParameterValue" ), QStringLiteral( "Layers is empty" ), 400 );
    }

    QString hrefString = serviceUrl( request, configParser ).toString( QUrl::FullyDecoded );
    return configParser->describeLayer( layersList, hrefString );
  }
} // samespace QgsWms




