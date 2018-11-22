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

#include "qgssymbol.h"
#include "qgssymbollayerutils.h"

#include "qgslogger.h"
#include "qgsfeature.h"
#include "qgsvectorlayer.h"
#include "qgssymbollayer.h"
#include "qgsogcutils.h"
#include "qgspainteffect.h"
#include "qgspainteffectregistry.h"

#include <QDomDocument>
#include <QDomElement>

QgsInvertedPolygonRenderer::QgsInvertedPolygonRenderer( QgsFeatureRenderer *subRenderer )
  : QgsFeatureRenderer( QStringLiteral( "invertedPolygonRenderer" ) )
{
  if ( subRenderer )
  {
    setEmbeddedRenderer( subRenderer );
  }
  else
  {
    mSubRenderer.reset( QgsFeatureRenderer::defaultRenderer( QgsWkbTypes::PolygonGeometry ) );
  }
}

void QgsInvertedPolygonRenderer::setEmbeddedRenderer( QgsFeatureRenderer *subRenderer )
{
  if ( subRenderer )
  {
    mSubRenderer.reset( subRenderer );
  }
  else
  {
    mSubRenderer.reset( nullptr );
  }
}

const QgsFeatureRenderer *QgsInvertedPolygonRenderer::embeddedRenderer() const
{
  return mSubRenderer.get();
}

void QgsInvertedPolygonRenderer::setLegendSymbolItem( const QString &key, QgsSymbol *symbol )
{
  if ( !mSubRenderer )
    return;

  mSubRenderer->setLegendSymbolItem( key, symbol );
}

bool QgsInvertedPolygonRenderer::legendSymbolItemsCheckable() const
{
  if ( !mSubRenderer )
    return false;

  return mSubRenderer->legendSymbolItemsCheckable();
}

bool QgsInvertedPolygonRenderer::legendSymbolItemChecked( const QString &key )
{
  if ( !mSubRenderer )
    return false;

  return mSubRenderer->legendSymbolItemChecked( key );
}

void QgsInvertedPolygonRenderer::checkLegendSymbolItem( const QString &key, bool state )
{
  if ( !mSubRenderer )
    return;

  mSubRenderer->checkLegendSymbolItem( key, state );
}

void QgsInvertedPolygonRenderer::startRender( QgsRenderContext &context, const QgsFields &fields )
{
  QgsFeatureRenderer::startRender( context, fields );

  if ( !mSubRenderer )
  {
    return;
  }

  // first call start render on the sub renderer
  mSubRenderer->startRender( context, fields );

  mFeaturesCategories.clear();
  mSymbolCategories.clear();
  mFeatureDecorations.clear();
  mFields = fields;

  // We compute coordinates of the extent which will serve as exterior ring
  // for the final polygon
  // It must be computed in the destination CRS if reprojection is enabled.
  const QgsMapToPixel &mtp( context.mapToPixel() );

  if ( !context.painter() )
  {
    return;
  }

  // convert viewport to dest CRS
  QRect e( context.painter()->viewport() );
  // add some space to hide borders and tend to infinity
  e.adjust( -e.width() * 5, -e.height() * 5, e.width() * 5, e.height() * 5 );
  QgsPolylineXY exteriorRing;
  exteriorRing << mtp.toMapCoordinates( e.topLeft() );
  exteriorRing << mtp.toMapCoordinates( e.topRight() );
  exteriorRing << mtp.toMapCoordinates( e.bottomRight() );
  exteriorRing << mtp.toMapCoordinates( e.bottomLeft() );
  exteriorRing << mtp.toMapCoordinates( e.topLeft() );

  // copy the rendering context
  mContext = context;

  // If reprojection is enabled, we must reproject during renderFeature
  // and act as if there is no reprojection
  // If we don't do that, there is no need to have a simple rectangular extent
  // that covers the whole screen
  // (a rectangle in the destCRS cannot be expressed as valid coordinates in the sourceCRS in general)
  if ( context.coordinateTransform().isValid() )
  {
    // disable projection
    mContext.setCoordinateTransform( QgsCoordinateTransform() );
    // recompute extent so that polygon clipping is correct
    QRect v( context.painter()->viewport() );
    mContext.setExtent( QgsRectangle( mtp.toMapCoordinates( v.topLeft() ), mtp.toMapCoordinates( v.bottomRight() ) ) );
    // do we have to recompute the MapToPixel ?
  }

  mExtentPolygon.clear();
  mExtentPolygon.append( exteriorRing );
}

