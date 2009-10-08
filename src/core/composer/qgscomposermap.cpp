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
    : QgsComposerItem( x, y, width, height, composition ), mKeepLayerSet( false ), mGridEnabled( false ), mGridStyle( Solid ), \
    mGridIntervalX( 0.0 ), mGridIntervalY( 0.0 ), mGridOffsetX( 0.0 ), mGridOffsetY( 0.0 ), mShowGridAnnotation( false ), \
    mGridAnnotationPosition( OutsideMapFrame ), mAnnotationFrameDistance( 1.0 ), mGridAnnotationDirection( Horizontal )
{
  mComposition = composition;
  mMapRenderer = mComposition->mapRenderer();
  mId = mCurrentComposerId++;
  mPreviewMode = QgsComposerMap::Rectangle;
  mCurrentRectangle = rect();

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
    : QgsComposerItem( 0, 0, 10, 10, composition ), mKeepLayerSet( false ), mGridEnabled( false ), mGridStyle( Solid ), \
    mGridIntervalX( 0.0 ), mGridIntervalY( 0.0 ), mGridOffsetX( 0.0 ), mGridOffsetY( 0.0 ), mShowGridAnnotation( false ), \
    mGridAnnotationPosition( OutsideMapFrame ), mAnnotationFrameDistance( 1.0 ), mGridAnnotationDirection( Horizontal )
{
  //Offset
  mXOffset = 0.0;
  mYOffset = 0.0;

  connectUpdateSlot();

  mComposition = composition;
  mMapRenderer = mComposition->mapRenderer();
  mId = mCurrentComposerId++;
  mPreviewMode = QgsComposerMap::Rectangle;
  mCurrentRectangle = rect();

  setToolTip( tr( "Map %1" ).arg( mId ) );
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
  if ( mGridEnabled )
  {
    drawGrid( painter );
  }

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

  //grid
  QDomElement gridElem = doc.createElement( "Grid" );
  gridElem.setAttribute( "show", mGridEnabled );
  gridElem.setAttribute( "gridStyle", mGridStyle );
  gridElem.setAttribute( "intervalX", mGridIntervalX );
  gridElem.setAttribute( "intervalY", mGridIntervalY );
  gridElem.setAttribute( "offsetX", mGridOffsetX );
  gridElem.setAttribute( "offsetY", mGridOffsetY );
  gridElem.setAttribute( "penWidth", mGridPen.widthF() );
  gridElem.setAttribute( "penColorRed", mGridPen.color().red() );
  gridElem.setAttribute( "penColorGreen", mGridPen.color().green() );
  gridElem.setAttribute( "penColorBlue", mGridPen.color().blue() );

  //grid annotation
  QDomElement annotationElem = doc.createElement( "Annotation" );
  annotationElem.setAttribute( "show", mShowGridAnnotation );
  annotationElem.setAttribute( "position", mGridAnnotationPosition );
  annotationElem.setAttribute( "frameDistance", mAnnotationFrameDistance );
  annotationElem.setAttribute( "direction", mGridAnnotationDirection );
  annotationElem.setAttribute( "font", mGridAnnotationFont.toString() );

  gridElem.appendChild( annotationElem );
  composerMapElem.appendChild( gridElem );

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

  //grid
  QDomNodeList gridNodeList = itemElem.elementsByTagName( "Grid" );
  if ( gridNodeList.size() > 0 )
  {
    QDomElement gridElem = gridNodeList.at( 0 ).toElement();
    mGridEnabled = ( gridElem.attribute( "show", "0" ) != "0" );
    mGridStyle = QgsComposerMap::GridStyle( gridElem.attribute( "gridStyle", "0" ).toInt() );
    mGridIntervalX = gridElem.attribute( "intervalX", "0" ).toDouble();
    mGridIntervalY = gridElem.attribute( "intervalY", "0" ).toDouble();
    mGridOffsetX = gridElem.attribute( "offsetX", "0" ).toDouble();
    mGridOffsetY = gridElem.attribute( "offsetY", "0" ).toDouble();
    mGridPen.setWidthF( gridElem.attribute( "penWidth", "0" ).toDouble() );
    mGridPen.setColor( QColor( gridElem.attribute( "penColorRed", "0" ).toInt(), \
                               gridElem.attribute( "penColorGreen", "0" ).toInt(), \
                               gridElem.attribute( "penColorBlue", "0" ).toInt() ) );

    QDomNodeList annotationNodeList = gridElem.elementsByTagName( "Annotation" );
    if ( annotationNodeList.size() > 0 )
    {
      QDomElement annotationElem = annotationNodeList.at( 0 ).toElement();
      mShowGridAnnotation = ( annotationElem.attribute( "show", "0" ) != "0" );
      mGridAnnotationPosition = QgsComposerMap::GridAnnotationPosition( annotationElem.attribute( "position", "0" ).toInt() );
      mAnnotationFrameDistance = annotationElem.attribute( "frameDistance", "0" ).toDouble();
      mGridAnnotationDirection = QgsComposerMap::GridAnnotationDirection( annotationElem.attribute( "direction", "0" ).toInt() );
      mGridAnnotationFont.fromString( annotationElem.attribute( "font", "" ) );
    }
  }

  //restore general composer item properties
  QDomNodeList composerItemList = itemElem.elementsByTagName( "ComposerItem" );
  if ( composerItemList.size() > 0 )
  {
    QDomElement composerItemElem = composerItemList.at( 0 ).toElement();
    _readXML( composerItemElem, doc );
  }

  updateBoundingRect();

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

void QgsComposerMap::drawGrid( QPainter* p )
{
  p->setPen( mGridPen );

  QList< QPair< double, QLineF > > verticalLines;
  verticalGridLines( verticalLines );
  QList< QPair< double, QLineF > >::const_iterator vIt = verticalLines.constBegin();
  QList< QPair< double, QLineF > > horizontalLines;
  horizontalGridLines( horizontalLines );
  QList< QPair< double, QLineF > >::const_iterator hIt = horizontalLines.constBegin();

  //simpler approach: draw vertical lines first, then horizontal ones
  if ( mGridStyle == QgsComposerMap::Solid )
  {
    for ( ; vIt != verticalLines.constEnd(); ++vIt )
    {
      p->drawLine( vIt->second );
    }

    for ( ; hIt != horizontalLines.constEnd(); ++hIt )
    {
      p->drawLine( hIt->second );
    }
  }
  //more complicated approach. Find out all the crossings between the lines
  //needs to be adapted once rotation is possible
  else if ( mGridStyle == QgsComposerMap::Cross )
  {

    double resolutionXSize = mGridIntervalX * ( rect().width() / mExtent.width() );
    double resolutionYSize = mGridIntervalY * ( rect().height() / mExtent.height() );

    QLineF currHLine;
    QLineF currVLine;
    for ( ; hIt != horizontalLines.constEnd(); ++hIt )
    {
      currHLine = hIt->second;
      vIt = verticalLines.constBegin();
      for ( ; vIt != verticalLines.constEnd(); ++vIt )
      {
        currVLine = vIt->second;

        //intersection
        //find out intersection point
        QPointF intersectionPoint;
        QLineF::IntersectType t = currHLine.intersect( currVLine, &intersectionPoint );
        if ( t == QLineF::BoundedIntersection )
        {
          p->drawLine( intersectionPoint.x() - resolutionXSize / 6.0, intersectionPoint.y(), intersectionPoint.x() + resolutionXSize / 6.0, intersectionPoint.y() );
          p->drawLine( intersectionPoint.x(), intersectionPoint.y() - resolutionYSize / 6.0, intersectionPoint.x(), intersectionPoint.y() + resolutionYSize / 6.0 );
        }
      }
    }
  }

  if ( mShowGridAnnotation )
  {
    drawGridAnnotations( p, horizontalLines, verticalLines );
  }
}

void QgsComposerMap::drawGridAnnotations( QPainter* p, const QList< QPair< double, QLineF > >& hLines, const QList< QPair< double, QLineF > >& vLines )
{
  //annotations. todo: make left / right, within / outside and distances configurable
  //current annotation, width and height
  QString currentAnnotationString;
  double currentFontWidth = 0;
  double currentFontHeight = fontAscentMillimeters( mGridAnnotationFont );

  QList< QPair< double, QLineF > >::const_iterator vIt = vLines.constBegin();
  for ( ; vIt != vLines.constEnd(); ++vIt )
  {
    currentAnnotationString = QString::number( vIt->first );
    currentFontWidth = textWidthMillimeters( mGridAnnotationFont, currentAnnotationString );

    if ( mGridAnnotationPosition == OutsideMapFrame )
    {
      drawText( p, vIt->second.x1() - currentFontWidth / 2.0, vIt->second.y1() - mAnnotationFrameDistance, currentAnnotationString, mGridAnnotationFont );
      drawText( p, vIt->second.x2() - currentFontWidth / 2.0, vIt->second.y2() + mAnnotationFrameDistance + currentFontHeight, currentAnnotationString, mGridAnnotationFont );
    }
    else
    {
      drawText( p, vIt->second.x1() - currentFontWidth / 2.0, vIt->second.y1() + mAnnotationFrameDistance + currentFontHeight, currentAnnotationString, mGridAnnotationFont );
      drawText( p, vIt->second.x2() - currentFontWidth / 2.0, vIt->second.y2() - mAnnotationFrameDistance, currentAnnotationString, mGridAnnotationFont );
    }
  }

  QList< QPair< double, QLineF > >::const_iterator hIt = hLines.constBegin();
  for ( ; hIt != hLines.constEnd(); ++hIt )
  {
    currentAnnotationString = QString::number( hIt->first );
    currentFontWidth = textWidthMillimeters( mGridAnnotationFont, currentAnnotationString );

    if ( mGridAnnotationPosition == OutsideMapFrame )
    {
      drawText( p, hIt->second.x1() - ( mAnnotationFrameDistance + currentFontWidth ), hIt->second.y1() + currentFontHeight / 2.0, currentAnnotationString, mGridAnnotationFont );
      drawText( p, hIt->second.x2() + mAnnotationFrameDistance, hIt->second.y2() + currentFontHeight / 2.0, currentAnnotationString, mGridAnnotationFont );
    }
    else
    {
      drawText( p, hIt->second.x1() + mAnnotationFrameDistance, hIt->second.y1() + currentFontHeight / 2.0, currentAnnotationString, mGridAnnotationFont );
      drawText( p, hIt->second.x2() - ( mAnnotationFrameDistance + currentFontWidth ), hIt->second.y2() + currentFontHeight / 2.0, currentAnnotationString, mGridAnnotationFont );
    }
  }
}

int QgsComposerMap::verticalGridLines( QList< QPair< double, QLineF > >& lines ) const
{
  lines.clear();
  if ( !mGridIntervalX > 0.0 )
  {
    return 1;
  }

  //consider the possible shift in case of content move
  QgsRectangle mapExtent = transformedExtent();

  double currentLevel = ( int )(( mapExtent.xMinimum() - mGridOffsetX ) / mGridIntervalX + 1.0 ) * mGridIntervalX + mGridOffsetX;

  double xCanvasCoord;

  double border = 0.0;
  if ( frame() )
  {
    border = pen().widthF();
  }

  while ( currentLevel <= mapExtent.xMaximum() )
  {
    xCanvasCoord = rect().width() * ( currentLevel - mapExtent.xMinimum() ) / mapExtent.width();
    lines.push_back( qMakePair( currentLevel, QLineF( xCanvasCoord, border, xCanvasCoord, rect().height() - border ) ) );
    currentLevel += mGridIntervalX;
  }
  return 0;
}

int QgsComposerMap::horizontalGridLines( QList< QPair< double, QLineF > >& lines ) const
{
  lines.clear();
  if ( !mGridIntervalY > 0.0 )
  {
    return 1;
  }

  //consider the possible shift in case of content move
  QgsRectangle mapExtent = transformedExtent();

  double currentLevel = ( int )(( mapExtent.yMinimum() - mGridOffsetY ) / mGridIntervalY + 1.0 ) * mGridIntervalY + mGridOffsetY;
  double yCanvasCoord;

  double border = 0.0;
  if ( frame() )
  {
    border = pen().widthF();
  }

  while ( currentLevel <= mapExtent.yMaximum() )
  {
    yCanvasCoord = rect().height() * ( 1 - ( currentLevel - mapExtent.yMinimum() ) / mapExtent.height() );
    lines.push_back( qMakePair( currentLevel, QLineF( border, yCanvasCoord, rect().width() - border, yCanvasCoord ) ) );
    currentLevel += mGridIntervalY;
  }
  return 0;
}

void QgsComposerMap::setGridPenWidth( double w )
{
  mGridPen.setWidthF( w );
}

void QgsComposerMap::setGridPenColor( const QColor& c )
{
  mGridPen.setColor( c );
}

QRectF QgsComposerMap::boundingRect() const
{
  return mCurrentRectangle;
}

void QgsComposerMap::updateBoundingRect()
{
  QRectF rectangle = rect();
  double xExtension = maxExtensionXDirection();
  double yExtension = maxExtensionYDirection();
  rectangle.setLeft( rectangle.left() - xExtension );
  rectangle.setRight( rectangle.right() + xExtension );
  rectangle.setTop( rectangle.top() - yExtension );
  rectangle.setBottom( rectangle.bottom() + yExtension );
  if ( rectangle != mCurrentRectangle )
  {
    prepareGeometryChange();
    mCurrentRectangle = rectangle;
  }
}

QgsRectangle QgsComposerMap::transformedExtent() const
{
  double paperToMapFactor = mExtent.width() / rect().width();
  double xMapOffset = -mXOffset * paperToMapFactor;
  double yMapOffset = mYOffset * paperToMapFactor;
  return QgsRectangle( mExtent.xMinimum() + xMapOffset, mExtent.yMinimum() + yMapOffset, mExtent.xMaximum() + xMapOffset, mExtent.yMaximum() + yMapOffset );
}

double QgsComposerMap::maxExtensionXDirection() const
{
  if ( mGridAnnotationPosition != OutsideMapFrame )
  {
    return 0;
  }

  QList< QPair< double, QLineF > > horizontalLines;
  if ( horizontalGridLines( horizontalLines ) != 0 )
  {
    return 0;
  }

  double currentExtension = 0;
  double maxExtension = 0;
  QString currentAnnotationString;

  QList< QPair< double, QLineF > >::const_iterator hIt = horizontalLines.constBegin();
  for ( ; hIt != horizontalLines.constEnd(); ++hIt )
  {
    currentAnnotationString = QString::number( hIt->first );
    if ( mGridAnnotationDirection == Vertical )
    {
      currentExtension =  fontAscentMillimeters( mGridAnnotationFont ) + mAnnotationFrameDistance;
    }
    else
    {
      currentExtension = textWidthMillimeters( mGridAnnotationFont, currentAnnotationString ) + mAnnotationFrameDistance;
    }

    if ( currentExtension > maxExtension )
    {
      maxExtension = currentExtension;
    }
  }

  return maxExtension;
}

double QgsComposerMap::maxExtensionYDirection() const
{
  if ( mGridAnnotationPosition != OutsideMapFrame )
  {
    return 0;
  }

  QList< QPair< double, QLineF > > verticalLines;
  if ( verticalGridLines( verticalLines ) != 0 )
  {
    return 0;
  }

  double currentExtension = 0;
  double maxExtension = 0;
  QString currentAnnotationString;

  QList< QPair< double, QLineF > >::const_iterator vIt = verticalLines.constBegin();
  for ( ; vIt != verticalLines.constEnd(); ++vIt )
  {
    currentAnnotationString = QString::number( vIt->first );
    if ( mGridAnnotationDirection == Horizontal )
    {
      currentExtension = fontAscentMillimeters( mGridAnnotationFont ) + mAnnotationFrameDistance;
    }
    else
    {
      currentExtension = textWidthMillimeters( mGridAnnotationFont, currentAnnotationString ) + mAnnotationFrameDistance;
    }

    if ( currentExtension > maxExtension )
    {
      maxExtension = currentExtension;
    }
  }
  return maxExtension;
}
