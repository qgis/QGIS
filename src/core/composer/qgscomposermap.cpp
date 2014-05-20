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
#include "qgsmaprendererjob.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaptopixel.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgsrendercontext.h"
#include "qgsscalecalculator.h"
#include "qgsvectorlayer.h"
#include "qgspallabeling.h"
#include "qgsexpression.h"

#include "qgslabel.h"
#include "qgslabelattributes.h"
#include "qgssymbollayerv2utils.h" //for pointOnLineWithDistance

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPainter>
#include <QSettings>
#include <cmath>

QgsComposerMap::QgsComposerMap( QgsComposition *composition, int x, int y, int width, int height )
    : QgsComposerItem( x, y, width, height, composition ), mMapRotation( 0 ), mKeepLayerSet( false )
    , mOverviewFrameMapId( -1 ), mOverviewBlendMode( QPainter::CompositionMode_SourceOver ), mOverviewInverted( false ), mOverviewCentered( false )
    , mUpdatesEnabled( true ), mGridEnabled( false ), mGridStyle( Solid )
    , mGridIntervalX( 0.0 ), mGridIntervalY( 0.0 ), mGridOffsetX( 0.0 ), mGridOffsetY( 0.0 ), mGridAnnotationFontColor( QColor( 0, 0, 0 ) )
    , mGridAnnotationPrecision( 3 ), mShowGridAnnotation( false ), mGridBlendMode( QPainter::CompositionMode_SourceOver )
    , mLeftGridAnnotationPosition( OutsideMapFrame ), mRightGridAnnotationPosition( OutsideMapFrame )
    , mTopGridAnnotationPosition( OutsideMapFrame ), mBottomGridAnnotationPosition( OutsideMapFrame ), mAnnotationFrameDistance( 1.0 )
    , mLeftGridAnnotationDirection( Horizontal ), mRightGridAnnotationDirection( Horizontal ), mTopGridAnnotationDirection( Horizontal )
    , mBottomGridAnnotationDirection( Horizontal ), mGridFrameStyle( NoGridFrame ),  mGridFrameWidth( 2.0 )
    , mGridFramePenThickness( 0.5 ), mGridFramePenColor( QColor( 0, 0, 0 ) ), mGridFrameFillColor1( Qt::white ), mGridFrameFillColor2( Qt::black )
    , mCrossLength( 3 ), mMapCanvas( 0 ), mDrawCanvasItems( true ), mAtlasDriven( false ), mAtlasScalingMode( Auto ), mAtlasMargin( 0.10 )
{
  mComposition = composition;
  mOverviewFrameMapSymbol = 0;
  mGridLineSymbol = 0;
  createDefaultOverviewFrameSymbol();
  createDefaultGridLineSymbol();

  mId = 0;
  assignFreeId();

  mPreviewMode = QgsComposerMap::Rectangle;
  mCurrentRectangle = rect();

  // Cache
  mCacheUpdated = false;
  mDrawing = false;

  //Offset
  mXOffset = 0.0;
  mYOffset = 0.0;

  //get default composer font from settings
  QSettings settings;
  QString defaultFontString = settings.value( "/Composer/defaultFont" ).toString();
  if ( !defaultFontString.isEmpty() )
  {
    mGridAnnotationFont.setFamily( defaultFontString );
  }

  //get the color for map canvas background and set map background color accordingly
  int bgRedInt = QgsProject::instance()->readNumEntry( "Gui", "/CanvasColorRedPart", 255 );
  int bgGreenInt = QgsProject::instance()->readNumEntry( "Gui", "/CanvasColorGreenPart", 255 );
  int bgBlueInt = QgsProject::instance()->readNumEntry( "Gui", "/CanvasColorBluePart", 255 );
  setBackgroundColor( QColor( bgRedInt, bgGreenInt, bgBlueInt ) );

  connectUpdateSlot();

  //calculate mExtent based on width/height ratio and map canvas extent
  mExtent = mComposition->mapSettings().visibleExtent();

  setSceneRect( QRectF( x, y, width, height ) );
  setToolTip( tr( "Map %1" ).arg( mId ) );

  initGridAnnotationFormatFromProject();
}

QgsComposerMap::QgsComposerMap( QgsComposition *composition )
    : QgsComposerItem( 0, 0, 10, 10, composition ), mMapRotation( 0 ), mKeepLayerSet( false ), mOverviewFrameMapId( -1 )
    , mOverviewBlendMode( QPainter::CompositionMode_SourceOver ), mOverviewInverted( false ), mOverviewCentered( false )
    , mUpdatesEnabled( true ), mGridEnabled( false ), mGridStyle( Solid )
    , mGridIntervalX( 0.0 ), mGridIntervalY( 0.0 ), mGridOffsetX( 0.0 ), mGridOffsetY( 0.0 ), mGridAnnotationFontColor( QColor( 0, 0, 0 ) )
    , mGridAnnotationPrecision( 3 ), mShowGridAnnotation( false ), mGridBlendMode( QPainter::CompositionMode_SourceOver )
    , mLeftGridAnnotationPosition( OutsideMapFrame ), mRightGridAnnotationPosition( OutsideMapFrame )
    , mTopGridAnnotationPosition( OutsideMapFrame ), mBottomGridAnnotationPosition( OutsideMapFrame ), mAnnotationFrameDistance( 1.0 )
    , mLeftGridAnnotationDirection( Horizontal ), mRightGridAnnotationDirection( Horizontal ), mTopGridAnnotationDirection( Horizontal )
    , mBottomGridAnnotationDirection( Horizontal ), mGridFrameStyle( NoGridFrame ), mGridFrameWidth( 2.0 ), mGridFramePenThickness( 0.5 )
    , mGridFramePenColor( QColor( 0, 0, 0 ) ), mGridFrameFillColor1( Qt::white ), mGridFrameFillColor2( Qt::black )
    , mCrossLength( 3 ), mMapCanvas( 0 ), mDrawCanvasItems( true ), mAtlasDriven( false ), mAtlasScalingMode( Auto ), mAtlasMargin( 0.10 )
{
  mOverviewFrameMapSymbol = 0;
  mGridLineSymbol = 0;
  createDefaultOverviewFrameSymbol();

  //Offset
  mXOffset = 0.0;
  mYOffset = 0.0;

  connectUpdateSlot();

  mComposition = composition;
  mId = mComposition->composerMapItems().size();
  mPreviewMode = QgsComposerMap::Rectangle;
  mCurrentRectangle = rect();

  setToolTip( tr( "Map %1" ).arg( mId ) );

  initGridAnnotationFormatFromProject();
}

