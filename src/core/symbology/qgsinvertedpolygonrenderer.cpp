/***************************************************************************
    qgsinvertedpolygonrenderer.cpp
    ---------------------
    begin                : April 2014
    copyright            : (C) 2014 Hugo Mercier / Oslandia
    email                : hugo dot mercier at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsinvertedpolygonrenderer.h"

#include "qgsfeature.h"
#include "qgslogger.h"
#include "qgsogcutils.h"
#include "qgspainteffect.h"
#include "qgspainteffectregistry.h"
#include "qgsstyleentityvisitor.h"
#include "qgssymbol.h"
#include "qgssymbollayer.h"
#include "qgssymbollayerutils.h"
#include "qgsvectorlayer.h"

#include <QDomDocument>
#include <QDomElement>

QgsInvertedPolygonRenderer::QgsInvertedPolygonRenderer( QgsFeatureRenderer *subRenderer )
  : QgsMergedFeatureRenderer( u"invertedPolygonRenderer"_s, subRenderer )
{
  if ( !subRenderer )
  {
    mSubRenderer.reset( QgsFeatureRenderer::defaultRenderer( Qgis::GeometryType::Polygon ) );
  }
  mOperation = InvertOnly;
}

QString QgsInvertedPolygonRenderer::dump() const
{
  if ( !mSubRenderer )
  {
    return u"INVERTED: NULL"_s;
  }
  return "INVERTED [" + mSubRenderer->dump() + ']';
}

QgsInvertedPolygonRenderer *QgsInvertedPolygonRenderer::clone() const
{
  QgsInvertedPolygonRenderer *newRenderer = nullptr;
  if ( !mSubRenderer )
  {
    newRenderer = new QgsInvertedPolygonRenderer( nullptr );
  }
  else
  {
    newRenderer = new QgsInvertedPolygonRenderer( mSubRenderer->clone() );
  }
  newRenderer->setPreprocessingEnabled( preprocessingEnabled() );
  copyRendererData( newRenderer );
  return newRenderer;
}

QgsFeatureRenderer *QgsInvertedPolygonRenderer::create( QDomElement &element, const QgsReadWriteContext &context ) // cppcheck-suppress duplInheritedMember
{
  QgsInvertedPolygonRenderer *r = new QgsInvertedPolygonRenderer();
  //look for an embedded renderer <renderer-v2>
  QDomElement embeddedRendererElem = element.firstChildElement( u"renderer-v2"_s );
  if ( !embeddedRendererElem.isNull() )
  {
    QgsFeatureRenderer *renderer = QgsFeatureRenderer::load( embeddedRendererElem, context );
    r->setEmbeddedRenderer( renderer );
  }
  r->setPreprocessingEnabled( element.attribute( u"preprocessing"_s, u"0"_s ).toInt() == 1 );
  return r;
}

QDomElement QgsInvertedPolygonRenderer::save( QDomDocument &doc, const QgsReadWriteContext &context )
{
  // clazy:skip

  QDomElement rendererElem = doc.createElement( RENDERER_TAG_NAME );
  rendererElem.setAttribute( u"type"_s, u"invertedPolygonRenderer"_s );
  rendererElem.setAttribute( u"preprocessing"_s, preprocessingEnabled() ? u"1"_s : u"0"_s );

  if ( mSubRenderer )
  {
    const QDomElement embeddedRendererElem = mSubRenderer->save( doc, context );
    rendererElem.appendChild( embeddedRendererElem );
  }

  saveRendererData( doc, rendererElem, context );

  return rendererElem;
}

QgsInvertedPolygonRenderer *QgsInvertedPolygonRenderer::convertFromRenderer( const QgsFeatureRenderer *renderer ) // cppcheck-suppress duplInheritedMember
{
  if ( renderer->type() == "invertedPolygonRenderer"_L1 )
  {
    return dynamic_cast<QgsInvertedPolygonRenderer *>( renderer->clone() );
  }
  else if ( renderer->type() == "singleSymbol"_L1 ||
            renderer->type() == "categorizedSymbol"_L1 ||
            renderer->type() == "graduatedSymbol"_L1 ||
            renderer->type() == "RuleRenderer"_L1 )
  {
    auto res = std::make_unique< QgsInvertedPolygonRenderer >( renderer->clone() );
    renderer->copyRendererData( res.get() );
    return res.release();
  }
  else if ( renderer->type() == "mergedFeatureRenderer"_L1 )
  {
    auto res = std::make_unique< QgsInvertedPolygonRenderer >( renderer->embeddedRenderer() ? renderer->embeddedRenderer()->clone() : nullptr );
    renderer->copyRendererData( res.get() );
    return res.release();
  }
  return nullptr;
}

