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
#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslayertreemodellegendnode.h"
#include "qgssettings.h"


namespace QgsWmts
{
  namespace
  {
    QMap< QString, double> populateInchesPerUnit();
    QMap< QString, tileMatrixInfo> populateTileMatrixInfoMap();

    QgsCoordinateReferenceSystem wgs84 = QgsCoordinateReferenceSystem::fromOgcWmsCrs( GEO_EPSG_CRS_AUTHID );

    int DOTS_PER_INCH = 72;
    double METERS_PER_INCH = 0.02540005080010160020;
    QMap< QString, double> INCHES_PER_UNIT = populateInchesPerUnit();
    int tileWidth = 256;
    int tileHeight = 256;

    QMap< QString, tileMatrixInfo> tileMatrixInfoMap = populateTileMatrixInfoMap();
  }

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

  tileMatrixInfo getTileMatrixInfo( const QString &crsStr )
  {
    if ( tileMatrixInfoMap.contains( crsStr ) )
      return tileMatrixInfoMap[crsStr];

    tileMatrixInfo tmi;
    tmi.ref = crsStr;

    QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( crsStr );
    Q_NOWARN_DEPRECATED_PUSH
    QgsCoordinateTransform crsTransform( wgs84, crs );
    Q_NOWARN_DEPRECATED_POP
    try
    {
      tmi.extent = crsTransform.transformBoundingBox( crs.bounds() );
    }
    catch ( QgsCsException &cse )
    {
      Q_UNUSED( cse );
    }

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

    // calculate tile matrix scale denominator
    double scaleDenominator = 0.0;
    int colRes = ( tmi.extent.xMaximum() - tmi.extent.xMinimum() ) / tileWidth;
    int rowRes = ( tmi.extent.yMaximum() - tmi.extent.yMinimum() ) / tileHeight;
    if ( colRes < rowRes )
      scaleDenominator = colRes * INCHES_PER_UNIT[ tmi.unit ] * METERS_PER_INCH / 0.00028;
    else
      scaleDenominator = rowRes * INCHES_PER_UNIT[ tmi.unit ] * METERS_PER_INCH / 0.00028;
    tmi.scaleDenominator = scaleDenominator;

    tileMatrixInfoMap[crsStr] = tmi;
    return tmi;
  }

