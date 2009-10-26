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
    mGridIntervalX( 0.0 ), mGridIntervalY( 0.0 ), mGridOffsetX( 0.0 ), mGridOffsetY( 0.0 ), mGridAnnotationPrecision( 3 ), mShowGridAnnotation( false ), \
    mGridAnnotationPosition( OutsideMapFrame ), mAnnotationFrameDistance( 1.0 ), mGridAnnotationDirection( Horizontal ), \
    mRotation( 0 ), mCrossLength( 3 )
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
  mGridPen.setCapStyle( Qt::FlatCap );
}

QgsComposerMap::QgsComposerMap( QgsComposition *composition )
    : QgsComposerItem( 0, 0, 10, 10, composition ), mKeepLayerSet( false ), mGridEnabled( false ), mGridStyle( Solid ), \
    mGridIntervalX( 0.0 ), mGridIntervalY( 0.0 ), mGridOffsetX( 0.0 ), mGridOffsetY( 0.0 ), mGridAnnotationPrecision( 3 ), mShowGridAnnotation( false ), \
    mGridAnnotationPosition( OutsideMapFrame ), mAnnotationFrameDistance( 1.0 ), mGridAnnotationDirection( Horizontal ), \
    mRotation( 0 ), mCrossLength( 3 )
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
  mGridPen.setCapStyle( Qt::FlatCap );
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

  //layer caching (as QImages) cannot be done for composer prints
  QSettings s;
  bool bkLayerCaching = s.value( "/qgis/enable_render_caching", false ).toBool();
  s.setValue( "/qgis/enable_render_caching", false );

  theMapRenderer.render( painter );
  s.setValue( "/qgis/enable_render_caching", bkLayerCaching );

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

  //in case of rotation, we need to request a larger rectangle and create a larger cache image
  QgsRectangle requestExtent;
  requestedExtent( requestExtent );

  double horizontalVScaleFactor = horizontalViewScaleFactor();
  if ( horizontalVScaleFactor < 0 )
  {
    horizontalVScaleFactor = mLastValidViewScaleFactor;
  }

  int w = requestExtent.width() * mapUnitsToMM() * horizontalVScaleFactor;
  int h = requestExtent.height() * mapUnitsToMM() * horizontalVScaleFactor;

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
  QgsMapToPixel transform( mapUnitsPerPixel, h, requestExtent.yMinimum(), requestExtent.xMinimum() );

  QPainter p( &mCacheImage );

  draw( &p, requestExtent, QSize( w, h ), mCacheImage.logicalDpiX() );
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

    QgsRectangle requestRectangle;
    requestedExtent( requestRectangle );
    double horizontalVScaleFactor = horizontalViewScaleFactor();
    if ( horizontalVScaleFactor < 0 )
    {
      horizontalVScaleFactor = mLastValidViewScaleFactor;
    }

    double imagePixelWidth = mExtent.width() / requestRectangle.width() * mCacheImage.width() ; //how many pixels of the image are for the map extent?
    double scale = rect().width() / imagePixelWidth;
    QgsPoint rotationPoint = QgsPoint(( mExtent.xMaximum() + mExtent.xMinimum() ) / 2.0, ( mExtent.yMaximum() + mExtent.yMinimum() ) / 2.0 );

    //shift such that rotation point is at 0/0 point in the coordinate system
    double yShiftMM = ( requestRectangle.yMaximum() - rotationPoint.y() ) * mapUnitsToMM();
    double xShiftMM = ( requestRectangle.xMinimum() - rotationPoint.x() ) * mapUnitsToMM();

    //shift such that top left point of the extent at point 0/0 in item coordinate system
    double xTopLeftShift = ( rotationPoint.x() - mExtent.xMinimum() ) * mapUnitsToMM();
    double yTopLeftShift = ( mExtent.yMaximum() - rotationPoint.y() ) * mapUnitsToMM();

    painter->save();
    //painter->scale( scale, scale );
    painter->translate( mXOffset, mYOffset );
    painter->translate( xTopLeftShift, yTopLeftShift );
    painter->rotate( mRotation );
    painter->translate( xShiftMM, -yShiftMM );
    painter->scale( scale, scale );
    painter->drawImage( 0, 0, mCacheImage );
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

    QgsRectangle requestRectangle;
    requestedExtent( requestRectangle );

    QSize theSize( requestRectangle.width() * mapUnitsToMM(), requestRectangle.height() * mapUnitsToMM() );
    QgsPoint rotationPoint = QgsPoint(( mExtent.xMaximum() + mExtent.xMinimum() ) / 2.0, ( mExtent.yMaximum() + mExtent.yMinimum() ) / 2.0 );

    //shift such that rotation point is at 0/0 point in the coordinate system
    double yShiftMM = ( requestRectangle.yMaximum() - rotationPoint.y() ) * mapUnitsToMM();
    double xShiftMM = ( requestRectangle.xMinimum() - rotationPoint.x() ) * mapUnitsToMM();

    //shift such that top left point of the extent at point 0/0 in item coordinate system
    double xTopLeftShift = ( rotationPoint.x() - mExtent.xMinimum() ) * mapUnitsToMM();
    double yTopLeftShift = ( mExtent.yMaximum() - rotationPoint.y() ) * mapUnitsToMM();
    painter->save();
    painter->translate( mXOffset, mYOffset );
    painter->translate( xTopLeftShift, yTopLeftShift );
    painter->rotate( mRotation );
    painter->translate( xShiftMM, -yShiftMM );
    draw( painter, requestRectangle, theSize, 25.4 ); //scene coordinates seem to be in mm
    painter->restore();

    mDrawing = false;
  }

  painter->setClipRect( thisPaintRect , Qt::NoClip );

  if ( mGridEnabled )
  {
    drawGrid( painter );
  }
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
    transformShift( dx, dy );
    mExtent.setXMinimum( mExtent.xMinimum() + dx );
    mExtent.setXMaximum( mExtent.xMaximum() + dx );
    mExtent.setYMinimum( mExtent.yMinimum() + dy );
    mExtent.setYMaximum( mExtent.yMaximum() + dy );
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
  updateBoundingRect();
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
      QgsRasterLayer* currentRasterLayer = qobject_cast<QgsRasterLayer *>( currentLayer );
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
  //rotation
  composerMapElem.setAttribute( "rotation", mRotation );

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
  gridElem.setAttribute( "crossLength", mCrossLength );

  //grid annotation
  QDomElement annotationElem = doc.createElement( "Annotation" );
  annotationElem.setAttribute( "show", mShowGridAnnotation );
  annotationElem.setAttribute( "position", mGridAnnotationPosition );
  annotationElem.setAttribute( "frameDistance", mAnnotationFrameDistance );
  annotationElem.setAttribute( "direction", mGridAnnotationDirection );
  annotationElem.setAttribute( "font", mGridAnnotationFont.toString() );
  annotationElem.setAttribute( "precision", mGridAnnotationPrecision );

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

  //rotation
  mRotation = itemElem.attribute( "rotation", "0" ).toDouble();

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
    mCrossLength = gridElem.attribute( "crossLength", "3" ).toDouble();

    QDomNodeList annotationNodeList = gridElem.elementsByTagName( "Annotation" );
    if ( annotationNodeList.size() > 0 )
    {
      QDomElement annotationElem = annotationNodeList.at( 0 ).toElement();
      mShowGridAnnotation = ( annotationElem.attribute( "show", "0" ) != "0" );
      mGridAnnotationPosition = QgsComposerMap::GridAnnotationPosition( annotationElem.attribute( "position", "0" ).toInt() );
      mAnnotationFrameDistance = annotationElem.attribute( "frameDistance", "0" ).toDouble();
      mGridAnnotationDirection = QgsComposerMap::GridAnnotationDirection( annotationElem.attribute( "direction", "0" ).toInt() );
      mGridAnnotationFont.fromString( annotationElem.attribute( "font", "" ) );
      mGridAnnotationPrecision = annotationElem.attribute( "precision", "3" ).toInt();
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
  yGridLines( verticalLines );
  QList< QPair< double, QLineF > >::const_iterator vIt = verticalLines.constBegin();
  QList< QPair< double, QLineF > > horizontalLines;
  xGridLines( horizontalLines );
  QList< QPair< double, QLineF > >::const_iterator hIt = horizontalLines.constBegin();

  QRectF thisPaintRect = QRectF( 0, 0, QGraphicsRectItem::rect().width(), QGraphicsRectItem::rect().height() );
  p->setClipRect( thisPaintRect );

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
  else //cross
  {
    QPointF intersectionPoint, crossEnd1, crossEnd2;
    for ( ; vIt != verticalLines.constEnd(); ++vIt )
    {
      //start mark
      crossEnd1 = pointOnLineWithDistance( vIt->second.p1(), vIt->second.p2(), mCrossLength );
      p->drawLine( vIt->second.p1(), crossEnd1 );

      //test for intersection with every horizontal line
      hIt = horizontalLines.constBegin();
      for ( ; hIt != horizontalLines.constEnd(); ++hIt )
      {
        if ( hIt->second.intersect( vIt->second, &intersectionPoint ) == QLineF::BoundedIntersection )
        {
          crossEnd1 = pointOnLineWithDistance( intersectionPoint, vIt->second.p1(), mCrossLength );
          crossEnd2 = pointOnLineWithDistance( intersectionPoint, vIt->second.p2(), mCrossLength );
          p->drawLine( crossEnd1, crossEnd2 );
        }
      }
      //end mark
      QPointF crossEnd2 = pointOnLineWithDistance( vIt->second.p2(), vIt->second.p1(), mCrossLength );
      p->drawLine( vIt->second.p2(), crossEnd2 );
    }

    hIt = horizontalLines.constBegin();
    for ( ; hIt != horizontalLines.constEnd(); ++hIt )
    {
      //start mark
      crossEnd1 = pointOnLineWithDistance( hIt->second.p1(), hIt->second.p2(), mCrossLength );
      p->drawLine( hIt->second.p1(), crossEnd1 );

      vIt = verticalLines.constBegin();
      for ( ; vIt != verticalLines.constEnd(); ++vIt )
      {
        if ( vIt->second.intersect( hIt->second, &intersectionPoint ) == QLineF::BoundedIntersection )
        {
          crossEnd1 = pointOnLineWithDistance( intersectionPoint, hIt->second.p1(), mCrossLength );
          crossEnd2 = pointOnLineWithDistance( intersectionPoint, hIt->second.p2(), mCrossLength );
          p->drawLine( crossEnd1, crossEnd2 );
        }
      }
      //end mark
      crossEnd1 = pointOnLineWithDistance( hIt->second.p2(), hIt->second.p1(), mCrossLength );
      p->drawLine( hIt->second.p2(), crossEnd1 );
    }


  }

  p->setClipRect( thisPaintRect , Qt::NoClip );

  if ( mShowGridAnnotation )
  {
    drawCoordinateAnnotations( p, horizontalLines, verticalLines );
  }
}

