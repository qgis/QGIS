/***************************************************************************
  qgs25drenderer.cpp - qgs25drenderer
  -----------------------------------

 begin                : 14.1.2016
 Copyright            : (C) 2016 Matthias Kuhn
 Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgs25drenderer.h"
#include "qgsgeometrygeneratorsymbollayer.h"
#include "qgsfillsymbollayer.h"
#include "qgspainteffect.h"
#include "qgseffectstack.h"
#include "qgsgloweffect.h"
#include "qgsproperty.h"
#include "qgssymbollayerutils.h"
#include "qgsdatadefinedsizelegend.h"
#include "qgsstyleentityvisitor.h"
#include "qgsfillsymbol.h"

#define ROOF_EXPRESSION \
  "translate(" \
  "  $geometry," \
  "  cos( radians( eval( @qgis_25d_angle ) ) ) * eval( @qgis_25d_height )," \
  "  sin( radians( eval( @qgis_25d_angle ) ) ) * eval( @qgis_25d_height )" \
  ")"

#define WALL_EXPRESSION \
  "order_parts( "\
  "  extrude(" \
  "    segments_to_lines( $geometry )," \
  "    cos( radians( eval( @qgis_25d_angle ) ) ) * eval( @qgis_25d_height )," \
  "    sin( radians( eval( @qgis_25d_angle ) ) ) * eval( @qgis_25d_height )" \
  "  )," \
  "  'distance(  $geometry,  translate(    @map_extent_center,    1000 * @map_extent_width * cos( radians( @qgis_25d_angle + 180 ) ),    1000 * @map_extent_width * sin( radians( @qgis_25d_angle + 180 ) )  ))'," \
  "  False" \
  ")"

#define ORDER_BY_EXPRESSION \
  "distance(" \
  "  $geometry," \
  "  translate(" \
  "    @map_extent_center," \
  "    1000 * @map_extent_width * cos( radians( @qgis_25d_angle + 180 ) )," \
  "    1000 * @map_extent_width * sin( radians( @qgis_25d_angle + 180 ) )" \
  "  )" \
  ")"

#define WALL_SHADING_EXPRESSION \
  "set_color_part( " \
  "  @symbol_color," \
  " 'value'," \
  "  40 + 19 * abs( $pi - azimuth( " \
  "    point_n( geometry_n($geometry, @geometry_part_num) , 1 ), " \
  "    point_n( geometry_n($geometry, @geometry_part_num) , 2 )" \
  "  ) ) " \
  ")"

Qgs25DRenderer::Qgs25DRenderer()
  : QgsFeatureRenderer( QStringLiteral( "25dRenderer" ) )
{
  mSymbol.reset( new QgsFillSymbol() );

  mSymbol->deleteSymbolLayer( 0 ); // We never asked for the default layer

  QgsSymbolLayer *floor = QgsSimpleFillSymbolLayer::create();

  QVariantMap wallProperties;
  wallProperties.insert( QStringLiteral( "geometryModifier" ), WALL_EXPRESSION );
  wallProperties.insert( QStringLiteral( "symbolType" ), QStringLiteral( "Fill" ) );
  QgsSymbolLayer *walls = QgsGeometryGeneratorSymbolLayer::create( wallProperties );

  QVariantMap roofProperties;
  roofProperties.insert( QStringLiteral( "geometryModifier" ), ROOF_EXPRESSION );
  roofProperties.insert( QStringLiteral( "symbolType" ), QStringLiteral( "Fill" ) );
  QgsSymbolLayer *roof = QgsGeometryGeneratorSymbolLayer::create( roofProperties );

  floor->setLocked( true );

  mSymbol->appendSymbolLayer( floor );
  mSymbol->appendSymbolLayer( walls );
  mSymbol->appendSymbolLayer( roof );

  QgsEffectStack *effectStack = new QgsEffectStack();
  QgsOuterGlowEffect *glowEffect = new QgsOuterGlowEffect();
  glowEffect->setBlurLevel( 5 );
  glowEffect->setSpreadUnit( QgsUnitTypes::RenderMapUnits );
  effectStack->appendEffect( glowEffect );
  floor->setPaintEffect( effectStack );

  // These methods must only be used after the above initialization!

  setRoofColor( QColor( 177, 169, 124 ) );
  setWallColor( QColor( 119, 119, 119 ) );

  wallLayer()->setDataDefinedProperty( QgsSymbolLayer::PropertyFillColor, QgsProperty::fromExpression( QString( WALL_SHADING_EXPRESSION ) ) );

  setShadowSpread( 4 );
  setShadowColor( QColor( 17, 17, 17 ) );

  QgsFeatureRequest::OrderBy orderBy;
  orderBy << QgsFeatureRequest::OrderByClause(
            ORDER_BY_EXPRESSION,
            false );

  setOrderBy( orderBy );
  setOrderByEnabled( true );
}

QDomElement Qgs25DRenderer::save( QDomDocument &doc, const QgsReadWriteContext &context )
{
  QDomElement rendererElem = doc.createElement( RENDERER_TAG_NAME );

  rendererElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "25dRenderer" ) );

  const QDomElement symbolElem = QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "symbol" ), mSymbol.get(), doc, context );

  saveRendererData( doc, rendererElem, context );

  rendererElem.appendChild( symbolElem );

  return rendererElem;
}

QgsFeatureRenderer *Qgs25DRenderer::create( QDomElement &element, const QgsReadWriteContext &context )
{
  Qgs25DRenderer *renderer = new Qgs25DRenderer();

  const QDomNodeList symbols = element.elementsByTagName( QStringLiteral( "symbol" ) );
  if ( symbols.size() )
  {
    renderer->mSymbol.reset( QgsSymbolLayerUtils::loadSymbol( symbols.at( 0 ).toElement(), context ) );
  }

  return renderer;
}

void Qgs25DRenderer::startRender( QgsRenderContext &context, const QgsFields &fields )
{
  QgsFeatureRenderer::startRender( context, fields );

  mSymbol->startRender( context, fields );
}

void Qgs25DRenderer::stopRender( QgsRenderContext &context )
{
  QgsFeatureRenderer::stopRender( context );

  mSymbol->stopRender( context );
}

QSet<QString> Qgs25DRenderer::usedAttributes( const QgsRenderContext &context ) const
{
  return mSymbol->usedAttributes( context );
}

QgsFeatureRenderer *Qgs25DRenderer::clone() const
{
  Qgs25DRenderer *c = new Qgs25DRenderer();
  c->mSymbol.reset( mSymbol->clone() );
  return c;
}

QgsSymbol *Qgs25DRenderer::symbolForFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  Q_UNUSED( feature )
  Q_UNUSED( context )
  return mSymbol.get();
}

QgsSymbolList Qgs25DRenderer::symbols( QgsRenderContext &context ) const
{
  Q_UNUSED( context )
  QgsSymbolList lst;
  lst.append( mSymbol.get() );
  return lst;
}

bool Qgs25DRenderer::accept( QgsStyleEntityVisitorInterface *visitor ) const
{
  if ( mSymbol )
  {
    QgsStyleSymbolEntity entity( mSymbol.get() );
    return visitor->visit( QgsStyleEntityVisitorInterface::StyleLeaf( &entity ) );
  }
  return true;
}

QgsFillSymbolLayer *Qgs25DRenderer::roofLayer() const
{
  return static_cast<QgsFillSymbolLayer *>( mSymbol->symbolLayer( 2 )->subSymbol()->symbolLayer( 0 ) );
}

QgsFillSymbolLayer *Qgs25DRenderer::wallLayer() const
{
  return static_cast<QgsFillSymbolLayer *>( mSymbol->symbolLayer( 1 )->subSymbol()->symbolLayer( 0 ) );
}

QgsOuterGlowEffect *Qgs25DRenderer::glowEffect() const
{
  QgsEffectStack *stack = static_cast<QgsEffectStack *>( mSymbol->symbolLayer( 0 )->paintEffect() );
  return static_cast<QgsOuterGlowEffect *>( stack->effect( 0 ) );
}

bool Qgs25DRenderer::shadowEnabled() const
{
  return glowEffect()->enabled();
}

void Qgs25DRenderer::setShadowEnabled( bool value ) const
{
  glowEffect()->setEnabled( value );
}

QColor Qgs25DRenderer::shadowColor() const
{
  return glowEffect()->color();
}

void Qgs25DRenderer::setShadowColor( const QColor &shadowColor ) const
{
  glowEffect()->setColor( shadowColor );
}

double Qgs25DRenderer::shadowSpread() const
{
  return glowEffect()->spread();
}

void Qgs25DRenderer::setShadowSpread( double spread ) const
{
  glowEffect()->setSpread( spread );
}

QColor Qgs25DRenderer::wallColor() const
{
  return wallLayer()->fillColor();
}

void Qgs25DRenderer::setWallColor( const QColor &wallColor ) const
{
  wallLayer()->setFillColor( wallColor );
  wallLayer()->setStrokeColor( wallColor );
}

void Qgs25DRenderer::setWallShadingEnabled( bool enabled ) const
{
  wallLayer()->dataDefinedProperties().property( QgsSymbolLayer::PropertyFillColor ).setActive( enabled );
}

bool Qgs25DRenderer::wallShadingEnabled() const
{
  return wallLayer()->dataDefinedProperties().property( QgsSymbolLayer::PropertyFillColor ).isActive();
}

QColor Qgs25DRenderer::roofColor() const
{
  return roofLayer()->fillColor();
}

void Qgs25DRenderer::setRoofColor( const QColor &roofColor ) const
{
  roofLayer()->setFillColor( roofColor );
  roofLayer()->setStrokeColor( roofColor );
}

Qgs25DRenderer *Qgs25DRenderer::convertFromRenderer( QgsFeatureRenderer *renderer )
{
  if ( renderer->type() == QLatin1String( "25dRenderer" ) )
  {
    return static_cast<Qgs25DRenderer *>( renderer->clone() );
  }
  else
  {
    std::unique_ptr< Qgs25DRenderer > res = std::make_unique< Qgs25DRenderer >();
    renderer->copyRendererData( res.get() );
    return res.release();
  }
}

