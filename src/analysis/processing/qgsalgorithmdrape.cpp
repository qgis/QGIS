/***************************************************************************
                         qgsalgorithmdrape.cpp
                         ---------------------
    begin                : November 2017
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

#include "qgsalgorithmdrape.h"

#include "qgsvectorlayer.h"

///@cond PRIVATE


QString QgsDrapeAlgorithmBase::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsDrapeAlgorithmBase::groupId() const
{
  return u"vectorgeometry"_s;
}

QString QgsDrapeAlgorithmBase::outputName() const
{
  return QObject::tr( "Draped" );
}

void QgsDrapeAlgorithmBase::initParameters( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( u"RASTER"_s, QObject::tr( "Raster layer" ) ) );
  addParameter( new QgsProcessingParameterBand( u"BAND"_s, QObject::tr( "Band number" ), 1, u"RASTER"_s ) );

  // nodata value
  auto nodata = std::make_unique<QgsProcessingParameterNumber>( u"NODATA"_s, QObject::tr( "Value for NoData or non-intersecting vertices" ), Qgis::ProcessingNumberParameterType::Double, 0.0 );
  nodata->setIsDynamic( true );
  nodata->setDynamicPropertyDefinition( QgsPropertyDefinition( u"NODATA"_s, QObject::tr( "Value for NoData or non-intersecting vertices" ), QgsPropertyDefinition::Double ) );
  nodata->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( nodata.release() );

  auto scaleParam = std::make_unique<QgsProcessingParameterNumber>( u"SCALE"_s, QObject::tr( "Scale factor" ), Qgis::ProcessingNumberParameterType::Double, 1.0, false, 0.0 );
  scaleParam->setIsDynamic( true );
  scaleParam->setDynamicPropertyDefinition( QgsPropertyDefinition( u"SCALE"_s, QObject::tr( "Scale factor" ), QgsPropertyDefinition::Double ) );
  scaleParam->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( scaleParam.release() );

  auto offsetParam = std::make_unique<QgsProcessingParameterNumber>( u"OFFSET"_s, QObject::tr( "Offset" ), Qgis::ProcessingNumberParameterType::Double, 0.0 );
  offsetParam->setIsDynamic( true );
  offsetParam->setDynamicPropertyDefinition( QgsPropertyDefinition( u"OFFSET"_s, QObject::tr( "Offset" ), QgsPropertyDefinition::Double ) );
  offsetParam->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( offsetParam.release() );
}

bool QgsDrapeAlgorithmBase::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mNoData = parameterAsDouble( parameters, u"NODATA"_s, context );
  mDynamicNoData = QgsProcessingParameters::isDynamic( parameters, u"NODATA"_s );
  if ( mDynamicNoData )
    mNoDataProperty = parameters.value( u"NODATA"_s ).value<QgsProperty>();

  mScale = parameterAsDouble( parameters, u"SCALE"_s, context );
  mDynamicScale = QgsProcessingParameters::isDynamic( parameters, u"SCALE"_s );
  if ( mDynamicScale )
    mScaleProperty = parameters.value( u"SCALE"_s ).value<QgsProperty>();

  mOffset = parameterAsDouble( parameters, u"OFFSET"_s, context );
  mDynamicOffset = QgsProcessingParameters::isDynamic( parameters, u"OFFSET"_s );
  if ( mDynamicOffset )
    mOffsetProperty = parameters.value( u"OFFSET"_s ).value<QgsProperty>();

  QgsRasterLayer *layer = parameterAsRasterLayer( parameters, u"RASTER"_s, context );

  if ( !layer )
    throw QgsProcessingException( invalidRasterError( parameters, u"RASTER"_s ) );

  mBand = parameterAsInt( parameters, u"BAND"_s, context );
  if ( mBand < 1 || mBand > layer->bandCount() )
    throw QgsProcessingException( QObject::tr( "Invalid band number for BAND (%1): Valid values for input raster are 1 to %2" ).arg( mBand ).arg( layer->bandCount() ) );
  mRasterExtent = layer->extent();

  mRasterProvider.reset( layer->dataProvider()->clone() );
  if ( !mRasterProvider )
    throw QgsProcessingException( invalidRasterError( parameters, u"RASTER"_s ) );

  return true;
}

QgsFeatureList QgsDrapeAlgorithmBase::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  if ( !mCreatedTransform )
  {
    mCreatedTransform = true;
    mTransform = QgsCoordinateTransform( sourceCrs(), mRasterProvider->crs(), context.transformContext() );

    // transform the raster extent back to the vector's crs, so that we can test
    // whether individual vector geometries are actually covered by the raster
    try
    {
      mRasterExtent = mTransform.transform( mRasterExtent, Qgis::TransformDirection::Reverse );
    }
    catch ( QgsCsException & )
    {
      mRasterExtent = QgsRectangle();
    }
  }

  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    QgsGeometry geometry = f.geometry();

    double nodata = mNoData;
    if ( mDynamicNoData )
      nodata = mNoDataProperty.valueAsDouble( context.expressionContext(), nodata );

    double scale = mScale;
    if ( mDynamicScale )
      scale = mScaleProperty.valueAsDouble( context.expressionContext(), scale );

    double offset = mOffset;
    if ( mDynamicOffset )
      offset = mOffsetProperty.valueAsDouble( context.expressionContext(), offset );

    prepareGeometry( geometry, nodata );

    // only do the "draping" if the geometry intersects the raster - otherwise skip
    // a pointless iteration over all vertices
    if ( !mRasterExtent.isNull() && geometry.boundingBoxIntersects( mRasterExtent ) )
    {
      geometry.transformVertices( [this, nodata, scale, offset, feedback, &f]( const QgsPoint &p ) -> QgsPoint {
        QgsPointXY t;
        double val = nodata;
        try
        {
          t = mTransform.transform( p );
          bool ok = false;
          val = mRasterProvider->sample( t, mBand, &ok );
          if ( !ok )
            val = nodata;
          else
          {
            val *= scale;
            val += offset;
          }
        }
        catch ( QgsCsException & )
        {
          feedback->reportError( QObject::tr( "Transform error while reprojecting feature {}" ).arg( f.id() ) );
        }

        return drapeVertex( p, val );
      } );
    }

    f.setGeometry( geometry );
  }
  return QgsFeatureList() << f;
}


//
// QgsDrapeToZAlgorithm
//

QString QgsDrapeToZAlgorithm::name() const
{
  return u"setzfromraster"_s;
}

QString QgsDrapeToZAlgorithm::displayName() const
{
  return QObject::tr( "Drape (set Z value from raster)" );
}

QStringList QgsDrapeToZAlgorithm::tags() const
{
  return QObject::tr( "3d,vertex,vertices,elevation,height,sample,dem,update,feature" ).split( ',' );
}

QString QgsDrapeToZAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm sets the z value of every vertex in the feature geometry to a value sampled from a band within a raster layer." )
         + u"\n\n"_s
         + QObject::tr( "The raster values can optionally be scaled by a preset amount and an offset can be algebraically added." );
}

QString QgsDrapeToZAlgorithm::shortDescription() const
{
  return QObject::tr( "Sets the z value for vertices to values sampled from a raster layer." );
}

QgsDrapeToZAlgorithm *QgsDrapeToZAlgorithm::createInstance() const
{
  return new QgsDrapeToZAlgorithm();
}

bool QgsDrapeToZAlgorithm::supportInPlaceEdit( const QgsMapLayer *l ) const
{
  const QgsVectorLayer *layer = qobject_cast<const QgsVectorLayer *>( l );
  if ( !layer )
    return false;

  if ( !QgsDrapeAlgorithmBase::supportInPlaceEdit( layer ) )
    return false;
  return QgsWkbTypes::hasZ( layer->wkbType() );
}

Qgis::WkbType QgsDrapeToZAlgorithm::outputWkbType( Qgis::WkbType inputWkbType ) const
{
  const Qgis::WkbType wkb = inputWkbType;
  return QgsWkbTypes::addZ( wkb );
}

void QgsDrapeToZAlgorithm::prepareGeometry( QgsGeometry &geometry, double defaultVal ) const
{
  geometry.get()->addZValue( defaultVal );
}

QgsPoint QgsDrapeToZAlgorithm::drapeVertex( const QgsPoint &p, double rasterVal ) const
{
  return QgsPoint( p.wkbType(), p.x(), p.y(), rasterVal, p.m() );
}

//
// QgsDrapeToMAlgorithm
//

QString QgsDrapeToMAlgorithm::name() const
{
  return u"setmfromraster"_s;
}

QString QgsDrapeToMAlgorithm::displayName() const
{
  return QObject::tr( "Set M value from raster" );
}

QStringList QgsDrapeToMAlgorithm::tags() const
{
  return QObject::tr( "drape,vertex,vertices,sample,dem,update,feature,measure" ).split( ',' );
}

QString QgsDrapeToMAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm sets the M value for every vertex in the feature geometry to a value sampled from a band within a raster layer." )
         + u"\n\n"_s
         + QObject::tr( "The raster values can optionally be scaled by a preset amount and an offset can be algebraically added." );
}

QString QgsDrapeToMAlgorithm::shortDescription() const
{
  return QObject::tr( "Sets the M value for vertices to values sampled from a raster layer." );
}

QgsDrapeToMAlgorithm *QgsDrapeToMAlgorithm::createInstance() const
{
  return new QgsDrapeToMAlgorithm();
}

bool QgsDrapeToMAlgorithm::supportInPlaceEdit( const QgsMapLayer *l ) const
{
  const QgsVectorLayer *layer = qobject_cast<const QgsVectorLayer *>( l );
  if ( !layer )
    return false;

  if ( !QgsDrapeAlgorithmBase::supportInPlaceEdit( layer ) )
    return false;
  return QgsWkbTypes::hasM( layer->wkbType() );
}

Qgis::WkbType QgsDrapeToMAlgorithm::outputWkbType( Qgis::WkbType inputWkbType ) const
{
  const Qgis::WkbType wkb = inputWkbType;
  return QgsWkbTypes::addM( wkb );
}

void QgsDrapeToMAlgorithm::prepareGeometry( QgsGeometry &geometry, double defaultVal ) const
{
  geometry.get()->addMValue( defaultVal );
}

QgsPoint QgsDrapeToMAlgorithm::drapeVertex( const QgsPoint &p, double rasterVal ) const
{
  return QgsPoint( p.wkbType(), p.x(), p.y(), p.z(), rasterVal );
}


///@endcond
