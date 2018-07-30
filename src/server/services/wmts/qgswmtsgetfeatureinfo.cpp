/***************************************************************************
                            qgswmsgetfeatureinfo.cpp
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
#include "qgswmtsgetfeatureinfo.h"

#include <QImage>

namespace QgsWmts
{

  void writeGetFeatureInfo( QgsServerInterface *serverIface, const QgsProject *project,
                            const QString &version, const QgsServerRequest &request,
                            QgsServerResponse &response )
  {
    Q_UNUSED( version );

    QgsServerRequest::Parameters params = request.parameters();

    // WMS query
    QUrlQuery query = translateWmtsParamToWmsQueryItem( QStringLiteral( "GetFeatureInfo" ), params, project, serverIface );

    // GetFeatureInfo query items
    query.addQueryItem( QStringLiteral( "query_layers" ), query.queryItemValue( QStringLiteral( "layers" ) ) );
    query.addQueryItem( QStringLiteral( "i" ), params.value( QStringLiteral( "I" ) ) );
    query.addQueryItem( QStringLiteral( "j" ), params.value( QStringLiteral( "J" ) ) );
    query.addQueryItem( QStringLiteral( "info_format" ), params.value( QStringLiteral( "INFOFORMAT" ) ) );

    QgsServerParameters wmsParams( query );
    QgsServerRequest wmsRequest( "?" + query.query( QUrl::FullyDecoded ) );
    QgsService *service = serverIface->serviceRegistry()->getService( wmsParams.service(), wmsParams.version() );
    service->executeRequest( wmsRequest, response, project );
  }

} // namespace QgsWmts