void QgsComposerMap::drawCoordinateAnnotations( QPainter* p, const QList< QPair< double, QLineF > >& hLines, const QList< QPair< double, QLineF > >& vLines )
{
  if ( !p )
  {
    return;
  }


  QString currentAnnotationString;
  QList< QPair< double, QLineF > >::const_iterator it = hLines.constBegin();
  for ( ; it != hLines.constEnd(); ++it )
  {
    currentAnnotationString = QString::number( it->first, 'f', mGridAnnotationPrecision );
    drawCoordinateAnnotation( p, it->second.p1(), currentAnnotationString );
    drawCoordinateAnnotation( p, it->second.p2(), currentAnnotationString );
  }

  it = vLines.constBegin();
  for ( ; it != vLines.constEnd(); ++it )
  {
    currentAnnotationString = QString::number( it->first, 'f', mGridAnnotationPrecision );
    drawCoordinateAnnotation( p, it->second.p1(), currentAnnotationString );
    drawCoordinateAnnotation( p, it->second.p2(), currentAnnotationString );
  }
}

void QgsComposerMap::drawCoordinateAnnotation( QPainter* p, const QPointF& pos, QString annotationString )
{
  Border frameBorder = borderForLineCoord( pos );
  double textWidth = textWidthMillimeters( mGridAnnotationFont, annotationString );
  double textHeight = fontAscentMillimeters( mGridAnnotationFont );
  double xpos = pos.x();
  double ypos = pos.y();
  int rotation = 0;

  if ( frameBorder == Left )
  {

    if ( mGridAnnotationPosition == InsideMapFrame )
    {
      if ( mGridAnnotationDirection == Vertical || mGridAnnotationDirection == BoundaryDirection )
      {
        xpos -= textHeight + mAnnotationFrameDistance;
        ypos += textWidth / 2.0;
        rotation = 270;
      }
      else
      {
        xpos += mAnnotationFrameDistance;
        ypos += textHeight / 2.0;
      }
    }
    else //Outside map frame
    {
      if ( mGridAnnotationDirection == Vertical || mGridAnnotationDirection == BoundaryDirection )
      {
        xpos -= mAnnotationFrameDistance;
        ypos += textWidth / 2.0;
        rotation = 270;
      }
      else
      {
        xpos -= textWidth + mAnnotationFrameDistance;
        ypos += textHeight / 2.0;
      }
    }

  }
  else if ( frameBorder == Right )
  {
    if ( mGridAnnotationPosition == InsideMapFrame )
    {
      if ( mGridAnnotationDirection == Vertical || mGridAnnotationDirection == BoundaryDirection )
      {
        xpos -= mAnnotationFrameDistance;
        ypos += textWidth / 2.0;
        rotation = 270;
      }
      else //Horizontal
      {
        xpos -= textWidth + mAnnotationFrameDistance;
        ypos += textHeight / 2.0;
      }
    }
    else //OutsideMapFrame
    {
      if ( mGridAnnotationDirection == Vertical || mGridAnnotationDirection == BoundaryDirection )
      {
        xpos += textHeight + mAnnotationFrameDistance;
        ypos += textWidth / 2.0;
        rotation = 270;
      }
      else //Horizontal
      {
        xpos += mAnnotationFrameDistance;
        ypos += textHeight / 2.0;
      }
    }
  }
  else if ( frameBorder == Bottom )
  {
    if ( mGridAnnotationPosition == InsideMapFrame )
    {
      if ( mGridAnnotationDirection == Horizontal || mGridAnnotationDirection == BoundaryDirection )
      {
        ypos -= mAnnotationFrameDistance;
        xpos -= textWidth / 2.0;
      }
      else //Vertical
      {
        xpos += textHeight / 2.0;
        ypos -= mAnnotationFrameDistance;
        rotation = 270;
      }
    }
    else //OutsideMapFrame
    {
      if ( mGridAnnotationDirection == Horizontal || mGridAnnotationDirection == BoundaryDirection )
      {
        ypos += mAnnotationFrameDistance + textHeight;
        xpos -= textWidth / 2.0;
      }
      else //Vertical
      {
        xpos += textHeight / 2.0;
        ypos += textWidth + mAnnotationFrameDistance;
        rotation = 270;
      }
    }
  }
  else //Top
  {
    if ( mGridAnnotationPosition == InsideMapFrame )
    {
      if ( mGridAnnotationDirection == Horizontal || mGridAnnotationDirection == BoundaryDirection )
      {
        xpos -= textWidth / 2.0;
        ypos += textHeight + mAnnotationFrameDistance;
      }
      else //Vertical
      {
        xpos += textHeight / 2.0;
        ypos += textWidth + mAnnotationFrameDistance;
        rotation = 270;
      }
    }
    else //OutsideMapFrame
    {
      if ( mGridAnnotationDirection == Horizontal || mGridAnnotationDirection == BoundaryDirection )
      {
        xpos -= textWidth / 2.0;
        ypos -= mAnnotationFrameDistance;
      }
      else //Vertical
      {
        xpos += textHeight / 2.0;
        ypos -= mAnnotationFrameDistance;
        rotation = 270;
      }
    }
  }

  drawAnnotation( p, QPointF( xpos, ypos ), rotation, annotationString );
}