bool QgsInvertedPolygonRenderer::renderFeature( const QgsFeature &feature, QgsRenderContext &context, int layer, bool selected, bool drawVertexMarker )
{
  if ( !context.painter() )
  {
    return false;
  }

  // store this feature as a feature to render with decoration if needed
  if ( selected || drawVertexMarker )
  {
    mFeatureDecorations.append( FeatureDecoration( feature, selected, drawVertexMarker, layer ) );
  }

  // Features are grouped by category of symbols (returned by symbol(s)ForFeature)
  // This way, users can have multiple inverted polygon fills for a layer,
  // for instance, with rule based renderer and different symbols
  // that have transparency.
  //
  // In order to assign a unique category to a set of symbols
  // during each rendering session (between startRender() and stopRender()),
  // we build an unique id as a QByteArray that is the concatenation
  // of each symbol's memory address.
  // The only assumption made here is that symbol(s)ForFeature will
  // always return the same address for the same symbol(s) shared amongst
  // different features.
  // This QByteArray can then be used as a key for a QMap where the list of
  // features for this category is stored
  QByteArray catId;
  if ( capabilities() & MoreSymbolsPerFeature )
  {
    QgsSymbolList syms( mSubRenderer->symbolsForFeature( feature, context ) );
    Q_FOREACH ( QgsSymbol *sym, syms )
    {
      // append the memory address
      catId.append( reinterpret_cast<const char *>( &sym ), sizeof( sym ) );
    }
  }
  else
  {
    QgsSymbol *sym = mSubRenderer->symbolForFeature( feature, context );
    if ( sym )
    {
      catId.append( reinterpret_cast<const char *>( &sym ), sizeof( sym ) );
    }
  }

  if ( catId.isEmpty() )
  {
    return false;
  }

  if ( ! mSymbolCategories.contains( catId ) )
  {
    CombinedFeature cFeat;
    // store the first feature
    cFeat.feature = feature;
    mSymbolCategories.insert( catId, mSymbolCategories.count() );
    mFeaturesCategories.append( cFeat );
  }

  // update the geometry
  CombinedFeature &cFeat = mFeaturesCategories[ mSymbolCategories[catId] ];
  if ( !feature.hasGeometry() )
  {
    return false;
  }
  QgsGeometry geom = feature.geometry();

  QgsCoordinateTransform xform = context.coordinateTransform();
  if ( xform.isValid() )
  {
    geom.transform( xform );
  }

  if ( mPreprocessingEnabled )
  {
    // fix the polygon if it is not valid
    if ( ! geom.isGeosValid() )
    {
      geom = geom.buffer( 0, 0 );
    }
  }

  if ( geom.isNull() )
    return false; // do not let invalid geometries sneak in!

  // add the geometry to the list of geometries for this feature
  cFeat.geometries.append( geom );

  return true;
}

