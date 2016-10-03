/***************************************************************************
                              qgspointclusterrenderer.cpp
                              ---------------------------
  begin                : February 2016
  copyright            : (C) 2016 by Nyall Dawson
  email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointclusterrenderer.h"
#include "qgspointdisplacementrenderer.h"
#include "qgssymbollayerutils.h"
#include "qgspainteffectregistry.h"
#include "qgspainteffect.h"
#include "qgsmarkersymbollayer.h"
#include "qgsdatadefined.h"
#include <cmath>

#ifndef M_SQRT2
#define M_SQRT2 1.41421356237309504880
#endif

QgsPointClusterRenderer::QgsPointClusterRenderer()
    : QgsPointDistanceRenderer( "pointCluster" )
{
  mClusterSymbol.reset( new QgsMarkerSymbol() );
  mClusterSymbol->setSize( 4 );
  mClusterSymbol->setColor( QColor( 245, 75, 80 ) );

  QgsFontMarkerSymbolLayer* fm = new QgsFontMarkerSymbolLayer();
  fm->setFontFamily( QFont().defaultFamily() );
  fm->setColor( QColor( 255, 255, 255 ) );
  fm->setSize( 3.2 );
  fm->setOffset( QPointF( 0, -0.4 ) );
  fm->setDataDefinedProperty( "char", new QgsDataDefined( true, true, "@cluster_size" ) );
  mClusterSymbol->insertSymbolLayer( 1, fm );
}

QgsPointClusterRenderer* QgsPointClusterRenderer::clone() const
{
  QgsPointClusterRenderer* r = new QgsPointClusterRenderer();
  if ( mRenderer )
    r->setEmbeddedRenderer( mRenderer->clone() );
  r->setLabelFont( mLabelFont );
  r->setLabelColor( mLabelColor );
  r->setMaxLabelScaleDenominator( mMaxLabelScaleDenominator );
  r->setTolerance( mTolerance );
  r->setToleranceUnit( mToleranceUnit );
  r->setToleranceMapUnitScale( mToleranceMapUnitScale );
  if ( mClusterSymbol )
  {
    r->setClusterSymbol( mClusterSymbol->clone() );
  }
  copyRendererData( r );
  return r;
}

void QgsPointClusterRenderer::drawGroup( QPointF centerPoint, QgsRenderContext& context, const ClusteredGroup& group )
{
  if ( group.size() > 1 )
  {
    mClusterSymbol->renderPoint( centerPoint, &( group.at( 0 ).feature ), context, -1, false );
  }
  else
  {
    //single isolated symbol, draw it untouched
    QgsMarkerSymbol* symbol = group.at( 0 ).symbol;
    symbol->renderPoint( centerPoint, &( group.at( 0 ).feature ), context, -1, group.at( 0 ).isSelected );
  }
}

void QgsPointClusterRenderer::startRender( QgsRenderContext& context, const QgsFields& fields )
{
  if ( mClusterSymbol )
  {
    mClusterSymbol->startRender( context, fields );
  }
  QgsPointDistanceRenderer::startRender( context, fields );
}

void QgsPointClusterRenderer::stopRender( QgsRenderContext& context )
{
  QgsPointDistanceRenderer::stopRender( context );
  if ( mClusterSymbol )
  {
    mClusterSymbol->stopRender( context );
  }
}

QgsFeatureRenderer* QgsPointClusterRenderer::create( QDomElement& symbologyElem )
{
  QgsPointClusterRenderer* r = new QgsPointClusterRenderer();
  r->setTolerance( symbologyElem.attribute( "tolerance", "0.00001" ).toDouble() );
  r->setToleranceUnit( QgsUnitTypes::decodeRenderUnit( symbologyElem.attribute( "toleranceUnit", "MapUnit" ) ) );
  r->setToleranceMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( symbologyElem.attribute( "toleranceUnitScale" ) ) );

  //look for an embedded renderer <renderer-v2>
  QDomElement embeddedRendererElem = symbologyElem.firstChildElement( "renderer-v2" );
  if ( !embeddedRendererElem.isNull() )
  {
    r->setEmbeddedRenderer( QgsFeatureRenderer::load( embeddedRendererElem ) );
  }

  //center symbol
  QDomElement centerSymbolElem = symbologyElem.firstChildElement( "symbol" );
  if ( !centerSymbolElem.isNull() )
  {
    r->setClusterSymbol( QgsSymbolLayerUtils::loadSymbol<QgsMarkerSymbol>( centerSymbolElem ) );
  }
  return r;
}

QgsMarkerSymbol* QgsPointClusterRenderer::clusterSymbol()
{
  return mClusterSymbol.data();
}

QDomElement QgsPointClusterRenderer::save( QDomDocument& doc )
{
  QDomElement rendererElement = doc.createElement( RENDERER_TAG_NAME );
  rendererElement.setAttribute( "forceraster", ( mForceRaster ? "1" : "0" ) );
  rendererElement.setAttribute( "type", "pointCluster" );
  rendererElement.setAttribute( "tolerance", QString::number( mTolerance ) );
  rendererElement.setAttribute( "toleranceUnit", QgsUnitTypes::encodeUnit( mToleranceUnit ) );
  rendererElement.setAttribute( "toleranceUnitScale", QgsSymbolLayerUtils::encodeMapUnitScale( mToleranceMapUnitScale ) );

  if ( mRenderer )
  {
    QDomElement embeddedRendererElem = mRenderer->save( doc );
    rendererElement.appendChild( embeddedRendererElem );
  }
  if ( mClusterSymbol )
  {
    QDomElement centerSymbolElem = QgsSymbolLayerUtils::saveSymbol( "centerSymbol", mClusterSymbol.data(), doc );
    rendererElement.appendChild( centerSymbolElem );
  }

  if ( mPaintEffect && !QgsPaintEffectRegistry::isDefaultStack( mPaintEffect ) )
    mPaintEffect->saveProperties( doc, rendererElement );

  if ( !mOrderBy.isEmpty() )
  {
    QDomElement orderBy = doc.createElement( "orderby" );
    mOrderBy.save( orderBy );
    rendererElement.appendChild( orderBy );
  }
  rendererElement.setAttribute( "enableorderby", ( mOrderByEnabled ? "1" : "0" ) );

  return rendererElement;
}

QSet<QString> QgsPointClusterRenderer::usedAttributes() const
{
  QSet<QString> attr = QgsPointDistanceRenderer::usedAttributes();
  if ( mClusterSymbol )
    attr.unite( mClusterSymbol->usedAttributes() );
  return attr;
}

void QgsPointClusterRenderer::setClusterSymbol( QgsMarkerSymbol* symbol )
{
  mClusterSymbol.reset( symbol );
}

QgsPointClusterRenderer* QgsPointClusterRenderer::convertFromRenderer( const QgsFeatureRenderer* renderer )
{
  if ( renderer->type() == "pointCluster" )
  {
    return dynamic_cast<QgsPointClusterRenderer*>( renderer->clone() );
  }
  else if ( renderer->type() == "singleSymbol" ||
            renderer->type() == "categorizedSymbol" ||
            renderer->type() == "graduatedSymbol" ||
            renderer->type() == "RuleRenderer" )
  {
    QgsPointClusterRenderer* pointRenderer = new QgsPointClusterRenderer();
    pointRenderer->setEmbeddedRenderer( renderer->clone() );
    return pointRenderer;
  }
  else if ( renderer->type() == "pointDisplacement" )
  {
    QgsPointClusterRenderer* pointRenderer = new QgsPointClusterRenderer();
    const QgsPointDisplacementRenderer* displacementRenderer = static_cast< const QgsPointDisplacementRenderer* >( renderer );
    if ( displacementRenderer->embeddedRenderer() )
      pointRenderer->setEmbeddedRenderer( displacementRenderer->embeddedRenderer()->clone() );
    pointRenderer->setTolerance( displacementRenderer->tolerance() );
    pointRenderer->setToleranceUnit( displacementRenderer->toleranceUnit() );
    pointRenderer->setToleranceMapUnitScale( displacementRenderer->toleranceMapUnitScale() );
    if ( const_cast< QgsPointDisplacementRenderer* >( displacementRenderer )->centerSymbol() )
      pointRenderer->setClusterSymbol( const_cast< QgsPointDisplacementRenderer* >( displacementRenderer )->centerSymbol()->clone() );
    return pointRenderer;
  }
  else
  {
    return nullptr;
  }
}
