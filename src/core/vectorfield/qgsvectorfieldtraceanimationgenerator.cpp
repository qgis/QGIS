/***************************************************************************
    qgsvectorfieldtraceanimationgenerator.cpp
    -----------------------------------------
    begin                : November 2019
    copyright            : (C) 2019 by Vincent Cloarec
    email                : vcloarec at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorfieldtraceanimationgenerator.h"

#include "qgsmeshlayer.h"
#include "qgsmeshlayerrenderer.h"
#include "qgsmeshlayerutils.h"
#include "qgsmeshvectorfieldvaluesource.h"
#include "qgsvectorfieldsettings.h"
#include "qgsvectorfieldstreamfield.h"
#include "qgsvectorfieldvaluesource.h"

QgsVectorFieldTraceAnimationGenerator::QgsVectorFieldTraceAnimationGenerator(
  std::unique_ptr<QgsVectorFieldValueSource> source, const QgsRenderContext &rendererContext, const QgsVectorFieldSettings &vectorSettings, bool minimizeFieldSize
)
  : mParticleField( std::make_unique<QgsVectorFieldParticleTracesField>( std::move( source ), rendererContext, vectorSettings.vectorStrokeColoring() ) )
  , mRendererContext( rendererContext )
{
  mParticleField->setMinimizeFieldSize( minimizeFieldSize );
  mParticleField->updateSize( mRendererContext );
}

QgsVectorFieldTraceAnimationGenerator *QgsVectorFieldTraceAnimationGenerator::fromMeshLayer( QgsMeshLayer *layer, const QgsRenderContext &rendererContext )
{
  if ( !layer || !layer->dataProvider() )
    return nullptr;

  if ( !layer->triangularMesh() )
    layer->reload();

  if ( !layer->triangularMesh() || !layer->nativeMesh() )
    return nullptr;

  return new QgsVectorFieldTraceAnimationGenerator( layer, rendererContext );
}
QgsVectorFieldTraceAnimationGenerator::QgsVectorFieldTraceAnimationGenerator( QgsMeshLayer *layer, const QgsRenderContext &rendererContext )
  : mRendererContext( rendererContext )
{
  QgsMeshDataBlock vectorDatasetValues;
  QgsMeshDataBlock scalarActiveFaceFlagValues;
  QVector<double> magnitudeValues;
  bool vectorDataOnVertices;
  double magMax;

  const QgsMeshDatasetIndex datasetIndex = layer->activeVectorDatasetAtTime( rendererContext.temporalRange() );

  // Find out if we can use cache up to date. If yes, use it and return
  const int datasetGroupCount = layer->dataProvider()->datasetGroupCount();
  const QgsVectorFieldSettings vectorSettings = layer->rendererSettings().vectorSettings( datasetIndex.group() );
  const QgsMeshLayerRendererCache *cache = layer->rendererCache();

  if ( cache && ( cache->mDatasetGroupsCount == datasetGroupCount ) && ( cache->mActiveVectorDatasetIndex == datasetIndex ) )
  {
    vectorDatasetValues = cache->mVectorDatasetValues;
    scalarActiveFaceFlagValues = cache->mScalarActiveFaceFlagValues;
    magnitudeValues = cache->mVectorDatasetValuesMag;
    magMax = cache->mVectorDatasetMagMaximum;
    vectorDataOnVertices = cache->mVectorDataType == QgsMeshDatasetGroupMetadata::DataOnVertices;
  }
  else
  {
    const QgsMeshDatasetGroupMetadata metadata = layer->dataProvider()->datasetGroupMetadata( datasetIndex.group() );
    magMax = metadata.maximum();
    vectorDataOnVertices = metadata.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices;

    int count;
    if ( vectorDataOnVertices )
      count = layer->nativeMesh()->vertices.count();
    else
      count = layer->nativeMesh()->faces.count();

    vectorDatasetValues = QgsMeshLayerUtils::datasetValues( layer, datasetIndex, 0, count );
    scalarActiveFaceFlagValues = layer->dataProvider()->areFacesActive( datasetIndex, 0, layer->nativeMesh()->faces.count() );
    magnitudeValues = QgsMeshLayerUtils::calculateMagnitudes( vectorDatasetValues );
  }

  mParticleField = std::make_unique<QgsVectorFieldParticleTracesField>(
    QgsMeshVectorFieldValueSource::create(
      *layer->triangularMesh(),
      vectorDatasetValues,
      scalarActiveFaceFlagValues,
      magnitudeValues,
      vectorDataOnVertices ? QgsMeshDatasetGroupMetadata::DataOnVertices : QgsMeshDatasetGroupMetadata::DataOnFaces,
      layer->extent(),
      magMax
    ),
    rendererContext,
    vectorSettings.vectorStrokeColoring()
  );

  // the whole layer is animated, not only the part of it which is currently on the device
  mParticleField->setMinimizeFieldSize( false );
  mParticleField->updateSize( mRendererContext );
}

QgsVectorFieldTraceAnimationGenerator::QgsVectorFieldTraceAnimationGenerator( const QgsVectorFieldTraceAnimationGenerator &other )
  : mParticleField( std::make_unique<QgsVectorFieldParticleTracesField>( *other.mParticleField ) )
  , mRendererContext( other.mRendererContext )
  , mFPS( other.mFPS )
  , mVpixMax( other.mVpixMax )
  , mParticleLifeTime( other.mParticleLifeTime )
{}

QgsVectorFieldTraceAnimationGenerator::~QgsVectorFieldTraceAnimationGenerator() = default;

void QgsVectorFieldTraceAnimationGenerator::seedRandomParticles( int count )
{
  mParticleField->setParticlesCount( count );
  mParticleField->addRandomParticles();
}

QImage QgsVectorFieldTraceAnimationGenerator::imageRendered()
{
  mParticleField->moveParticles();
  return mParticleField->image();
}

void QgsVectorFieldTraceAnimationGenerator::setFPS( int FPS )
{
  if ( FPS > 0 )
    mFPS = FPS;
  else
    mFPS = 1;

  updateFieldParameter();
}

void QgsVectorFieldTraceAnimationGenerator::setMaxSpeedPixel( int max )
{
  mVpixMax = max;
  updateFieldParameter();
}

void QgsVectorFieldTraceAnimationGenerator::setParticlesLifeTime( double particleLifeTime )
{
  mParticleLifeTime = particleLifeTime;
  updateFieldParameter();
}

void QgsVectorFieldTraceAnimationGenerator::setParticlesColor( const QColor &c )
{
  mParticleField->setParticlesColor( c );
}

void QgsVectorFieldTraceAnimationGenerator::setParticlesSize( double width )
{
  mParticleField->setParticleSize( width );
}

void QgsVectorFieldTraceAnimationGenerator::setTailFactor( double fct )
{
  mParticleField->setTailFactor( fct );
}

void QgsVectorFieldTraceAnimationGenerator::setMinimumTailLength( int l )
{
  mParticleField->setMinTailLength( l );
}

void QgsVectorFieldTraceAnimationGenerator::setTailPersitence( double p )
{
  if ( p < 0 )
    p = 0;
  if ( p > 1 )
    p = 1;
  mParticleField->setStumpFactor( int( 255 * p ) );
}

QgsVectorFieldTraceAnimationGenerator &QgsVectorFieldTraceAnimationGenerator::operator=( const QgsVectorFieldTraceAnimationGenerator &other )
{
  mParticleField = std::make_unique<QgsVectorFieldParticleTracesField>( *( other.mParticleField ) );
  const_cast<QgsRenderContext &>( mRendererContext ) = other.mRendererContext;
  mFPS = other.mFPS;
  mVpixMax = other.mVpixMax;
  mParticleLifeTime = other.mParticleLifeTime;

  return ( *this );
}

void QgsVectorFieldTraceAnimationGenerator::updateFieldParameter()
{
  const double fieldTimeStep = mVpixMax / static_cast<double>( mFPS );
  const double fieldLifeTime = mParticleLifeTime * mFPS * fieldTimeStep;
  mParticleField->setTimeStep( fieldTimeStep );
  mParticleField->setParticlesLifeTime( fieldLifeTime );
}
