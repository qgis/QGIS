/***************************************************************************
                         qgscomposermap.cpp
                             -------------------
    begin                : January 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposermap.h"

#include "qgscoordinatetransform.h"
#include "qgslogger.h"
#include "qgsmaprenderer.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaptopixel.h"
#include "qgsproject.h"
#include "qgsmaprenderer.h"
#include "qgsrasterlayer.h"
#include "qgsrendercontext.h"
#include "qgsscalecalculator.h"
#include "qgsvectorlayer.h"

#include "qgslabel.h"
#include "qgslabelattributes.h"

#include <QGraphicsScene>
#include <QPainter>
#include <QSettings>
#include <iostream>
#include <cmath>

int QgsComposerMap::mCurrentComposerId = 0;

QgsComposerMap::QgsComposerMap( QgsComposition *composition, int x, int y, int width, int height )
    : QgsComposerItem( x, y, width, height, composition ), mKeepLayerSet( false )
{
  mComposition = composition;
  mMapRenderer = mComposition->mapRenderer();
  mId = mCurrentComposerId++;
  mPreviewMode = QgsComposerMap::Rectangle;

  // Cache
  mCacheUpdated = false;
  mDrawing = false;

  //Offset
  mXOffset = 0.0;
  mYOffset = 0.0;

  connectUpdateSlot();

  //calculate mExtent based on width/height ratio and map canvas extent
  if ( mMapRenderer )
  {
    mExtent = mMapRenderer->extent();
  }
  setSceneRect( QRectF( x, y, width, height ) );
  setToolTip( tr( "Map %1" ).arg( mId ) );
}

QgsComposerMap::QgsComposerMap( QgsComposition *composition )
    : QgsComposerItem( 0, 0, 10, 10, composition ), mKeepLayerSet( false )
{
  //Offset
  mXOffset = 0.0;
  mYOffset = 0.0;

  connectUpdateSlot();

  mComposition = composition;
  mMapRenderer = mComposition->mapRenderer();
  mId = mCurrentComposerId++;
  mPreviewMode = QgsComposerMap::Rectangle;
  setToolTip( tr( "Map %1" ).arg( mId ) );
  QGraphicsRectItem::show();
}

QgsComposerMap::~QgsComposerMap()
{
}

/* This function is called by paint() and cache() to render the map.  It does not override any functions
from QGraphicsItem. */
void QgsComposerMap::draw( QPainter *painter, const QgsRectangle& extent, const QSize& size, int dpi )
{
  if ( !painter )
  {
    return;
  }

  if ( !mMapRenderer )
  {
    return;
  }

  QgsMapRenderer theMapRenderer;
  theMapRenderer.setExtent( extent );
  theMapRenderer.setOutputSize( size, dpi );

  //use stored layer set or read current set from main canvas
  if ( mKeepLayerSet )
  {
    theMapRenderer.setLayerSet( mLayerSet );
  }
  else
  {
    theMapRenderer.setLayerSet( mMapRenderer->layerSet() );
  }
  theMapRenderer.setProjectionsEnabled( mMapRenderer->hasCrsTransformEnabled() );
  theMapRenderer.setDestinationSrs( mMapRenderer->destinationSrs() );

  //set antialiasing if enabled in options
  QSettings settings;
  if ( settings.value( "/qgis/enable_anti_aliasing", false ).toBool() )
  {
    painter->setRenderHint( QPainter::Antialiasing );
  }

  QgsRenderContext* theRendererContext = theMapRenderer.rendererContext();
  if ( theRendererContext )
  {
    theRendererContext->setDrawEditingInformation( false );
    theRendererContext->setRenderingStopped( false );
  }

  //force composer map scale for scale dependent visibility
  double bk_scale = theMapRenderer.scale();
  theMapRenderer.setScale( scale() );
  theMapRenderer.render( painter );
  theMapRenderer.setScale( bk_scale );
}

