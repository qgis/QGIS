/***************************************************************************
                              qgswmtsutils.cpp
                              -------------------------
  begin                : July 23, 2018
  copyright            : (C) 2018 by René-Luc D'Hont
  email                : rldhont at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                  *
 *                                                                         *
 ***************************************************************************/

#include "qgswmtsutils.h"
#include "qgsconfigcache.h"
#include "qgsserverprojectutils.h"

#include "qgsproject.h"
#include "qgsexception.h"
#include "qgsmapserviceexception.h"
#include "qgscoordinatereferencesystem.h"
#include "qgssettings.h"


namespace QgsWmts
{
  QString implementationVersion()
  {
    return QStringLiteral( "1.0.0" );
  }


  QString serviceUrl( const QgsServerRequest &request, const QgsProject *project )
  {
    QString href;
    if ( project )
    {
      href = QgsServerProjectUtils::wmsServiceUrl( *project );
    }

    // Build default url
    if ( href.isEmpty() )
    {
      QUrl url = request.url();
      QUrlQuery q( url );

      q.removeAllQueryItems( QStringLiteral( "REQUEST" ) );
      q.removeAllQueryItems( QStringLiteral( "VERSION" ) );
      q.removeAllQueryItems( QStringLiteral( "SERVICE" ) );
      q.removeAllQueryItems( QStringLiteral( "_DC" ) );

      url.setQuery( q );
      href = url.toString( QUrl::FullyDecoded );

    }

    return  href;
  }

  QgsRectangle parseBbox( const QString &bboxStr )
  {
    QStringList lst = bboxStr.split( ',' );
    if ( lst.count() != 4 )
      return QgsRectangle();

    double d[4];
    bool ok;
    for ( int i = 0; i < 4; i++ )
    {
      lst[i].replace( ' ', '+' );
      d[i] = lst[i].toDouble( &ok );
      if ( !ok )
        return QgsRectangle();
    }
    return QgsRectangle( d[0], d[1], d[2], d[3] );
  }

  tileMatrixSet getTileMatrixSet( tileMatrixInfo tmi, double minScale )
  {
    int DOTS_PER_INCH = 72;
    double METERS_PER_INCH = 0.02540005080010160020;
    QMap< QString, double> INCHES_PER_UNIT;
    INCHES_PER_UNIT["inches"] = 1.0;
    INCHES_PER_UNIT["ft"] = 12.0;
    INCHES_PER_UNIT["mi"] = 63360.0;
    INCHES_PER_UNIT["m"] = 39.37;
    INCHES_PER_UNIT["km"] = 39370.0;
    INCHES_PER_UNIT["dd"] = 4374754.0;
    INCHES_PER_UNIT["yd"] = 36.0;
    INCHES_PER_UNIT["in"] = INCHES_PER_UNIT["inches"];
    INCHES_PER_UNIT["degrees"] = INCHES_PER_UNIT["dd"];
    INCHES_PER_UNIT["nmi"] = 1852.0 * INCHES_PER_UNIT["m"];
    INCHES_PER_UNIT["cm"] = INCHES_PER_UNIT["m"] / 100.0;
    INCHES_PER_UNIT["mm"] = INCHES_PER_UNIT["m"] / 1000.0;

    int tileWidth = 256;
    int tileHeight = 256;

    QList< tileMatrix > tileMatrixList;
    double scaleDenominator = tmi.scaleDenominator;
    QgsRectangle extent = tmi.extent;
    QString unit = tmi.unit;

    while ( scaleDenominator >= minScale )
    {
      double scale = scaleDenominator;
      double res = 0.00028 * scale / METERS_PER_INCH / INCHES_PER_UNIT[ unit ];
      int col = std::round( ( extent.xMaximum() - extent.xMinimum() ) / ( tileWidth * res ) );
      int row = std::round( ( extent.yMaximum() - extent.yMinimum() ) / ( tileHeight * res ) );
      double left = ( extent.xMinimum() + ( extent.xMaximum() - extent.xMinimum() ) / 2.0 ) - ( col / 2.0 ) * ( tileWidth * res );
      double top = ( extent.yMinimum() + ( extent.yMaximum() - extent.yMinimum() ) / 2.0 ) + ( row / 2.0 ) * ( tileHeight * res );

      tileMatrix tm;
      tm.resolution = res;
      tm.scaleDenominator = scale;
      tm.col = col;
      tm.row = row;
      tm.left = std::max( left, extent.xMinimum() );
      tm.top = std::min( top, extent.yMaximum() );
      tileMatrixList.append( tm );

      scaleDenominator = scale / 2;
    }

    tileMatrixSet tms;
    tms.ref = tmi.ref;
    tms.extent = extent;
    tms.unit = unit;
    tms.tileMatrixList = tileMatrixList;

    return tms;
  }

