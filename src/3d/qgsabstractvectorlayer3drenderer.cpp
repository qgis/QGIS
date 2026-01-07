/***************************************************************************
  qgsabstractvectorlayer3drenderer.cpp
  --------------------------------------
  Date                 : January 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsabstractvectorlayer3drenderer.h"

#include "qgsvectorlayer.h"

void QgsVectorLayer3DTilingSettings::writeXml( QDomElement &elem ) const
{
  QDomDocument doc = elem.ownerDocument();

  QDomElement elemTiling = doc.createElement( u"vector-layer-3d-tiling"_s );
  elemTiling.setAttribute( u"zoom-levels-count"_s, mZoomLevelsCount );
  elemTiling.setAttribute( u"show-bounding-boxes"_s, mShowBoundingBoxes ? u"1"_s : u"0"_s );
  elem.appendChild( elemTiling );
}

void QgsVectorLayer3DTilingSettings::readXml( const QDomElement &elem )
{
  const QDomElement elemTiling = elem.firstChildElement( u"vector-layer-3d-tiling"_s );
  if ( !elemTiling.isNull() )
  {
    mZoomLevelsCount = elemTiling.attribute( u"zoom-levels-count"_s ).toInt();
    mShowBoundingBoxes = elemTiling.attribute( u"show-bounding-boxes"_s ).toInt();
  }
}


//////////////////


QgsAbstractVectorLayer3DRenderer::QgsAbstractVectorLayer3DRenderer() = default;

void QgsAbstractVectorLayer3DRenderer::setLayer( QgsVectorLayer *layer )
{
  mLayerRef = QgsMapLayerRef( layer );
}

QgsVectorLayer *QgsAbstractVectorLayer3DRenderer::layer() const
{
  return qobject_cast<QgsVectorLayer *>( mLayerRef.layer );
}

void QgsAbstractVectorLayer3DRenderer::copyBaseProperties( QgsAbstractVectorLayer3DRenderer *r ) const
{
  r->mLayerRef = mLayerRef;
  r->mTilingSettings = mTilingSettings;
}

void QgsAbstractVectorLayer3DRenderer::writeXmlBaseProperties( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( context )
  elem.setAttribute( u"layer"_s, mLayerRef.layerId );
  mTilingSettings.writeXml( elem );
}

void QgsAbstractVectorLayer3DRenderer::readXmlBaseProperties( const QDomElement &elem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )
  mLayerRef = QgsMapLayerRef( elem.attribute( u"layer"_s ) );
  mTilingSettings.readXml( elem );
}

void QgsAbstractVectorLayer3DRenderer::resolveReferences( const QgsProject &project )
{
  mLayerRef.setLayer( project.mapLayer( mLayerRef.layerId ) );
}