void QgsComposerMap::cache( void )
{
  if ( mPreviewMode == Rectangle )
  {
    return;
  }

  if ( mDrawing )
  {
    return;
  }

  mDrawing = true;

  int w = rect().width() * horizontalViewScaleFactor();
  int h = rect().height() * horizontalViewScaleFactor();

  if ( w > 5000 ) //limit size of image for better performance
  {
    w = 5000;
  }

  if ( h > 5000 )
  {
    h = 5000;
  }

  mCacheImage = QImage( w, h,  QImage::Format_ARGB32 );
  mCacheImage.fill( brush().color().rgb() ); //consider the item background brush
  double mapUnitsPerPixel = mExtent.width() / w;

  // WARNING: ymax in QgsMapToPixel is device height!!!
  QgsMapToPixel transform( mapUnitsPerPixel, h, mExtent.yMinimum(), mExtent.xMinimum() );

  QPainter p( &mCacheImage );

  draw( &p, mExtent, QSize( w, h ), mCacheImage.logicalDpiX() );
  p.end();
  mCacheUpdated = true;

  mDrawing = false;
}

void QgsComposerMap::paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget )
{
  if ( !mComposition || !painter )
  {
    return;
  }

  QRectF thisPaintRect = QRectF( 0, 0, QGraphicsRectItem::rect().width(), QGraphicsRectItem::rect().height() );
  painter->save();
  painter->setClipRect( thisPaintRect );

  drawBackground( painter );


  double currentScaleFactorX = horizontalViewScaleFactor();

  if ( mComposition->plotStyle() == QgsComposition::Preview && mPreviewMode == Rectangle )
  {
    QFont messageFont( "", 12 );
    painter->setFont( messageFont );
    painter->setPen( QColor( 0, 0, 0 ) );
    painter->drawText( thisPaintRect, tr( "Map will be printed here" ) );
  }
  else if ( mComposition->plotStyle() == QgsComposition::Preview )
  {
    //draw cached pixmap. This function does not call cache() any more because
    //Qt 4.4.0 and 4.4.1 have problems with recursive paintings
    //QgsComposerMap::cache() and QgsComposerMap::update() need to be called by
    //client functions

    // Scale so that the cache fills the map rectangle
    double scale = 1.0 * QGraphicsRectItem::rect().width() / mCacheImage.width();

    painter->save();
    painter->scale( scale, scale );
    painter->drawImage( mXOffset / scale, mYOffset / scale, mCacheImage );
    painter->restore();
  }
  else if ( mComposition->plotStyle() == QgsComposition::Print ||
            mComposition->plotStyle() == QgsComposition::Postscript )
  {
    if ( mDrawing )
    {
      return;
    }

    mDrawing = true;
    QPaintDevice* thePaintDevice = painter->device();
    if ( !thePaintDevice )
    {
      return;
    }

    QRectF bRect = boundingRect();
    QSize theSize( bRect.width(), bRect.height() );
    draw( painter, mExtent, theSize, 25.4 ); //scene coordinates seem to be in mm
    mDrawing = false;
  }

  painter->setClipRect( thisPaintRect , Qt::NoClip );

  drawFrame( painter );
  if ( isSelected() )
  {
    drawSelectionBoxes( painter );
  }


  painter->restore();
}

void QgsComposerMap::updateCachedImage( void )
{
  syncLayerSet(); //layer list may have changed
  mCacheUpdated = false;
  cache();
  QGraphicsRectItem::update();
}

void QgsComposerMap::renderModeUpdateCachedImage()
{
  if ( mPreviewMode == Render )
  {
    updateCachedImage();
  }
}

void QgsComposerMap::setCacheUpdated( bool u )
{
  mCacheUpdated = u;
}

double QgsComposerMap::scale() const
{
  QgsScaleCalculator calculator;
  calculator.setMapUnits( mMapRenderer->mapUnits() );
  calculator.setDpi( 25.4 );  //QGraphicsView units are mm
  return calculator.calculate( mExtent, rect().width() );
}

void QgsComposerMap::resize( double dx, double dy )
{
  //setRect
  QRectF currentRect = rect();
  QRectF newSceneRect = QRectF( transform().dx(), transform().dy(), currentRect.width() + dx, currentRect.height() + dy );
  setSceneRect( newSceneRect );
}