  QList< tileMatrixSet > getTileMatrixSetList( const QgsProject *project )
  {
    QList< tileMatrixSet > tmsList;

    double minScale = -1.0;
    double maxScale = -1.0;


    QgsRectangle projRect = QgsServerProjectUtils::wmsExtent( *project );
    QgsCoordinateReferenceSystem projCrs = project->crs();

    // default scales
    QgsSettings settings;
    QStringList scaleList = settings.value( QStringLiteral( "Map/scales" ), PROJECT_SCALES ).toString().split( ',' );
    //load project scales
    bool projectScales = project->readBoolEntry( QStringLiteral( "Scales" ), QStringLiteral( "/useProjectScales" ) );
    if ( projectScales )
    {
      scaleList = project->readListEntry( QStringLiteral( "Scales" ), QStringLiteral( "/ScalesList" ) );
    }
    // get min and max scales
    if ( !scaleList.isEmpty() )
    {
      Q_FOREACH ( const QString &scaleText, scaleList )
      {
        double scaleValue = scaleText.toDouble();
        if ( minScale == -1.0 && maxScale == -1.0 )
        {
          minScale = scaleValue;
          maxScale = scaleValue;
        }
        else
        {
          if ( scaleValue < minScale )
          {
            minScale = scaleValue;
          }
          if ( scaleValue > maxScale )
          {
            maxScale = scaleValue;
          }
        }
      }
    }
    else
    {
      minScale = 5000.0;
      maxScale = 1000000.0;
    }
    if ( minScale < 500.0 )
    {
      minScale = 500.0;
    }
    if ( minScale == maxScale || minScale > maxScale )
    {
      maxScale = minScale * 2.0;
    }

    QStringList crsList = QgsServerProjectUtils::wmsOutputCrsList( *project );
    Q_FOREACH ( const QString &crsText, crsList )
    {
      if ( crsText == "EPSG:3857" )
      {
        tileMatrixInfo tmi3857;
        tmi3857.ref = "EPSG:3857";
        tmi3857.extent = QgsRectangle( -20037508.3427892480, -20037508.3427892480, 20037508.3427892480, 20037508.3427892480 );
        tmi3857.scaleDenominator = 559082264.0287179;
        tmi3857.unit = "m";

        tmsList.append( getTileMatrixSet( tmi3857, minScale ) );
      }
      else if ( crsText == "EPSG:4326" )
      {
        tileMatrixInfo tmi4326;
        tmi4326.ref = "EPSG:4326";
        tmi4326.extent = QgsRectangle( -180, -90, 180, 90 );
        tmi4326.scaleDenominator = 279541132.0143588675418869;
        tmi4326.unit = "dd";

        tmsList.append( getTileMatrixSet( tmi4326, minScale ) );
      }
      else
      {
        tileMatrixInfo tmi;
        tmi.ref = crsText;

        QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( crsText );
        Q_NOWARN_DEPRECATED_PUSH
        QgsCoordinateTransform crsTransform( projCrs, crs );
        Q_NOWARN_DEPRECATED_POP
        try
        {
          tmi.extent = crsTransform.transformBoundingBox( projRect );
        }
        catch ( QgsCsException &cse )
        {
          Q_UNUSED( cse );
          continue;
        }

        tmi.scaleDenominator = maxScale;

        QgsUnitTypes::DistanceUnit mapUnits = crs.mapUnits();
        if ( mapUnits == QgsUnitTypes::DistanceMeters )
          tmi.unit = "m";
        else if ( mapUnits == QgsUnitTypes::DistanceKilometers )
          tmi.unit = "km";
        else if ( mapUnits == QgsUnitTypes::DistanceFeet )
          tmi.unit = "ft";
        else if ( mapUnits == QgsUnitTypes::DistanceNauticalMiles )
          tmi.unit = "nmi";
        else if ( mapUnits == QgsUnitTypes::DistanceYards )
          tmi.unit = "yd";
        else if ( mapUnits == QgsUnitTypes::DistanceMiles )
          tmi.unit = "mi";
        else if ( mapUnits == QgsUnitTypes::DistanceDegrees )
          tmi.unit = "dd";
        else if ( mapUnits == QgsUnitTypes::DistanceCentimeters )
          tmi.unit = "cm";
        else if ( mapUnits == QgsUnitTypes::DistanceMillimeters )
          tmi.unit = "mm";

        tmsList.append( getTileMatrixSet( tmi, minScale ) );
      }
    }

    return tmsList;
  }

  QUrlQuery translateWmtsParamToWmsQueryItem( const QString &request, const QgsServerRequest::Parameters &params, const QgsProject *project )
  {

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
    query.addQueryItem( QStringLiteral( "request" ), request );
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

    return query;
  }

} // namespace QgsWmts


