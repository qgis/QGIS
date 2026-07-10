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

#include "qgs3dsymbolregistry.h"
#include "qgs3dutils.h"
#include "qgsapplication.h"
#include "qgsrulebased3drenderer.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerchunkloader_p.h"
#include "qgsxmlutils.h"

#include <QString>

using namespace Qt::StringLiterals;

QgsVectorLayer3DRendererMetadata::QgsVectorLayer3DRendererMetadata()
  : Qgs3DRendererAbstractMetadata( u"vector"_s )
{}

QgsAbstract3DRenderer *QgsVectorLayer3DRendererMetadata::createRenderer( QDomElement &elem, const QgsReadWriteContext &context )
{
  QgsVectorLayer3DRenderer *r = new QgsVectorLayer3DRenderer;
  r->readXml( elem, context );
  return r;
}


// ---------


QgsVectorLayer3DRenderer::QgsVectorLayer3DRenderer( QgsAbstract3DSymbol *s )
  : mSymbol( s )
{}

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

Qt3DCore::QEntity *QgsVectorLayer3DRenderer::createEntity( Qgs3DMapSettings *map ) const
{
  QgsVectorLayer *vl = layer();

  if ( !mSymbol || !vl )
    return nullptr;

  return new QgsVectorLayerChunkedEntity( map, vl, Qgs3DUtils::MINIMUM_VECTOR_Z_ESTIMATE, Qgs3DUtils::MAXIMUM_VECTOR_Z_ESTIMATE, tilingSettings(), mSymbol.get() );
}

void QgsVectorLayer3DRenderer::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  QDomDocument doc = elem.ownerDocument();

  writeXmlBaseProperties( elem, context );

  QDomElement elemSymbol = doc.createElement( u"symbol"_s );
  if ( mSymbol )
  {
    elemSymbol.setAttribute( u"type"_s, mSymbol->type() );
    mSymbol->writeXml( elemSymbol, context );
  }
  elem.appendChild( elemSymbol );
}

void QgsVectorLayer3DRenderer::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  readXmlBaseProperties( elem, context );

  const QDomElement elemSymbol = elem.firstChildElement( u"symbol"_s );
  const QString symbolType = elemSymbol.attribute( u"type"_s );
  mSymbol.reset( QgsApplication::symbol3DRegistry()->createSymbol( symbolType ) );
  if ( mSymbol )
    mSymbol->readXml( elemSymbol, context );
}

std::unique_ptr<QgsVectorLayer3DRenderer> QgsVectorLayer3DRenderer::convertFromRenderer( const QgsAbstractVectorLayer3DRenderer *renderer, QgsVectorLayer * )
{
  std::unique_ptr< QgsVectorLayer3DRenderer > r;
  if ( renderer->type() == "vector"_L1 )
  {
    r.reset( dynamic_cast<const QgsVectorLayer3DRenderer *>( renderer )->clone() );
  }
  else if ( renderer->type() == "rulebased"_L1 )
  {
    const QgsRuleBased3DRenderer *ruleBasedRenderer = dynamic_cast<const QgsRuleBased3DRenderer *>( renderer );
    if ( !ruleBasedRenderer->rootRule()->children().isEmpty() )
    {
      std::unique_ptr< QgsAbstract3DSymbol > origSymbol;
      const QList< QgsRuleBased3DRenderer::Rule * > children = ruleBasedRenderer->rootRule()->children();
      for ( const QgsRuleBased3DRenderer::Rule *child : children )
      {
        if ( child->symbol() )
        {
          origSymbol.reset( child->symbol()->clone() );
          break;
        }
      }
      if ( origSymbol )
      {
        r = std::make_unique< QgsVectorLayer3DRenderer >( origSymbol.release() );
      }
    }
  }

  if ( r )
  {
    renderer->copyBaseProperties( r.get() );
  }

  return r;
}
