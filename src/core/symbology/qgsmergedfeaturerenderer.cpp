/***************************************************************************
    qgsmergedfeaturerenderer.h
    ---------------------
    begin                : December 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmergedfeaturerenderer.h"

#include "qgssymbol.h"
#include "qgssymbollayerutils.h"

#include "qgslogger.h"
#include "qgsfeature.h"
#include "qgsvectorlayer.h"
#include "qgssymbollayer.h"
#include "qgsogcutils.h"
#include "qgspainteffect.h"
#include "qgspainteffectregistry.h"
#include "qgsstyleentityvisitor.h"

#include <QDomDocument>
#include <QDomElement>

QgsMergedFeatureRenderer::QgsMergedFeatureRenderer( QgsFeatureRenderer *subRenderer )
  : QgsMergedFeatureRenderer( QStringLiteral( "mergedFeatureRenderer" ), subRenderer )
{

}

QgsMergedFeatureRenderer::QgsMergedFeatureRenderer( const QString &type, QgsFeatureRenderer *subRenderer )
  : QgsFeatureRenderer( type )
{
  if ( subRenderer )
  {
    mSubRenderer.reset( subRenderer );
  }
}

void QgsMergedFeatureRenderer::setEmbeddedRenderer( QgsFeatureRenderer *subRenderer )
{
  mSubRenderer.reset( subRenderer );
}

const QgsFeatureRenderer *QgsMergedFeatureRenderer::embeddedRenderer() const
{
  return mSubRenderer.get();
}

void QgsMergedFeatureRenderer::setLegendSymbolItem( const QString &key, QgsSymbol *symbol )
{
  if ( !mSubRenderer )
    return;

  mSubRenderer->setLegendSymbolItem( key, symbol );
}

bool QgsMergedFeatureRenderer::legendSymbolItemsCheckable() const
{
  if ( !mSubRenderer )
    return false;

  return mSubRenderer->legendSymbolItemsCheckable();
}

bool QgsMergedFeatureRenderer::legendSymbolItemChecked( const QString &key )
{
  if ( !mSubRenderer )
    return false;

  return mSubRenderer->legendSymbolItemChecked( key );
}

void QgsMergedFeatureRenderer::checkLegendSymbolItem( const QString &key, bool state )
{
  if ( !mSubRenderer )
    return;

  mSubRenderer->checkLegendSymbolItem( key, state );
}

bool QgsMergedFeatureRenderer::accept( QgsStyleEntityVisitorInterface *visitor ) const
{
  if ( !mSubRenderer )
    return true;

  return mSubRenderer->accept( visitor );
}

void QgsMergedFeatureRenderer::startRender( QgsRenderContext &context, const QgsFields &fields )
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

  if ( !context.painter() )
  {
    return;
  }

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
    mContext.setExtent( context.mapExtent() );
    // do we have to recompute the MapToPixel ?
  }

  switch ( mOperation )
  {
    case InvertOnly:
    case MergeAndInvert:
    {
      // convert viewport to dest CRS
      // add some space to hide borders and tend to infinity
      const double buffer = std::max( context.mapExtent().width(), context.mapExtent().height() ) * 0.1;
      const QRectF outer = context.mapExtent().buffered( buffer ).toRectF();
      QgsPolylineXY exteriorRing;
      exteriorRing.reserve( 5 );
      exteriorRing << outer.topLeft();
      exteriorRing << outer.topRight();
      exteriorRing << outer.bottomRight();
      exteriorRing << outer.bottomLeft();
      exteriorRing << outer.topLeft();

      mExtentPolygon.clear();
      mExtentPolygon.append( exteriorRing );
      break;
    }

    case Merge:
      break;
  }
}

bool QgsMergedFeatureRenderer::renderFeature( const QgsFeature &feature, QgsRenderContext &context, int layer, bool selected, bool drawVertexMarker )
{
  if ( !context.painter() || !mSubRenderer )
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
    const QgsSymbolList syms( mSubRenderer->symbolsForFeature( feature, context ) );
    for ( QgsSymbol *sym : syms )
    {
      // append the memory address
      catId.append( reinterpret_cast<const char *>( &sym ), sizeof( sym ) );
    }
  }
  else
  {
    if ( QgsSymbol *sym = mSubRenderer->symbolForFeature( feature, context ) )
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

  switch ( mOperation )
  {
    case QgsMergedFeatureRenderer::MergeAndInvert:
      // fix the polygon if it is not valid
      if ( ! geom.isGeosValid() )
      {
        geom = geom.buffer( 0, 0 );
      }
      break;

    case QgsMergedFeatureRenderer::InvertOnly:
    case QgsMergedFeatureRenderer::Merge: // maybe we should also fix for this? not sure if the fixing step was only required for the differencing operation...
      break;
  }

  if ( geom.isNull() )
    return false; // do not let invalid geometries sneak in!

  // add the geometry to the list of geometries for this feature
  cFeat.geometries.append( geom );

  return true;
}

void QgsMergedFeatureRenderer::stopRender( QgsRenderContext &context )
{
  QgsFeatureRenderer::stopRender( context );
  if ( context.renderingStopped() )
  {
    if ( mSubRenderer )
      mSubRenderer->stopRender( mContext );
    return;
  }

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

  for ( const CombinedFeature &cit : std::as_const( mFeaturesCategories ) )
  {
    finalMulti.resize( 0 ); //preserve capacity - don't use clear!
    QgsFeature feat = cit.feature; // just a copy, so that we do not accumulate geometries again

    switch ( mOperation )
    {
      case QgsMergedFeatureRenderer::Merge:
      {
        QgsGeometry unioned( QgsGeometry::unaryUnion( cit.geometries ) );
        if ( unioned.type() == QgsWkbTypes::LineGeometry )
          unioned = unioned.mergeLines();
        feat.setGeometry( unioned );
        break;
      }

      case QgsMergedFeatureRenderer::MergeAndInvert:
      {
        // compute the unary union on the polygons
        const QgsGeometry unioned( QgsGeometry::unaryUnion( cit.geometries ) );
        // compute the difference with the extent
        const QgsGeometry rect = QgsGeometry::fromPolygonXY( mExtentPolygon );
        const QgsGeometry final = rect.difference( unioned );
        feat.setGeometry( final );
        break;
      }

      case QgsMergedFeatureRenderer::InvertOnly:
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
        for ( const QgsGeometry &geom : std::as_const( cit.geometries ) )
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
        break;
      }
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
  switch ( mOperation )
  {
    case QgsMergedFeatureRenderer::Merge:
      break;
    case QgsMergedFeatureRenderer::InvertOnly:
    case QgsMergedFeatureRenderer::MergeAndInvert:
      if ( mFeaturesCategories.isEmpty() )
      {
        // empty feature with default attributes
        QgsFeature feat( mFields );
        feat.setGeometry( QgsGeometry::fromPolygonXY( mExtentPolygon ) );
        mSubRenderer->renderFeature( feat, mContext );
      }
      break;
  }

  // draw feature decorations
  for ( FeatureDecoration deco : std::as_const( mFeatureDecorations ) )
  {
    mSubRenderer->renderFeature( deco.feature, mContext, deco.layer, deco.selected, deco.drawMarkers );
  }

  mSubRenderer->stopRender( mContext );
}

QString QgsMergedFeatureRenderer::dump() const
{
  if ( !mSubRenderer )
  {
    return QStringLiteral( "MERGED FEATURES: NULL" );
  }
  return "MERGED FEATURES [" + mSubRenderer->dump() + ']';
}

QgsMergedFeatureRenderer *QgsMergedFeatureRenderer::clone() const
{
  QgsMergedFeatureRenderer *newRenderer = nullptr;
  if ( !mSubRenderer )
  {
    newRenderer = new QgsMergedFeatureRenderer( nullptr );
  }
  else
  {
    newRenderer = new QgsMergedFeatureRenderer( mSubRenderer->clone() );
  }
  copyRendererData( newRenderer );
  return newRenderer;
}

QgsFeatureRenderer *QgsMergedFeatureRenderer::create( QDomElement &element, const QgsReadWriteContext &context )
{
  QgsMergedFeatureRenderer *r = new QgsMergedFeatureRenderer( nullptr );
  //look for an embedded renderer <renderer-v2>
  QDomElement embeddedRendererElem = element.firstChildElement( QStringLiteral( "renderer-v2" ) );
  if ( !embeddedRendererElem.isNull() )
  {
    QgsFeatureRenderer *renderer = QgsFeatureRenderer::load( embeddedRendererElem, context );
    r->setEmbeddedRenderer( renderer );
  }
  return r;
}

QDomElement QgsMergedFeatureRenderer::save( QDomDocument &doc, const QgsReadWriteContext &context )
{
  // clazy:skip

  QDomElement rendererElem = doc.createElement( RENDERER_TAG_NAME );
  rendererElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "mergedFeatureRenderer" ) );

  if ( mSubRenderer )
  {
    QDomElement embeddedRendererElem = mSubRenderer->save( doc, context );
    rendererElem.appendChild( embeddedRendererElem );
  }

  saveRendererData( doc, rendererElem, context );

  return rendererElem;
}

QgsSymbol *QgsMergedFeatureRenderer::symbolForFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  if ( !mSubRenderer )
  {
    return nullptr;
  }
  return mSubRenderer->symbolForFeature( feature, context );
}

QgsSymbol *QgsMergedFeatureRenderer::originalSymbolForFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  if ( !mSubRenderer )
    return nullptr;
  return mSubRenderer->originalSymbolForFeature( feature, context );
}

QgsSymbolList QgsMergedFeatureRenderer::symbolsForFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  if ( !mSubRenderer )
  {
    return QgsSymbolList();
  }
  return mSubRenderer->symbolsForFeature( feature, context );
}

QgsSymbolList QgsMergedFeatureRenderer::originalSymbolsForFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  if ( !mSubRenderer )
    return QgsSymbolList();
  return mSubRenderer->originalSymbolsForFeature( feature, context );
}

QSet<QString> QgsMergedFeatureRenderer::legendKeysForFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  if ( !mSubRenderer )
    return QSet<QString>();
  return mSubRenderer->legendKeysForFeature( feature, context );
}

QString QgsMergedFeatureRenderer::legendKeyToExpression( const QString &key, QgsVectorLayer *layer, bool &ok ) const
{
  ok = false;
  if ( !mSubRenderer )
    return QString();
  return mSubRenderer->legendKeyToExpression( key, layer, ok );
}

QgsSymbolList QgsMergedFeatureRenderer::symbols( QgsRenderContext &context ) const
{
  if ( !mSubRenderer )
  {
    return QgsSymbolList();
  }
  return mSubRenderer->symbols( context );
}

QgsFeatureRenderer::Capabilities QgsMergedFeatureRenderer::capabilities()
{
  if ( !mSubRenderer )
  {
    return Capabilities();
  }
  return mSubRenderer->capabilities();
}

QSet<QString> QgsMergedFeatureRenderer::usedAttributes( const QgsRenderContext &context ) const
{
  if ( !mSubRenderer )
  {
    return QSet<QString>();
  }
  return mSubRenderer->usedAttributes( context );
}

bool QgsMergedFeatureRenderer::filterNeedsGeometry() const
{
  return mSubRenderer ? mSubRenderer->filterNeedsGeometry() : false;
}

QgsLegendSymbolList QgsMergedFeatureRenderer::legendSymbolItems() const
{
  if ( !mSubRenderer )
  {
    return QgsLegendSymbolList();
  }
  return mSubRenderer->legendSymbolItems();
}

bool QgsMergedFeatureRenderer::willRenderFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  if ( !mSubRenderer )
  {
    return false;
  }
  return mSubRenderer->willRenderFeature( feature, context );
}

QgsMergedFeatureRenderer *QgsMergedFeatureRenderer::convertFromRenderer( const QgsFeatureRenderer *renderer )
{
  if ( renderer->type() == QLatin1String( "mergedFeatureRenderer" ) )
  {
    return dynamic_cast<QgsMergedFeatureRenderer *>( renderer->clone() );
  }

  if ( renderer->type() == QLatin1String( "singleSymbol" ) ||
       renderer->type() == QLatin1String( "categorizedSymbol" ) ||
       renderer->type() == QLatin1String( "graduatedSymbol" ) ||
       renderer->type() == QLatin1String( "RuleRenderer" ) )
  {
    std::unique_ptr< QgsMergedFeatureRenderer > res = std::make_unique< QgsMergedFeatureRenderer >( renderer->clone() );
    renderer->copyRendererData( res.get() );
    return res.release();
  }
  else if ( renderer->type() == QLatin1String( "invertedPolygonRenderer" ) )
  {
    std::unique_ptr< QgsMergedFeatureRenderer > res = std::make_unique< QgsMergedFeatureRenderer >( renderer->embeddedRenderer() ? renderer->embeddedRenderer()->clone() : nullptr );
    renderer->copyRendererData( res.get() );
    return res.release();
  }
  return nullptr;
}

