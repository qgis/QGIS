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

#include "qgssymbolv2.h"
#include "qgssymbollayerv2utils.h"

#include "qgslogger.h"
#include "qgsfeature.h"
#include "qgsvectorlayer.h"
#include "qgssymbollayerv2.h"
#include "qgsogcutils.h"

#include <QDomDocument>
#include <QDomElement>

QgsInvertedPolygonRenderer::QgsInvertedPolygonRenderer( const QgsFeatureRendererV2* subRenderer )
    : QgsFeatureRendererV2( "invertedPolygonRenderer" )
    , mPreprocessingEnabled( false )
{
  if ( subRenderer )
  {
    setEmbeddedRenderer( subRenderer );
  }
  else
  {
    mSubRenderer.reset( QgsFeatureRendererV2::defaultRenderer( QGis::Polygon ) );
  }
}

QgsInvertedPolygonRenderer::~QgsInvertedPolygonRenderer()
{
}

void QgsInvertedPolygonRenderer::setEmbeddedRenderer( const QgsFeatureRendererV2* subRenderer )
{
  if ( subRenderer )
  {
    mSubRenderer.reset( const_cast<QgsFeatureRendererV2*>( subRenderer )->clone() );
  }
  else
  {
    mSubRenderer.reset( 0 );
  }
}

const QgsFeatureRendererV2* QgsInvertedPolygonRenderer::embeddedRenderer() const
{
  return mSubRenderer.data();
}

void QgsInvertedPolygonRenderer::startRender( QgsRenderContext& context, const QgsFields& fields )
{
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
  const QgsMapToPixel& mtp( context.mapToPixel() );

  if ( !context.painter() )
  {
    return;
  }

  // convert viewport to dest CRS
  QRect e( context.painter()->viewport() );
  // add some space to hide borders and tend to infinity
  e.adjust( -e.width()*5, -e.height()*5, e.width()*5, e.height()*5 );
  QgsPolyline exteriorRing;
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
  if ( context.coordinateTransform() )
  {
    // disable projection
    mContext.setCoordinateTransform( 0 );
    // recompute extent so that polygon clipping is correct
    QRect v( context.painter()->viewport() );
    mContext.setExtent( QgsRectangle( mtp.toMapCoordinates( v.topLeft() ), mtp.toMapCoordinates( v.bottomRight() ) ) );
    // do we have to recompute the MapToPixel ?
  }

  mExtentPolygon.clear();
  mExtentPolygon.append( exteriorRing );
}

