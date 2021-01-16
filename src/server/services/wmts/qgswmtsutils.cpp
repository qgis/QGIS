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
#include "qgsserverprojectutils.h"

#include "qgsproject.h"
#include "qgsexception.h"
#include "qgscoordinatereferencesystem.h"
#include "qgslayertree.h"
#include "qgssettings.h"
#include "qgsprojectviewsettings.h"

namespace QgsWmts
{
  namespace
  {
    QMap< QString, tileMatrixInfo> populateFixedTileMatrixInfoMap();

    QgsCoordinateReferenceSystem wgs84 = QgsCoordinateReferenceSystem::fromOgcWmsCrs( geoEpsgCrsAuthId() );

    // Constant
    int tileSize = 256;
    double POINTS_TO_M = 2.83464567 / 10000.0;

    QMap< QString, tileMatrixInfo> fixedTileMatrixInfoMap = populateFixedTileMatrixInfoMap();
    QMap< QString, tileMatrixInfo> calculatedTileMatrixInfoMap; // for project without WMTSGrids configuration
  }

  QString implementationVersion()
  {
    return QStringLiteral( "1.0.0" );
  }


  QString serviceUrl( const QgsServerRequest &request, const QgsProject *project, const QgsServerSettings &settings )
  {
    QString href = QgsServerProjectUtils::wmtsServiceUrl( project ? *project : *QgsProject::instance(), request, settings );

    // Build default url
    if ( href.isEmpty() )
    {
      QUrl url = request.originalUrl();

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

  tileMatrixInfo calculateTileMatrixInfo( const QString &crsStr, const QgsProject *project )
  {
    // Does the CRS have fixed tile matrices
    if ( fixedTileMatrixInfoMap.contains( crsStr ) )
      return fixedTileMatrixInfoMap[crsStr];

    // Does the CRS have already calculated tile matrices
    if ( calculatedTileMatrixInfoMap.contains( crsStr ) )
      return calculatedTileMatrixInfoMap[crsStr];

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
      Q_UNUSED( cse )
    }

    tmi.unit = crs.mapUnits();
    tmi.hasAxisInverted = crs.hasAxisInverted();

    // calculate tile matrix scale denominator
    double scaleDenominator = 0.0;
    int colRes = ( tmi.extent.xMaximum() - tmi.extent.xMinimum() ) / tileSize;
    int rowRes = ( tmi.extent.yMaximum() - tmi.extent.yMinimum() ) / tileSize;
    double UNIT_TO_M = QgsUnitTypes::fromUnitToUnitFactor( tmi.unit, QgsUnitTypes::DistanceMeters );
    if ( colRes > rowRes )
      scaleDenominator = std::ceil( colRes * UNIT_TO_M / POINTS_TO_M );
    else
      scaleDenominator = std::ceil( rowRes * UNIT_TO_M / POINTS_TO_M );

    // Update extent to get a square one
    QgsRectangle extent = tmi.extent;
    double res = POINTS_TO_M * scaleDenominator / UNIT_TO_M;
    int col = std::ceil( ( extent.xMaximum() - extent.xMinimum() ) / ( tileSize * res ) );
    int row = std::ceil( ( extent.yMaximum() - extent.yMinimum() ) / ( tileSize * res ) );
    if ( col > 1 || row > 1 )
    {
      // Update scale
      if ( col > row )
      {
        res = col * res;
        scaleDenominator = col * scaleDenominator;
      }
      else
      {
        res = row * res;
        scaleDenominator = row * scaleDenominator;
      }
      // set col and row to 1 for the square
      col = 1;
      row = 1;
    }
    // Calculate extent
    double left = ( extent.xMinimum() + ( extent.xMaximum() - extent.xMinimum() ) / 2.0 ) - ( col / 2.0 ) * ( tileSize * res );
    double bottom = ( extent.yMinimum() + ( extent.yMaximum() - extent.yMinimum() ) / 2.0 ) - ( row / 2.0 ) * ( tileSize * res );
    double right = ( extent.xMinimum() + ( extent.xMaximum() - extent.xMinimum() ) / 2.0 ) + ( col / 2.0 ) * ( tileSize * res );
    double top = ( extent.yMinimum() + ( extent.yMaximum() - extent.yMinimum() ) / 2.0 ) + ( row / 2.0 ) * ( tileSize * res );
    tmi.extent = QgsRectangle( left, bottom, right, top );

    tmi.resolution = res;
    tmi.scaleDenominator = scaleDenominator;

    calculatedTileMatrixInfoMap[crsStr] = tmi;

    return tmi;
  }

  tileMatrixSetDef calculateTileMatrixSet( tileMatrixInfo tmi, double minScale )
  {
    QList< tileMatrixDef > tileMatrixList;
    double scaleDenominator = tmi.scaleDenominator;
    QgsRectangle extent = tmi.extent;
    QgsUnitTypes::DistanceUnit unit = tmi.unit;

    // constant
    double resolution = tmi.resolution;
    int column = std::ceil( ( extent.xMaximum() - extent.xMinimum() ) / ( tileSize * resolution ) );
    int row = std::ceil( ( extent.yMaximum() - extent.yMinimum() ) / ( tileSize * resolution ) );

    while ( scaleDenominator >= minScale )
    {
      double scale = scaleDenominator;
      // Calculate resolution based on scale denominator
      double res = resolution;
      int col = column;
      int r = row;

      tileMatrixDef tm;
      tm.resolution = res;
      tm.scaleDenominator = scale;
      tm.col = col;
      tm.row = r;
      tm.left = extent.xMinimum();
      tm.top = extent.yMaximum();
      tileMatrixList.append( tm );

      scaleDenominator = scale / 2;
      resolution = res / 2;
      column = col * 2;
      row = r * 2;
    }

    tileMatrixSetDef tms;
    tms.ref = tmi.ref;
    tms.extent = extent;
    tms.unit = unit;
    tms.hasAxisInverted = tmi.hasAxisInverted;
    tms.tileMatrixList = tileMatrixList;

    return tms;
  }

  double getProjectMinScale( const QgsProject *project )
  {
    double scale = -1.0;

    // default scales
    QgsSettings settings;
    QStringList scaleList = settings.value( QStringLiteral( "Map/scales" ), Qgis::defaultProjectScales() ).toString().split( ',' );
    //load project scales
    bool useProjectScales = project->viewSettings()->useProjectScales();
    const QVector< double >projectScales = project->viewSettings()->mapScales();
    if ( useProjectScales && projectScales.empty() )
    {
      scale = *std::min_element( projectScales.begin(), projectScales.end() );
    }
    else
    {
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
    }
    if ( scale < 500.0 )
    {
      return 500.0;
    }
    return scale;
  }

  QList< tileMatrixSetDef > getTileMatrixSetList( const QgsProject *project, const QString &tms_ref )
  {
    QList< tileMatrixSetDef > tmsList;

    bool gridsDefined = false;
    QStringList wmtsGridList = project->readListEntry( QStringLiteral( "WMTSGrids" ), QStringLiteral( "CRS" ), QStringList(), &gridsDefined );
    if ( gridsDefined )
    {
      if ( !tms_ref.isEmpty() && !wmtsGridList.contains( tms_ref ) )
      {
        throw QgsRequestNotWellFormedException( QStringLiteral( "TileMatrixSet is unknown" ) );
      }

      QStringList wmtsGridConfigList = project->readListEntry( QStringLiteral( "WMTSGrids" ), QStringLiteral( "Config" ) );
      for ( const QString &c : wmtsGridConfigList )
      {
        QStringList config = c.split( ',' );
        QString crsStr = config[0];
        if ( !tms_ref.isEmpty() && tms_ref != crsStr )
        {
          continue;
        }

        tileMatrixInfo tmi;
        double fixedTop = 0.0;
        double fixedLeft = 0.0;
        double resolution = -1.0;
        int col = -1;
        int row = -1;
        // Does the CRS have fixed tile matrices
        if ( fixedTileMatrixInfoMap.contains( crsStr ) )
        {
          tmi = fixedTileMatrixInfoMap[crsStr];
          // Calculate resolution based on scale denominator
          resolution = tmi.resolution;
          // Get fixed corner
          QgsRectangle extent = tmi.extent;
          fixedTop = extent.yMaximum();
          fixedLeft = extent.xMinimum();
          // Get numbers of column and row for the resolution to cover the extent
          col = std::ceil( ( extent.xMaximum() - extent.xMinimum() ) / ( tileSize * resolution ) );
          row = std::ceil( ( extent.yMaximum() - extent.yMinimum() ) / ( tileSize * resolution ) );
        }
        else
        {
          tmi.ref = crsStr;

          fixedTop = QVariant( config[1] ).toDouble();
          fixedLeft = QVariant( config[2] ).toDouble();
          double minScale = QVariant( config[3] ).toDouble();

          tmi.scaleDenominator = minScale;

          QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( crsStr );
          tmi.unit = crs.mapUnits();
          tmi.hasAxisInverted = crs.hasAxisInverted();

          QgsCoordinateTransform crsTransform( QgsCoordinateReferenceSystem::fromOgcWmsCrs( geoEpsgCrsAuthId() ), crs, project );
          try
          {
            // firstly transform CRS bounds expressed in WGS84 to CRS
            QgsRectangle extent = crsTransform.transformBoundingBox( crs.bounds() );
            // Calculate resolution based on scale denominator
            resolution = POINTS_TO_M * minScale / QgsUnitTypes::fromUnitToUnitFactor( tmi.unit, QgsUnitTypes::DistanceMeters );
            // Get numbers of column and row for the resolution to cover the extent
            col = std::ceil( ( extent.xMaximum() - extent.xMinimum() ) / ( tileSize * resolution ) );
            row = std::ceil( ( extent.yMaximum() - extent.yMinimum() ) / ( tileSize * resolution ) );
            // Calculate extent
            double bottom =  fixedTop - row * tileSize * resolution;
            double right =  fixedLeft + col * tileSize * resolution;
            tmi.extent = QgsRectangle( fixedLeft, bottom, right, fixedTop );
          }
          catch ( QgsCsException &cse )
          {
            Q_UNUSED( cse )
            continue;
          }
        }
        // get lastLevel
        tmi.lastLevel = QVariant( config[4] ).toInt();

        QList< tileMatrixDef > tileMatrixList;
        for ( int i = 0; i <= tmi.lastLevel; ++i )
        {
          double scale = tmi.scaleDenominator / std::pow( 2, i );
          double res = resolution / std::pow( 2, i );

          tileMatrixDef tm;
          tm.resolution = res;
          tm.scaleDenominator = scale;
          tm.col = col * std::pow( 2, i );
          tm.row = row * std::pow( 2, i );
          tm.left = fixedLeft;
          tm.top = fixedTop;
          tileMatrixList.append( tm );
        }

        tileMatrixSetDef tms;
        tms.ref = tmi.ref;
        tms.extent = tmi.extent;
        tms.unit = tmi.unit;
        tms.hasAxisInverted = tmi.hasAxisInverted;
        tms.tileMatrixList = tileMatrixList;

        tmsList.append( tms );
      }
      return tmsList;
    }

    double minScale = project->readNumEntry( QStringLiteral( "WMTSMinScale" ), QStringLiteral( "/" ), -1.0 );
    if ( minScale == -1.0 )
    {
      minScale = getProjectMinScale( project );
    }

    QStringList crsList = QgsServerProjectUtils::wmsOutputCrsList( *project );
    if ( !tms_ref.isEmpty() && !crsList.contains( tms_ref ) )
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "TileMatrixSet is unknown" ) );
    }
    for ( const QString &crsStr : crsList )
    {
      if ( !tms_ref.isEmpty() && tms_ref != crsStr )
      {
        continue;
      }
      tileMatrixInfo tmi = calculateTileMatrixInfo( crsStr, project );
      if ( tmi.scaleDenominator > 0.0 )
      {
        tmsList.append( calculateTileMatrixSet( tmi, minScale ) );
      }
    }

    return tmsList;
  }

  QList< layerDef > getWmtsLayerList( QgsServerInterface *serverIface, const QgsProject *project )
  {
    QList< layerDef > wmtsLayers;
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    QgsAccessControl *accessControl = serverIface->accessControls();
#else
    ( void )serverIface;
#endif

    // WMTS Project configuration
    bool wmtsProject = project->readBoolEntry( QStringLiteral( "WMTSLayers" ), QStringLiteral( "Project" ) );

    // Root Layer name
    QString rootLayerName = QgsServerProjectUtils::wmsRootName( *project );
    if ( rootLayerName.isEmpty() && !project->title().isEmpty() )
    {
      rootLayerName = project->title();
    }

    if ( wmtsProject && !rootLayerName.isEmpty() )
    {
      layerDef pLayer;
      pLayer.id = rootLayerName;

      if ( !project->title().isEmpty() )
      {
        pLayer.title = project->title();
        pLayer.abstract = project->title();
      }

      //transform the project native CRS into WGS84
      QgsRectangle projRect = QgsServerProjectUtils::wmsExtent( *project );
      QgsCoordinateReferenceSystem projCrs = project->crs();
      QgsCoordinateTransform exGeoTransform( projCrs, wgs84, project );
      try
      {
        pLayer.wgs84BoundingRect = exGeoTransform.transformBoundingBox( projRect );
      }
      catch ( const QgsCsException & )
      {
        pLayer.wgs84BoundingRect = QgsRectangle( -180, -90, 180, 90 );
      }

      // Formats
      bool wmtsPngProject = project->readBoolEntry( QStringLiteral( "WMTSPngLayers" ), QStringLiteral( "Project" ) );
      if ( wmtsPngProject )
        pLayer.formats << QStringLiteral( "image/png" );
      bool wmtsJpegProject = project->readBoolEntry( QStringLiteral( "WMTSJpegLayers" ), QStringLiteral( "Project" ) );
      if ( wmtsJpegProject )
        pLayer.formats << QStringLiteral( "image/jpeg" );

      // Project is not queryable in WMS
      //pLayer.queryable = ( nonIdentifiableLayers.count() != project->count() );
      pLayer.queryable = false;

      wmtsLayers.append( pLayer );
    }

    QStringList wmtsGroupNameList = project->readListEntry( QStringLiteral( "WMTSLayers" ), QStringLiteral( "Group" ) );
    if ( !wmtsGroupNameList.isEmpty() )
    {
      QgsLayerTreeGroup *treeRoot = project->layerTreeRoot();

      QStringList wmtsPngGroupNameList = project->readListEntry( QStringLiteral( "WMTSPngLayers" ), QStringLiteral( "Group" ) );
      QStringList wmtsJpegGroupNameList = project->readListEntry( QStringLiteral( "WMTSJpegLayers" ), QStringLiteral( "Group" ) );

      for ( const QString &gName : wmtsGroupNameList )
      {
        QgsLayerTreeGroup *treeGroup = treeRoot->findGroup( gName );
        if ( !treeGroup )
        {
          continue;
        }

        layerDef pLayer;
        pLayer.id = treeGroup->customProperty( QStringLiteral( "wmsShortName" ) ).toString();
        if ( pLayer.id.isEmpty() )
          pLayer.id = gName;

        pLayer.title = treeGroup->customProperty( QStringLiteral( "wmsTitle" ) ).toString();
        if ( pLayer.title.isEmpty() )
          pLayer.title = gName;

        pLayer.abstract = treeGroup->customProperty( QStringLiteral( "wmsAbstract" ) ).toString();

        QgsRectangle wgs84BoundingRect;
        bool queryable = false;
        double maxScale = 0.0;
        double minScale = 0.0;
        for ( QgsLayerTreeLayer *layer : treeGroup->findLayers() )
        {
          QgsMapLayer *l = layer->layer();
          if ( !l )
          {
            continue;
          }
          //transform the layer native CRS into WGS84
          QgsCoordinateReferenceSystem layerCrs = l->crs();
          QgsCoordinateTransform exGeoTransform( layerCrs, wgs84, project );
          try
          {
            wgs84BoundingRect.combineExtentWith( exGeoTransform.transformBoundingBox( l->extent() ) );
          }
          catch ( const QgsCsException & )
          {
            wgs84BoundingRect.combineExtentWith( QgsRectangle( -180, -90, 180, 90 ) );
          }
          if ( !queryable && l->flags().testFlag( QgsMapLayer::Identifiable ) )
          {
            queryable = true;
          }

          if ( l->hasScaleBasedVisibility() )
          {
            double lMaxScale = l->maximumScale();
            if ( lMaxScale > 0.0 && lMaxScale > maxScale )
            {
              maxScale = lMaxScale;
            }
            double lMinScale = l->minimumScale();
            if ( lMinScale > 0.0 && ( minScale == 0.0 || lMinScale < minScale ) )
            {
              minScale = lMinScale;
            }
          }
        }
        pLayer.wgs84BoundingRect = wgs84BoundingRect;
        pLayer.queryable = queryable;
        pLayer.maxScale = maxScale;
        pLayer.minScale = minScale;

        // Formats
        if ( wmtsPngGroupNameList.contains( gName ) )
          pLayer.formats << QStringLiteral( "image/png" );
        if ( wmtsJpegGroupNameList.contains( gName ) )
          pLayer.formats << QStringLiteral( "image/jpeg" );

        wmtsLayers.append( pLayer );
      }
    }

    QStringList wmtsLayerIdList = project->readListEntry( QStringLiteral( "WMTSLayers" ), QStringLiteral( "Layer" ) );
    QStringList wmtsPngLayerIdList = project->readListEntry( QStringLiteral( "WMTSPngLayers" ), QStringLiteral( "Layer" ) );
    QStringList wmtsJpegLayerIdList = project->readListEntry( QStringLiteral( "WMTSJpegLayers" ), QStringLiteral( "Layer" ) );

    for ( const QString &lId : wmtsLayerIdList )
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

      layerDef pLayer;
      pLayer.id = l->name();
      if ( !l->shortName().isEmpty() )
        pLayer.id = l->shortName();
      pLayer.id = pLayer.id.replace( ' ', '_' );

      pLayer.title = l->title();
      pLayer.abstract = l->abstract();

      //transform the layer native CRS into WGS84
      QgsCoordinateReferenceSystem layerCrs = l->crs();
      QgsCoordinateTransform exGeoTransform( layerCrs, wgs84, project );
      try
      {
        pLayer.wgs84BoundingRect = exGeoTransform.transformBoundingBox( l->extent() );
      }
      catch ( const QgsCsException & )
      {
        pLayer.wgs84BoundingRect = QgsRectangle( -180, -90, 180, 90 );
      }

      // Formats
      if ( wmtsPngLayerIdList.contains( lId ) )
        pLayer.formats << QStringLiteral( "image/png" );
      if ( wmtsJpegLayerIdList.contains( lId ) )
        pLayer.formats << QStringLiteral( "image/jpeg" );

      pLayer.queryable = ( l->flags().testFlag( QgsMapLayer::Identifiable ) );

      if ( l->hasScaleBasedVisibility() )
      {
        pLayer.maxScale = l->maximumScale();
        pLayer.minScale = l->minimumScale();
      }
      else
      {
        pLayer.maxScale = 0.0;
        pLayer.minScale = 0.0;
      }

      wmtsLayers.append( pLayer );
    }
    return wmtsLayers;
  }

  tileMatrixSetLinkDef getLayerTileMatrixSetLink( const layerDef layer, const tileMatrixSetDef tms, const QgsProject *project )
  {
    tileMatrixSetLinkDef tmsl;

    QMap< int, tileMatrixLimitDef > tileMatrixLimits;

    QgsRectangle rect( layer.wgs84BoundingRect );
    if ( tms.ref != QLatin1String( "EPSG:4326" ) )
    {
      QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( tms.ref );
      QgsCoordinateTransform exGeoTransform( wgs84, crs, project );
      try
      {
        rect = exGeoTransform.transformBoundingBox( layer.wgs84BoundingRect );
      }
      catch ( const QgsCsException & )
      {
        return tmsl;
      }
    }
    tmsl.ref = tms.ref;

    rect = rect.intersect( tms.extent );

    int tmIdx = -1;
    for ( const tileMatrixDef &tm : tms.tileMatrixList )
    {
      ++tmIdx;

      if ( layer.maxScale > 0.0 && tm.scaleDenominator > layer.maxScale )
      {
        continue;
      }
      if ( layer.minScale > 0.0 && tm.scaleDenominator < layer.minScale )
      {
        continue;
      }

      double res = tm.resolution;

      tileMatrixLimitDef tml;
      tml.minCol = std::floor( ( rect.xMinimum() - tm.left ) / ( tileSize * res ) );
      tml.maxCol = std::ceil( ( rect.xMaximum() - tm.left ) / ( tileSize * res ) ) - 1;
      tml.minRow = std::floor( ( tm.top - rect.yMaximum() ) / ( tileSize * res ) );
      tml.maxRow = std::ceil( ( tm.top - rect.yMinimum() ) / ( tileSize * res ) ) - 1;

      tileMatrixLimits[tmIdx] = tml;
    }

    tmsl.tileMatrixLimits = tileMatrixLimits;
    return tmsl;
  }

  QUrlQuery translateWmtsParamToWmsQueryItem( const QString &request, const QgsWmtsParameters &params,
      const QgsProject *project, QgsServerInterface *serverIface )
  {
#ifndef HAVE_SERVER_PYTHON_PLUGINS
    ( void )serverIface;
#endif

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
      for ( const QString &gName : wmtsGroupNameList )
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
      for ( const QString &lId : wmtsLayerIdList )
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

    // verifying TileMatrixSet value
    QList< tileMatrixSetDef > tmsList = getTileMatrixSetList( project, tms_ref );
    if ( tmsList.isEmpty() )
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "TileMatrixSet is unknown" ) );
    }
    tileMatrixSetDef tms = tmsList[0];
    if ( tms.ref != tms_ref )
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "TileMatrixSet is unknown" ) );
    }

    //defining TileMatrix idx
    int tm_idx = params.tileMatrixAsInt();
    //read TileMatrix
    if ( tm_idx < 0 || tms.tileMatrixList.count() < tm_idx )
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "TileMatrix is unknown" ) );
    }
    tileMatrixDef tm = tms.tileMatrixList.at( tm_idx );

    //defining TileRow
    int tr = params.tileRowAsInt();
    //read TileRow
    if ( tr < 0 || tm.row <= tr )
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "TileRow is unknown" ) );
    }

    //defining TileCol
    int tc = params.tileColAsInt();
    //read TileCol
    if ( tc < 0 || tm.col <= tc )
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "TileCol is unknown" ) );
    }

    double res = tm.resolution;
    double minx = tm.left + tc * ( tileSize * res );
    double miny = tm.top - ( tr + 1 ) * ( tileSize * res );
    double maxx = tm.left + ( tc + 1 ) * ( tileSize * res );
    double maxy = tm.top - tr * ( tileSize * res );
    QString bbox;
    if ( tms.hasAxisInverted )
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
    query.addQueryItem( QgsWmsParameterForWmts::name( QgsWmsParameterForWmts::LAYERS ), layer );
    query.addQueryItem( QgsWmsParameterForWmts::name( QgsWmsParameterForWmts::STYLES ), QString() );
    query.addQueryItem( QgsWmsParameterForWmts::name( QgsWmsParameterForWmts::CRS ), tms.ref );
    query.addQueryItem( QgsWmsParameterForWmts::name( QgsWmsParameterForWmts::BBOX ), bbox );
    query.addQueryItem( QgsWmsParameterForWmts::name( QgsWmsParameterForWmts::WIDTH ), QStringLiteral( "256" ) );
    query.addQueryItem( QgsWmsParameterForWmts::name( QgsWmsParameterForWmts::HEIGHT ), QStringLiteral( "256" ) );
    query.addQueryItem( QgsWmsParameterForWmts::name( QgsWmsParameterForWmts::FORMAT ), format );
    if ( params.format() == QgsWmtsParameters::Format::PNG )
    {
      query.addQueryItem( QgsWmsParameterForWmts::name( QgsWmsParameterForWmts::TRANSPARENT ), QStringLiteral( "true" ) );
    }
    query.addQueryItem( QgsWmsParameterForWmts::name( QgsWmsParameterForWmts::DPI ), QStringLiteral( "96" ) );
    query.addQueryItem( QgsWmsParameterForWmts::name( QgsWmsParameterForWmts::TILED ), QStringLiteral( "true" ) );

    return query;
  }

  namespace
  {

    QMap< QString, tileMatrixInfo> populateFixedTileMatrixInfoMap()
    {
      QMap< QString, tileMatrixInfo> m;

      // Tile matrix information
      // to build tile matrix set like Google Mercator or TMS
      // some references for resolution
      // https://github.com/mapserver/mapcache/blob/master/lib/configuration.c#L94
      tileMatrixInfo tmi3857;
      tmi3857.ref = QStringLiteral( "EPSG:3857" );
      tmi3857.extent = QgsRectangle( -20037508.3427892480, -20037508.3427892480, 20037508.3427892480, 20037508.3427892480 );
      tmi3857.resolution = 156543.0339280410;
      tmi3857.scaleDenominator = 559082264.0287179;
      tmi3857.unit = QgsUnitTypes::DistanceMeters;
      m[tmi3857.ref] = tmi3857;

      // To build tile matrix set like mapcache for WGS84
      // some references for resolution
      // https://github.com/mapserver/mapcache/blob/master/lib/configuration.c#L73
      tileMatrixInfo tmi4326;
      tmi4326.ref = QStringLiteral( "EPSG:4326" );
      tmi4326.extent = QgsRectangle( -180, -90, 180, 90 );
      tmi4326.resolution = 0.703125000000000;
      tmi4326.scaleDenominator = 279541132.0143588675418869;
      tmi4326.unit = QgsUnitTypes::DistanceDegrees;
      tmi4326.hasAxisInverted = true;
      m[tmi4326.ref] = tmi4326;

      return m;
    }

  }

} // namespace QgsWmts


