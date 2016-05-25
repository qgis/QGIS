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
#include "qgsgeometrygeneratorsymbollayerv2.h"
#include "qgsfillsymbollayerv2.h"
#include "qgspainteffect.h"
#include "qgseffectstack.h"
#include "qgsgloweffect.h"

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
    : QgsFeatureRendererV2( "25dRenderer" )
{
  mSymbol.reset( new QgsFillSymbolV2() );

  mSymbol->deleteSymbolLayer( 0 ); // We never asked for the default layer

  QgsSymbolLayerV2* floor = QgsSimpleFillSymbolLayerV2::create();

  QgsStringMap wallProperties;
  wallProperties.insert( "geometryModifier", WALL_EXPRESSION );
  wallProperties.insert( "symbolType", "Fill" );
  QgsSymbolLayerV2* walls = QgsGeometryGeneratorSymbolLayerV2::create( wallProperties );

  QgsStringMap roofProperties;
  roofProperties.insert( "geometryModifier", ROOF_EXPRESSION );
  roofProperties.insert( "symbolType", "Fill" );
  QgsSymbolLayerV2* roof = QgsGeometryGeneratorSymbolLayerV2::create( roofProperties );

  floor->setLocked( true );

  mSymbol->appendSymbolLayer( floor );
  mSymbol->appendSymbolLayer( walls );
  mSymbol->appendSymbolLayer( roof );

  QgsEffectStack* effectStack = new QgsEffectStack();
  QgsOuterGlowEffect* glowEffect = new QgsOuterGlowEffect();
  glowEffect->setBlurLevel( 5 );
  glowEffect->setSpreadUnit( QgsSymbolV2::MapUnit );
  effectStack->appendEffect( glowEffect );
  floor->setPaintEffect( effectStack );

  // These methods must only be used after the above initialization!

  setRoofColor( QColor( "#b1a97c" ) );
  setWallColor( QColor( "#777777" ) );

  wallLayer()->setDataDefinedProperty( "color", new QgsDataDefined( QString( WALL_SHADING_EXPRESSION ) ) );

  setShadowSpread( 4 );
  setShadowColor( QColor( "#111111" ) );

  QgsFeatureRequest::OrderBy orderBy;
  orderBy << QgsFeatureRequest::OrderByClause(
    ORDER_BY_EXPRESSION,
    false );

  setOrderBy( orderBy );
  setOrderByEnabled( true );
}

QDomElement Qgs25DRenderer::save( QDomDocument& doc )
{
  QDomElement rendererElem = doc.createElement( RENDERER_TAG_NAME );

  rendererElem.setAttribute( "type", "25dRenderer" );

  QDomElement symbolElem = QgsSymbolLayerV2Utils::saveSymbol( "symbol", mSymbol.data(), doc );

  rendererElem.appendChild( symbolElem );

  return rendererElem;
}

QgsFeatureRendererV2* Qgs25DRenderer::create( QDomElement& element )
{
  Qgs25DRenderer* renderer = new Qgs25DRenderer();

  QDomNodeList symbols = element.elementsByTagName( "symbol" );
  if ( symbols.size() )
  {
    renderer->mSymbol.reset( QgsSymbolLayerV2Utils::loadSymbol( symbols.at( 0 ).toElement() ) );
  }

  return renderer;
}

void Qgs25DRenderer::startRender( QgsRenderContext& context, const QgsFields& fields )
{
  mSymbol->startRender( context, &fields );
}

void Qgs25DRenderer::stopRender( QgsRenderContext& context )
{
  mSymbol->stopRender( context );
}

QList<QString> Qgs25DRenderer::usedAttributes()
{
  return mSymbol->usedAttributes().toList();
}

QgsFeatureRendererV2* Qgs25DRenderer::clone() const
{
  Qgs25DRenderer* c = new Qgs25DRenderer();
  c->mSymbol.reset( mSymbol->clone() );
  return c;
}

QgsSymbolV2*Qgs25DRenderer::symbolForFeature( QgsFeature& feature, QgsRenderContext& context )
{
  Q_UNUSED( feature )
  Q_UNUSED( context )
  return mSymbol.data();
}

QgsSymbolV2List Qgs25DRenderer::symbols( QgsRenderContext& context )
{
  Q_UNUSED( context );
  QgsSymbolV2List lst;
  lst.append( mSymbol.data() );
  return lst;
}

QgsFillSymbolLayerV2* Qgs25DRenderer::roofLayer() const
{
  return static_cast<QgsFillSymbolLayerV2*>( mSymbol->symbolLayer( 2 )->subSymbol()->symbolLayer( 0 ) );
}

QgsFillSymbolLayerV2* Qgs25DRenderer::wallLayer() const
{
  return static_cast<QgsFillSymbolLayerV2*>( mSymbol->symbolLayer( 1 )->subSymbol()->symbolLayer( 0 ) );
}

QgsOuterGlowEffect* Qgs25DRenderer::glowEffect() const
{
  QgsEffectStack* stack = static_cast<QgsEffectStack*>( mSymbol->symbolLayer( 0 )->paintEffect() );
  return static_cast<QgsOuterGlowEffect*>( stack->effect( 0 ) );
}

bool Qgs25DRenderer::shadowEnabled() const
{
  return glowEffect()->enabled();
}

void Qgs25DRenderer::setShadowEnabled( bool value )
{
  glowEffect()->setEnabled( value );
}

QColor Qgs25DRenderer::shadowColor() const
{
  return glowEffect()->color();
}

void Qgs25DRenderer::setShadowColor( const QColor& shadowColor )
{
  glowEffect()->setColor( shadowColor );
}

double Qgs25DRenderer::shadowSpread() const
{
  return glowEffect()->spread();
}

void Qgs25DRenderer::setShadowSpread( double spread )
{
  glowEffect()->setSpread( spread );
}

QColor Qgs25DRenderer::wallColor() const
{
  return wallLayer()->fillColor();
}

void Qgs25DRenderer::setWallColor( const QColor& wallColor )
{
  wallLayer()->setFillColor( wallColor );
  wallLayer()->setOutlineColor( wallColor );
}

void Qgs25DRenderer::setWallShadingEnabled( bool enabled )
{
  wallLayer()->getDataDefinedProperty( "color" )->setActive( enabled );
}

bool Qgs25DRenderer::wallShadingEnabled()
{
  return wallLayer()->getDataDefinedProperty( "color" )->isActive();
}

QColor Qgs25DRenderer::roofColor() const
{
  return roofLayer()->fillColor();
}

void Qgs25DRenderer::setRoofColor( const QColor& roofColor )
{
  roofLayer()->setFillColor( roofColor );
  roofLayer()->setOutlineColor( roofColor );
}

Qgs25DRenderer* Qgs25DRenderer::convertFromRenderer( QgsFeatureRendererV2* renderer )
{
  if ( renderer->type() == "25dRenderer" )
  {
    return static_cast<Qgs25DRenderer*>( renderer->clone() );
  }
  else
  {
    return new Qgs25DRenderer();
  }
}

