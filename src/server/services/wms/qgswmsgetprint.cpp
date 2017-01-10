/***************************************************************************
                              qgswmsgetprint.h
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
#include "qgswmsgetprint.h"
#include "qgswmsservertransitional.h"

namespace QgsWms
{
  void writeGetPrint( QgsServerInterface* serverIface, const QString& version,
                      const QgsServerRequest& request, QgsServerResponse& response )
  {
    QgsServerRequest::Parameters params = request.parameters();
    try
    {
      Q_UNUSED( version );
      QgsWmsServer server( serverIface->configFilePath(),
                           *serverIface->serverSettings(), params,
                           getConfigParser( serverIface ),
                           serverIface->accessControls() );

      QString format = params.value( "FORMAT" );
      QString contentType;

      // GetPrint supports svg/png/pdf
      if ( format.compare( QStringLiteral( "image/png" ), Qt::CaseInsensitive ) == 0 ||
           format.compare( QStringLiteral( "png" ), Qt::CaseInsensitive ) == 0 )
      {
        format   = "png";
        contentType = "image/png";
      }
      else if ( format.compare( QStringLiteral( "image/svg" ), Qt::CaseInsensitive ) == 0 ||
                format.compare( QStringLiteral( "image/svg+xml" ), Qt::CaseInsensitive ) == 0 ||
                format.compare( QStringLiteral( "svg" ), Qt::CaseInsensitive ) == 0 )
      {
        format   = "svg";
        contentType = "image/svg+xml";
      }
      else if ( format.compare( QStringLiteral( "application/pdf" ), Qt::CaseInsensitive ) == 0 ||
                format.compare( QStringLiteral( "pdf" ), Qt::CaseInsensitive ) == 0 )
      {
        format = "pdf";
        contentType = "application/pdf";
      }
      else
      {
        writeError( response, QStringLiteral( "InvalidFormat" ),
                    QString( "Output format %1 is not supported by the GetPrint request" ).arg( format ) );
      }

      QScopedPointer<QByteArray> result( server.getPrint( format ) );
      response.setHeader( QStringLiteral( "Content-Type" ), contentType );
      response.write( *result );
    }
    catch ( QgsMapServiceException& ex )
    {
      writeError( response, ex.code(), ex.message() );
    }
  }

} // samespace QgsWms




