/***************************************************************************
                         qgsalgorithmfillsinkswangliu.cpp
                         ---------------------
    begin                : April 2025
    copyright            : (C) 2025 by Nyall Dawson
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

#include "qgsalgorithmfillsinkswangliu.h"

#include "qgsrasterfilewriter.h"

#include <queue>

///@cond PRIVATE

QString QgsFillSinksWangLiuAlgorithm::name() const
{
  return u"fillsinkswangliu"_s;
}

QString QgsFillSinksWangLiuAlgorithm::displayName() const
{
  return QObject::tr( "Fill sinks (Wang & Liu)" );
}

QStringList QgsFillSinksWangLiuAlgorithm::tags() const
{
  return QObject::tr( "fill,filter,slope,dsm,dtm,terrain,water,shed,basin,direction,flow" ).split( ',' );
}

QString QgsFillSinksWangLiuAlgorithm::group() const
{
  return QObject::tr( "Raster terrain analysis" );
}

QString QgsFillSinksWangLiuAlgorithm::groupId() const
{
  return u"rasterterrainanalysis"_s;
}

QString QgsFillSinksWangLiuAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm uses a method proposed by Wang & Liu to identify and fill surface depressions in digital elevation models.\n\n"

                      "The method was enhanced to allow the creation of hydrologically sound elevation models, i.e. not only to fill the depression(s) "
                      "but also to preserve a downward slope along the flow path. If desired, this is accomplished by preserving a minimum slope "
                      "gradient (and thus elevation difference) between cells.\n\n"

                      "References: Wang, L. & H. Liu (2006): An efficient method for identifying and filling surface depressions in digital elevation models for hydrologic analysis and modelling. International Journal of Geographical Information Science, Vol. 20, No. 2: 193-213.\n\n"

                      "This algorithm is a port of the SAGA 'Fill Sinks (Wang & Liu)' tool." );
}

QString QgsFillSinksWangLiuAlgorithm::shortDescription() const
{
  return QObject::tr( "Identifies and fills surface depressions in digital elevation models using a method proposed by Wang & Liu." );
}

void QgsFillSinksWangLiuAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( u"INPUT"_s, QObject::tr( "Input layer" ) ) );

  addParameter( new QgsProcessingParameterBand( u"BAND"_s, QObject::tr( "Band number" ), 1, u"INPUT"_s ) );

  auto minSlopeParam = std::make_unique<QgsProcessingParameterNumber>( u"MIN_SLOPE"_s, QObject::tr( "Minimum slope (degrees)" ), Qgis::ProcessingNumberParameterType::Double, 0.1, false, 0 );
  minSlopeParam->setHelp( QObject::tr( "Minimum slope gradient to preserve from cell to cell. With a value of zero sinks are filled up to the spill elevation (which results in flat areas). Units are degrees." ) );
  addParameter( minSlopeParam.release() );

  auto createOptsParam = std::make_unique<QgsProcessingParameterString>( u"CREATION_OPTIONS"_s, QObject::tr( "Creation options" ), QVariant(), false, true );
  createOptsParam->setMetadata( QVariantMap( { { u"widget_wrapper"_s, QVariantMap( { { u"widget_type"_s, u"rasteroptions"_s } } ) } } ) );
  createOptsParam->setFlags( createOptsParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( createOptsParam.release() );

  auto outputFilledDem = std::make_unique<QgsProcessingParameterRasterDestination>( u"OUTPUT_FILLED_DEM"_s, QObject::tr( "Output layer (filled DEM)" ), QVariant(), true, true );
  outputFilledDem->setHelp( QObject::tr( "Depression-free digital elevation model." ) );
  addParameter( outputFilledDem.release() );

  auto outputFlowDirections = std::make_unique<QgsProcessingParameterRasterDestination>( u"OUTPUT_FLOW_DIRECTIONS"_s, QObject::tr( "Output layer (flow directions)" ), QVariant(), true, false );
  outputFlowDirections->setHelp( QObject::tr( "Computed flow directions, 0=N, 1=NE, 2=E, ... 7=NW." ) );
  addParameter( outputFlowDirections.release() );

  auto outputWatershedBasins = std::make_unique<QgsProcessingParameterRasterDestination>( u"OUTPUT_WATERSHED_BASINS"_s, QObject::tr( "Output layer (watershed basins)" ), QVariant(), true, false );
  outputWatershedBasins->setHelp( QObject::tr( "Delineated watershed basin." ) );
  addParameter( outputWatershedBasins.release() );
}

QgsFillSinksWangLiuAlgorithm *QgsFillSinksWangLiuAlgorithm::createInstance() const
{
  return new QgsFillSinksWangLiuAlgorithm();
}

bool QgsFillSinksWangLiuAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsRasterLayer *layer = parameterAsRasterLayer( parameters, u"INPUT"_s, context );
  if ( !layer )
    throw QgsProcessingException( invalidRasterError( parameters, u"INPUT"_s ) );

  const int band = parameterAsInt( parameters, u"BAND"_s, context );

  mBand = parameterAsInt( parameters, u"BAND"_s, context );
  if ( mBand < 1 || mBand > layer->bandCount() )
    throw QgsProcessingException( QObject::tr( "Invalid band number for BAND (%1): Valid values for input raster are 1 to %2" ).arg( mBand ).arg( layer->bandCount() ) );

  mInterface.reset( layer->dataProvider()->clone() );
  mHasNoDataValue = layer->dataProvider()->sourceHasNoDataValue( band );
  mLayerWidth = layer->width();
  mLayerHeight = layer->height();
  mExtent = layer->extent();
  mCrs = layer->crs();
  mRasterUnitsPerPixelX = layer->rasterUnitsPerPixelX();
  mRasterUnitsPerPixelY = layer->rasterUnitsPerPixelY();
  mRasterDiagonal = std::sqrt( mRasterUnitsPerPixelX * mRasterUnitsPerPixelX + mRasterUnitsPerPixelY * mRasterUnitsPerPixelY );
  mDataType = layer->dataProvider()->dataType( mBand );
  mNoData = layer->dataProvider()->sourceNoDataValue( mBand );
  mDirectionalLengths = { mRasterUnitsPerPixelY, mRasterDiagonal, mRasterUnitsPerPixelX, mRasterDiagonal, mRasterUnitsPerPixelY, mRasterDiagonal, mRasterUnitsPerPixelX, mRasterDiagonal };
  return true;
}

static constexpr std::array< int, 8 > COL_DIRECTION_OFFSETS { 0, 1, 1, 1, 0, -1, -1, -1 };
static constexpr std::array< int, 8 > ROW_DIRECTION_OFFSETS { -1, -1, 0, 1, 1, 1, 0, -1 };

bool QgsFillSinksWangLiuAlgorithm::isInGrid( int row, int col ) const
{
  return col >= 0 && col < mLayerWidth && row >= 0 && row < mLayerHeight;
}

QgsFillSinksWangLiuAlgorithm::Direction QgsFillSinksWangLiuAlgorithm::getDir( int row, int col, double z, const QgsRasterBlock *filled ) const
{
  Direction steepestDirection = Invalid;
  double maxGradient = 0;
  bool isNoData = false;

  for ( Direction direction : { North, NorthEast, East, SouthEast, South, SouthWest, West, NorthWest } )
  {
    const int neighborCol = col + COL_DIRECTION_OFFSETS[direction];
    const int neighborRow = row + ROW_DIRECTION_OFFSETS[direction];

    if ( isInGrid( neighborRow, neighborCol ) )
    {
      const double neighborZ = filled->valueAndNoData( neighborRow, neighborCol, isNoData );
      if ( !isNoData && neighborZ < z )
      {
        const double gradient = ( z - neighborZ ) / mDirectionalLengths[direction];
        if ( gradient >= maxGradient )
        {
          maxGradient = gradient;
          steepestDirection = direction;
        }
      }
    }
  }

  return steepestDirection;
}

struct CFillSinks_WL_Node
{
    int row = 0;
    int col = 0;
    double spill = 0;
};

class CompareGreater
{
  public:
    bool operator()( CFillSinks_WL_Node n1, CFillSinks_WL_Node n2 ) const
    {
      return n1.spill > n2.spill;
    }
};

typedef std::vector< CFillSinks_WL_Node > nodeVector;
typedef std::priority_queue< CFillSinks_WL_Node, nodeVector, CompareGreater > PriorityQ;

QVariantMap QgsFillSinksWangLiuAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QString createOptions = parameterAsString( parameters, u"CREATION_OPTIONS"_s, context ).trimmed();

  const QString filledDemOutputFile = parameterAsOutputLayer( parameters, u"OUTPUT_FILLED_DEM"_s, context );
  const QString flowDirectionsOutputFile = parameterAsOutputLayer( parameters, u"OUTPUT_FLOW_DIRECTIONS"_s, context );
  const QString watershedBasinsOutputFile = parameterAsOutputLayer( parameters, u"OUTPUT_WATERSHED_BASINS"_s, context );

  std::unique_ptr<QgsRasterFileWriter> filledDemWriter;
  std::unique_ptr<QgsRasterDataProvider> filledDemDestProvider;

  if ( !filledDemOutputFile.isEmpty() )
  {
    const QString outputFormat = parameterAsOutputRasterFormat( parameters, u"OUTPUT_FILLED_DEM"_s, context );

    filledDemWriter = std::make_unique<QgsRasterFileWriter>( filledDemOutputFile );
    filledDemWriter->setOutputProviderKey( u"gdal"_s );
    if ( !createOptions.isEmpty() )
    {
      filledDemWriter->setCreationOptions( createOptions.split( '|' ) );
    }
    filledDemWriter->setOutputFormat( outputFormat );

    filledDemDestProvider.reset( filledDemWriter->createOneBandRaster( mDataType, mLayerWidth, mLayerHeight, mExtent, mCrs ) );

    if ( !filledDemDestProvider )
      throw QgsProcessingException( QObject::tr( "Could not create raster output: %1" ).arg( filledDemOutputFile ) );
    if ( !filledDemDestProvider->isValid() )
      throw QgsProcessingException( QObject::tr( "Could not create raster output %1: %2" ).arg( filledDemOutputFile, filledDemDestProvider->error().message( QgsErrorMessage::Text ) ) );

    filledDemDestProvider->setNoDataValue( 1, mNoData );
    filledDemDestProvider->setEditable( true );
  }

  std::unique_ptr<QgsRasterFileWriter> flowDirectionsWriter;
  std::unique_ptr<QgsRasterDataProvider> flowDirectionsDestProvider;

  if ( !flowDirectionsOutputFile.isEmpty() )
  {
    const QString outputFormat = parameterAsOutputRasterFormat( parameters, u"OUTPUT_FLOW_DIRECTIONS"_s, context );

    flowDirectionsWriter = std::make_unique<QgsRasterFileWriter>( flowDirectionsOutputFile );
    flowDirectionsWriter->setOutputProviderKey( u"gdal"_s );
    flowDirectionsWriter->setOutputFormat( outputFormat );

    flowDirectionsDestProvider.reset( flowDirectionsWriter->createOneBandRaster( Qgis::DataType::Byte, mLayerWidth, mLayerHeight, mExtent, mCrs ) );

    if ( !flowDirectionsDestProvider )
      throw QgsProcessingException( QObject::tr( "Could not create raster output: %1" ).arg( flowDirectionsOutputFile ) );
    if ( !flowDirectionsDestProvider->isValid() )
      throw QgsProcessingException( QObject::tr( "Could not create raster output %1: %2" ).arg( flowDirectionsOutputFile, flowDirectionsDestProvider->error().message( QgsErrorMessage::Text ) ) );

    flowDirectionsDestProvider->setNoDataValue( 1, 255 );
    flowDirectionsDestProvider->setEditable( true );
  }

  std::unique_ptr<QgsRasterFileWriter> watershedBasinsWriter;
  std::unique_ptr<QgsRasterDataProvider> watershedBasinsDestProvider;

  if ( !watershedBasinsOutputFile.isEmpty() )
  {
    const QString outputFormat = parameterAsOutputRasterFormat( parameters, u"OUTPUT_WATERSHED_BASINS"_s, context );

    watershedBasinsWriter = std::make_unique<QgsRasterFileWriter>( watershedBasinsOutputFile );
    watershedBasinsWriter->setOutputProviderKey( u"gdal"_s );
    watershedBasinsWriter->setOutputFormat( outputFormat );

    watershedBasinsDestProvider.reset( watershedBasinsWriter->createOneBandRaster( Qgis::DataType::Int32, mLayerWidth, mLayerHeight, mExtent, mCrs ) );

    if ( !watershedBasinsDestProvider )
      throw QgsProcessingException( QObject::tr( "Could not create raster output: %1" ).arg( watershedBasinsOutputFile ) );
    if ( !watershedBasinsDestProvider->isValid() )
      throw QgsProcessingException( QObject::tr( "Could not create raster output %1: %2" ).arg( watershedBasinsOutputFile, watershedBasinsDestProvider->error().message( QgsErrorMessage::Text ) ) );

    watershedBasinsDestProvider->setNoDataValue( 1, -1 );
    watershedBasinsDestProvider->setEditable( true );
  }

  std::unique_ptr< QgsRasterBlock > sourceDemData( mInterface->block( mBand, mExtent, mLayerWidth, mLayerHeight ) );
  if ( !sourceDemData )
  {
    throw QgsProcessingException( QObject::tr( "Could not read DEM raster" ) );
  }

  auto filledDemData = std::make_unique<QgsRasterBlock>( mDataType, mLayerWidth, mLayerHeight );
  filledDemData->setNoDataValue( mNoData );
  filledDemData->setIsNoData();

  auto watershedData = std::make_unique<QgsRasterBlock>( Qgis::DataType::Int32, mLayerWidth, mLayerHeight );
  watershedData->setNoDataValue( -1 );
  watershedData->setIsNoData();

  auto outputFlowData = std::make_unique<QgsRasterBlock>( Qgis::DataType::Byte, mLayerWidth, mLayerHeight );
  outputFlowData->setNoDataValue( 255 );
  outputFlowData->setIsNoData();

  auto seedData = std::make_unique<QgsRasterBlock>( Qgis::DataType::Byte, mLayerWidth, mLayerHeight );
  seedData->fill( 0 );

  double minSlope = parameterAsDouble( parameters, u"MIN_SLOPE"_s, context );
  double mindiff[8];
  bool preserve = false;
  if ( minSlope > 0.0 )
  {
    minSlope = tan( minSlope * M_PI / 180.0 );
    for ( int i = 0; i < 8; i++ )
      mindiff[i] = minSlope * mDirectionalLengths[i];
    preserve = true;
  }

  // fill priority queue with boundary, i.e. seed cells
  CFillSinks_WL_Node tempNode;
  PriorityQ theQueue;

  long long id = 0;
  double value = 0;
  bool isNoData = false;
  feedback->setProgressText( QObject::tr( "Seed boundary cells" ) );

  std::size_t processed = 0;
  const std::size_t totalCells = static_cast< std::size_t >( mLayerWidth ) * mLayerHeight;

  for ( int row = 0; row < mLayerHeight; row++ )
  {
    if ( feedback->isCanceled() )
      break;

    for ( int col = 0; col < mLayerWidth; col++ )
    {
      value = sourceDemData->valueAndNoData( row, col, isNoData );
      if ( !isNoData )
      {
        for ( Direction direction : { North, NorthEast, East, SouthEast, South, SouthWest, West, NorthWest } )
        {
          int iCol = col + COL_DIRECTION_OFFSETS[direction];
          int iRow = row + ROW_DIRECTION_OFFSETS[direction];
          ;
          if ( !isInGrid( iRow, iCol ) || sourceDemData->isNoData( iRow, iCol ) )
          {
            const double z = value;
            filledDemData->setValue( row, col, z );
            seedData->setValue( row, col, 1.0 );
            watershedData->setValue( row, col, static_cast< double >( id ) );
            id += 1;

            tempNode.row = row;
            tempNode.col = col;
            tempNode.spill = z;
            theQueue.push( tempNode );
            processed += 1;
            break;
          }
        }
      }
      feedback->setProgress( static_cast< double >( processed ) / static_cast< double >( totalCells ) * 100 );
    }
  }

  if ( feedback->isCanceled() )
    return {};

  // work through least cost path
  feedback->setProgressText( QObject::tr( "Filling using least cost paths" ) );

  while ( !theQueue.empty() )
  {
    PriorityQ::value_type tempNode = theQueue.top();

    const int row = tempNode.row;
    const int col = tempNode.col;
    const double z = tempNode.spill;
    theQueue.pop();

    const long long id = static_cast< long long >( watershedData->value( row, col ) );

    for ( Direction direction : { North, NorthEast, East, SouthEast, South, SouthWest, West, NorthWest } )
    {
      const int iCol = col + COL_DIRECTION_OFFSETS[direction];
      const int iRow = row + ROW_DIRECTION_OFFSETS[direction];
      isNoData = false;
      const bool iInGrid = isInGrid( iRow, iCol );
      double iz = iInGrid ? sourceDemData->valueAndNoData( iRow, iCol, isNoData ) : 0;
      if ( iInGrid && !isNoData )
      {
        if ( filledDemData->isNoData( iRow, iCol ) )
        {
          if ( preserve )
          {
            iz = std::max( iz, z + mindiff[static_cast< int >( direction )] );
          }
          else if ( iz <= z )
          {
            iz = z;
            outputFlowData->setValue( iRow, iCol, INVERSE_DIRECTION[static_cast< int >( direction )] );
          }

          tempNode.row = iRow;
          tempNode.col = iCol;
          tempNode.spill = iz;
          theQueue.push( tempNode );

          filledDemData->setValue( iRow, iCol, iz );
          watershedData->setValue( iRow, iCol, id );
          processed += 1;
        }
        else if ( seedData->value( iRow, iCol ) == 1 )
        {
          watershedData->setValue( iRow, iCol, id );
        }
      }
    }

    if ( outputFlowData->isNoData( row, col ) )
      outputFlowData->setValue( row, col, getDir( row, col, z, filledDemData.get() ) );

    feedback->setProgress( static_cast< double >( processed ) / static_cast< double >( totalCells ) * 100 );
    if ( feedback->isCanceled() )
      break;
  }

  if ( feedback->isCanceled() )
    return {};

  QVariantMap outputs;

  if ( filledDemDestProvider )
  {
    if ( !filledDemDestProvider->writeBlock( filledDemData.get(), 1, 0, 0 ) )
    {
      throw QgsProcessingException( QObject::tr( "Could not write raster block: %1" ).arg( filledDemDestProvider->error().summary() ) );
    }
    filledDemDestProvider->setEditable( false );
    outputs.insert( u"OUTPUT_FILLED_DEM"_s, filledDemOutputFile );
  }
  if ( flowDirectionsDestProvider )
  {
    if ( !flowDirectionsDestProvider->writeBlock( outputFlowData.get(), 1, 0, 0 ) )
    {
      throw QgsProcessingException( QObject::tr( "Could not write raster block: %1" ).arg( flowDirectionsDestProvider->error().summary() ) );
    }
    flowDirectionsDestProvider->setEditable( false );
    outputs.insert( u"OUTPUT_FLOW_DIRECTIONS"_s, flowDirectionsOutputFile );
  }
  if ( watershedBasinsDestProvider )
  {
    if ( !watershedBasinsDestProvider->writeBlock( watershedData.get(), 1, 0, 0 ) )
    {
      throw QgsProcessingException( QObject::tr( "Could not write raster block: %1" ).arg( watershedBasinsDestProvider->error().summary() ) );
    }
    watershedBasinsDestProvider->setEditable( false );
    outputs.insert( u"OUTPUT_WATERSHED_BASINS"_s, watershedBasinsOutputFile );
  }

  return outputs;
}


///@endcond
