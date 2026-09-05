/***************************************************************************
                         qgsalgorithmtpsinterpolation.cpp
                         -----------------------------------
    begin                : August 2026
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

#include "qgsalgorithmtpsinterpolation.h"

#include <numeric>

#include "qgsacademicreference.h"
#include "qgsapplication.h"
#include "qgsmatrixsolver.h"
#include "qgsprocessingfeedback.h"
#include "qgsprocessingparameterinterpolationpixelsize.h"
#include "qgsprocessingutils.h"
#include "qgsrasterfilewriter.h"
#include "qgsrasteriterator.h"
#include "qgsspatialindexkdbush.h"
#include "qgsvariantutils.h"

#include <QString>
#include <QtConcurrentMap>

using namespace Qt::StringLiterals;

///@cond PRIVATE

//
// QgsThinPlateSplineAlgorithmBase
//

QIcon QgsThinPlateSplineAlgorithmBase::icon() const
{
  return QgsApplication::getThemeIcon( u"/algorithms/mAlgorithmInterpolation.svg"_s );
}

QString QgsThinPlateSplineAlgorithmBase::svgIconPath() const
{
  return QgsApplication::iconPath( u"/algorithms/mAlgorithmInterpolation.svg"_s );
}

QString QgsThinPlateSplineAlgorithmBase::group() const
{
  return QObject::tr( "Interpolation" );
}

QString QgsThinPlateSplineAlgorithmBase::groupId() const
{
  return u"interpolation"_s;
}

QList<QgsAcademicReference> QgsThinPlateSplineAlgorithmBase::academicReferences() const
{
  const QgsAcademicReference donatoReference = QgsAcademicReference::createPresentation(
    { u"Donato, G."_s, u"Belongie, S."_s },
    2002,
    u"Approximation Methods for Thin Plate Spline Mappings and Principal Warps"_s,
    u"In Heyden, A., Sparr, G., Nielsen, M., Johansen, P. (Eds.): Computer Vision - ECCV 2002: 7th European Conference on Computer Vision, Copenhagen, Denmark, May 28-31, 2002, Proceedings, Part III, Lecture Notes in Computer Science."_s,
    u"Springer-Verlag Heidelberg"_s,
    u"21-31"_s
  );

  const QgsAcademicReference elonenReference
    = QgsAcademicReference::createWebPage( { u"Elonen, J."_s }, 2005, u"Thin Plate Spline editor - an example program in C++"_s, u"http://elonen.iki.fi/code/tpsdemo/index.html"_s );
  return { donatoReference, elonenReference };
}

void QgsThinPlateSplineAlgorithmBase::addCommonParameters()
{
  auto inputParam = std::make_unique<QgsProcessingParameterFeatureSource>( u"INPUT"_s, QObject::tr( "Point layer" ), QList<int> { static_cast< int >( Qgis::ProcessingSourceType::VectorPoint ) } );
  inputParam->setHelp( QObject::tr( "Vector point layer containing scattered control points with 3D coordinate or attribute values." ) );
  addParameter( inputParam.release() );

  auto fieldParam = std::make_unique<QgsProcessingParameterField>( u"FIELD"_s, QObject::tr( "Z Field" ), QVariant(), u"INPUT"_s, Qgis::ProcessingFieldParameterDataType::Numeric );
  fieldParam->setHelp( QObject::tr( "Numeric attribute field containing the values (elevation/Z) to interpolate." ) );
  addParameter( fieldParam.release() );

  auto regularizationParam = std::make_unique<QgsProcessingParameterNumber>( u"REGULARIZATION"_s, QObject::tr( "Regularization" ), Qgis::ProcessingNumberParameterType::Double, 0.0001, false, 0.0 );
  regularizationParam->setHelp(
    QObject::tr(
      "Regularization parameter (lambda), where a value of 0 produces an exact spline interpolation passing precisely "
      "through all control points. Values > 0 introduce smoothing/tension to reduce noise and flatten high-frequency variations."
    )
  );
  addParameter( regularizationParam.release() );
}

void QgsThinPlateSplineAlgorithmBase::addOutputParameters()
{
  auto extentParam = std::make_unique<QgsProcessingParameterExtent>( u"EXTENT"_s, QObject::tr( "Extent" ), QVariant(), false );
  extentParam->setHelp( QObject::tr( "Bounding box defining the extent of the output raster grid." ) );
  addParameter( extentParam.release() );

  auto pixelSizeParam = std::make_unique<QgsProcessingParameterInterpolationPixelSize>( u"PIXEL_SIZE"_s, QObject::tr( "Output raster size" ), u"INTERPOLATION_DATA"_s, u"EXTENT"_s, 0.1 );
  pixelSizeParam->setHelp( QObject::tr( "Pixel size in layer units used to calculate output grid dimensions." ) );
  addParameter( pixelSizeParam.release() );

  auto colsParam = std::make_unique<QgsProcessingParameterNumber>( u"COLUMNS"_s, QObject::tr( "Number of columns" ), Qgis::ProcessingNumberParameterType::Integer, QVariant(), true, 0, 10000000 );
  colsParam->setFlags( colsParam->flags() | Qgis::ProcessingParameterFlag::Hidden );
  addParameter( colsParam.release() );

  auto rowsParam = std::make_unique<QgsProcessingParameterNumber>( u"ROWS"_s, QObject::tr( "Number of rows" ), Qgis::ProcessingNumberParameterType::Integer, QVariant(), true, 0, 10000000 );
  rowsParam->setFlags( rowsParam->flags() | Qgis::ProcessingParameterFlag::Hidden );
  addParameter( rowsParam.release() );

  auto outputNodataParam = std::make_unique<QgsProcessingParameterNumber>( u"NODATA"_s, QObject::tr( "Output NoData value" ), Qgis::ProcessingNumberParameterType::Double, -9999.0 );
  outputNodataParam->setHelp( QObject::tr( "The NODATA value to use in the output raster." ) );
  outputNodataParam->setFlags( outputNodataParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( outputNodataParam.release() );

  auto creationOptsParam = std::make_unique<QgsProcessingParameterString>( u"CREATION_OPTIONS"_s, QObject::tr( "Creation options" ), QVariant(), false, true );
  creationOptsParam->setHelp( QObject::tr( "The raster creation options for the output raster. These options control things like colorimetry, compression, etc." ) );
  creationOptsParam->setMetadata( QVariantMap( { { u"widget_wrapper"_s, QVariantMap( { { u"widget_type"_s, u"rasteroptions"_s } } ) } } ) );
  creationOptsParam->setFlags( creationOptsParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( creationOptsParam.release() );

  auto outputParam = std::make_unique<QgsProcessingParameterRasterDestination>( u"OUTPUT"_s, QObject::tr( "Interpolated" ) );
  addParameter( outputParam.release() );
}

void QgsThinPlateSplineAlgorithmBase::processBase( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  if ( !QgsMatrixSolver::isAvailable() )
  {
    throw QgsProcessingException( QObject::tr( "This algorithm requires a QGIS build with GSL support enabled." ) );
  }

  mSource.reset( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !mSource )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  mFieldName = parameterAsString( parameters, u"FIELD"_s, context );
  mRegularization = parameterAsDouble( parameters, u"REGULARIZATION"_s, context );

  mExtent = parameterAsExtent( parameters, u"EXTENT"_s, context, mSource->sourceCrs() );

  mPixelSize = parameterAsDouble( parameters, u"PIXEL_SIZE"_s, context );
  mOutputPath = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );
  mCreationOptions = parameterAsString( parameters, u"CREATION_OPTIONS"_s, context ).trimmed();
  mNoDataValue = parameterAsDouble( parameters, u"NODATA"_s, context );

  mFieldIndex = mSource->fields().lookupField( mFieldName );
  if ( mFieldIndex < 0 )
  {
    throw QgsProcessingException( QObject::tr( "Attribute field '%1' was not found in input layer." ).arg( mFieldName ) );
  }
}


namespace
{

  struct Neighbor
  {
      double x;
      double y;
      double z;
      double distSq;
      bool operator<( const Neighbor &other ) const { return distSq < other.distSq; }
  };

  struct CellStats
  {
      qsizetype totalCells = 0;
      qsizetype validCells = 0;
      qsizetype unsolvableCells = 0;
      std::size_t minNeighbors = std::numeric_limits<std::size_t>::max();
      std::size_t maxNeighbors = 0;
      unsigned long long sumNeighbors = 0;
      double minDistance = std::numeric_limits<double>::max();
      double maxDistance = 0.0;
      double sumDistance = 0.0;
      unsigned long long countDistances = 0;

      void merge( const CellStats &other )
      {
        totalCells += other.totalCells;
        validCells += other.validCells;
        unsolvableCells += other.unsolvableCells;
        if ( other.minNeighbors != std::numeric_limits<std::size_t>::max() )
        {
          minNeighbors = std::min( minNeighbors, other.minNeighbors );
          maxNeighbors = std::max( maxNeighbors, other.maxNeighbors );
        }
        sumNeighbors += other.sumNeighbors;
        if ( other.minDistance != std::numeric_limits<double>::max() )
        {
          minDistance = std::min( minDistance, other.minDistance );
          maxDistance = std::max( maxDistance, other.maxDistance );
        }
        sumDistance += other.sumDistance;
        countDistances += other.countDistances;
      }
  };

  struct RowResult
  {
      int r = 0;
      std::vector<double> values;
      CellStats stats;
  };

  typedef QHash<QgsFeatureId, double> FeatureZValueHash;
} //namespace


//
// QgsLocalThinPlateSplineAlgorithm
//

QgsLocalThinPlateSplineAlgorithm::QgsLocalThinPlateSplineAlgorithm() = default;

QString QgsLocalThinPlateSplineAlgorithm::name() const
{
  return u"localtpsinterpolation"_s;
}

QString QgsLocalThinPlateSplineAlgorithm::displayName() const
{
  return QObject::tr( "Thin Plate Spline interpolation (local)" );
}

QStringList QgsLocalThinPlateSplineAlgorithm::tags() const
{
  return QObject::tr( "tps,thin,plate,spline,interpolation,surface" ).split( ',' );
}

QString QgsLocalThinPlateSplineAlgorithm::shortDescription() const
{
  return QObject::tr( "Generates a Thin Plate Spline interpolation from scattered vector points." );
}

QString QgsLocalThinPlateSplineAlgorithm::shortHelpString() const
{
  return QObject::tr(
    "This algorithm creates a 'Thin Plate Spline' (TPS) surface for each grid point based on scattered data points "
    "within a specified local search distance. The number of points evaluated per cell can be constrained "
    "to a maximum number of closest neighbors.\n\n"
    "Thin Plate Splines minimize the integral of the squared second derivatives, creating a smooth surface "
    "resembling a bent thin metal plate. Regularisation allows softening the exact fitting constraint to smooth out noise.\n\n"
    "This algorithm is a port of the SAGA 'Thin Plate Spline' tool."
  );
}

QgsLocalThinPlateSplineAlgorithm *QgsLocalThinPlateSplineAlgorithm::createInstance() const
{
  return new QgsLocalThinPlateSplineAlgorithm();
}

void QgsLocalThinPlateSplineAlgorithm::initAlgorithm( const QVariantMap & )
{
  addCommonParameters();

  auto radiusParam = std::make_unique<QgsProcessingParameterDistance>( u"SEARCH_RADIUS"_s, QObject::tr( "Maximum Search Distance" ), 1000.0, u"INPUT"_s, false, 0.0 );
  radiusParam->setHelp( QObject::tr( "Local maximum search radius. Points farther than this distance from a grid cell center are ignored." ) );
  addParameter( radiusParam.release() );

  auto maxPtsParam = std::make_unique<QgsProcessingParameterNumber>( u"SEARCH_POINTS_MAX"_s, QObject::tr( "Maximum number of nearest points" ), Qgis::ProcessingNumberParameterType::Integer, 20, false, 1 );
  maxPtsParam->setHelp( QObject::tr( "Maximum number of nearest points within the search distance to evaluate per grid cell." ) );
  addParameter( maxPtsParam.release() );

  auto minPtsParam = std::make_unique<QgsProcessingParameterNumber>( u"SEARCH_POINTS_MIN"_s, QObject::tr( "Minimum number of points" ), Qgis::ProcessingNumberParameterType::Integer, 16, false, 3 );
  minPtsParam->setHelp( QObject::tr( "Minimum required points within search distance. At least 3 points are mandatory to solve a 2D spline; cells with fewer points are assigned NoData." ) );
  addParameter( minPtsParam.release() );

  addOutputParameters();
}

QVariantMap QgsLocalThinPlateSplineAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  processBase( parameters, context, feedback );

  const double searchRadius = parameterAsDouble( parameters, u"SEARCH_RADIUS"_s, context );
  const int maxPoints = parameterAsInt( parameters, u"SEARCH_POINTS_MAX"_s, context );
  const int minPoints = parameterAsInt( parameters, u"SEARCH_POINTS_MIN"_s, context );

  QgsProcessingMultiStepFeedback multiStepFeedback( 2, feedback );
  // estimate 5% of overall time for feature iteration and spatial index construction...
  multiStepFeedback.setStepWeights( { 0.05, 0.95 } );

  multiStepFeedback.setCurrentStep( 0 );
  multiStepFeedback.pushInfo( QObject::tr( "Building spatial index…" ) );

  FeatureZValueHash zValues;
  QgsFeature feat;
  QgsFeatureRequest request;
  request.setSubsetOfAttributes( { mFieldIndex } );
  QgsFeatureIterator fit = mSource->getFeatures( request );

  const long count = mSource->featureCount();
  const double step = count > 0 ? 100.0 / count : 1;

  long long current = 0;
  QgsSpatialIndexKDBush kdTree(
    fit,
    [&zValues, &current, &multiStepFeedback, step, this]( const QgsFeature &feature ) -> bool {
      multiStepFeedback.setProgress( current * step );
      current++;

      if ( !feature.hasGeometry() )
        return true;

      const QVariant fieldValue = feature.attribute( mFieldIndex );
      if ( !QgsVariantUtils::isNull( fieldValue ) )
      {
        zValues[feature.id()] = fieldValue.toDouble();
      }
      return true;
    },
    &multiStepFeedback
  );

  if ( multiStepFeedback.isCanceled() )
    return {};

  if ( zValues.size() < 3 )
  {
    throw QgsProcessingException( QObject::tr( "At least 3 valid points with non-null Z attributes are required to form a Thin Plate Spline." ) );
  }

  const FeatureZValueHash constZValues = std::as_const( zValues );

  multiStepFeedback.setCurrentStep( 1 );
  multiStepFeedback.pushInfo( QObject::tr( "Interpolating…" ) );

  int cols = std::max( 1, static_cast<int>( std::ceil( mExtent.width() / mPixelSize ) ) );
  int rows = std::max( 1, static_cast<int>( std::ceil( mExtent.height() / mPixelSize ) ) );

  auto writer = std::make_unique<QgsRasterFileWriter>( mOutputPath );
  writer->setOutputProviderKey( u"gdal"_s );
  if ( !mCreationOptions.isEmpty() )
  {
    writer->setCreationOptions( mCreationOptions.split( '|' ) );
  }
  std::unique_ptr<QgsRasterDataProvider> provider( writer->createOneBandRaster( Qgis::DataType::Float32, cols, rows, mExtent, mSource->sourceCrs() ) );

  if ( !provider )
    throw QgsProcessingException( QObject::tr( "Could not write destination raster file: %1" ).arg( mOutputPath ) );
  if ( !provider->isValid() )
    throw QgsProcessingException( QObject::tr( "Could not create raster output %1: %2" ).arg( mOutputPath, provider->error().message( QgsErrorMessage::Text ) ) );

  provider->setNoDataValue( 1, mNoDataValue );
  provider->setEditable( true );

  QgsRasterIterator iter( provider.get() );
  iter.startRasterRead( 1, cols, rows, mExtent );

  int iterCols = 0;
  int iterRows = 0;
  int topLeftCol = 0;
  int topLeftRow = 0;
  QgsRectangle blockExtent;

  std::unique_ptr<QgsRasterBlock> outputBlock;

  CellStats globalStats;

  while ( iter.readNextRasterPart( 1, iterCols, iterRows, outputBlock, topLeftCol, topLeftRow, &blockExtent ) )
  {
    if ( multiStepFeedback.isCanceled() )
      break;

    QVector<int> tileRowIndices( iterRows );
    std::iota( tileRowIndices.begin(), tileRowIndices.end(), 0 );

    int completedTileRows = 0;

    const CellStats tileStats = QtConcurrent::blockingMappedReduced<CellStats>(
      tileRowIndices,
      [iterCols, maxPoints, topLeftRow, topLeftCol, searchRadius, minPoints, &constZValues, &kdTree, &multiStepFeedback, this]( int r ) -> RowResult {
        RowResult rowResult;
        rowResult.r = r;
        rowResult.values.resize( iterCols );

        if ( multiStepFeedback.isCanceled() )
          return rowResult;

        QgsMatrixSolver solver( maxPoints + 3 );
        QVector<double> W;
        std::vector<Neighbor> neighbors;

        const int globalRow = topLeftRow + r;
        const double y = mExtent.yMaximum() - ( globalRow + 0.5 ) * mPixelSize;

        for ( int c = 0; c < iterCols; ++c )
        {
          const int globalCol = topLeftCol + c;
          const double x = mExtent.xMinimum() + ( globalCol + 0.5 ) * mPixelSize;
          neighbors.clear();

          kdTree.within( QgsPointXY( x, y ), searchRadius, [&constZValues, &neighbors, x, y]( const QgsSpatialIndexKDBushData &data ) {
            const auto it = constZValues.constFind( data.id );
            if ( it != constZValues.constEnd() )
            {
              const double pX = data.coords.first;
              const double pY = data.coords.second;
              const double dx = pX - x;
              const double dy = pY - y;
              neighbors.emplace_back( Neighbor { pX, pY, it.value(), dx * dx + dy * dy } );
            }
          } );

          const std::size_t neighborCount = neighbors.size();
          rowResult.stats.totalCells++;
          rowResult.stats.minNeighbors = std::min( rowResult.stats.minNeighbors, neighborCount );
          rowResult.stats.maxNeighbors = std::max( rowResult.stats.maxNeighbors, neighborCount );
          rowResult.stats.sumNeighbors += neighborCount;

          if ( static_cast< int >( neighborCount ) < minPoints )
          {
            rowResult.values[c] = mNoDataValue;
            continue;
          }

          int n = static_cast< int >( neighborCount );
          if ( n > maxPoints )
          {
            n = maxPoints;
            std::partial_sort( neighbors.begin(), neighbors.begin() + n, neighbors.end() );
          }

          // distance statistics
          for ( int i = 0; i < n; ++i )
          {
            const double dist = std::sqrt( neighbors[i].distSq );
            rowResult.stats.minDistance = std::min( rowResult.stats.minDistance, dist );
            rowResult.stats.maxDistance = std::max( rowResult.stats.maxDistance, dist );
            rowResult.stats.sumDistance += dist;
            rowResult.stats.countDistances++;
          }

          // Calculate Thin Plate Spline (TPS) weights from control points.
          // System matrix L of size (n+3) x (n+3) consists of:
          //   - K (n x n, upper-left kernel matrix)
          //   - P (n x 3, upper-right polynomial matrix)
          //   - P^T (3 x n, lower-left transposed polynomial matrix)
          //   - O (3 x 3, lower-right zero matrix)
          const int systemSize = n + 3;

          // Fill K (n x n upper-left block of matrix L) and calculate mean edge length from control points.
          // K is symmetric so we only compute the upper triangle and mirror values to the lower triangle.
          double meanDist = 0.0;
          for ( int i = 0; i < n; ++i )
          {
            const double iX = neighbors[i].x;
            const double iY = neighbors[i].y;
            for ( int j = i + 1; j < n; ++j )
            {
              const double dx = iX - neighbors[j].x;
              const double dy = iY - neighbors[j].y;
              const double distanceSquared = dx * dx + dy * dy;
              const double distance = std::sqrt( distanceSquared );
              const double baseVal = ( distance > 0.0 ) ? ( distanceSquared * std::log( distance ) ) : 0.0;
              // Symmetric entries in K
              meanDist += distance * 2.0;
              solver.setValue( i, j, baseVal );
              solver.setValue( j, i, baseVal );
            }
          }
          meanDist /= ( n * n );

          // Fill the remaining blocks of system matrix L and right-hand side vector v
          for ( int i = 0; i < n; ++i )
          {
            // K diagonal: regularization parameters (lambda * meanDist^2)
            solver.setValue( i, i, mRegularization * ( meanDist * meanDist ) );

            // P (n x 3 upper-right block), P^T (3 x n bottom-left block)
            solver.setValue( i, n, 1.0 );
            solver.setValue( i, n + 1, neighbors[i].x );
            solver.setValue( i, n + 2, neighbors[i].y );

            solver.setValue( n, i, 1.0 );
            solver.setValue( n + 1, i, neighbors[i].x );
            solver.setValue( n + 2, i, neighbors[i].y );

            // Fill right-hand side vector v (control point z values for 0 <= i < n)
            solver.setRightHandSide( i, neighbors[i].z );
          }

          for ( int i = n; i < n + 3; ++i )
          {
            for ( int j = n; j < n + 3; ++j )
            {
              // O (3 x 3 lower-right block)
              solver.setValue( i, j, 0.0 );
            }
            // Zero polynomial constraints for vector v
            solver.setRightHandSide( i, 0.0 );
          }

          // NOTE: SAGA's version of this tool uses Lu solving only, but with a bug in the solver
          // which prevents it correctly flagging singular matrices. Here we use the SVD fallback approach
          // for a more tolerant solver, so that the results more closely represent SAGA's results (i.e.
          // avoiding nodata pixels were SAGA's tools output data pixels)
          if ( !solver.solve( systemSize, W, Qgis::LinearMatrixMethod::LuWithSvdFallback ) )
          {
            rowResult.values[c] = mNoDataValue;
            rowResult.stats.unsolvableCells++;
            continue;
          }

          rowResult.stats.validCells++;

          // Evaluate local spline equation: z = a0 + ax*x + ay*y + sum(w_i * U(r_i))
          double zVal = W[n] + W[n + 1] * x + W[n + 2] * y;
          for ( int i = 0; i < n; ++i )
          {
            double distSq = neighbors[i].distSq;
            double U = ( distSq > 0.0 ) ? ( distSq * 0.5 * std::log( distSq ) ) : 0.0;
            zVal += W[i] * U;
          }

          rowResult.values[c] = zVal;
        }

        return rowResult;
      },
      [&completedTileRows, feedback, iterRows, iterCols, &outputBlock, &iter]( CellStats &accumulatedStats, const RowResult &rowResult ) {
        accumulatedStats.merge( rowResult.stats );

        for ( int c = 0; c < iterCols; ++c )
        {
          outputBlock->setValue( rowResult.r, c, rowResult.values[c] );
        }

        completedTileRows++;

        const double currentBlockProgress = static_cast< double >( completedTileRows ) / iterRows;
        const double blockProgressFraction = iter.progress( 1, currentBlockProgress );
        const double overallProgress = 100.0 * ( 0.05 + 0.95 * blockProgressFraction );
        feedback->setProgress( overallProgress );
      }
    );

    globalStats.merge( tileStats );

    if ( multiStepFeedback.isCanceled() )
      break;

    if ( !provider->writeBlock( outputBlock.get(), 1, topLeftCol, topLeftRow ) )
    {
      throw QgsProcessingException( QObject::tr( "Could not write raster block: %1" ).arg( provider->error().summary() ) );
    }
    multiStepFeedback.setProgress( 100.0 * iter.progress( 1 ) );
  }
  provider->setEditable( false );

  iter.stopRasterRead( 1 );

  if ( globalStats.unsolvableCells > 0 )
  {
    multiStepFeedback.pushWarning( QObject::tr( "The thin plate spline could not be solved for %1 raster cells. Try increasing the maximum search distance." ).arg( globalStats.unsolvableCells ) );
  }

  if ( globalStats.validCells > 0 )
  {
    const double meanNeighbors = static_cast< double >( globalStats.sumNeighbors ) / globalStats.totalCells;
    multiStepFeedback.pushInfo( QObject::tr( "Neighbor count statistics:" ) );
    multiStepFeedback.pushInfo( QObject::tr( "• Minimum: %1" ).arg( globalStats.minNeighbors ) );
    multiStepFeedback.pushInfo( QObject::tr( "• Maximum: %1" ).arg( globalStats.maxNeighbors ) );
    multiStepFeedback.pushInfo( QObject::tr( "• Mean: %1" ).arg( QString::number( meanNeighbors, 'f', 2 ) ) );
  }

  if ( globalStats.countDistances > 0 )
  {
    const double meanDistance = globalStats.sumDistance / globalStats.countDistances;
    multiStepFeedback.pushInfo( QObject::tr( "Neighbor distance statistics:" ) );
    multiStepFeedback.pushInfo( QObject::tr( "• Minimum: %1" ).arg( QString::number( globalStats.minDistance, 'f', 4 ) ) );
    multiStepFeedback.pushInfo( QObject::tr( "• Maximum: %1" ).arg( QString::number( globalStats.maxDistance, 'f', 4 ) ) );
    multiStepFeedback.pushInfo( QObject::tr( "• Mean: %1" ).arg( QString::number( meanDistance, 'f', 4 ) ) );
  }

  if ( static_cast< int >( globalStats.maxNeighbors ) < minPoints )
  {
    multiStepFeedback.pushWarning(
      QObject::tr( "Maximum neighbors found within search radius was too small (got %1, required at least %2), output raster is empty" ).arg( globalStats.maxNeighbors ).arg( minPoints )
    );
  }

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, mOutputPath );
  return outputs;
}


//
// QgsGlobalThinPlateSplineAlgorithm
//

QgsGlobalThinPlateSplineAlgorithm::QgsGlobalThinPlateSplineAlgorithm() = default;

QString QgsGlobalThinPlateSplineAlgorithm::name() const
{
  return u"globaltpsinterpolation"_s;
}

QString QgsGlobalThinPlateSplineAlgorithm::displayName() const
{
  return QObject::tr( "Thin Plate Spline interpolation (global)" );
}

QStringList QgsGlobalThinPlateSplineAlgorithm::tags() const
{
  return QObject::tr( "tps,thin,plate,spline,interpolation,surface" ).split( ',' );
}

QString QgsGlobalThinPlateSplineAlgorithm::shortDescription() const
{
  return QObject::tr( "Generates a Thin Plate Spline interpolation from scattered vector points." );
}

QString QgsGlobalThinPlateSplineAlgorithm::shortHelpString() const
{
  return QObject::tr(
    "This algorithm calculates a single global Thin Plate Spline surface passing through all input points simultaneously.\n\n"
    "Thin Plate Splines minimize the integral of the squared second derivatives, creating a smooth surface "
    "resembling a bent thin metal plate. Regularisation allows softening the exact fitting constraint to smooth out noise.\n\n"
    "A global Thin Plate Spline interpolation constructs and solves a single linear system across all control points up front. "
    "It guarantees a continuous surface without spatial windowing boundaries, but requires high memory "
    "and computation time for datasets with large point counts.\n\n"
    "This algorithm is a port of the SAGA 'Thin Plate Spline' tool."
  );
}

QgsGlobalThinPlateSplineAlgorithm *QgsGlobalThinPlateSplineAlgorithm::createInstance() const
{
  return new QgsGlobalThinPlateSplineAlgorithm();
}

void QgsGlobalThinPlateSplineAlgorithm::initAlgorithm( const QVariantMap & )
{
  addCommonParameters();
  addOutputParameters();
}

namespace
{
  struct ControlPoint
  {
      double x = 0.0;
      double y = 0.0;
      double z = 0.0;
  };
} //namespace

QVariantMap QgsGlobalThinPlateSplineAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  processBase( parameters, context, feedback );

  QgsProcessingMultiStepFeedback multiStepFeedback( 2, feedback );
  // estimate 10% of overall time for feature iteration and matrix construction...
  multiStepFeedback.setStepWeights( { 0.10, 0.90 } );

  multiStepFeedback.setCurrentStep( 0 );
  multiStepFeedback.pushInfo( QObject::tr( "Collecting control points and building global TPS matrix…" ) );

  QgsFeature feat;
  QgsFeatureRequest request;
  request.setSubsetOfAttributes( { mFieldIndex } );
  QgsFeatureIterator fit = mSource->getFeatures( request );

  const long count = mSource->featureCount();
  const double step = count > 0 ? 100.0 / count : 1;

  std::vector<ControlPoint> globalPoints;

  long long current = 0;
  while ( fit.nextFeature( feat ) )
  {
    if ( multiStepFeedback.isCanceled() )
      return {};

    if ( feat.hasGeometry() )
    {
      const QVariant fieldValue = feat.attribute( mFieldIndex );
      if ( !QgsVariantUtils::isNull( fieldValue ) )
      {
        const QgsPointXY pt = feat.geometry().asPoint();
        globalPoints.push_back( ControlPoint { pt.x(), pt.y(), fieldValue.toDouble() } );
      }
    }
    current++;
    multiStepFeedback.setProgress( current * step );
  }

  if ( multiStepFeedback.isCanceled() )
    return {};

  if ( globalPoints.size() < 3 )
  {
    throw QgsProcessingException( QObject::tr( "At least 3 valid points with non-null Z attributes are required to form a Thin Plate Spline." ) );
  }
  const int n = static_cast<int>( globalPoints.size() );

  constexpr double MAX_MEMORY_MB = 1024.0; // limit to max of 1gb memory
  constexpr double MAX_MEMORY_BYTES = MAX_MEMORY_MB * 1024 * 1024;

  const double matrixSizeBytes = ( static_cast<double>( n + 3 ) * ( n + 3 ) * sizeof( double ) );

  if ( matrixSizeBytes > MAX_MEMORY_BYTES )
  {
    const int maxPointsAllowed = static_cast<int>( std::sqrt( MAX_MEMORY_BYTES / sizeof( double ) ) ) - 3;

    throw QgsProcessingException(
      QObject::tr(
        "Global Thin Plate Spline failed: Input layer contains %1 points, requiring approximately %2 MB of RAM for matrix operations. "
        "Global TPS solving is limited to %3 MB of RAM (approximately %4 points) to prevent memory allocation crashes. "
        "Please use the 'Thin Plate Spline interpolation (local)' algorithm for large point layers."
      )
        .arg( n )
        .arg( QString::number( matrixSizeBytes / ( 1024.0 * 1024.0 ), 'f', 1 ) )
        .arg( MAX_MEMORY_MB )
        .arg( maxPointsAllowed )
    );
  }

  const int systemSize = n + 3;
  QgsMatrixSolver globalSolver( systemSize );
  QVector<double> globalW;

  double meanDist = 0.0;
  for ( int i = 0; i < n; ++i )
  {
    for ( int j = i + 1; j < n; ++j )
    {
      const double dx = globalPoints[i].x - globalPoints[j].x;
      const double dy = globalPoints[i].y - globalPoints[j].y;
      const double distSq = dx * dx + dy * dy;
      const double dist = std::sqrt( distSq );
      const double baseVal = ( distSq > 0.0 ) ? ( distSq * 0.5 * std::log( distSq ) ) : 0.0;
      meanDist += dist * 2.0;
      globalSolver.setValue( i, j, baseVal );
      globalSolver.setValue( j, i, baseVal );
    }
  }
  meanDist /= ( static_cast<double>( n ) * n );

  // Populate regularization diagonal, polynomial terms P, and RHS z-vector
  for ( int i = 0; i < n; ++i )
  {
    globalSolver.setValue( i, i, mRegularization * ( meanDist * meanDist ) );

    globalSolver.setValue( i, n, 1.0 );
    globalSolver.setValue( i, n + 1, globalPoints[i].x );
    globalSolver.setValue( i, n + 2, globalPoints[i].y );

    globalSolver.setValue( n, i, 1.0 );
    globalSolver.setValue( n + 1, i, globalPoints[i].x );
    globalSolver.setValue( n + 2, i, globalPoints[i].y );

    globalSolver.setRightHandSide( i, globalPoints[i].z );
  }
  for ( int i = n; i < n + 3; ++i )
  {
    for ( int j = n; j < n + 3; ++j )
    {
      globalSolver.setValue( i, j, 0.0 );
    }
    globalSolver.setRightHandSide( i, 0.0 );
  }

  multiStepFeedback.pushInfo( QObject::tr( "Solving global linear system (%1 x %1)…" ).arg( systemSize ) );
  // NOTE: SAGA's version of this tool uses Lu solving only, but with a bug in the solver
  // which prevents it correctly flagging singular matrices. Here we use the SVD fallback approach
  // for a more tolerant solver, so that the results more closely represent SAGA's results (i.e.
  // avoiding nodata pixels were SAGA's tools output data pixels)
  if ( !globalSolver.solve( systemSize, globalW, Qgis::LinearMatrixMethod::LuWithSvdFallback ) )
  {
    throw QgsProcessingException( QObject::tr( "Global matrix is singular and could not be solved." ) );
  }

  multiStepFeedback.setCurrentStep( 1 );
  multiStepFeedback.pushInfo( QObject::tr( "Interpolating…" ) );

  int cols = std::max( 1, static_cast<int>( std::ceil( mExtent.width() / mPixelSize ) ) );
  int rows = std::max( 1, static_cast<int>( std::ceil( mExtent.height() / mPixelSize ) ) );

  auto writer = std::make_unique<QgsRasterFileWriter>( mOutputPath );
  writer->setOutputProviderKey( u"gdal"_s );
  if ( !mCreationOptions.isEmpty() )
  {
    writer->setCreationOptions( mCreationOptions.split( '|' ) );
  }
  std::unique_ptr<QgsRasterDataProvider> provider( writer->createOneBandRaster( Qgis::DataType::Float32, cols, rows, mExtent, mSource->sourceCrs() ) );

  if ( !provider )
    throw QgsProcessingException( QObject::tr( "Could not write destination raster file: %1" ).arg( mOutputPath ) );
  if ( !provider->isValid() )
    throw QgsProcessingException( QObject::tr( "Could not create raster output %1: %2" ).arg( mOutputPath, provider->error().message( QgsErrorMessage::Text ) ) );

  provider->setNoDataValue( 1, mNoDataValue );
  provider->setEditable( true );

  QgsRasterIterator iter( provider.get() );
  iter.startRasterRead( 1, cols, rows, mExtent );

  int iterCols = 0;
  int iterRows = 0;
  int topLeftCol = 0;
  int topLeftRow = 0;
  QgsRectangle blockExtent;

  std::unique_ptr<QgsRasterBlock> outputBlock;
  const std::vector<ControlPoint> &constGlobalPoints = std::as_const( globalPoints );
  const double a0 = globalW[n];
  const double ax = globalW[n + 1];
  const double ay = globalW[n + 2];

  while ( iter.readNextRasterPart( 1, iterCols, iterRows, outputBlock, topLeftCol, topLeftRow, &blockExtent ) )
  {
    if ( multiStepFeedback.isCanceled() )
      break;

    QVector<int> tileRowIndices( iterRows );
    std::iota( tileRowIndices.begin(), tileRowIndices.end(), 0 );

    int completedTileRows = 0;

    QtConcurrent::blockingMappedReduced<int>(
      tileRowIndices,
      [iterCols, topLeftRow, topLeftCol, &multiStepFeedback, a0, ax, ay, n, &globalW, &constGlobalPoints, this]( int r ) -> RowResult {
        RowResult rowResult;
        rowResult.r = r;
        rowResult.values.resize( iterCols );

        if ( multiStepFeedback.isCanceled() )
          return rowResult;

        const int globalRow = topLeftRow + r;
        const double y = mExtent.yMaximum() - ( globalRow + 0.5 ) * mPixelSize;

        for ( int c = 0; c < iterCols; ++c )
        {
          const int globalCol = topLeftCol + c;
          const double x = mExtent.xMinimum() + ( globalCol + 0.5 ) * mPixelSize;

          double zVal = a0 + ax * x + ay * y;
          for ( int i = 0; i < n; ++i )
          {
            const double dx = constGlobalPoints[i].x - x;
            const double dy = constGlobalPoints[i].y - y;
            const double distSq = dx * dx + dy * dy;
            if ( distSq > 0.0 )
            {
              zVal += globalW[i] * ( distSq * 0.5 * std::log( distSq ) );
            }
          }
          rowResult.values[c] = zVal;
        }

        return rowResult;
      },
      [&completedTileRows, feedback, iterRows, iterCols, &outputBlock, &iter]( int &, const RowResult &rowResult ) {
        for ( int c = 0; c < iterCols; ++c )
        {
          outputBlock->setValue( rowResult.r, c, rowResult.values[c] );
        }

        completedTileRows++;

        completedTileRows++;
        const double currentBlockProgress = static_cast< double >( completedTileRows ) / iterRows;
        feedback->setProgress( 100.0 * ( 0.10 + 0.90 * iter.progress( 1, currentBlockProgress ) ) );
      }
    );

    if ( multiStepFeedback.isCanceled() )
      break;

    if ( !provider->writeBlock( outputBlock.get(), 1, topLeftCol, topLeftRow ) )
    {
      throw QgsProcessingException( QObject::tr( "Could not write raster block: %1" ).arg( provider->error().summary() ) );
    }
    multiStepFeedback.setProgress( 100.0 * iter.progress( 1 ) );
  }
  provider->setEditable( false );

  iter.stopRasterRead( 1 );

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, mOutputPath );
  return outputs;
}
///@endcond
