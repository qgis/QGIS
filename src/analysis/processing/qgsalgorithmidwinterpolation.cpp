/***************************************************************************
                         qgsalgorithmidwinterpolation.cpp
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

#include "qgsalgorithmidwinterpolation.h"

#include "qgsgridfilewriter.h"
#include "qgsidwinterpolator.h"
#include "qgsprocessingparameterinterpolationpixelsize.h"
#include "qgsprocessingparameterinterpolationsource.h"
#include "qgsprocessingutils.h"

#include <QString>

using namespace Qt::StringLiterals;

///@cond PRIVATE
QgsIdwInterpolationAlgorithm::QgsIdwInterpolationAlgorithm() = default;

QString QgsIdwInterpolationAlgorithm::name() const
{
  return u"idwinterpolation"_s;
}

QString QgsIdwInterpolationAlgorithm::displayName() const
{
  return QObject::tr( "IDW interpolation" );
}

QStringList QgsIdwInterpolationAlgorithm::tags() const
{
  return QObject::tr( "idw,inverse,distance,weighted,interpolation,surface,grid,breaklines,breaks,structures" ).split( ',' );
}

QString QgsIdwInterpolationAlgorithm::group() const
{
  return QObject::tr( "Interpolation" );
}

QString QgsIdwInterpolationAlgorithm::groupId() const
{
  return u"interpolation"_s;
}

QString QgsIdwInterpolationAlgorithm::shortDescription() const
{
  return QObject::tr( "Generates an Inverse Distance Weighted (IDW) interpolation from vector layers." );
}

QString QgsIdwInterpolationAlgorithm::shortHelpString() const
{
  return QObject::tr(
    "This algorithm generates an Inverse Distance Weighted (IDW) interpolation surface raster from one or more vector layers.\n\n"
    "IDW calculates cell values using a linearly weighted combination of sample points. The weighting is an inverse function of "
    "distance, meaning closer sample points exert greater influence on the target cell value than more distant points.\n\n"
    "Input layers can supply sample values from feature attributes or feature Z coordinates. Features can be specified "
    "as discrete points, structure lines, or breaklines."
  );
}

QgsIdwInterpolationAlgorithm *QgsIdwInterpolationAlgorithm::createInstance() const
{
  return new QgsIdwInterpolationAlgorithm();
}

void QgsIdwInterpolationAlgorithm::initAlgorithm( const QVariantMap & )
{
  auto dataParam = std::make_unique<QgsProcessingParameterInterpolationSource>( u"INTERPOLATION_DATA"_s, QObject::tr( "Input layer(s)" ) );
  dataParam->setHelp( QObject::tr( "Vector layers to use for interpolation along with their attributes and source types." ) );
  addParameter( dataParam.release() );

  auto distanceParam
    = std::make_unique<QgsProcessingParameterNumber>( u"DISTANCE_COEFFICIENT"_s, QObject::tr( "Distance coefficient P" ), Qgis::ProcessingNumberParameterType::Double, 2.0, false, 0.0, 99.99 );
  distanceParam->setHelp( QObject::tr( "Distance coefficient determines how influence decreases with distance." ) );
  addParameter( distanceParam.release() );

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

QVariantMap QgsIdwInterpolationAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QString interpolationData = parameterAsString( parameters, u"INTERPOLATION_DATA"_s, context );

  const double coefficient = parameterAsDouble( parameters, u"DISTANCE_COEFFICIENT"_s, context );
  const QgsRectangle boundingBox = parameterAsExtent( parameters, u"EXTENT"_s, context );
  const double pixelSize = parameterAsDouble( parameters, u"PIXEL_SIZE"_s, context );
  const QString creationOptions = parameterAsString( parameters, u"CREATION_OPTIONS"_s, context ).trimmed();
  const double outputNodata = parameterAsDouble( parameters, u"NODATA"_s, context );
  const QString output = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );

  int columns = parameterAsInt( parameters, u"COLUMNS"_s, context );
  int rows = parameterAsInt( parameters, u"ROWS"_s, context );
  if ( columns == 0 )
  {
    columns = std::max( static_cast<int>( std::ceil( boundingBox.width() / pixelSize ) ), 1 );
  }
  if ( rows == 0 )
  {
    rows = std::max( static_cast<int>( std::ceil( boundingBox.height() / pixelSize ) ), 1 );
  }

  if ( interpolationData.isEmpty() )
  {
    throw QgsProcessingException( QObject::tr( "At least one input layer must be specified." ) );
  }

  QList<QgsInterpolator::LayerData> layerDataList;
  std::vector<std::unique_ptr<QgsFeatureSource>> sourceHolders;

  const QStringList layerRows = interpolationData.split( "::|::"_L1, Qt::SkipEmptyParts );
  for ( const QString &row : std::as_const( layerRows ) )
  {
    const QStringList tokens = row.split( "::~::"_L1 );
    if ( tokens.size() < 4 )
    {
      continue;
    }

    QgsInterpolator::LayerData data;
    std::unique_ptr<QgsProcessingFeatureSource> source( QgsProcessingUtils::variantToSource( tokens.at( 0 ), context ) );

    if ( !source )
    {
      throw QgsProcessingException( QObject::tr( "Could not load source layer for input %1." ).arg( tokens.at( 0 ) ) );
    }

    data.source = source.get();
    data.transformContext = context.transformContext();

    data.valueSource = static_cast<Qgis::InterpolationValueSource>( tokens.at( 1 ).toInt() );

    bool intOk = false;
    data.interpolationAttribute = tokens.at( 2 ).toInt( &intOk );

    if ( data.valueSource == Qgis::InterpolationValueSource::Attribute )
    {
      if ( !intOk )
      {
        data.interpolationAttribute = source->fields().lookupField( tokens.at( 2 ) );
        if ( data.interpolationAttribute < 0 )
        {
          throw QgsProcessingException( QObject::tr( "Field %1 does not exist in layer %2." ).arg( tokens.at( 2 ), source->sourceName() ) );
        }
      }
      else if ( data.interpolationAttribute < 0 )
      {
        throw QgsProcessingException( QObject::tr( "Layer %1 is set to use a value attribute, but no attribute was set." ).arg( source->sourceName() ) );
      }
      else if ( data.interpolationAttribute >= source->fields().size() )
      {
        throw QgsProcessingException( QObject::tr( "Layer %1 is set to use an invalid attribute." ).arg( source->sourceName() ) );
      }
    }

    data.sourceType = static_cast<Qgis::InterpolationSourceType>( tokens.at( 3 ).toInt() );

    layerDataList.append( data );
    sourceHolders.push_back( std::move( source ) );
  }

  QgsIDWInterpolator interpolator( layerDataList );
  interpolator.setDistanceCoefficient( coefficient );

  QgsGridFileWriter writer( &interpolator, output, boundingBox, columns, rows );
  if ( !creationOptions.isEmpty() )
  {
    writer.setCreationOptions( creationOptions.split( '|' ) );
  }
  writer.setNoDataValue( outputNodata );

  writer.writeFile( feedback );

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, output );
  return outputs;
}
///@endcond