void QgsInvertedPolygonRenderer::stopRender( QgsRenderContext &context )
{
  QgsFeatureRenderer::stopRender( context );

  if ( !mSubRenderer )
  {
    return;
  }
  if ( !context.painter() )
  {
    return;
  }

  QgsMultiPolygonXY finalMulti; //avoid expensive allocation for list for every feature
  QgsPolygonXY newPoly;

  Q_FOREACH ( const CombinedFeature &cit, mFeaturesCategories )
  {
    finalMulti.resize( 0 ); //preserve capacity - don't use clear!
    QgsFeature feat = cit.feature; // just a copy, so that we do not accumulate geometries again
    if ( mPreprocessingEnabled )
    {
      // compute the unary union on the polygons
      QgsGeometry unioned( QgsGeometry::unaryUnion( cit.geometries ) );
      // compute the difference with the extent
      QgsGeometry rect = QgsGeometry::fromPolygonXY( mExtentPolygon );
      QgsGeometry final = rect.difference( unioned );
      feat.setGeometry( final );
    }
    else
    {
      // No preprocessing involved.
      // We build here a "reversed" geometry of all the polygons
      //
      // The final geometry is a multipolygon F, with :
      // * the first polygon of F having the current extent as its exterior ring
      // * each polygon's exterior ring is added as interior ring of the first polygon of F
      // * each polygon's interior ring is added as new polygons in F
      //
      // No validity check is done, on purpose, it will be very slow and painting
      // operations do not need geometries to be valid

      finalMulti.append( mExtentPolygon );
      Q_FOREACH ( const QgsGeometry &geom, cit.geometries )
      {
        QgsMultiPolygonXY multi;
        QgsWkbTypes::Type type = QgsWkbTypes::flatType( geom.constGet()->wkbType() );

        if ( ( type == QgsWkbTypes::Polygon ) || ( type == QgsWkbTypes::CurvePolygon ) )
        {
          multi.append( geom.asPolygon() );
        }
        else if ( ( type == QgsWkbTypes::MultiPolygon ) || ( type == QgsWkbTypes::MultiSurface ) )
        {
          multi = geom.asMultiPolygon();
        }

        for ( int i = 0; i < multi.size(); i++ )
        {
          const QgsPolylineXY &exterior = multi[i][0];
          // add the exterior ring as interior ring to the first polygon
          // make sure it satisfies at least very basic requirements of GEOS
          // (otherwise the creation of GEOS geometry will fail)
          if ( exterior.count() < 4 || exterior[0] != exterior[exterior.count() - 1] )
            continue;
          finalMulti[0].append( exterior );

          // add interior rings as new polygons
          for ( int j = 1; j < multi[i].size(); j++ )
          {
            newPoly.resize( 0 ); //preserve capacity - don't use clear!
            newPoly.append( multi[i][j] );
            finalMulti.append( newPoly );
          }
        }
      }
      feat.setGeometry( QgsGeometry::fromMultiPolygonXY( finalMulti ) );
    }
    if ( feat.hasGeometry() )
    {
      mContext.expressionContext().setFeature( feat );
      mSubRenderer->renderFeature( feat, mContext );
    }
  }

  // when no features are visible, we still have to draw the exterior rectangle
  // warning: when sub renderers have more than one possible symbols,
  // there is no way to choose a correct one, because there is no attribute here
  // in that case, nothing will be rendered
  if ( mFeaturesCategories.isEmpty() )
  {
    // empty feature with default attributes
    QgsFeature feat( mFields );
    feat.setGeometry( QgsGeometry::fromPolygonXY( mExtentPolygon ) );
    mSubRenderer->renderFeature( feat, mContext );
  }

  // draw feature decorations
  Q_FOREACH ( FeatureDecoration deco, mFeatureDecorations )
  {
    mSubRenderer->renderFeature( deco.feature, mContext, deco.layer, deco.selected, deco.drawMarkers );
  }

  mSubRenderer->stopRender( mContext );
}

