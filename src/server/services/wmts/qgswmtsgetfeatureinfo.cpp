/***************************************************************************
                            qgswmtsgetfeatureinfo.cpp
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
#include "qgswmtsparameters.h"
#include "qgswmtsgetfeatureinfo.h"

namespace QgsWmts
{

  void writeGetFeatureInfo( QgsServerInterface *serverIface, const QgsProject *project,
                            const QString &version, const QgsServerRequest &request,
                            QgsServerResponse &response )
  {
    Q_UNUSED( version )
    const QgsWmtsParameters params( QUrlQuery( request.url() ) );

    // WMS query
    QUrlQuery query = translateWmtsParamToWmsQueryItem( QStringLiteral( "GetFeatureInfo" ), params, project, serverIface );

    // GetFeatureInfo query items
    query.addQueryItem( QgsWmsParameterForWmts::name( QgsWmsParameterForWmts::QUERY_LAYERS ), params.layer() );
    query.addQueryItem( QgsWmsParameterForWmts::name( QgsWmsParameterForWmts::I ), params.i() );
    query.addQueryItem( QgsWmsParameterForWmts::name( QgsWmsParameterForWmts::J ), params.j() );
    query.addQueryItem( QgsWmsParameterForWmts::name( QgsWmsParameterForWmts::INFO_FORMAT ), params.infoFormatAsString() );

    const QgsServerParameters wmsParams( query );
    const QgsServerRequest wmsRequest( "?" + query.query( QUrl::FullyDecoded ) );
    QgsService *service = serverIface->serviceRegistry()->getService( wmsParams.service(), wmsParams.version() );
    service->executeRequest( wmsRequest, response, project );
  }

} // namespace QgsWmts




