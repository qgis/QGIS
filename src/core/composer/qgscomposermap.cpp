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
#include "qgscomposermapgrid.h"
#include "qgscomposermapoverview.h"
#include "qgscomposition.h"
#include "qgscomposerutils.h"
#include "qgscoordinatetransform.h"
#include "qgslogger.h"
#include "qgsmaprenderer.h"
#include "qgsmaprenderercustompainterjob.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaplayerstylemanager.h"
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
    : QgsComposerItem( x, y, width, height, composition )
    , mGridStack( 0 )
    , mOverviewStack( 0 )
    , mMapRotation( 0 )
    , mEvaluatedMapRotation( 0 )
    , mKeepLayerSet( false )
    , mUpdatesEnabled( true )
    , mMapCanvas( 0 )
    , mDrawCanvasItems( true )
    , mAtlasDriven( false )
    , mAtlasScalingMode( Auto )
    , mAtlasMargin( 0.10 )
{
  mComposition = composition;

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

  //get the color for map canvas background and set map background color accordingly
  int bgRedInt = QgsProject::instance()->readNumEntry( "Gui", "/CanvasColorRedPart", 255 );
  int bgGreenInt = QgsProject::instance()->readNumEntry( "Gui", "/CanvasColorGreenPart", 255 );
  int bgBlueInt = QgsProject::instance()->readNumEntry( "Gui", "/CanvasColorBluePart", 255 );
  setBackgroundColor( QColor( bgRedInt, bgGreenInt, bgBlueInt ) );

  //calculate mExtent based on width/height ratio and map canvas extent
  mExtent = mComposition->mapSettings().visibleExtent();

  init();

  setSceneRect( QRectF( x, y, width, height ) );
}

QgsComposerMap::QgsComposerMap( QgsComposition *composition )
    : QgsComposerItem( 0, 0, 10, 10, composition )
    , mGridStack( 0 )
    , mOverviewStack( 0 )
    , mMapRotation( 0 )
    , mEvaluatedMapRotation( 0 )
    , mKeepLayerSet( false )
    , mUpdatesEnabled( true )
    , mMapCanvas( 0 )
    , mDrawCanvasItems( true )
    , mAtlasDriven( false )
    , mAtlasScalingMode( Auto )
    , mAtlasMargin( 0.10 )
{
  //Offset
  mXOffset = 0.0;
  mYOffset = 0.0;

  mComposition = composition;
  mId = mComposition->composerMapItems().size();
  mPreviewMode = QgsComposerMap::Rectangle;
  mCurrentRectangle = rect();

  init();
  updateToolTip();
}

void QgsComposerMap::init()
{
  mGridStack = new QgsComposerMapGridStack( this );
  mOverviewStack = new QgsComposerMapOverviewStack( this );
  connectUpdateSlot();

  // data defined strings
  mDataDefinedNames.insert( QgsComposerObject::MapRotation, QString( "dataDefinedMapRotation" ) );
  mDataDefinedNames.insert( QgsComposerObject::MapScale, QString( "dataDefinedMapScale" ) );
  mDataDefinedNames.insert( QgsComposerObject::MapXMin, QString( "dataDefinedMapXMin" ) );
  mDataDefinedNames.insert( QgsComposerObject::MapYMin, QString( "dataDefinedMapYMin" ) );
  mDataDefinedNames.insert( QgsComposerObject::MapXMax, QString( "dataDefinedMapXMax" ) );
  mDataDefinedNames.insert( QgsComposerObject::MapYMax, QString( "dataDefinedMapYMax" ) );
  mDataDefinedNames.insert( QgsComposerObject::MapAtlasMargin, QString( "dataDefinedMapAtlasMargin" ) );
}

void QgsComposerMap::updateToolTip()
{
  setToolTip( tr( "Map %1" ).arg( mId ) );
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
  delete mOverviewStack;
  delete mGridStack;
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
  if ( size.width() == 0 || size.height() == 0 )
  {
    //don't attempt to draw if size is invalid
    return;
  }


  // render
  QgsMapRendererCustomPainterJob job( mapSettings( extent, size, dpi ), painter );
  // Render the map in this thread. This is done because of problems
  // with printing to printer on Windows (printing to PDF is fine though).
  // Raster images were not displayed - see #10599
  job.renderSynchronously();
}

