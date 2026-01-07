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

#include <cmath>
#include <memory>

#include "qgsmarkersymbol.h"
#include "qgsmarkersymbollayer.h"
#include "qgspointdisplacementrenderer.h"
#include "qgsproperty.h"
#include "qgsstyleentityvisitor.h"
#include "qgssymbollayerutils.h"
#include "qgsunittypes.h"

QgsPointClusterRenderer::QgsPointClusterRenderer()
  : QgsPointDistanceRenderer( u"pointCluster"_s )
{
  mClusterSymbol = std::make_unique<QgsMarkerSymbol>( );
  mClusterSymbol->setSize( 4 );
  mClusterSymbol->setColor( QColor( 245, 75, 80 ) );

  QgsFontMarkerSymbolLayer *fm = new QgsFontMarkerSymbolLayer();
  fm->setFontFamily( QFont().defaultFamily() );
  fm->setColor( QColor( 255, 255, 255 ) );
  fm->setSize( 3.2 );
  fm->setOffset( QPointF( 0, -0.4 ) );
  fm->setDataDefinedProperty( QgsSymbolLayer::Property::Character, QgsProperty::fromExpression( u"@cluster_size"_s ) );
  mClusterSymbol->insertSymbolLayer( 1, fm );
}

Qgis::FeatureRendererFlags QgsPointClusterRenderer::flags() const
{
  Qgis::FeatureRendererFlags res;
  if ( mClusterSymbol && mClusterSymbol->flags().testFlag( Qgis::SymbolFlag::AffectsLabeling ) )
    res.setFlag( Qgis::FeatureRendererFlag::AffectsLabeling );
  if ( mRenderer && mRenderer->flags().testFlag( Qgis::FeatureRendererFlag::AffectsLabeling ) )
    res.setFlag( Qgis::FeatureRendererFlag::AffectsLabeling );
  return res;
}

QgsPointClusterRenderer *QgsPointClusterRenderer::clone() const
{
  QgsPointClusterRenderer *r = new QgsPointClusterRenderer();
  if ( mRenderer )
    r->setEmbeddedRenderer( mRenderer->clone() );
  r->setLabelFont( mLabelFont );
  r->setLabelColor( mLabelColor );
  r->setMinimumLabelScale( mMinLabelScale );
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

void QgsPointClusterRenderer::drawGroup( QPointF centerPoint, QgsRenderContext &context, const ClusteredGroup &group ) const
{
  if ( group.size() > 1 )
  {
    mClusterSymbol->renderPoint( centerPoint, &( group.at( 0 ).feature ), context, -1, false );
  }
  else
  {
    //single isolated symbol, draw it untouched
    QgsMarkerSymbol *symbol = group.at( 0 ).symbol();
    symbol->startRender( context );
    symbol->renderPoint( centerPoint, &( group.at( 0 ).feature ), context, -1, group.at( 0 ).isSelected );
    symbol->stopRender( context );
  }
}

void QgsPointClusterRenderer::startRender( QgsRenderContext &context, const QgsFields &fields )
{
  if ( mClusterSymbol )
  {
    mClusterSymbol->startRender( context, fields );
  }
  QgsPointDistanceRenderer::startRender( context, fields );
}

void QgsPointClusterRenderer::stopRender( QgsRenderContext &context )
{
  QgsPointDistanceRenderer::stopRender( context );
  if ( mClusterSymbol )
  {
    mClusterSymbol->stopRender( context );
  }
}

QgsFeatureRenderer *QgsPointClusterRenderer::create( QDomElement &symbologyElem, const QgsReadWriteContext &context )
{
  QgsPointClusterRenderer *r = new QgsPointClusterRenderer();
  r->setTolerance( symbologyElem.attribute( u"tolerance"_s, u"0.00001"_s ).toDouble() );
  r->setToleranceUnit( QgsUnitTypes::decodeRenderUnit( symbologyElem.attribute( u"toleranceUnit"_s, u"MapUnit"_s ) ) );
  r->setToleranceMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( symbologyElem.attribute( u"toleranceUnitScale"_s ) ) );

  //look for an embedded renderer <renderer-v2>
  QDomElement embeddedRendererElem = symbologyElem.firstChildElement( u"renderer-v2"_s );
  if ( !embeddedRendererElem.isNull() )
  {
    r->setEmbeddedRenderer( QgsFeatureRenderer::load( embeddedRendererElem, context ) );
  }

  //center symbol
  const QDomElement centerSymbolElem = symbologyElem.firstChildElement( u"symbol"_s );
  if ( !centerSymbolElem.isNull() )
  {
    r->setClusterSymbol( QgsSymbolLayerUtils::loadSymbol<QgsMarkerSymbol>( centerSymbolElem, context ).release() );
  }
  return r;
}

