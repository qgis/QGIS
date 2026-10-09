/***************************************************************************
                         qgsalgorithmchannelnetwork.cpp
                         ---------------------
    begin                : September 2026
    copyright            : (C) 2026 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmchannelnetwork.h"

#include <gdal.h>
#include <gdal_alg.h>
#include <ogrsf_frmts.h>

#include "qgslinestring.h"
#include "qgsogrutils.h"
#include "qgsrasteranalysisutils.h"

#include <QString>

using namespace Qt::StringLiterals;

///@cond PRIVATE

QStringList QgsChannelNetworkAlgorithmBase::tags() const
{
  return QObject::tr( "dem,channels,network,basins,drainage,junctions,strahler,hydrology" ).split( ',' );
}

void QgsChannelNetworkAlgorithmBase::addCommonParameters()
{
  auto thresholdParam = std::make_unique<QgsProcessingParameterNumber>( u"THRESHOLD"_s, QObject::tr( "Minimum stream order threshold" ), Qgis::ProcessingNumberParameterType::Integer, 5, false, 1 );
  thresholdParam->setHelp(
    QObject::tr(
      "Minimum Strahler stream order required to initiate a channel segment. Cells with a Strahler order equal to or greater than this threshold will be extracted as channel networks. Output stream "
      "orders on extracted vector features will be shifted so that the threshold order equals order 1."
    )
  );
  addParameter( thresholdParam.release() );

  auto subbasinsParam = std::make_unique<QgsProcessingParameterBoolean>( u"SUBBASINS"_s, QObject::tr( "Delineate subbasins" ), true );
  subbasinsParam->setHelp(
    QObject::tr( "If checked, individual subbasins will be delineated for every channel junction and tributary confluence. If unchecked, only major drainage basins for outer outlet nodes will be generated." )
  );
  addParameter( subbasinsParam.release() );

  addParameter( new QgsProcessingParameterVectorDestination( u"CHANNELS"_s, QObject::tr( "Channels" ), Qgis::ProcessingSourceType::VectorLine, QVariant(), true, true ) );
  addParameter( new QgsProcessingParameterVectorDestination( u"BASINS"_s, QObject::tr( "Drainage basins" ), Qgis::ProcessingSourceType::VectorPolygon, QVariant(), true, true ) );
  addParameter( new QgsProcessingParameterVectorDestination( u"JUNCTIONS"_s, QObject::tr( "Junctions" ), Qgis::ProcessingSourceType::VectorPoint, QVariant(), true, true ) );
}

void QgsChannelNetworkAlgorithmBase::extractChannelNetwork(
  const QgsRasterBlock *demBlock,
  const std::vector<int8_t> &d8Directions,
  const std::vector<int16_t> &strahlerOrders,
  int width,
  int height,
  const QgsRectangle &extent,
  const QgsCoordinateReferenceSystem &,
  int threshold,
  bool subbasins,
  QgsFeatureSink *channelSink,
  QgsFeatureSink *basinSink,
  QgsFeatureSink *junctionSink,
  QgsProcessingFeedback *feedback,
  const QVariantMap &parameters
)
{
  const std::size_t totalCells = static_cast<std::size_t>( width ) * height;
  const double cellWidth = extent.width() / width;
  const double cellHeight = extent.height() / height;

  std::vector<int32_t> nodesGrid( totalCells, 0 );
  std::vector<int32_t> basinsGrid( totalCells, 0 );

  QgsProcessingMultiStepFeedback multiStepFeedback( 5, feedback );
  multiStepFeedback.setStepWeights( { 1, 1, 1, 0.5, 4 } );

  for ( int row = 0; row < height; ++row )
  {
    for ( int col = 0; col < width; ++col )
    {
      const qgssize idx = static_cast<qgssize>( row ) * width + col;
      basinsGrid[idx] = demBlock->isNoData( row, col ) ? 0 : -1;
    }
  }

  int nNodes = 0;
  int nBasins = 0;
  QHash<int, int> basinToOrderMap;

  auto setNode = [&]( int column, int row, int id, NodeType type, int rawOrder, int basinId ) {
    if ( type != NodeType::Mouth )
    {
      nodesGrid[static_cast<qgssize>( row ) * width + column] = id;
    }

    const int shiftedOrder = rawOrder + 1 - threshold;
    if ( type == NodeType::Outlet || type == NodeType::Mouth )
    {
      basinToOrderMap[basinId] = shiftedOrder;
    }

    if ( junctionSink )
    {
      double xWorld;
      double yWorld;
      QgsRasterAnalysisUtils::pixelToMap( column, row, extent, cellWidth, cellHeight, xWorld, yWorld );
      const double z = demBlock->value( row, column );

      QString typeStr;
      switch ( type )
      {
        case NodeType::Spring:
          typeStr = u"Spring"_s;
          break;
        case NodeType::Junction:
          typeStr = u"Junction"_s;
          break;
        case NodeType::Outlet:
          typeStr = u"Outlet"_s;
          break;
        case NodeType::Mouth:
          typeStr = u"Mouth"_s;
          break;
      }

      QgsFeature feat;
      feat.setGeometry( QgsGeometry( std::make_unique<QgsPoint>( xWorld, yWorld, z ) ) );
      feat.setAttributes( QgsAttributes() << id << typeStr << shiftedOrder << basinId );
      if ( !junctionSink->addFeature( feat, QgsFeatureSink::FastInsert ) )
      {
        throw QgsProcessingException( writeFeatureError( junctionSink, parameters, QString() ) );
      }
      else
      {
        feedback->featureAddedToSink( u"JUNCTIONS"_s );
      }
    }
  };

  multiStepFeedback.setProgressText( QObject::tr( "Calculating junction nodes and seeding basins" ) );
  multiStepFeedback.setCurrentStep( 0 );
  for ( int row = 0; row < height; ++row )
  {
    if ( feedback->isCanceled() )
      return;

    multiStepFeedback.setProgress( static_cast<double>( row ) / height );

    for ( int col = 0; col < width; ++col )
    {
      const qgssize idx = static_cast<qgssize>( row ) * width + col;
      const int order = strahlerOrders[idx];
      if ( order >= threshold )
      {
        const int dir = d8Directions[idx];
        if ( dir >= 0 )
        {
          int neighborColumn = 0;
          int neighborRow = 0;
          if ( QgsRasterAnalysisUtils::neighborCellCoordinates( dir, row, col, neighborRow, neighborColumn, height, width ) )
          {
            const qgssize neighborIdx = static_cast<qgssize>( neighborRow ) * width + neighborColumn;
            if ( nodesGrid[neighborIdx] == 0 && strahlerOrders[neighborIdx] > order && d8Directions[neighborIdx] >= 0 )
            {
              setNode( neighborColumn, neighborRow, ++nNodes, NodeType::Junction, strahlerOrders[neighborIdx], 0 );

              if ( subbasins )
              {
                for ( int j = 0; j < 8; ++j )
                {
                  const int oppositeJ = ( j + 4 ) % 8;
                  int jColumn = 0;
                  int jRow = 0;
                  if ( QgsRasterAnalysisUtils::neighborCellCoordinates( oppositeJ, neighborRow, neighborColumn, jRow, jColumn, height, width ) )
                  {
                    const qgssize jIdx = static_cast<qgssize>( jRow ) * width + jColumn;
                    if ( d8Directions[jIdx] == j && strahlerOrders[jIdx] >= threshold )
                    {
                      basinsGrid[jIdx] = ++nBasins;
                      setNode( jColumn, jRow, 0, NodeType::Mouth, strahlerOrders[jIdx], nBasins );
                    }
                  }
                }
              }
            }
          }

          if ( order == threshold )
          {
            bool isSpring = true;
            for ( int j = 0; j < 8 && isSpring; ++j )
            {
              const int oppositeJ = ( j + 4 ) % 8;
              int jColumn = 0;
              int jRow = 0;
              if ( QgsRasterAnalysisUtils::neighborCellCoordinates( oppositeJ, row, col, jRow, jColumn, height, width ) )
              {
                const qgssize jIdx = static_cast<qgssize>( jRow ) * width + jColumn;
                if ( d8Directions[jIdx] == j )
                {
                  isSpring = strahlerOrders[jIdx] < threshold;
                }
              }
            }

            if ( isSpring )
            {
              setNode( col, row, ++nNodes, NodeType::Spring, order, 0 );
            }
          }
        }
        else
        {
          basinsGrid[idx] = ++nBasins;
          setNode( col, row, ++nNodes, NodeType::Outlet, order, nBasins );
        }
      }
    }
  }

  multiStepFeedback.setProgressText( QObject::tr( "Calculating drainage basins" ) );
  multiStepFeedback.setCurrentStep( 1 );
  auto getBasin = [&]( int startColumn, int startRow ) {
    int currentColumn = startColumn;
    int currentRow = startRow;
    qgssize currentIdx = static_cast<qgssize>( currentRow ) * width + currentColumn;
    int basin = basinsGrid[currentIdx];

    if ( basin < 0 )
    {
      std::vector<std::size_t> stack;
      while ( basin < 0 )
      {
        const int dir = d8Directions[currentIdx];
        if ( dir < 0 )
          break;

        stack.push_back( currentIdx );

        int neighborColumn = 0;
        int neighborRow = 0;
        if ( !QgsRasterAnalysisUtils::neighborCellCoordinates( dir, currentRow, currentColumn, neighborRow, neighborColumn, height, width ) )
          break;

        currentColumn = neighborColumn;
        currentRow = neighborRow;
        currentIdx = static_cast<qgssize>( currentRow ) * width + currentColumn;
        basin = basinsGrid[currentIdx];
      }

      if ( basin < 0 )
      {
        // not linked to any basin, mark as processed!
        basin = 0;
      }

      if ( stack.empty() )
      {
        basinsGrid[currentIdx] = basin;
      }
      for ( const std::size_t idx : stack )
      {
        basinsGrid[idx] = basin;
      }
    }
    return basin;
  };

  for ( int row = 0; row < height; ++row )
  {
    if ( feedback->isCanceled() )
      return;

    multiStepFeedback.setProgress( static_cast<double>( row ) / height );

    for ( int col = 0; col < width; ++col )
    {
      getBasin( col, row );
    }
  }

  if ( feedback->isCanceled() )
    return;

  multiStepFeedback.setProgressText( QObject::tr( "Extracting basins as polygons" ) );
  multiStepFeedback.setCurrentStep( 2 );
  if ( basinSink )
  {
    // create an in-memory GDAL raster dataset for the basins grid
    GDALDriverH hMemDriver = GDALGetDriverByName( "MEM" );
    if ( !hMemDriver )
      throw QgsProcessingException( QObject::tr( "GDAL MEM driver is unavailable." ) );

    GDALDatasetH hMemDS = GDALCreate( hMemDriver, "", width, height, 1, GDT_Int32, nullptr );
    if ( !hMemDS )
      throw QgsProcessingException( QObject::tr( "Could not create in-memory GDAL dataset for basin polygonization." ) );

    double adfGeoTransform[6] = { extent.xMinimum(), cellWidth, 0.0, extent.yMaximum(), 0.0, -cellHeight };
    GDALSetGeoTransform( hMemDS, adfGeoTransform );

    GDALRasterBandH hBand = GDALGetRasterBand( hMemDS, 1 );
    GDALSetRasterNoDataValue( hBand, 0 );

    const CPLErr writeErr = GDALRasterIO( hBand, GF_Write, 0, 0, width, height, const_cast<int32_t *>( basinsGrid.data() ), width, height, GDT_Int32, 0, 0 );
    if ( writeErr != CE_None )
    {
      GDALClose( hMemDS );
      throw QgsProcessingException( QObject::tr( "Failed to write basin raster buffer to GDAL dataset." ) );
    }

    // create an in-memory OGR layer to hold the polygonized output features
    GDALDriverH hOgrMemDriver = GDALGetDriverByName( "Memory" );
    if ( !hOgrMemDriver )
    {
      GDALClose( hMemDS );
      throw QgsProcessingException( QObject::tr( "OGR Memory driver is unavailable." ) );
    }

    GDALDatasetH hOgrDS = GDALCreate( hOgrMemDriver, "", 0, 0, 0, GDT_Unknown, nullptr );
    OGRLayerH hLayer = GDALDatasetCreateLayer( hOgrDS, "basins", nullptr, wkbPolygon, nullptr );

    // create a "VALUE" field to receive the raster basin ID
    OGRFieldDefnH hFieldDefn = OGR_Fld_Create( "VALUE", OFTInteger );
    OGR_L_CreateField( hLayer, hFieldDefn, TRUE );
    OGR_Fld_Destroy( hFieldDefn );

    // handoff to GDALPolygonize to do the actual raster to vector logic

    struct GdalProgressData
    {
        QgsProcessingFeedback *feedback = nullptr;
    } progressData { &multiStepFeedback };

    auto gdalProgressCallback = []( double dfComplete, const char *, void *pProgressArg ) -> int CPL_STDCALL {
      if ( pProgressArg )
      {
        GdalProgressData *data = static_cast<GdalProgressData *>( pProgressArg );
        if ( data->feedback )
        {
          if ( data->feedback->isCanceled() )
            return FALSE;

          data->feedback->setProgress( dfComplete );
        }
      }
      return TRUE;
    };

    char **papszOptions = nullptr;
    papszOptions = CSLSetNameValue( papszOptions, "8CONNECTED", "8" );

    const CPLErr polyErr = GDALPolygonize( hBand, nullptr, hLayer, 0, papszOptions, gdalProgressCallback, &progressData );
    CSLDestroy( papszOptions );

    if ( polyErr != CE_None || feedback->isCanceled() )
    {
      GDALClose( hOgrDS );
      GDALClose( hMemDS );
      throw QgsProcessingException( QObject::tr( "GDAL Polygonize failed during basin vectorization." ) );
    }

    const GIntBig totalFeatures = OGR_L_GetFeatureCount( hLayer, TRUE );
    GIntBig featureIdx = 0;

    // get features from ogr memory layer
    OGR_L_ResetReading( hLayer );
    OGRFeatureH hFeat = nullptr;
    multiStepFeedback.setCurrentStep( 3 );
    while ( ( hFeat = OGR_L_GetNextFeature( hLayer ) ) != nullptr )
    {
      featureIdx++;
      if ( feedback->isCanceled() )
      {
        OGR_F_Destroy( hFeat );
        GDALClose( hOgrDS );
        GDALClose( hMemDS );
        return;
      }

      multiStepFeedback.setProgress( static_cast<double>( featureIdx ) / totalFeatures );

      const int basinId = OGR_F_GetFieldAsInteger( hFeat, 0 );
      // explicitly ignore no data pixels
      if ( basinId > 0 )
      {
        OGRGeometryH hGeom = OGR_F_GetGeometryRef( hFeat );
        if ( hGeom )
        {
          const QgsGeometry qGeom = QgsOgrUtils::ogrGeometryToQgsGeometry( hGeom );
          if ( !qGeom.isEmpty() )
          {
            const double area = qGeom.area();
            const double perimeter = qGeom.length();
            const int basinOrder = basinToOrderMap.value( basinId, 0 );

            QgsFeature feat;
            feat.setGeometry( qGeom );
            feat.setAttributes( QgsAttributes() << basinId << area << perimeter << basinOrder );
            basinSink->addFeature( feat, QgsFeatureSink::FastInsert );
          }
        }
      }
      OGR_F_Destroy( hFeat );
    }

    GDALClose( hOgrDS );
    GDALClose( hMemDS );
  }

  multiStepFeedback.setProgressText( QObject::tr( "Vectorizing channel lines" ) );
  multiStepFeedback.setCurrentStep( 4 );
  if ( channelSink )
  {
    int segmentCount = 0;
    for ( int row = 0; row < height; ++row )
    {
      if ( feedback->isCanceled() )
        return;

      multiStepFeedback.setProgress( static_cast<double>( row ) / height );

      for ( int column = 0; column < width; ++column )
      {
        const qgssize idx = static_cast<qgssize>( row ) * width + column;
        if ( nodesGrid[idx] > 0 )
        {
          int currentColumn = column;
          int currentRow = row;
          std::size_t currentIdx = idx;
          int dir = d8Directions[currentIdx];
          if ( dir >= 0 )
          {
            const int nodeA = nodesGrid[idx];
            const int basin = basinsGrid[idx];
            const int rawOrder = strahlerOrders[idx];
            const int shiftedOrder = rawOrder + 1 - threshold;

            auto line = std::make_unique<QgsLineString>();

            double xWorld;
            double yWorld;
            QgsRasterAnalysisUtils::pixelToMap( currentColumn, currentRow, extent, cellWidth, cellHeight, xWorld, yWorld );
            double z = demBlock->value( currentRow, currentColumn );
            line->addVertex( QgsPoint( xWorld, yWorld, z ) );

            int nodeB = 0;
            while ( dir >= 0 )
            {
              int neighborColumn = 0;
              int neighborRow = 0;
              if ( !QgsRasterAnalysisUtils::neighborCellCoordinates( dir, currentRow, currentColumn, neighborRow, neighborColumn, height, width ) )
                break;

              currentColumn = neighborColumn;
              currentRow = neighborRow;
              currentIdx = static_cast<qgssize>( currentRow ) * width + currentColumn;

              QgsRasterAnalysisUtils::pixelToMap( currentColumn, currentRow, extent, cellWidth, cellHeight, xWorld, yWorld );
              z = demBlock->value( currentRow, currentColumn );
              line->addVertex( QgsPoint( xWorld, yWorld, z ) );

              if ( nodesGrid[currentIdx] > 0 )
              {
                nodeB = nodesGrid[currentIdx];
                break;
              }

              dir = d8Directions[currentIdx];
            }

            QgsGeometry lineGeom( std::move( line ) );
            const double length = lineGeom.length();

            QgsFeature feat;
            feat.setGeometry( lineGeom );
            feat.setAttributes( QgsAttributes() << segmentCount++ << nodeA << nodeB << basin << shiftedOrder << rawOrder << length );
            if ( !channelSink->addFeature( feat, QgsFeatureSink::FastInsert ) )
            {
              throw QgsProcessingException( writeFeatureError( channelSink, parameters, QString() ) );
            }
            else
            {
              feedback->featureAddedToSink( u"CHANNELS"_s );
            }
          }
        }
      }
    }
  }
}

QgsFields QgsChannelNetworkAlgorithmBase::channelFields()
{
  QgsFields channelFields;
  channelFields.append( QgsField( u"SEGMENT_ID"_s, QMetaType::Type::Int ) );
  channelFields.append( QgsField( u"NODE_A"_s, QMetaType::Type::Int ) );
  channelFields.append( QgsField( u"NODE_B"_s, QMetaType::Type::Int ) );
  channelFields.append( QgsField( u"BASIN"_s, QMetaType::Type::Int ) );
  channelFields.append( QgsField( u"ORDER"_s, QMetaType::Type::Int ) );
  channelFields.append( QgsField( u"ORDER_CELL"_s, QMetaType::Type::Int ) );
  channelFields.append( QgsField( u"LENGTH"_s, QMetaType::Type::Double ) );
  return channelFields;
}

QgsFields QgsChannelNetworkAlgorithmBase::basinFields()
{
  QgsFields basinFields;
  basinFields.append( QgsField( u"VALUE"_s, QMetaType::Type::Int ) );
  basinFields.append( QgsField( u"AREA"_s, QMetaType::Type::Double ) );
  basinFields.append( QgsField( u"PERIMETER"_s, QMetaType::Type::Double ) );
  basinFields.append( QgsField( u"ORDER"_s, QMetaType::Type::Int ) );
  return basinFields;
}

QgsFields QgsChannelNetworkAlgorithmBase::junctionFields()
{
  QgsFields junctionFields;
  junctionFields.append( QgsField( u"ID"_s, QMetaType::Type::Int ) );
  junctionFields.append( QgsField( u"TYPE"_s, QMetaType::Type::QString ) );
  junctionFields.append( QgsField( u"ORDER"_s, QMetaType::Type::Int ) );
  junctionFields.append( QgsField( u"BASIN"_s, QMetaType::Type::Int ) );
  return junctionFields;
}

//
// QgsChannelNetworkFromDemAlgorithm
//

QString QgsChannelNetworkFromDemAlgorithm::name() const
{
  return u"channelnetworkfromdem"_s;
}

QString QgsChannelNetworkFromDemAlgorithm::displayName() const
{
  return QObject::tr( "Channel network and drainage basins from DEM" );
}

QString QgsChannelNetworkFromDemAlgorithm::shortDescription() const
{
  return QObject::tr( "Calculates channel network lines, drainage basin polygons, and junction nodes directly from an elevation raster (DEM)." );
}

QString QgsChannelNetworkFromDemAlgorithm::shortHelpString() const
{
  return QObject::tr(
    "This algorithm extracts vector channel network lines, drainage basin polygons, and topological junction node points directly from an elevation raster (DEM).\n\n"
    "The analysis executes a 3-step pipeline:\n"
    "1. D8 Flow Routing: Computes single-direction steepest descent flow directions.\n"
    "2. Strahler Stream Ordering: Calculates topological stream orders.\n"
    "3. Vector Network Extraction: Traces vector channels, delineates catchments, and identifies key topological junction nodes.\n\n"
    "The output Junctions layer contains topological nodes from the channel network. These are classified according to type:\n"
    "• Spring: Channel headwater initiation point matching the stream order threshold.\n"
    "• Junction: Tributary confluence point where two or more stream channels meet.\n"
    "• Outlet: Terminal discharge node exiting the raster boundary or draining into a terrain sink.\n"
    "• Mouth: Confluence pour point entering a higher-order stream segment (delineated when subbasins are enabled).\n\n"
    "This algorithm is a port of SAGA's 'Channel Network and Drainage Basins' tool."
  );
}

void QgsChannelNetworkFromDemAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( u"INPUT"_s, QObject::tr( "Elevation raster" ) ) );
  addCommonParameters();
}

QgsProcessingAlgorithm *QgsChannelNetworkFromDemAlgorithm::createInstance() const
{
  return new QgsChannelNetworkFromDemAlgorithm();
}

bool QgsChannelNetworkFromDemAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsRasterLayer *layer = parameterAsRasterLayer( parameters, u"INPUT"_s, context );
  if ( !layer || !layer->dataProvider() )
    throw QgsProcessingException( invalidRasterError( parameters, u"INPUT"_s ) );

  mDemInterface.reset( layer->dataProvider()->clone() );
  mLayerWidth = layer->width();
  mLayerHeight = layer->height();
  mExtent = layer->extent();
  mCrs = layer->crs();
  mCellSizeX = layer->rasterUnitsPerPixelX();
  mCellSizeY = layer->rasterUnitsPerPixelY();

  return true;
}

QVariantMap QgsChannelNetworkFromDemAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QGS_MARK_ALGORITHM_SOURCE

  const int threshold = parameterAsInt( parameters, u"THRESHOLD"_s, context );
  const bool subbasins = parameterAsBool( parameters, u"SUBBASINS"_s, context );

  std::unique_ptr<QgsRasterBlock> demBlock( mDemInterface->block( 1, mExtent, mLayerWidth, mLayerHeight ) );
  if ( !demBlock )
    throw QgsProcessingException( QObject::tr( "Could not read input DEM block." ) );

  const qgssize totalCells = static_cast<qgssize>( mLayerWidth ) * mLayerHeight;
  QgsProcessingMultiStepFeedback multiStepFeedback( 3, feedback );
  multiStepFeedback.setStepWeights( { 1, 1, 10 } );

  multiStepFeedback.setCurrentStep( 0 );
  multiStepFeedback.setProgressText( QObject::tr( "Calculating flow direction" ) );
  std::vector<int8_t> d8Directions( totalCells, -1 );
  for ( int row = 0; row < mLayerHeight; ++row )
  {
    if ( multiStepFeedback.isCanceled() )
      return {};

    multiStepFeedback.setProgress( 100.0 * static_cast<double>( row ) / mLayerHeight );
    const qgssize rowOffset = static_cast<qgssize>( row ) * mLayerWidth;
    for ( int col = 0; col < mLayerWidth; ++col )
    {
      const int dir = QgsRasterAnalysisUtils::steepestGradientDirection( demBlock.get(), row, col, mCellSizeX, mCellSizeY, true, true );
      d8Directions[rowOffset + col] = static_cast<int8_t>( dir );
    }
  }

  multiStepFeedback.setProgressText( QObject::tr( "Calculating Strahler stream orders" ) );
  multiStepFeedback.setCurrentStep( 1 );
  std::vector<int16_t> strahlerOrders( totalCells, 0 );
  computeStrahlerOrder( demBlock.get(), d8Directions, mLayerWidth, mLayerHeight, 1, strahlerOrders.data(), &multiStepFeedback, -1 );
  if ( multiStepFeedback.isCanceled() )
    return {};

  multiStepFeedback.setCurrentStep( 2 );

  QString channelsDest;
  std::unique_ptr<QgsFeatureSink> channelSink;
  if ( !QgsVariantUtils::isNull( parameters.value( u"CHANNELS"_s ) ) )
  {
    channelSink.reset( parameterAsSink( parameters, u"CHANNELS"_s, context, channelsDest, channelFields(), Qgis::WkbType::LineStringZ, mCrs ) );
    if ( !channelSink )
    {
      throw QgsProcessingException( invalidSinkError( parameters, u"CHANNELS"_s ) );
    }
  }

  QString basinsDest;
  std::unique_ptr<QgsFeatureSink> basinSink;
  if ( !QgsVariantUtils::isNull( parameters.value( u"BASINS"_s ) ) )
  {
    basinSink.reset( parameterAsSink( parameters, u"BASINS"_s, context, basinsDest, basinFields(), Qgis::WkbType::Polygon, mCrs ) );
    if ( !basinSink )
    {
      throw QgsProcessingException( invalidSinkError( parameters, u"BASINS"_s ) );
    }
  }

  QString junctionsDest;
  std::unique_ptr<QgsFeatureSink> junctionSink;
  if ( !QgsVariantUtils::isNull( parameters.value( u"JUNCTIONS"_s ) ) )
  {
    junctionSink.reset( parameterAsSink( parameters, u"JUNCTIONS"_s, context, junctionsDest, junctionFields(), Qgis::WkbType::PointZ, mCrs ) );
    if ( !junctionSink )
    {
      throw QgsProcessingException( invalidSinkError( parameters, u"JUNCTIONS"_s ) );
    }
  }

  extractChannelNetwork( demBlock.get(), d8Directions, strahlerOrders, mLayerWidth, mLayerHeight, mExtent, mCrs, threshold, subbasins, channelSink.get(), basinSink.get(), junctionSink.get(), &multiStepFeedback, parameters );

  if ( channelSink )
  {
    channelSink->finalize();
    feedback->featureSinkFinalized( u"CHANNELS"_s );
  }
  if ( basinSink )
  {
    basinSink->finalize();
    feedback->featureSinkFinalized( u"BASINS"_s );
  }
  if ( junctionSink )
  {
    junctionSink->finalize();
    feedback->featureSinkFinalized( u"JUNCTIONS"_s );
  }

  QVariantMap outputs;
  if ( channelSink )
    outputs.insert( u"CHANNELS"_s, channelsDest );
  if ( basinSink )
    outputs.insert( u"BASINS"_s, basinsDest );
  if ( junctionSink )
    outputs.insert( u"JUNCTIONS"_s, junctionsDest );

  return outputs;
}

//
// QgsChannelNetworkFromFlowDirAndOrderAlgorithm
//
QString QgsChannelNetworkFromFlowDirAndOrderAlgorithm::name() const
{
  return u"channelnetworkfromflowdirandorder"_s;
}

QString QgsChannelNetworkFromFlowDirAndOrderAlgorithm::displayName() const
{
  return QObject::tr( "Channel network and drainage basins from multiple inputs" );
}

QString QgsChannelNetworkFromFlowDirAndOrderAlgorithm::shortDescription() const
{
  return QObject::tr( "Calculates channel network lines, drainage basin polygons, and junction nodes using DEM, flow direction, and Strahler order rasters." );
}

QString QgsChannelNetworkFromFlowDirAndOrderAlgorithm::shortHelpString() const
{
  return QObject::tr(
    "This algorithm extracts vector channel network lines, drainage basin polygons, and topological junction node points using pre-computed elevation (DEM), D8 flow direction, and Strahler stream "
    "order rasters.\n\n"
    "This variant bypasses internal raster flow routing and stream order generation, making it ideal when flow direction and Strahler order rasters have already been computed in prior processing "
    "steps.\n\n"
    "The output Junctions layer contains topological nodes from the channel network. These are classified according to type:\n"
    "• Spring: Channel headwater initiation point matching the stream order threshold.\n"
    "• Junction: Tributary confluence point where two or more stream channels meet.\n"
    "• Outlet: Terminal discharge node exiting the raster boundary or draining into a terrain sink.\n"
    "• Mouth: Confluence pour point entering a higher-order stream segment (delineated when subbasins are enabled).\n\n"
    "This algorithm is a port of SAGA's 'Channel Network and Drainage Basins' tool."
  );
}

void QgsChannelNetworkFromFlowDirAndOrderAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( u"INPUT_DEM"_s, QObject::tr( "Elevation raster" ) ) );
  addParameter( new QgsProcessingParameterRasterLayer( u"INPUT_FLOW_DIR"_s, QObject::tr( "Flow direction raster" ) ) );
  addParameter( new QgsProcessingParameterRasterLayer( u"INPUT_STRAHLER"_s, QObject::tr( "Strahler order raster" ) ) );
  addCommonParameters();
}

QgsProcessingAlgorithm *QgsChannelNetworkFromFlowDirAndOrderAlgorithm::createInstance() const
{
  return new QgsChannelNetworkFromFlowDirAndOrderAlgorithm();
}

bool QgsChannelNetworkFromFlowDirAndOrderAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsRasterLayer *demLayer = parameterAsRasterLayer( parameters, u"INPUT_DEM"_s, context );
  if ( !demLayer || !demLayer->dataProvider() )
    throw QgsProcessingException( invalidRasterError( parameters, u"INPUT_DEM"_s ) );

  QgsRasterLayer *flowDirLayer = parameterAsRasterLayer( parameters, u"INPUT_FLOW_DIR"_s, context );
  if ( !flowDirLayer || !flowDirLayer->dataProvider() )
    throw QgsProcessingException( invalidRasterError( parameters, u"INPUT_FLOW_DIR"_s ) );

  QgsRasterLayer *strahlerLayer = parameterAsRasterLayer( parameters, u"INPUT_STRAHLER"_s, context );
  if ( !strahlerLayer || !strahlerLayer->dataProvider() )
    throw QgsProcessingException( invalidRasterError( parameters, u"INPUT_STRAHLER"_s ) );

  mDemInterface.reset( demLayer->dataProvider()->clone() );
  mFlowDirInterface.reset( flowDirLayer->dataProvider()->clone() );
  mStrahlerInterface.reset( strahlerLayer->dataProvider()->clone() );
  mLayerWidth = demLayer->width();
  mLayerHeight = demLayer->height();
  mExtent = demLayer->extent();
  mCrs = demLayer->crs();

  return true;
}

QVariantMap QgsChannelNetworkFromFlowDirAndOrderAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QGS_MARK_ALGORITHM_SOURCE

  const int threshold = parameterAsInt( parameters, u"THRESHOLD"_s, context );
  const bool subbasins = parameterAsBool( parameters, u"SUBBASINS"_s, context );

  std::unique_ptr<QgsRasterBlock> demBlock( mDemInterface->block( 1, mExtent, mLayerWidth, mLayerHeight ) );
  if ( !demBlock )
    throw QgsProcessingException( QObject::tr( "Could not read input DEM block." ) );

  std::unique_ptr<QgsRasterBlock> flowDirBlock( mFlowDirInterface->block( 1, mExtent, mLayerWidth, mLayerHeight ) );
  if ( !flowDirBlock )
    throw QgsProcessingException( QObject::tr( "Could not read input flow direction block." ) );

  std::unique_ptr<QgsRasterBlock> strahlerBlock( mStrahlerInterface->block( 1, mExtent, mLayerWidth, mLayerHeight ) );
  if ( !strahlerBlock )
    throw QgsProcessingException( QObject::tr( "Could not read input Strahler order block." ) );

  const qgssize totalCells = static_cast<qgssize>( mLayerWidth ) * mLayerHeight;

  std::vector<int8_t> d8Directions( totalCells, -1 );
  std::vector<int16_t> strahlerOrders( totalCells, 0 );

  for ( int row = 0; row < mLayerHeight; ++row )
  {
    if ( feedback->isCanceled() )
      return {};

    const qgssize rowOffset = static_cast<qgssize>( row ) * mLayerWidth;
    for ( int col = 0; col < mLayerWidth; ++col )
    {
      const std::size_t idx = rowOffset + col;
      if ( !flowDirBlock->isNoData( row, col ) )
      {
        d8Directions[idx] = static_cast<int8_t>( flowDirBlock->value( row, col ) );
      }
      if ( !strahlerBlock->isNoData( row, col ) )
      {
        strahlerOrders[idx] = static_cast<int16_t>( strahlerBlock->value( row, col ) );
      }
    }
  }

  QString channelsDest;
  std::unique_ptr<QgsFeatureSink> channelSink;
  if ( !QgsVariantUtils::isNull( parameters.value( u"CHANNELS"_s ) ) )
  {
    channelSink.reset( parameterAsSink( parameters, u"CHANNELS"_s, context, channelsDest, channelFields(), Qgis::WkbType::LineStringZ, mCrs ) );
    if ( !channelSink )
    {
      throw QgsProcessingException( invalidSinkError( parameters, u"CHANNELS"_s ) );
    }
  }

  QString basinsDest;
  std::unique_ptr<QgsFeatureSink> basinSink;
  if ( !QgsVariantUtils::isNull( parameters.value( u"BASINS"_s ) ) )
  {
    basinSink.reset( parameterAsSink( parameters, u"BASINS"_s, context, basinsDest, basinFields(), Qgis::WkbType::Polygon, mCrs ) );
    if ( !basinSink )
    {
      throw QgsProcessingException( invalidSinkError( parameters, u"BASINS"_s ) );
    }
  }

  QString junctionsDest;
  std::unique_ptr<QgsFeatureSink> junctionSink;
  if ( !QgsVariantUtils::isNull( parameters.value( u"JUNCTIONS"_s ) ) )
  {
    junctionSink.reset( parameterAsSink( parameters, u"JUNCTIONS"_s, context, junctionsDest, junctionFields(), Qgis::WkbType::PointZ, mCrs ) );
    if ( !junctionSink )
    {
      throw QgsProcessingException( invalidSinkError( parameters, u"JUNCTIONS"_s ) );
    }
  }

  extractChannelNetwork( demBlock.get(), d8Directions, strahlerOrders, mLayerWidth, mLayerHeight, mExtent, mCrs, threshold, subbasins, channelSink.get(), basinSink.get(), junctionSink.get(), feedback, parameters );

  if ( channelSink )
  {
    channelSink->finalize();
    feedback->featureSinkFinalized( u"CHANNELS"_s );
  }
  if ( basinSink )
  {
    basinSink->finalize();
    feedback->featureSinkFinalized( u"BASINS"_s );
  }
  if ( junctionSink )
  {
    junctionSink->finalize();
    feedback->featureSinkFinalized( u"JUNCTIONS"_s );
  }

  QVariantMap outputs;
  if ( channelSink )
    outputs.insert( u"CHANNELS"_s, channelsDest );
  if ( basinSink )
    outputs.insert( u"BASINS"_s, basinsDest );
  if ( junctionSink )
    outputs.insert( u"JUNCTIONS"_s, junctionsDest );

  return outputs;
}

///@endcond
