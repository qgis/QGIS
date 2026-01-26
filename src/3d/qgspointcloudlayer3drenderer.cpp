/***************************************************************************
  qgspointcloudlayer3drenderer.cpp
  --------------------------------------
  Date                 : October 2020
  Copyright            : (C) 2020 by Peter Petrik
  Email                : zilolv dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointcloudlayer3drenderer.h"

#include <memory>

#include "qgs3dsymbolregistry.h"
#include "qgs3dutils.h"
#include "qgspointcloud3dsymbol.h"
#include "qgspointcloudindex.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudlayerchunkloader_p.h"
#include "qgspointcloudlayerelevationproperties.h"
#include "qgsvirtualpointcloudentity_p.h"
#include "qgsxmlutils.h"

QgsPointCloud3DRenderContext::QgsPointCloud3DRenderContext( const Qgs3DRenderContext &context, const QgsCoordinateTransform &coordinateTransform, std::unique_ptr<QgsPointCloud3DSymbol> symbol, double zValueScale, double zValueFixedOffset )
  : Qgs3DRenderContext( context )
  , mSymbol( std::move( symbol ) )
  , mZValueScale( zValueScale )
  , mZValueFixedOffset( zValueFixedOffset )
  , mCoordinateTransform( coordinateTransform )
  , mFeedback( new QgsFeedback )
{
  updateExtent();
}

void QgsPointCloud3DRenderContext::setAttributes( const QgsPointCloudAttributeCollection &attributes )
{
  mAttributes = attributes;
}

void QgsPointCloud3DRenderContext::setSymbol( QgsPointCloud3DSymbol *symbol )
{
  mSymbol.reset( symbol );
}

void QgsPointCloud3DRenderContext::setFilteredOutCategories( const QgsPointCloudCategoryList &categories )
{
  mFilteredOutCategories = categories;
}

QSet<int> QgsPointCloud3DRenderContext::getFilteredOutValues() const
{
  QSet<int> filteredOut;
  for ( const QgsPointCloudCategory &category : mFilteredOutCategories )
    filteredOut.insert( category.value() );
  return filteredOut;
}

void QgsPointCloud3DRenderContext::setCoordinateTransform( const QgsCoordinateTransform &coordinateTransform )
{
  mCoordinateTransform = coordinateTransform;
  updateExtent();
}

bool QgsPointCloud3DRenderContext::isCanceled() const
{
  return mFeedback->isCanceled();
}

void QgsPointCloud3DRenderContext::cancelRendering() const
{
  mFeedback->cancel();
}

void QgsPointCloud3DRenderContext::updateExtent()
{
  if ( extent().isEmpty() )
  {
    // an empty extent means no filter, so let's pass it without transformation
    mLayerExtent = QgsRectangle();
  }
  else
  {
    try
    {
      mLayerExtent = mCoordinateTransform.transformBoundingBox( extent(), Qgis::TransformDirection::Reverse );
    }
    catch ( const QgsCsException & )
    {
      // bad luck, can't reproject for some reason. Let's use an empty extent to skip filtering.
      QgsDebugError( u"Transformation of extent failed!"_s );
      mLayerExtent = QgsRectangle();
    }
  }
}
// ---------


QgsPointCloudLayer3DRendererMetadata::QgsPointCloudLayer3DRendererMetadata()
  : Qgs3DRendererAbstractMetadata( u"pointcloud"_s )
{
}

QgsAbstract3DRenderer *QgsPointCloudLayer3DRendererMetadata::createRenderer( QDomElement &elem, const QgsReadWriteContext &context )
{
  QgsPointCloudLayer3DRenderer *r = new QgsPointCloudLayer3DRenderer;
  r->readXml( elem, context );
  return r;
}


// ---------


QgsPointCloudLayer3DRenderer::QgsPointCloudLayer3DRenderer()
{
}

void QgsPointCloudLayer3DRenderer::setLayer( QgsPointCloudLayer *layer )
{
  mLayerRef = QgsMapLayerRef( layer );
}

QgsPointCloudLayer *QgsPointCloudLayer3DRenderer::layer() const
{
  return qobject_cast<QgsPointCloudLayer *>( mLayerRef.layer );
}

QString QgsPointCloudLayer3DRenderer::type() const
{
  return u"pointcloud"_s;
}

QgsPointCloudLayer3DRenderer *QgsPointCloudLayer3DRenderer::clone() const
{
  QgsPointCloudLayer3DRenderer *r = new QgsPointCloudLayer3DRenderer;
  if ( mSymbol )
  {
    QgsAbstract3DSymbol *symbolClone = mSymbol->clone();
    r->setSymbol( dynamic_cast<QgsPointCloud3DSymbol *>( symbolClone ) );
  }
  r->setMaximumScreenError( mMaximumScreenError );
  r->setShowBoundingBoxes( mShowBoundingBoxes );
  r->setZoomOutBehavior( mZoomOutBehavior );
  return r;
}

Qt3DCore::QEntity *QgsPointCloudLayer3DRenderer::createEntity( Qgs3DMapSettings *map ) const
{
  QgsPointCloudLayer *pcl = layer();
  if ( !pcl || !pcl->dataProvider() )
    return nullptr;
  if ( !mSymbol )
    return nullptr;

  const QgsCoordinateTransform coordinateTransform( pcl->crs3D(), map->crs(), map->transformContext() );

  Qt3DCore::QEntity *entity = nullptr;
  if ( pcl->index() )
  {
    entity = new QgsPointCloudLayerChunkedEntity( map, pcl, pcl->index(), coordinateTransform, mSymbol->clone(), static_cast<float>( maximumScreenError() ), showBoundingBoxes(), static_cast<const QgsPointCloudLayerElevationProperties *>( pcl->elevationProperties() )->zScale(), static_cast<const QgsPointCloudLayerElevationProperties *>( pcl->elevationProperties() )->zOffset(), mPointBudget );
  }
  else if ( !pcl->dataProvider()->subIndexes().isEmpty() )
  {
    entity = new QgsVirtualPointCloudEntity( map, pcl, coordinateTransform, mSymbol->clone(), static_cast<float>( maximumScreenError() ), showBoundingBoxes(), static_cast<const QgsPointCloudLayerElevationProperties *>( pcl->elevationProperties() )->zScale(), static_cast<const QgsPointCloudLayerElevationProperties *>( pcl->elevationProperties() )->zOffset(), mPointBudget );
  }
  return entity;
}

void QgsPointCloudLayer3DRenderer::setSymbol( QgsPointCloud3DSymbol *symbol )
{
  mSymbol.reset( symbol );
}

void QgsPointCloudLayer3DRenderer::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( context )

  QDomDocument doc = elem.ownerDocument();

  elem.setAttribute( u"layer"_s, mLayerRef.layerId );
  elem.setAttribute( u"max-screen-error"_s, maximumScreenError() );
  elem.setAttribute( u"show-bounding-boxes"_s, showBoundingBoxes() ? u"1"_s : u"0"_s );
  elem.setAttribute( u"point-budget"_s, mPointBudget );
  elem.setAttribute( u"zoom-out-behavior"_s, qgsEnumValueToKey( mZoomOutBehavior ) );

  QDomElement elemSymbol = doc.createElement( u"symbol"_s );
  if ( mSymbol )
  {
    elemSymbol.setAttribute( u"type"_s, mSymbol->symbolType() );
    mSymbol->writeXml( elemSymbol, context );
  }
  elem.appendChild( elemSymbol );
}

void QgsPointCloudLayer3DRenderer::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  mLayerRef = QgsMapLayerRef( elem.attribute( u"layer"_s ) );

  const QDomElement elemSymbol = elem.firstChildElement( u"symbol"_s );

  const QString symbolType = elemSymbol.attribute( u"type"_s );
  mShowBoundingBoxes = elem.attribute( u"show-bounding-boxes"_s, u"0"_s ).toInt();
  mMaximumScreenError = elem.attribute( u"max-screen-error"_s, u"3.0"_s ).toDouble();
  mPointBudget = elem.attribute( u"point-budget"_s, u"5000000"_s ).toInt();
  mZoomOutBehavior = qgsEnumKeyToValue( elem.attribute( u"zoom-out-behavior"_s ), Qgis::PointCloudZoomOutRenderBehavior::RenderExtents );

  if ( symbolType == "single-color"_L1 )
    mSymbol = std::make_unique<QgsSingleColorPointCloud3DSymbol>();
  else if ( symbolType == "color-ramp"_L1 )
    mSymbol = std::make_unique<QgsColorRampPointCloud3DSymbol>();
  else if ( symbolType == "rgb"_L1 )
    mSymbol = std::make_unique<QgsRgbPointCloud3DSymbol>();
  else if ( symbolType == "classification"_L1 )
    mSymbol = std::make_unique<QgsClassificationPointCloud3DSymbol>();
  else
    mSymbol.reset();

  if ( mSymbol )
    mSymbol->readXml( elemSymbol, context );
}

void QgsPointCloudLayer3DRenderer::resolveReferences( const QgsProject &project )
{
  mLayerRef.setLayer( project.mapLayer( mLayerRef.layerId ) );
}

double QgsPointCloudLayer3DRenderer::maximumScreenError() const
{
  return mMaximumScreenError;
}

void QgsPointCloudLayer3DRenderer::setMaximumScreenError( double error )
{
  mMaximumScreenError = error;
}

bool QgsPointCloudLayer3DRenderer::showBoundingBoxes() const
{
  return mShowBoundingBoxes;
}

void QgsPointCloudLayer3DRenderer::setShowBoundingBoxes( bool showBoundingBoxes )
{
  mShowBoundingBoxes = showBoundingBoxes;
}

void QgsPointCloudLayer3DRenderer::setPointRenderingBudget( int budget )
{
  mPointBudget = budget;
}

bool QgsPointCloudLayer3DRenderer::convertFrom2DRenderer( QgsPointCloudRenderer *renderer )
{
  std::unique_ptr<QgsPointCloudLayer3DRenderer> renderer3D = Qgs3DUtils::convert2DPointCloudRendererTo3D( renderer );
  if ( !renderer3D )
  {
    setSymbol( nullptr );
    return false;
  }

  QgsPointCloud3DSymbol *newSymbol = const_cast<QgsPointCloud3DSymbol *>(
    static_cast<QgsPointCloud3DSymbol *>( renderer3D->symbol()->clone() )
  );
  // we need to retain some settings from the previous symbol, like point size
  const QgsPointCloud3DSymbol *oldSymbol = symbol();
  if ( oldSymbol )
    oldSymbol->copyBaseSettings( newSymbol );
  setSymbol( newSymbol );
  return true;
}
