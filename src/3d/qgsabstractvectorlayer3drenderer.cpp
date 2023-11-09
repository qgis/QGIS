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

  QDomElement elemTiling = doc.createElement( QStringLiteral( "vector-layer-3d-tiling" ) );
  elemTiling.setAttribute( QStringLiteral( "zoom-levels-count" ), mZoomLevelsCount );
  elemTiling.setAttribute( QStringLiteral( "show-bounding-boxes" ), mShowBoundingBoxes ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  elem.appendChild( elemTiling );
}

void QgsVectorLayer3DTilingSettings::readXml( const QDomElement &elem )
{
  const QDomElement elemTiling = elem.firstChildElement( QStringLiteral( "vector-layer-3d-tiling" ) );
  if ( !elemTiling.isNull() )
  {
    mZoomLevelsCount = elemTiling.attribute( QStringLiteral( "zoom-levels-count" ) ).toInt();
    mShowBoundingBoxes = elemTiling.attribute( QStringLiteral( "show-bounding-boxes" ) ).toInt();
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
  elem.setAttribute( QStringLiteral( "layer" ), mLayerRef.layerId );
  mTilingSettings.writeXml( elem );
}

void QgsAbstractVectorLayer3DRenderer::readXmlBaseProperties( const QDomElement &elem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )
  mLayerRef = QgsMapLayerRef( elem.attribute( QStringLiteral( "layer" ) ) );
  mTilingSettings.readXml( elem );
}

void QgsAbstractVectorLayer3DRenderer::resolveReferences( const QgsProject &project )
{
  mLayerRef.setLayer( project.mapLayer( mLayerRef.layerId ) );
}
