/***************************************************************************
    qgsmaskrendererv2.cpp
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

#include "qgsmaskrendererv2.h"

#include "qgssymbolv2.h"
#include "qgssymbollayerv2utils.h"

#include "qgslogger.h"
#include "qgsfeature.h"
#include "qgsvectorlayer.h"
#include "qgssymbollayerv2.h"
#include "qgsogcutils.h"

#include <QDomDocument>
#include <QDomElement>

QgsMaskRendererV2::QgsMaskRendererV2( const QgsFeatureRendererV2* subRenderer )
  : QgsFeatureRendererV2( "maskRenderer" )
{
  if ( subRenderer ) {
    setEmbeddedRenderer( subRenderer );
  }
  else {
    mSubRenderer.reset( QgsFeatureRendererV2::defaultRenderer( QGis::Polygon ) );
  }
}

QgsMaskRendererV2::~QgsMaskRendererV2()
{
}

void QgsMaskRendererV2::setEmbeddedRenderer( const QgsFeatureRendererV2* subRenderer )
{
  if ( subRenderer ) {
    mSubRenderer.reset( const_cast<QgsFeatureRendererV2*>(subRenderer)->clone() );
  }
  else {
    mSubRenderer.reset( 0 );
  }
}

const QgsFeatureRendererV2* QgsMaskRendererV2::embeddedRenderer() const
{
  return mSubRenderer.data();
}

void QgsMaskRendererV2::startRender( QgsRenderContext& context, const QgsFields& fields )
{
  if ( !mSubRenderer ) {
    return;
  }

  mSubRenderer->startRender( context, fields );
  mFeaturesCategoryMap.clear();
  mFeatureDecorations.clear();
  mFields = fields;
}

bool QgsMaskRendererV2::renderFeature( QgsFeature& feature, QgsRenderContext& context, int layer, bool selected, bool drawVertexMarker )
{
  Q_UNUSED( context );

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
  if ( capabilities() & MoreSymbolsPerFeature ) {
    QgsSymbolV2List syms( mSubRenderer->symbolsForFeature( feature ) );
    foreach ( QgsSymbolV2* sym, syms )
    {
      // append the memory address
      catId.append( reinterpret_cast<const char*>(&sym), sizeof(sym) );
    }
  }
  else
  {
    QgsSymbolV2* sym = mSubRenderer->symbolForFeature( feature );
    if (sym) {
      catId.append( reinterpret_cast<const char*>(&sym), sizeof(sym) );
    }
  }
  if ( !catId.isEmpty() )
  {
    mFeaturesCategoryMap[catId].append( feature );
  }

  // store this feature as a feature to render with decoration if needed
  if ( selected || drawVertexMarker )
  {
    mFeatureDecorations.append( FeatureDecoration( feature, selected, drawVertexMarker, layer) );
  }

  return true;
}

void QgsMaskRendererV2::stopRender( QgsRenderContext& context )
{
  if ( !mSubRenderer ) {
    return;
  }

  // We build here a "reversed" geometry of all the polygons
  //
  // The final geometry is a multipolygon F, with :
  // * the first polygon of F having the current extent as its exterior ring
  // * each polygon's exterior ring is added as interior ring of the first polygon of F
  // * each polygon's interior ring is added as new polygons in F
  //
  // No validity check is done, on purpose, it will be very slow and painting
  // operations do not need geometries to be valid
  for ( FeatureCategoryMap::iterator cit = mFeaturesCategoryMap.begin(); cit != mFeaturesCategoryMap.end(); ++cit)
  {
    QgsMultiPolygon geom;
    geom.push_back( QgsPolygon() );

    // build a rectangle out of the current extent
    QgsRectangle e = context.extent();
    // add some space to hide borders
    e.scale(2.0);
    QgsPolyline extent_ring;
    extent_ring.push_back( QgsPoint(e.xMinimum(), e.yMinimum()) );
    extent_ring.push_back( QgsPoint(e.xMaximum(), e.yMinimum()) );
    extent_ring.push_back( QgsPoint(e.xMaximum(), e.yMaximum()) );
    extent_ring.push_back( QgsPoint(e.xMinimum(), e.yMaximum()) );
    extent_ring.push_back( QgsPoint(e.xMinimum(), e.yMinimum()) );
    geom[0].push_back( extent_ring );

    for ( QList<QgsFeature>::iterator fit = cit.value().begin(); fit != cit.value().end(); ++fit )
    {
      if ( ! fit->geometry() ) {
        continue;
      }
      QgsMultiPolygon multi;
      if ( (fit->geometry()->wkbType() == QGis::WKBPolygon) || (fit->geometry()->wkbType() == QGis::WKBPolygon25D) ) {
        multi.push_back( fit->geometry()->asPolygon() );
      }
      else if ( (fit->geometry()->wkbType() == QGis::WKBMultiPolygon) || (fit->geometry()->wkbType() == QGis::WKBMultiPolygon25D) ) {
        multi = fit->geometry()->asMultiPolygon();
      }
      for ( int i = 0; i < multi.size(); i++ ) {
        // add the exterior ring as interior ring
        geom[0].push_back( multi[i][0] );
        // add interior rings as new exterior rings
        for ( int j = 1; j < multi[i].size(); j++ ) {
          QgsPolygon new_poly;
          new_poly.push_back( multi[i][j] );
          geom.push_back( new_poly );
        }
      }
    }

    QgsFeature feat( cit.value()[0] );
    feat.setGeometry( QgsGeometry::fromMultiPolygon( geom ) );
    mSubRenderer->renderFeature( feat, context );
  }

  // when no features are visible, we still have to draw the exterior rectangle
  if ( mFeaturesCategoryMap.isEmpty() )
  {
    QgsRectangle e = context.extent();
    // add some space to hide borders
    e.scale(2.0);
    QgsFeature feat( mFields );
    feat.setGeometry( QgsGeometry::fromRect(e) );
    mSubRenderer->renderFeature( feat, context );    
  }

  // draw feature decorations
  foreach (FeatureDecoration deco, mFeatureDecorations )
  {
    mSubRenderer->renderFeature( deco.feature, context, deco.layer, deco.selected, deco.drawMarkers );
  }

  mSubRenderer->stopRender( context );
}

QString QgsMaskRendererV2::dump() const
{
  if ( !mSubRenderer ) {
    return "MASK: NULL";
  }
  return "MASK [" + mSubRenderer->dump() + "]";
}

QgsFeatureRendererV2* QgsMaskRendererV2::clone()
{
  QgsMaskRendererV2* r = new QgsMaskRendererV2( mSubRenderer.data() );
  return r;
}

QgsFeatureRendererV2* QgsMaskRendererV2::create( QDomElement& element )
{
  QgsMaskRendererV2* r = new QgsMaskRendererV2();
  //look for an embedded renderer <renderer-v2>
  QDomElement embeddedRendererElem = element.firstChildElement( "renderer-v2" );
  if ( !embeddedRendererElem.isNull() )
  {
    r->setEmbeddedRenderer( QgsFeatureRendererV2::load( embeddedRendererElem ) );
  }
  return r;
}

QDomElement QgsMaskRendererV2::save( QDomDocument& doc )
{
  QDomElement rendererElem = doc.createElement( RENDERER_TAG_NAME );
  rendererElem.setAttribute( "type", "maskRenderer" );

  if ( mSubRenderer )
  {
    QDomElement embeddedRendererElem = mSubRenderer->save( doc );
    rendererElem.appendChild( embeddedRendererElem );
  }

  return rendererElem;
}

QgsSymbolV2* QgsMaskRendererV2::symbolForFeature( QgsFeature& feature )
{
  if ( !mSubRenderer ) {
    return 0;
  }
  return mSubRenderer->symbolForFeature( feature );
}

QgsSymbolV2List QgsMaskRendererV2::symbolsForFeature( QgsFeature& feature )
{
  if ( !mSubRenderer ) {
    return QgsSymbolV2List();
  }
  return mSubRenderer->symbolsForFeature( feature );
}

QgsSymbolV2List QgsMaskRendererV2::symbols()
{
  if ( !mSubRenderer ) {
    return QgsSymbolV2List();
  }
  return mSubRenderer->symbols();
}

int QgsMaskRendererV2::capabilities()
{
  if ( !mSubRenderer ) {
    return 0;
  }
  return mSubRenderer->capabilities();
}

QList<QString> QgsMaskRendererV2::usedAttributes()
{
  if ( !mSubRenderer ) {
    return QList<QString>();
  }
  return mSubRenderer->usedAttributes();
}

QgsLegendSymbologyList QgsMaskRendererV2::legendSymbologyItems( QSize iconSize )
{
  if ( !mSubRenderer ) {
    return QgsLegendSymbologyList();
  }
  return mSubRenderer->legendSymbologyItems( iconSize );
}

QgsLegendSymbolList QgsMaskRendererV2::legendSymbolItems( double scaleDenominator, QString rule )
{
  if ( !mSubRenderer ) {
    return QgsLegendSymbolList();
  }
  return mSubRenderer->legendSymbolItems( scaleDenominator, rule );
}

bool QgsMaskRendererV2::willRenderFeature( QgsFeature& feat )
{
  if ( !mSubRenderer ) {
    return false;
  }
  return mSubRenderer->willRenderFeature( feat );
}