void QgsComposerMap::adjustExtentToItemShape( double itemWidth, double itemHeight, QgsRectangle& extent ) const
{
  double itemWidthHeightRatio = itemWidth / itemHeight;
  double newWidthHeightRatio = extent.width() / extent.height();

  if ( itemWidthHeightRatio <= newWidthHeightRatio )
  {
    //enlarge height of new extent, ensuring the map center stays the same
    double newHeight = extent.width() / itemWidthHeightRatio;
    double deltaHeight = newHeight - extent.height();
    extent.setYMinimum( extent.yMinimum() - deltaHeight / 2 );
    extent.setYMaximum( extent.yMaximum() + deltaHeight / 2 );
  }
  else
  {
    //enlarge width of new extent, ensuring the map center stays the same
    double newWidth = itemWidthHeightRatio * extent.height();
    double deltaWidth = newWidth - extent.width();
    extent.setXMinimum( extent.xMinimum() - deltaWidth / 2 );
    extent.setXMaximum( extent.xMaximum() + deltaWidth / 2 );
  }
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
  Q_UNUSED( forceWidthScale );

  if ( !painter )
  {
    return;
  }

  const QgsMapSettings& ms = mComposition->mapSettings();

  QgsMapSettings jobMapSettings;
  jobMapSettings.setExtent( extent );
  jobMapSettings.setOutputSize( size.toSize() );
  jobMapSettings.setOutputDpi( dpi );
  jobMapSettings.setMapUnits( ms.mapUnits() );
  jobMapSettings.setBackgroundColor( Qt::transparent );

  //set layers to render
  QStringList theLayerSet = layersToRender();
  if ( -1 != mCurrentExportLayer )
  {
    //exporting with separate layers (eg, to svg layers), so we only want to render a single map layer
    const int layerIdx = mCurrentExportLayer - ( hasBackground() ? 1 : 0 );
    theLayerSet =
      ( layerIdx >= 0 && layerIdx < theLayerSet.length() )
      ? QStringList( theLayerSet[ theLayerSet.length() - layerIdx - 1 ] )
      : QStringList(); //exporting decorations such as map frame/grid/overview, so no map layers required
  }
  jobMapSettings.setLayers( theLayerSet );
  jobMapSettings.setDestinationCrs( ms.destinationCrs() );
  jobMapSettings.setCrsTransformEnabled( ms.hasCrsTransformEnabled() );
  jobMapSettings.setFlags( ms.flags() );
  /* TODO[MD] fix after merge
  if ( mComposition->plotStyle() == QgsComposition::Print ||
       mComposition->plotStyle() == QgsComposition::Postscript )
  {
    //if outputing composer, disable optimisations like layer simplification
    theRendererContext->setUseRenderingOptimization( false );
  }*/

  //update $map variable. Use QgsComposerItem's id since that is user-definable
  QgsExpression::setSpecialColumn( "$map", QgsComposerItem::id() );

  // composer-specific overrides of flags
  jobMapSettings.setFlag( QgsMapSettings::ForceVectorOutput ); // force vector output (no caching of marker images etc.)
  jobMapSettings.setFlag( QgsMapSettings::DrawEditingInfo, false );
  jobMapSettings.setFlag( QgsMapSettings::UseAdvancedEffects, mComposition->useAdvancedEffects() ); // respect the composition's useAdvancedEffects flag

  // render
  QgsMapRendererCustomPainterJob job( jobMapSettings, painter );
  job.start();
  job.waitForFinished();
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
    //make sure scale factor is positive
    horizontalVScaleFactor = mLastValidViewScaleFactor > 0 ? mLastValidViewScaleFactor : 1;
  }

  double widthMM = requestExtent.width() * mapUnitsToMM();
  double heightMM = requestExtent.height() * mapUnitsToMM();

  int w = widthMM * horizontalVScaleFactor;
  int h = heightMM * horizontalVScaleFactor;

  if ( w > 5000 ) //limit size of image for better performance
  {
    w = 5000;
  }

  if ( h > 5000 )
  {
    h = 5000;
  }

  mCacheImage = QImage( w, h,  QImage::Format_ARGB32 );

  // set DPI of the image
  mCacheImage.setDotsPerMeterX( 1000 * w / widthMM );
  mCacheImage.setDotsPerMeterY( 1000 * h / heightMM );

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

  QPainter p( &mCacheImage );

  draw( &p, requestExtent, QSizeF( w, h ), mCacheImage.logicalDpiX() );
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
    painter->setPen( QColor( 0, 0, 0, 125 ) );
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

    QgsRectangle cExtent = *currentMapExtent();

    double imagePixelWidth = cExtent.width() / requestRectangle.width() * mCacheImage.width() ; //how many pixels of the image are for the map extent?
    double scale = rect().width() / imagePixelWidth;
    QgsPoint rotationPoint = QgsPoint(( cExtent.xMaximum() + cExtent.xMinimum() ) / 2.0, ( cExtent.yMaximum() + cExtent.yMinimum() ) / 2.0 );

    //shift such that rotation point is at 0/0 point in the coordinate system
    double yShiftMM = ( requestRectangle.yMaximum() - rotationPoint.y() ) * mapUnitsToMM();
    double xShiftMM = ( requestRectangle.xMinimum() - rotationPoint.x() ) * mapUnitsToMM();

    //shift such that top left point of the extent at point 0/0 in item coordinate system
    double xTopLeftShift = ( rotationPoint.x() - cExtent.xMinimum() ) * mapUnitsToMM();
    double yTopLeftShift = ( cExtent.yMaximum() - rotationPoint.y() ) * mapUnitsToMM();

    painter->save();

    painter->translate( mXOffset, mYOffset );
    painter->translate( xTopLeftShift, yTopLeftShift );
    painter->rotate( mMapRotation );
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
    if ( shouldDrawPart( Background ) )
    {
      drawBackground( painter );
    }

    QgsRectangle requestRectangle;
    requestedExtent( requestRectangle );

    QgsRectangle cExtent = *currentMapExtent();

    QSizeF theSize( requestRectangle.width() * mapUnitsToMM(), requestRectangle.height() * mapUnitsToMM() );

    QgsPoint rotationPoint = QgsPoint(( cExtent.xMaximum() + cExtent.xMinimum() ) / 2.0, ( cExtent.yMaximum() + cExtent.yMinimum() ) / 2.0 );

    //shift such that rotation point is at 0/0 point in the coordinate system
    double yShiftMM = ( requestRectangle.yMaximum() - rotationPoint.y() ) * mapUnitsToMM();
    double xShiftMM = ( requestRectangle.xMinimum() - rotationPoint.x() ) * mapUnitsToMM();

    //shift such that top left point of the extent at point 0/0 in item coordinate system
    double xTopLeftShift = ( rotationPoint.x() - cExtent.xMinimum() ) * mapUnitsToMM();
    double yTopLeftShift = ( cExtent.yMaximum() - rotationPoint.y() ) * mapUnitsToMM();
    painter->save();
    painter->translate( mXOffset, mYOffset );
    painter->translate( xTopLeftShift, yTopLeftShift );
    painter->rotate( mMapRotation );
    painter->translate( xShiftMM, -yShiftMM );

    double dotsPerMM = thePaintDevice->logicalDpiX() / 25.4;
    theSize *= dotsPerMM; // output size will be in dots (pixels)
    painter->scale( 1 / dotsPerMM, 1 / dotsPerMM ); // scale painter from mm to dots
    draw( painter, requestRectangle, theSize, thePaintDevice->logicalDpiX() );

    //restore rotation
    painter->restore();

    //draw canvas items
    drawCanvasItems( painter, itemStyle );

    mDrawing = false;
  }

  painter->setClipRect( thisPaintRect , Qt::NoClip );

  if ( mGridEnabled  && shouldDrawPart( Grid ) )
  {
    drawGrid( painter );
  }
  if ( mOverviewFrameMapId != -1 && shouldDrawPart( OverviewMapExtent ) )
  {
    drawOverviewMapExtent( painter );
  }
  if ( shouldDrawPart( Frame ) )
  {
    drawFrame( painter );
  }
  if ( isSelected() &&  shouldDrawPart( SelectionBoxes ) )
  {
    drawSelectionBoxes( painter );
  }

  painter->restore();
}

