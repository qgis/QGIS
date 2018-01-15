/***************************************************************************
                         qgsalgorithmtransform.cpp
                         ---------------------
    begin                : April 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgsalgorithmtransform.h"

///@cond PRIVATE


void QgsTransformAlgorithm::initParameters( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterCrs( QStringLiteral( "TARGET_CRS" ), QObject::tr( "Target CRS" ), QStringLiteral( "EPSG:4326" ) ) );
}

QgsCoordinateReferenceSystem QgsTransformAlgorithm::outputCrs( const QgsCoordinateReferenceSystem & ) const
{
  return mDestCrs;
}

QString QgsTransformAlgorithm::outputName() const
{
  return QObject::tr( "Reprojected" );
}

QgsProcessingAlgorithm::Flags QgsTransformAlgorithm::flags() const
{
  return QgsProcessingFeatureBasedAlgorithm::flags() | QgsProcessingAlgorithm::FlagCanRunInBackground;
}

QString QgsTransformAlgorithm::name() const
{
  return QStringLiteral( "reprojectlayer" );
}

QString QgsTransformAlgorithm::displayName() const
{
  return QObject::tr( "Reproject layer" );
}

QStringList QgsTransformAlgorithm::tags() const
{
  return QObject::tr( "transform,reproject,crs,srs,warp" ).split( ',' );
}

QString QgsTransformAlgorithm::group() const
{
  return QObject::tr( "Vector general" );
}

QString QgsTransformAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeneral" );
}

QString QgsTransformAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm reprojects a vector layer. It creates a new layer with the same features "
                      "as the input one, but with geometries reprojected to a new CRS.\n\n"
                      "Attributes are not modified by this algorithm." );
}

QgsTransformAlgorithm *QgsTransformAlgorithm::createInstance() const
{
  return new QgsTransformAlgorithm();
}

bool QgsTransformAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mDestCrs = parameterAsCrs( parameters, QStringLiteral( "TARGET_CRS" ), context );
  mTransformContext = context.project() ? context.project()->transformContext() : QgsCoordinateTransformContext();
  return true;
}

QgsFeature QgsTransformAlgorithm::processFeature( const QgsFeature &f, QgsProcessingContext &, QgsProcessingFeedback * )
{
  QgsFeature feature = f;
  if ( !mCreatedTransform )
  {
    mCreatedTransform = true;
    mTransform = QgsCoordinateTransform( sourceCrs(), mDestCrs, mTransformContext );
  }

  if ( feature.hasGeometry() )
  {
    QgsGeometry g = feature.geometry();
    if ( g.transform( mTransform ) == 0 )
    {
      feature.setGeometry( g );
    }
    else
    {
      feature.clearGeometry();
    }
  }
  return feature;
}

///@endcond



