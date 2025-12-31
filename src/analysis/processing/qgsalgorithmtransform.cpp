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
  addParameter( new QgsProcessingParameterCrs( u"TARGET_CRS"_s, QObject::tr( "Target CRS" ), u"EPSG:4326"_s ) );

  // Convert curves to straight segments
  auto convertCurvesParam = std::make_unique<QgsProcessingParameterBoolean>( u"CONVERT_CURVED_GEOMETRIES"_s, QObject::tr( "Convert curved geometries to straight segments" ), false );
  convertCurvesParam->setHelp( QObject::tr( "If checked, curved geometries will be converted to straight segments. Otherwise, they will be kept as curves. This can fix distortion issues." ) );
  addParameter( convertCurvesParam.release() );

  // Transform Z
  auto transformZParam = std::make_unique<QgsProcessingParameterBoolean>( u"TRANSFORM_Z"_s, QObject::tr( "Also transform Z coordinates" ), false, true );
  transformZParam->setHelp( QObject::tr( "If checked, the z coordinates will also be transformed. This requires that the z coordinates in the geometries represent height relative to the vertical datum of the source CRS (generally ellipsoidal heights) and are expressed in its vertical units (generally meters). If unchecked, then z coordinates will not be changed by the transform." ) );
  addParameter( transformZParam.release() );

  // Optional coordinate operation
  auto crsOpParam = std::make_unique<QgsProcessingParameterCoordinateOperation>( u"OPERATION"_s, QObject::tr( "Coordinate operation" ), QVariant(), u"INPUT"_s, u"TARGET_CRS"_s, QVariant(), QVariant(), true );
  crsOpParam->setFlags( crsOpParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( crsOpParam.release() );
}

QgsCoordinateReferenceSystem QgsTransformAlgorithm::outputCrs( const QgsCoordinateReferenceSystem & ) const
{
  return mDestCrs;
}

QString QgsTransformAlgorithm::outputName() const
{
  return QObject::tr( "Reprojected" );
}

Qgis::ProcessingFeatureSourceFlags QgsTransformAlgorithm::sourceFlags() const
{
  return Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks;
}

QString QgsTransformAlgorithm::name() const
{
  return u"reprojectlayer"_s;
}

QString QgsTransformAlgorithm::displayName() const
{
  return QObject::tr( "Reproject layer" );
}

QStringList QgsTransformAlgorithm::tags() const
{
  return QObject::tr( "transform,reprojection,crs,srs,warp" ).split( ',' );
}

QString QgsTransformAlgorithm::group() const
{
  return QObject::tr( "Vector general" );
}

QString QgsTransformAlgorithm::groupId() const
{
  return u"vectorgeneral"_s;
}

QString QgsTransformAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm reprojects a vector layer. It creates a new layer with the same features "
                      "as the input one, but with geometries reprojected to a new CRS.\n\n"
                      "Attributes are not modified by this algorithm." );
}

QString QgsTransformAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates a vector layer with geometries transformed to a new CRS." );
}

QgsTransformAlgorithm *QgsTransformAlgorithm::createInstance() const
{
  return new QgsTransformAlgorithm();
}

bool QgsTransformAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  prepareSource( parameters, context );
  mDestCrs = parameterAsCrs( parameters, u"TARGET_CRS"_s, context );
  mTransformContext = context.transformContext();
  mConvertCurveToSegments = parameterAsBoolean( parameters, u"CONVERT_CURVED_GEOMETRIES"_s, context );
  mTransformZ = parameterAsBoolean( parameters, u"TRANSFORM_Z"_s, context );
  mCoordOp = parameterAsString( parameters, u"OPERATION"_s, context );
  return true;
}

QgsFeatureList QgsTransformAlgorithm::processFeature( const QgsFeature &f, QgsProcessingContext &, QgsProcessingFeedback *feedback )
{
  QgsFeature feature = f;
  if ( !mCreatedTransform )
  {
    mCreatedTransform = true;
    if ( !mCoordOp.isEmpty() )
      mTransformContext.addCoordinateOperation( sourceCrs(), mDestCrs, mCoordOp, false );
    mTransform = QgsCoordinateTransform( sourceCrs(), mDestCrs, mTransformContext );

    mTransform.disableFallbackOperationHandler( true );
  }

  if ( feature.hasGeometry() )
  {
    QgsGeometry g = feature.geometry();

    if ( !mTransform.isShortCircuited() && mConvertCurveToSegments )
    {
      // convert to straight segments to avoid issues with distorted curves
      g.convertToStraightSegment();
    }
    try
    {
      if ( g.transform( mTransform, Qgis::TransformDirection::Forward, mTransformZ ) == Qgis::GeometryOperationResult::Success )
      {
        feature.setGeometry( g );
      }
      else
      {
        feature.clearGeometry();
      }

      if ( !mWarnedAboutFallbackTransform && mTransform.fallbackOperationOccurred() )
      {
        feedback->reportError( QObject::tr( "An alternative, ballpark-only transform was used when transforming coordinates for one or more features. "
                                            "(Possibly an incorrect choice of operation was made for transformations between these reference systems - check "
                                            "that the selected operation is valid for the full extent of the input layer.)" ) );
        mWarnedAboutFallbackTransform = true; // only warn once to avoid flooding the log
      }
    }
    catch ( QgsCsException & )
    {
      if ( feedback )
        feedback->reportError( QObject::tr( "Encountered a transform error when reprojecting feature with id %1." ).arg( f.id() ) );
      feature.clearGeometry();
    }
  }
  return QgsFeatureList() << feature;
}

///@endcond
