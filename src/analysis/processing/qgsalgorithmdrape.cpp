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
  return QStringLiteral( "vectorgeometry" );
}

QString QgsDrapeAlgorithmBase::outputName() const
{
  return QObject::tr( "Draped" );
}

void QgsDrapeAlgorithmBase::initParameters( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( QStringLiteral( "RASTER" ),
                QObject::tr( "Raster layer" ) ) );
  addParameter( new QgsProcessingParameterBand( QStringLiteral( "BAND" ),
                QObject::tr( "Band number" ), 1, QStringLiteral( "RASTER" ) ) );

  // nodata value
  std::unique_ptr< QgsProcessingParameterNumber > nodata = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "NODATA" ),
      QObject::tr( "Value for nodata or non-intersecting vertices" ), QgsProcessingParameterNumber::Double,
      0.0 );
  nodata->setIsDynamic( true );
  nodata->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "NODATA" ), QObject::tr( "Value for nodata or non-intersecting vertices" ), QgsPropertyDefinition::Double ) );
  nodata->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( nodata.release() );

  auto scaleParam = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "SCALE" ), QObject::tr( "Scale factor" ), QgsProcessingParameterNumber::Double, 1.0, false, 0.0 );
  scaleParam->setIsDynamic( true );
  scaleParam->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "SCALE" ), QObject::tr( "Scale factor" ), QgsPropertyDefinition::Double ) );
  scaleParam->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( scaleParam.release() );
}

bool QgsDrapeAlgorithmBase::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mNoData = parameterAsDouble( parameters, QStringLiteral( "NODATA" ), context );
  mDynamicNoData = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "NODATA" ) );
  if ( mDynamicNoData )
    mNoDataProperty = parameters.value( QStringLiteral( "NODATA" ) ).value< QgsProperty >();

  mScale = parameterAsDouble( parameters, QStringLiteral( "SCALE" ), context );
  mDynamicScale = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "SCALE" ) );
  if ( mDynamicScale )
    mScaleProperty = parameters.value( QStringLiteral( "SCALE" ) ).value< QgsProperty >();

  QgsRasterLayer *layer = parameterAsRasterLayer( parameters, QStringLiteral( "RASTER" ), context );

  if ( !layer )
    throw QgsProcessingException( invalidRasterError( parameters, QStringLiteral( "RASTER" ) ) );

  mBand = parameterAsInt( parameters, QStringLiteral( "BAND" ), context );
  if ( mBand < 1 || mBand > layer->bandCount() )
    throw QgsProcessingException( QObject::tr( "Invalid band number for BAND (%1): Valid values for input raster are 1 to %2" ).arg( mBand )
                                  .arg( layer->bandCount() ) );
  mRasterExtent = layer->extent();

  std::unique_ptr< QgsRasterInterface > provider( layer->dataProvider()->clone() );
  QgsRasterDataProvider *dp = dynamic_cast< QgsRasterDataProvider * >( provider.get() );
  if ( !dp )
    throw QgsProcessingException( invalidRasterError( parameters, QStringLiteral( "RASTER" ) ) );

  mRasterProvider.reset( dp );
  provider.release();

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

    prepareGeometry( geometry, nodata );

    // only do the "draping" if the geometry intersects the raster - otherwise skip
    // a pointless iteration over all vertices
    if ( !mRasterExtent.isNull() && geometry.boundingBoxIntersects( mRasterExtent ) )
    {
      geometry.transformVertices( [ = ]( const QgsPoint & p )->QgsPoint
      {
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
            val *= scale;
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
  return QStringLiteral( "setzfromraster" );
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
         + QStringLiteral( "\n\n" )
         + QObject::tr( "The raster values can optionally be scaled by a preset amount." );
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
  const QgsVectorLayer *layer = qobject_cast< const QgsVectorLayer * >( l );
  if ( !layer )
    return false;

  if ( ! QgsDrapeAlgorithmBase::supportInPlaceEdit( layer ) )
    return false;
  return QgsWkbTypes::hasZ( layer->wkbType() );
}

QgsWkbTypes::Type QgsDrapeToZAlgorithm::outputWkbType( QgsWkbTypes::Type inputWkbType ) const
{
  const QgsWkbTypes::Type wkb = inputWkbType;
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
  return QStringLiteral( "setmfromraster" );
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
         + QStringLiteral( "\n\n" )
         + QObject::tr( "The raster values can optionally be scaled by a preset amount." );
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
  const QgsVectorLayer *layer = qobject_cast< const QgsVectorLayer * >( l );
  if ( !layer )
    return false;

  if ( ! QgsDrapeAlgorithmBase::supportInPlaceEdit( layer ) )
    return false;
  return QgsWkbTypes::hasM( layer->wkbType() );
}

QgsWkbTypes::Type QgsDrapeToMAlgorithm::outputWkbType( QgsWkbTypes::Type inputWkbType ) const
{
  const QgsWkbTypes::Type wkb = inputWkbType;
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


