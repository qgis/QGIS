/***************************************************************************
                         qgslayoutitemmapoverview.cpp
                         --------------------
    begin                : October 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgslayoutitemmapoverview.h"
#include "qgslayoutitemmap.h"
#include "qgslayout.h"
#include "qgssymbollayerutils.h"
#include "qgssymbol.h"
#include "qgsmapsettings.h"
#include "qgspainting.h"
#include "qgspathresolver.h"
#include "qgsreadwritecontext.h"
#include "qgslayoututils.h"
#include "qgsexception.h"

#include <QPainter>

QgsLayoutItemMapOverview::QgsLayoutItemMapOverview( const QString &name, QgsLayoutItemMap *map )
  : QgsLayoutItemMapItem( name, map )
{
  createDefaultFrameSymbol();
}

void QgsLayoutItemMapOverview::createDefaultFrameSymbol()
{
  QgsStringMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "255,0,0,75" ) );
  properties.insert( QStringLiteral( "style" ), QStringLiteral( "solid" ) );
  properties.insert( QStringLiteral( "style_border" ), QStringLiteral( "no" ) );
  mFrameSymbol.reset( QgsFillSymbol::createSimple( properties ) );
}

void QgsLayoutItemMapOverview::draw( QPainter *painter )
{
  if ( !mEnabled || mFrameMapId.isEmpty() || !mMap || !mMap->layout() )
  {
    return;
  }
  if ( !painter )
  {
    return;
  }

  const QgsLayoutItemMap *overviewFrameMap = frameMap();
  if ( !overviewFrameMap )
  {
    return;
  }

  //get polygon for other overview frame map's extent (use visibleExtentPolygon as it accounts for map rotation)
  QPolygonF otherExtent = overviewFrameMap->visibleExtentPolygon();
  if ( overviewFrameMap->crs() !=
       mMap->crs() )
  {
    QgsGeometry g = QgsGeometry::fromQPolygonF( otherExtent );

    // reproject extent
    QgsCoordinateTransform ct( overviewFrameMap->crs(),
                               mMap->crs() );
    g = g.densifyByCount( 20 );
    try
    {
      g.transform( ct );
    }
    catch ( QgsCsException & )
    {
    }

    otherExtent = g.asQPolygonF();
  }

  //get current map's extent as a QPolygonF
  QPolygonF thisExtent = mMap->visibleExtentPolygon();
  //intersect the two
  QPolygonF intersectExtent = thisExtent.intersected( otherExtent );

  //setup painter scaling to dots so that raster symbology is drawn to scale
  double dotsPerMM = painter->device()->logicalDpiX() / 25.4;

  //setup render context
  QgsRenderContext context = QgsLayoutUtils::createRenderContextForLayout( mLayout, painter );
  context.setForceVectorOutput( true );
  QgsExpressionContext expressionContext = createExpressionContext();
  context.setExpressionContext( expressionContext );

  painter->save();
  painter->setCompositionMode( mBlendMode );
  painter->translate( mMap->mXOffset, mMap->mYOffset );
  painter->scale( 1 / dotsPerMM, 1 / dotsPerMM ); // scale painter from mm to dots
  painter->setRenderHint( QPainter::Antialiasing );

  mFrameSymbol->startRender( context );

  //construct a polygon corresponding to the intersecting map extent
  //need to scale line to dots, rather then mm, since the painter has been scaled to dots
  QTransform mapTransform;
  QPolygonF thisRectPoly = QPolygonF( QRectF( 0, 0, dotsPerMM * mMap->rect().width(), dotsPerMM * mMap->rect().height() ) );

  //workaround QT Bug #21329
  thisRectPoly.pop_back();
  thisExtent.pop_back();

  //create transform from map coordinates to painter coordinates
  QTransform::quadToQuad( thisExtent, thisRectPoly, mapTransform );
  QPolygonF intersectPolygon;
  intersectPolygon = mapTransform.map( intersectExtent );

  QList<QPolygonF> rings; //empty list
  if ( !mInverted )
  {
    //Render the intersecting map extent
    mFrameSymbol->renderPolygon( intersectPolygon, &rings, nullptr, context );
  }
  else
  {
    //We are inverting the overview frame (ie, shading outside the intersecting extent)
    //Construct a polygon corresponding to the overview map extent
    QPolygonF outerPolygon;
    outerPolygon << QPointF( 0, 0 )
                 << QPointF( mMap->rect().width() * dotsPerMM, 0 )
                 << QPointF( mMap->rect().width() * dotsPerMM, mMap->rect().height() * dotsPerMM )
                 << QPointF( 0, mMap->rect().height() * dotsPerMM )
                 << QPointF( 0, 0 );

    //Intersecting extent is an inner ring for the shaded area
    rings.append( intersectPolygon );
    mFrameSymbol->renderPolygon( outerPolygon, &rings, nullptr, context );
  }

  mFrameSymbol->stopRender( context );
  painter->restore();
}

bool QgsLayoutItemMapOverview::writeXml( QDomElement &elem, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  if ( elem.isNull() )
  {
    return false;
  }

  //overview map frame
  QDomElement overviewFrameElem = doc.createElement( QStringLiteral( "ComposerMapOverview" ) );

  overviewFrameElem.setAttribute( QStringLiteral( "frameMap" ), mFrameMapId );
  overviewFrameElem.setAttribute( QStringLiteral( "blendMode" ), QgsPainting::getBlendModeEnum( mBlendMode ) );
  overviewFrameElem.setAttribute( QStringLiteral( "inverted" ), mInverted );
  overviewFrameElem.setAttribute( QStringLiteral( "centered" ), mCentered );

  QDomElement frameStyleElem = QgsSymbolLayerUtils::saveSymbol( QString(), mFrameSymbol.get(), doc, context );
  overviewFrameElem.appendChild( frameStyleElem );

  bool ok = QgsLayoutItemMapItem::writeXml( overviewFrameElem, doc, context );
  elem.appendChild( overviewFrameElem );
  return ok;
}

bool QgsLayoutItemMapOverview::readXml( const QDomElement &itemElem, const QDomDocument &doc, const QgsReadWriteContext &context )
{
  Q_UNUSED( doc );
  if ( itemElem.isNull() )
  {
    return false;
  }

  bool ok = QgsLayoutItemMapItem::readXml( itemElem, doc, context );

#if 0 //TODO
  setFrameMapUuid( itemElem.attribute( QStringLiteral( "frameMap" ), QStringLiteral( "-1" ) ).toInt() );
#endif
  mBlendMode = QgsPainting::getCompositionMode( static_cast< QgsPainting::BlendMode >( itemElem.attribute( QStringLiteral( "blendMode" ), QStringLiteral( "0" ) ).toUInt() ) );
  mInverted = ( itemElem.attribute( QStringLiteral( "inverted" ), QStringLiteral( "0" ) ) != QLatin1String( "0" ) );
  mCentered = ( itemElem.attribute( QStringLiteral( "centered" ), QStringLiteral( "0" ) ) != QLatin1String( "0" ) );

  QDomElement frameStyleElem = itemElem.firstChildElement( QStringLiteral( "symbol" ) );
  if ( !frameStyleElem.isNull() )
  {
    mFrameSymbol.reset( QgsSymbolLayerUtils::loadSymbol<QgsFillSymbol>( frameStyleElem, context ) );
  }
  return ok;
}

bool QgsLayoutItemMapOverview::usesAdvancedEffects() const
{
  return mBlendMode != QPainter::CompositionMode_SourceOver;
}

void QgsLayoutItemMapOverview::setFrameMapUuid( const QString &mapId )
{
  if ( mFrameMapId == mapId )
  {
    //no change
    return;
  }

  //disconnect old map
  if ( QgsLayoutItemMap *map = frameMap() )
  {
    disconnect( map, &QgsLayoutItemMap::extentChanged, this, &QgsLayoutItemMapOverview::overviewExtentChanged );
    disconnect( map, &QgsLayoutItemMap::mapRotationChanged, this, &QgsLayoutItemMapOverview::overviewExtentChanged );
  }
  mFrameMapId = mapId;
  //connect to new map signals
  connectSignals();
  mMap->update();
}

void QgsLayoutItemMapOverview::setFrameMap( QgsLayoutItemMap *map )
{
  setFrameMapUuid( map ? map->uuid() : QString() );
}

QgsLayoutItemMap *QgsLayoutItemMapOverview::frameMap()
{
  if ( !mFrameMapId.isEmpty() && mMap && mMap->layout() )
  {
    return dynamic_cast< QgsLayoutItemMap * >( mMap->layout()->itemByUuid( mFrameMapId ) );
  }
  return nullptr;
}

void QgsLayoutItemMapOverview::connectSignals()
{
  if ( !mMap )
  {
    return;
  }

  if ( QgsLayoutItemMap *map = frameMap() )
  {
    connect( map, &QgsLayoutItemMap::extentChanged, this, &QgsLayoutItemMapOverview::overviewExtentChanged );
    connect( map, &QgsLayoutItemMap::mapRotationChanged, this, &QgsLayoutItemMapOverview::overviewExtentChanged );
  }
}

void QgsLayoutItemMapOverview::setFrameSymbol( QgsFillSymbol *symbol )
{
  mFrameSymbol.reset( symbol );
}

QgsFillSymbol *QgsLayoutItemMapOverview::frameSymbol()
{
  return mFrameSymbol.get();
}

const QgsFillSymbol *QgsLayoutItemMapOverview::frameSymbol() const
{
  return mFrameSymbol.get();
}

void QgsLayoutItemMapOverview::setBlendMode( const QPainter::CompositionMode blendMode )
{
  mBlendMode = blendMode;
}

void QgsLayoutItemMapOverview::setInverted( const bool inverted )
{
  mInverted = inverted;
}

void QgsLayoutItemMapOverview::setCentered( const bool centered )
{
  mCentered = centered;
  overviewExtentChanged();
}

void QgsLayoutItemMapOverview::overviewExtentChanged()
{
  if ( !mMap )
  {
    return;
  }

  //if using overview centering, update the map's extent
  if ( mMap->layout() && mCentered && !mFrameMapId.isEmpty() )
  {
    QgsRectangle extent = mMap->extent();

    QgsLayoutItemMap *overviewFrameMap = frameMap();
    if ( !overviewFrameMap )
    {
      //redraw map so that overview gets updated
      mMap->update();
      return;
    }
    QgsRectangle otherExtent = overviewFrameMap->extent();

    QgsPointXY center = otherExtent.center();
    QgsRectangle movedExtent( center.x() - extent.width() / 2,
                              center.y() - extent.height() / 2,
                              center.x() - extent.width() / 2 + extent.width(),
                              center.y() - extent.height() / 2 + extent.height() );
    mMap->setExtent( movedExtent );

    //must invalidate cache so that map gets redrawn
    mMap->invalidateCache();
  }

  //repaint map so that overview gets updated
  mMap->update();
}


//
// QgsLayoutItemMapOverviewStack
//

QgsLayoutItemMapOverviewStack::QgsLayoutItemMapOverviewStack( QgsLayoutItemMap *map )
  : QgsLayoutItemMapItemStack( map )
{

}

void QgsLayoutItemMapOverviewStack::addOverview( QgsLayoutItemMapOverview *overview )
{
  QgsLayoutItemMapItemStack::addItem( overview );
}

void QgsLayoutItemMapOverviewStack::removeOverview( const QString &overviewId )
{
  QgsLayoutItemMapItemStack::removeItem( overviewId );
}

void QgsLayoutItemMapOverviewStack::moveOverviewUp( const QString &overviewId )
{
  QgsLayoutItemMapItemStack::moveItemUp( overviewId );
}

void QgsLayoutItemMapOverviewStack::moveOverviewDown( const QString &overviewId )
{
  QgsLayoutItemMapItemStack::moveItemDown( overviewId );
}

QgsLayoutItemMapOverview *QgsLayoutItemMapOverviewStack::overview( const QString &overviewId ) const
{
  QgsLayoutItemMapItem *item = QgsLayoutItemMapItemStack::item( overviewId );
  return dynamic_cast<QgsLayoutItemMapOverview *>( item );
}

QgsLayoutItemMapOverview *QgsLayoutItemMapOverviewStack::overview( const int index ) const
{
  QgsLayoutItemMapItem *item = QgsLayoutItemMapItemStack::item( index );
  return dynamic_cast<QgsLayoutItemMapOverview *>( item );
}

QgsLayoutItemMapOverview &QgsLayoutItemMapOverviewStack::operator[]( int idx )
{
  QgsLayoutItemMapItem *item = mItems.at( idx );
  QgsLayoutItemMapOverview *overview = dynamic_cast<QgsLayoutItemMapOverview *>( item );
  return *overview;
}

QList<QgsLayoutItemMapOverview *> QgsLayoutItemMapOverviewStack::asList() const
{
  QList< QgsLayoutItemMapOverview * > list;
  QList< QgsLayoutItemMapItem * >::const_iterator it = mItems.begin();
  for ( ; it != mItems.end(); ++it )
  {
    QgsLayoutItemMapOverview *overview = dynamic_cast<QgsLayoutItemMapOverview *>( *it );
    if ( overview )
    {
      list.append( overview );
    }
  }
  return list;
}

bool QgsLayoutItemMapOverviewStack::readXml( const QDomElement &elem, const QDomDocument &doc, const QgsReadWriteContext &context )
{
  removeItems();

  //read overview stack
  QDomNodeList mapOverviewNodeList = elem.elementsByTagName( QStringLiteral( "ComposerMapOverview" ) );
  for ( int i = 0; i < mapOverviewNodeList.size(); ++i )
  {
    QDomElement mapOverviewElem = mapOverviewNodeList.at( i ).toElement();
    QgsLayoutItemMapOverview *mapOverview = new QgsLayoutItemMapOverview( mapOverviewElem.attribute( QStringLiteral( "name" ) ), mMap );
    mapOverview->readXml( mapOverviewElem, doc, context );
    mItems.append( mapOverview );
  }

  return true;
}
