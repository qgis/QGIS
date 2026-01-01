/***************************************************************************
                         qgsalgorithmrastercalculator.cpp
                         ---------------------
    begin                : July 2023
    copyright            : (C) 2023 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmrastercalculator.h"

#include "qgsrastercalculator.h"
#include "qgsrasterfilewriter.h"

///@cond PRIVATE

Qgis::ProcessingAlgorithmFlags QgsRasterCalculatorAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::HideFromModeler;
}

QString QgsRasterCalculatorAlgorithm::name() const
{
  return u"rastercalc"_s;
}

QString QgsRasterCalculatorAlgorithm::displayName() const
{
  return QObject::tr( "Raster calculator" );
}

QStringList QgsRasterCalculatorAlgorithm::tags() const
{
  return QObject::tr( "raster,calculator" ).split( ',' );
}

QString QgsRasterCalculatorAlgorithm::group() const
{
  return QObject::tr( "Raster analysis" );
}

QString QgsRasterCalculatorAlgorithm::groupId() const
{
  return u"rasteranalysis"_s;
}

QString QgsRasterCalculatorAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm performs algebraic operations using raster layers." );
}

QString QgsRasterCalculatorAlgorithm::shortDescription() const
{
  return QObject::tr( "Performs algebraic operations using raster layers." );
}

QgsRasterCalculatorAlgorithm *QgsRasterCalculatorAlgorithm::createInstance() const
{
  return new QgsRasterCalculatorAlgorithm();
}

void QgsRasterCalculatorAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMultipleLayers( u"LAYERS"_s, QObject::tr( "Input layers" ), Qgis::ProcessingSourceType::Raster ) );
  addParameter( new QgsProcessingParameterExpression( u"EXPRESSION"_s, QObject::tr( "Expression" ), QVariant(), u"LAYERS"_s, false, Qgis::ExpressionType::RasterCalculator ) );
  auto extentParam = std::make_unique<QgsProcessingParameterExtent>( u"EXTENT"_s, QObject::tr( "Output extent" ), QVariant(), true );
  extentParam->setHelp( QObject::tr( "Extent of the output layer. If not specified, the extent will be the overall extent of all input layers" ) );
  addParameter( extentParam.release() );
  auto cellSizeParam = std::make_unique<QgsProcessingParameterNumber>( u"CELL_SIZE"_s, QObject::tr( "Output cell size (leave empty to set automatically)" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true, 0.0 );
  cellSizeParam->setHelp( QObject::tr( "Cell size of the output layer. If not specified, the smallest cell size from the input layers will be used" ) );
  addParameter( cellSizeParam.release() );
  auto crsParam = std::make_unique<QgsProcessingParameterCrs>( u"CRS"_s, QObject::tr( "Output CRS" ), QVariant(), true );
  crsParam->setHelp( QObject::tr( "CRS of the output layer. If not specified, the CRS of the first input layer will be used" ) );
  addParameter( crsParam.release() );

  auto creationOptsParam = std::make_unique<QgsProcessingParameterString>( u"CREATION_OPTIONS"_s, QObject::tr( "Creation options" ), QVariant(), false, true );
  creationOptsParam->setMetadata( QVariantMap( { { u"widget_wrapper"_s, QVariantMap( { { u"widget_type"_s, u"rasteroptions"_s } } ) } } ) );
  creationOptsParam->setFlags( creationOptsParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( creationOptsParam.release() );

  addParameter( new QgsProcessingParameterRasterDestination( u"OUTPUT"_s, QObject::tr( "Calculated" ) ) );
}

bool QgsRasterCalculatorAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QList<QgsMapLayer *> layers = parameterAsLayerList( parameters, u"LAYERS"_s, context );

  for ( const QgsMapLayer *layer : std::as_const( layers ) )
  {
    QgsMapLayer *clonedLayer { layer->clone() };
    clonedLayer->moveToThread( nullptr );
    mLayers << clonedLayer;
  }

  if ( mLayers.isEmpty() )
  {
    feedback->reportError( QObject::tr( "No layers selected" ), false );
    return false;
  }

  return true;
}

QVariantMap QgsRasterCalculatorAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  for ( QgsMapLayer *layer : std::as_const( mLayers ) )
  {
    layer->moveToThread( QThread::currentThread() );
  }

  QgsCoordinateReferenceSystem crs;
  if ( parameters.value( u"CRS"_s ).isValid() )
  {
    crs = parameterAsCrs( parameters, u"CRS"_s, context );
  }
  else
  {
    crs = mLayers.at( 0 )->crs();
  }

  QgsRectangle bbox;
  if ( parameters.value( u"EXTENT"_s ).isValid() )
  {
    bbox = parameterAsExtent( parameters, u"EXTENT"_s, context, crs );
  }
  else
  {
    bbox = QgsProcessingUtils::combineLayerExtents( mLayers, crs, context );
  }

  double minCellSize = 1e9;

  QVector<QgsRasterCalculatorEntry> entries;
  for ( QgsMapLayer *layer : mLayers )
  {
    QgsRasterLayer *rLayer = qobject_cast<QgsRasterLayer *>( layer );
    if ( !rLayer )
    {
      continue;
    }

    const int nBands = rLayer->dataProvider()->bandCount();
    for ( int i = 0; i < nBands; ++i )
    {
      QgsRasterCalculatorEntry entry;
      entry.ref = u"%1@%2"_s.arg( rLayer->name() ).arg( i + 1 );
      entry.raster = rLayer;
      entry.bandNumber = i + 1;
      entries << entry;
    }

    QgsRectangle ext = rLayer->extent();
    if ( rLayer->crs() != crs )
    {
      QgsCoordinateTransform ct( rLayer->crs(), crs, context.transformContext() );
      ext = ct.transformBoundingBox( ext );
    }

    double cellSize = ( ext.xMaximum() - ext.xMinimum() ) / rLayer->width();
    if ( cellSize < minCellSize )
    {
      minCellSize = cellSize;
    }
  }

  double cellSize = parameterAsDouble( parameters, u"CELL_SIZE"_s, context );
  if ( cellSize == 0 )
  {
    cellSize = minCellSize;
  }

  const QString creationOptions = parameterAsString( parameters, u"CREATION_OPTIONS"_s, context ).trimmed();
  const QString expression = parameterAsExpression( parameters, u"EXPRESSION"_s, context );
  const QString outputFile = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );
  const QString outputFormat = parameterAsOutputRasterFormat( parameters, u"OUTPUT"_s, context );

  double width = std::round( ( bbox.xMaximum() - bbox.xMinimum() ) / cellSize );
  double height = std::round( ( bbox.yMaximum() - bbox.yMinimum() ) / cellSize );

  QgsRasterCalculator calc( expression, outputFile, outputFormat, bbox, crs, width, height, entries, context.transformContext() );
  calc.setCreationOptions( creationOptions.split( '|', Qt::SplitBehaviorFlags::SkipEmptyParts ) );
  QgsRasterCalculator::Result result = calc.processCalculation( feedback );
  qDeleteAll( mLayers );
  mLayers.clear();
  switch ( result )
  {
    case QgsRasterCalculator::Result::CreateOutputError:
      throw QgsProcessingException( QObject::tr( "Error creating output file." ) );
    case QgsRasterCalculator::Result::InputLayerError:
      throw QgsProcessingException( QObject::tr( "Error reading input layer." ) );
    case QgsRasterCalculator::Result::ParserError:
      throw QgsProcessingException( QObject::tr( "Error parsing formula." ) );
    case QgsRasterCalculator::Result::MemoryError:
      throw QgsProcessingException( QObject::tr( "Error allocating memory for result." ) );
    case QgsRasterCalculator::Result::BandError:
      throw QgsProcessingException( QObject::tr( "Invalid band number for input." ) );
    case QgsRasterCalculator::Result::CalculationError:
      throw QgsProcessingException( QObject::tr( "Error occurred while performing calculation." ) );
    default:
      break;
  }

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, outputFile );
  return outputs;
}

Qgis::ProcessingAlgorithmFlags QgsRasterCalculatorModelerAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::HideFromToolbox;
}

QString QgsRasterCalculatorModelerAlgorithm::name() const
{
  return u"modelerrastercalc"_s;
}

QString QgsRasterCalculatorModelerAlgorithm::displayName() const
{
  return QObject::tr( "Raster calculator" );
}

QStringList QgsRasterCalculatorModelerAlgorithm::tags() const
{
  return QObject::tr( "raster,calculator" ).split( ',' );
}

QString QgsRasterCalculatorModelerAlgorithm::group() const
{
  return QObject::tr( "Raster analysis" );
}

QString QgsRasterCalculatorModelerAlgorithm::groupId() const
{
  return u"rasteranalysis"_s;
}

QgsRasterCalculatorModelerAlgorithm *QgsRasterCalculatorModelerAlgorithm::createInstance() const
{
  return new QgsRasterCalculatorModelerAlgorithm();
}

QVariantMap QgsRasterCalculatorModelerAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  for ( QgsMapLayer *layer : std::as_const( mLayers ) )
  {
    layer->moveToThread( QThread::currentThread() );
  }

  QgsCoordinateReferenceSystem crs;
  if ( parameters.value( u"CRS"_s ).isValid() )
  {
    crs = parameterAsCrs( parameters, u"CRS"_s, context );
  }
  else
  {
    crs = mLayers.at( 0 )->crs();
  }

  QgsRectangle bbox;
  if ( parameters.value( u"EXTENT"_s ).isValid() )
  {
    bbox = parameterAsExtent( parameters, u"EXTENT"_s, context, crs );
  }
  else
  {
    bbox = QgsProcessingUtils::combineLayerExtents( mLayers, crs, context );
  }

  double minCellSize = 1e9;

  QVector<QgsRasterCalculatorEntry> entries;
  int n = 0;
  for ( QgsMapLayer *layer : mLayers )
  {
    QgsRasterLayer *rLayer = qobject_cast<QgsRasterLayer *>( layer );
    if ( !rLayer )
    {
      continue;
    }

    n++;
    const int nBands = rLayer->dataProvider()->bandCount();
    for ( int i = 0; i < nBands; ++i )
    {
      QgsRasterCalculatorEntry entry;
      entry.ref = u"%1@%2"_s.arg( indexToName( n ) ).arg( i + 1 );
      entry.raster = rLayer;
      entry.bandNumber = i + 1;
      entries << entry;
    }

    QgsRectangle ext = rLayer->extent();
    if ( rLayer->crs() != crs )
    {
      QgsCoordinateTransform ct( rLayer->crs(), crs, context.transformContext() );
      ext = ct.transformBoundingBox( ext );
    }

    double cellSize = ( ext.xMaximum() - ext.xMinimum() ) / rLayer->width();
    if ( cellSize < minCellSize )
    {
      minCellSize = cellSize;
    }
  }

  double cellSize = parameterAsDouble( parameters, u"CELL_SIZE"_s, context );
  if ( cellSize == 0 )
  {
    cellSize = minCellSize;
  }

  const QString expression = parameterAsExpression( parameters, u"EXPRESSION"_s, context );
  const QString outputFile = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );
  const QString outputFormat = parameterAsOutputRasterFormat( parameters, u"OUTPUT"_s, context );

  double width = std::round( ( bbox.xMaximum() - bbox.xMinimum() ) / cellSize );
  double height = std::round( ( bbox.yMaximum() - bbox.yMinimum() ) / cellSize );

  QgsRasterCalculator calc( expression, outputFile, outputFormat, bbox, crs, width, height, entries, context.transformContext() );
  QgsRasterCalculator::Result result = calc.processCalculation( feedback );
  qDeleteAll( mLayers );
  mLayers.clear();
  switch ( result )
  {
    case QgsRasterCalculator::Result::CreateOutputError:
      throw QgsProcessingException( QObject::tr( "Error creating output file." ) );
    case QgsRasterCalculator::Result::InputLayerError:
      throw QgsProcessingException( QObject::tr( "Error reading input layer." ) );
    case QgsRasterCalculator::Result::ParserError:
      throw QgsProcessingException( QObject::tr( "Error parsing formula." ) );
    case QgsRasterCalculator::Result::MemoryError:
      throw QgsProcessingException( QObject::tr( "Error allocating memory for result." ) );
    case QgsRasterCalculator::Result::BandError:
      throw QgsProcessingException( QObject::tr( "Invalid band number for input." ) );
    case QgsRasterCalculator::Result::CalculationError:
      throw QgsProcessingException( QObject::tr( "Error occurred while performing calculation." ) );
    default:
      break;
  }

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, outputFile );
  return outputs;
}

QString QgsRasterCalculatorModelerAlgorithm::indexToName( int index ) const
{
  QString name;
  int div = index;
  int mod = 0;

  while ( div > 0 )
  {
    mod = ( div - 1 ) % 26;
    name = static_cast<char>( 65 + mod ) + name;
    div = ( int ) ( ( div - mod ) / 26 );
  }
  return name;
}

///@endcond
