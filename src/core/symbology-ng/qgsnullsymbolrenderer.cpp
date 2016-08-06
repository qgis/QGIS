/***************************************************************************
    qgsnullsymbolrenderer.cpp
    ---------------------
    begin                : November 2014
    copyright            : (C) 2014 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsnullsymbolrenderer.h"
#include "qgssymbol.h"
#include "qgsgeometry.h"

#include <QDomDocument>
#include <QDomElement>

QgsNullSymbolRenderer::QgsNullSymbolRenderer()
    : QgsFeatureRenderer( "nullSymbol" )
{
}

QgsNullSymbolRenderer::~QgsNullSymbolRenderer()
{
}

QgsSymbol* QgsNullSymbolRenderer::symbolForFeature( QgsFeature& , QgsRenderContext& )
{
  return nullptr;
}

QgsSymbol* QgsNullSymbolRenderer::originalSymbolForFeature( QgsFeature&, QgsRenderContext& )
{
  return nullptr;
}

bool QgsNullSymbolRenderer::renderFeature( QgsFeature &feature, QgsRenderContext &context, int layer, bool selected, bool drawVertexMarker )
{
  //render selected features or features being edited only
  if ( !selected && !drawVertexMarker )
  {
    return true;
  }

  if ( !feature.hasGeometry() ||
       feature.geometry().type() == QgsWkbTypes::NullGeometry ||
       feature.geometry().type() == QgsWkbTypes::UnknownGeometry )
    return true;

  if ( mSymbol.isNull() )
  {
    //create default symbol
    mSymbol.reset( QgsSymbol::defaultSymbol( feature.geometry().type() ) );
    mSymbol->startRender( context );
  }

  mSymbol->renderFeature( feature, context, layer, selected, drawVertexMarker, mCurrentVertexMarkerType, mCurrentVertexMarkerSize );

  return true;
}

void QgsNullSymbolRenderer::startRender( QgsRenderContext& context, const QgsFields& fields )
{
  Q_UNUSED( context );
  Q_UNUSED( fields );
}

void QgsNullSymbolRenderer::stopRender( QgsRenderContext& context )
{
  if ( mSymbol.data() )
  {
    mSymbol->stopRender( context );
  }
}

bool QgsNullSymbolRenderer::willRenderFeature( QgsFeature&, QgsRenderContext& )
{
  //return true for every feature - so they are still selectable
  return true;
}

QList<QString> QgsNullSymbolRenderer::usedAttributes()
{
  return QList<QString>();
}

QString QgsNullSymbolRenderer::dump() const
{
  return QString( "NULL" );
}

QgsFeatureRenderer* QgsNullSymbolRenderer::clone() const
{
  QgsNullSymbolRenderer* r = new QgsNullSymbolRenderer();
  return r;
}

QgsSymbolList QgsNullSymbolRenderer::symbols( QgsRenderContext& )
{
  return QgsSymbolList();
}

QgsFeatureRenderer* QgsNullSymbolRenderer::create( QDomElement& element )
{
  Q_UNUSED( element );
  QgsNullSymbolRenderer* r = new QgsNullSymbolRenderer();
  return r;
}

QDomElement QgsNullSymbolRenderer::save( QDomDocument& doc )
{
  QDomElement rendererElem = doc.createElement( RENDERER_TAG_NAME );
  rendererElem.setAttribute( "type", "nullSymbol" );
  return rendererElem;
}

QgsNullSymbolRenderer* QgsNullSymbolRenderer::convertFromRenderer( const QgsFeatureRenderer *renderer )
{
  Q_UNUSED( renderer );
  return new QgsNullSymbolRenderer();
}