QgsMapSettings QgsComposerMap::mapSettings( const QgsRectangle& extent, const QSizeF& size, int dpi ) const
{
  const QgsMapSettings& ms = mComposition->mapSettings();

  QgsMapSettings jobMapSettings;
  jobMapSettings.setExtent( extent );
  jobMapSettings.setOutputSize( size.toSize() );
  jobMapSettings.setOutputDpi( dpi );
  jobMapSettings.setMapUnits( ms.mapUnits() );
  jobMapSettings.setBackgroundColor( Qt::transparent );
  jobMapSettings.setOutputImageFormat( ms.outputImageFormat() );

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
  jobMapSettings.setLayerStyleOverrides( mLayerStyleOverrides );
  jobMapSettings.setDestinationCrs( ms.destinationCrs() );
  jobMapSettings.setCrsTransformEnabled( ms.hasCrsTransformEnabled() );
  jobMapSettings.setFlags( ms.flags() );
  jobMapSettings.setFlag( QgsMapSettings::DrawSelection, false );

  if ( mComposition->plotStyle() == QgsComposition::Print ||
       mComposition->plotStyle() == QgsComposition::Postscript )
  {
    //if outputing composer, disable optimisations like layer simplification
    jobMapSettings.setFlag( QgsMapSettings::UseRenderingOptimization, false );
  }

  //update $map variable. Use QgsComposerItem's id since that is user-definable
  QgsExpression::setSpecialColumn( "$map", QgsComposerItem::id() );

  // composer-specific overrides of flags
  jobMapSettings.setFlag( QgsMapSettings::ForceVectorOutput ); // force vector output (no caching of marker images etc.)
  jobMapSettings.setFlag( QgsMapSettings::DrawEditingInfo, false );
  jobMapSettings.setFlag( QgsMapSettings::UseAdvancedEffects, mComposition->useAdvancedEffects() ); // respect the composition's useAdvancedEffects flag

  return jobMapSettings;
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

  mCacheImage = QImage( w, h, QImage::Format_ARGB32 );

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
  if ( !shouldDrawItem() )
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
    painter->rotate( mEvaluatedMapRotation );
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
    painter->rotate( mEvaluatedMapRotation );
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

  painter->setClipRect( thisPaintRect, Qt::NoClip );
  if ( shouldDrawPart( OverviewMapExtent ) &&
       ( mComposition->plotStyle() != QgsComposition::Preview || mPreviewMode != Rectangle ) )
  {
    mOverviewStack->drawItems( painter );
  }
  if ( shouldDrawPart( Grid ) &&
       ( mComposition->plotStyle() != QgsComposition::Preview || mPreviewMode != Rectangle ) )
  {
    mGridStack->drawItems( painter );
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
    + 1 // for grids, if they exist
    + 1 // for overviews, if they exist
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
  --idx;
  if ( OverviewMapExtent == part )
  {
    return mCurrentExportLayer == idx;
  }
  --idx;
  if ( Grid == part )
  {
    return mCurrentExportLayer == idx;
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

    //in case data defined extents are set, these override the calculated values
    refreshMapExtents();

    cache();
    update();
    emit itemChanged();
    emit extentChanged();
  }
}

void QgsComposerMap::zoomContent( int delta, double x, double y )
{
  QSettings settings;

  //read zoom mode
  QgsComposerItem::ZoomMode zoomMode = ( QgsComposerItem::ZoomMode )settings.value( "/qgis/wheel_action", 2 ).toInt();
  if ( zoomMode == QgsComposerItem::NoZoom )
  {
    //do nothing
    return;
  }

  double zoomFactor = settings.value( "/qgis/zoom_factor", 2.0 ).toDouble();
  zoomFactor = delta > 0 ? zoomFactor : 1 / zoomFactor;

  zoomContent( zoomFactor, QPointF( x, y ), zoomMode );
}

void QgsComposerMap::zoomContent( const double factor, const QPointF point, const ZoomMode mode )
{
  if ( mDrawing )
  {
    return;
  }

  if ( mode == QgsComposerItem::NoZoom )
  {
    //do nothing
    return;
  }

  //find out map coordinates of position
  double mapX = currentMapExtent()->xMinimum() + ( point.x() / rect().width() ) * ( currentMapExtent()->xMaximum() - currentMapExtent()->xMinimum() );
  double mapY = currentMapExtent()->yMinimum() + ( 1 - ( point.y() / rect().height() ) ) * ( currentMapExtent()->yMaximum() - currentMapExtent()->yMinimum() );

  //find out new center point
  double centerX = ( currentMapExtent()->xMaximum() + currentMapExtent()->xMinimum() ) / 2;
  double centerY = ( currentMapExtent()->yMaximum() + currentMapExtent()->yMinimum() ) / 2;

  if ( mode != QgsComposerItem::Zoom )
  {
    if ( mode == QgsComposerItem::ZoomRecenter )
    {
      centerX = mapX;
      centerY = mapY;
    }
    else if ( mode == QgsComposerItem::ZoomToPoint )
    {
      centerX = mapX + ( centerX - mapX ) * ( 1.0 / factor );
      centerY = mapY + ( centerY - mapY ) * ( 1.0 / factor );
    }
  }

  double newIntervalX, newIntervalY;

  if ( factor > 0 )
  {
    newIntervalX = ( currentMapExtent()->xMaximum() - currentMapExtent()->xMinimum() ) / factor;
    newIntervalY = ( currentMapExtent()->yMaximum() - currentMapExtent()->yMinimum() ) / factor;
  }
  else //no need to zoom
  {
    return;
  }

  currentMapExtent()->setXMaximum( centerX + newIntervalX / 2 );
  currentMapExtent()->setXMinimum( centerX - newIntervalX / 2 );
  currentMapExtent()->setYMaximum( centerY + newIntervalY / 2 );
  currentMapExtent()->setYMinimum( centerY - newIntervalY / 2 );

  if ( mAtlasDriven && mAtlasScalingMode == Fixed && mComposition->atlasMode() != QgsComposition::AtlasOff )
  {
    //if map is atlas controlled and set to fixed scaling mode, then scale changes should be treated as permanant
    //and also apply to the map's original extent (see #9602)
    //we can't use the scaleRatio calculated earlier, as the scale can vary depending on extent for geographic coordinate systems
    QgsScaleCalculator calculator;
    calculator.setMapUnits( mComposition->mapSettings().mapUnits() );
    calculator.setDpi( 25.4 );  //QGraphicsView units are mm
    double scaleRatio = scale() / calculator.calculate( mExtent, rect().width() );
    mExtent.scale( scaleRatio );
  }

  //recalculate data defined scale and extents, since that may override zoom
  refreshMapExtents();

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
  double newHeight = mExtent.width() * h / w;
  mExtent = QgsRectangle( mExtent.xMinimum(), mExtent.yMinimum(), mExtent.xMaximum(), mExtent.yMinimum() + newHeight );

  //recalculate data defined scale and extents
  refreshMapExtents();
  mCacheUpdated = false;

  updateBoundingRect();
  update();
  emit itemChanged();
  emit extentChanged();
}

void QgsComposerMap::setNewExtent( const QgsRectangle& extent )
{
  if ( *currentMapExtent() == extent )
  {
    return;
  }
  *currentMapExtent() = extent;

  //recalculate data defined scale and extents, since that may override extent
  refreshMapExtents();

  //adjust height
  QRectF currentRect = rect();

  double newHeight = currentRect.width() * currentMapExtent()->height() / currentMapExtent()->width();

  setSceneRect( QRectF( pos().x(), pos().y(), currentRect.width(), newHeight ) );
  updateItem();
}

void QgsComposerMap::zoomToExtent( const QgsRectangle &extent )
{
  QgsRectangle newExtent = extent;
  //Make sure the width/height ratio is the same as the current composer map extent.
  //This is to keep the map item frame size fixed
  double currentWidthHeightRatio = currentMapExtent()->width() / currentMapExtent()->height();
  double newWidthHeightRatio = newExtent.width() / newExtent.height();

  if ( currentWidthHeightRatio < newWidthHeightRatio )
  {
    //enlarge height of new extent, ensuring the map center stays the same
    double newHeight = newExtent.width() / currentWidthHeightRatio;
    double deltaHeight = newHeight - newExtent.height();
    newExtent.setYMinimum( newExtent.yMinimum() - deltaHeight / 2 );
    newExtent.setYMaximum( newExtent.yMaximum() + deltaHeight / 2 );
  }
  else
  {
    //enlarge width of new extent, ensuring the map center stays the same
    double newWidth = currentWidthHeightRatio * newExtent.height();
    double deltaWidth = newWidth - newExtent.width();
    newExtent.setXMinimum( newExtent.xMinimum() - deltaWidth / 2 );
    newExtent.setXMaximum( newExtent.xMaximum() + deltaWidth / 2 );
  }

  if ( *currentMapExtent() == newExtent )
  {
    return;
  }
  *currentMapExtent() = newExtent;

  //recalculate data defined scale and extents, since that may override extent
  refreshMapExtents();

  mCacheUpdated = false;
  updateItem();
  emit itemChanged();
  emit extentChanged();
}

void QgsComposerMap::setNewAtlasFeatureExtent( const QgsRectangle& extent )
{
  if ( mAtlasFeatureExtent != extent )
  {
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
  }

  //recalculate data defined scale and extents, since that may override extents
  refreshMapExtents();

  mCacheUpdated = false;
  emit preparedForAtlas();
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

void QgsComposerMap::setNewScale( double scaleDenominator, bool forceUpdate )
{
  double currentScaleDenominator = scale();

  if ( scaleDenominator == currentScaleDenominator || scaleDenominator == 0 )
  {
    return;
  }

  double scaleRatio = scaleDenominator / currentScaleDenominator;
  currentMapExtent()->scale( scaleRatio );

  if ( mAtlasDriven && mAtlasScalingMode == Fixed && mComposition->atlasMode() != QgsComposition::AtlasOff )
  {
    //if map is atlas controlled and set to fixed scaling mode, then scale changes should be treated as permanant
    //and also apply to the map's original extent (see #9602)
    //we can't use the scaleRatio calculated earlier, as the scale can vary depending on extent for geographic coordinate systems
    QgsScaleCalculator calculator;
    calculator.setMapUnits( mComposition->mapSettings().mapUnits() );
    calculator.setDpi( 25.4 );  //QGraphicsView units are mm
    scaleRatio = scaleDenominator / calculator.calculate( mExtent, rect().width() );
    mExtent.scale( scaleRatio );
  }

  mCacheUpdated = false;
  if ( forceUpdate )
  {
    cache();
    update();
    emit itemChanged();
  }
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
  mEvaluatedMapRotation = mMapRotation;
  emit mapRotationChanged( r );
  emit itemChanged();
  update();
}

double QgsComposerMap::mapRotation( QgsComposerObject::PropertyValueType valueType ) const
{
  return valueType == QgsComposerObject::EvaluatedValue ? mEvaluatedMapRotation : mMapRotation;
}

void QgsComposerMap::refreshMapExtents()
{
  //data defined map extents set?
  QVariant exprVal;

  QgsRectangle newExtent = *currentMapExtent();
  bool useDdXMin = false;
  bool useDdXMax = false;
  bool useDdYMin = false;
  bool useDdYMax = false;
  double minXD = 0;
  double minYD = 0;
  double maxXD = 0;
  double maxYD = 0;

  if ( dataDefinedEvaluate( QgsComposerObject::MapXMin, exprVal ) )
  {
    bool ok;
    minXD = exprVal.toDouble( &ok );
    QgsDebugMsg( QString( "exprVal Map XMin:%1" ).arg( minXD ) );
    if ( ok && !exprVal.isNull() )
    {
      useDdXMin = true;
      newExtent.setXMinimum( minXD );
    }
  }
  if ( dataDefinedEvaluate( QgsComposerObject::MapYMin, exprVal ) )
  {
    bool ok;
    minYD = exprVal.toDouble( &ok );
    QgsDebugMsg( QString( "exprVal Map YMin:%1" ).arg( minYD ) );
    if ( ok && !exprVal.isNull() )
    {
      useDdYMin = true;
      newExtent.setYMinimum( minYD );
    }
  }
  if ( dataDefinedEvaluate( QgsComposerObject::MapXMax, exprVal ) )
  {
    bool ok;
    maxXD = exprVal.toDouble( &ok );
    QgsDebugMsg( QString( "exprVal Map XMax:%1" ).arg( maxXD ) );
    if ( ok && !exprVal.isNull() )
    {
      useDdXMax = true;
      newExtent.setXMaximum( maxXD );
    }
  }
  if ( dataDefinedEvaluate( QgsComposerObject::MapYMax, exprVal ) )
  {
    bool ok;
    maxYD = exprVal.toDouble( &ok );
    QgsDebugMsg( QString( "exprVal Map YMax:%1" ).arg( maxYD ) );
    if ( ok && !exprVal.isNull() )
    {
      useDdYMax = true;
      newExtent.setYMaximum( maxYD );
    }
  }

  if ( newExtent != *currentMapExtent() )
  {
    //calculate new extents to fit data defined extents

    //Make sure the width/height ratio is the same as in current map extent.
    //This is to keep the map item frame and the page layout fixed
    double currentWidthHeightRatio = currentMapExtent()->width() / currentMapExtent()->height();
    double newWidthHeightRatio = newExtent.width() / newExtent.height();

    if ( currentWidthHeightRatio < newWidthHeightRatio )
    {
      //enlarge height of new extent, ensuring the map center stays the same
      double newHeight = newExtent.width() / currentWidthHeightRatio;
      double deltaHeight = newHeight - newExtent.height();
      newExtent.setYMinimum( newExtent.yMinimum() - deltaHeight / 2 );
      newExtent.setYMaximum( newExtent.yMaximum() + deltaHeight / 2 );
    }
    else
    {
      //enlarge width of new extent, ensuring the map center stays the same
      double newWidth = currentWidthHeightRatio * newExtent.height();
      double deltaWidth = newWidth - newExtent.width();
      newExtent.setXMinimum( newExtent.xMinimum() - deltaWidth / 2 );
      newExtent.setXMaximum( newExtent.xMaximum() + deltaWidth / 2 );
    }

    *currentMapExtent() = newExtent;
  }

  //now refresh scale, as this potentially overrides extents

  //data defined map scale set?
  if ( dataDefinedEvaluate( QgsComposerObject::MapScale, exprVal ) )
  {
    bool ok;
    double scaleD = exprVal.toDouble( &ok );
    QgsDebugMsg( QString( "exprVal Map Scale:%1" ).arg( scaleD ) );
    if ( ok && !exprVal.isNull() )
    {
      setNewScale( scaleD, false );
      newExtent = *currentMapExtent();
    }
  }

  if ( useDdXMax || useDdXMin || useDdYMax || useDdYMin )
  {
    //if only one of min/max was set for either x or y, then make sure our extent is locked on that value
    //as we can do this without altering the scale
    if ( useDdXMin && !useDdXMax )
    {
      double xMax = currentMapExtent()->xMaximum() - ( currentMapExtent()->xMinimum() - minXD );
      newExtent.setXMinimum( minXD );
      newExtent.setXMaximum( xMax );
    }
    else if ( !useDdXMin && useDdXMax )
    {
      double xMin = currentMapExtent()->xMinimum() - ( currentMapExtent()->xMaximum() - maxXD );
      newExtent.setXMinimum( xMin );
      newExtent.setXMaximum( maxXD );
    }
    if ( useDdYMin && !useDdYMax )
    {
      double yMax = currentMapExtent()->yMaximum() - ( currentMapExtent()->yMinimum() - minYD );
      newExtent.setYMinimum( minYD );
      newExtent.setYMaximum( yMax );
    }
    else if ( !useDdYMin && useDdYMax )
    {
      double yMin = currentMapExtent()->yMinimum() - ( currentMapExtent()->yMaximum() - maxYD );
      newExtent.setYMinimum( yMin );
      newExtent.setYMaximum( maxYD );
    }

    if ( newExtent != *currentMapExtent() )
    {
      *currentMapExtent() = newExtent;
    }
  }

  //lastly, map rotation overrides all
  double mapRotation = mMapRotation;

  //data defined map rotation set?
  if ( dataDefinedEvaluate( QgsComposerObject::MapRotation, exprVal ) )
  {
    bool ok;
    double rotationD = exprVal.toDouble( &ok );
    QgsDebugMsg( QString( "exprVal Map Rotation:%1" ).arg( rotationD ) );
    if ( ok && !exprVal.isNull() )
    {
      mapRotation = rotationD;
    }
  }

  if ( mEvaluatedMapRotation != mapRotation )
  {
    mEvaluatedMapRotation = mapRotation;
    emit mapRotationChanged( mapRotation );
  }

}

void QgsComposerMap::updateItem()
{
  if ( !mUpdatesEnabled )
  {
    return;
  }

  if ( mPreviewMode != QgsComposerMap::Rectangle && !mCacheUpdated )
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
  //check easy things first

  //overviews
  if ( mOverviewStack->containsAdvancedEffects() )
  {
    return true;
  }

  //grids
  if ( mGridStack->containsAdvancedEffects() )
  {
    return true;
  }

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

  //extent
  QDomElement extentElem = doc.createElement( "Extent" );
  extentElem.setAttribute( "xmin", qgsDoubleToString( mExtent.xMinimum() ) );
  extentElem.setAttribute( "xmax", qgsDoubleToString( mExtent.xMaximum() ) );
  extentElem.setAttribute( "ymin", qgsDoubleToString( mExtent.yMinimum() ) );
  extentElem.setAttribute( "ymax", qgsDoubleToString( mExtent.yMaximum() ) );
  composerMapElem.appendChild( extentElem );

  //map rotation
  composerMapElem.setAttribute( "mapRotation", QString::number( mMapRotation ) );

  //layer set
  QDomElement layerSetElem = doc.createElement( "LayerSet" );
  QStringList::const_iterator layerIt = mLayerSet.constBegin();
  for ( ; layerIt != mLayerSet.constEnd(); ++layerIt )
  {
    QDomElement layerElem = doc.createElement( "Layer" );
    QDomText layerIdText = doc.createTextNode( *layerIt );
    layerElem.appendChild( layerIdText );
    if ( mLayerStyleOverrides.contains( *layerIt ) )
      layerElem.setAttribute( "style", mLayerStyleOverrides[*layerIt] );
    layerSetElem.appendChild( layerElem );
  }
  composerMapElem.appendChild( layerSetElem );

  //write a dummy "Grid" element to prevent crashes on pre 2.5 versions (refs #10905)
  QDomElement gridElem = doc.createElement( "Grid" );
  composerMapElem.appendChild( gridElem );

  //grids
  mGridStack->writeXML( composerMapElem, doc );

  //overviews
  mOverviewStack->writeXML( composerMapElem, doc );

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
    updateToolTip();
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

  QString drawCanvasItemsFlag = itemElem.attribute( "drawCanvasItems", "true" );
  if ( drawCanvasItemsFlag.compare( "true", Qt::CaseInsensitive ) == 0 )
  {
    mDrawCanvasItems = true;
  }
  else
  {
    mDrawCanvasItems = false;
  }

  mLayerStyleOverrides.clear();

  //mLayerSet
  QDomNodeList layerSetNodeList = itemElem.elementsByTagName( "LayerSet" );
  QStringList layerSet;
  if ( layerSetNodeList.size() > 0 )
  {
    QDomElement layerSetElem = layerSetNodeList.at( 0 ).toElement();
    QDomNodeList layerIdNodeList = layerSetElem.elementsByTagName( "Layer" );
    for ( int i = 0; i < layerIdNodeList.size(); ++i )
    {
      const QDomElement& layerIdElement = layerIdNodeList.at( i ).toElement();
      layerSet << layerIdElement.text();
      if ( layerIdElement.hasAttribute( "style" ) )
        mLayerStyleOverrides.insert( layerSet.last(), layerIdElement.attribute( "style" ) );
    }
  }
  mLayerSet = layerSet;

  mDrawing = false;
  mNumCachedLayers = 0;
  mCacheUpdated = false;

  //overviews
  mOverviewStack->readXML( itemElem, doc );

  //grids
  mGridStack->readXML( itemElem, doc );

  //load grid / grid annotation in old xml format
  //only do this if the grid stack didn't load any grids, otherwise this will
  //be the dummy element created by QGIS >= 2.5 (refs #10905)
  QDomNodeList gridNodeList = itemElem.elementsByTagName( "Grid" );
  if ( mGridStack->size() == 0 && gridNodeList.size() > 0 )
  {
    QDomElement gridElem = gridNodeList.at( 0 ).toElement();
    QgsComposerMapGrid* mapGrid = new QgsComposerMapGrid( tr( "Grid %1" ).arg( 1 ), this );
    mapGrid->setEnabled( gridElem.attribute( "show", "0" ) != "0" );
    mapGrid->setStyle( QgsComposerMapGrid::GridStyle( gridElem.attribute( "gridStyle", "0" ).toInt() ) );
    mapGrid->setIntervalX( gridElem.attribute( "intervalX", "0" ).toDouble() );
    mapGrid->setIntervalY( gridElem.attribute( "intervalY", "0" ).toDouble() );
    mapGrid->setOffsetX( gridElem.attribute( "offsetX", "0" ).toDouble() );
    mapGrid->setOffsetY( gridElem.attribute( "offsetY", "0" ).toDouble() );
    mapGrid->setCrossLength( gridElem.attribute( "crossLength", "3" ).toDouble() );
    mapGrid->setFrameStyle(( QgsComposerMapGrid::FrameStyle )gridElem.attribute( "gridFrameStyle", "0" ).toInt() );
    mapGrid->setFrameWidth( gridElem.attribute( "gridFrameWidth", "2.0" ).toDouble() );
    mapGrid->setFramePenSize( gridElem.attribute( "gridFramePenThickness", "0.5" ).toDouble() );
    mapGrid->setFramePenColor( QgsSymbolLayerV2Utils::decodeColor( gridElem.attribute( "framePenColor", "0,0,0" ) ) );
    mapGrid->setFrameFillColor1( QgsSymbolLayerV2Utils::decodeColor( gridElem.attribute( "frameFillColor1", "255,255,255,255" ) ) );
    mapGrid->setFrameFillColor2( QgsSymbolLayerV2Utils::decodeColor( gridElem.attribute( "frameFillColor2", "0,0,0,255" ) ) );
    mapGrid->setBlendMode( QgsMapRenderer::getCompositionMode(( QgsMapRenderer::BlendMode ) itemElem.attribute( "gridBlendMode", "0" ).toUInt() ) );
    QDomElement gridSymbolElem = gridElem.firstChildElement( "symbol" );
    QgsLineSymbolV2* lineSymbol = 0;
    if ( gridSymbolElem.isNull() )
    {
      //old project file, read penWidth /penColorRed, penColorGreen, penColorBlue
      lineSymbol = QgsLineSymbolV2::createSimple( QgsStringMap() );
      lineSymbol->setWidth( gridElem.attribute( "penWidth", "0" ).toDouble() );
      lineSymbol->setColor( QColor( gridElem.attribute( "penColorRed", "0" ).toInt(),
                                    gridElem.attribute( "penColorGreen", "0" ).toInt(),
                                    gridElem.attribute( "penColorBlue", "0" ).toInt() ) );
    }
    else
    {
      lineSymbol = dynamic_cast<QgsLineSymbolV2*>( QgsSymbolLayerV2Utils::loadSymbol( gridSymbolElem ) );
    }
    mapGrid->setLineSymbol( lineSymbol );

    //annotation
    QDomNodeList annotationNodeList = gridElem.elementsByTagName( "Annotation" );
    if ( annotationNodeList.size() > 0 )
    {
      QDomElement annotationElem = annotationNodeList.at( 0 ).toElement();
      mapGrid->setAnnotationEnabled( annotationElem.attribute( "show", "0" ) != "0" );
      mapGrid->setAnnotationFormat( QgsComposerMapGrid::AnnotationFormat( annotationElem.attribute( "format", "0" ).toInt() ) );
      mapGrid->setAnnotationPosition( QgsComposerMapGrid::AnnotationPosition( annotationElem.attribute( "leftPosition", "0" ).toInt() ), QgsComposerMapGrid::Left );
      mapGrid->setAnnotationPosition( QgsComposerMapGrid::AnnotationPosition( annotationElem.attribute( "rightPosition", "0" ).toInt() ), QgsComposerMapGrid::Right );
      mapGrid->setAnnotationPosition( QgsComposerMapGrid::AnnotationPosition( annotationElem.attribute( "topPosition", "0" ).toInt() ), QgsComposerMapGrid::Top );
      mapGrid->setAnnotationPosition( QgsComposerMapGrid::AnnotationPosition( annotationElem.attribute( "bottomPosition", "0" ).toInt() ), QgsComposerMapGrid::Bottom );
      mapGrid->setAnnotationDirection( QgsComposerMapGrid::AnnotationDirection( annotationElem.attribute( "leftDirection", "0" ).toInt() ), QgsComposerMapGrid::Left );
      mapGrid->setAnnotationDirection( QgsComposerMapGrid::AnnotationDirection( annotationElem.attribute( "rightDirection", "0" ).toInt() ), QgsComposerMapGrid::Right );
      mapGrid->setAnnotationDirection( QgsComposerMapGrid::AnnotationDirection( annotationElem.attribute( "topDirection", "0" ).toInt() ), QgsComposerMapGrid::Top );
      mapGrid->setAnnotationDirection( QgsComposerMapGrid::AnnotationDirection( annotationElem.attribute( "bottomDirection", "0" ).toInt() ), QgsComposerMapGrid::Bottom );
      mapGrid->setAnnotationFrameDistance( annotationElem.attribute( "frameDistance", "0" ).toDouble() );
      QFont annotationFont;
      annotationFont.fromString( annotationElem.attribute( "font", "" ) );
      mapGrid->setAnnotationFont( annotationFont );
      mapGrid->setAnnotationFontColor( QgsSymbolLayerV2Utils::decodeColor( itemElem.attribute( "fontColor", "0,0,0,255" ) ) );

      mapGrid->setAnnotationPrecision( annotationElem.attribute( "precision", "3" ).toInt() );
    }
    mGridStack->addGrid( mapGrid );
  }

  //load overview in old xml format
  QDomElement overviewFrameElem = itemElem.firstChildElement( "overviewFrame" );
  if ( !overviewFrameElem.isNull() )
  {
    QgsComposerMapOverview* mapOverview = new QgsComposerMapOverview( tr( "Overview %1" ).arg( mOverviewStack->size() + 1 ), this );

    mapOverview->setFrameMap( overviewFrameElem.attribute( "overviewFrameMap", "-1" ).toInt() );
    mapOverview->setBlendMode( QgsMapRenderer::getCompositionMode(( QgsMapRenderer::BlendMode ) overviewFrameElem.attribute( "overviewBlendMode", "0" ).toUInt() ) );
    mapOverview->setInverted( overviewFrameElem.attribute( "overviewInverted" ).compare( "true", Qt::CaseInsensitive ) == 0 );
    mapOverview->setCentered( overviewFrameElem.attribute( "overviewCentered" ).compare( "true", Qt::CaseInsensitive ) == 0 );

    QgsFillSymbolV2* fillSymbol = 0;
    QDomElement overviewFrameSymbolElem = overviewFrameElem.firstChildElement( "symbol" );
    if ( !overviewFrameSymbolElem.isNull() )
    {
      fillSymbol = dynamic_cast<QgsFillSymbolV2*>( QgsSymbolLayerV2Utils::loadSymbol( overviewFrameSymbolElem ) );
      mapOverview->setFrameSymbol( fillSymbol );
    }
    mOverviewStack->addOverview( mapOverview );
  }

  //atlas
  QDomNodeList atlasNodeList = itemElem.elementsByTagName( "AtlasMap" );
  if ( atlasNodeList.size() > 0 )
  {
    QDomElement atlasElem = atlasNodeList.at( 0 ).toElement();
    mAtlasDriven = ( atlasElem.attribute( "atlasDriven", "0" ) != "0" );
    if ( atlasElem.hasAttribute( "fixedScale" ) ) // deprecated XML
    {
      mAtlasScalingMode = ( atlasElem.attribute( "fixedScale", "0" ) != "0" ) ? Fixed : Auto;
    }
    else if ( atlasElem.hasAttribute( "scalingMode" ) )
    {
      mAtlasScalingMode = static_cast<AtlasScalingMode>( atlasElem.attribute( "scalingMode" ).toInt() );
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

  // also store styles associated with the layers
  mLayerStyleOverrides.clear();
  foreach ( const QString& layerID, mLayerSet )
  {
    if ( QgsMapLayer* ml = QgsMapLayerRegistry::instance()->mapLayer( layerID ) )
      mLayerStyleOverrides.insert( layerID, ml->styleManager()->currentStyle() );
  }
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
      mLayerStyleOverrides.remove( mLayerSet.at( i ) );
      mLayerSet.removeAt( i );
    }
  }
}

QgsComposerMapGrid* QgsComposerMap::grid()
{
  if ( mGridStack->size() < 1 )
  {
    QgsComposerMapGrid* grid = new QgsComposerMapGrid( tr( "Grid %1" ).arg( 1 ), this );
    mGridStack->addGrid( grid );
  }
  return mGridStack->grid( 0 );
}

const QgsComposerMapGrid* QgsComposerMap::constFirstMapGrid() const
{
  return const_cast<QgsComposerMap*>( this )->grid();
}

void QgsComposerMap::setGridStyle( GridStyle style )
{
  QgsComposerMapGrid* g = grid();
  g->setStyle( QgsComposerMapGrid::GridStyle( style ) );
}

QgsComposerMap::GridStyle QgsComposerMap::gridStyle() const
{
  const QgsComposerMapGrid* g = constFirstMapGrid();
  return ( QgsComposerMap::GridStyle )g->style();
}

void QgsComposerMap::setGridIntervalX( double interval )
{
  QgsComposerMapGrid* g = grid();
  g->setIntervalX( interval );
}

double QgsComposerMap::gridIntervalX() const
{
  const QgsComposerMapGrid* g = constFirstMapGrid();
  return g->intervalX();
}

void QgsComposerMap::setGridIntervalY( double interval )
{
  QgsComposerMapGrid* g = grid();
  g->setIntervalY( interval );
}

double QgsComposerMap::gridIntervalY() const
{
  const QgsComposerMapGrid* g = constFirstMapGrid();
  return g->intervalY();
}

void QgsComposerMap::setGridOffsetX( double offset )
{
  QgsComposerMapGrid* g = grid();
  g->setOffsetX( offset );
}

double QgsComposerMap::gridOffsetX() const
{
  const QgsComposerMapGrid* g = constFirstMapGrid();
  return g->offsetX();
}

void QgsComposerMap::setGridOffsetY( double offset )
{
  QgsComposerMapGrid* g = grid();
  g->setOffsetY( offset );
}

double QgsComposerMap::gridOffsetY() const
{
  const QgsComposerMapGrid* g = constFirstMapGrid();
  return g->offsetY();
}

void QgsComposerMap::setGridPenWidth( double w )
{
  QgsComposerMapGrid* g = grid();
  g->setGridLineWidth( w );
}

void QgsComposerMap::setGridPenColor( const QColor& c )
{
  QgsComposerMapGrid* g = grid();
  g->setGridLineColor( c );
}

void QgsComposerMap::setGridPen( const QPen& p )
{
  QgsComposerMapGrid* g = grid();
  g->setGridLineWidth( p.widthF() );
  g->setGridLineColor( p.color() );
}

QPen QgsComposerMap::gridPen() const
{
  const QgsComposerMapGrid* g = constFirstMapGrid();
  QPen p;
  if ( g->lineSymbol() )
  {
    QgsLineSymbolV2* line = dynamic_cast<QgsLineSymbolV2*>( g->lineSymbol()->clone() );
    p.setWidthF( line->width() );
    p.setColor( line->color() );
    p.setCapStyle( Qt::FlatCap );
    delete line;
  }
  return p;
}

void QgsComposerMap::setGridAnnotationFont( const QFont& f )
{
  QgsComposerMapGrid* g = grid();
  g->setAnnotationFont( f );
}

QFont QgsComposerMap::gridAnnotationFont() const
{
  const QgsComposerMapGrid* g = constFirstMapGrid();
  return g->annotationFont();
}

void QgsComposerMap::setAnnotationFontColor( const QColor& c )
{
  QgsComposerMapGrid* g = grid();
  g->setAnnotationFontColor( c );
}

QColor QgsComposerMap::annotationFontColor() const
{
  const QgsComposerMapGrid* g = constFirstMapGrid();
  return g->annotationFontColor();
}

void QgsComposerMap::setGridAnnotationPrecision( int p )
{
  QgsComposerMapGrid* g = grid();
  g->setAnnotationPrecision( p );
}

int QgsComposerMap::gridAnnotationPrecision() const
{
  const QgsComposerMapGrid* g = constFirstMapGrid();
  return g->annotationPrecision();
}

void QgsComposerMap::setShowGridAnnotation( bool show )
{
  QgsComposerMapGrid* g = grid();
  g->setAnnotationEnabled( show );
}

bool QgsComposerMap::showGridAnnotation() const
{
  const QgsComposerMapGrid* g = constFirstMapGrid();
  return g->annotationEnabled();
}

void QgsComposerMap::setGridAnnotationPosition( QgsComposerMap::GridAnnotationPosition p, QgsComposerMap::Border border )
{
  QgsComposerMapGrid* g = grid();
  if ( p != QgsComposerMap::Disabled )
  {
    g->setAnnotationPosition(( QgsComposerMapGrid::AnnotationPosition )p, ( QgsComposerMapGrid::BorderSide )border );
  }
  else
  {
    g->setAnnotationDisplay( QgsComposerMapGrid::HideAll, ( QgsComposerMapGrid::BorderSide )border );
  }
}

QgsComposerMap::GridAnnotationPosition QgsComposerMap::gridAnnotationPosition( QgsComposerMap::Border border ) const
{
  const QgsComposerMapGrid* g = constFirstMapGrid();
  return ( QgsComposerMap::GridAnnotationPosition )g->annotationPosition(( QgsComposerMapGrid::BorderSide )border );
}

void QgsComposerMap::setAnnotationFrameDistance( double d )
{
  QgsComposerMapGrid* g = grid();
  g->setAnnotationFrameDistance( d );
}

double QgsComposerMap::annotationFrameDistance() const
{
  const QgsComposerMapGrid* g = constFirstMapGrid();
  return g->annotationFrameDistance();
}

void QgsComposerMap::setGridAnnotationDirection( GridAnnotationDirection d, QgsComposerMap::Border border )
{
  QgsComposerMapGrid* g = grid();
  //map grid direction to QgsComposerMapGrid direction (values are different)
  QgsComposerMapGrid::AnnotationDirection gridDirection;
  switch ( d )
  {
    case QgsComposerMap::Horizontal:
      gridDirection = QgsComposerMapGrid::Horizontal;
      break;
    case QgsComposerMap::Vertical:
      gridDirection = QgsComposerMapGrid::Vertical;
      break;
    case QgsComposerMap::BoundaryDirection:
      gridDirection = QgsComposerMapGrid::BoundaryDirection;
      break;
    default:
      gridDirection = QgsComposerMapGrid::Horizontal;
  }
  g->setAnnotationDirection( gridDirection, ( QgsComposerMapGrid::BorderSide )border );

}

QgsComposerMap::GridAnnotationDirection QgsComposerMap::gridAnnotationDirection( QgsComposerMap::Border border ) const
{
  const QgsComposerMapGrid* g = constFirstMapGrid();
  return ( QgsComposerMap::GridAnnotationDirection )g->annotationDirection(( QgsComposerMapGrid::BorderSide )border );
}

void QgsComposerMap::setGridAnnotationFormat( QgsComposerMap::GridAnnotationFormat f )
{
  QgsComposerMapGrid* g = grid();
  g->setAnnotationFormat(( QgsComposerMapGrid::AnnotationFormat )f );
}

QgsComposerMap::GridAnnotationFormat QgsComposerMap::gridAnnotationFormat() const
{
  const QgsComposerMapGrid* g = constFirstMapGrid();
  return ( QgsComposerMap::GridAnnotationFormat )g->annotationFormat();
}

void QgsComposerMap::setGridFrameStyle( GridFrameStyle style )
{
  QgsComposerMapGrid* g = grid();
  g->setFrameStyle(( QgsComposerMapGrid::FrameStyle )style );
}

QgsComposerMap::GridFrameStyle QgsComposerMap::gridFrameStyle() const
{
  const QgsComposerMapGrid* g = constFirstMapGrid();
  return ( QgsComposerMap::GridFrameStyle )g->frameStyle();
}

void QgsComposerMap::setGridFrameWidth( double w )
{
  QgsComposerMapGrid* g = grid();
  g->setFrameWidth( w );
}

double QgsComposerMap::gridFrameWidth() const
{
  const QgsComposerMapGrid* g = constFirstMapGrid();
  return g->frameWidth();
}

void QgsComposerMap::setGridFramePenSize( double w )
{
  QgsComposerMapGrid* g = grid();
  g->setFramePenSize( w );
}

double QgsComposerMap::gridFramePenSize() const
{
  const QgsComposerMapGrid* g = constFirstMapGrid();
  return g->framePenSize();
}

void QgsComposerMap::setGridFramePenColor( const QColor& c )
{
  QgsComposerMapGrid* g = grid();
  g->setFramePenColor( c );
}

QColor QgsComposerMap::gridFramePenColor() const
{
  const QgsComposerMapGrid* g = constFirstMapGrid();
  return g->framePenColor();
}

void QgsComposerMap::setGridFrameFillColor1( const QColor& c )
{
  QgsComposerMapGrid* g = grid();
  g->setFrameFillColor1( c );
}

QColor QgsComposerMap::gridFrameFillColor1() const
{
  const QgsComposerMapGrid* g = constFirstMapGrid();
  return g->frameFillColor1();
}

void QgsComposerMap::setGridFrameFillColor2( const QColor& c )
{
  QgsComposerMapGrid* g = grid();
  g->setFrameFillColor2( c );
}

QColor QgsComposerMap::gridFrameFillColor2() const
{
  const QgsComposerMapGrid* g = constFirstMapGrid();
  return g->frameFillColor2();
}

void QgsComposerMap::setCrossLength( double l )
{
  QgsComposerMapGrid* g = grid();
  g->setCrossLength( l );
}

double QgsComposerMap::crossLength()
{
  const QgsComposerMapGrid* g = constFirstMapGrid();
  return g->crossLength();
}

QgsComposerMapOverview *QgsComposerMap::overview()
{
  if ( mOverviewStack->size() < 1 )
  {
    QgsComposerMapOverview* overview = new QgsComposerMapOverview( tr( "Overview %1" ).arg( 1 ), this );
    mOverviewStack->addOverview( overview );
  }
  return mOverviewStack->overview( 0 );
}

const QgsComposerMapOverview *QgsComposerMap::constFirstMapOverview() const
{
  return const_cast<QgsComposerMap*>( this )->overview();
}

void QgsComposerMap::setGridBlendMode( QPainter::CompositionMode blendMode )
{
  QgsComposerMapGrid* g = grid();
  g->setBlendMode( blendMode );
}

QPainter::CompositionMode QgsComposerMap::gridBlendMode() const
{
  const QgsComposerMapGrid* g = constFirstMapGrid();
  return g->blendMode();
}

QRectF QgsComposerMap::boundingRect() const
{
  return mCurrentRectangle;
}

void QgsComposerMap::updateBoundingRect()
{
  QRectF rectangle = rect();
  double frameExtension = mFrame ? pen().widthF() / 2.0 : 0.0;
  double maxGridExtension = mGridStack ? mGridStack->maxGridExtension() : 0;

  double maxExtension = qMax( frameExtension, maxGridExtension );

  rectangle.setLeft( rectangle.left() - maxExtension );
  rectangle.setRight( rectangle.right() + maxExtension );
  rectangle.setTop( rectangle.top() - maxExtension );
  rectangle.setBottom( rectangle.bottom() + maxExtension );
  if ( rectangle != mCurrentRectangle )
  {
    prepareGeometryChange();
    mCurrentRectangle = rectangle;
  }
}

void QgsComposerMap::setFrameOutlineWidth( const double outlineWidth )
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
  QPolygonF poly = visibleExtentPolygon();
  poly.translate( -dx, -dy );
  return poly;
}

void QgsComposerMap::mapPolygon( const QgsRectangle& extent, QPolygonF& poly ) const
{
  poly.clear();
  if ( mEvaluatedMapRotation == 0 )
  {
    poly << QPointF( extent.xMinimum(), extent.yMaximum() );
    poly << QPointF( extent.xMaximum(), extent.yMaximum() );
    poly << QPointF( extent.xMaximum(), extent.yMinimum() );
    poly << QPointF( extent.xMinimum(), extent.yMinimum() );
    //ensure polygon is closed by readding first point
    poly << QPointF( poly.at( 0 ) );
    return;
  }

  //there is rotation
  QgsPoint rotationPoint(( extent.xMaximum() + extent.xMinimum() ) / 2.0, ( extent.yMaximum() + extent.yMinimum() ) / 2.0 );
  double dx, dy; //x-, y- shift from rotation point to corner point

  //top left point
  dx = rotationPoint.x() - extent.xMinimum();
  dy = rotationPoint.y() - extent.yMaximum();
  QgsComposerUtils::rotate( mEvaluatedMapRotation, dx, dy );
  poly << QPointF( rotationPoint.x() - dx, rotationPoint.y() - dy );

  //top right point
  dx = rotationPoint.x() - extent.xMaximum();
  dy = rotationPoint.y() - extent.yMaximum();
  QgsComposerUtils::rotate( mEvaluatedMapRotation, dx, dy );
  poly << QPointF( rotationPoint.x() - dx, rotationPoint.y() - dy );

  //bottom right point
  dx = rotationPoint.x() - extent.xMaximum();
  dy = rotationPoint.y() - extent.yMinimum();
  QgsComposerUtils::rotate( mEvaluatedMapRotation, dx, dy );
  poly << QPointF( rotationPoint.x() - dx, rotationPoint.y() - dy );

  //bottom left point
  dx = rotationPoint.x() - extent.xMinimum();
  dy = rotationPoint.y() - extent.yMinimum();
  QgsComposerUtils::rotate( mEvaluatedMapRotation, dx, dy );
  poly << QPointF( rotationPoint.x() - dx, rotationPoint.y() - dy );

  //ensure polygon is closed by readding first point
  poly << QPointF( poly.at( 0 ) );
}

QPolygonF QgsComposerMap::visibleExtentPolygon() const
{
  QPolygonF poly;
  mapPolygon( *currentMapExtent(), poly );
  return poly;
}

QString QgsComposerMap::displayName() const
{
  if ( !QgsComposerItem::id().isEmpty() )
  {
    return QgsComposerItem::id();
  }

  return tr( "Map %1" ).arg( mId );
}

void QgsComposerMap::requestedExtent( QgsRectangle& extent ) const
{
  QgsRectangle newExtent = *currentMapExtent();
  if ( mEvaluatedMapRotation == 0 )
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
  QgsComposerMapOverview* o = overview();
  o->setFrameMap( mapId );
}

int QgsComposerMap::overviewFrameMapId() const
{
  const QgsComposerMapOverview* o = constFirstMapOverview();
  return o->frameMapId();
}

void QgsComposerMap::refreshDataDefinedProperty( const QgsComposerObject::DataDefinedProperty property )
{
  //updates data defined properties and redraws item to match
  if ( property == QgsComposerObject::MapRotation || property == QgsComposerObject::MapScale ||
       property == QgsComposerObject::MapXMin || property == QgsComposerObject::MapYMin ||
       property == QgsComposerObject::MapXMax || property == QgsComposerObject::MapYMax ||
       property == QgsComposerObject::MapAtlasMargin ||
       property == QgsComposerObject::AllProperties )
  {
    QgsRectangle beforeExtent = *currentMapExtent();
    refreshMapExtents();
    emit itemChanged();
    if ( *currentMapExtent() != beforeExtent )
    {
      emit extentChanged();
    }
  }

  //force redraw
  mCacheUpdated = false;

  QgsComposerItem::refreshDataDefinedProperty( property );
}

void QgsComposerMap::setOverviewFrameMapSymbol( QgsFillSymbolV2* symbol )
{
  QgsComposerMapOverview* o = overview();
  o->setFrameSymbol( symbol );
}

QgsFillSymbolV2 *QgsComposerMap::overviewFrameMapSymbol()
{
  QgsComposerMapOverview* o = overview();
  return o->frameSymbol();
}

QPainter::CompositionMode QgsComposerMap::overviewBlendMode() const
{
  const QgsComposerMapOverview* o = constFirstMapOverview();
  return o->blendMode();
}

void QgsComposerMap::setOverviewBlendMode( QPainter::CompositionMode blendMode )
{
  QgsComposerMapOverview* o = overview();
  o->setBlendMode( blendMode );
}

bool QgsComposerMap::overviewInverted() const
{
  const QgsComposerMapOverview* o = constFirstMapOverview();
  return o->inverted();
}

void QgsComposerMap::setOverviewInverted( bool inverted )
{
  QgsComposerMapOverview* o = overview();
  o->setInverted( inverted );
}

bool QgsComposerMap::overviewCentered() const
{
  const QgsComposerMapOverview* o = constFirstMapOverview();
  return o->centered();
}

void QgsComposerMap::setOverviewCentered( bool centered )
{
  QgsComposerMapOverview* o = overview();
  o->setCentered( centered );
  //overviewExtentChanged();
}

void QgsComposerMap::setGridLineSymbol( QgsLineSymbolV2* symbol )
{
  QgsComposerMapGrid* g = grid();
  g->setLineSymbol( symbol );
}

QgsLineSymbolV2* QgsComposerMap::gridLineSymbol()
{
  QgsComposerMapGrid* g = grid();
  return g->lineSymbol();
}

void QgsComposerMap::setGridEnabled( bool enabled )
{
  QgsComposerMapGrid* g = grid();
  g->setEnabled( enabled );
}

bool QgsComposerMap::gridEnabled() const
{
  const QgsComposerMapGrid* g = constFirstMapGrid();
  return g->enabled();
}

void QgsComposerMap::transformShift( double& xShift, double& yShift ) const
{
  double mmToMapUnits = 1.0 / mapUnitsToMM();
  double dxScaled = xShift * mmToMapUnits;
  double dyScaled = - yShift * mmToMapUnits;

  QgsComposerUtils::rotate( mEvaluatedMapRotation, dxScaled, dyScaled );

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
  QgsComposerUtils::rotate( -mEvaluatedMapRotation, dx, dy );
  QgsPoint backRotatedCoords( rotationPoint.x() + dx, rotationPoint.y() + dy );

  QgsRectangle unrotatedExtent = transformedExtent();
  double xItem = rect().width() * ( backRotatedCoords.x() - unrotatedExtent.xMinimum() ) / unrotatedExtent.width();
  double yItem = rect().height() * ( 1 - ( backRotatedCoords.y() - unrotatedExtent.yMinimum() ) / unrotatedExtent.height() );
  return QPointF( xItem, yItem );
}

void QgsComposerMap::connectMapOverviewSignals()
{

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
  painter->setRenderHint( QPainter::Antialiasing );

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
  updateToolTip();
}

bool QgsComposerMap::imageSizeConsideringRotation( double& width, double& height ) const
{
  //kept for api compatibility with QGIS 2.0 - use mMapRotation
  Q_NOWARN_DEPRECATED_PUSH
  return QgsComposerItem::imageSizeConsideringRotation( width, height, mEvaluatedMapRotation );
  Q_NOWARN_DEPRECATED_POP
}

bool QgsComposerMap::cornerPointOnRotatedAndScaledRect( double& x, double& y, double width, double height ) const
{
  //kept for api compatibility with QGIS 2.0 - use mMapRotation
  Q_NOWARN_DEPRECATED_PUSH
  return QgsComposerItem::cornerPointOnRotatedAndScaledRect( x, y, width, height, mEvaluatedMapRotation );
  Q_NOWARN_DEPRECATED_POP
}

void QgsComposerMap::sizeChangedByRotation( double& width, double& height )
{
  //kept for api compatibility with QGIS 2.0 - use mMapRotation
  Q_NOWARN_DEPRECATED_PUSH
  return QgsComposerItem::sizeChangedByRotation( width, height, mEvaluatedMapRotation );
  Q_NOWARN_DEPRECATED_POP
}

void QgsComposerMap::setAtlasDriven( bool enabled )
{
  mAtlasDriven = enabled;

  if ( !enabled )
  {
    //if not enabling the atlas, we still need to refresh the map extents
    //so that data defined extents and scale are recalculated
    refreshMapExtents();
  }
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

double QgsComposerMap::atlasMargin( const QgsComposerObject::PropertyValueType valueType )
{
  if ( valueType == QgsComposerObject::EvaluatedValue )
  {
    //evaluate data defined atlas margin

    //start with user specified margin
    double margin = mAtlasMargin;
    QVariant exprVal;
    if ( dataDefinedEvaluate( QgsComposerObject::MapAtlasMargin, exprVal ) )
    {
      bool ok;
      double ddMargin = exprVal.toDouble( &ok );
      QgsDebugMsg( QString( "exprVal Map Atlas Margin:%1" ).arg( ddMargin ) );
      if ( ok && !exprVal.isNull() )
      {
        //divide by 100 to convert to 0 -> 1.0 range
        margin = ddMargin / 100;
      }
    }
    return margin;
  }
  else
  {
    return mAtlasMargin;
  }
}

