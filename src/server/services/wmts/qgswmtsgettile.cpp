/***************************************************************************
                              qgswmsgetmap.cpp
                            -------------------------
  begin                : July 23 , 2017
  copyright            : (C) 2018 by René-Luc D'Hont
  email                : rldhont at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgswmtsutils.h"
#include "qgswmtsgettile.h"

#include <QImage>

namespace QgsWmts
{

  void writeGetTile( QgsServerInterface *serverIface, const QgsProject *project,
                     const QString &version, const QgsServerRequest &request,
                     QgsServerResponse &response )
  {
    Q_UNUSED( version );

    QgsServerRequest::Parameters params = request.parameters();

    // WMS query
    QUrlQuery query = translateWmtsParamToWmsQueryItem( QStringLiteral( "GetMap" ), params, project, serverIface );

    QgsServerParameters wmsParams( query );
    QgsServerRequest wmsRequest( "?" + query.query( QUrl::FullyDecoded ) );
    QgsService *service = serverIface->serviceRegistry()->getService( wmsParams.service(), wmsParams.version() );
    service->executeRequest( wmsRequest, response, project );
  }

} // namespace QgsWmts




