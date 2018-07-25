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

    //defining Layer
    QString layer;
    //read Layer
    QMap<QString, QString>::const_iterator layer_it = params.constFind( QStringLiteral( "LAYER" ) );
    if ( layer_it != params.constEnd() )
    {
      layer = layer_it.value();
    }
    else
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "Layer is mandatory" ) );
    }

    //defining Format
    QString format;
    //read Format
    QMap<QString, QString>::const_iterator format_it = params.constFind( QStringLiteral( "FORMAT" ) );
    if ( format_it != params.constEnd() )
    {
      format = format_it.value();
    }
    else
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "Format is mandatory" ) );
    }

    QList< tileMatrixSet > tmsList = getTileMatrixSetList( project );
    if ( tmsList.isEmpty() )
    {
      throw QgsServiceException( QStringLiteral( "UnknownError" ),
                                 QStringLiteral( "Service not well configured" ) );
    }

    //defining TileMatrixSet ref
    QString tms_ref;
    //read TileMatrixSet
    QMap<QString, QString>::const_iterator tms_ref_it = params.constFind( QStringLiteral( "TILEMATRIXSET" ) );
    if ( tms_ref_it != params.constEnd() )
    {
      tms_ref = tms_ref_it.value();
    }
    else
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "TileMatrixSet is mandatory" ) );
    }

    bool tms_ref_valid = false;
    tileMatrixSet tms;
    QList<tileMatrixSet>::iterator tmsIt = tmsList.begin();
    for ( ; tmsIt != tmsList.end(); ++tmsIt )
    {
      tileMatrixSet &tmsi = *tmsIt;
      if ( tmsi.ref == tms_ref )
      {
        tms_ref_valid = true;
        tms = tmsi;
        break;
      }
    }
    if ( !tms_ref_valid )
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "TileMatrixSet is unknown" ) );
    }

    bool conversionSuccess = false;

    //difining TileMatrix idx
    int tm_idx;
    //read TileMatrix
    QMap<QString, QString>::const_iterator tm_ref_it = params.constFind( QStringLiteral( "TILEMATRIX" ) );
    if ( tm_ref_it != params.constEnd() )
    {
      QString tm_ref = tm_ref_it.value();
      tm_idx = tm_ref.toInt( &conversionSuccess );
      if ( !conversionSuccess )
      {
        throw QgsRequestNotWellFormedException( QStringLiteral( "TileMatrix is unknown" ) );
      }
    }
    else
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "TileMatrix is mandatory" ) );
    }
    if ( tms.tileMatrixList.count() < tm_idx )
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "TileMatrix is unknown" ) );
    }
    tileMatrix tm = tms.tileMatrixList.at( tm_idx );

    //defining TileRow
    int tr;
    //read TileRow
    QMap<QString, QString>::const_iterator tr_it = params.constFind( QStringLiteral( "TILEROW" ) );
    if ( tr_it != params.constEnd() )
    {
      QString tr_str = tr_it.value();
      conversionSuccess = false;
      tr = tr_str.toInt( &conversionSuccess );
      if ( !conversionSuccess )
      {
        throw QgsRequestNotWellFormedException( QStringLiteral( "TileRow is unknown" ) );
      }
    }
    else
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "TileRow is mandatory" ) );
    }
    if ( tm.row <= tr )
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "TileRow is unknown" ) );
    }

    //defining TileCol
    int tc;
    //read TileCol
    QMap<QString, QString>::const_iterator tc_it = params.constFind( QStringLiteral( "TILECOL" ) );
    if ( tc_it != params.constEnd() )
    {
      QString tc_str = tc_it.value();
      conversionSuccess = false;
      tc = tc_str.toInt( &conversionSuccess );
      if ( !conversionSuccess )
      {
        throw QgsRequestNotWellFormedException( QStringLiteral( "TileCol is unknown" ) );
      }
    }
    else
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "TileCol is mandatory" ) );
    }
    if ( tm.col <= tc )
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "TileCol is unknown" ) );
    }

    int tileWidth = 256;
    int tileHeight = 256;
    double res = tm.resolution;
    double minx = tm.left + tc * ( tileWidth * res );
    double miny = tm.top - ( tr + 1 ) * ( tileHeight * res );
    double maxx = tm.left + ( tc + 1 ) * ( tileWidth * res );
    double maxy = tm.top - tr * ( tileHeight * res );
    QString bbox;
    if ( tms.ref == "EPSG:4326" )
    {
      bbox = qgsDoubleToString( miny, 6 ) + ',' +
             qgsDoubleToString( minx, 6 ) + ',' +
             qgsDoubleToString( maxy, 6 ) + ',' +
             qgsDoubleToString( maxx, 6 );
    }
    else
    {
      bbox = qgsDoubleToString( minx, 6 ) + ',' +
             qgsDoubleToString( miny, 6 ) + ',' +
             qgsDoubleToString( maxx, 6 ) + ',' +
             qgsDoubleToString( maxy, 6 );
    }

    QUrlQuery query;
    if ( !params.value( QStringLiteral( "MAP" ) ).isEmpty() )
    {
      query.addQueryItem( QStringLiteral( "map" ), params.value( QStringLiteral( "MAP" ) ) );
    }
    query.addQueryItem( QStringLiteral( "service" ), QStringLiteral( "WMS" ) );
    query.addQueryItem( QStringLiteral( "version" ), QStringLiteral( "1.3.0" ) );
    query.addQueryItem( QStringLiteral( "request" ), QStringLiteral( "GetMap" ) );
    query.addQueryItem( QStringLiteral( "layers" ), layer );
    query.addQueryItem( QStringLiteral( "styles" ), QString() );
    query.addQueryItem( QStringLiteral( "crs" ), tms.ref );
    query.addQueryItem( QStringLiteral( "bbox" ), bbox );
    query.addQueryItem( QStringLiteral( "width" ), QStringLiteral( "256" ) );
    query.addQueryItem( QStringLiteral( "height" ), QStringLiteral( "256" ) );
    query.addQueryItem( QStringLiteral( "format" ), format );
    if ( format.startsWith( QStringLiteral( "image/png" ) ) )
    {
      query.addQueryItem( QStringLiteral( "transparent" ), QStringLiteral( "true" ) );
    }
    query.addQueryItem( QStringLiteral( "dpi" ), QStringLiteral( "96" ) );

    QgsServerParameters wmsParams( query );
    QgsServerRequest wmsRequest( "?" + query.query( QUrl::FullyDecoded ) );
    QgsService *service = serverIface->serviceRegistry()->getService( wmsParams.service(), wmsParams.version() );
    service->executeRequest( wmsRequest, response, project );
  }

} // namespace QgsWmts




