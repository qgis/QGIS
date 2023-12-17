/***************************************************************************
                         qgstiledscenetexturerenderer.h
                         --------------------
    begin                : August 2023
    copyright            : (C) 2023 by Nyall Dawson
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

#include "qgstiledscenetexturerenderer.h"
#include "qgspainting.h"
#include "qgsfillsymbol.h"
#include "qgssymbollayerutils.h"

QgsTiledSceneTextureRenderer::QgsTiledSceneTextureRenderer()
{
  mFillSymbol.reset( createDefaultFillSymbol() );
}

QgsTiledSceneTextureRenderer::~QgsTiledSceneTextureRenderer() = default;

QString QgsTiledSceneTextureRenderer::type() const
{
  return QStringLiteral( "texture" );
}

QgsTiledSceneRenderer *QgsTiledSceneTextureRenderer::clone() const
{
  std::unique_ptr< QgsTiledSceneTextureRenderer > res = std::make_unique< QgsTiledSceneTextureRenderer >();
  res->setFillSymbol( mFillSymbol->clone() );

  copyCommonProperties( res.get() );

  return res.release();
}

QgsTiledSceneRenderer *QgsTiledSceneTextureRenderer::create( QDomElement &element, const QgsReadWriteContext &context )
{
  std::unique_ptr< QgsTiledSceneTextureRenderer > r = std::make_unique< QgsTiledSceneTextureRenderer >();
  {
    const QDomElement fillSymbolElem = element.firstChildElement( QStringLiteral( "fillSymbol" ) );
    if ( !fillSymbolElem.isNull() )
    {
      const QDomElement symbolElem = fillSymbolElem.firstChildElement( QStringLiteral( "symbol" ) );
      std::unique_ptr< QgsFillSymbol > fillSymbol( QgsSymbolLayerUtils::loadSymbol<QgsFillSymbol>( symbolElem, context ) );
      if ( fillSymbol )
        r->mFillSymbol = std::move( fillSymbol );
    }
  }

  r->restoreCommonProperties( element, context );

  return r.release();
}

QgsFillSymbol *QgsTiledSceneTextureRenderer::createDefaultFillSymbol()
{
  QVariantMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "224,224,224" ) );
  properties.insert( QStringLiteral( "style" ), QStringLiteral( "solid" ) );
  properties.insert( QStringLiteral( "style_border" ), QStringLiteral( "solid" ) );
  properties.insert( QStringLiteral( "color_border" ), QStringLiteral( "124,124,124" ) );
  properties.insert( QStringLiteral( "width_border" ), QStringLiteral( "0.1" ) );
  properties.insert( QStringLiteral( "joinstyle" ), QStringLiteral( "round" ) );

  return QgsFillSymbol::createSimple( properties );
}

QgsFillSymbol *QgsTiledSceneTextureRenderer::fillSymbol() const
{
  return mFillSymbol.get();
}

void QgsTiledSceneTextureRenderer::setFillSymbol( QgsFillSymbol *symbol )
{
  mFillSymbol.reset( symbol );
}

QDomElement QgsTiledSceneTextureRenderer::save( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement rendererElem = doc.createElement( QStringLiteral( "renderer" ) );

  rendererElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "texture" ) );

  {
    QDomElement fillSymbolElem = doc.createElement( QStringLiteral( "fillSymbol" ) );
    const QDomElement symbolElement = QgsSymbolLayerUtils::saveSymbol( QString(),
                                      mFillSymbol.get(),
                                      doc,
                                      context );
    fillSymbolElem.appendChild( symbolElement );
    rendererElem.appendChild( fillSymbolElem );
  }

  saveCommonProperties( rendererElem, context );

  return rendererElem;
}

Qgis::TiledSceneRendererFlags QgsTiledSceneTextureRenderer::flags() const
{
  // force raster rendering for this renderer type -- there's no benefit in exporting these layers as a bunch
  // of triangular images which are pieced together, that adds a lot of extra content to the exports and results
  // in files which can be extremely slow to open and render in other viewers.
  return Qgis::TiledSceneRendererFlag::RequiresTextures |
         Qgis::TiledSceneRendererFlag::ForceRasterRender |
         Qgis::TiledSceneRendererFlag::RendersTriangles;
}

void QgsTiledSceneTextureRenderer::renderTriangle( QgsTiledSceneRenderContext &context, const QPolygonF &triangle )
{
  if ( context.textureImage().isNull() )
  {
    mFillSymbol->renderPolygon( triangle, nullptr, nullptr, context.renderContext() );
    return;
  }

  float textureX1;
  float textureY1;
  float textureX2;
  float textureY2;
  float textureX3;
  float textureY3;
  context.textureCoordinates( textureX1, textureY1, textureX2, textureY2, textureX3, textureY3 );

  QPainter *painter = context.renderContext().painter();
  painter->setPen( Qt::NoPen );

  auto unitNormal = []( const QPointF p1, const QPointF p2 )
  {
    const float dx = p2.x() - p1.x();
    const float dy = p2.y() - p1.y();
    QPointF n( -dy, dx );
    const double length = std::sqrt( n.x() * n.x() + n.y() * n.y() );
    return QPointF( n.x() / length, n.y() / length );
  };

  auto intersect = []( const QPointF p1, const QPointF p2, const QPointF q1, const QPointF q2 )
  {
    const double a1 = p2.y() - p1.y();
    const double b1 = p1.x() - p2.x();
    const double c1 = a1 * p1.x() + b1 * p1.y();

    const double a2 = q2.y() - q1.y();
    const double b2 = q1.x() - q2.x();
    const double c2 = a2 * q1.x() + b2 * q1.y();

    const double det = a1 * b2 - a2 * b1;

    if ( qgsDoubleNear( det, 0 ) )
    {
      return QPointF();
    }
    else
    {
      return QPointF( ( b2 * c1 - b1 * c2 ) / det,
                      ( a1 * c2 - a2 * c1 ) / det );
    }
  };

  auto smallestAngleInTriangle = []( const QPolygonF & triangle )
  {
    const QPointF p1 = triangle.at( 0 );
    const QPointF p2 = triangle.at( 1 );
    const QPointF p3 = triangle.at( 2 );

    const QPointF v1 = p2 - p1;
    const QPointF v2 = p3 - p2;
    const QPointF v3 = p1 - p3;

    const double a = std::sqrt( v1.x() * v1.x() + v1.y() * v1.y() );
    const double b = std::sqrt( v2.x() * v2.x() + v2.y() * v2.y() );
    const double c = std::sqrt( v3.x() * v3.x() + v3.y() * v3.y() );

    return std::min(
             std::min(
               std::acos( ( b * b + c * c - a * a )  / ( 2 * b * c ) ),
               std::acos( ( a * a + c * c - b * b )  / ( 2 * a * c ) ) ),
             std::acos( ( a * a + b * b - c * c )  / ( 2 * a * b ) )
           );
  };

  auto growTriangle = [&unitNormal, &intersect]( const QPolygonF & triangle, float pixels )
  {
    QPair< QPointF, QPointF > offsetEdges[3];
    for ( int i = 0; i < 3; ++i )
    {
      const QPointF p1 = triangle.at( i );
      const QPointF p2 = triangle.at( i + 1 );
      const QPointF n = unitNormal( p1, p2 );

      const QPointF offsetP1( p1.x() + n.x() * pixels, p1.y() + n.y() * pixels );
      const QPointF offsetP2( p2.x() + n.x() * pixels, p2.y() + n.y() * pixels );

      offsetEdges[i] = { offsetP1, offsetP2 };
    }

    QPolygonF result;
    result.reserve( 4 );
    // limit triangle vertices to shifting AT MOST 2 pixels from their original locations, to avoid narrow triangles
    // getting buffered too large
    static double constexpr MAX_TRIANGLE_GROW_PIXELS_SQUARED = 3 * 3;
    for ( int i = 0; i < 3; ++i )
    {
      const auto &edge1 = offsetEdges[i];
      const auto &edge2 = offsetEdges[i == 0 ? 2 : ( i - 1 )];

      const QPointF vertex = intersect( edge1.first, edge1.second, edge2.first, edge2.second );
      if ( vertex.isNull() )
        return triangle;

      const QPointF originalPoint = triangle.at( i );

      // don't allow triangle to grow too large
      double delta = std::pow( vertex.x() - originalPoint.x(), 2 ) + std::pow( vertex.y() - originalPoint.y(), 2 );
      if ( delta > MAX_TRIANGLE_GROW_PIXELS_SQUARED )
      {
        double dx = ( vertex.x() - originalPoint.x() ) * MAX_TRIANGLE_GROW_PIXELS_SQUARED / delta;
        double dy = ( vertex.y() - originalPoint.y() ) * MAX_TRIANGLE_GROW_PIXELS_SQUARED / delta;
        result << triangle.at( i ) + QPointF( dx, dy );
      }
      else
        result << vertex;
    }
    result << result.at( 0 );
    return result;
  };

  // buffer the triangles out slightly to reduce artifacts caused by antialiasing,
  // but try to avoid new artifacts caused by buffering narrow triangles
  const double minAngle = smallestAngleInTriangle( triangle ) * 180 / M_PI;
  if ( std::isnan( minAngle ) || minAngle < 0.1 )
  {
    // don't try to draw slivers
    return;
  }

  QgsPainting::drawTriangleUsingTexture(
    painter,
    growTriangle( triangle, 1 ),
    context.textureImage(),
    textureX1, textureY1,
    textureX2, textureY2,
    textureX3, textureY3
  );
}

void QgsTiledSceneTextureRenderer::renderLine( QgsTiledSceneRenderContext &, const QPolygonF & )
{

}

void QgsTiledSceneTextureRenderer::startRender( QgsTiledSceneRenderContext &context )
{
  QgsTiledSceneRenderer::startRender( context );
  mFillSymbol->startRender( context.renderContext() );
}

void QgsTiledSceneTextureRenderer::stopRender( QgsTiledSceneRenderContext &context )
{
  mFillSymbol->stopRender( context.renderContext() );

  QgsTiledSceneRenderer::stopRender( context );
}