QgsMarkerSymbol *QgsPointClusterRenderer::clusterSymbol()
{
  return mClusterSymbol.get();
}

QDomElement QgsPointClusterRenderer::save( QDomDocument &doc, const QgsReadWriteContext &context )
{
  QDomElement rendererElement = doc.createElement( RENDERER_TAG_NAME );
  rendererElement.setAttribute( u"type"_s, u"pointCluster"_s );
  rendererElement.setAttribute( u"tolerance"_s, QString::number( mTolerance ) );
  rendererElement.setAttribute( u"toleranceUnit"_s, QgsUnitTypes::encodeUnit( mToleranceUnit ) );
  rendererElement.setAttribute( u"toleranceUnitScale"_s, QgsSymbolLayerUtils::encodeMapUnitScale( mToleranceMapUnitScale ) );

  if ( mRenderer )
  {
    const QDomElement embeddedRendererElem = mRenderer->save( doc, context );
    rendererElement.appendChild( embeddedRendererElem );
  }
  if ( mClusterSymbol )
  {
    const QDomElement centerSymbolElem = QgsSymbolLayerUtils::saveSymbol( u"centerSymbol"_s, mClusterSymbol.get(), doc, context );
    rendererElement.appendChild( centerSymbolElem );
  }

  saveRendererData( doc, rendererElement, context );

  return rendererElement;
}

QSet<QString> QgsPointClusterRenderer::usedAttributes( const QgsRenderContext &context ) const
{
  QSet<QString> attr = QgsPointDistanceRenderer::usedAttributes( context );
  if ( mClusterSymbol )
    attr.unite( mClusterSymbol->usedAttributes( context ) );
  return attr;
}

bool QgsPointClusterRenderer::accept( QgsStyleEntityVisitorInterface *visitor ) const
{
  if ( !QgsPointDistanceRenderer::accept( visitor ) )
    return false;

  if ( mClusterSymbol )
  {
    QgsStyleSymbolEntity entity( mClusterSymbol.get() );
    if ( !visitor->visit( QgsStyleEntityVisitorInterface::StyleLeaf( &entity, u"cluster"_s, QObject::tr( "Cluster Symbol" ) ) ) )
      return false;
  }

  return true;
}

void QgsPointClusterRenderer::setClusterSymbol( QgsMarkerSymbol *symbol )
{
  mClusterSymbol.reset( symbol );
}

QgsPointClusterRenderer *QgsPointClusterRenderer::convertFromRenderer( const QgsFeatureRenderer *renderer )
{
  if ( renderer->type() == "pointCluster"_L1 )
  {
    return dynamic_cast<QgsPointClusterRenderer *>( renderer->clone() );
  }
  else if ( renderer->type() == "singleSymbol"_L1 ||
            renderer->type() == "categorizedSymbol"_L1 ||
            renderer->type() == "graduatedSymbol"_L1 ||
            renderer->type() == "RuleRenderer"_L1 )
  {
    QgsPointClusterRenderer *pointRenderer = new QgsPointClusterRenderer();
    pointRenderer->setEmbeddedRenderer( renderer->clone() );
    renderer->copyRendererData( pointRenderer );
    return pointRenderer;
  }
  else if ( renderer->type() == "pointDisplacement"_L1 )
  {
    QgsPointClusterRenderer *pointRenderer = new QgsPointClusterRenderer();
    const QgsPointDisplacementRenderer *displacementRenderer = static_cast< const QgsPointDisplacementRenderer * >( renderer );
    if ( displacementRenderer->embeddedRenderer() )
      pointRenderer->setEmbeddedRenderer( displacementRenderer->embeddedRenderer()->clone() );
    pointRenderer->setTolerance( displacementRenderer->tolerance() );
    pointRenderer->setToleranceUnit( displacementRenderer->toleranceUnit() );
    pointRenderer->setToleranceMapUnitScale( displacementRenderer->toleranceMapUnitScale() );
    renderer->copyRendererData( pointRenderer );
    return pointRenderer;
  }
  else
  {
    return nullptr;
  }
}