void QgsComposerMap::moveContent( double dx, double dy )
{
  if ( !mDrawing )
  {
    QRectF itemRect = rect();
    double xRatio = dx / itemRect.width();
    double yRatio = dy / itemRect.height();

    double xMoveMapCoord = mExtent.width() * xRatio;
    double yMoveMapCoord = -( mExtent.height() * yRatio );

    mExtent.setXMinimum( mExtent.xMinimum() + xMoveMapCoord );
    mExtent.setXMaximum( mExtent.xMaximum() + xMoveMapCoord );
    mExtent.setYMinimum( mExtent.yMinimum() + yMoveMapCoord );
    mExtent.setYMaximum( mExtent.yMaximum() + yMoveMapCoord );
    emit extentChanged();
    cache();
    update();
  }
}

void QgsComposerMap::zoomContent( int delta, double x, double y )
{
  if ( mDrawing )
  {
    return;
  }

  QSettings settings;

  //read zoom mode
  //0: zoom, 1: zoom and recenter, 2: zoom to cursor, 3: nothing
  int zoomMode = settings.value( "/qgis/wheel_action", 0 ).toInt();
  if ( zoomMode == 3 ) //do nothing
  {
    return;
  }

  double zoomFactor = settings.value( "/qgis/zoom_factor", 2.0 ).toDouble();

  //find out new center point
  double centerX = ( mExtent.xMaximum() + mExtent.xMinimum() ) / 2;
  double centerY = ( mExtent.yMaximum() + mExtent.yMinimum() ) / 2;

  if ( zoomMode != 0 )
  {
    //find out map coordinates of mouse position
    double mapMouseX = mExtent.xMinimum() + ( x / rect().width() ) * ( mExtent.xMaximum() - mExtent.xMinimum() );
    double mapMouseY = mExtent.yMinimum() + ( 1 - ( y / rect().height() ) ) * ( mExtent.yMaximum() - mExtent.yMinimum() );
    if ( zoomMode == 1 ) //zoom and recenter
    {
      centerX = mapMouseX;
      centerY = mapMouseY;
    }
    else if ( zoomMode == 2 ) //zoom to cursor
    {
      centerX = mapMouseX + ( centerX - mapMouseX ) * ( 1.0 / zoomFactor );
      centerY = mapMouseY + ( centerY - mapMouseY ) * ( 1.0 / zoomFactor );
    }
  }

  double newIntervalX, newIntervalY;

  if ( delta > 0 )
  {
    newIntervalX = ( mExtent.xMaximum() - mExtent.xMinimum() ) / zoomFactor;
    newIntervalY = ( mExtent.yMaximum() - mExtent.yMinimum() ) / zoomFactor;
  }
  else if ( delta < 0 )
  {
    newIntervalX = ( mExtent.xMaximum() - mExtent.xMinimum() ) * zoomFactor;
    newIntervalY = ( mExtent.yMaximum() - mExtent.yMinimum() ) * zoomFactor;
  }
  else //no need to zoom
  {
    return;
  }

  mExtent.setXMaximum( centerX + newIntervalX / 2 );
  mExtent.setXMinimum( centerX - newIntervalX / 2 );
  mExtent.setYMaximum( centerY + newIntervalY / 2 );
  mExtent.setYMinimum( centerY - newIntervalY / 2 );

  emit extentChanged();
  cache();
  update();
}

void QgsComposerMap::setSceneRect( const QRectF& rectangle )
{
  double w = rectangle.width();
  double h = rectangle.height();
  //prepareGeometryChange();

  QgsComposerItem::setSceneRect( rectangle );

  //QGraphicsRectItem::update();
  double newHeight = mExtent.width() * h / w ;
  mExtent = QgsRectangle( mExtent.xMinimum(), mExtent.yMinimum(), mExtent.xMaximum(), mExtent.yMinimum() + newHeight );
  mCacheUpdated = false;
  emit extentChanged();
  if ( mPreviewMode != Rectangle )
  {
    cache();
  }
  update();
}

