/***************************************************************************
                         qgsalgorithmheatmap.cpp
                         ---------------------
    begin                : June 2026
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

#include "qgsalgorithmheatmap.h"

#include "qgsapplication.h"
#include "qgskde.h"
#include "qgsprocessingparameterheatmappixelsize.h"

#include <QString>

using namespace Qt::StringLiterals;

///@cond PRIVATE


QString QgsHeatmapAlgorithm::name() const
{
  return u"heatmapkerneldensityestimation"_s;
}

QString QgsHeatmapAlgorithm::displayName() const
{
  return QObject::tr( "Heatmap (Kernel Density Estimation)" );
}

QStringList QgsHeatmapAlgorithm::tags() const
{
  return QObject::tr( "heatmap,kde,hotspot" ).split( ',' );
}

QString QgsHeatmapAlgorithm::group() const
{
  return QObject::tr( "Interpolation" );
}

QString QgsHeatmapAlgorithm::groupId() const
{
  return u"interpolation"_s;
}

QIcon QgsHeatmapAlgorithm::icon() const
{
  return QgsApplication::getThemeIcon( u"/heatmap.svg"_s );
}

QgsHeatmapAlgorithm::~QgsHeatmapAlgorithm() = default;

void QgsHeatmapAlgorithm::initAlgorithm( const QVariantMap & )
{
  auto inputParam = std::make_unique<QgsProcessingParameterFeatureSource>( u"INPUT"_s, QObject::tr( "Point layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint ) );
  inputParam->setHelp( QObject::tr( "The point vector layer to calculate the heatmap from." ) );
  addParameter( inputParam.release() );

  auto radiusParam = std::make_unique<QgsProcessingParameterDistance>( u"RADIUS"_s, QObject::tr( "Radius" ), 100.0, u"INPUT"_s, false, 0.0 );
  radiusParam->setHelp( QObject::tr( "The search radius to use for calculating the heatmap. This represents the distance around each point that will be considered when calculating the density." ) );
  addParameter( radiusParam.release() );

  auto radiusFieldParam
    = std::make_unique<QgsProcessingParameterField>( u"RADIUS_FIELD"_s, QObject::tr( "Radius from field" ), QVariant(), u"INPUT"_s, Qgis::ProcessingFieldParameterDataType::Numeric, false, true );
  radiusFieldParam->setFlags( radiusFieldParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  radiusFieldParam->setHelp( QObject::tr( "An optional numeric field from the input layer which specifies a per-feature radius to use instead of the global radius parameter." ) );
  addParameter( radiusFieldParam.release() );

  auto pixelSizeParam = std::make_unique<QgsProcessingParameterHeatmapPixelSize>( u"PIXEL_SIZE"_s, QObject::tr( "Output raster size" ), u"INPUT"_s, u"RADIUS"_s, u"RADIUS_FIELD"_s, 0.1 );
  pixelSizeParam->setHelp( QObject::tr( "The size of pixels in the output raster. This will determine the spatial resolution of the generated heatmap." ) );
  addParameter( pixelSizeParam.release() );

  auto weightFieldParam
    = std::make_unique<QgsProcessingParameterField>( u"WEIGHT_FIELD"_s, QObject::tr( "Weight from field" ), QVariant(), u"INPUT"_s, Qgis::ProcessingFieldParameterDataType::Numeric, false, true );
  weightFieldParam->setFlags( weightFieldParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  weightFieldParam->setHelp(
    QObject::tr( "An optional numeric field from the input layer to use for weighting the points. If specified, the density will be calculated using this weight instead of simply counting the points." )
  );
  addParameter( weightFieldParam.release() );

  const QStringList kernels = { QObject::tr( "Quartic" ), QObject::tr( "Triangular" ), QObject::tr( "Uniform" ), QObject::tr( "Triweight" ), QObject::tr( "Epanechnikov" ) };
  auto kernelShapeParam = std::make_unique<QgsProcessingParameterEnum>( u"KERNEL"_s, QObject::tr( "Kernel shape" ), kernels, false, 0 );
  kernelShapeParam->setFlags( kernelShapeParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  kernelShapeParam->setHelp( QObject::tr( "The shape of the kernel to use for density estimation. Different kernel shapes affect the influence of points at varying distances from the center." ) );
  addParameter( kernelShapeParam.release() );

  auto decayRatio
    = std::make_unique<QgsProcessingParameterNumber>( u"DECAY"_s, QObject::tr( "Decay ratio (Triangular kernels only)" ), Qgis::ProcessingNumberParameterType::Double, 0.0, true, -100.0, 100.0 );
  decayRatio->setFlags( decayRatio->flags() | Qgis::ProcessingParameterFlag::Advanced );
  decayRatio->setHelp( QObject::tr( "The decay ratio for triangular kernels. This controls how the weight of a point decreases as the distance from the point increases." ) );
  addParameter( decayRatio.release() );

  const QStringList outputScalings = { QObject::tr( "Raw" ), QObject::tr( "Scaled" ) };
  auto outputScaling = std::make_unique<QgsProcessingParameterEnum>( u"OUTPUT_VALUE"_s, QObject::tr( "Output value scaling" ), outputScalings, false, 0 );
  outputScaling->setFlags( outputScaling->flags() | Qgis::ProcessingParameterFlag::Advanced );
  outputScaling->setHelp( QObject::tr( "The scaling to apply to output values. 'Raw' will output the raw density values, while 'Scaled' will scale the values to represent an overall density." ) );
  addParameter( outputScaling.release() );

  auto outputParam = std::make_unique<QgsProcessingParameterRasterDestination>( u"OUTPUT"_s, QObject::tr( "Heatmap" ) );
  outputParam->setHelp( QObject::tr( "The output heatmap raster layer." ) );
  addParameter( outputParam.release() );
}

QString QgsHeatmapAlgorithm::shortHelpString() const
{
  return QObject::tr(
    "This algorithm creates a density (heatmap) raster of an input point vector layer using kernel density estimation. "
    "Heatmaps allow easy identification of hotspots and clustering of points.\n"
    "The density is calculated based on the number of points in a location, "
    "with larger numbers of clustered points resulting in larger values."
  );
}

QString QgsHeatmapAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates a heatmap from points using kernel density estimation." );
}

QgsHeatmapAlgorithm *QgsHeatmapAlgorithm::createInstance() const
{
  return new QgsHeatmapAlgorithm();
}

QVariantMap QgsHeatmapAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  const double radius = parameterAsDouble( parameters, u"RADIUS"_s, context );
  const int kernelShape = parameterAsEnum( parameters, u"KERNEL"_s, context );
  const double pixelSize = parameterAsDouble( parameters, u"PIXEL_SIZE"_s, context );
  const double decay = parameterAsDouble( parameters, u"DECAY"_s, context );
  const int outputValues = parameterAsEnum( parameters, u"OUTPUT_VALUE"_s, context );
  const QString outputFile = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );
  const QString weightField = parameterAsString( parameters, u"WEIGHT_FIELD"_s, context );
  const QString radiusField = parameterAsString( parameters, u"RADIUS_FIELD"_s, context );

  const QString outputFormat = parameterAsOutputRasterFormat( parameters, u"OUTPUT"_s, context );

  QList<int> attrs;
  QgsKernelDensityEstimation::Parameters kdeParams;
  kdeParams.source = source.get();
  kdeParams.radius = radius;
  kdeParams.pixelSize = pixelSize;

  if ( !radiusField.isEmpty() )
  {
    const int fieldIndex = source->fields().lookupField( radiusField );
    if ( fieldIndex < 0 )
    {
      throw QgsProcessingException( QObject::tr( "Invalid radius field: “%1” does not exist" ).arg( radiusField ) );
    }
    kdeParams.radiusField = radiusField;
    attrs.append( fieldIndex );
  }

  if ( !weightField.isEmpty() )
  {
    const int fieldIndex = source->fields().lookupField( weightField );
    if ( fieldIndex < 0 )
    {
      throw QgsProcessingException( QObject::tr( "Invalid weight field: “%1” does not exist" ).arg( weightField ) );
    }

    kdeParams.weightField = weightField;
    attrs.append( fieldIndex );
  }

  kdeParams.shape = static_cast<QgsKernelDensityEstimation::KernelShape>( kernelShape );
  kdeParams.decayRatio = decay;
  kdeParams.outputValues = static_cast<QgsKernelDensityEstimation::OutputValues>( outputValues );

  QgsKernelDensityEstimation kde( kdeParams, outputFile, outputFormat );
  if ( kde.prepare() != QgsKernelDensityEstimation::Result::Success )
    throw QgsProcessingException( QObject::tr( "Could not create destination layer" ) );

  QgsFeatureRequest request;
  request.setSubsetOfAttributes( attrs );
  QgsFeatureIterator features = source->getFeatures( request );
  QgsFeature f;
  const double total = source->featureCount() > 0 ? 100.0 / source->featureCount() : 0;
  int current = 0;
  while ( features.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
      break;

    if ( kde.addFeature( f ) != QgsKernelDensityEstimation::Result::Success )
      feedback->reportError( QObject::tr( "Error adding feature with ID %1 to heatmap" ).arg( f.id() ) );

    feedback->setProgress( current * total );
    current++;
  }

  if ( kde.finalise() != QgsKernelDensityEstimation::Result::Success )
    throw QgsProcessingException( QObject::tr( "Could not save destination layer" ) );

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, outputFile );
  return outputs;
}

///@endcond
