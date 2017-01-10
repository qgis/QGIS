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
#include "qgswmsservertransitional.h"

namespace QgsWms
{

  void writeGetContext( QgsServerInterface* serverIface, const QString& version,
                        const QgsServerRequest& request, QgsServerResponse& response )
  {
    QgsServerRequest::Parameters params = request.parameters();
    try
    {
      Q_UNUSED( version );
      QgsWmsServer server( serverIface->configFilePath(),
                           *serverIface->serverSettings(),
                           params,
                           getConfigParser( serverIface ),
                           serverIface->accessControls() );
      QDomDocument doc = server.getContext();
      response.setHeader( QStringLiteral( "Content-Type" ), QStringLiteral( "text/xml; charset=utf-8" ) );
      response.write( doc.toByteArray() );
    }
    catch ( QgsMapServiceException& ex )
    {
      writeError( response, ex.code(), ex.message() );
    }
  }


} // samespace QgsWms