void QgsComposerMap::drawAnnotation( QPainter* p, const QPointF& pos, int rotation, const QString& annotationText )
{
  p->save();
  p->translate( pos );
  p->rotate( rotation );
  drawText( p, 0, 0, annotationText, mGridAnnotationFont );
  p->restore();
}

int QgsComposerMap::xGridLines( QList< QPair< double, QLineF > >& lines ) const
{
  lines.clear();
  if ( mGridIntervalY <= 0.0 )
  {
    return 1;
  }

  QPolygonF mapPolygon = transformedMapPolygon();
  QRectF mapBoundingRect = mapPolygon.boundingRect();
  double currentLevel = ( int )(( mapBoundingRect.top() - mGridOffsetY ) / mGridIntervalY + 1.0 ) * mGridIntervalY + mGridOffsetY;

  if ( mRotation <= 0.0 )
  {
    //no rotation. Do it 'the easy way'

    double yCanvasCoord;

    while ( currentLevel <= mapBoundingRect.bottom() )
    {
      yCanvasCoord = rect().height() * ( 1 - ( currentLevel - mapBoundingRect.top() ) / mapBoundingRect.height() );
      lines.push_back( qMakePair( currentLevel, QLineF( 0, yCanvasCoord, rect().width(), yCanvasCoord ) ) );
      currentLevel += mGridIntervalY;
    }
  }

  //the four border lines
  QVector<QLineF> borderLines;
  borderLines << QLineF( mapPolygon.at( 0 ), mapPolygon.at( 1 ) );
  borderLines << QLineF( mapPolygon.at( 1 ), mapPolygon.at( 2 ) );
  borderLines << QLineF( mapPolygon.at( 2 ), mapPolygon.at( 3 ) );
  borderLines << QLineF( mapPolygon.at( 3 ), mapPolygon.at( 0 ) );

  QList<QPointF> intersectionList; //intersects between border lines and grid lines

  while ( currentLevel <= mapBoundingRect.bottom() )
  {
    intersectionList.clear();
    QLineF gridLine( mapBoundingRect.left(), currentLevel, mapBoundingRect.right(), currentLevel );

    QVector<QLineF>::const_iterator it = borderLines.constBegin();
    for ( ; it != borderLines.constEnd(); ++it )
    {
      QPointF intersectionPoint;
      if ( it->intersect( gridLine, &intersectionPoint ) == QLineF::BoundedIntersection )
      {
        intersectionList.push_back( intersectionPoint );
        if ( intersectionList.size() >= 2 )
        {
          break; //we already have two intersections, skip further tests
        }
      }
    }

    if ( intersectionList.size() >= 2 )
    {
      lines.push_back( qMakePair( currentLevel, QLineF( mapToItemCoords( intersectionList.at( 0 ) ), mapToItemCoords( intersectionList.at( 1 ) ) ) ) );
    }
    currentLevel += mGridIntervalY;
  }


  return 0;
}