QString QgsInvertedPolygonRenderer::dump() const
{
  if ( !mSubRenderer )
  {
    return QStringLiteral( "INVERTED: NULL" );
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

QgsFeatureRenderer *QgsInvertedPolygonRenderer::create( QDomElement &element, const QgsReadWriteContext &context )
{
  QgsInvertedPolygonRenderer *r = new QgsInvertedPolygonRenderer();
  //look for an embedded renderer <renderer-v2>
  QDomElement embeddedRendererElem = element.firstChildElement( QStringLiteral( "renderer-v2" ) );
  if ( !embeddedRendererElem.isNull() )
  {
    QgsFeatureRenderer *renderer = QgsFeatureRenderer::load( embeddedRendererElem, context );
    r->setEmbeddedRenderer( renderer );
  }
  r->setPreprocessingEnabled( element.attribute( QStringLiteral( "preprocessing" ), QStringLiteral( "0" ) ).toInt() == 1 );
  return r;
}

QDomElement QgsInvertedPolygonRenderer::save( QDomDocument &doc, const QgsReadWriteContext &context )
{
  // clazy:skip

  QDomElement rendererElem = doc.createElement( RENDERER_TAG_NAME );
  rendererElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "invertedPolygonRenderer" ) );
  rendererElem.setAttribute( QStringLiteral( "preprocessing" ), preprocessingEnabled() ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  rendererElem.setAttribute( QStringLiteral( "forceraster" ), ( mForceRaster ? QStringLiteral( "1" ) : QStringLiteral( "0" ) ) );

  if ( mSubRenderer )
  {
    QDomElement embeddedRendererElem = mSubRenderer->save( doc, context );
    rendererElem.appendChild( embeddedRendererElem );
  }

  if ( mPaintEffect && !QgsPaintEffectRegistry::isDefaultStack( mPaintEffect ) )
    mPaintEffect->saveProperties( doc, rendererElem );

  if ( !mOrderBy.isEmpty() )
  {
    QDomElement orderBy = doc.createElement( QStringLiteral( "orderby" ) );
    mOrderBy.save( orderBy );
    rendererElem.appendChild( orderBy );
  }
  rendererElem.setAttribute( QStringLiteral( "enableorderby" ), ( mOrderByEnabled ? QStringLiteral( "1" ) : QStringLiteral( "0" ) ) );

  return rendererElem;
}

QgsSymbol *QgsInvertedPolygonRenderer::symbolForFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  if ( !mSubRenderer )
  {
    return nullptr;
  }
  return mSubRenderer->symbolForFeature( feature, context );
}

QgsSymbol *QgsInvertedPolygonRenderer::originalSymbolForFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  if ( !mSubRenderer )
    return nullptr;
  return mSubRenderer->originalSymbolForFeature( feature, context );
}

QgsSymbolList QgsInvertedPolygonRenderer::symbolsForFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  if ( !mSubRenderer )
  {
    return QgsSymbolList();
  }
  return mSubRenderer->symbolsForFeature( feature, context );
}

QgsSymbolList QgsInvertedPolygonRenderer::originalSymbolsForFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  if ( !mSubRenderer )
    return QgsSymbolList();
  return mSubRenderer->originalSymbolsForFeature( feature, context );
}

QgsSymbolList QgsInvertedPolygonRenderer::symbols( QgsRenderContext &context ) const
{
  if ( !mSubRenderer )
  {
    return QgsSymbolList();
  }
  return mSubRenderer->symbols( context );
}

QgsFeatureRenderer::Capabilities QgsInvertedPolygonRenderer::capabilities()
{
  if ( !mSubRenderer )
  {
    return nullptr;
  }
  return mSubRenderer->capabilities();
}

QSet<QString> QgsInvertedPolygonRenderer::usedAttributes( const QgsRenderContext &context ) const
{
  if ( !mSubRenderer )
  {
    return QSet<QString>();
  }
  return mSubRenderer->usedAttributes( context );
}

bool QgsInvertedPolygonRenderer::filterNeedsGeometry() const
{
  return mSubRenderer ? mSubRenderer->filterNeedsGeometry() : false;
}

QgsLegendSymbolList QgsInvertedPolygonRenderer::legendSymbolItems() const
{
  if ( !mSubRenderer )
  {
    return QgsLegendSymbolList();
  }
  return mSubRenderer->legendSymbolItems();
}

bool QgsInvertedPolygonRenderer::willRenderFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  if ( !mSubRenderer )
  {
    return false;
  }
  return mSubRenderer->willRenderFeature( feature, context );
}

QgsInvertedPolygonRenderer *QgsInvertedPolygonRenderer::convertFromRenderer( const QgsFeatureRenderer *renderer )
{
  if ( renderer->type() == QLatin1String( "invertedPolygonRenderer" ) )
  {
    return dynamic_cast<QgsInvertedPolygonRenderer *>( renderer->clone() );
  }

  if ( renderer->type() == QLatin1String( "singleSymbol" ) ||
       renderer->type() == QLatin1String( "categorizedSymbol" ) ||
       renderer->type() == QLatin1String( "graduatedSymbol" ) ||
       renderer->type() == QLatin1String( "RuleRenderer" ) )
  {
    return new QgsInvertedPolygonRenderer( renderer->clone() );
  }
  return nullptr;
}

