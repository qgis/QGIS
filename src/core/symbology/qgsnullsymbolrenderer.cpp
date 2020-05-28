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
  : QgsFeatureRenderer( QStringLiteral( "nullSymbol" ) )
{
}

QgsSymbol *QgsNullSymbolRenderer::symbolForFeature( const QgsFeature &, QgsRenderContext & ) const
{
  return nullptr;
}

QgsSymbol *QgsNullSymbolRenderer::originalSymbolForFeature( const QgsFeature &, QgsRenderContext & ) const
{
  return nullptr;
}

bool QgsNullSymbolRenderer::renderFeature( const QgsFeature &feature, QgsRenderContext &context, int layer, bool selected, bool drawVertexMarker )
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

  if ( !mSymbol )
  {
    //create default symbol
    mSymbol.reset( QgsSymbol::defaultSymbol( feature.geometry().type() ) );
    mSymbol->startRender( context );
  }

  mSymbol->renderFeature( feature, context, layer, selected, drawVertexMarker, mCurrentVertexMarkerType, mCurrentVertexMarkerSize );

  return true;
}

void QgsNullSymbolRenderer::stopRender( QgsRenderContext &context )
{
  QgsFeatureRenderer::stopRender( context );

  if ( mSymbol )
  {
    mSymbol->stopRender( context );
  }
}

bool QgsNullSymbolRenderer::willRenderFeature( const QgsFeature &, QgsRenderContext & ) const
{
  //return true for every feature - so they are still selectable
  return true;
}

QSet<QString> QgsNullSymbolRenderer::usedAttributes( const QgsRenderContext & ) const
{
  return QSet<QString>();
}

QString QgsNullSymbolRenderer::dump() const
{
  return QStringLiteral( "NULL" );
}

QgsFeatureRenderer *QgsNullSymbolRenderer::clone() const
{
  QgsNullSymbolRenderer *r = new QgsNullSymbolRenderer();
  return r;
}

QgsSymbolList QgsNullSymbolRenderer::symbols( QgsRenderContext & ) const
{
  return QgsSymbolList();
}

QgsFeatureRenderer *QgsNullSymbolRenderer::create( QDomElement &element, const QgsReadWriteContext &context )
{
  Q_UNUSED( element )
  Q_UNUSED( context )
  QgsNullSymbolRenderer *r = new QgsNullSymbolRenderer();
  return r;
}

QDomElement QgsNullSymbolRenderer::save( QDomDocument &doc, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )
  QDomElement rendererElem = doc.createElement( RENDERER_TAG_NAME );
  rendererElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "nullSymbol" ) );
  return rendererElem;
}

QgsNullSymbolRenderer *QgsNullSymbolRenderer::convertFromRenderer( const QgsFeatureRenderer *renderer )
{
  Q_UNUSED( renderer )
  return new QgsNullSymbolRenderer();
}
