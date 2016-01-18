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
  "    segments_to_lines( exterior_ring( $geometry ) )," \
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

Qgs25DRenderer::Qgs25DRenderer()
    : QgsFeatureRendererV2( "25dRenderer" )
{
  mSymbol.reset( new QgsFillSymbolV2() );

  mSymbol->deleteSymbolLayer( 0 ); // We never asked for the default layer

  QgsSymbolLayerV2* floor = QgsSimpleFillSymbolLayerV2::create();

  QgsStringMap wallProperties;
  wallProperties.insert( "geometryModifier", WALL_EXPRESSION );
  wallProperties.insert( "symbolType", "Fill" );
  QgsSymbolLayerV2* walls = QgsGeometryGeneratorSymbolLayerV2::create( wallProperties );;

  QgsStringMap roofProperties;
  roofProperties.insert( "geometryModifier", ROOF_EXPRESSION );
  roofProperties.insert( "symbolType", "Fill" );
  QgsSymbolLayerV2* roof = QgsGeometryGeneratorSymbolLayerV2::create( roofProperties );

  mSymbol->appendSymbolLayer( floor );
  mSymbol->appendSymbolLayer( walls );
  mSymbol->appendSymbolLayer( roof );

  QgsOuterGlowEffect* glowEffect = new QgsOuterGlowEffect();
  glowEffect->setBlurLevel( 5 );
  glowEffect->setSpreadUnit( QgsSymbolV2::MapUnit );
  floor->setPaintEffect( glowEffect );

  // These methods must only be used after the above initialisation!

  setRoofColor( QColor( "#fdbf6f" ) );
  setWallColor( QColor( "#777777" ) );

  setShadowSpread( 4 );
  setShadowColor( QColor( "#1111111" ) );

  setHeight( "20" );
  setAngle( 40 );

  QgsFeatureRequest::OrderBy orderBy;
  orderBy << QgsFeatureRequest::OrderByClause(
    ORDER_BY_EXPRESSION,
    false );

  setOrderBy( orderBy );
}

QDomElement Qgs25DRenderer::save( QDomDocument& doc )
{
  QDomElement rendererElem = doc.createElement( RENDERER_TAG_NAME );

  rendererElem.setAttribute( "type", "25dRenderer" );
  rendererElem.setAttribute( "height", mHeight.expressionOrField() );
  rendererElem.setAttribute( "angle", mAngle );

  QDomElement symbolElem = QgsSymbolLayerV2Utils::saveSymbol( "symbol", mSymbol.data(), doc );

  rendererElem.appendChild( symbolElem );

  return rendererElem;
}

QgsFeatureRendererV2* Qgs25DRenderer::create( QDomElement& element )
{
  Qgs25DRenderer* renderer = new Qgs25DRenderer();
  renderer->mHeight.setField( element.attribute( "height" ) );
  renderer->mAngle = element.attribute( "angle", "45" ).toInt();

  return renderer;
}

void Qgs25DRenderer::startRender( QgsRenderContext& context, const QgsFields& fields )
{
  QgsExpressionContextScope* scope = new QgsExpressionContextScope( "2.5D Renderer" );
  scope->setVariable( "qgis_25d_height", mHeight.field() );
  scope->setVariable( "qgis_25d_angle", mAngle );
  context.expressionContext().appendScope( scope );
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
  c->mAngle = mAngle;
  c->mHeight = mHeight;
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

QgsDataDefined Qgs25DRenderer::height() const
{
  return mHeight;
}

void Qgs25DRenderer::setHeight( const QgsDataDefined& height )
{
  mHeight = height;
}

int Qgs25DRenderer::angle() const
{
  return mAngle;
}

void Qgs25DRenderer::setAngle( int angle )
{
  mAngle = angle;
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
  return static_cast<QgsOuterGlowEffect*>( mSymbol->symbolLayer( 0 )->paintEffect() );
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