bool QgsInvertedPolygonRenderer::renderFeature( QgsFeature& feature, QgsRenderContext& context, int layer, bool selected, bool drawVertexMarker )
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
    QgsSymbolV2List syms( mSubRenderer->symbolsForFeature( feature ) );
    foreach ( QgsSymbolV2* sym, syms )
    {
      // append the memory address
      catId.append( reinterpret_cast<const char*>( &sym ), sizeof( sym ) );
    }
  }
  else
  {
    QgsSymbolV2* sym = mSubRenderer->symbolForFeature( feature );
    if ( sym )
    {
      catId.append( reinterpret_cast<const char*>( &sym ), sizeof( sym ) );
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
  CombinedFeature& cFeat = mFeaturesCategories[ mSymbolCategories[catId] ];
  if ( !feature.geometry() )
  {
    return false;
  }
  QScopedPointer<QgsGeometry> geom( new QgsGeometry( *feature.geometry() ) );

  const QgsCoordinateTransform* xform = context.coordinateTransform();
  if ( xform )
  {
    geom->transform( *xform );
  }

  if ( mPreprocessingEnabled )
  {
    // fix the polygon if it is not valid
    if ( ! geom->isGeosValid() )
    {
      geom.reset( geom->buffer( 0, 0 ) );
    }
  }

  if ( !geom )
    return false; // do not let invalid geometries sneak in!

  // add the geometry to the list of geometries for this feature
  cFeat.geometries.append( geom.take() );

  return true;
}

void QgsInvertedPolygonRenderer::stopRender( QgsRenderContext& context )
{
  if ( !mSubRenderer )
  {
    return;
  }
  if ( !context.painter() )
  {
    return;
  }

  for ( FeatureCategoryVector::iterator cit = mFeaturesCategories.begin(); cit != mFeaturesCategories.end(); ++cit )
  {
    QgsFeature feat = cit->feature; // just a copy, so that we do not accumulate geometries again
    if ( mPreprocessingEnabled )
    {
      // compute the unary union on the polygons
      QScopedPointer<QgsGeometry> unioned( QgsGeometry::unaryUnion( cit->geometries ) );
      // compute the difference with the extent
      QScopedPointer<QgsGeometry> rect( QgsGeometry::fromPolygon( mExtentPolygon ) );
      QgsGeometry *final = rect->difference( const_cast<QgsGeometry*>( unioned.data() ) );
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
      QgsMultiPolygon finalMulti;
      finalMulti.append( mExtentPolygon );
      foreach ( QgsGeometry* geom, cit->geometries )
      {
        QgsMultiPolygon multi;
        if (( geom->wkbType() == QGis::WKBPolygon ) ||
            ( geom->wkbType() == QGis::WKBPolygon25D ) )
        {
          multi.append( geom->asPolygon() );
        }
        else if (( geom->wkbType() == QGis::WKBMultiPolygon ) ||
                 ( geom->wkbType() == QGis::WKBMultiPolygon25D ) )
        {
          multi = geom->asMultiPolygon();
        }

        for ( int i = 0; i < multi.size(); i++ )
        {
          const QgsPolyline& exterior = multi[i][0];
          // add the exterior ring as interior ring to the first polygon
          // make sure it satisfies at least very basic requirements of GEOS
          // (otherwise the creation of GEOS geometry will fail)
          if ( exterior.count() < 4 || exterior[0] != exterior[exterior.count() - 1] )
            continue;
          finalMulti[0].append( exterior );

          // add interior rings as new polygons
          for ( int j = 1; j < multi[i].size(); j++ )
          {
            QgsPolygon new_poly;
            new_poly.append( multi[i][j] );
            finalMulti.append( new_poly );
          }
        }
      }
      feat.setGeometry( QgsGeometry::fromMultiPolygon( finalMulti ) );
    }
    if ( feat.geometry() )
      mSubRenderer->renderFeature( feat, mContext );
  }
  for ( FeatureCategoryVector::iterator cit = mFeaturesCategories.begin(); cit != mFeaturesCategories.end(); ++cit )
  {
    foreach ( QgsGeometry* g, cit->geometries )
    {
      delete g;
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
    feat.setGeometry( QgsGeometry::fromPolygon( mExtentPolygon ) );
    mSubRenderer->renderFeature( feat, mContext );
  }

  // draw feature decorations
  foreach ( FeatureDecoration deco, mFeatureDecorations )
  {
    mSubRenderer->renderFeature( deco.feature, mContext, deco.layer, deco.selected, deco.drawMarkers );
  }

  mSubRenderer->stopRender( mContext );
}

QString QgsInvertedPolygonRenderer::dump() const
{
  if ( !mSubRenderer )
  {
    return "INVERTED: NULL";
  }
  return "INVERTED [" + mSubRenderer->dump() + "]";
}

QgsFeatureRendererV2* QgsInvertedPolygonRenderer::clone()
{
  QgsInvertedPolygonRenderer* newRenderer;
  if ( mSubRenderer.isNull() )
  {
    newRenderer = new QgsInvertedPolygonRenderer( 0 );
  }
  else
  {
    newRenderer = new QgsInvertedPolygonRenderer( mSubRenderer->clone() );
  }
  newRenderer->setPreprocessingEnabled( preprocessingEnabled() );
  return newRenderer;
}

QgsFeatureRendererV2* QgsInvertedPolygonRenderer::create( QDomElement& element )
{
  QgsInvertedPolygonRenderer* r = new QgsInvertedPolygonRenderer();
  //look for an embedded renderer <renderer-v2>
  QDomElement embeddedRendererElem = element.firstChildElement( "renderer-v2" );
  if ( !embeddedRendererElem.isNull() )
  {
    r->setEmbeddedRenderer( QgsFeatureRendererV2::load( embeddedRendererElem ) );
  }
  r->setPreprocessingEnabled( element.attribute( "preprocessing", "0" ).toInt() == 1 );
  return r;
}

QDomElement QgsInvertedPolygonRenderer::save( QDomDocument& doc )
{
  QDomElement rendererElem = doc.createElement( RENDERER_TAG_NAME );
  rendererElem.setAttribute( "type", "invertedPolygonRenderer" );
  rendererElem.setAttribute( "preprocessing", preprocessingEnabled() ? "1" : "0" );

  if ( mSubRenderer )
  {
    QDomElement embeddedRendererElem = mSubRenderer->save( doc );
    rendererElem.appendChild( embeddedRendererElem );
  }

  return rendererElem;
}

QgsSymbolV2* QgsInvertedPolygonRenderer::symbolForFeature( QgsFeature& feature )
{
  if ( !mSubRenderer )
  {
    return 0;
  }
  return mSubRenderer->symbolForFeature( feature );
}

QgsSymbolV2List QgsInvertedPolygonRenderer::symbolsForFeature( QgsFeature& feature )
{
  if ( !mSubRenderer )
  {
    return QgsSymbolV2List();
  }
  return mSubRenderer->symbolsForFeature( feature );
}

QgsSymbolV2List QgsInvertedPolygonRenderer::symbols()
{
  if ( !mSubRenderer )
  {
    return QgsSymbolV2List();
  }
  return mSubRenderer->symbols();
}

int QgsInvertedPolygonRenderer::capabilities()
{
  if ( !mSubRenderer )
  {
    return 0;
  }
  return mSubRenderer->capabilities();
}

QList<QString> QgsInvertedPolygonRenderer::usedAttributes()
{
  if ( !mSubRenderer )
  {
    return QList<QString>();
  }
  return mSubRenderer->usedAttributes();
}

QgsLegendSymbologyList QgsInvertedPolygonRenderer::legendSymbologyItems( QSize iconSize )
{
  if ( !mSubRenderer )
  {
    return QgsLegendSymbologyList();
  }
  return mSubRenderer->legendSymbologyItems( iconSize );
}

QgsLegendSymbolList QgsInvertedPolygonRenderer::legendSymbolItems( double scaleDenominator, QString rule )
{
  if ( !mSubRenderer )
  {
    return QgsLegendSymbolList();
  }
  return mSubRenderer->legendSymbolItems( scaleDenominator, rule );
}

bool QgsInvertedPolygonRenderer::willRenderFeature( QgsFeature& feat )
{
  if ( !mSubRenderer )
  {
    return false;
  }
  return mSubRenderer->willRenderFeature( feat );
}