int QgsComposerMap::numberExportLayers() const
{
  return
    ( hasBackground()           ? 1 : 0 )
    + layersToRender().length()
    + ( mGridEnabled              ? 1 : 0 )
    + ( mOverviewFrameMapId != -1 ? 1 : 0 )
    + ( hasFrame()                ? 1 : 0 )
    + ( isSelected()              ? 1 : 0 )
    ;
}

bool QgsComposerMap::shouldDrawPart( PartType part ) const
{
  if ( -1 == mCurrentExportLayer )
  {
    //all parts of the composer map are visible
    return true;
  }

  int idx = numberExportLayers();
  if ( isSelected() )
  {
    --idx;
    if ( SelectionBoxes == part )
    {
      return mCurrentExportLayer == idx;
    }
  }

  if ( hasFrame() )
  {
    --idx;
    if ( Frame == part )
    {
      return mCurrentExportLayer == idx;
    }
  }
  if ( mOverviewFrameMapId )
  {
    --idx;
    if ( OverviewMapExtent == part )
    {
      return mCurrentExportLayer == idx;
    }
  }
  if ( mGridEnabled )
  {
    --idx;
    if ( Grid == part )
    {
      return mCurrentExportLayer == idx;
    }
  }
  if ( hasBackground() )
  {
    if ( Background == part )
    {
      return mCurrentExportLayer == 0;
    }
  }

  return true; // for Layer
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

const QgsMapRenderer *QgsComposerMap::mapRenderer() const
{
  Q_NOWARN_DEPRECATED_PUSH
  return mComposition->mapRenderer();
  Q_NOWARN_DEPRECATED_POP
}

QStringList QgsComposerMap::layersToRender() const
{
  //use stored layer set or read current set from main canvas
  QStringList renderLayerSet;
  if ( mKeepLayerSet )
  {
    renderLayerSet = mLayerSet;
  }
  else
  {
    renderLayerSet = mComposition->mapSettings().layers();
  }

  //remove atlas coverage layer if required
  //TODO - move setting for hiding coverage layer to map item properties
  if ( mComposition->atlasMode() != QgsComposition::AtlasOff )
  {
    if ( mComposition->atlasComposition().hideCoverage() )
    {
      //hiding coverage layer
      int removeAt = renderLayerSet.indexOf( mComposition->atlasComposition().coverageLayer()->id() );
      if ( removeAt != -1 )
      {
        renderLayerSet.removeAt( removeAt );
      }
    }
  }

  return renderLayerSet;
}

double QgsComposerMap::scale() const
{
  QgsScaleCalculator calculator;
  calculator.setMapUnits( mComposition->mapSettings().mapUnits() );
  calculator.setDpi( 25.4 );  //QGraphicsView units are mm
  return calculator.calculate( *currentMapExtent(), rect().width() );
}

void QgsComposerMap::resize( double dx, double dy )
{
  //setRect
  QRectF currentRect = rect();
  QRectF newSceneRect = QRectF( pos().x(), pos().y(), currentRect.width() + dx, currentRect.height() + dy );
  setSceneRect( newSceneRect );
  updateItem();
}

void QgsComposerMap::moveContent( double dx, double dy )
{
  if ( !mDrawing )
  {
    transformShift( dx, dy );
    currentMapExtent()->setXMinimum( currentMapExtent()->xMinimum() + dx );
    currentMapExtent()->setXMaximum( currentMapExtent()->xMaximum() + dx );
    currentMapExtent()->setYMinimum( currentMapExtent()->yMinimum() + dy );
    currentMapExtent()->setYMaximum( currentMapExtent()->yMaximum() + dy );
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
  double centerX = ( currentMapExtent()->xMaximum() + currentMapExtent()->xMinimum() ) / 2;
  double centerY = ( currentMapExtent()->yMaximum() + currentMapExtent()->yMinimum() ) / 2;

  if ( zoomMode != 0 )
  {
    //find out map coordinates of mouse position
    double mapMouseX = currentMapExtent()->xMinimum() + ( x / rect().width() ) * ( currentMapExtent()->xMaximum() - currentMapExtent()->xMinimum() );
    double mapMouseY = currentMapExtent()->yMinimum() + ( 1 - ( y / rect().height() ) ) * ( currentMapExtent()->yMaximum() - currentMapExtent()->yMinimum() );
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
    newIntervalX = ( currentMapExtent()->xMaximum() - currentMapExtent()->xMinimum() ) / zoomFactor;
    newIntervalY = ( currentMapExtent()->yMaximum() - currentMapExtent()->yMinimum() ) / zoomFactor;
  }
  else if ( delta < 0 )
  {
    newIntervalX = ( currentMapExtent()->xMaximum() - currentMapExtent()->xMinimum() ) * zoomFactor;
    newIntervalY = ( currentMapExtent()->yMaximum() - currentMapExtent()->yMinimum() ) * zoomFactor;
  }
  else //no need to zoom
  {
    return;
  }

  currentMapExtent()->setXMaximum( centerX + newIntervalX / 2 );
  currentMapExtent()->setXMinimum( centerX - newIntervalX / 2 );
  currentMapExtent()->setYMaximum( centerY + newIntervalY / 2 );
  currentMapExtent()->setYMinimum( centerY - newIntervalY / 2 );

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

  setSceneRect( QRectF( pos().x(), pos().y(), currentRect.width(), newHeight ) );
  updateItem();
}

void QgsComposerMap::setNewAtlasFeatureExtent( const QgsRectangle& extent )
{
  if ( mAtlasFeatureExtent == extent )
  {
    return;
  }

  //don't adjust size of item, instead adjust size of bounds to fit
  QgsRectangle newExtent = extent;

  //Make sure the width/height ratio is the same as the map item size
  double currentWidthHeightRatio = rect().width() / rect().height();
  double newWidthHeightRatio = newExtent.width() / newExtent.height();

  if ( currentWidthHeightRatio < newWidthHeightRatio )
  {
    //enlarge height of new extent, ensuring the map center stays the same
    double newHeight = newExtent.width() / currentWidthHeightRatio;
    double deltaHeight = newHeight - newExtent.height();
    newExtent.setYMinimum( extent.yMinimum() - deltaHeight / 2 );
    newExtent.setYMaximum( extent.yMaximum() + deltaHeight / 2 );
  }
  else if ( currentWidthHeightRatio >= newWidthHeightRatio )
  {
    //enlarge width of new extent, ensuring the map center stays the same
    double newWidth = currentWidthHeightRatio * newExtent.height();
    double deltaWidth = newWidth - newExtent.width();
    newExtent.setXMinimum( extent.xMinimum() - deltaWidth / 2 );
    newExtent.setXMaximum( extent.xMaximum() + deltaWidth / 2 );
  }

  mAtlasFeatureExtent = newExtent;
  mCacheUpdated = false;
  emit preparedForAtlas();
  updateItem();
  emit itemChanged();
  emit extentChanged();
}

void QgsComposerMap::toggleAtlasPreview()
{
  //atlas preview has been toggled, so update item and extents
  mCacheUpdated = false;
  updateItem();
  emit itemChanged();
  emit extentChanged();
}

QgsRectangle* QgsComposerMap::currentMapExtent()
{
  //non-const version
  if ( mAtlasDriven && mComposition->atlasMode() != QgsComposition::AtlasOff )
  {
    //if atlas is enabled, and we are either exporting the composition or previewing the atlas, then
    //return the current temporary atlas feature extent
    return &mAtlasFeatureExtent;
  }
  else
  {
    //otherwise return permenant user set extent
    return &mExtent;
  }
}

const QgsRectangle* QgsComposerMap::currentMapExtent() const
{
  //const version
  if ( mAtlasDriven && mComposition->atlasMode() != QgsComposition::AtlasOff )
  {
    //if atlas is enabled, and we are either exporting the composition or previewing the atlas, then
    //return the current temporary atlas feature extent
    return &mAtlasFeatureExtent;
  }
  else
  {
    //otherwise return permenant user set extent
    return &mExtent;
  }
}

void QgsComposerMap::setNewScale( double scaleDenominator )
{
  double currentScaleDenominator = scale();

  if ( scaleDenominator == currentScaleDenominator )
  {
    return;
  }

  double scaleRatio = scaleDenominator / currentScaleDenominator;
  currentMapExtent()->scale( scaleRatio );
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

void QgsComposerMap::setRotation( double r )
{
  //kept for api compatibility with QGIS 2.0
  setMapRotation( r );
}

void QgsComposerMap::setMapRotation( double r )
{
  mMapRotation = r;
  emit mapRotationChanged( r );
  emit itemChanged();
  update();
}

void QgsComposerMap::updateItem()
{
  if ( !mUpdatesEnabled )
  {
    return;
  }

  if ( mPreviewMode != QgsComposerMap::Rectangle &&  !mCacheUpdated )
  {
    cache();
  }
  QgsComposerItem::updateItem();
}

bool QgsComposerMap::containsWMSLayer() const
{
  QStringList layers = mComposition->mapSettings().layers();

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

bool QgsComposerMap::containsAdvancedEffects() const
{
  // check if map contains advanced effects like blend modes, or flattened layers for transparency

  QStringList layers = mComposition->mapSettings().layers();

  QStringList::const_iterator layer_it = layers.constBegin();
  QgsMapLayer* currentLayer = 0;

  for ( ; layer_it != layers.constEnd(); ++layer_it )
  {
    currentLayer = QgsMapLayerRegistry::instance()->mapLayer( *layer_it );
    if ( currentLayer )
    {
      if ( currentLayer->blendMode() != QPainter::CompositionMode_SourceOver )
      {
        return true;
      }
      // if vector layer, check labels and feature blend mode
      QgsVectorLayer* currentVectorLayer = qobject_cast<QgsVectorLayer *>( currentLayer );
      if ( currentVectorLayer )
      {
        if ( currentVectorLayer->layerTransparency() != 0 )
        {
          return true;
        }
        if ( currentVectorLayer->featureBlendMode() != QPainter::CompositionMode_SourceOver )
        {
          return true;
        }
        // check label blend modes
        if ( QgsPalLabeling::staticWillUseLayer( currentVectorLayer ) )
        {
          // Check all label blending properties
          QgsPalLayerSettings layerSettings = QgsPalLayerSettings::fromLayer( currentVectorLayer );
          if (( layerSettings.blendMode != QPainter::CompositionMode_SourceOver ) ||
              ( layerSettings.bufferSize != 0 && layerSettings.bufferBlendMode != QPainter::CompositionMode_SourceOver ) ||
              ( layerSettings.shadowDraw && layerSettings.shadowBlendMode != QPainter::CompositionMode_SourceOver ) ||
              ( layerSettings.shapeDraw && layerSettings.shapeBlendMode != QPainter::CompositionMode_SourceOver ) )
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
  overviewFrameElem.setAttribute( "overviewBlendMode", QgsMapRenderer::getBlendModeEnum( mOverviewBlendMode ) );
  if ( mOverviewInverted )
  {
    overviewFrameElem.setAttribute( "overviewInverted", "true" );
  }
  else
  {
    overviewFrameElem.setAttribute( "overviewInverted", "false" );
  }

  overviewFrameElem.setAttribute( "overviewCentered", mOverviewCentered ? "true" : "false" );

  QDomElement overviewFrameStyleElem = QgsSymbolLayerV2Utils::saveSymbol( QString(), mOverviewFrameMapSymbol, doc );
  overviewFrameElem.appendChild( overviewFrameStyleElem );
  composerMapElem.appendChild( overviewFrameElem );


  //extent
  QDomElement extentElem = doc.createElement( "Extent" );
  extentElem.setAttribute( "xmin", qgsDoubleToString( mExtent.xMinimum() ) );
  extentElem.setAttribute( "xmax", qgsDoubleToString( mExtent.xMaximum() ) );
  extentElem.setAttribute( "ymin", qgsDoubleToString( mExtent.yMinimum() ) );
  extentElem.setAttribute( "ymax", qgsDoubleToString( mExtent.yMaximum() ) );
  composerMapElem.appendChild( extentElem );

  //map rotation
  composerMapElem.setAttribute( "mapRotation",  QString::number( mMapRotation ) );

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
  gridElem.setAttribute( "intervalX", qgsDoubleToString( mGridIntervalX ) );
  gridElem.setAttribute( "intervalY", qgsDoubleToString( mGridIntervalY ) );
  gridElem.setAttribute( "offsetX", qgsDoubleToString( mGridOffsetX ) );
  gridElem.setAttribute( "offsetY", qgsDoubleToString( mGridOffsetY ) );
  gridElem.setAttribute( "crossLength",  qgsDoubleToString( mCrossLength ) );
  gridElem.setAttribute( "gridFrameStyle", mGridFrameStyle );
  gridElem.setAttribute( "gridFrameWidth", qgsDoubleToString( mGridFrameWidth ) );
  gridElem.setAttribute( "gridFramePenThickness", qgsDoubleToString( mGridFramePenThickness ) );
  //grid frame pen color
  QDomElement framePenColorElem = doc.createElement( "framePenColor" );
  framePenColorElem.setAttribute( "red", mGridFramePenColor.red() );
  framePenColorElem.setAttribute( "green", mGridFramePenColor.green() );
  framePenColorElem.setAttribute( "blue", mGridFramePenColor.blue() );
  framePenColorElem.setAttribute( "alpha", mGridFramePenColor.alpha() );
  gridElem.appendChild( framePenColorElem );
  //grid frame fill colors
  QDomElement frameFillColor1Elem = doc.createElement( "frameFillColor1" );
  frameFillColor1Elem.setAttribute( "red", mGridFrameFillColor1.red() );
  frameFillColor1Elem.setAttribute( "green", mGridFrameFillColor1.green() );
  frameFillColor1Elem.setAttribute( "blue", mGridFrameFillColor1.blue() );
  frameFillColor1Elem.setAttribute( "alpha", mGridFrameFillColor1.alpha() );
  gridElem.appendChild( frameFillColor1Elem );
  QDomElement frameFillColor2Elem = doc.createElement( "frameFillColor2" );
  frameFillColor2Elem.setAttribute( "red", mGridFrameFillColor2.red() );
  frameFillColor2Elem.setAttribute( "green", mGridFrameFillColor2.green() );
  frameFillColor2Elem.setAttribute( "blue", mGridFrameFillColor2.blue() );
  frameFillColor2Elem.setAttribute( "alpha", mGridFrameFillColor2.alpha() );
  gridElem.appendChild( frameFillColor2Elem );

  gridElem.setAttribute( "gridBlendMode", QgsMapRenderer::getBlendModeEnum( mGridBlendMode ) );
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
  //annotation font color
  QDomElement annotationFontColorElem = doc.createElement( "fontColor" );
  annotationFontColorElem.setAttribute( "red", mGridAnnotationFontColor.red() );
  annotationFontColorElem.setAttribute( "green", mGridAnnotationFontColor.green() );
  annotationFontColorElem.setAttribute( "blue", mGridAnnotationFontColor.blue() );
  annotationFontColorElem.setAttribute( "alpha", mGridAnnotationFontColor.alpha() );
  annotationElem.appendChild( annotationFontColorElem );

  gridElem.appendChild( annotationElem );
  composerMapElem.appendChild( gridElem );

  //atlas
  QDomElement atlasElem = doc.createElement( "AtlasMap" );
  atlasElem.setAttribute( "atlasDriven", mAtlasDriven );
  atlasElem.setAttribute( "scalingMode", mAtlasScalingMode );
  atlasElem.setAttribute( "margin", qgsDoubleToString( mAtlasMargin ) );
  composerMapElem.appendChild( atlasElem );

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
    setOverviewBlendMode( QgsMapRenderer::getCompositionMode(( QgsMapRenderer::BlendMode ) overviewFrameElem.attribute( "overviewBlendMode", "0" ).toUInt() ) );

    QString overviewInvertedFlag = overviewFrameElem.attribute( "overviewInverted" );
    if ( overviewInvertedFlag.compare( "true", Qt::CaseInsensitive ) == 0 )
    {
      setOverviewInverted( true );
    }
    else
    {
      setOverviewInverted( false );
    }

    if ( overviewFrameElem.attribute( "overviewCentered" ).compare( "true", Qt::CaseInsensitive ) == 0 )
    {
      mOverviewCentered = true;
    }
    else
    {
      mOverviewCentered = false;
    }

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
    setNewExtent( QgsRectangle( xmin, ymin, xmax, ymax ) );
  }

  //map rotation
  if ( itemElem.attribute( "mapRotation", "0" ).toDouble() != 0 )
  {
    mMapRotation = itemElem.attribute( "mapRotation", "0" ).toDouble();
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
    mGridFramePenThickness = gridElem.attribute( "gridFramePenThickness", "0.5" ).toDouble();

    //grid frame pen color
    QDomNodeList gridFramePenColorList = gridElem.elementsByTagName( "framePenColor" );
    if ( gridFramePenColorList.size() > 0 )
    {
      QDomElement penColorElem = gridFramePenColorList.at( 0 ).toElement();
      int red = penColorElem.attribute( "red", "0" ).toInt();
      int green = penColorElem.attribute( "green", "0" ).toInt();
      int blue = penColorElem.attribute( "blue", "0" ).toInt();
      int alpha = penColorElem.attribute( "alpha", "255" ).toInt();
      mGridFramePenColor = QColor( red, green, blue, alpha );
    }
    else
    {
      mGridFramePenColor = QColor( 0, 0, 0 );
    }
    //grid frame fill color 1
    QDomNodeList gridFrameFillColor1List = gridElem.elementsByTagName( "frameFillColor1" );
    if ( gridFrameFillColor1List.size() > 0 )
    {
      QDomElement fillColorElem = gridFrameFillColor1List.at( 0 ).toElement();
      int red = fillColorElem.attribute( "red", "0" ).toInt();
      int green = fillColorElem.attribute( "green", "0" ).toInt();
      int blue = fillColorElem.attribute( "blue", "0" ).toInt();
      int alpha = fillColorElem.attribute( "alpha", "255" ).toInt();
      mGridFrameFillColor1 = QColor( red, green, blue, alpha );
    }
    else
    {
      mGridFrameFillColor1 = Qt::white;
    }
    //grid frame fill color 2
    QDomNodeList gridFrameFillColor2List = gridElem.elementsByTagName( "frameFillColor2" );
    if ( gridFrameFillColor2List.size() > 0 )
    {
      QDomElement fillColorElem = gridFrameFillColor2List.at( 0 ).toElement();
      int red = fillColorElem.attribute( "red", "0" ).toInt();
      int green = fillColorElem.attribute( "green", "0" ).toInt();
      int blue = fillColorElem.attribute( "blue", "0" ).toInt();
      int alpha = fillColorElem.attribute( "alpha", "255" ).toInt();
      mGridFrameFillColor2 = QColor( red, green, blue, alpha );
    }
    else
    {
      mGridFrameFillColor2 = Qt::black;
    }

    setGridBlendMode( QgsMapRenderer::getCompositionMode(( QgsMapRenderer::BlendMode ) gridElem.attribute( "gridBlendMode", "0" ).toUInt() ) );

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

      //annotation font color
      QDomNodeList annotationFontColorList = annotationElem.elementsByTagName( "fontColor" );
      if ( annotationFontColorList.size() > 0 )
      {
        QDomElement fontColorElem = annotationFontColorList.at( 0 ).toElement();
        int red = fontColorElem.attribute( "red", "0" ).toInt();
        int green = fontColorElem.attribute( "green", "0" ).toInt();
        int blue = fontColorElem.attribute( "blue", "0" ).toInt();
        int alpha = fontColorElem.attribute( "alpha", "255" ).toInt();
        mGridAnnotationFontColor = QColor( red, green, blue, alpha );
      }
      else
      {
        mGridAnnotationFontColor = QColor( 0, 0, 0 );
      }

      mGridAnnotationPrecision = annotationElem.attribute( "precision", "3" ).toInt();
    }
  }

  //atlas
  QDomNodeList atlasNodeList = itemElem.elementsByTagName( "AtlasMap" );
  if ( atlasNodeList.size() > 0 )
  {
    QDomElement atlasElem = atlasNodeList.at( 0 ).toElement();
    mAtlasDriven = ( atlasElem.attribute( "atlasDriven", "0" ) != "0" );
    if ( atlasElem.hasAttribute("fixedScale") ) { // deprecated XML
      mAtlasScalingMode = (atlasElem.attribute( "fixedScale", "0" ) != "0") ? Fixed : Auto;
    }
    else if ( atlasElem.hasAttribute("scalingMode") ) {
      mAtlasScalingMode = static_cast<AtlasScalingMode>(atlasElem.attribute("scalingMode").toInt());
    }
    mAtlasMargin = atlasElem.attribute( "margin", "0.1" ).toDouble();
  }

  //restore general composer item properties
  QDomNodeList composerItemList = itemElem.elementsByTagName( "ComposerItem" );
  if ( composerItemList.size() > 0 )
  {
    QDomElement composerItemElem = composerItemList.at( 0 ).toElement();

    if ( composerItemElem.attribute( "rotation", "0" ).toDouble() != 0 )
    {
      //in versions prior to 2.1 map rotation was stored in the rotation attribute
      mMapRotation = composerItemElem.attribute( "rotation", "0" ).toDouble();
    }

    _readXML( composerItemElem, doc );
  }

  updateBoundingRect();
  emit itemChanged();
  return true;
}

void QgsComposerMap::storeCurrentLayerSet()
{
  mLayerSet = mComposition->mapSettings().layers();
}

void QgsComposerMap::syncLayerSet()
{
  if ( mLayerSet.size() < 1 )
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
    currentLayerSet = mComposition->mapSettings().layers();
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

  // set the blend mode for drawing grid lines
  p->save();
  p->setCompositionMode( mGridBlendMode );

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
  // reset composition mode
  p->restore();

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
  bool color1 = true;
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

  //set pen to current frame pen
  QPen framePen = QPen( mGridFramePenColor );
  framePen.setWidthF( mGridFramePenThickness );
  framePen.setJoinStyle( Qt::MiterJoin );
  p->setPen( framePen );

  QMap< double, double >::const_iterator posIt = pos.constBegin();
  for ( ; posIt != pos.constEnd(); ++posIt )
  {
    p->setBrush( QBrush( color1 ? mGridFrameFillColor1 : mGridFrameFillColor2 ) );
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
    color1 = !color1;
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

  double gridFrameDistance = ( mGridFrameStyle == NoGridFrame ) ? 0 : mGridFrameWidth + ( mGridFramePenThickness / 2.0 );

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
  p->setPen( QPen( QColor( mGridAnnotationFontColor ) ) );
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

  if ( qgsDoubleNear( mMapRotation, 0.0 ) )
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

  if ( qgsDoubleNear( mMapRotation, 0.0 ) )
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

void QgsComposerMap::setGridBlendMode( QPainter::CompositionMode blendMode )
{
  mGridBlendMode = blendMode;
  update();
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

void QgsComposerMap::setFrameOutlineWidth( double outlineWidth )
{
  QgsComposerItem::setFrameOutlineWidth( outlineWidth );
  updateBoundingRect();
}

QgsRectangle QgsComposerMap::transformedExtent() const
{
  double dx = mXOffset;
  double dy = mYOffset;
  transformShift( dx, dy );
  return QgsRectangle( currentMapExtent()->xMinimum() - dx, currentMapExtent()->yMinimum() - dy, currentMapExtent()->xMaximum() - dx, currentMapExtent()->yMaximum() - dy );
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
  double frameExtension = mFrame ? pen().widthF() / 2.0 : 0.0;
  if ( !mGridEnabled || ( mGridFrameStyle == QgsComposerMap::NoGridFrame && ( !mShowGridAnnotation || ( mLeftGridAnnotationPosition != OutsideMapFrame && mRightGridAnnotationPosition != OutsideMapFrame
                          && mTopGridAnnotationPosition != OutsideMapFrame && mBottomGridAnnotationPosition != OutsideMapFrame ) ) ) )
  {
    return frameExtension;
  }

  QList< QPair< double, QLineF > > xLines;
  QList< QPair< double, QLineF > > yLines;

  int xGridReturn = xGridLines( xLines );
  int yGridReturn = yGridLines( yLines );

  if ( xGridReturn != 0 && yGridReturn != 0 )
  {
    return frameExtension;
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
  double gridFrameDist = ( mGridFrameStyle == NoGridFrame ) ? 0 : mGridFrameWidth + ( mGridFramePenThickness / 2.0 );
  return qMax( frameExtension, maxExtension + mAnnotationFrameDistance + gridFrameDist );
}

void QgsComposerMap::mapPolygon( const QgsRectangle& extent, QPolygonF& poly ) const
{
  poly.clear();
  if ( mMapRotation == 0 )
  {
    poly << QPointF( extent.xMinimum(), extent.yMaximum() );
    poly << QPointF( extent.xMaximum(), extent.yMaximum() );
    poly << QPointF( extent.xMaximum(), extent.yMinimum() );
    poly << QPointF( extent.xMinimum(), extent.yMinimum() );
    return;
  }

  //there is rotation
  QgsPoint rotationPoint(( extent.xMaximum() + extent.xMinimum() ) / 2.0, ( extent.yMaximum() + extent.yMinimum() ) / 2.0 );
  double dx, dy; //x-, y- shift from rotation point to corner point

  //top left point
  dx = rotationPoint.x() - extent.xMinimum();
  dy = rotationPoint.y() - extent.yMaximum();
  rotate( mMapRotation, dx, dy );
  poly << QPointF( rotationPoint.x() + dx, rotationPoint.y() + dy );

  //top right point
  dx = rotationPoint.x() - extent.xMaximum();
  dy = rotationPoint.y() - extent.yMaximum();
  rotate( mMapRotation, dx, dy );
  poly << QPointF( rotationPoint.x() + dx, rotationPoint.y() + dy );

  //bottom right point
  dx = rotationPoint.x() - extent.xMaximum();
  dy = rotationPoint.y() - extent.yMinimum();
  rotate( mMapRotation, dx, dy );
  poly << QPointF( rotationPoint.x() + dx, rotationPoint.y() + dy );

  //bottom left point
  dx = rotationPoint.x() - extent.xMinimum();
  dy = rotationPoint.y() - extent.yMinimum();
  rotate( mMapRotation, dx, dy );
  poly << QPointF( rotationPoint.x() + dx, rotationPoint.y() + dy );
}

void QgsComposerMap::mapPolygon( QPolygonF& poly ) const
{
  return mapPolygon( *currentMapExtent(), poly );
}

void QgsComposerMap::requestedExtent( QgsRectangle& extent ) const
{
  QgsRectangle newExtent = *currentMapExtent();
  if ( mMapRotation == 0 )
  {
    extent = newExtent;
  }
  else
  {
    QPolygonF poly;
    mapPolygon( newExtent, poly );
    QRectF bRect = poly.boundingRect();
    extent.setXMinimum( bRect.left() );
    extent.setXMaximum( bRect.right() );
    extent.setYMinimum( bRect.top() );
    extent.setYMaximum( bRect.bottom() );
  }
}

double QgsComposerMap::mapUnitsToMM() const
{
  double extentWidth = currentMapExtent()->width();
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
      QObject::disconnect( map, SIGNAL( extentChanged() ), this, SLOT( overviewExtentChanged() ) );
    }
  }
  mOverviewFrameMapId = mapId;
  if ( mOverviewFrameMapId != -1 )
  {
    const QgsComposerMap* map = mComposition->getComposerMapById( mapId );
    if ( map )
    {
      QObject::connect( map, SIGNAL( extentChanged() ), this, SLOT( overviewExtentChanged() ) );
    }
  }
  update();
}

void QgsComposerMap::overviewExtentChanged()
{
  //if using overview centering, update the map's extent
  if ( mOverviewCentered && mOverviewFrameMapId != -1 )
  {
    QgsRectangle extent = *currentMapExtent();

    const QgsComposerMap* overviewFrameMap = mComposition->getComposerMapById( mOverviewFrameMapId );
    QgsRectangle otherExtent = *overviewFrameMap->currentMapExtent();

    QgsPoint center = otherExtent.center();
    QgsRectangle movedExtent( center.x() - currentMapExtent()->width() / 2,
                              center.y() - currentMapExtent()->height() / 2,
                              center.x() - currentMapExtent()->width() / 2 + currentMapExtent()->width(),
                              center.y() - currentMapExtent()->height() / 2 + currentMapExtent()->height() );
    *currentMapExtent() = movedExtent;

    emit itemChanged();
    emit extentChanged();
  }

  //redraw so that overview gets updated
  cache();
  update();
}


void QgsComposerMap::setOverviewFrameMapSymbol( QgsFillSymbolV2* symbol )
{
  delete mOverviewFrameMapSymbol;
  mOverviewFrameMapSymbol = symbol;
}

void QgsComposerMap::setOverviewBlendMode( QPainter::CompositionMode blendMode )
{
  mOverviewBlendMode = blendMode;
  update();
}

void QgsComposerMap::setOverviewInverted( bool inverted )
{
  mOverviewInverted = inverted;
  update();
}

void QgsComposerMap::setOverviewCentered( bool centered )
{
  mOverviewCentered = centered;
  overviewExtentChanged();
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

  rotate( mMapRotation, dxScaled, dyScaled );

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
  rotate( -mMapRotation, dx, dy );
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
}

void QgsComposerMap::drawCanvasItem( QGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* itemStyle )
{
  if ( !item || !mMapCanvas || !item->isVisible() )
  {
    return;
  }

  painter->save();

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
  if ( !item || !mMapCanvas )
  {
    return QPointF( 0, 0 );
  }

  if ( currentMapExtent()->height() <= 0 || currentMapExtent()->width() <= 0 || mMapCanvas->width() <= 0 || mMapCanvas->height() <= 0 )
  {
    return QPointF( 0, 0 );
  }

  QRectF graphicsSceneRect = mMapCanvas->sceneRect();
  QPointF itemScenePos = item->scenePos();
  QgsRectangle mapRendererExtent = mComposition->mapSettings().visibleExtent();

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

  if ( mComposition->plotStyle() == QgsComposition::Preview && mPreviewMode == Rectangle )
  {
    //if map item is set to rectangle preview mode and we are not exporting the composition
    //then don't draw an overview rectangle
    return;
  }

  const QgsComposerMap* overviewFrameMap = mComposition->getComposerMapById( mOverviewFrameMapId );
  if ( !overviewFrameMap )
  {
    return;
  }

  QgsRectangle otherExtent = *overviewFrameMap->currentMapExtent();
  QgsRectangle thisExtent = *currentMapExtent();
  QgsRectangle intersectRect = thisExtent.intersect( &otherExtent );

  QgsRenderContext context;
  context.setPainter( p );
  context.setScaleFactor( 1.0 );
  context.setRasterScaleFactor( mComposition->printResolution() / 25.4 );

  p->save();
  p->setCompositionMode( mOverviewBlendMode );
  p->translate( mXOffset, mYOffset );
  mOverviewFrameMapSymbol->startRender( context );

  //construct a polygon corresponding to the intersecting map extent
  QPolygonF intersectPolygon;
  double x = ( intersectRect.xMinimum() - thisExtent.xMinimum() ) / thisExtent.width() * rect().width();
  double y = ( thisExtent.yMaximum() - intersectRect.yMaximum() ) / thisExtent.height() * rect().height();
  double width = intersectRect.width() / thisExtent.width() * rect().width();
  double height = intersectRect.height() / thisExtent.height() * rect().height();
  intersectPolygon << QPointF( x, y ) << QPointF( x + width, y ) << QPointF( x + width, y + height ) << QPointF( x, y + height ) << QPointF( x, y );

  QList<QPolygonF> rings; //empty list
  if ( !mOverviewInverted )
  {
    //Render the intersecting map extent
    mOverviewFrameMapSymbol->renderPolygon( intersectPolygon, &rings, 0, context );;
  }
  else
  {
    //We are inverting the overview frame (ie, shading outside the intersecting extent)
    //Construct a polygon corresponding to the overview map extent
    QPolygonF outerPolygon;
    outerPolygon << QPointF( 0, 0 ) << QPointF( rect().width(), 0 ) << QPointF( rect().width(), rect().height() ) << QPointF( 0, rect().height() ) << QPointF( 0, 0 );

    //Intersecting extent is an inner ring for the shaded area
    rings.append( intersectPolygon );
    mOverviewFrameMapSymbol->renderPolygon( outerPolygon, &rings, 0, context );
  }

  mOverviewFrameMapSymbol->stopRender( context );
  p->restore();
}

void QgsComposerMap::createDefaultOverviewFrameSymbol()
{
  delete mOverviewFrameMapSymbol;
  QgsStringMap properties;
  properties.insert( "color", "255,0,0,255" );
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

  bool degreeUnits = ( mComposition->mapSettings().mapUnits() == QGis::Degrees );

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

bool QgsComposerMap::imageSizeConsideringRotation( double& width, double& height ) const
{
  //kept for api compatibility with QGIS 2.0 - use mMapRotation
  return QgsComposerItem::imageSizeConsideringRotation( width, height, mMapRotation );
}

bool QgsComposerMap::cornerPointOnRotatedAndScaledRect( double& x, double& y, double width, double height ) const
{
  //kept for api compatibility with QGIS 2.0 - use mMapRotation
  return QgsComposerItem::cornerPointOnRotatedAndScaledRect( x, y, width, height, mMapRotation );
}

void QgsComposerMap::sizeChangedByRotation( double& width, double& height )
{
  //kept for api compatibility with QGIS 2.0 - use mMapRotation
  return QgsComposerItem::sizeChangedByRotation( width, height, mMapRotation );
}

bool QgsComposerMap::atlasFixedScale() const
{
  return mAtlasScalingMode == Fixed;
}

void QgsComposerMap::setAtlasFixedScale( bool fixed )
{
  // implicit : if set to false => auto scaling
  mAtlasScalingMode = fixed ? Fixed : Auto;
}

