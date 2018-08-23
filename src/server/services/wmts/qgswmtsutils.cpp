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
#include "qgswmtsparameters.h"
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
    QMap< QgsUnitTypes::DistanceUnit, double> populateInchesPerUnit();
    QMap< QString, tileMatrixInfo> populateTileMatrixInfoMap();

    QgsCoordinateReferenceSystem wgs84 = QgsCoordinateReferenceSystem::fromOgcWmsCrs( GEO_EPSG_CRS_AUTHID );

    int DOTS_PER_INCH = 72;
    double METERS_PER_INCH = 0.02540005080010160020;
    QMap< QgsUnitTypes::DistanceUnit, double> INCHES_PER_UNIT = populateInchesPerUnit();
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
      href = QgsServerProjectUtils::wmtsServiceUrl( *project );
    }

    // Build default url
    if ( href.isEmpty() )
    {
      QUrl url = request.url();

      QgsWmtsParameters params;
      params.load( QUrlQuery( url ) );
      params.remove( QgsServerParameter::REQUEST );
      params.remove( QgsServerParameter::VERSION_SERVICE );
      params.remove( QgsServerParameter::SERVICE );

      url.setQuery( params.urlQuery() );
      href = url.toString();
    }

    return  href;
  }

  tileMatrixInfo getTileMatrixInfo( const QString &crsStr, const QgsProject *project )
  {
    if ( tileMatrixInfoMap.contains( crsStr ) )
      return tileMatrixInfoMap[crsStr];

    tileMatrixInfo tmi;
    tmi.ref = crsStr;

    QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( crsStr );
    QgsCoordinateTransform crsTransform( wgs84, crs, project );
    try
    {
      tmi.extent = crsTransform.transformBoundingBox( crs.bounds() );
    }
    catch ( QgsCsException &cse )
    {
      Q_UNUSED( cse );
    }

    tmi.unit = crs.mapUnits();

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

  tileMatrixSetDef getTileMatrixSet( tileMatrixInfo tmi, double minScale )
  {
    QList< tileMatrixDef > tileMatrixList;
    double scaleDenominator = tmi.scaleDenominator;
    QgsRectangle extent = tmi.extent;
    QgsUnitTypes::DistanceUnit unit = tmi.unit;

    while ( scaleDenominator >= minScale )
    {
      double scale = scaleDenominator;
      double res = 0.00028 * scale / METERS_PER_INCH / INCHES_PER_UNIT[ unit ];
      int col = std::round( ( extent.xMaximum() - extent.xMinimum() ) / ( tileWidth * res ) );
      int row = std::round( ( extent.yMaximum() - extent.yMinimum() ) / ( tileHeight * res ) );
      double left = ( extent.xMinimum() + ( extent.xMaximum() - extent.xMinimum() ) / 2.0 ) - ( col / 2.0 ) * ( tileWidth * res );
      double top = ( extent.yMinimum() + ( extent.yMaximum() - extent.yMinimum() ) / 2.0 ) + ( row / 2.0 ) * ( tileHeight * res );

      tileMatrixDef tm;
      tm.resolution = res;
      tm.scaleDenominator = scale;
      tm.col = col;
      tm.row = row;
      tm.left = std::max( left, extent.xMinimum() );
      tm.top = std::min( top, extent.yMaximum() );
      tileMatrixList.append( tm );

      scaleDenominator = scale / 2;
    }

    tileMatrixSetDef tms;
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
      for ( const QString &scaleText : scaleList )
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

  QList< tileMatrixSetDef > getTileMatrixSetList( const QgsProject *project )
  {
    QList< tileMatrixSetDef > tmsList;

    double minScale = project->readNumEntry( QStringLiteral( "WMTSMinScale" ), QStringLiteral( "/" ), -1.0 );
    if ( minScale == -1.0 )
    {
      minScale = getProjectMinScale( project );
    }

    QStringList crsList = QgsServerProjectUtils::wmsOutputCrsList( *project );
    for ( const QString &crsStr : crsList )
    {
      tileMatrixInfo tmi = getTileMatrixInfo( crsStr, project );
      if ( tmi.scaleDenominator > 0.0 )
      {
        tmsList.append( getTileMatrixSet( tmi, minScale ) );
      }
    }

    return tmsList;
  }

  QUrlQuery translateWmtsParamToWmsQueryItem( const QString &request, const QgsWmtsParameters &params,
      const QgsProject *project, QgsServerInterface *serverIface )
  {
    //defining Layer
    QString layer = params.layer();
    //read Layer
    if ( layer.isEmpty() )
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
      for ( QString gName : wmtsGroupNameList )
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
      for ( QString lId : wmtsLayerIdList )
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
    QString format = params.formatAsString();
    //read Format
    if ( format.isEmpty() )
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "Format is mandatory" ) );
    }

    //defining TileMatrixSet ref
    QString tms_ref = params.tileMatrixSet();
    //read TileMatrixSet
    if ( tms_ref.isEmpty() )
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "TileMatrixSet is mandatory" ) );
    }

    // verifying TileMatricSet value
    QStringList crsList = QgsServerProjectUtils::wmsOutputCrsList( *project );
    if ( !crsList.contains( tms_ref ) )
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "TileMatrixSet is unknown" ) );
    }

    tileMatrixInfo tmi = getTileMatrixInfo( tms_ref, project );
    if ( tmi.scaleDenominator == 0.0 )
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "TileMatrixSet is unknown" ) );
    }
    tileMatrixSetDef tms = getTileMatrixSet( tmi, getProjectMinScale( project ) );

    //difining TileMatrix idx
    int tm_idx = params.tileMatrixAsInt();
    //read TileMatrix
    if ( tm_idx == -1 )
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "TileMatrix is mandatory" ) );
    }
    if ( tms.tileMatrixList.count() < tm_idx )
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "TileMatrix is unknown" ) );
    }
    tileMatrixDef tm = tms.tileMatrixList.at( tm_idx );

    //defining TileRow
    int tr = params.tileRowAsInt();
    //read TileRow
    if ( tr == -1 )
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "TileRow is mandatory" ) );
    }
    if ( tm.row <= tr )
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "TileRow is unknown" ) );
    }

    //defining TileCol
    int tc = params.tileColAsInt();
    //read TileCol
    if ( tc == -1 )
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
      query.addQueryItem( QgsServerParameter::name( QgsServerParameter::MAP ), params.value( QStringLiteral( "MAP" ) ) );
    }
    query.addQueryItem( QgsServerParameter::name( QgsServerParameter::SERVICE ), QStringLiteral( "WMS" ) );
    query.addQueryItem( QgsServerParameter::name( QgsServerParameter::VERSION_SERVICE ), QStringLiteral( "1.3.0" ) );
    query.addQueryItem( QgsServerParameter::name( QgsServerParameter::REQUEST ), request );
    query.addQueryItem( QgsWmsParameter::name( QgsWmsParameter::LAYERS ), layer );
    query.addQueryItem( QgsWmsParameter::name( QgsWmsParameter::STYLES ), QString() );
    query.addQueryItem( QgsWmsParameter::name( QgsWmsParameter::CRS ), tms.ref );
    query.addQueryItem( QgsWmsParameter::name( QgsWmsParameter::BBOX ), bbox );
    query.addQueryItem( QgsWmsParameter::name( QgsWmsParameter::WIDTH ), QStringLiteral( "256" ) );
    query.addQueryItem( QgsWmsParameter::name( QgsWmsParameter::HEIGHT ), QStringLiteral( "256" ) );
    query.addQueryItem( QgsWmsParameter::name( QgsWmsParameter::FORMAT ), format );
    if ( params.format() == QgsWmtsParameters::Format::PNG )
    {
      query.addQueryItem( QgsWmsParameter::name( QgsWmsParameter::TRANSPARENT ), QStringLiteral( "true" ) );
    }
    query.addQueryItem( QgsWmsParameter::name( QgsWmsParameter::DPI ), QStringLiteral( "96" ) );

    return query;
  }

  namespace
  {

    QMap< QgsUnitTypes::DistanceUnit, double> populateInchesPerUnit()
    {
      QMap< QgsUnitTypes::DistanceUnit, double>  m;
      m[ QgsUnitTypes::DistanceMeters ] = 39.37;
      m[ QgsUnitTypes::DistanceFeet ] = 12.0;
      m[ QgsUnitTypes::DistanceYards ] = 36.0;
      m[ QgsUnitTypes::DistanceMiles ] = 63360.0;
      m[ QgsUnitTypes::DistanceDegrees ] = 4374754.0;
      m[ QgsUnitTypes::DistanceKilometers ] = m[ QgsUnitTypes::DistanceMeters ] * 1000.0;
      m[ QgsUnitTypes::DistanceNauticalMiles ] = m[ QgsUnitTypes::DistanceMeters ] * 1852.0;
      m[ QgsUnitTypes::DistanceCentimeters ] = m[ QgsUnitTypes::DistanceMeters ] / 100.0;
      m[ QgsUnitTypes::DistanceMillimeters ] = m[ QgsUnitTypes::DistanceMeters ] / 1000.0;
      return m;
    }

    QMap< QString, tileMatrixInfo> populateTileMatrixInfoMap()
    {
      QMap< QString, tileMatrixInfo> m;

      // Tile matrix information
      // to build tile matrix set like Google Mercator or TMS
      tileMatrixInfo tmi3857;
      tmi3857.ref = QStringLiteral( "EPSG:3857" );
      tmi3857.extent = QgsRectangle( -20037508.3427892480, -20037508.3427892480, 20037508.3427892480, 20037508.3427892480 );
      tmi3857.scaleDenominator = 559082264.0287179;
      tmi3857.unit = QgsUnitTypes::DistanceMeters;
      m[tmi3857.ref] = tmi3857;


      tileMatrixInfo tmi4326;
      tmi4326.ref = QStringLiteral( "EPSG:4326" );
      tmi4326.extent = QgsRectangle( -180, -90, 180, 90 );
      tmi4326.scaleDenominator = 279541132.0143588675418869;
      tmi4326.unit = QgsUnitTypes::DistanceDegrees;
      m[tmi4326.ref] = tmi4326;

      return m;
    }

  }

} // namespace QgsWmts


