/***************************************************************************
                         qgsalgorithmupslopearea.cpp
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

#include "qgsalgorithmupslopearea.h"

#include "qgsacademicreference.h"
#include "qgsrasteranalysisutils.h"
#include "qgsrasterfilewriter.h"
#include "qgssortedrasterblockindex.h"
#include "qgsvariantutils.h"

#include <QString>

using namespace Qt::StringLiterals;

///@cond PRIVATE

// SAGA 8-neighbor directions: 0: N, 1: NE, 2: E, 3: SE, 4: S, 5: SW, 6: W, 7: NW
// SAGA Cartesian directions (SAGA_X points East, SAGA_Y points North):
static constexpr std::array<int, 8> SAGA_X { 0, 1, 1, 1, 0, -1, -1, -1 };
static constexpr std::array<int, 8> SAGA_Y { 1, 1, 0, -1, -1, -1, 0, 1 };

QString QgsUpslopeAreaAlgorithmBase::group() const
{
  return QObject::tr( "Raster terrain analysis" );
}

QString QgsUpslopeAreaAlgorithmBase::groupId() const
{
  return u"rasterterrainanalysis"_s;
}

QStringList QgsUpslopeAreaAlgorithmBase::tags() const
{
  return QObject::tr( "upslope,area,flow,accumulation,hydrology,catchment,watershed,target" ).split( ',' );
}

QList<QgsAcademicReference> QgsUpslopeAreaAlgorithmBase::academicReferences() const
{
  const QgsAcademicReference freemanReference = QgsAcademicReference::
    createJournalArticle( { u"Freeman, G. T."_s }, 1991, u"Calculating catchment area with divergent flow based on a regular grid"_s, u"Computers and Geosciences"_s, u"17"_s, QString(), u"413-422"_s );

  const QgsAcademicReference ocallaghanReference = QgsAcademicReference::
    createJournalArticle( { u"O'Callaghan, J. F."_s, u"Mark, D. M."_s }, 1984, u"The extraction of drainage networks from digital elevation data"_s, u"Computer Vision, Graphics and Image Processing"_s, u"28"_s, QString(), u"323-344"_s );

  const QgsAcademicReference qinReference = QgsAcademicReference::createJournalArticle(
    { u"Qin, C. Z."_s, u"Zhu, A. X."_s, u"Pei, T."_s, u"Li, B. L."_s, u"Scholten, T."_s, u"Behrens, T."_s, u"Zhou, C. H."_s },
    2011,
    u"An approach to computing topographic wetness index based on maximum downslope gradient"_s,
    u"Precision Agriculture"_s,
    u"12"_s,
    u"1"_s,
    u"32-43"_s
  );

  const QgsAcademicReference quinnReference = QgsAcademicReference::createJournalArticle(
    { u"Quinn, P. F."_s, u"Beven, K. J."_s, u"Chevallier, P."_s, u"Planchon, O."_s },
    1991,
    u"The prediction of hillslope flow paths for distributed hydrological modelling using digital terrain models"_s,
    u"Hydrological Processes"_s,
    u"5"_s,
    QString(),
    u"59-79"_s
  );

  const QgsAcademicReference seibertReference = QgsAcademicReference::
    createJournalArticle( { u"Seibert, J."_s, u"McGlynn, B."_s }, 2007, u"A new triangular multiple flow direction algorithm for computing upslope areas from gridded digital elevation models"_s, u"Water Resources Research"_s, u"43"_s, QString(), u"W04501"_s );

  const QgsAcademicReference tarbotonReference = QgsAcademicReference::
    createJournalArticle( { u"Tarboton, D. G."_s }, 1997, u"A new method for the determination of flow directions and upslope areas in grid digital elevation models"_s, u"Water Resources Research"_s, u"33"_s, u"2"_s, u"309-319"_s );

  return { freemanReference, ocallaghanReference, qinReference, quinnReference, seibertReference, tarbotonReference };
}

QList<QgsProcessingAlgorithm::ExternalLink> QgsUpslopeAreaAlgorithmBase::externalLinks() const
{
  return {
    QgsProcessingAlgorithm::ExternalLink { QObject::tr( "SAGA tool source code" ), u"https://sourceforge.net/p/saga-gis/code/ci/0df4dbfc4122ea022d3d1ca69dce6df882c9e081/tree/saga-gis/src/tools/terrain_analysis/ta_hydrology/Flow_AreaUpslope.cpp"_s }
  };
}

void QgsUpslopeAreaAlgorithmBase::addCommonParameters()
{
  auto demParam = std::make_unique<QgsProcessingParameterRasterLayer>( u"ELEVATION"_s, QObject::tr( "Elevation" ) );
  demParam->setHelp( QObject::tr( "Input digital elevation model (DEM) raster layer." ) );
  addParameter( demParam.release() );

  auto routeParam = std::make_unique<QgsProcessingParameterRasterLayer>( u"SINK_ROUTES"_s, QObject::tr( "Sink routes" ), QVariant(), true );
  routeParam->setHelp( QObject::tr( "Optional raster layer specifying explicit flow routes through sinks/depressions." ) );
  addParameter( routeParam.release() );

  const QStringList methods
    = { QObject::tr( "Deterministic 8" ), QObject::tr( "Deterministic Infinity" ), QObject::tr( "Multiple Flow Direction" ), QObject::tr( "Multiple Triangular Flow Direction" ), QObject::tr( "Multiple Maximum Downslope Gradient Based Flow Direction" ) };
  auto methodParam = std::make_unique<QgsProcessingParameterEnum>( u"METHOD"_s, QObject::tr( "Method" ), methods, false, 2 );
  methodParam->setHelp( QObject::tr( "Flow routing algorithm used to determine flow distribution to downslope cells." ) );
  addParameter( methodParam.release() );

  auto convergeParam = std::make_unique<QgsProcessingParameterNumber>( u"CONVERGE"_s, QObject::tr( "Convergence" ), Qgis::ProcessingNumberParameterType::Double, 1.1, false, 0.001 );
  convergeParam->setHelp( QObject::tr( "Convergence factor for Multiple Flow Direction algorithms." ) );
  addParameter( convergeParam.release() );

  auto contourParam = std::make_unique<QgsProcessingParameterBoolean>( u"MFD_CONTOUR"_s, QObject::tr( "Use contour length weighting" ), false );
  contourParam->setHelp(
    QObject::tr( "Include pseudo contour length weighting factor in multiple flow routing. Reduces flow to diagonal neighbour cells by a factor of 0.71 (see Quinn et al. 1991 for details)." )
  );
  addParameter( contourParam.release() );

  auto outputNodataParam = std::make_unique<QgsProcessingParameterNumber>( u"NODATA"_s, QObject::tr( "Output NoData value" ), Qgis::ProcessingNumberParameterType::Integer, -9999 );
  outputNodataParam->setHelp( QObject::tr( "The NODATA value to use in the output raster." ) );
  outputNodataParam->setFlags( outputNodataParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( outputNodataParam.release() );

  auto creationOptsParam = std::make_unique<QgsProcessingParameterString>( u"CREATION_OPTIONS"_s, QObject::tr( "Creation options" ), QVariant(), false, true );
  creationOptsParam->setHelp( QObject::tr( "The raster creation options for the output raster. These options control things like colorimetry, compression, etc." ) );
  creationOptsParam->setMetadata( QVariantMap( { { u"widget_wrapper"_s, QVariantMap( { { u"widget_type"_s, u"rasteroptions"_s } } ) } } ) );
  creationOptsParam->setFlags( creationOptsParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( creationOptsParam.release() );

  auto outputParam = std::make_unique<QgsProcessingParameterRasterDestination>( u"OUTPUT"_s, QObject::tr( "Upslope area" ) );
  addParameter( outputParam.release() );
}

bool QgsUpslopeAreaAlgorithmBase::prepareBase( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsRasterLayer *demLayer = parameterAsRasterLayer( parameters, u"ELEVATION"_s, context );
  if ( !demLayer || !demLayer->dataProvider() )
    throw QgsProcessingException( invalidRasterError( parameters, u"ELEVATION"_s ) );

  mDemProvider.reset( demLayer->dataProvider()->clone() );
  mDemCrs = demLayer->crs();
  mExtent = demLayer->extent();
  mCols = demLayer->width();
  mRows = demLayer->height();
  mCellSizeX = demLayer->rasterUnitsPerPixelX();
  mCellSizeY = demLayer->rasterUnitsPerPixelY();

  if ( demLayer->dataProvider()->sourceHasNoDataValue( 1 ) )
    mDemNoData = demLayer->dataProvider()->sourceNoDataValue( 1 );

  QgsRasterLayer *routeLayer = parameterAsRasterLayer( parameters, u"SINK_ROUTES"_s, context );
  if ( routeLayer && routeLayer->dataProvider() )
  {
    mRouteProvider.reset( routeLayer->dataProvider()->clone() );
    if ( routeLayer->dataProvider()->sourceHasNoDataValue( 1 ) )
      mRouteNoData = routeLayer->dataProvider()->sourceNoDataValue( 1 );
  }
  else if ( !QgsVariantUtils::isNull( parameters.value( u"SINK_ROUTES"_s ) ) )
  {
    throw QgsProcessingException( invalidRasterError( parameters, u"SINK_ROUTES"_s ) );
  }

  mMethod = static_cast<Method>( parameterAsInt( parameters, u"METHOD"_s, context ) );
  mConvergence = parameterAsDouble( parameters, u"CONVERGE"_s, context );
  mMfdContour = parameterAsBool( parameters, u"MFD_CONTOUR"_s, context );

  mCreationOptions = parameterAsString( parameters, u"CREATION_OPTIONS"_s, context ).trimmed();
  mOutputNoData = parameterAsDouble( parameters, u"NODATA"_s, context );
  return true;
}

bool QgsUpslopeAreaAlgorithmBase::calculateUpslopeArea( const std::vector<QgsPointXY> &targetPoints, QgsProcessingContext &, QgsProcessingFeedback *feedback )
{
  const qgssize nCells = static_cast<qgssize>( mCols ) * mRows;

  std::unique_ptr<QgsRasterBlock> demBlock( mDemProvider->block( 1, mExtent, mCols, mRows ) );
  if ( !demBlock )
    throw QgsProcessingException( QObject::tr( "Could not read DEM raster block." ) );

  // load (optional) sink routes
  if ( mRouteProvider )
  {
    mRouteData.resize( nCells, -1.0 );
    std::unique_ptr<QgsRasterBlock> routeBlock( mRouteProvider->block( 1, mExtent, mCols, mRows ) );
    if ( routeBlock )
    {
      for ( int r = 0; r < mRows; ++r )
      {
        for ( int c = 0; c < mCols; ++c )
        {
          mRouteData[static_cast<qgssize>( r ) * mCols + c] = routeBlock->value( r, c );
        }
      }
    }
  }

  mFlowData.assign( nCells, 0.0 );

  // map input target points to raster cell coordinates and set initial target flow to 100.0
  // (matches SAGA's CFlow_AreaUpslope::Add_Target)
  bool hasValidTarget = false;
  int column = 0;
  int row = 0;
  for ( const QgsPointXY &pt : targetPoints )
  {
    QgsRasterAnalysisUtils::mapToPixel( pt.x(), pt.y(), mExtent, mCellSizeX, mCellSizeY, column, row );
    if ( column >= 0 && column < mCols && row >= 0 && row < mRows )
    {
      mFlowData[static_cast<qgssize>( row ) * mCols + column] = 100.0;
      hasValidTarget = true;
    }
  }

  if ( !hasValidTarget )
  {
    feedback->reportError( QObject::tr( "All target point(s) lie outside the DEM extent." ) );
    return false;
  }

  // process cells in ascending topological order (lowest to highest elevation)
  // (matching SAGA's CFlow_AreaUpslope::Get_Area)
  const QgsSortedRasterBlockIndex sortedIndex( demBlock.get() );
  const qgssize count = sortedIndex.sortedCount();
  for ( qgssize i = 0; i < count; ++i )
  {
    if ( feedback->isCanceled() )
      return false;
    feedback->setProgress( 100.0 * static_cast<double>( i ) / count );
    sortedIndex.sortedColumnRow( i, column, row, Qt::AscendingOrder );

    // calculate cell flow if not already set by target initialization

    if ( mFlowData[static_cast<qgssize>( row ) * mCols + column] <= 0.0 )
    {
      computeCellValue( demBlock.get(), column, row, mCols, mRows, mCellSizeX, mCellSizeY, mMethod, mConvergence, mMfdContour );
    }
  }

  if ( feedback->isCanceled() )
    return false;

  auto writer = std::make_unique<QgsRasterFileWriter>( mOutputPath );
  writer->setOutputProviderKey( u"gdal"_s );
  if ( !mCreationOptions.isEmpty() )
  {
    writer->setCreationOptions( mCreationOptions.split( '|' ) );
  }
  writer->setOutputFormat( mOutputFormat );

  std::unique_ptr<QgsRasterDataProvider> provider( writer->createOneBandRaster( Qgis::DataType::Float32, mCols, mRows, mExtent, mDemCrs ) );
  if ( !provider )
    throw QgsProcessingException( QObject::tr( "Could not create raster output: %1" ).arg( mOutputPath ) );
  if ( !provider->isValid() )
    throw QgsProcessingException( QObject::tr( "Could not create raster output %1: %2" ).arg( mOutputPath, provider->error().message( QgsErrorMessage::Text ) ) );

  provider->setNoDataValue( 1, mOutputNoData );
  provider->setEditable( true );

  QgsRasterBlock outputBlock( Qgis::DataType::Float32, mCols, mRows );
  for ( int r = 0; r < mRows; ++r )
  {
    for ( int c = 0; c < mCols; ++c )
    {
      outputBlock.setValue( r, c, static_cast<float>( mFlowData[static_cast<qgssize>( r ) * mCols + c] ) );
    }
  }

  if ( !provider->writeBlock( &outputBlock, 1 ) )
  {
    throw QgsProcessingException( QObject::tr( "Could not write raster block: %1" ).arg( provider->error().summary() ) );
  }

  provider->setEditable( false );
  return true;
}

void QgsUpslopeAreaAlgorithmBase::computeCellValue( const QgsRasterBlock *demBlock, int col, int row, int cols, int rows, double cellSizeX, double cellSizeY, Method method, double converge, bool contour )
{
  const qgssize idx = static_cast<qgssize>( row ) * cols + col;

  // check explicit sink route if available (see SAGA's CFlow_AreaUpslope::Set_Value)
  if ( !mRouteData.empty() )
  {
    const int routeDir = static_cast<int>( mRouteData[idx] );
    if ( routeDir >= 0 && routeDir < 8 )
    {
      int neighborCol = 0;
      int neighborRow = 0;
      if ( QgsRasterAnalysisUtils::neighborCellCoordinates( routeDir, row, col, neighborRow, neighborCol, row, cols ) )
      {
        const double routedFlow = mFlowData[static_cast<qgssize>( neighborRow ) * cols + neighborCol];
        if ( routedFlow > 0.0 )
        {
          mFlowData[idx] = routedFlow;
        }
      }
      return;
    }
  }

  switch ( method )
  {
    case Method::D8:
      computeD8( demBlock, col, row, cols, rows, cellSizeX, cellSizeY );
      break;
    case Method::DInf:
      computeDInf( demBlock, col, row, cols, rows, cellSizeX, cellSizeY );
      break;
    case Method::MFD:
      computeMFD( demBlock, col, row, cols, rows, cellSizeX, cellSizeY, converge, contour );
      break;
    case Method::MDInf:
      computeMDInf( demBlock, col, row, cols, rows, cellSizeX, cellSizeY, converge );
      break;
    case Method::MMDGFD:
      computeMMDGFD( demBlock, col, row, cols, rows, cellSizeX, cellSizeY, contour );
      break;
  }
}

void QgsUpslopeAreaAlgorithmBase::computeD8( const QgsRasterBlock *demBlock, int col, int row, int cols, int rows, double cellSizeX, double cellSizeY )
{
  const int steepestDir = QgsRasterAnalysisUtils::steepestGradientDirection( demBlock, row, col, cellSizeX, cellSizeY );

  if ( steepestDir >= 0 )
  {
    int neighborCol = 0;
    int neighborRow = 0;
    QgsRasterAnalysisUtils::neighborCellCoordinates( steepestDir, row, col, neighborRow, neighborCol, rows, cols );
    const double neighborFlow = mFlowData[static_cast<qgssize>( neighborRow ) * cols + neighborCol];
    if ( neighborFlow > 0.0 )
    {
      mFlowData[static_cast<qgssize>( row ) * cols + col] = neighborFlow;
    }
  }
}

void QgsUpslopeAreaAlgorithmBase::computeDInf( const QgsRasterBlock *demBlock, int col, int row, int cols, int rows, double cellSizeX, double cellSizeY )
{
  bool isNoData = false;
  const double z = demBlock->valueAndNoData( row, col, isNoData );
  if ( isNoData )
  {
    // follow SAGA -- fallback to D8
    computeD8( demBlock, col, row, cols, rows, cellSizeX, cellSizeY );
    return;
  }

  // Following SAGA's CSG_Grid::Get_Gradient (which follows Zevenbergen & Thorne 1986)
  double dz[4] = { 0.0, 0.0, 0.0, 0.0 };
  const std::array<int, 4> dirs { 0, 2, 4, 6 };
  for ( int i = 0; i < 4; ++i )
  {
    const int iDir = dirs[i];
    const int oppositeDir = ( iDir + 4 ) % 8;

    int neighborCol = 0;
    int neighborRow = 0;
    int oppositeNeighborCol = 0;
    int oppositeNeighborRow = 0;
    bool neighborIsNoData = true;
    double neighborZ = 0;
    if ( QgsRasterAnalysisUtils::neighborCellCoordinates( iDir, row, col, neighborRow, neighborCol, rows, cols ) )
    {
      neighborZ = demBlock->valueAndNoData( neighborRow, neighborCol, neighborIsNoData );
    }
    bool oppositeIsNoData = true;
    double oppositeNeighborZ = 0;
    if ( QgsRasterAnalysisUtils::neighborCellCoordinates( oppositeDir, row, col, oppositeNeighborRow, oppositeNeighborCol, rows, cols ) )
    {
      oppositeNeighborZ = demBlock->valueAndNoData( oppositeNeighborRow, oppositeNeighborCol, oppositeIsNoData );
    }

    if ( !neighborIsNoData )
    {
      dz[i] = neighborZ - z;
    }
    else if ( !oppositeIsNoData )
    {
      dz[i] = z - oppositeNeighborZ;
    }
    else
    {
      dz[i] = 0.0;
    }
  }

  const double G = ( dz[0] - dz[2] ) / ( 2.0 * cellSizeY );
  const double H = ( dz[1] - dz[3] ) / ( 2.0 * cellSizeX );

  double aspect = -1.0;
  if ( G != 0.0 )
  {
    aspect = M_PI + std::atan2( H, G );
    if ( aspect < 0.0 )
      aspect += 2.0 * M_PI;
    if ( aspect >= 2.0 * M_PI )
      aspect -= 2.0 * M_PI;
  }
  else if ( H > 0.0 )
  {
    aspect = 1.5 * M_PI;
  }
  else if ( H < 0.0 )
  {
    aspect = 0.5 * M_PI;
  }

  if ( aspect >= 0.0 )
  {
    const int i = static_cast<int>( aspect / ( M_PI / 4.0 ) ) % 8;
    const int j = ( i + 1 ) % 8;

    int iCol = 0;
    int iRow = 0;
    int jCol = 0;
    int jRow = 0;
    if ( QgsRasterAnalysisUtils::neighborCellCoordinates( i, row, col, iRow, iCol, rows, cols ) && QgsRasterAnalysisUtils::neighborCellCoordinates( j, row, col, jRow, jCol, rows, cols ) )
    {
      bool iIsNoData = false;
      const double zi = demBlock->valueAndNoData( iRow, iCol, iIsNoData );
      bool jIsNoData = false;
      const double zj = demBlock->valueAndNoData( jRow, jCol, jIsNoData );
      if ( !iIsNoData && !jIsNoData )
      {
        // both sector neighbors must be lower in elevation
        if ( zi < z && zj < z )
        {
          const double aspectFraction = std::fmod( aspect, M_PI / 4.0 ) / ( M_PI / 4.0 );
          const double flowI = mFlowData[static_cast<qgssize>( iRow ) * cols + iCol];
          const double flowJ = mFlowData[static_cast<qgssize>( jRow ) * cols + jCol];

          const double accumulatedFlow = flowI * ( 1.0 - aspectFraction ) + flowJ * aspectFraction;
          if ( accumulatedFlow > 0.0 )
          {
            mFlowData[static_cast<qgssize>( row ) * cols + col] = accumulatedFlow;
          }
          return;
        }
      }
    }
  }

  computeD8( demBlock, col, row, cols, rows, cellSizeX, cellSizeY );
}

void QgsUpslopeAreaAlgorithmBase::computeMFD( const QgsRasterBlock *demBlock, int col, int row, int cols, int rows, double cellSizeX, double cellSizeY, double converge, bool contour )
{
  const double z = demBlock->value( row, col );
  double dz[8];
  double dzSum = 0.0;

  bool isNodata = false;
  for ( int dir = 0; dir < 8; ++dir )
  {
    dz[dir] = 0.0;
    int neighborCol = 0;
    int neighborRow = 0;
    if ( QgsRasterAnalysisUtils::neighborCellCoordinates( dir, row, col, neighborRow, neighborCol, rows, cols ) )
    {
      const double nZ = demBlock->valueAndNoData( neighborRow, neighborCol, isNodata );
      if ( !isNodata )
      {
        const double diff = z - nZ;
        if ( diff > 0.0 )
        {
          const double length = QgsRasterAnalysisUtils::neighborCellDistance( dir, cellSizeX, cellSizeY );
          const double weight = std::pow( diff / length, converge ) * ( ( contour && ( dir % 2 ) ) ? ( M_SQRT1_2 ) : 1.0 );
          dz[dir] = weight;
          dzSum += weight;
        }
      }
    }
  }

  if ( dzSum > 0.0 )
  {
    double flow = 0.0;
    for ( int dir = 0; dir < 8; ++dir )
    {
      if ( dz[dir] > 0.0 )
      {
        int nCol = 0;
        int nRow = 0;
        QgsRasterAnalysisUtils::neighborCellCoordinates( dir, row, col, nRow, nCol, rows, cols );
        const double nFlow = mFlowData[static_cast<qgssize>( nRow ) * cols + nCol];
        if ( nFlow > 0.0 )
        {
          flow += ( dz[dir] / dzSum ) * nFlow;
        }
      }
    }

    if ( flow > 0.0 )
    {
      mFlowData[static_cast<qgssize>( row ) * cols + col] = flow;
    }
  }
}

void QgsUpslopeAreaAlgorithmBase::computeMMDGFD( const QgsRasterBlock *demBlock, int col, int row, int cols, int rows, double cellSizeX, double cellSizeY, bool contour )
{
  const double z = demBlock->value( row, col );
  double dz[8];
  double dzMax = 0.0;

  bool isNodata = false;
  for ( int dir = 0; dir < 8; ++dir )
  {
    dz[dir] = 0.0;
    int neighborCol = 0;
    int neighborRow = 0;
    if ( QgsRasterAnalysisUtils::neighborCellCoordinates( dir, row, col, neighborRow, neighborCol, rows, cols ) )
    {
      const double nZ = demBlock->valueAndNoData( neighborRow, neighborCol, isNodata );
      if ( !isNodata )
      {
        const double diff = z - nZ;
        if ( diff > 0.0 )
        {
          dz[dir] = diff / QgsRasterAnalysisUtils::neighborCellDistance( dir, cellSizeX, cellSizeY );
          if ( dzMax < dz[dir] )
          {
            dzMax = dz[dir];
          }
        }
      }
    }
  }

  if ( dzMax > 0.0 )
  {
    const double exponent = ( dzMax < 1.0 ) ? ( 8.9 * dzMax + 1.1 ) : 10.0;
    double dzSum = 0.0;

    for ( int i = 0; i < 8; ++i )
    {
      if ( dz[i] > 0.0 )
      {
        dz[i] = std::pow( dz[i], exponent ) * ( ( contour && ( i % 2 ) ) ? M_SQRT1_2 : 1.0 );
        dzSum += dz[i];
      }
    }

    if ( dzSum > 0.0 )
    {
      double flow = 0.0;
      for ( int i = 0; i < 8; ++i )
      {
        if ( dz[i] > 0.0 )
        {
          int neighborCol = 0;
          int neighborRow = 0;
          QgsRasterAnalysisUtils::neighborCellCoordinates( i, row, col, neighborRow, neighborCol, rows, cols );
          const double nFlow = mFlowData[static_cast<qgssize>( neighborRow ) * cols + neighborCol];
          if ( nFlow > 0.0 )
          {
            // NOTE: SAGA's implementation has a bug here -- it does NOT scale by nFlow, which does not match
            // the original paper (Qin et al 2011). This results in outputs which contain only 1 or 100 cell values.
            // From Qin et al: "di is the FRACTION of flow into the ith neighboring cell"
            flow += ( dz[i] / dzSum ) * nFlow;
          }
        }
      }

      if ( flow > 0.0 )
      {
        mFlowData[static_cast<qgssize>( row ) * cols + col] = flow;
      }
    }
  }
}

void QgsUpslopeAreaAlgorithmBase::computeMDInf( const QgsRasterBlock *demBlock, int col, int row, int cols, int rows, double cellSizeX, double cellSizeY, double converge )
{
  const double z = demBlock->value( row, col );
  bool bInGrid[8];
  double dz[8];
  double sFacet[8];
  double rFacet[8];

  bool isNodata = false;
  for ( int i = 0; i < 8; ++i )
  {
    bInGrid[i] = false;
    dz[i] = 0;
    sFacet[i] = -999;
    rFacet[i] = -999;
    int neighborCol = 0;
    int neighborRow = 0;
    if ( QgsRasterAnalysisUtils::neighborCellCoordinates( i, row, col, neighborRow, neighborCol, rows, cols ) )
    {
      const double nZ = demBlock->valueAndNoData( neighborRow, neighborCol, isNodata );
      if ( !isNodata )
      {
        bInGrid[i] = true;
        dz[i] = z - nZ;
      }
    }
  }

  for ( int i = 0; i < 8; ++i )
  {
    double hs = -999.0;
    double hr = -999.0;

    if ( bInGrid[i] )
    {
      const int j = ( i < 7 ) ? i + 1 : 0;

      if ( bInGrid[j] )
      {
        const double nx = ( dz[j] * SAGA_Y[i] - dz[i] * SAGA_Y[j] ) * cellSizeY;
        const double ny = ( dz[i] * SAGA_X[j] - dz[j] * SAGA_X[i] ) * cellSizeX;
        const double nz = ( SAGA_X[i] * SAGA_Y[j] - SAGA_X[j] * SAGA_Y[i] ) * ( cellSizeX * cellSizeY );

        const double nNorm = std::sqrt( nx * nx + ny * ny + nz * nz );

        if ( nx == 0.0 )
        {
          hr = ( ny >= 0.0 ) ? 0.0 : M_PI;
        }
        else if ( nx < 0.0 )
        {
          hr = ( 1.5 * M_PI ) - std::atan( ny / nx );
        }
        else
        {
          hr = ( 0.5 * M_PI ) - std::atan( ny / nx );
        }

        const double cosAngle = std::clamp( nz / nNorm, -1.0, 1.0 );
        hs = -std::tan( std::acos( cosAngle ) );

        if ( hr < i * ( M_PI / 4.0 ) || hr > ( i + 1 ) * ( M_PI / 4.0 ) )
        {
          if ( dz[i] > dz[j] )
          {
            hr = i * ( M_PI / 4.0 );
            hs = dz[i] / QgsRasterAnalysisUtils::neighborCellDistance( i, cellSizeX, cellSizeY );
          }
          else
          {
            hr = j * ( M_PI / 4.0 );
            hs = dz[j] / QgsRasterAnalysisUtils::neighborCellDistance( j, cellSizeX, cellSizeY );
          }
        }
      }
      else if ( dz[i] > 0.0 )
      {
        hr = i * ( M_PI / 4.0 );
        hs = dz[i] / QgsRasterAnalysisUtils::neighborCellDistance( i, cellSizeX, cellSizeY );
      }

      sFacet[i] = hs;
      rFacet[i] = hr;
    }
  }

  double dzSum = 0.0;
  double valley[8];
  double portion[8];
  for ( int i = 0; i < 8; ++i )
  {
    valley[i] = 0;
    portion[i] = 0;
    int j = ( i < 7 ) ? i + 1 : 0;

    if ( sFacet[i] > 0.0 )
    {
      if ( rFacet[i] > i * ( M_PI / 4.0 ) && rFacet[i] < ( i + 1 ) * ( M_PI / 4.0 ) )
      {
        valley[i] = sFacet[i];
      }
      else if ( rFacet[i] == rFacet[j] )
      {
        valley[i] = sFacet[i];
      }
      else if ( sFacet[j] == -999.0 && rFacet[i] == ( i + 1 ) * ( M_PI / 4.0 ) )
      {
        valley[i] = sFacet[i];
      }
      else
      {
        const int k = ( i > 0 ) ? i - 1 : 7;
        if ( sFacet[k] == -999.0 && rFacet[i] == i * ( M_PI / 4.0 ) )
        {
          valley[i] = sFacet[i];
        }
      }

      valley[i] = std::pow( valley[i], converge );
      dzSum += valley[i];
    }
    portion[i] = 0.0;
  }

  if ( dzSum > 0.0 )
  {
    for ( int i = 0; i < 8; ++i )
    {
      const int j = ( i < 7 ) ? i + 1 : 0;

      if ( i >= 7 && rFacet[i] == 0.0 )
      {
        rFacet[i] = 2.0 * M_PI;
      }

      if ( valley[i] > 0.0 )
      {
        valley[i] /= dzSum;
        portion[i] += valley[i] * ( ( i + 1 ) * ( M_PI / 4.0 ) - rFacet[i] ) / ( M_PI / 4.0 );
        portion[j] += valley[i] * ( rFacet[i] - i * ( M_PI / 4.0 ) ) / ( M_PI / 4.0 );
      }
    }

    double flow = 0.0;
    for ( int i = 0; i < 8; ++i )
    {
      if ( portion[i] > 0.0 )
      {
        int nCol = 0, nRow = 0;
        if ( QgsRasterAnalysisUtils::neighborCellCoordinates( i, row, col, nRow, nCol, rows, cols ) )
        {
          const double nFlow = mFlowData[static_cast<qgssize>( nRow ) * cols + nCol];
          if ( nFlow > 0.0 )
          {
            flow += nFlow * portion[i];
          }
        }
      }
    }

    if ( flow > 0.0 )
    {
      mFlowData[static_cast<qgssize>( row ) * cols + col] = flow;
    }
  }
}


//
// QgsUpslopeAreaPointAlgorithm
//

QgsUpslopeAreaPointAlgorithm::QgsUpslopeAreaPointAlgorithm() = default;

QString QgsUpslopeAreaPointAlgorithm::name() const
{
  return u"upslopeareafrompoint"_s;
}

QString QgsUpslopeAreaPointAlgorithm::displayName() const
{
  return QObject::tr( "Upslope area (from point)" );
}

QString QgsUpslopeAreaPointAlgorithm::shortDescription() const
{
  return QObject::tr( "Calculates the upslope contributing area for a specified target point coordinate." );
}

QString QgsUpslopeAreaPointAlgorithm::shortHelpString() const
{
  return QObject::tr(
    "This algorithm calculates the upslope contributing area (catchment) for a single target coordinate point on a Digital Elevation Model (DEM).\n\n"
    "Each output raster cell value represents the percentage (0% to 100%) of surface flow originating at that cell that drains to or passes through the target point.\n\n"
    "Supported flow routing methods are:\n\n"
    "• Deterministic 8: Single-flow direction algorithm, routing 100% of flow to the steepest downslope neighbor (O'Callaghan & Mark 1984).\n"
    "• Deterministic Infinity: Continuous single-facet flow direction algorithm, routing flow along triangular facets using a 3×3 finite-difference aspect calculation (Tarboton 1997).\n"
    "• Multiple Flow Direction: Divergent flow distribution to all lower-elevation neighbors, weighted by slope and a configurable convergence exponent (Freeman 1991, Quinn et al. 1991).\n"
    "• Multiple Triangular Flow Direction: Advanced divergent routing utilizing 3D vector normal cross-products across triangular facets to distribute flow smoothly across complex terrain (Seibert & "
    "McGlynn 2007).\n"
    "• Multiple Maximum Downslope Gradient: Adaptive MFD variant scaling exponent weights dynamically based on the local maximum gradient (Qin et al. 2011).\n\n"
    "An optional sink routes raster layer can be provided to explicitly override topographic flow and direct water through karst features, culverts, or artificial depressions.\n\n"
    "This algorithm is a port of the SAGA 'Upslope Area' tool."
  );
}

QgsProcessingAlgorithm *QgsUpslopeAreaPointAlgorithm::createInstance() const
{
  return new QgsUpslopeAreaPointAlgorithm();
}

void QgsUpslopeAreaPointAlgorithm::initAlgorithm( const QVariantMap & )
{
  addCommonParameters();

  auto pointParam = std::make_unique<QgsProcessingParameterPoint>( u"TARGET_PT"_s, QObject::tr( "Target point" ) );
  pointParam->setHelp( QObject::tr( "World coordinate point defining the target cell." ) );
  addParameter( pointParam.release() );
}

bool QgsUpslopeAreaPointAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  return prepareBase( parameters, context, feedback );
}

QVariantMap QgsUpslopeAreaPointAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QGS_MARK_ALGORITHM_SOURCE

  const QgsPointXY targetPt = parameterAsPoint( parameters, u"TARGET_PT"_s, context, mDemCrs );

  mOutputPath = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );
  mOutputFormat = parameterAsOutputRasterFormat( parameters, u"OUTPUT"_s, context );

  calculateUpslopeArea( { targetPt }, context, feedback );

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, mOutputPath );
  return outputs;
}


//
// QgsUpslopeAreaLayerAlgorithm
//

QgsUpslopeAreaLayerAlgorithm::QgsUpslopeAreaLayerAlgorithm() = default;

QString QgsUpslopeAreaLayerAlgorithm::name() const
{
  return u"upslopeareafromlayer"_s;
}

QString QgsUpslopeAreaLayerAlgorithm::displayName() const
{
  return QObject::tr( "Upslope area (from layer)" );
}

QString QgsUpslopeAreaLayerAlgorithm::shortDescription() const
{
  return QObject::tr( "Calculates the combined upslope contributing area for target points in a vector layer." );
}

QString QgsUpslopeAreaLayerAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates the combined upslope contributing catchment area for target points provided in an input vector point layer." );

  return QObject::tr(
    "This algorithm calculates the combined upslope contributing area (catchments) for all target point locations provided in an input vector layer.\n\n"
    "Each output raster cell value represents the percentage (0% to 100%) of surface flow originating at that cell that reaches at least one of the target points in the input vector layer.\n\n"
    "Supported flow routing methods are:\n\n"
    "• Deterministic 8: Single-flow direction algorithm, routing 100% of flow to the steepest downslope neighbor (O'Callaghan & Mark 1984).\n"
    "• Deterministic Infinity: Continuous single-facet flow direction algorithm, routing flow along triangular facets using a 3×3 finite-difference aspect calculation (Tarboton 1997).\n"
    "• Multiple Flow Direction: Divergent flow distribution to all lower-elevation neighbors, weighted by slope and a configurable convergence exponent (Freeman 1991, Quinn et al. 1991).\n"
    "• Multiple Triangular Flow Direction: Advanced divergent routing utilizing 3D vector normal cross-products across triangular facets to distribute flow smoothly across complex terrain (Seibert & "
    "McGlynn 2007).\n"
    "• Multiple Maximum Downslope Gradient: Adaptive MFD variant scaling exponent weights dynamically based on the local maximum gradient (Qin et al. 2011).\n\n"
    "An optional sink routes raster layer can be provided to explicitly override topographic flow and direct water through karst features, culverts, or artificial depressions.\n\n"
    "This algorithm is a port of the SAGA 'Upslope Area' tool."
  );
}

QgsProcessingAlgorithm *QgsUpslopeAreaLayerAlgorithm::createInstance() const
{
  return new QgsUpslopeAreaLayerAlgorithm();
}

void QgsUpslopeAreaLayerAlgorithm::initAlgorithm( const QVariantMap & )
{
  addCommonParameters();

  auto layerParam
    = std::make_unique<QgsProcessingParameterFeatureSource>( u"TARGET_LAYER"_s, QObject::tr( "Target point layer" ), QList<int> { static_cast<int>( Qgis::ProcessingSourceType::VectorPoint ) } );
  layerParam->setHelp( QObject::tr( "Vector point layer containing target locations." ) );
  addParameter( layerParam.release() );
}

bool QgsUpslopeAreaLayerAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  return prepareBase( parameters, context, feedback );
}

QVariantMap QgsUpslopeAreaLayerAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QGS_MARK_ALGORITHM_SOURCE

  std::unique_ptr<QgsFeatureSource> targetSource( parameterAsSource( parameters, u"TARGET_LAYER"_s, context ) );
  if ( !targetSource )
    throw QgsProcessingException( invalidSourceError( parameters, u"TARGET_LAYER"_s ) );

  std::vector<QgsPointXY> targetPoints;
  QgsFeature f;
  QgsFeatureIterator fit = targetSource->getFeatures( QgsFeatureRequest().setNoAttributes().setDestinationCrs( mDemCrs, context.transformContext() ) );
  while ( fit.nextFeature( f ) )
  {
    if ( f.hasGeometry() )
    {
      const QgsPointXY pt = f.geometry().asPoint();
      targetPoints.push_back( pt );
    }
  }

  if ( targetPoints.empty() )
  {
    throw QgsProcessingException( QObject::tr( "Input target point layer contains no valid point geometries." ) );
  }

  mOutputPath = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );
  mOutputFormat = parameterAsOutputRasterFormat( parameters, u"OUTPUT"_s, context );

  calculateUpslopeArea( targetPoints, context, feedback );

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, mOutputPath );
  return outputs;
}

///@endcond
