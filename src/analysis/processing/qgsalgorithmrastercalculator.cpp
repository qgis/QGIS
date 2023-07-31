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
#include "qgsrasterfilewriter.h"
#include "qgsrastercalculator.h"

///@cond PRIVATE

QString QgsRasterCalculatorAlgorithm::name() const
{
  return QStringLiteral( "rastercalc" );
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
  return QStringLiteral( "rasteranalysis" );
}

QString QgsRasterCalculatorAlgorithm::shortHelpString() const
{
  return QObject::tr( "Performing algebraic operations using raster layers." );
}

QgsRasterCalculatorAlgorithm *QgsRasterCalculatorAlgorithm::createInstance() const
{
  return new QgsRasterCalculatorAlgorithm();
}

void QgsRasterCalculatorAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMultipleLayers( QStringLiteral( "INPUT" ), QObject::tr( "Input layers" ), QgsProcessing::SourceType::TypeRaster ) );
  addParameter( new QgsProcessingParameterExpression( QStringLiteral( "EXPRESSION" ), QObject::tr( "Expression" ), QVariant(),  QStringLiteral( "INPUT" ), false, Qgis::ExpressionType::RasterCalculator ) );
  addParameter( new QgsProcessingParameterExtent( QStringLiteral( "EXTENT" ), QObject::tr( "Output extent" ), QVariant(), true ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "CELL_SIZE" ), QObject::tr( "Output cell size (leave empty to set automatically)" ), QgsProcessingParameterNumber::Double, QVariant(), true, 0.0 ) );
  addParameter( new QgsProcessingParameterCrs( QStringLiteral( "CRS" ), QObject::tr( "Output CRS" ), QVariant(), true ) );
  addParameter( new QgsProcessingParameterRasterDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Calculated" ) ) );
}

bool QgsRasterCalculatorAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QList< QgsMapLayer * > layers = parameterAsLayerList( parameters, QStringLiteral( "INPUT" ), context );

  for ( const QgsMapLayer *layer : std::as_const( layers ) )
  {
    QgsMapLayer *clonedLayer { layer->clone() };
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
  QgsCoordinateReferenceSystem crs;
  if ( parameters.value( QStringLiteral( "CRS" ) ).isValid() )
  {
    crs = parameterAsCrs( parameters, QStringLiteral( "CRS" ), context );
  }
  else
  {
    crs = mLayers.at( 0 )->crs();
  }

  QgsRectangle bbox;
  if ( parameters.value( QStringLiteral( "EXTENT" ) ).isValid() )
  {
    bbox = parameterAsExtent( parameters, QStringLiteral( "EXTENT" ), context, crs );
  }
  else
  {
    bbox = QgsProcessingUtils::combineLayerExtents( mLayers, crs, context );
  }

  double minCellSize = 1e9;

  QVector< QgsRasterCalculatorEntry > entries;
  for ( QgsMapLayer *layer : mLayers )
  {
    QgsRasterLayer *rLayer = static_cast<QgsRasterLayer *>( layer );
    if ( !rLayer )
    {
      continue;
    }

    const int nBands = rLayer->dataProvider()->bandCount();
    for ( int i = 0; i < nBands; ++i )
    {
      QgsRasterCalculatorEntry entry;
      entry.ref = QStringLiteral( "%1@%2" ).arg( rLayer->name() ).arg( i + 1 );
      entry.raster = rLayer;
      entry.bandNumber = i + 1;
      entries << entry;
    }

    QgsRectangle ext = rLayer->extent();
    if ( rLayer->crs().authid() != crs.authid() )
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

  double cellSize = parameterAsDouble( parameters, QStringLiteral( "CELL_SIZE" ), context );
  if ( cellSize == 0 )
  {
    cellSize = minCellSize;
  }

  const QString expression = parameterAsExpression( parameters, QStringLiteral( "EXPRESSION" ), context );
  const QString outputFile = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  const QFileInfo fi( outputFile );
  const QString outputFormat = QgsRasterFileWriter::driverForExtension( fi.suffix() );

  double width = std::round( ( bbox.xMaximum() - bbox.xMinimum() ) / cellSize );
  double height = std::round( ( bbox.yMaximum() - bbox.yMinimum() ) / cellSize );

  QgsRasterCalculator calc( expression, outputFile, outputFormat, bbox, crs, width, height, entries, context.transformContext() );
  QgsRasterCalculator::Result result = calc.processCalculation( feedback );
  switch ( result )
  {
    case QgsRasterCalculator::CreateOutputError:
      throw QgsProcessingException( QObject::tr( "Error creating output file." ) );
      break;
    case QgsRasterCalculator::InputLayerError:
      throw QgsProcessingException( QObject::tr( "Error reading input layer." ) );
      break;
    case QgsRasterCalculator::ParserError:
      throw QgsProcessingException( QObject::tr( "Error parsing formula." ) );
      break;
    case QgsRasterCalculator::MemoryError:
      throw QgsProcessingException( QObject::tr( "Error allocating memory for result." ) );
      break;
    case QgsRasterCalculator::BandError:
      throw QgsProcessingException( QObject::tr( "Invalid band number for input." ) );
      break;
    case QgsRasterCalculator::CalculationError:
      throw QgsProcessingException( QObject::tr( "Error occurred while performing calculation." ) );
      break;
    default:
      break;
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), outputFile );
  return outputs;
}

///@endcond