void QgsComposerMap::setNewExtent( const QgsRectangle& extent )
{
  if ( mExtent == extent )
  {
    return;
  }
  mExtent = extent;

  //adjust height
  QRectF currentRect = rect();

  double newHeight = currentRect.width() * extent.height() / extent.width();

  setSceneRect( QRectF( transform().dx(), transform().dy(), currentRect.width(), newHeight ) );
}

void QgsComposerMap::setNewScale( double scaleDenominator )
{
  double currentScaleDenominator = scale();

  if ( scaleDenominator == currentScaleDenominator )
  {
    return;
  }

  double scaleRatio = scaleDenominator / currentScaleDenominator;

  double newXMax = mExtent.xMinimum() + scaleRatio * ( mExtent.xMaximum() - mExtent.xMinimum() );
  double newYMax = mExtent.yMinimum() + scaleRatio * ( mExtent.yMaximum() - mExtent.yMinimum() );

  QgsRectangle newExtent( mExtent.xMinimum(), mExtent.yMinimum(), newXMax, newYMax );
  mExtent = newExtent;
  mCacheUpdated = false;
  emit extentChanged();
  cache();
  update();
}

void QgsComposerMap::setOffset( double xOffset, double yOffset )
{
  mXOffset = xOffset;
  mYOffset = yOffset;
}

bool QgsComposerMap::containsWMSLayer() const
{
  if ( !mMapRenderer )
  {
    return false;
  }

  QStringList layers = mMapRenderer->layerSet();

  QStringList::const_iterator layer_it = layers.constBegin();
  QgsMapLayer* currentLayer = 0;

  for ( ; layer_it != layers.constEnd(); ++layer_it )
  {
    currentLayer = QgsMapLayerRegistry::instance()->mapLayer( *layer_it );
    if ( currentLayer )
    {
      QgsRasterLayer* currentRasterLayer = dynamic_cast<QgsRasterLayer*>( currentLayer );
      if ( currentRasterLayer )
      {
        const QgsRasterDataProvider* rasterProvider = 0;
        if (( rasterProvider = currentRasterLayer->dataProvider() ) )
        {
          if ( rasterProvider->name() == "wms" )
          {
            return true;
          }
        }
      }
    }
  }
  return false;
}

void QgsComposerMap::connectUpdateSlot()
{
  //connect signal from layer registry to update in case of new or deleted layers
  QgsMapLayerRegistry* layerRegistry = QgsMapLayerRegistry::instance();
  if ( layerRegistry )
  {
    connect( layerRegistry, SIGNAL( layerWillBeRemoved( QString ) ), this, SLOT( updateCachedImage() ) );
    connect( layerRegistry, SIGNAL( layerWasAdded( QgsMapLayer* ) ), this, SLOT( updateCachedImage() ) );
  }
}

bool QgsComposerMap::writeXML( QDomElement& elem, QDomDocument & doc ) const
{
  if ( elem.isNull() )
  {
    return false;
  }

  QDomElement composerMapElem = doc.createElement( "ComposerMap" );

  //previewMode
  if ( mPreviewMode == Cache )
  {
    composerMapElem.setAttribute( "previewMode", "Cache" );
  }
  else if ( mPreviewMode == Render )
  {
    composerMapElem.setAttribute( "previewMode", "Render" );
  }
  else //rectangle
  {
    composerMapElem.setAttribute( "previewMode", "Rectangle" );
  }

  if ( mKeepLayerSet )
  {
    composerMapElem.setAttribute( "keepLayerSet", "true" );
  }
  else
  {
    composerMapElem.setAttribute( "keepLayerSet", "false" );
  }

  //extent
  QDomElement extentElem = doc.createElement( "Extent" );
  extentElem.setAttribute( "xmin", QString::number( mExtent.xMinimum() ) );
  extentElem.setAttribute( "xmax", QString::number( mExtent.xMaximum() ) );
  extentElem.setAttribute( "ymin", QString::number( mExtent.yMinimum() ) );
  extentElem.setAttribute( "ymax", QString::number( mExtent.yMaximum() ) );
  composerMapElem.appendChild( extentElem );

  //layer set
  QDomElement layerSetElem = doc.createElement( "LayerSet" );
  QStringList::const_iterator layerIt = mLayerSet.constBegin();
  for ( ; layerIt != mLayerSet.constEnd(); ++layerIt )
  {
    QDomElement layerElem = doc.createElement( "Layer" );
    QDomText layerIdText = doc.createTextNode( *layerIt );
    layerElem.appendChild( layerIdText );
    layerSetElem.appendChild( layerElem );
  }
  composerMapElem.appendChild( layerSetElem );

  elem.appendChild( composerMapElem );
  return _writeXML( composerMapElem, doc );
}

