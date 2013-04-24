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
#include "qgscomposition.h"
#include "qgscoordinatetransform.h"
#include "qgslogger.h"
#include "qgsmaprenderer.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaptopixel.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgsrendercontext.h"
#include "qgsscalecalculator.h"
#include "qgsvectorlayer.h"

#include "qgslabel.h"
#include "qgslabelattributes.h"
#include "qgssymbollayerv2utils.h" //for pointOnLineWithDistance

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPainter>
#include <QSettings>
#include <cmath>

QgsComposerMap::QgsComposerMap( QgsComposition *composition, int x, int y, int width, int height )
    : QgsComposerItem( x, y, width, height, composition ), mKeepLayerSet( false ),
    mOverviewFrameMapId( -1 ), mGridEnabled( false ), mGridStyle( Solid ),
    mGridIntervalX( 0.0 ), mGridIntervalY( 0.0 ), mGridOffsetX( 0.0 ), mGridOffsetY( 0.0 ), mGridAnnotationPrecision( 3 ), mShowGridAnnotation( false ),
    mLeftGridAnnotationPosition( OutsideMapFrame ), mRightGridAnnotationPosition( OutsideMapFrame ), mTopGridAnnotationPosition( OutsideMapFrame ),
    mBottomGridAnnotationPosition( OutsideMapFrame ), mAnnotationFrameDistance( 1.0 ), mLeftGridAnnotationDirection( Horizontal ), mRightGridAnnotationDirection( Horizontal ),
    mTopGridAnnotationDirection( Horizontal ), mBottomGridAnnotationDirection( Horizontal ), mGridFrameStyle( NoGridFrame ), mGridFrameWidth( 2.0 ),
    mCrossLength( 3 ), mMapCanvas( 0 ), mDrawCanvasItems( true )
{
  mComposition = composition;
  mOverviewFrameMapSymbol = 0;
  mGridLineSymbol = 0;
  createDefaultOverviewFrameSymbol();
  createDefaultGridLineSymbol();

  mId = 0;
  assignFreeId();

  mMapRenderer = mComposition->mapRenderer();
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

  initGridAnnotationFormatFromProject();
}

QgsComposerMap::QgsComposerMap( QgsComposition *composition )
    : QgsComposerItem( 0, 0, 10, 10, composition ), mKeepLayerSet( false ), mOverviewFrameMapId( -1 ), mGridEnabled( false ), mGridStyle( Solid ),
    mGridIntervalX( 0.0 ), mGridIntervalY( 0.0 ), mGridOffsetX( 0.0 ), mGridOffsetY( 0.0 ), mGridAnnotationPrecision( 3 ), mShowGridAnnotation( false ),
    mLeftGridAnnotationPosition( OutsideMapFrame ), mRightGridAnnotationPosition( OutsideMapFrame ), mTopGridAnnotationPosition( OutsideMapFrame ),
    mBottomGridAnnotationPosition( OutsideMapFrame ), mAnnotationFrameDistance( 1.0 ), mLeftGridAnnotationDirection( Horizontal ), mRightGridAnnotationDirection( Horizontal ),
    mTopGridAnnotationDirection( Horizontal ), mBottomGridAnnotationDirection( Horizontal ), mGridFrameStyle( NoGridFrame ), mGridFrameWidth( 2.0 ), mCrossLength( 3 ),
    mMapCanvas( 0 ), mDrawCanvasItems( true )
{
  mOverviewFrameMapSymbol = 0;
  mGridLineSymbol = 0;
  createDefaultOverviewFrameSymbol();

  //Offset
  mXOffset = 0.0;
  mYOffset = 0.0;

  connectUpdateSlot();

  mComposition = composition;
  mMapRenderer = mComposition->mapRenderer();
  mId = mComposition->composerMapItems().size();
  mPreviewMode = QgsComposerMap::Rectangle;
  mCurrentRectangle = rect();

  setToolTip( tr( "Map %1" ).arg( mId ) );

  initGridAnnotationFormatFromProject();
}

QgsComposerMap::~QgsComposerMap()
{
  delete mOverviewFrameMapSymbol;
  delete mGridLineSymbol;
}

/* This function is called by paint() and cache() to render the map.  It does not override any functions
from QGraphicsItem. */
void QgsComposerMap::draw( QPainter *painter, const QgsRectangle& extent, const QSizeF& size, double dpi, double* forceWidthScale )
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
  if ( mMapRenderer->labelingEngine() )
    theMapRenderer.setLabelingEngine( mMapRenderer->labelingEngine()->clone() );

  //use stored layer set or read current set from main canvas
  if ( mKeepLayerSet )
  {
    theMapRenderer.setLayerSet( mLayerSet );
  }
  else
  {
    theMapRenderer.setLayerSet( mMapRenderer->layerSet() );
  }
  theMapRenderer.setDestinationCrs( mMapRenderer->destinationCrs() );
  theMapRenderer.setProjectionsEnabled( mMapRenderer->hasCrsTransformEnabled() );

  //set antialiasing if enabled in options
  QSettings settings;
  // Changed to enable anti aliased rendering by default as of QGIS 1.7
  if ( settings.value( "/qgis/enable_anti_aliasing", true ).toBool() )
  {
    painter->setRenderHint( QPainter::Antialiasing );
  }

  QgsRenderContext* theRendererContext = theMapRenderer.rendererContext();
  if ( theRendererContext )
  {
    theRendererContext->setDrawEditingInformation( false );
    theRendererContext->setRenderingStopped( false );
  }

  // force vector output (no caching of marker images etc.)
  theRendererContext->setForceVectorOutput( true );

  //force composer map scale for scale dependent visibility
  double bk_scale = theMapRenderer.scale();
  theMapRenderer.setScale( scale() );

  //layer caching (as QImages) cannot be done for composer prints
  QSettings s;
  bool bkLayerCaching = s.value( "/qgis/enable_render_caching", false ).toBool();
  s.setValue( "/qgis/enable_render_caching", false );

  if ( forceWidthScale ) //force wysiwyg line widths / marker sizes
  {
    theMapRenderer.render( painter, forceWidthScale );
  }
  else
  {
    theMapRenderer.render( painter );
  }
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

  double forcedWidthScaleFactor = w / requestExtent.width() / mapUnitsToMM();

  mCacheImage = QImage( w, h,  QImage::Format_ARGB32 );

  if ( hasBackground() )
  {
    //Initially fill image with specified background color. This ensures that layers with blend modes will
    //preview correctly
    mCacheImage.fill( backgroundColor().rgba() );
  }
  else
  {
    //no background, but start with empty fill to avoid artifacts
    mCacheImage.fill( QColor( 255, 255, 255, 0 ).rgba() );
  }

  double mapUnitsPerPixel = mExtent.width() / w;

  // WARNING: ymax in QgsMapToPixel is device height!!!
  QgsMapToPixel transform( mapUnitsPerPixel, h, requestExtent.yMinimum(), requestExtent.xMinimum() );

  QPainter p( &mCacheImage );

  draw( &p, requestExtent, QSizeF( w, h ), mCacheImage.logicalDpiX(), &forcedWidthScaleFactor );
  p.end();
  mCacheUpdated = true;

  mDrawing = false;
}

