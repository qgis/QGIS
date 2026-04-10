/***************************************************************************
                         qgsalgorithmextrude.cpp
                         ---------------------
    begin                : April 2026
    copyright            : (C) 2026 by Jean Felder
    email                : jean dot felder at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsalgorithmextrude.h"

#include "qgsexception.h"
#include "qgsgeometry.h"

#include <QString>

using namespace Qt::StringLiterals;

#ifdef WITH_SFCGAL
#include "qgssfcgalgeometry.h"
#endif

///@cond PRIVATE

QString QgsExtrudeAlgorithm::name() const
{
  return u"extrude"_s;
}

QString QgsExtrudeAlgorithm::displayName() const
{
  return QObject::tr( "Extrude" );
}

QStringList QgsExtrudeAlgorithm::tags() const
{
  return QObject::tr( "extrude,extrusion,3d,volume" ).split( ',' );
}

QString QgsExtrudeAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsExtrudeAlgorithm::groupId() const
{
  return u"vectorgeometry"_s;
}

QString QgsExtrudeAlgorithm::shortHelpString() const
{
  return QObject::tr(
    "This algorithm generates 3D geometries by extruding 2D polygon features along a specified direction.\n\n"
    "Each feature is displaced according to the X, Y, and Z extrusion parameters: "
    "X and Y control the horizontal displacement, while Z controls the vertical elevation. "
    "Setting only the Z parameter produces vertical extrusions, whereas combining X, Y, and Z "
    "allows the creation of non-vertical extrusions. Negative values are supported, enabling extrusions "
    "in the opposite direction.\n\n"
    "If the input features already carry Z values, those values are preserved and used as the base "
    "elevation of the extruded geometry.\n\n"
    "For MultiPolygon geometries, each part is extruded separately, producing one output feature per part.\n\n"
    "Output geometries are of type PolyhedralSurfaceZ, representing the extruded surface of each input feature."
  );
}

QString QgsExtrudeAlgorithm::shortDescription() const
{
  return QObject::tr( "Generates a 3D extrusion from 2D polygon features." );
}

QgsExtrudeAlgorithm *QgsExtrudeAlgorithm::createInstance() const
{
  return new QgsExtrudeAlgorithm();
}

QList<int> QgsExtrudeAlgorithm::inputLayerTypes() const
{
  return QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon );
}

QString QgsExtrudeAlgorithm::outputName() const
{
  return QObject::tr( "Extrusion" );
}

Qgis::ProcessingSourceType QgsExtrudeAlgorithm::outputLayerType() const
{
  return Qgis::ProcessingSourceType::VectorPolygon;
}

Qgis::WkbType QgsExtrudeAlgorithm::outputWkbType( Qgis::WkbType inputWkbType ) const
{
  Q_UNUSED( inputWkbType )
  return Qgis::WkbType::PolyhedralSurfaceZ;
}

void QgsExtrudeAlgorithm::initParameters( const QVariantMap & )
{
  auto xExtrude = std::make_unique<QgsProcessingParameterNumber>( u"EXTRUDE_X"_s, QObject::tr( "Extrusion (x-axis)" ), Qgis::ProcessingNumberParameterType::Double, 0.0 );
  xExtrude->setIsDynamic( true );
  xExtrude->setDynamicPropertyDefinition( QgsPropertyDefinition( u"EXTRUDE_X"_s, QObject::tr( "Extrusion (x-axis)" ), QgsPropertyDefinition::Double ) );
  xExtrude->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( xExtrude.release() );

  auto yExtrude = std::make_unique<QgsProcessingParameterNumber>( u"EXTRUDE_Y"_s, QObject::tr( "Extrusion (y-axis)" ), Qgis::ProcessingNumberParameterType::Double, 0.0 );
  yExtrude->setIsDynamic( true );
  yExtrude->setDynamicPropertyDefinition( QgsPropertyDefinition( u"EXTRUDE_Y"_s, QObject::tr( "Extrusion (y-axis)" ), QgsPropertyDefinition::Double ) );
  yExtrude->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( yExtrude.release() );

  auto zExtrude = std::make_unique<QgsProcessingParameterNumber>( u"EXTRUDE_Z"_s, QObject::tr( "Extrusion (z-axis)" ), Qgis::ProcessingNumberParameterType::Double, 0.0 );
  zExtrude->setIsDynamic( true );
  zExtrude->setDynamicPropertyDefinition( QgsPropertyDefinition( u"EXTRUDE_Z"_s, QObject::tr( "Extrusion (z-axis)" ), QgsPropertyDefinition::Double ) );
  zExtrude->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( zExtrude.release() );
}

bool QgsExtrudeAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback )

#ifdef WITH_SFCGAL
  mExtrudeX = parameterAsDouble( parameters, u"EXTRUDE_X"_s, context );
  mDynamicExtrudeX = QgsProcessingParameters::isDynamic( parameters, u"EXTRUDE_X"_s );
  if ( mDynamicExtrudeX )
    mExtrudeXProperty = parameters.value( u"EXTRUDE_X"_s ).value<QgsProperty>();

  mExtrudeY = parameterAsDouble( parameters, u"EXTRUDE_Y"_s, context );
  mDynamicExtrudeY = QgsProcessingParameters::isDynamic( parameters, u"EXTRUDE_Y"_s );
  if ( mDynamicExtrudeY )
    mExtrudeYProperty = parameters.value( u"EXTRUDE_Y"_s ).value<QgsProperty>();

  mExtrudeZ = parameterAsDouble( parameters, u"EXTRUDE_Z"_s, context );
  mDynamicExtrudeZ = QgsProcessingParameters::isDynamic( parameters, u"EXTRUDE_Z"_s );
  if ( mDynamicExtrudeZ )
    mExtrudeZProperty = parameters.value( u"EXTRUDE_Z"_s ).value<QgsProperty>();

  return true;
#else
  Q_UNUSED( parameters )
  Q_UNUSED( context )
  throw QgsProcessingException( QObject::tr( "This processing algorithm requires a QGIS installation with SFCGAL support enabled. Please use a version of QGIS that includes SFCGAL." ) );
#endif
}

std::optional<QgsGeometry> QgsExtrudeAlgorithm::extrudePolygon( const QgsAbstractGeometry *polygon, const QgsVector3D &extrusion, const QgsFeatureId &featureId, QgsProcessingFeedback *feedback )
{
  try
  {
    QgsSfcgalGeometry inputSfcgalGeometry( polygon );
    if ( inputSfcgalGeometry.isEmpty() )
    {
      return std::nullopt;
    }

    std::unique_ptr<QgsSfcgalGeometry> outputSfcgalGeometry = inputSfcgalGeometry.extrude( extrusion );
    return QgsGeometry( outputSfcgalGeometry->asQgisGeometry() );
  }
  catch ( const QgsSfcgalException &exception )
  {
    feedback->reportError( QObject::tr( "Cannot calculate extrusion for feature %1: %2" ).arg( featureId ).arg( exception.what() ) );
    return std::nullopt;
  }
}

QgsFeatureList QgsExtrudeAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
#ifdef WITH_SFCGAL
  if ( !feature.hasGeometry() )
  {
    return QgsFeatureList() << feature;
  }

  double extrudeX = mExtrudeX;
  if ( mDynamicExtrudeX )
  {
    extrudeX = mExtrudeXProperty.valueAsDouble( context.expressionContext(), extrudeX );
  }

  double extrudeY = mExtrudeY;
  if ( mDynamicExtrudeY )
  {
    extrudeY = mExtrudeYProperty.valueAsDouble( context.expressionContext(), extrudeY );
  }

  double extrudeZ = mExtrudeZ;
  if ( mDynamicExtrudeZ )
  {
    extrudeZ = mExtrudeZProperty.valueAsDouble( context.expressionContext(), extrudeZ );
  }

  const QgsVector3D extrusion( extrudeX, extrudeY, extrudeZ );

  QgsFeatureList outputFeatures;
  const QgsGeometry inputGeometry = feature.geometry();

  // For MultiPolygons, each part is extruded separately
  if ( inputGeometry.isMultipart() )
  {
    for ( auto partIt = inputGeometry.const_parts_begin(); partIt != inputGeometry.const_parts_end(); ++partIt )
    {
      std::optional<QgsGeometry> extruded = extrudePolygon( *partIt, extrusion, feature.id(), feedback );
      if ( extruded.has_value() )
      {
        QgsFeature modifiedFeature;
        modifiedFeature.setGeometry( extruded.value() );
        modifiedFeature.setAttributes( feature.attributes() );
        outputFeatures << modifiedFeature;
      }
    }
  }
  else
  {
    std::optional<QgsGeometry> extruded = extrudePolygon( inputGeometry.constGet(), extrusion, feature.id(), feedback );
    if ( extruded.has_value() )
    {
      QgsFeature modifiedFeature = feature;
      modifiedFeature.setGeometry( extruded.value() );
      outputFeatures << modifiedFeature;
    }
  }

  return outputFeatures;
#else
  Q_UNUSED( feature )
  Q_UNUSED( context )
  Q_UNUSED( feedback )
  throw QgsProcessingException( QObject::tr( "This processing algorithm requires a QGIS installation with SFCGAL support enabled. Please use a version of QGIS that includes SFCGAL." ) );
#endif
}

///@endcond PRIVATE