int QgsComposerMap::yGridLines( QList< QPair< double, QLineF > >& lines ) const
{
  lines.clear();
  if ( mGridIntervalX <= 0.0 )
  {
    return 1;
  }

  QPolygonF mapPolygon = transformedMapPolygon();
  QRectF mapBoundingRect = mapPolygon.boundingRect();
  double currentLevel = ( int )(( mapBoundingRect.left() - mGridOffsetX ) / mGridIntervalX + 1.0 ) * mGridIntervalX + mGridOffsetX;

  if ( mRotation <= 0.0 )
  {
    //no rotation. Do it 'the easy way'
    double xCanvasCoord;

    while ( currentLevel <= mapBoundingRect.right() )
    {
      xCanvasCoord = rect().width() * ( currentLevel - mapBoundingRect.left() ) / mapBoundingRect.width();
      lines.push_back( qMakePair( currentLevel, QLineF( xCanvasCoord, 0, xCanvasCoord, rect().height() ) ) );
      currentLevel += mGridIntervalX;
    }
  }

  //the four border lines
  QVector<QLineF> borderLines;
  borderLines << QLineF( mapPolygon.at( 0 ), mapPolygon.at( 1 ) );
  borderLines << QLineF( mapPolygon.at( 1 ), mapPolygon.at( 2 ) );
  borderLines << QLineF( mapPolygon.at( 2 ), mapPolygon.at( 3 ) );
  borderLines << QLineF( mapPolygon.at( 3 ), mapPolygon.at( 0 ) );

  QList<QPointF> intersectionList; //intersects between border lines and grid lines

  while ( currentLevel <= mapBoundingRect.right() )
  {
    intersectionList.clear();
    QLineF gridLine( currentLevel, mapBoundingRect.bottom(), currentLevel, mapBoundingRect.top() );

    QVector<QLineF>::const_iterator it = borderLines.constBegin();
    for ( ; it != borderLines.constEnd(); ++it )
    {
      QPointF intersectionPoint;
      if ( it->intersect( gridLine, &intersectionPoint ) == QLineF::BoundedIntersection )
      {
        intersectionList.push_back( intersectionPoint );
        if ( intersectionList.size() >= 2 )
        {
          break; //we already have two intersections, skip further tests
        }
      }
    }

    if ( intersectionList.size() >= 2 )
    {
      lines.push_back( qMakePair( currentLevel, QLineF( mapToItemCoords( intersectionList.at( 0 ) ), mapToItemCoords( intersectionList.at( 1 ) ) ) ) );
    }
    currentLevel += mGridIntervalX;
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
  double extension = maxExtension();
  rectangle.setLeft( rectangle.left() - extension );
  rectangle.setRight( rectangle.right() + extension );
  rectangle.setTop( rectangle.top() - extension );
  rectangle.setBottom( rectangle.bottom() + extension );
  if ( rectangle != mCurrentRectangle )
  {
    prepareGeometryChange();
    mCurrentRectangle = rectangle;
  }
}

QgsRectangle QgsComposerMap::transformedExtent() const
{
  double dx = mXOffset;
  double dy = mYOffset;
  transformShift( dx, dy );
  return QgsRectangle( mExtent.xMinimum() - dx, mExtent.yMinimum() - dy, mExtent.xMaximum() - dx, mExtent.yMaximum() - dy );
}

QPolygonF QgsComposerMap::transformedMapPolygon() const
{
  double dx = mXOffset;
  double dy = mYOffset;
  //qWarning("offset");
  //qWarning(QString::number(dx).toLocal8Bit().data());
  //qWarning(QString::number(dy).toLocal8Bit().data());
  transformShift( dx, dy );
  //qWarning("transformed:");
  //qWarning(QString::number(dx).toLocal8Bit().data());
  //qWarning(QString::number(dy).toLocal8Bit().data());
  QPolygonF poly;
  mapPolygon( poly );
  poly.translate( -dx, -dy );
  return poly;
}

double QgsComposerMap::maxExtension() const
{
  if ( !mGridEnabled || !mShowGridAnnotation || mGridAnnotationPosition != OutsideMapFrame )
  {
    return 0;
  }

  QList< QPair< double, QLineF > > xLines;
  QList< QPair< double, QLineF > > yLines;

  if ( xGridLines( xLines ) != 0 )
  {
    return 0;
  }

  if ( yGridLines( yLines ) != 0 )
  {
    return 0;
  }

  double maxExtension = 0;
  double currentExtension = 0;
  QString currentAnnotationString;

  QList< QPair< double, QLineF > >::const_iterator it = xLines.constBegin();
  for ( ; it != xLines.constEnd(); ++it )
  {
    currentAnnotationString = QString::number( it->first );
    currentExtension = std::max( textWidthMillimeters( mGridAnnotationFont, currentAnnotationString ), fontAscentMillimeters( mGridAnnotationFont ) );
    maxExtension = std::max( maxExtension, currentExtension );
  }

  it = yLines.constBegin();
  for ( ; it != yLines.constEnd(); ++it )
  {
    currentAnnotationString = QString::number( it->first );
    currentExtension = std::max( textWidthMillimeters( mGridAnnotationFont, currentAnnotationString ), fontAscentMillimeters( mGridAnnotationFont ) );
    maxExtension = std::max( maxExtension, currentExtension );
  }

  return maxExtension + mAnnotationFrameDistance;
}

void QgsComposerMap::mapPolygon( QPolygonF& poly ) const
{
  poly.clear();
  if ( mRotation == 0 )
  {
    poly << QPointF( mExtent.xMinimum(), mExtent.yMaximum() );
    poly << QPointF( mExtent.xMaximum(), mExtent.yMaximum() );
    poly << QPointF( mExtent.xMaximum(), mExtent.yMinimum() );
    poly << QPointF( mExtent.xMinimum(), mExtent.yMinimum() );
    return;
  }

  //there is rotation
  QgsPoint rotationPoint(( mExtent.xMaximum() + mExtent.xMinimum() ) / 2.0, ( mExtent.yMaximum() + mExtent.yMinimum() ) / 2.0 );
  double dx, dy; //x-, y- shift from rotation point to corner point

  //top left point
  dx = rotationPoint.x() - mExtent.xMinimum();
  dy = rotationPoint.y() - mExtent.yMaximum();
  rotate( mRotation, dx, dy );
  poly << QPointF( rotationPoint.x() + dx, rotationPoint.y() + dy );

  //top right point
  dx = rotationPoint.x() - mExtent.xMaximum();
  dy = rotationPoint.y() - mExtent.yMaximum();
  rotate( mRotation, dx, dy );
  poly << QPointF( rotationPoint.x() + dx, rotationPoint.y() + dy );

  //bottom right point
  dx = rotationPoint.x() - mExtent.xMaximum();
  dy = rotationPoint.y() - mExtent.yMinimum();
  rotate( mRotation, dx, dy );
  poly << QPointF( rotationPoint.x() + dx, rotationPoint.y() + dy );

  //bottom left point
  dx = rotationPoint.x() - mExtent.xMinimum();
  dy = rotationPoint.y() - mExtent.yMinimum();
  rotate( mRotation, dx, dy );
  poly << QPointF( rotationPoint.x() + dx, rotationPoint.y() + dy );
}

void QgsComposerMap::requestedExtent( QgsRectangle& extent ) const
{
  if ( mRotation == 0 )
  {
    extent = mExtent;
    return;
  }

  QPolygonF poly;
  mapPolygon( poly );
  QRectF bRect = poly.boundingRect();
  extent.setXMinimum( bRect.left() );
  extent.setXMaximum( bRect.right() );
  extent.setYMinimum( bRect.top() );
  extent.setYMaximum( bRect.bottom() );
  return;
}

double QgsComposerMap::mapUnitsToMM() const
{
  double extentWidth = mExtent.width();
  if ( extentWidth <= 0 )
  {
    return 1;
  }
  return rect().width() / extentWidth;
}

void QgsComposerMap::transformShift( double& xShift, double& yShift ) const
{
  double mmToMapUnits = 1.0 / mapUnitsToMM();
  double dxScaled = xShift * mmToMapUnits;
  double dyScaled = - yShift * mmToMapUnits;

  rotate( mRotation, dxScaled, dyScaled );

  xShift = dxScaled;
  yShift = dyScaled;
}

QPointF QgsComposerMap::mapToItemCoords( const QPointF& mapCoords ) const
{
  QPolygonF mapPoly = transformedMapPolygon();
  if ( mapPoly.size() < 1 )
  {
    return QPointF( 0, 0 );
  }

  QgsRectangle tExtent = transformedExtent();
  QgsPoint rotationPoint(( tExtent.xMaximum() + tExtent.xMinimum() ) / 2.0, ( tExtent.yMaximum() + tExtent.yMinimum() ) / 2.0 );
  double dx = mapCoords.x() - rotationPoint.x();
  double dy = mapCoords.y() - rotationPoint.y();
  rotate( -mRotation, dx, dy );
  QgsPoint backRotatedCoords( rotationPoint.x() + dx, rotationPoint.y() + dy );

  QgsRectangle unrotatedExtent = transformedExtent();
  double xItem = rect().width() * ( backRotatedCoords.x() - unrotatedExtent.xMinimum() ) / unrotatedExtent.width();
  double yItem = rect().height() * ( 1 - ( backRotatedCoords.y() - unrotatedExtent.yMinimum() ) / unrotatedExtent.height() );
  return QPointF( xItem, yItem );
}

QgsComposerMap::Border QgsComposerMap::borderForLineCoord( const QPointF& p ) const
{
  if ( p.x() <= pen().widthF() )
  {
    return Left;
  }
  else if ( p.x() >= ( rect().width() - pen().widthF() ) )
  {
    return Right;
  }
  else if ( p.y() <= pen().widthF() )
  {
    return Top;
  }
  else
  {
    return Bottom;
  }
}

void QgsComposerMap::rotate( double angle, double& x, double& y ) const
{
  double rotToRad = angle * M_PI / 180.0;
  double xRot, yRot;
  xRot = x * cos( rotToRad ) - y * sin( rotToRad );
  yRot = x * sin( rotToRad ) + y * cos( rotToRad );
  x = xRot;
  y = yRot;
}

QPointF QgsComposerMap::pointOnLineWithDistance( const QPointF& startPoint, const QPointF& directionPoint, double distance ) const
{
  double dx = directionPoint.x() - startPoint.x();
  double dy = directionPoint.y() - startPoint.y();
  double length = sqrt( dx * dx + dy * dy );
  double scaleFactor = distance / length;
  return QPointF( startPoint.x() + dx * scaleFactor, startPoint.y() + dy * scaleFactor );
}
