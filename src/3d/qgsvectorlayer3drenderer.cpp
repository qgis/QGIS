/***************************************************************************
  qgsvectorlayer3drenderer.cpp
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorlayer3drenderer.h"

#include "qgschunkedentity_p.h"
#include "qgsline3dsymbol.h"
#include "qgspoint3dsymbol.h"
#include "qgspolygon3dsymbol.h"
#include "qgsvectorlayerchunkloader_p.h"

#include "qgsvectorlayer.h"
#include "qgsxmlutils.h"


QgsVectorLayer3DRendererMetadata::QgsVectorLayer3DRendererMetadata()
  : Qgs3DRendererAbstractMetadata( QStringLiteral( "vector" ) )
{
}

QgsAbstract3DRenderer *QgsVectorLayer3DRendererMetadata::createRenderer( QDomElement &elem, const QgsReadWriteContext &context )
{
  QgsVectorLayer3DRenderer *r = new QgsVectorLayer3DRenderer;
  r->readXml( elem, context );
  return r;
}


// ---------


QgsVectorLayer3DRenderer::QgsVectorLayer3DRenderer( QgsAbstract3DSymbol *s )
  : mSymbol( s )
{
}

QgsVectorLayer3DRenderer *QgsVectorLayer3DRenderer::clone() const
{
  QgsVectorLayer3DRenderer *r = new QgsVectorLayer3DRenderer( mSymbol ? mSymbol->clone() : nullptr );
  copyBaseProperties( r );
  return r;
}

void QgsVectorLayer3DRenderer::setSymbol( QgsAbstract3DSymbol *symbol )
{
  mSymbol.reset( symbol );
}

const QgsAbstract3DSymbol *QgsVectorLayer3DRenderer::symbol() const
{
  return mSymbol.get();
}

Qt3DCore::QEntity *QgsVectorLayer3DRenderer::createEntity( const Qgs3DMapSettings &map ) const
{
  QgsVectorLayer *vl = layer();

  if ( !mSymbol || !vl )
    return nullptr;

  return new QgsVectorLayerChunkedEntity( vl, tilingSettings(), mSymbol.get(), map );
}

void QgsVectorLayer3DRenderer::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  QDomDocument doc = elem.ownerDocument();

  writeXmlBaseProperties( elem, context );

  QDomElement elemSymbol = doc.createElement( QStringLiteral( "symbol" ) );
  if ( mSymbol )
  {
    elemSymbol.setAttribute( QStringLiteral( "type" ), mSymbol->type() );
    mSymbol->writeXml( elemSymbol, context );
  }
  elem.appendChild( elemSymbol );
}

void QgsVectorLayer3DRenderer::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  readXmlBaseProperties( elem, context );

  QDomElement elemSymbol = elem.firstChildElement( QStringLiteral( "symbol" ) );
  QString symbolType = elemSymbol.attribute( QStringLiteral( "type" ) );
  QgsAbstract3DSymbol *symbol = nullptr;
  if ( symbolType == QLatin1String( "polygon" ) )
    symbol = new QgsPolygon3DSymbol;
  else if ( symbolType == QLatin1String( "point" ) )
    symbol = new QgsPoint3DSymbol;
  else if ( symbolType == QLatin1String( "line" ) )
    symbol = new QgsLine3DSymbol;

  if ( symbol )
    symbol->readXml( elemSymbol, context );
  mSymbol.reset( symbol );
}