void QgsComposerMap::paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget )
{
  Q_UNUSED( pWidget );

  if ( !mComposition || !painter )
  {
    return;
  }

  QRectF thisPaintRect = QRectF( 0, 0, QGraphicsRectItem::rect().width(), QGraphicsRectItem::rect().height() );
  painter->save();
  painter->setClipRect( thisPaintRect );

  if ( mComposition->plotStyle() == QgsComposition::Preview && mPreviewMode == Rectangle )
  {
    // Fill with background color
    drawBackground( painter );
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
    
    //Background color is already included in cached image, so no need to draw

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

    painter->translate( mXOffset, mYOffset );
    painter->translate( xTopLeftShift, yTopLeftShift );
    painter->rotate( mRotation );
    painter->translate( xShiftMM, -yShiftMM );
    painter->scale( scale, scale );
    painter->drawImage( 0, 0, mCacheImage );

    //restore rotation
    painter->restore();

    //draw canvas items
    drawCanvasItems( painter, itemStyle );
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

    // Fill with background color
    drawBackground( painter );

    QgsRectangle requestRectangle;
    requestedExtent( requestRectangle );

    QSizeF theSize( requestRectangle.width() * mapUnitsToMM(), requestRectangle.height() * mapUnitsToMM() );
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

    //restore rotation
    painter->restore();

    //draw canvas items
    drawCanvasItems( painter, itemStyle );

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

  if ( mOverviewFrameMapId != -1 )
  {
    drawOverviewMapExtent( painter );
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
  updateItem();
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
    cache();
    update();
    emit itemChanged();
    emit extentChanged();
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
  int zoomMode = settings.value( "/qgis/wheel_action", 2 ).toInt();
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

  cache();
  update();
  emit itemChanged();
  emit extentChanged();
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

  updateBoundingRect();
  update();
  emit itemChanged();
  emit extentChanged();
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
  updateItem();
}

void QgsComposerMap::setNewScale( double scaleDenominator )
{
  double currentScaleDenominator = scale();

  if ( scaleDenominator == currentScaleDenominator )
  {
    return;
  }

  double scaleRatio = scaleDenominator / currentScaleDenominator;
  mExtent.scale( scaleRatio );
  mCacheUpdated = false;
  cache();
  update();
  emit itemChanged();
  emit extentChanged();
}

void QgsComposerMap::setPreviewMode( PreviewMode m )
{
  mPreviewMode = m;
  emit itemChanged();
}

void QgsComposerMap::setOffset( double xOffset, double yOffset )
{
  mXOffset = xOffset;
  mYOffset = yOffset;
}

void QgsComposerMap::setMapRotation( double r )
{
  setRotation( r );
  emit rotationChanged( r );
  emit itemChanged();
}

void QgsComposerMap::updateItem()
{
  if ( mPreviewMode != QgsComposerMap::Rectangle &&  !mCacheUpdated )
  {
    cache();
  }
  QgsComposerItem::updateItem();
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
  composerMapElem.setAttribute( "id", mId );

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

  if ( mDrawCanvasItems )
  {
    composerMapElem.setAttribute( "drawCanvasItems", "true" );
  }
  else
  {
    composerMapElem.setAttribute( "drawCanvasItems", "false" );
  }

  //overview map frame
  QDomElement overviewFrameElem = doc.createElement( "overviewFrame" );
  overviewFrameElem.setAttribute( "overviewFrameMap", mOverviewFrameMapId );
  QDomElement overviewFrameStyleElem = QgsSymbolLayerV2Utils::saveSymbol( QString(), mOverviewFrameMapSymbol, doc );
  overviewFrameElem.appendChild( overviewFrameStyleElem );
  composerMapElem.appendChild( overviewFrameElem );

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

  //overview map frame
  composerMapElem.setAttribute( "overviewFrameMap", mOverviewFrameMapId );

  //grid
  QDomElement gridElem = doc.createElement( "Grid" );
  gridElem.setAttribute( "show", mGridEnabled );
  gridElem.setAttribute( "gridStyle", mGridStyle );
  gridElem.setAttribute( "intervalX", QString::number( mGridIntervalX ) );
  gridElem.setAttribute( "intervalY", QString::number( mGridIntervalY ) );
  gridElem.setAttribute( "offsetX", QString::number( mGridOffsetX ) );
  gridElem.setAttribute( "offsetY", QString::number( mGridOffsetY ) );
  gridElem.setAttribute( "crossLength",  QString::number( mCrossLength ) );
  gridElem.setAttribute( "gridFrameStyle", mGridFrameStyle );
  gridElem.setAttribute( "gridFrameWidth", QString::number( mGridFrameWidth ) );
  QDomElement gridLineStyleElem = QgsSymbolLayerV2Utils::saveSymbol( QString(), mGridLineSymbol, doc );
  gridElem.appendChild( gridLineStyleElem );

  //grid annotation
  QDomElement annotationElem = doc.createElement( "Annotation" );
  annotationElem.setAttribute( "format", mGridAnnotationFormat );
  annotationElem.setAttribute( "show", mShowGridAnnotation );
  annotationElem.setAttribute( "leftPosition", mLeftGridAnnotationPosition );
  annotationElem.setAttribute( "rightPosition", mRightGridAnnotationPosition );
  annotationElem.setAttribute( "topPosition", mTopGridAnnotationPosition );
  annotationElem.setAttribute( "bottomPosition", mBottomGridAnnotationPosition );
  annotationElem.setAttribute( "leftDirection", mLeftGridAnnotationDirection );
  annotationElem.setAttribute( "rightDirection", mRightGridAnnotationDirection );
  annotationElem.setAttribute( "topDirection", mTopGridAnnotationDirection );
  annotationElem.setAttribute( "bottomDirection", mBottomGridAnnotationDirection );
  annotationElem.setAttribute( "frameDistance",  QString::number( mAnnotationFrameDistance ) );
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

  QString idRead = itemElem.attribute( "id", "not found" );
  if ( idRead != "not found" )
  {
    mId = idRead.toInt();
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

  QDomElement overviewFrameElem = itemElem.firstChildElement( "overviewFrame" );
  if ( !overviewFrameElem.isNull() )
  {
    setOverviewFrameMap( overviewFrameElem.attribute( "overviewFrameMap", "-1" ).toInt() );
    QDomElement overviewFrameSymbolElem = overviewFrameElem.firstChildElement( "symbol" );
    if ( !overviewFrameSymbolElem.isNull() )
    {
      delete mOverviewFrameMapSymbol;
      mOverviewFrameMapSymbol = dynamic_cast<QgsFillSymbolV2*>( QgsSymbolLayerV2Utils::loadSymbol( overviewFrameSymbolElem ) );
    }
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

  QString drawCanvasItemsFlag = itemElem.attribute( "drawCanvasItems" );
  if ( drawCanvasItemsFlag.compare( "true", Qt::CaseInsensitive ) == 0 )
  {
    mDrawCanvasItems = true;
  }
  else
  {
    mDrawCanvasItems = false;
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
    mCrossLength = gridElem.attribute( "crossLength", "3" ).toDouble();
    mGridFrameStyle = ( QgsComposerMap::GridFrameStyle )gridElem.attribute( "gridFrameStyle", "0" ).toInt();
    mGridFrameWidth = gridElem.attribute( "gridFrameWidth", "2.0" ).toDouble();

    QDomElement gridSymbolElem = gridElem.firstChildElement( "symbol" );
    delete mGridLineSymbol;
    if ( gridSymbolElem.isNull( ) )
    {
      //old project file, read penWidth /penColorRed, penColorGreen, penColorBlue
      mGridLineSymbol = QgsLineSymbolV2::createSimple( QgsStringMap() );
      mGridLineSymbol->setWidth( gridElem.attribute( "penWidth", "0" ).toDouble() );
      mGridLineSymbol->setColor( QColor( gridElem.attribute( "penColorRed", "0" ).toInt(),
                                         gridElem.attribute( "penColorGreen", "0" ).toInt(),
                                         gridElem.attribute( "penColorBlue", "0" ).toInt() ) );
    }
    else
    {
      mGridLineSymbol = dynamic_cast<QgsLineSymbolV2*>( QgsSymbolLayerV2Utils::loadSymbol( gridSymbolElem ) );
    }

    QDomNodeList annotationNodeList = gridElem.elementsByTagName( "Annotation" );
    if ( annotationNodeList.size() > 0 )
    {
      QDomElement annotationElem = annotationNodeList.at( 0 ).toElement();
      mShowGridAnnotation = ( annotationElem.attribute( "show", "0" ) != "0" );
      mGridAnnotationFormat = QgsComposerMap::GridAnnotationFormat( annotationElem.attribute( "format", "0" ).toInt() );
      mLeftGridAnnotationPosition = QgsComposerMap::GridAnnotationPosition( annotationElem.attribute( "leftPosition", "0" ).toInt() );
      mRightGridAnnotationPosition = QgsComposerMap::GridAnnotationPosition( annotationElem.attribute( "rightPosition", "0" ).toInt() );
      mTopGridAnnotationPosition = QgsComposerMap::GridAnnotationPosition( annotationElem.attribute( "topPosition", "0" ).toInt() );
      mBottomGridAnnotationPosition = QgsComposerMap::GridAnnotationPosition( annotationElem.attribute( "bottomPosition", "0" ).toInt() );
      mLeftGridAnnotationDirection = QgsComposerMap::GridAnnotationDirection( annotationElem.attribute( "leftDirection", "0" ).toInt() );
      mRightGridAnnotationDirection = QgsComposerMap::GridAnnotationDirection( annotationElem.attribute( "rightDirection", "0" ).toInt() );
      mTopGridAnnotationDirection = QgsComposerMap::GridAnnotationDirection( annotationElem.attribute( "topDirection", "0" ).toInt() );
      mBottomGridAnnotationDirection = QgsComposerMap::GridAnnotationDirection( annotationElem.attribute( "bottomDirection", "0" ).toInt() );
      mAnnotationFrameDistance = annotationElem.attribute( "frameDistance", "0" ).toDouble();
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
  emit itemChanged();
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

  //if layer set is fixed, do a lookup in the layer registry to also find the non-visible layers
  QStringList currentLayerSet;
  if ( mKeepLayerSet )
  {
    currentLayerSet = QgsMapLayerRegistry::instance()->mapLayers().uniqueKeys();
  }
  else //only consider layers visible in the map
  {
    currentLayerSet = mMapRenderer->layerSet();
  }

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
      drawGridLine( vIt->second, p );
    }

    for ( ; hIt != horizontalLines.constEnd(); ++hIt )
    {
      drawGridLine( hIt->second, p );
    }
  }
  else //cross
  {
    QPointF intersectionPoint, crossEnd1, crossEnd2;
    for ( ; vIt != verticalLines.constEnd(); ++vIt )
    {
      //start mark
      crossEnd1 = QgsSymbolLayerV2Utils::pointOnLineWithDistance( vIt->second.p1(), vIt->second.p2(), mCrossLength );
      drawGridLine( QLineF( vIt->second.p1(), crossEnd1 ), p );

      //test for intersection with every horizontal line
      hIt = horizontalLines.constBegin();
      for ( ; hIt != horizontalLines.constEnd(); ++hIt )
      {
        if ( hIt->second.intersect( vIt->second, &intersectionPoint ) == QLineF::BoundedIntersection )
        {
          crossEnd1 = QgsSymbolLayerV2Utils::pointOnLineWithDistance( intersectionPoint, vIt->second.p1(), mCrossLength );
          crossEnd2 = QgsSymbolLayerV2Utils::pointOnLineWithDistance( intersectionPoint, vIt->second.p2(), mCrossLength );
          drawGridLine( QLineF( crossEnd1, crossEnd2 ), p );
        }
      }
      //end mark
      QPointF crossEnd2 = QgsSymbolLayerV2Utils::pointOnLineWithDistance( vIt->second.p2(), vIt->second.p1(), mCrossLength );
      drawGridLine( QLineF( vIt->second.p2(), crossEnd2 ), p );
    }

    hIt = horizontalLines.constBegin();
    for ( ; hIt != horizontalLines.constEnd(); ++hIt )
    {
      //start mark
      crossEnd1 = QgsSymbolLayerV2Utils::pointOnLineWithDistance( hIt->second.p1(), hIt->second.p2(), mCrossLength );
      drawGridLine( QLineF( hIt->second.p1(), crossEnd1 ), p );

      vIt = verticalLines.constBegin();
      for ( ; vIt != verticalLines.constEnd(); ++vIt )
      {
        if ( vIt->second.intersect( hIt->second, &intersectionPoint ) == QLineF::BoundedIntersection )
        {
          crossEnd1 = QgsSymbolLayerV2Utils::pointOnLineWithDistance( intersectionPoint, hIt->second.p1(), mCrossLength );
          crossEnd2 = QgsSymbolLayerV2Utils::pointOnLineWithDistance( intersectionPoint, hIt->second.p2(), mCrossLength );
          drawGridLine( QLineF( crossEnd1, crossEnd2 ), p );
        }
      }
      //end mark
      crossEnd1 = QgsSymbolLayerV2Utils::pointOnLineWithDistance( hIt->second.p2(), hIt->second.p1(), mCrossLength );
      drawGridLine( QLineF( hIt->second.p2(), crossEnd1 ), p );
    }
  }

  p->setClipRect( thisPaintRect , Qt::NoClip );

  if ( mGridFrameStyle != QgsComposerMap::NoGridFrame )
  {
    drawGridFrame( p, horizontalLines, verticalLines );
  }

  if ( mShowGridAnnotation )
  {
    drawCoordinateAnnotations( p, horizontalLines, verticalLines );
  }
}

void QgsComposerMap::drawGridFrame( QPainter* p, const QList< QPair< double, QLineF > >& hLines, const QList< QPair< double, QLineF > >& vLines )
{
  //Sort the coordinate positions for each side
  QMap< double, double > leftGridFrame;
  QMap< double, double > rightGridFrame;
  QMap< double, double > topGridFrame;
  QMap< double, double > bottomGridFrame;

  sortGridLinesOnBorders( hLines, vLines, leftGridFrame, rightGridFrame, topGridFrame, bottomGridFrame );

  drawGridFrameBorder( p, leftGridFrame, QgsComposerMap::Left );
  drawGridFrameBorder( p, rightGridFrame, QgsComposerMap::Right );
  drawGridFrameBorder( p, topGridFrame, QgsComposerMap::Top );
  drawGridFrameBorder( p, bottomGridFrame, QgsComposerMap::Bottom );
}

void QgsComposerMap::drawGridLine( const QLineF& line, QPainter* p )
{
  if ( !mGridLineSymbol || !p )
  {
    return;
  }

  //setup render context
  QgsRenderContext context;
  context.setPainter( p );
  if ( mPreviewMode == Rectangle )
  {
    return;
  }
  else
  {
    context.setScaleFactor( 1.0 );
    context.setRasterScaleFactor( mComposition->printResolution() / 25.4 );
  }

  QPolygonF poly;
  poly << line.p1() << line.p2();
  mGridLineSymbol->startRender( context );
  mGridLineSymbol->renderPolyline( poly, 0, context );
  mGridLineSymbol->stopRender( context );
}

void QgsComposerMap::drawGridFrameBorder( QPainter* p, const QMap< double, double >& borderPos, Border border )
{
  double currentCoord = - mGridFrameWidth;
  bool white = true;
  double x = 0;
  double y = 0;
  double width = 0;
  double height = 0;

  QMap< double, double > pos = borderPos;
  pos.insert( 0, 0 );
  if ( border == Left || border == Right )
  {
    pos.insert( rect().height(), rect().height() );
    pos.insert( rect().height() + mGridFrameWidth, rect().height() + mGridFrameWidth );
  }
  else //top or bottom
  {
    pos.insert( rect().width(), rect().width() );
    pos.insert( rect().width() + mGridFrameWidth, rect().width() + mGridFrameWidth );
  }

  QMap< double, double >::const_iterator posIt = pos.constBegin();
  for ( ; posIt != pos.constEnd(); ++posIt )
  {
    p->setBrush( QBrush( white ? Qt::white : Qt::black ) );
    if ( border == Left || border == Right )
    {
      height = posIt.key() - currentCoord;
      width = mGridFrameWidth;
      x = ( border == Left ) ? -mGridFrameWidth : rect().width();
      y = currentCoord;
    }
    else //top or bottom
    {
      height = mGridFrameWidth;
      width = posIt.key() - currentCoord;
      x = currentCoord;
      y = ( border == Top ) ? -mGridFrameWidth : rect().height();
    }
    p->drawRect( QRectF( x, y, width, height ) );
    currentCoord = posIt.key();
    white = !white;
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
    currentAnnotationString = gridAnnotationString( it->first, Latitude );
    drawCoordinateAnnotation( p, it->second.p1(), currentAnnotationString );
    drawCoordinateAnnotation( p, it->second.p2(), currentAnnotationString );
  }

  it = vLines.constBegin();
  for ( ; it != vLines.constEnd(); ++it )
  {
    currentAnnotationString =  gridAnnotationString( it->first, Longitude );
    drawCoordinateAnnotation( p, it->second.p1(), currentAnnotationString );
    drawCoordinateAnnotation( p, it->second.p2(), currentAnnotationString );
  }
}

void QgsComposerMap::drawCoordinateAnnotation( QPainter* p, const QPointF& pos, QString annotationString )
{
  Border frameBorder = borderForLineCoord( pos );
  double textWidth = textWidthMillimeters( mGridAnnotationFont, annotationString );
  //relevant for annotations is the height of digits
  double textHeight = fontHeightCharacterMM( mGridAnnotationFont, QChar( '0' ) );
  double xpos = pos.x();
  double ypos = pos.y();
  int rotation = 0;

  double gridFrameDistance = ( mGridFrameStyle == NoGridFrame ) ? 0 : mGridFrameWidth;

  if ( frameBorder == Left )
  {

    if ( mLeftGridAnnotationPosition == InsideMapFrame )
    {
      if ( mLeftGridAnnotationDirection == Vertical || mLeftGridAnnotationDirection == BoundaryDirection )
      {
        xpos += textHeight + mAnnotationFrameDistance;
        ypos += textWidth / 2.0;
        rotation = 270;
      }
      else
      {
        xpos += mAnnotationFrameDistance;
        ypos += textHeight / 2.0;
      }
    }
    else if ( mLeftGridAnnotationPosition == OutsideMapFrame ) //Outside map frame
    {
      if ( mLeftGridAnnotationDirection == Vertical || mLeftGridAnnotationDirection == BoundaryDirection )
      {
        xpos -= ( mAnnotationFrameDistance + gridFrameDistance );
        ypos += textWidth / 2.0;
        rotation = 270;
      }
      else
      {
        xpos -= ( textWidth + mAnnotationFrameDistance + gridFrameDistance );
        ypos += textHeight / 2.0;
      }
    }
    else
    {
      return;
    }

  }
  else if ( frameBorder == Right )
  {
    if ( mRightGridAnnotationPosition == InsideMapFrame )
    {
      if ( mRightGridAnnotationDirection == Vertical || mRightGridAnnotationDirection == BoundaryDirection )
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
    else if ( mRightGridAnnotationPosition == OutsideMapFrame )//OutsideMapFrame
    {
      if ( mRightGridAnnotationDirection == Vertical || mRightGridAnnotationDirection == BoundaryDirection )
      {
        xpos += ( textHeight + mAnnotationFrameDistance + gridFrameDistance );
        ypos += textWidth / 2.0;
        rotation = 270;
      }
      else //Horizontal
      {
        xpos += ( mAnnotationFrameDistance + gridFrameDistance );
        ypos += textHeight / 2.0;
      }
    }
    else
    {
      return;
    }
  }
  else if ( frameBorder == Bottom )
  {
    if ( mBottomGridAnnotationPosition == InsideMapFrame )
    {
      if ( mBottomGridAnnotationDirection == Horizontal || mBottomGridAnnotationDirection == BoundaryDirection )
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
    else if ( mBottomGridAnnotationPosition == OutsideMapFrame ) //OutsideMapFrame
    {
      if ( mBottomGridAnnotationDirection == Horizontal || mBottomGridAnnotationDirection == BoundaryDirection )
      {
        ypos += ( mAnnotationFrameDistance + textHeight + gridFrameDistance );
        xpos -= textWidth / 2.0;
      }
      else //Vertical
      {
        xpos += textHeight / 2.0;
        ypos += ( textWidth + mAnnotationFrameDistance + gridFrameDistance );
        rotation = 270;
      }
    }
    else
    {
      return;
    }
  }
  else //Top
  {
    if ( mTopGridAnnotationPosition == InsideMapFrame )
    {
      if ( mTopGridAnnotationDirection == Horizontal || mTopGridAnnotationDirection == BoundaryDirection )
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
    else if ( mTopGridAnnotationPosition == OutsideMapFrame ) //OutsideMapFrame
    {
      if ( mTopGridAnnotationDirection == Horizontal || mTopGridAnnotationDirection == BoundaryDirection )
      {
        xpos -= textWidth / 2.0;
        ypos -= ( mAnnotationFrameDistance + gridFrameDistance );
      }
      else //Vertical
      {
        xpos += textHeight / 2.0;
        ypos -= ( mAnnotationFrameDistance + gridFrameDistance );
        rotation = 270;
      }
    }
    else
    {
      return;
    }
  }

  drawAnnotation( p, QPointF( xpos, ypos ), rotation, annotationString );
}

void QgsComposerMap::drawAnnotation( QPainter* p, const QPointF& pos, int rotation, const QString& annotationText )
{
  p->save();
  p->translate( pos );
  p->rotate( rotation );
  p->setPen( QColor( 0, 0, 0 ) );
  drawText( p, 0, 0, annotationText, mGridAnnotationFont );
  p->restore();
}

QString QgsComposerMap::gridAnnotationString( double value, AnnotationCoordinate coord ) const
{
  if ( mGridAnnotationFormat == Decimal )
  {
    return QString::number( value, 'f', mGridAnnotationPrecision );
  }

  QgsPoint p;
  p.setX( coord == Longitude ? value : 0 );
  p.setY( coord == Longitude ? 0 : value );

  QString annotationString;
  if ( mGridAnnotationFormat == DegreeMinute )
  {
    annotationString = p.toDegreesMinutes( mGridAnnotationPrecision );
  }
  else //DegreeMinuteSecond
  {
    annotationString = p.toDegreesMinutesSeconds( mGridAnnotationPrecision );
  }

  QStringList split = annotationString.split( "," );
  if ( coord == Longitude )
  {
    return split.at( 0 );
  }
  else
  {
    if ( split.size() < 2 )
    {
      return "";
    }
    return split.at( 1 );
  }
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

  //consider to round up to the next step in case the left boundary is > 0
  double roundCorrection = mapBoundingRect.top() > 0 ? 1.0 : 0.0;
  double currentLevel = ( int )(( mapBoundingRect.top() - mGridOffsetY ) / mGridIntervalY + roundCorrection ) * mGridIntervalY + mGridOffsetY;

  if ( qgsDoubleNear( mRotation, 0.0 ) )
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

  //consider to round up to the next step in case the left boundary is > 0
  double roundCorrection = mapBoundingRect.left() > 0 ? 1.0 : 0.0;
  double currentLevel = ( int )(( mapBoundingRect.left() - mGridOffsetX ) / mGridIntervalX + roundCorrection ) * mGridIntervalX + mGridOffsetX;

  if ( qgsDoubleNear( mRotation, 0.0 ) )
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
  if ( mGridLineSymbol )
  {
    mGridLineSymbol->setWidth( w );
  }
}

void QgsComposerMap::setGridPenColor( const QColor& c )
{
  if ( mGridLineSymbol )
  {
    mGridLineSymbol->setColor( c );
  }
}

void QgsComposerMap::setGridPen( const QPen& p )
{
  setGridPenWidth( p.widthF() );
  setGridPenColor( p.color() );
}

QPen QgsComposerMap::gridPen() const
{
  QPen p;
  if ( mGridLineSymbol )
  {
    p.setWidthF( mGridLineSymbol->width() );
    p.setColor( mGridLineSymbol->color() );
    p.setCapStyle( Qt::FlatCap );
  }
  return p;
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
  if ( !mGridEnabled || !mShowGridAnnotation || ( mLeftGridAnnotationPosition != OutsideMapFrame && mRightGridAnnotationPosition != OutsideMapFrame
       && mTopGridAnnotationPosition != OutsideMapFrame && mBottomGridAnnotationPosition != OutsideMapFrame ) )
  {
    return 0;
  }

  QList< QPair< double, QLineF > > xLines;
  QList< QPair< double, QLineF > > yLines;

  int xGridReturn = xGridLines( xLines );
  int yGridReturn = yGridLines( yLines );

  if ( xGridReturn != 0 && yGridReturn != 0 )
  {
    return 0;
  }

  double maxExtension = 0;
  double currentExtension = 0;
  QString currentAnnotationString;

  QList< QPair< double, QLineF > >::const_iterator it = xLines.constBegin();
  for ( ; it != xLines.constEnd(); ++it )
  {
    currentAnnotationString = gridAnnotationString( it->first, Latitude );
    currentExtension = qMax( textWidthMillimeters( mGridAnnotationFont, currentAnnotationString ), fontAscentMillimeters( mGridAnnotationFont ) );
    maxExtension = qMax( maxExtension, currentExtension );
  }

  it = yLines.constBegin();
  for ( ; it != yLines.constEnd(); ++it )
  {
    currentAnnotationString = gridAnnotationString( it->first, Longitude );
    currentExtension = qMax( textWidthMillimeters( mGridAnnotationFont, currentAnnotationString ), fontAscentMillimeters( mGridAnnotationFont ) );
    maxExtension = qMax( maxExtension, currentExtension );
  }

  //grid frame
  double gridFrameDist = ( mGridFrameStyle == NoGridFrame ) ? 0 : mGridFrameWidth;
  return maxExtension + mAnnotationFrameDistance + gridFrameDist;
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

void QgsComposerMap::setOverviewFrameMap( int mapId )
{
  if ( mOverviewFrameMapId != -1 )
  {
    const QgsComposerMap* map = mComposition->getComposerMapById( mapId );
    if ( map )
    {
      QObject::disconnect( map, SIGNAL( extentChanged() ), this, SLOT( repaint() ) );
    }
  }
  mOverviewFrameMapId = mapId;
  if ( mOverviewFrameMapId != -1 )
  {
    const QgsComposerMap* map = mComposition->getComposerMapById( mapId );
    if ( map )
    {
      QObject::connect( map, SIGNAL( extentChanged() ), this, SLOT( repaint() ) );
    }
  }
  update();
}

void QgsComposerMap::setOverviewFrameMapSymbol( QgsFillSymbolV2* symbol )
{
  delete mOverviewFrameMapSymbol;
  mOverviewFrameMapSymbol = symbol;
}

void QgsComposerMap::setGridLineSymbol( QgsLineSymbolV2* symbol )
{
  delete mGridLineSymbol;
  mGridLineSymbol = symbol;
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

void QgsComposerMap::drawCanvasItems( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle )
{
  if ( !mMapCanvas || !mDrawCanvasItems )
  {
    return;
  }

  QList<QGraphicsItem*> itemList = mMapCanvas->items();
  if ( itemList.size() < 1 )
  {
    return;
  }
  QGraphicsItem* currentItem = 0;

#if QT_VERSION >= 0x40600 //Qt 4.6 provides the items in visibility order
  for ( int i = itemList.size() - 1; i >= 0; --i )
  {
    currentItem = itemList.at( i );
    //don't draw mapcanvasmap (has z value -10)
    if ( !currentItem || currentItem->data( 0 ).toString() != "AnnotationItem" )
    {
      continue;
    }
    drawCanvasItem( currentItem, painter, itemStyle );
  }
#else //Qt <4.6 provides the items in random order
  QMultiMap<int, QGraphicsItem*> topLevelItems;
  QMultiMap<QGraphicsItem*, QGraphicsItem*> childItems; //QMultiMap<parentItem, childItem>

  for ( int i = 0; i < itemList.size(); ++i )
  {
    currentItem = itemList.at( i );
    //don't draw mapcanvasmap (has z value -10)
    if ( !currentItem || currentItem->data( 0 ) != "AnnotationItem" )
    {
      continue;
    }
    if ( currentItem->parentItem() )
    {
      childItems.insert( currentItem->parentItem(), currentItem );
    }
    else
    {
      topLevelItems.insert( currentItem->zValue(), currentItem );
    }
  }

  QMultiMap<int, QGraphicsItem*>::iterator topLevelIt = topLevelItems.begin();
  for ( ; topLevelIt != topLevelItems.end(); ++topLevelIt )
  {
    drawCanvasItem( topLevelIt.value(), painter, itemStyle );
    //Draw children. They probably should be sorted according to z-order, but we don't do it because this code is only
    //there for backward compatibility. And currently, having several embedded children is not used in QGIS
    QMap<QGraphicsItem*, QGraphicsItem*>::iterator childIt = childItems.find( topLevelIt.value() );
    while ( childIt != childItems.end() && childIt.key() == topLevelIt.value() )
    {
      drawCanvasItem( childIt.value(), painter, itemStyle );
      ++childIt;
    }
  }
#endif
}

void QgsComposerMap::drawCanvasItem( QGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* itemStyle )
{
  if ( !item || !mMapCanvas || !mMapRenderer  || !item->isVisible() )
  {
    return;
  }

  painter->save();

  QgsRectangle rendererExtent = mMapRenderer->extent();
  QgsRectangle composerMapExtent = mExtent;

  //determine scale factor according to graphics view dpi
  double scaleFactor = 1.0 / mMapCanvas->logicalDpiX() * 25.4;

  double itemX, itemY;
  QGraphicsItem* parent = item->parentItem();
  if ( !parent )
  {
    QPointF mapPos = composerMapPosForItem( item );
    itemX = mapPos.x();
    itemY = mapPos.y();
  }
  else //place item relative to the parent item
  {
    QPointF itemScenePos = item->scenePos();
    QPointF parentScenePos = parent->scenePos();

    QPointF mapPos = composerMapPosForItem( parent );

    itemX = mapPos.x() + ( itemScenePos.x() - parentScenePos.x() ) * scaleFactor;
    itemY = mapPos.y() + ( itemScenePos.y() - parentScenePos.y() ) * scaleFactor;
  }
  painter->translate( itemX, itemY );


  painter->scale( scaleFactor, scaleFactor );

  //a little trick to let the item know that the paint request comes from the composer
  item->setData( 1, "composer" );
  item->paint( painter, itemStyle, 0 );
  item->setData( 1, "" );
  painter->restore();
}

QPointF QgsComposerMap::composerMapPosForItem( const QGraphicsItem* item ) const
{
  if ( !item || !mMapCanvas || !mMapRenderer )
  {
    return QPointF( 0, 0 );
  }

  if ( mExtent.height() <= 0 || mExtent.width() <= 0 || mMapCanvas->width() <= 0 || mMapCanvas->height() <= 0 )
  {
    return QPointF( 0, 0 );
  }

  QRectF graphicsSceneRect = mMapCanvas->sceneRect();
  QPointF itemScenePos = item->scenePos();
  QgsRectangle mapRendererExtent = mMapRenderer->extent();

  double mapX = itemScenePos.x() / graphicsSceneRect.width() * mapRendererExtent.width() + mapRendererExtent.xMinimum();
  double mapY = mapRendererExtent.yMaximum() - itemScenePos.y() / graphicsSceneRect.height() * mapRendererExtent.height();
  return mapToItemCoords( QPointF( mapX, mapY ) );
}

void QgsComposerMap::setGridAnnotationPosition( GridAnnotationPosition p, QgsComposerMap::Border border )
{
  switch ( border )
  {
    case QgsComposerMap::Left:
      mLeftGridAnnotationPosition = p;
      break;
    case QgsComposerMap::Right:
      mRightGridAnnotationPosition = p;
      break;
    case QgsComposerMap::Top:
      mTopGridAnnotationPosition = p;
      break;
    case QgsComposerMap::Bottom:
      mBottomGridAnnotationPosition = p;
      break;
    default:
      return;
  }
  updateBoundingRect();
  update();
}

QgsComposerMap::GridAnnotationPosition QgsComposerMap::gridAnnotationPosition( QgsComposerMap::Border border ) const
{
  switch ( border )
  {
    case QgsComposerMap::Left:
      return mLeftGridAnnotationPosition;
      break;
    case QgsComposerMap::Right:
      return mRightGridAnnotationPosition;
      break;
    case QgsComposerMap::Top:
      return mTopGridAnnotationPosition;
      break;
    case QgsComposerMap::Bottom:
    default:
      return mBottomGridAnnotationPosition;
      break;
  }
}

void QgsComposerMap::setGridAnnotationDirection( GridAnnotationDirection d, QgsComposerMap::Border border )
{
  switch ( border )
  {
    case QgsComposerMap::Left:
      mLeftGridAnnotationDirection = d;
      break;
    case QgsComposerMap::Right:
      mRightGridAnnotationDirection = d;
      break;
    case QgsComposerMap::Top:
      mTopGridAnnotationDirection = d;
      break;
    case QgsComposerMap::Bottom:
      mBottomGridAnnotationDirection = d;
      break;
    default:
      return;
      break;
  }
  updateBoundingRect();
  update();
}

QgsComposerMap::GridAnnotationDirection QgsComposerMap::gridAnnotationDirection( QgsComposerMap::Border border ) const
{
  switch ( border )
  {
    case QgsComposerMap::Left:
      return mLeftGridAnnotationDirection;
      break;
    case QgsComposerMap::Right:
      return mRightGridAnnotationDirection;
      break;
    case QgsComposerMap::Top:
      return mTopGridAnnotationDirection;
      break;
    case QgsComposerMap::Bottom:
    default:
      return mBottomGridAnnotationDirection;
      break;
  }
}

void QgsComposerMap::sortGridLinesOnBorders( const QList< QPair< double, QLineF > >& hLines, const QList< QPair< double, QLineF > >& vLines,  QMap< double, double >& leftFrameEntries,
    QMap< double, double >& rightFrameEntries, QMap< double, double >& topFrameEntries, QMap< double, double >& bottomFrameEntries ) const
{
  QList< QPair< double, QPointF > > borderPositions;
  QList< QPair< double, QLineF > >::const_iterator it = hLines.constBegin();
  for ( ; it != hLines.constEnd(); ++it )
  {
    borderPositions << qMakePair( it->first, it->second.p1() );
    borderPositions << qMakePair( it->first, it->second.p2() );
  }
  it = vLines.constBegin();
  for ( ; it != vLines.constEnd(); ++it )
  {
    borderPositions << qMakePair( it->first, it->second.p1() );
    borderPositions << qMakePair( it->first, it->second.p2() );
  }

  QList< QPair< double, QPointF > >::const_iterator bIt = borderPositions.constBegin();
  for ( ; bIt != borderPositions.constEnd(); ++bIt )
  {
    Border frameBorder = borderForLineCoord( bIt->second );
    if ( frameBorder == QgsComposerMap::Left )
    {
      leftFrameEntries.insert( bIt->second.y(), bIt->first );
    }
    else if ( frameBorder == QgsComposerMap::Right )
    {
      rightFrameEntries.insert( bIt->second.y(), bIt->first );
    }
    else if ( frameBorder == QgsComposerMap::Top )
    {
      topFrameEntries.insert( bIt->second.x(), bIt->first );
    }
    else //Bottom
    {
      bottomFrameEntries.insert( bIt->second.x(), bIt->first );
    }
  }
}

void QgsComposerMap::drawOverviewMapExtent( QPainter* p )
{
  if ( mOverviewFrameMapId == -1 || !mComposition )
  {
    return;
  }

  const QgsComposerMap* overviewFrameMap = mComposition->getComposerMapById( mOverviewFrameMapId );
  if ( !overviewFrameMap )
  {
    return;
  }

  QgsRectangle otherExtent = overviewFrameMap->extent();
  QgsRectangle thisExtent = extent();
  QgsRectangle intersectRect = thisExtent.intersect( &otherExtent );

  QgsRenderContext context;
  context.setPainter( p );
  if ( mPreviewMode == Rectangle )
  {
    return;
  }
  else
  {
    context.setScaleFactor( 1.0 );
    context.setRasterScaleFactor( mComposition->printResolution() / 25.4 );
  }

  QPolygonF polygon;
  double x = ( intersectRect.xMinimum() - thisExtent.xMinimum() ) / thisExtent.width() * rect().width();
  double y = ( thisExtent.yMaximum() - intersectRect.yMaximum() ) / thisExtent.height() * rect().height();
  double width = intersectRect.width() / thisExtent.width() * rect().width();
  double height = intersectRect.height() / thisExtent.height() * rect().height();
  polygon << QPointF( x, y ) << QPointF( x + width, y ) << QPointF( x + width, y + height ) << QPointF( x, y + height ) << QPointF( x, y );

  QList<QPolygonF> rings; //empty list
  mOverviewFrameMapSymbol->startRender( context );
  mOverviewFrameMapSymbol->renderPolygon( polygon, &rings, 0, context );
  mOverviewFrameMapSymbol->stopRender( context );
}

void QgsComposerMap::createDefaultOverviewFrameSymbol()
{
  delete mOverviewFrameMapSymbol;
  QgsStringMap properties;
  properties.insert( "color", "255,0,0,125" );
  properties.insert( "style", "solid" );
  properties.insert( "style_border", "no" );
  mOverviewFrameMapSymbol = QgsFillSymbolV2::createSimple( properties );
  mOverviewFrameMapSymbol->setAlpha( 0.3 );
}

void QgsComposerMap::createDefaultGridLineSymbol()
{
  delete mGridLineSymbol;
  QgsStringMap properties;
  properties.insert( "color", "0,0,0,255" );
  properties.insert( "width", "0.3" );
  properties.insert( "capstyle", "flat" );
  mGridLineSymbol = QgsLineSymbolV2::createSimple( properties );
}

void QgsComposerMap::initGridAnnotationFormatFromProject()
{
  QString format = QgsProject::instance()->readEntry( "PositionPrecision", "/DegreeFormat", "D" );

  bool degreeUnits = true;
  if ( mMapRenderer )
  {
    degreeUnits = ( mMapRenderer->mapUnits() == QGis::Degrees );
  }

  if ( format == "DM" && degreeUnits )
  {
    mGridAnnotationFormat = DegreeMinute;
  }
  else if ( format == "DMS" && degreeUnits )
  {
    mGridAnnotationFormat = DegreeMinuteSecond;
  }
  else
  {
    mGridAnnotationFormat = Decimal;
  }
}

void QgsComposerMap::assignFreeId()
{
  if ( !mComposition )
  {
    return;
  }

  const QgsComposerMap* existingMap = mComposition->getComposerMapById( mId );
  if ( !existingMap )
  {
    return; //keep mId as it is still available
  }

  int maxId = -1;
  QList<const QgsComposerMap*> mapList = mComposition->composerMapItems();
  QList<const QgsComposerMap*>::const_iterator mapIt = mapList.constBegin();
  for ( ; mapIt != mapList.constEnd(); ++mapIt )
  {
    if (( *mapIt )->id() > maxId )
    {
      maxId = ( *mapIt )->id();
    }
  }
  mId = maxId + 1;
}