  tileMatrixSet getTileMatrixSet( tileMatrixInfo tmi, double minScale )
  {
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

  double getProjectMinScale( const QgsProject *project )
  {
    double scale = -1.0;

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
        if ( scale == -1.0 )
        {
          scale = scaleValue;
        }
        else if ( scaleValue < scale )
        {
          scale = scaleValue;
        }
      }
    }
    if ( scale < 500.0 )
    {
      return 500.0;
    }
    return scale;
  }

  QList< tileMatrixSet > getTileMatrixSetList( const QgsProject *project )
  {
    QList< tileMatrixSet > tmsList;

    double minScale = getProjectMinScale( project );

    QStringList crsList = QgsServerProjectUtils::wmsOutputCrsList( *project );
    Q_FOREACH ( const QString &crsStr, crsList )
    {
      tileMatrixInfo tmi = getTileMatrixInfo( crsStr );
      if ( tmi.scaleDenominator > 0.0 )
      {
        tmsList.append( getTileMatrixSet( tmi, minScale ) );
      }
    }

    return tmsList;
  }

  QUrlQuery translateWmtsParamToWmsQueryItem( const QString &request, const QgsServerRequest::Parameters &params,
      const QgsProject *project, QgsServerInterface *serverIface )
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
    //check layer value
    bool wmtsProject = project->readBoolEntry( QStringLiteral( "WMTSLayers" ), QStringLiteral( "Project" ) );
    QStringList wmtsGroupNameList = project->readListEntry( QStringLiteral( "WMTSLayers" ), QStringLiteral( "Group" ) );
    QStringList wmtsLayerIdList = project->readListEntry( QStringLiteral( "WMTSLayers" ), QStringLiteral( "Layer" ) );
    QStringList wmtsLayerIds;
    if ( wmtsProject )
    {
      // Root Layer name
      QString rootLayerId = QgsServerProjectUtils::wmsRootName( *project );
      if ( rootLayerId.isEmpty() )
      {
        rootLayerId = project->title();
      }
      if ( !rootLayerId.isEmpty() )
      {
        wmtsLayerIds << rootLayerId;
      }
    }
    if ( !wmtsGroupNameList.isEmpty() )
    {
      QgsLayerTreeGroup *treeRoot = project->layerTreeRoot();
      Q_FOREACH ( QString gName, wmtsGroupNameList )
      {
        QgsLayerTreeGroup *treeGroup = treeRoot->findGroup( gName );
        if ( !treeGroup )
        {
          continue;
        }
        QString groupLayerId = treeGroup->customProperty( QStringLiteral( "wmsShortName" ) ).toString();
        if ( groupLayerId.isEmpty() )
        {
          groupLayerId = gName;
        }
        wmtsLayerIds << groupLayerId;
      }
    }
    if ( !wmtsLayerIdList.isEmpty() )
    {
#ifdef HAVE_SERVER_PYTHON_PLUGINS
      QgsAccessControl *accessControl = serverIface->accessControls();
#endif
      Q_FOREACH ( QString lId, wmtsLayerIdList )
      {
        QgsMapLayer *l = project->mapLayer( lId );
        if ( !l )
        {
          continue;
        }
#ifdef HAVE_SERVER_PYTHON_PLUGINS
        if ( !accessControl->layerReadPermission( l ) )
        {
          continue;
        }
#endif
        QString layerLayerId = l->shortName();
        if ( layerLayerId.isEmpty() )
        {
          layerLayerId = l->name();
        }
        wmtsLayerIds << layerLayerId;
      }
    }
    if ( !wmtsLayerIds.contains( layer ) )
    {
      QString msg = QObject::tr( "Layer '%1' not found" ).arg( layer );
      throw QgsBadRequestException( QStringLiteral( "LayerNotDefined" ), msg );
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

    // verifying TileMatricSet value
    QStringList crsList = QgsServerProjectUtils::wmsOutputCrsList( *project );
    if ( !crsList.contains( tms_ref ) )
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "TileMatrixSet is unknown" ) );
    }

    tileMatrixInfo tmi = getTileMatrixInfo( tms_ref );
    if ( tmi.scaleDenominator == 0.0 )
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "TileMatrixSet is unknown" ) );
    }
    tileMatrixSet tms = getTileMatrixSet( tmi, getProjectMinScale( project ) );

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

  namespace
  {

    QMap< QString, double> populateInchesPerUnit()
    {
      QMap< QString, double>  m;
      m["inches"] = 1.0;
      m["ft"] = 12.0;
      m["mi"] = 63360.0;
      m["m"] = 39.37;
      m["km"] = 39370.0;
      m["dd"] = 4374754.0;
      m["yd"] = 36.0;
      m["in"] = m["inches"];
      m["degrees"] = m["dd"];
      m["nmi"] = 1852.0 * m["m"];
      m["cm"] = m["m"] / 100.0;
      m["mm"] = m["m"] / 1000.0;
      return m;
    }

    QMap< QString, tileMatrixInfo> populateTileMatrixInfoMap()
    {
      QMap< QString, tileMatrixInfo> m;

      tileMatrixInfo tmi3857;
      tmi3857.ref = "EPSG:3857";
      tmi3857.extent = QgsRectangle( -20037508.3427892480, -20037508.3427892480, 20037508.3427892480, 20037508.3427892480 );
      tmi3857.scaleDenominator = 559082264.0287179;
      tmi3857.unit = "m";
      m[tmi3857.ref] = tmi3857;


      tileMatrixInfo tmi4326;
      tmi4326.ref = "EPSG:4326";
      tmi4326.extent = QgsRectangle( -180, -90, 180, 90 );
      tmi4326.scaleDenominator = 279541132.0143588675418869;
      tmi4326.unit = "dd";
      m[tmi4326.ref] = tmi4326;

      return m;
    }

  }

} // namespace QgsWmts


