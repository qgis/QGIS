/***************************************************************************
  qgstiledscenelayer3drenderer.cpp
  --------------------------------------
  Date                 : July 2023
  Copyright            : (C) 2023 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstiledscenelayer3drenderer.h"

#include "qgs3dmapsettings.h"
#include "qgstiledscenechunkloader_p.h"
#include "qgstiledsceneindex.h"
#include "qgstiledscenelayer.h"
#include "qgstiledscenelayerelevationproperties.h"

QgsTiledSceneLayer3DRendererMetadata::QgsTiledSceneLayer3DRendererMetadata()
  : Qgs3DRendererAbstractMetadata( u"tiledscene"_s )
{
}

QgsAbstract3DRenderer *QgsTiledSceneLayer3DRendererMetadata::createRenderer( QDomElement &elem, const QgsReadWriteContext &context )
{
  QgsTiledSceneLayer3DRenderer *r = new QgsTiledSceneLayer3DRenderer;
  r->readXml( elem, context );
  return r;
}

///

QgsTiledSceneLayer3DRenderer::QgsTiledSceneLayer3DRenderer()
{
}

void QgsTiledSceneLayer3DRenderer::setLayer( QgsTiledSceneLayer *layer )
{
  mLayerRef = QgsMapLayerRef( layer );
}

QgsTiledSceneLayer *QgsTiledSceneLayer3DRenderer::layer() const
{
  return qobject_cast<QgsTiledSceneLayer *>( mLayerRef.layer );
}

QgsAbstract3DRenderer *QgsTiledSceneLayer3DRenderer::clone() const
{
  QgsTiledSceneLayer3DRenderer *r = new QgsTiledSceneLayer3DRenderer;
  r->setLayer( layer() );
  return r;
}

Qt3DCore::QEntity *QgsTiledSceneLayer3DRenderer::createEntity( Qgs3DMapSettings *map ) const
{
  QgsTiledSceneLayer *tsl = layer();
  if ( !tsl || !tsl->dataProvider() )
    return nullptr;

  QgsTiledSceneIndex index = tsl->dataProvider()->index();

  return new QgsTiledSceneLayerChunkedEntity( map, index, tsl->dataProvider()->sceneCrs(), tsl->dataProvider()->crs(), maximumScreenError(), showBoundingBoxes(), qgis::down_cast<const QgsTiledSceneLayerElevationProperties *>( tsl->elevationProperties() )->zScale(), qgis::down_cast<const QgsTiledSceneLayerElevationProperties *>( tsl->elevationProperties() )->zOffset() );
}

void QgsTiledSceneLayer3DRenderer::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( context )

  QDomDocument doc = elem.ownerDocument();

  elem.setAttribute( u"layer"_s, mLayerRef.layerId );
  elem.setAttribute( u"max-screen-error"_s, maximumScreenError() );
  elem.setAttribute( u"show-bounding-boxes"_s, showBoundingBoxes() ? u"1"_s : u"0"_s );
}

void QgsTiledSceneLayer3DRenderer::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )

  mLayerRef = QgsMapLayerRef( elem.attribute( u"layer"_s ) );

  mShowBoundingBoxes = elem.attribute( u"show-bounding-boxes"_s, u"0"_s ).toInt();
  mMaximumScreenError = elem.attribute( u"max-screen-error"_s, u"16.0"_s ).toDouble();
}

void QgsTiledSceneLayer3DRenderer::resolveReferences( const QgsProject &project )
{
  mLayerRef.setLayer( project.mapLayer( mLayerRef.layerId ) );
}

double QgsTiledSceneLayer3DRenderer::maximumScreenError() const
{
  return mMaximumScreenError;
}

void QgsTiledSceneLayer3DRenderer::setMaximumScreenError( double error )
{
  mMaximumScreenError = error;
}

bool QgsTiledSceneLayer3DRenderer::showBoundingBoxes() const
{
  return mShowBoundingBoxes;
}

void QgsTiledSceneLayer3DRenderer::setShowBoundingBoxes( bool showBoundingBoxes )
{
  mShowBoundingBoxes = showBoundingBoxes;
}