bool QgsComposerMap::readXML( const QDomElement& itemElem, const QDomDocument& doc )
{
  if ( itemElem.isNull() )
  {
    return false;
  }

  mPreviewMode = Rectangle;

  //previewMode
  QString previewMode = itemElem.attribute( "previewMode" );
  if ( previewMode == "Cache" )
  {
    mPreviewMode = Cache;
  }
  else if ( previewMode == "Render" )
  {
    mPreviewMode = Render;
  }
  else
  {
    mPreviewMode = Rectangle;
  }

  //extent
  QDomNodeList extentNodeList = itemElem.elementsByTagName( "Extent" );
  if ( extentNodeList.size() > 0 )
  {
    QDomElement extentElem = extentNodeList.at( 0 ).toElement();
    double xmin, xmax, ymin, ymax;
    xmin = extentElem.attribute( "xmin" ).toDouble();
    xmax = extentElem.attribute( "xmax" ).toDouble();
    ymin = extentElem.attribute( "ymin" ).toDouble();
    ymax = extentElem.attribute( "ymax" ).toDouble();

    mExtent = QgsRectangle( xmin, ymin, xmax, ymax );
  }

  //mKeepLayerSet flag
  QString keepLayerSetFlag = itemElem.attribute( "keepLayerSet" );
  if ( keepLayerSetFlag.compare( "true", Qt::CaseInsensitive ) == 0 )
  {
    mKeepLayerSet = true;
  }
  else
  {
    mKeepLayerSet = false;
  }

  //mLayerSet
  QDomNodeList layerSetNodeList = itemElem.elementsByTagName( "LayerSet" );
  QStringList layerSet;
  if ( layerSetNodeList.size() > 0 )
  {
    QDomElement layerSetElem = layerSetNodeList.at( 0 ).toElement();
    QDomNodeList layerIdNodeList = layerSetElem.elementsByTagName( "Layer" );
    for ( int i = 0; i < layerIdNodeList.size(); ++i )
    {
      layerSet << layerIdNodeList.at( i ).toElement().text();
    }
  }
  mLayerSet = layerSet;

  mDrawing = false;
  mNumCachedLayers = 0;
  mCacheUpdated = false;

  //restore general composer item properties
  QDomNodeList composerItemList = itemElem.elementsByTagName( "ComposerItem" );
  if ( composerItemList.size() > 0 )
  {
    QDomElement composerItemElem = composerItemList.at( 0 ).toElement();
    _readXML( composerItemElem, doc );
  }

  if ( mPreviewMode != Rectangle )
  {
    cache();
    update();
  }

  return true;
}

void QgsComposerMap::storeCurrentLayerSet()
{
  if ( mMapRenderer )
  {
    mLayerSet = mMapRenderer->layerSet();
  }
}

void QgsComposerMap::syncLayerSet()
{
  if ( mLayerSet.size() < 1 && !mMapRenderer )
  {
    return;
  }

  QStringList currentLayerSet = mMapRenderer->layerSet();
  for ( int i = mLayerSet.size() - 1; i >= 0; --i )
  {
    if ( !currentLayerSet.contains( mLayerSet.at( i ) ) )
    {
      mLayerSet.removeAt( i );
    }
  }
}
