/***************************************************************************
                         qgsalgorithmrotate.cpp
                         ---------------------
    begin                : March 2018
    copyright            : (C) 2018 by Nyall Dawson
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

#include "qgsalgorithmrotate.h"

///@cond PRIVATE

QString QgsRotateFeaturesAlgorithm::name() const
{
  return u"rotatefeatures"_s;
}

QString QgsRotateFeaturesAlgorithm::displayName() const
{
  return QObject::tr( "Rotate" );
}

QStringList QgsRotateFeaturesAlgorithm::tags() const
{
  return QObject::tr( "rotate,around,center,point" ).split( ',' );
}

QString QgsRotateFeaturesAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsRotateFeaturesAlgorithm::groupId() const
{
  return u"vectorgeometry"_s;
}

QString QgsRotateFeaturesAlgorithm::outputName() const
{
  return QObject::tr( "Rotated" );
}

QString QgsRotateFeaturesAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm rotates feature geometries by the specified angle clockwise." )
         + u"\n\n"_s
         + QObject::tr( "Optionally, the rotation can occur around a preset point. If not set the rotation occurs around each feature's centroid." );
}

QString QgsRotateFeaturesAlgorithm::shortDescription() const
{
  return QObject::tr( "Rotates feature geometries by a specified angle clockwise." );
}

QgsRotateFeaturesAlgorithm *QgsRotateFeaturesAlgorithm::createInstance() const
{
  return new QgsRotateFeaturesAlgorithm();
}

void QgsRotateFeaturesAlgorithm::initParameters( const QVariantMap & )
{
  auto rotation = std::make_unique<QgsProcessingParameterNumber>( u"ANGLE"_s, QObject::tr( "Rotation (degrees clockwise)" ), Qgis::ProcessingNumberParameterType::Double, 0.0 );
  rotation->setIsDynamic( true );
  rotation->setDynamicPropertyDefinition( QgsPropertyDefinition( u"ANGLE"_s, QObject::tr( "Rotation (degrees clockwise)" ), QgsPropertyDefinition::Rotation ) );
  rotation->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( rotation.release() );

  auto anchor = std::make_unique<QgsProcessingParameterPoint>( u"ANCHOR"_s, QObject::tr( "Rotation anchor point" ), QVariant(), true );
  addParameter( anchor.release() );
}

bool QgsRotateFeaturesAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mAngle = parameterAsDouble( parameters, u"ANGLE"_s, context );
  mDynamicAngle = QgsProcessingParameters::isDynamic( parameters, u"ANGLE"_s );
  if ( mDynamicAngle )
    mAngleProperty = parameters.value( u"ANGLE"_s ).value<QgsProperty>();

  mUseAnchor = parameters.value( u"ANCHOR"_s ).isValid();
  if ( mUseAnchor )
  {
    mAnchor = parameterAsPoint( parameters, u"ANCHOR"_s, context );
    mAnchorCrs = parameterAsPointCrs( parameters, u"ANCHOR"_s, context );
  }

  return true;
}

QgsFeatureList QgsRotateFeaturesAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  if ( mUseAnchor && !mTransformedAnchor )
  {
    mTransformedAnchor = true;
    if ( mAnchorCrs != sourceCrs() )
    {
      const QgsCoordinateTransform ct( mAnchorCrs, sourceCrs(), context.transformContext() );
      try
      {
        mAnchor = ct.transform( mAnchor );
      }
      catch ( QgsCsException & )
      {
        throw QgsProcessingException( QObject::tr( "Could not transform anchor point to destination CRS" ) );
      }
    }
  }

  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    QgsGeometry geometry = f.geometry();

    double angle = mAngle;
    if ( mDynamicAngle )
      angle = mAngleProperty.valueAsDouble( context.expressionContext(), angle );

    if ( mUseAnchor )
    {
      geometry.rotate( angle, mAnchor );
      f.setGeometry( geometry );
    }
    else
    {
      const QgsGeometry centroid = geometry.centroid();
      if ( !centroid.isNull() )
      {
        geometry.rotate( angle, centroid.asPoint() );
        f.setGeometry( geometry );
      }
      else
      {
        feedback->reportError( QObject::tr( "Could not calculate centroid for feature %1: %2" ).arg( feature.id() ).arg( centroid.lastError() ) );
      }
    }
  }
  return QgsFeatureList() << f;
}


///@endcond
