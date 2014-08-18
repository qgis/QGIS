/***************************************************************************
                         qgscomposermapoverview.cpp
                         --------------------
    begin                : July 2014
    copyright            : (C) 2014 by Nyall Dawson
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

#include "qgscomposermapoverview.h"
#include "qgscomposermap.h"
#include "qgscomposition.h"
#include "qgssymbollayerv2utils.h"
#include "qgssymbolv2.h"

#include <QPainter>

QgsComposerMapOverview::QgsComposerMapOverview( const QString& name, QgsComposerMap* map ):
    mComposerMap( map ),
    mName( name ),
    mUuid( QUuid::createUuid().toString() ),
    mEnabled( true ),
    mFrameMapId( -1 ),
    mFrameSymbol( 0 ),
    mBlendMode( QPainter::CompositionMode_SourceOver ),
    mInverted( false ),
    mCentered( false )
{
  createDefaultFrameSymbol();
}

QgsComposerMapOverview::QgsComposerMapOverview(): mComposerMap( 0 )
{
}

void QgsComposerMapOverview::createDefaultFrameSymbol()
{
  delete mFrameSymbol;
  QgsStringMap properties;
  properties.insert( "color", "255,0,0,255" );
  properties.insert( "style", "solid" );
  properties.insert( "style_border", "no" );
  mFrameSymbol = QgsFillSymbolV2::createSimple( properties );
  mFrameSymbol->setAlpha( 0.3 );
}

QgsComposerMapOverview::~QgsComposerMapOverview()
{
  delete mFrameSymbol;
}

void QgsComposerMapOverview::drawOverview( QPainter *painter ) const
{
  if ( !mEnabled || mFrameMapId == -1 || !mComposerMap || !mComposerMap->composition() )
  {
    return;
  }
  if ( !painter )
  {
    return;
  }

  const QgsComposerMap* overviewFrameMap = mComposerMap->composition()->getComposerMapById( mFrameMapId );
  if ( !overviewFrameMap )
  {
    return;
  }

  //get polygon for other overview frame map's extent (use visibleExtentPolygon as it accounts for map rotation)
  QPolygonF otherExtent = overviewFrameMap->visibleExtentPolygon();

  //get current map's extent as a QPolygonF
  QPolygonF thisExtent = mComposerMap->visibleExtentPolygon();
  //intersect the two
  QPolygonF intersectExtent = thisExtent.intersected( otherExtent );

  //setup painter scaling to dots so that raster symbology is drawn to scale
  double dotsPerMM = painter->device()->logicalDpiX() / 25.4;

  //setup render context
  QgsMapSettings ms = mComposerMap->composition()->mapSettings();
  //context units should be in dots
  ms.setOutputSize( QSizeF( mComposerMap->rect().width() * dotsPerMM, mComposerMap->rect().height() * dotsPerMM ).toSize() );
  ms.setExtent( *mComposerMap->currentMapExtent() );
  ms.setOutputDpi( painter->device()->logicalDpiX() );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( ms );
  context.setForceVectorOutput( true );
  context.setPainter( painter );

  painter->save();
  painter->setCompositionMode( mBlendMode );
  painter->translate( mComposerMap->mXOffset, mComposerMap->mYOffset );
  painter->scale( 1 / dotsPerMM, 1 / dotsPerMM ); // scale painter from mm to dots
  painter->setRenderHint( QPainter::Antialiasing );

  mFrameSymbol->startRender( context );

  //construct a polygon corresponding to the intersecting map extent
  //need to scale line to dots, rather then mm, since the painter has been scaled to dots
  QTransform mapTransform;
  QPolygonF thisRectPoly = QPolygonF( QRectF( 0, 0, dotsPerMM * mComposerMap->rect().width(), dotsPerMM * mComposerMap->rect().height() ) );

  //workaround QT Bug #21329
  thisRectPoly.pop_back();
  //create transform from map coordinates to painter coordinates
  QTransform::quadToQuad( thisExtent, thisRectPoly, mapTransform );
  QPolygonF intersectPolygon;
  intersectPolygon = mapTransform.map( intersectExtent );

  QList<QPolygonF> rings; //empty list
  if ( !mInverted )
  {
    //Render the intersecting map extent
    mFrameSymbol->renderPolygon( intersectPolygon, &rings, 0, context );;
  }
  else
  {
    //We are inverting the overview frame (ie, shading outside the intersecting extent)
    //Construct a polygon corresponding to the overview map extent
    QPolygonF outerPolygon;
    outerPolygon << QPointF( 0, 0 )
    << QPointF( mComposerMap->rect().width() * dotsPerMM, 0 )
    << QPointF( mComposerMap->rect().width() * dotsPerMM, mComposerMap->rect().height() * dotsPerMM )
    << QPointF( 0, mComposerMap->rect().height() * dotsPerMM )
    << QPointF( 0, 0 );

    //Intersecting extent is an inner ring for the shaded area
    rings.append( intersectPolygon );
    mFrameSymbol->renderPolygon( outerPolygon, &rings, 0, context );
  }

  mFrameSymbol->stopRender( context );
  painter->restore();
}

bool QgsComposerMapOverview::writeXML( QDomElement &elem, QDomDocument &doc ) const
{
  if ( elem.isNull() )
  {
    return false;
  }

  //overview map frame
  QDomElement overviewFrameElem = doc.createElement( "ComposerMapOverview" );

  overviewFrameElem.setAttribute( "name", mName );
  overviewFrameElem.setAttribute( "uuid", mUuid );
  overviewFrameElem.setAttribute( "show", mEnabled );

  overviewFrameElem.setAttribute( "frameMap", mFrameMapId );
  overviewFrameElem.setAttribute( "blendMode", QgsMapRenderer::getBlendModeEnum( mBlendMode ) );
  overviewFrameElem.setAttribute( "inverted", mInverted );
  overviewFrameElem.setAttribute( "centered", mCentered );

  QDomElement frameStyleElem = QgsSymbolLayerV2Utils::saveSymbol( QString(), mFrameSymbol, doc );
  overviewFrameElem.appendChild( frameStyleElem );

  elem.appendChild( overviewFrameElem );
  return true;
}

bool QgsComposerMapOverview::readXML( const QDomElement &itemElem, const QDomDocument &doc )
{
  Q_UNUSED( doc );
  if ( itemElem.isNull() )
  {
    return false;
  }

  mName = itemElem.attribute( "name" );
  mUuid = itemElem.attribute( "uuid" );
  mEnabled = ( itemElem.attribute( "show", "0" ) != "0" );

  setFrameMap( itemElem.attribute( "frameMap", "-1" ).toInt() );
  mBlendMode = QgsMapRenderer::getCompositionMode(( QgsMapRenderer::BlendMode ) itemElem.attribute( "blendMode", "0" ).toUInt() );
  mInverted = ( itemElem.attribute( "inverted", "0" ) != "0" );
  mCentered = ( itemElem.attribute( "centered", "0" ) != "0" );


  QDomElement frameStyleElem = itemElem.firstChildElement( "symbol" );
  if ( !frameStyleElem.isNull( ) )
  {
    delete mFrameSymbol;
    mFrameSymbol = dynamic_cast<QgsFillSymbolV2*>( QgsSymbolLayerV2Utils::loadSymbol( frameStyleElem ) );
  }
  return true;
}

void QgsComposerMapOverview::setFrameMap( const int mapId )
{
  if ( mFrameMapId == mapId )
  {
    //no change
    return;
  }

  //disconnect old map
  if ( mFrameMapId != -1 && mComposerMap && mComposerMap->composition() )
  {
    const QgsComposerMap* map = mComposerMap->composition()->getComposerMapById( mFrameMapId );
    if ( map )
    {
      QObject::disconnect( map, SIGNAL( extentChanged() ), this, SLOT( overviewExtentChanged() ) );
    }
  }
  mFrameMapId = mapId;
  //connect to new map signals
  connectSignals();
}

void QgsComposerMapOverview::connectSignals()
{
  if ( !mComposerMap )
  {
    return;
  }

  if ( mFrameMapId != -1 && mComposerMap->composition() )
  {
    const QgsComposerMap* map = mComposerMap->composition()->getComposerMapById( mFrameMapId );
    if ( map )
    {
      QObject::connect( map, SIGNAL( extentChanged() ), this, SLOT( overviewExtentChanged() ) );
    }
  }
}

void QgsComposerMapOverview::setFrameSymbol( QgsFillSymbolV2 *symbol )
{
  delete mFrameSymbol;
  mFrameSymbol = symbol;
}

void QgsComposerMapOverview::setBlendMode( const QPainter::CompositionMode blendMode )
{
  mBlendMode = blendMode;
}

void QgsComposerMapOverview::setInverted( const bool inverted )
{
  mInverted = inverted;
}

void QgsComposerMapOverview::setCentered( const bool centered )
{
  mCentered = centered;
  overviewExtentChanged();
}

void QgsComposerMapOverview::overviewExtentChanged()
{
  if ( !mComposerMap )
  {
    return;
  }

  //if using overview centering, update the map's extent
  if ( mComposerMap->composition() && mCentered && mFrameMapId != -1 )
  {
    QgsRectangle extent = *mComposerMap->currentMapExtent();

    const QgsComposerMap* overviewFrameMap = mComposerMap->composition()->getComposerMapById( mFrameMapId );
    if ( !overviewFrameMap )
    {
      //redraw map so that overview gets updated
      mComposerMap->update();
      return;
    }
    QgsRectangle otherExtent = *overviewFrameMap->currentMapExtent();

    QgsPoint center = otherExtent.center();
    QgsRectangle movedExtent( center.x() - extent.width() / 2,
                              center.y() - extent.height() / 2,
                              center.x() - extent.width() / 2 + extent.width(),
                              center.y() - extent.height() / 2 + extent.height() );
    *mComposerMap->currentMapExtent() = movedExtent;

    //trigger a recalculation of data defined extents, scale and rotation, since that
    //may override the map centering
    mComposerMap->refreshDataDefinedProperty( QgsComposerObject::MapScale );

    //must invalidate cache so that map gets redrawn
    mComposerMap->cache();
  }

  //repaint map so that overview gets updated
  mComposerMap->update();
}
