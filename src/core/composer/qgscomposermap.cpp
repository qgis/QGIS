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
#include "qgslayertree.h"
#include "qgslogger.h"
#include "qgsmaprenderercustompainterjob.h"
#include "qgsmaplayerlistutils.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsmaptopixel.h"
#include "qgsmapsettingsutils.h"
#include "qgspainting.h"
#include "qgspathresolver.h"
#include "qgsproject.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterlayer.h"
#include "qgsreadwritecontext.h"
#include "qgsrendercontext.h"
#include "qgsscalecalculator.h"
#include "qgsvectorlayer.h"
#include "qgspallabeling.h"
#include "qgsexpression.h"
#include "qgsmapthemecollection.h"
#include "qgsannotation.h"
#include "qgsannotationmanager.h"

#include "qgssymbollayerutils.h" //for pointOnLineWithDistance

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPainter>
#include <QSettings>
#include <cmath>

QgsComposerMap::QgsComposerMap( QgsComposition *composition, int x, int y, int width, int height )
  : QgsComposerItem( x, y, width, height, composition )
{
  assignFreeId();

  mCurrentRectangle = rect();

  QgsProject *project = mComposition->project();

  //get the color for map canvas background and set map background color accordingly
  int bgRedInt = project->readNumEntry( QStringLiteral( "Gui" ), QStringLiteral( "/CanvasColorRedPart" ), 255 );
  int bgGreenInt = project->readNumEntry( QStringLiteral( "Gui" ), QStringLiteral( "/CanvasColorGreenPart" ), 255 );
  int bgBlueInt = project->readNumEntry( QStringLiteral( "Gui" ), QStringLiteral( "/CanvasColorBluePart" ), 255 );
  setBackgroundColor( QColor( bgRedInt, bgGreenInt, bgBlueInt ) );

  init();

  setSceneRect( QRectF( x, y, width, height ) );
}

QgsComposerMap::QgsComposerMap( QgsComposition *composition )
  : QgsComposerItem( 0, 0, 10, 10, composition )
{
  mId = mComposition->composerMapItems().size();
  mCurrentRectangle = rect();

  init();
  updateToolTip();
}

void QgsComposerMap::init()
{
  mGridStack = new QgsComposerMapGridStack( this );
  mOverviewStack = new QgsComposerMapOverviewStack( this );
  connectUpdateSlot();
}

void QgsComposerMap::updateToolTip()
{
  setToolTip( tr( "Map %1" ).arg( mId ) );
}

void QgsComposerMap::adjustExtentToItemShape( double itemWidth, double itemHeight, QgsRectangle &extent ) const
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

  if ( mPainterJob )
  {
    disconnect( mPainterJob.get(), &QgsMapRendererCustomPainterJob::finished, this, &QgsComposerMap::painterJobFinished );
    mPainterJob->cancel();
    mPainter->end();
  }
}

/* This function is called by paint() to render the map.  It does not override any functions
from QGraphicsItem. */
void QgsComposerMap::draw( QPainter *painter, const QgsRectangle &extent, QSizeF size, double dpi, double *forceWidthScale )
{
  Q_UNUSED( forceWidthScale );

  if ( !painter )
  {
    return;
  }
  if ( qgsDoubleNear( size.width(), 0.0 ) || qgsDoubleNear( size.height(), 0.0 ) )
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

QgsMapSettings QgsComposerMap::mapSettings( const QgsRectangle &extent, QSizeF size, int dpi ) const
{
  QgsExpressionContext expressionContext = createExpressionContext();
  QgsCoordinateReferenceSystem renderCrs = crs();

  QgsMapSettings jobMapSettings;
  jobMapSettings.setDestinationCrs( renderCrs );
  jobMapSettings.setExtent( extent );
  jobMapSettings.setOutputSize( size.toSize() );
  jobMapSettings.setOutputDpi( dpi );
  jobMapSettings.setBackgroundColor( Qt::transparent );
  jobMapSettings.setRotation( mEvaluatedMapRotation );
  jobMapSettings.setEllipsoid( mComposition->project()->ellipsoid() );

  //set layers to render
  QList<QgsMapLayer *> layers = layersToRender( &expressionContext );
  if ( -1 != mCurrentExportLayer )
  {
    const int layerIdx = mCurrentExportLayer - ( hasBackground() ? 1 : 0 );
    if ( layerIdx >= 0 && layerIdx < layers.length() )
    {
      // exporting with separate layers (e.g., to svg layers), so we only want to render a single map layer
      QgsMapLayer *ml = layers[ layers.length() - layerIdx - 1 ];
      layers.clear();
      layers << ml;
    }
    else
    {
      // exporting decorations such as map frame/grid/overview, so no map layers required
      layers.clear();
    }
  }
  jobMapSettings.setLayers( layers );
  jobMapSettings.setLayerStyleOverrides( layerStyleOverridesToRender( expressionContext ) );

  if ( mComposition->plotStyle() == QgsComposition::Print ||
       mComposition->plotStyle() == QgsComposition::Postscript )
  {
    //if outputting composer, disable optimisations like layer simplification
    jobMapSettings.setFlag( QgsMapSettings::UseRenderingOptimization, false );
  }

  QgsExpressionContext context = createExpressionContext();
  jobMapSettings.setExpressionContext( context );

  // composer-specific overrides of flags
  jobMapSettings.setFlag( QgsMapSettings::ForceVectorOutput, true ); // force vector output (no caching of marker images etc.)
  jobMapSettings.setFlag( QgsMapSettings::Antialiasing, true );
  jobMapSettings.setFlag( QgsMapSettings::DrawEditingInfo, false );
  jobMapSettings.setFlag( QgsMapSettings::DrawSelection, false );
  jobMapSettings.setFlag( QgsMapSettings::UseAdvancedEffects, mComposition->useAdvancedEffects() ); // respect the composition's useAdvancedEffects flag

  jobMapSettings.datumTransformStore().setDestinationCrs( renderCrs );

  jobMapSettings.setLabelingEngineSettings( mComposition->project()->labelingEngineSettings() );

  return jobMapSettings;
}

void QgsComposerMap::recreateCachedImageInBackground()
{
  if ( mPainterJob )
  {
    disconnect( mPainterJob.get(), &QgsMapRendererCustomPainterJob::finished, this, &QgsComposerMap::painterJobFinished );
    QgsMapRendererCustomPainterJob *oldJob = mPainterJob.release();
    QPainter *oldPainter = mPainter.release();
    QImage *oldImage = mCacheRenderingImage.release();
    connect( oldJob, &QgsMapRendererCustomPainterJob::finished, this, [oldPainter, oldJob, oldImage]
    {
      oldJob->deleteLater();
      delete oldPainter;
      delete oldImage;
    } );
    oldJob->cancelWithoutBlocking();
  }
  else
  {
    mCacheRenderingImage.reset( nullptr );
  }

  Q_ASSERT( !mPainterJob );
  Q_ASSERT( !mPainter );
  Q_ASSERT( !mCacheRenderingImage );

  double horizontalVScaleFactor = horizontalViewScaleFactor();
  if ( horizontalVScaleFactor < 0 )
  {
    //make sure scale factor is positive
    horizontalVScaleFactor = mLastValidViewScaleFactor > 0 ? mLastValidViewScaleFactor : 1;
  }

  const QgsRectangle &ext = *currentMapExtent();
  double widthMM = ext.width() * mapUnitsToMM();
  double heightMM = ext.height() * mapUnitsToMM();

  int w = widthMM * horizontalVScaleFactor;
  int h = heightMM * horizontalVScaleFactor;

  // limit size of image for better performance
  if ( w > 5000 || h > 5000 )
  {
    if ( w > h )
    {
      w = 5000;
      h = w * heightMM / widthMM;
    }
    else
    {
      h = 5000;
      w = h * widthMM / heightMM;
    }
  }

  if ( w <= 0 || h <= 0 )
    return;

  mCacheRenderingImage.reset( new QImage( w, h, QImage::Format_ARGB32 ) );

  // set DPI of the image
  mCacheRenderingImage->setDotsPerMeterX( 1000 * w / widthMM );
  mCacheRenderingImage->setDotsPerMeterY( 1000 * h / heightMM );

  if ( hasBackground() )
  {
    //Initially fill image with specified background color. This ensures that layers with blend modes will
    //preview correctly
    mCacheRenderingImage->fill( backgroundColor().rgba() );
  }
  else
  {
    //no background, but start with empty fill to avoid artifacts
    mCacheRenderingImage->fill( QColor( 255, 255, 255, 0 ).rgba() );
  }

  mCacheInvalidated = false;
  mPainter.reset( new QPainter( mCacheRenderingImage.get() ) );
  QgsMapSettings settings( mapSettings( ext, QSizeF( w, h ), mCacheRenderingImage->logicalDpiX() ) );
  mPainterJob.reset( new QgsMapRendererCustomPainterJob( settings, mPainter.get() ) );
  connect( mPainterJob.get(), &QgsMapRendererCustomPainterJob::finished, this, &QgsComposerMap::painterJobFinished );
  mPainterJob->start();
}

void QgsComposerMap::painterJobFinished()
{
  mPainter->end();
  mPainterJob.reset( nullptr );
  mPainter.reset( nullptr );
  mCacheFinalImage = std::move( mCacheRenderingImage );
  mLastRenderedImageOffsetX = 0;
  mLastRenderedImageOffsetY = 0;
  updateItem();
}

void QgsComposerMap::paint( QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *pWidget )
{
  Q_UNUSED( pWidget );

  if ( !mComposition || !painter || !painter->device() )
  {
    return;
  }
  if ( !shouldDrawItem() )
  {
    return;
  }

  QRectF thisPaintRect = QRectF( 0, 0, QGraphicsRectItem::rect().width(), QGraphicsRectItem::rect().height() );
  if ( thisPaintRect.width() == 0 || thisPaintRect.height() == 0 )
    return;

  painter->save();
  painter->setClipRect( thisPaintRect );

  if ( mComposition->plotStyle() == QgsComposition::Preview )
  {
    if ( !mCacheFinalImage || mCacheFinalImage->isNull() )
    {
      // No initial render available - so draw some preview text alerting user
      drawBackground( painter );
      painter->setBrush( QBrush( QColor( 125, 125, 125, 125 ) ) );
      painter->drawRect( thisPaintRect );
      painter->setBrush( Qt::NoBrush );
      QFont messageFont;
      messageFont.setPointSize( 12 );
      painter->setFont( messageFont );
      painter->setPen( QColor( 255, 255, 255, 255 ) );
      painter->drawText( thisPaintRect, Qt::AlignCenter | Qt::AlignHCenter, tr( "Rendering map" ) );
      if ( !mPainterJob )
      {
        // this is the map's very first paint - trigger a cache update
        recreateCachedImageInBackground();
      }
    }
    else
    {
      if ( mCacheInvalidated )
      {
        // cache was invalidated - trigger a background update
        recreateCachedImageInBackground();
      }

      //Background color is already included in cached image, so no need to draw

      double imagePixelWidth = mCacheFinalImage->width(); //how many pixels of the image are for the map extent?
      double scale = rect().width() / imagePixelWidth;

      painter->save();

      painter->translate( mLastRenderedImageOffsetX + mXOffset, mLastRenderedImageOffsetY + mYOffset );
      painter->scale( scale, scale );
      painter->drawImage( 0, 0, *mCacheFinalImage );

      //restore rotation
      painter->restore();
    }
  }
  else if ( mComposition->plotStyle() == QgsComposition::Print ||
            mComposition->plotStyle() == QgsComposition::Postscript )
  {
    if ( mDrawing )
    {
      return;
    }

    mDrawing = true;
    QPaintDevice *paintDevice = painter->device();
    if ( !paintDevice )
    {
      return;
    }

    // Fill with background color
    if ( shouldDrawPart( Background ) )
    {
      drawBackground( painter );
    }

    QgsRectangle cExtent = *currentMapExtent();

    QSizeF size( cExtent.width() * mapUnitsToMM(), cExtent.height() * mapUnitsToMM() );

    painter->save();
    painter->translate( mXOffset, mYOffset );

    double dotsPerMM = paintDevice->logicalDpiX() / 25.4;
    size *= dotsPerMM; // output size will be in dots (pixels)
    painter->scale( 1 / dotsPerMM, 1 / dotsPerMM ); // scale painter from mm to dots
    draw( painter, cExtent, size, paintDevice->logicalDpiX() );

    //restore rotation
    painter->restore();
    mDrawing = false;
  }

  painter->setClipRect( thisPaintRect, Qt::NoClip );
  if ( shouldDrawPart( OverviewMapExtent ) )
  {
    mOverviewStack->drawItems( painter );
  }
  if ( shouldDrawPart( Grid ) )
  {
    mGridStack->drawItems( painter );
  }

  //draw canvas items
  drawAnnotations( painter );

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

void QgsComposerMap::invalidateCache()
{
  mCacheInvalidated = true;
  updateItem();
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

QList<QgsMapLayer *> QgsComposerMap::layersToRender( const QgsExpressionContext *context ) const
{
  QgsExpressionContext scopedContext = createExpressionContext();
  const QgsExpressionContext *evalContext = context ? context : &scopedContext;

  QList<QgsMapLayer *> renderLayers;

  if ( mFollowVisibilityPreset )
  {
    QString presetName = mFollowVisibilityPresetName;

    // preset name can be overridden by data-defined one
    presetName = mDataDefinedProperties.valueAsString( QgsComposerObject::MapStylePreset, *evalContext, presetName );

    if ( mComposition->project()->mapThemeCollection()->hasMapTheme( presetName ) )
      renderLayers = mComposition->project()->mapThemeCollection()->mapThemeVisibleLayers( presetName );
    else  // fallback to using map canvas layers
      renderLayers = mComposition->project()->mapThemeCollection()->masterVisibleLayers();
  }
  else if ( !layers().isEmpty() )
  {
    renderLayers = layers();
  }
  else
  {
    renderLayers = mComposition->project()->mapThemeCollection()->masterVisibleLayers();
  }

  bool ok = false;
  QString ddLayers = mDataDefinedProperties.valueAsString( QgsComposerObject::MapLayers, *evalContext, QString(), &ok );
  if ( ok )
  {
    renderLayers.clear();

    QStringList layerNames = ddLayers.split( '|' );
    //need to convert layer names to layer ids
    Q_FOREACH ( const QString &name, layerNames )
    {
      QList< QgsMapLayer * > matchingLayers = mComposition->project()->mapLayersByName( name );
      Q_FOREACH ( QgsMapLayer *layer, matchingLayers )
      {
        renderLayers << layer;
      }
    }
  }

  //remove atlas coverage layer if required
  //TODO - move setting for hiding coverage layer to map item properties
  if ( mComposition->atlasMode() != QgsComposition::AtlasOff )
  {
    if ( mComposition->atlasComposition().hideCoverage() )
    {
      //hiding coverage layer
      int removeAt = renderLayers.indexOf( mComposition->atlasComposition().coverageLayer() );
      if ( removeAt != -1 )
      {
        renderLayers.removeAt( removeAt );
      }
    }
  }

  return renderLayers;
}

QMap<QString, QString> QgsComposerMap::layerStyleOverridesToRender( const QgsExpressionContext &context ) const
{
  if ( mFollowVisibilityPreset )
  {
    QString presetName = mFollowVisibilityPresetName;

    // data defined preset name?
    presetName = mDataDefinedProperties.valueAsString( QgsComposerObject::MapStylePreset, context, presetName );

    if ( mComposition->project()->mapThemeCollection()->hasMapTheme( presetName ) )
      return mComposition->project()->mapThemeCollection()->mapThemeStyleOverrides( presetName );
    else
      return QMap<QString, QString>();
  }
  else if ( mKeepLayerStyles )
  {
    return mLayerStyleOverrides;
  }
  else
  {
    return QMap<QString, QString>();
  }
}

double QgsComposerMap::scale() const
{
  QgsScaleCalculator calculator;
  calculator.setMapUnits( crs().mapUnits() );
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
  mLastRenderedImageOffsetX -= dx;
  mLastRenderedImageOffsetY -= dy;
  if ( !mDrawing )
  {
    transformShift( dx, dy );
    currentMapExtent()->setXMinimum( currentMapExtent()->xMinimum() + dx );
    currentMapExtent()->setXMaximum( currentMapExtent()->xMaximum() + dx );
    currentMapExtent()->setYMinimum( currentMapExtent()->yMinimum() + dy );
    currentMapExtent()->setYMaximum( currentMapExtent()->yMaximum() + dy );

    //in case data defined extents are set, these override the calculated values
    refreshMapExtents();

    invalidateCache();
    emit itemChanged();
    emit extentChanged();
  }
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
    //if map is atlas controlled and set to fixed scaling mode, then scale changes should be treated as permanent
    //and also apply to the map's original extent (see #9602)
    //we can't use the scaleRatio calculated earlier, as the scale can vary depending on extent for geographic coordinate systems
    QgsScaleCalculator calculator;
    calculator.setMapUnits( crs().mapUnits() );
    calculator.setDpi( 25.4 );  //QGraphicsView units are mm
    double scaleRatio = scale() / calculator.calculate( mExtent, rect().width() );
    mExtent.scale( scaleRatio );
  }

  //recalculate data defined scale and extents, since that may override zoom
  refreshMapExtents();

  invalidateCache();
  emit itemChanged();
  emit extentChanged();
}

void QgsComposerMap::setSceneRect( const QRectF &rectangle )
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
  updateBoundingRect();
  invalidateCache();
  emit itemChanged();
  emit extentChanged();
}

void QgsComposerMap::setNewExtent( const QgsRectangle &extent )
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
  QgsRectangle currentExtent = *currentMapExtent();
  //Make sure the width/height ratio is the same as the current composer map extent.
  //This is to keep the map item frame size fixed
  double currentWidthHeightRatio = 1.0;
  if ( !currentExtent.isNull() )
    currentWidthHeightRatio = currentExtent.width() / currentExtent.height();
  else
    currentWidthHeightRatio = rect().width() / rect().height();
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

  invalidateCache();
  emit itemChanged();
  emit extentChanged();
}

void QgsComposerMap::setNewAtlasFeatureExtent( const QgsRectangle &extent )
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
    else
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

  emit preparedForAtlas();
  invalidateCache();
  emit itemChanged();
  emit extentChanged();
}

QgsRectangle *QgsComposerMap::currentMapExtent()
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
    //otherwise return permanent user set extent
    return &mExtent;
  }
}

QgsCoordinateReferenceSystem QgsComposerMap::crs() const
{
  if ( mCrs.isValid() )
    return mCrs;
  else if ( mComposition && mComposition->project() )
    return mComposition->project()->crs();
  return QgsCoordinateReferenceSystem();
}

void QgsComposerMap::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  mCrs = crs;
}

const QgsRectangle *QgsComposerMap::currentMapExtent() const
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
    //otherwise return permanent user set extent
    return &mExtent;
  }
}

void QgsComposerMap::setNewScale( double scaleDenominator, bool forceUpdate )
{
  double currentScaleDenominator = scale();

  if ( qgsDoubleNear( scaleDenominator, currentScaleDenominator ) || qgsDoubleNear( scaleDenominator, 0.0 ) )
  {
    return;
  }

  double scaleRatio = scaleDenominator / currentScaleDenominator;
  currentMapExtent()->scale( scaleRatio );

  if ( mAtlasDriven && mAtlasScalingMode == Fixed && mComposition->atlasMode() != QgsComposition::AtlasOff )
  {
    //if map is atlas controlled and set to fixed scaling mode, then scale changes should be treated as permanent
    //and also apply to the map's original extent (see #9602)
    //we can't use the scaleRatio calculated earlier, as the scale can vary depending on extent for geographic coordinate systems
    QgsScaleCalculator calculator;
    calculator.setMapUnits( crs().mapUnits() );
    calculator.setDpi( 25.4 );  //QGraphicsView units are mm
    scaleRatio = scaleDenominator / calculator.calculate( mExtent, rect().width() );
    mExtent.scale( scaleRatio );
  }

  invalidateCache();
  if ( forceUpdate )
  {
    emit itemChanged();
  }
  emit extentChanged();
}

void QgsComposerMap::setOffset( double xOffset, double yOffset )
{
  mXOffset = xOffset;
  mYOffset = yOffset;
}

void QgsComposerMap::setMapRotation( double rotation )
{
  mMapRotation = rotation;
  mEvaluatedMapRotation = mMapRotation;
  invalidateCache();
  emit mapRotationChanged( rotation );
  emit itemChanged();
}

double QgsComposerMap::mapRotation( QgsComposerObject::PropertyValueType valueType ) const
{
  return valueType == QgsComposerObject::EvaluatedValue ? mEvaluatedMapRotation : mMapRotation;
}

void QgsComposerMap::refreshMapExtents( const QgsExpressionContext *context )
{
  QgsExpressionContext scopedContext = createExpressionContext();
  const QgsExpressionContext *evalContext = context ? context : &scopedContext;

  //data defined map extents set?
  QgsRectangle newExtent = *currentMapExtent();
  bool useDdXMin = false;
  bool useDdXMax = false;
  bool useDdYMin = false;
  bool useDdYMax = false;
  double minXD = 0;
  double minYD = 0;
  double maxXD = 0;
  double maxYD = 0;

  bool ok = false;
  minXD = mDataDefinedProperties.valueAsDouble( QgsComposerObject::MapXMin, *evalContext, 0.0, &ok );
  if ( ok )
  {
    useDdXMin = true;
    newExtent.setXMinimum( minXD );
  }
  minYD = mDataDefinedProperties.valueAsDouble( QgsComposerObject::MapYMin, *evalContext, 0.0, &ok );
  if ( ok )
  {
    useDdYMin = true;
    newExtent.setYMinimum( minYD );
  }
  maxXD = mDataDefinedProperties.valueAsDouble( QgsComposerObject::MapXMax, *evalContext, 0.0, &ok );
  if ( ok )
  {
    useDdXMax = true;
    newExtent.setXMaximum( maxXD );
  }
  maxYD = mDataDefinedProperties.valueAsDouble( QgsComposerObject::MapYMax, *evalContext, 0.0, &ok );
  if ( ok )
  {
    useDdYMax = true;
    newExtent.setYMaximum( maxYD );
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
  double scaleD = mDataDefinedProperties.valueAsDouble( QgsComposerObject::MapScale, *evalContext, 0.0, &ok );
  if ( ok )
  {
    setNewScale( scaleD, false );
    newExtent = *currentMapExtent();
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
  mapRotation = mDataDefinedProperties.valueAsDouble( QgsComposerObject::MapRotation, *evalContext, mapRotation );

  if ( !qgsDoubleNear( mEvaluatedMapRotation, mapRotation ) )
  {
    mEvaluatedMapRotation = mapRotation;
    emit mapRotationChanged( mapRotation );
  }
}

bool QgsComposerMap::containsWmsLayer() const
{
  Q_FOREACH ( QgsMapLayer *layer, layersToRender() )
  {
    if ( QgsRasterLayer *currentRasterLayer = qobject_cast<QgsRasterLayer *>( layer ) )
    {
      const QgsRasterDataProvider *rasterProvider = nullptr;
      if ( ( rasterProvider = currentRasterLayer->dataProvider() ) )
      {
        if ( rasterProvider->name() == QLatin1String( "wms" ) )
        {
          return true;
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


  QgsMapSettings ms;
  ms.setLayers( layersToRender() );
  return ( !QgsMapSettingsUtils::containsAdvancedEffects( ms ).isEmpty() );
}

void QgsComposerMap::connectUpdateSlot()
{
  //connect signal from layer registry to update in case of new or deleted layers
  QgsProject *project = mComposition->project();
  if ( project )
  {
    // handles updating the stored layer state BEFORE the layers are removed
    connect( project, static_cast < void ( QgsProject::* )( const QList<QgsMapLayer *>& layers ) > ( &QgsProject::layersWillBeRemoved ),
             this, &QgsComposerMap::layersAboutToBeRemoved );
    // redraws the map AFTER layers are removed
    connect( project->layerTreeRoot(), &QgsLayerTree::layerOrderChanged, this, [ = ]
    {
      if ( layers().isEmpty() )
      {
        //using project layers, and layer order has changed
        invalidateCache();
      }
    } );

    connect( project, &QgsProject::crsChanged, this, [ = ]
    {
      if ( !mCrs.isValid() )
      {
        //using project CRS, which just changed....
        invalidateCache();
      }
    } );

  }
  connect( mComposition, &QgsComposition::refreshItemsTriggered, this, &QgsComposerMap::invalidateCache );
}

bool QgsComposerMap::writeXml( QDomElement &elem, QDomDocument &doc ) const
{
  if ( elem.isNull() )
  {
    return false;
  }

  QDomElement composerMapElem = doc.createElement( QStringLiteral( "ComposerMap" ) );
  composerMapElem.setAttribute( QStringLiteral( "id" ), mId );

  if ( mKeepLayerSet )
  {
    composerMapElem.setAttribute( QStringLiteral( "keepLayerSet" ), QStringLiteral( "true" ) );
  }
  else
  {
    composerMapElem.setAttribute( QStringLiteral( "keepLayerSet" ), QStringLiteral( "false" ) );
  }

  if ( mDrawAnnotations )
  {
    composerMapElem.setAttribute( QStringLiteral( "drawCanvasItems" ), QStringLiteral( "true" ) );
  }
  else
  {
    composerMapElem.setAttribute( QStringLiteral( "drawCanvasItems" ), QStringLiteral( "false" ) );
  }

  //extent
  QDomElement extentElem = doc.createElement( QStringLiteral( "Extent" ) );
  extentElem.setAttribute( QStringLiteral( "xmin" ), qgsDoubleToString( mExtent.xMinimum() ) );
  extentElem.setAttribute( QStringLiteral( "xmax" ), qgsDoubleToString( mExtent.xMaximum() ) );
  extentElem.setAttribute( QStringLiteral( "ymin" ), qgsDoubleToString( mExtent.yMinimum() ) );
  extentElem.setAttribute( QStringLiteral( "ymax" ), qgsDoubleToString( mExtent.yMaximum() ) );
  composerMapElem.appendChild( extentElem );

  if ( mCrs.isValid() )
  {
    QDomElement crsElem = doc.createElement( QStringLiteral( "crs" ) );
    mCrs.writeXml( crsElem, doc );
    composerMapElem.appendChild( crsElem );
  }

  // follow map theme
  composerMapElem.setAttribute( QStringLiteral( "followPreset" ), mFollowVisibilityPreset ? "true" : "false" );
  composerMapElem.setAttribute( QStringLiteral( "followPresetName" ), mFollowVisibilityPresetName );

  //map rotation
  composerMapElem.setAttribute( QStringLiteral( "mapRotation" ), QString::number( mMapRotation ) );

  //layer set
  QDomElement layerSetElem = doc.createElement( QStringLiteral( "LayerSet" ) );
  Q_FOREACH ( const QgsMapLayerRef &layerRef, mLayers )
  {
    if ( !layerRef )
      continue;
    QDomElement layerElem = doc.createElement( QStringLiteral( "Layer" ) );
    QDomText layerIdText = doc.createTextNode( layerRef.layerId );
    layerElem.appendChild( layerIdText );

    layerElem.setAttribute( QStringLiteral( "name" ), layerRef.name );
    layerElem.setAttribute( QStringLiteral( "source" ), layerRef.source );
    layerElem.setAttribute( QStringLiteral( "provider" ), layerRef.provider );

    layerSetElem.appendChild( layerElem );
  }
  composerMapElem.appendChild( layerSetElem );

  // override styles
  if ( mKeepLayerStyles )
  {
    QDomElement stylesElem = doc.createElement( QStringLiteral( "LayerStyles" ) );
    QMap<QString, QString>::const_iterator styleIt = mLayerStyleOverrides.constBegin();
    for ( ; styleIt != mLayerStyleOverrides.constEnd(); ++styleIt )
    {
      QDomElement styleElem = doc.createElement( QStringLiteral( "LayerStyle" ) );

      QgsMapLayerRef ref( styleIt.key() );
      ref.resolve( mComposition->project() );

      styleElem.setAttribute( QStringLiteral( "layerid" ), ref.layerId );
      styleElem.setAttribute( QStringLiteral( "name" ), ref.name );
      styleElem.setAttribute( QStringLiteral( "source" ), ref.source );
      styleElem.setAttribute( QStringLiteral( "provider" ), ref.provider );

      QgsMapLayerStyle style( styleIt.value() );
      style.writeXml( styleElem );
      stylesElem.appendChild( styleElem );
    }
    composerMapElem.appendChild( stylesElem );
  }

  //write a dummy "Grid" element to prevent crashes on pre 2.5 versions (refs #10905)
  QDomElement gridElem = doc.createElement( QStringLiteral( "Grid" ) );
  composerMapElem.appendChild( gridElem );

  //grids
  mGridStack->writeXml( composerMapElem, doc );

  //overviews
  mOverviewStack->writeXml( composerMapElem, doc );

  //atlas
  QDomElement atlasElem = doc.createElement( QStringLiteral( "AtlasMap" ) );
  atlasElem.setAttribute( QStringLiteral( "atlasDriven" ), mAtlasDriven );
  atlasElem.setAttribute( QStringLiteral( "scalingMode" ), mAtlasScalingMode );
  atlasElem.setAttribute( QStringLiteral( "margin" ), qgsDoubleToString( mAtlasMargin ) );
  composerMapElem.appendChild( atlasElem );

  elem.appendChild( composerMapElem );
  return _writeXml( composerMapElem, doc );
}

bool QgsComposerMap::readXml( const QDomElement &itemElem, const QDomDocument &doc )
{
  if ( itemElem.isNull() )
  {
    return false;
  }

  setUpdatesEnabled( false );

  QString idRead = itemElem.attribute( QStringLiteral( "id" ), QStringLiteral( "not found" ) );
  if ( idRead != QLatin1String( "not found" ) )
  {
    mId = idRead.toInt();
    updateToolTip();
  }

  QgsReadWriteContext context;
  context.setPathResolver( mComposition->project()->pathResolver() );

  //extent
  QDomNodeList extentNodeList = itemElem.elementsByTagName( QStringLiteral( "Extent" ) );
  if ( !extentNodeList.isEmpty() )
  {
    QDomElement extentElem = extentNodeList.at( 0 ).toElement();
    double xmin, xmax, ymin, ymax;
    xmin = extentElem.attribute( QStringLiteral( "xmin" ) ).toDouble();
    xmax = extentElem.attribute( QStringLiteral( "xmax" ) ).toDouble();
    ymin = extentElem.attribute( QStringLiteral( "ymin" ) ).toDouble();
    ymax = extentElem.attribute( QStringLiteral( "ymax" ) ).toDouble();
    setNewExtent( QgsRectangle( xmin, ymin, xmax, ymax ) );
  }

  QDomNodeList crsNodeList = itemElem.elementsByTagName( QStringLiteral( "crs" ) );
  if ( !crsNodeList.isEmpty() )
  {
    QDomElement crsElem = crsNodeList.at( 0 ).toElement();
    mCrs.readXml( crsElem );
  }
  else
  {
    mCrs = QgsCoordinateReferenceSystem();
  }

  //map rotation
  if ( !qgsDoubleNear( itemElem.attribute( QStringLiteral( "mapRotation" ), QStringLiteral( "0" ) ).toDouble(), 0.0 ) )
  {
    mMapRotation = itemElem.attribute( QStringLiteral( "mapRotation" ), QStringLiteral( "0" ) ).toDouble();
  }

  // follow map theme
  mFollowVisibilityPreset = itemElem.attribute( QStringLiteral( "followPreset" ) ).compare( QLatin1String( "true" ) ) == 0;
  mFollowVisibilityPresetName = itemElem.attribute( QStringLiteral( "followPresetName" ) );

  //mKeepLayerSet flag
  QString keepLayerSetFlag = itemElem.attribute( QStringLiteral( "keepLayerSet" ) );
  if ( keepLayerSetFlag.compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0 )
  {
    mKeepLayerSet = true;
  }
  else
  {
    mKeepLayerSet = false;
  }

  QString drawCanvasItemsFlag = itemElem.attribute( QStringLiteral( "drawCanvasItems" ), QStringLiteral( "true" ) );
  if ( drawCanvasItemsFlag.compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0 )
  {
    mDrawAnnotations = true;
  }
  else
  {
    mDrawAnnotations = false;
  }

  mLayerStyleOverrides.clear();

  //mLayers
  mLayers.clear();
  QDomNodeList layerSetNodeList = itemElem.elementsByTagName( QStringLiteral( "LayerSet" ) );
  if ( !layerSetNodeList.isEmpty() )
  {
    QDomElement layerSetElem = layerSetNodeList.at( 0 ).toElement();
    QDomNodeList layerIdNodeList = layerSetElem.elementsByTagName( QStringLiteral( "Layer" ) );
    mLayers.reserve( layerIdNodeList.size() );
    for ( int i = 0; i < layerIdNodeList.size(); ++i )
    {
      QDomElement layerElem = layerIdNodeList.at( i ).toElement();
      QString layerId = layerElem.text();
      QString layerName = layerElem.attribute( QStringLiteral( "name" ) );
      QString layerSource = layerElem.attribute( QStringLiteral( "source" ) );
      QString layerProvider = layerElem.attribute( QStringLiteral( "provider" ) );

      QgsMapLayerRef ref( layerId, layerName, layerSource, layerProvider );
      ref.resolveWeakly( mComposition->project() );
      mLayers << ref;
    }
  }

  // override styles
  QDomNodeList layerStylesNodeList = itemElem.elementsByTagName( QStringLiteral( "LayerStyles" ) );
  mKeepLayerStyles = !layerStylesNodeList.isEmpty();
  if ( mKeepLayerStyles )
  {
    QDomElement layerStylesElem = layerStylesNodeList.at( 0 ).toElement();
    QDomNodeList layerStyleNodeList = layerStylesElem.elementsByTagName( QStringLiteral( "LayerStyle" ) );
    for ( int i = 0; i < layerStyleNodeList.size(); ++i )
    {
      const QDomElement &layerStyleElement = layerStyleNodeList.at( i ).toElement();
      QString layerId = layerStyleElement.attribute( QStringLiteral( "layerid" ) );
      QString layerName = layerStyleElement.attribute( QStringLiteral( "name" ) );
      QString layerSource = layerStyleElement.attribute( QStringLiteral( "source" ) );
      QString layerProvider = layerStyleElement.attribute( QStringLiteral( "provider" ) );
      QgsMapLayerRef ref( layerId, layerName, layerSource, layerProvider );
      ref.resolveWeakly( mComposition->project() );

      QgsMapLayerStyle style;
      style.readXml( layerStyleElement );
      mLayerStyleOverrides.insert( ref.layerId, style.xmlData() );
    }
  }

  mDrawing = false;
  mNumCachedLayers = 0;
  mCacheInvalidated = true;

  //overviews
  mOverviewStack->readXml( itemElem, doc );

  //grids
  mGridStack->readXml( itemElem, doc );

  //load grid / grid annotation in old xml format
  //only do this if the grid stack didn't load any grids, otherwise this will
  //be the dummy element created by QGIS >= 2.5 (refs #10905)
  QDomNodeList gridNodeList = itemElem.elementsByTagName( QStringLiteral( "Grid" ) );
  if ( mGridStack->size() == 0 && !gridNodeList.isEmpty() )
  {
    QDomElement gridElem = gridNodeList.at( 0 ).toElement();
    QgsComposerMapGrid *mapGrid = new QgsComposerMapGrid( tr( "Grid %1" ).arg( 1 ), this );
    mapGrid->setEnabled( gridElem.attribute( QStringLiteral( "show" ), QStringLiteral( "0" ) ) != QLatin1String( "0" ) );
    mapGrid->setStyle( QgsComposerMapGrid::GridStyle( gridElem.attribute( QStringLiteral( "gridStyle" ), QStringLiteral( "0" ) ).toInt() ) );
    mapGrid->setIntervalX( gridElem.attribute( QStringLiteral( "intervalX" ), QStringLiteral( "0" ) ).toDouble() );
    mapGrid->setIntervalY( gridElem.attribute( QStringLiteral( "intervalY" ), QStringLiteral( "0" ) ).toDouble() );
    mapGrid->setOffsetX( gridElem.attribute( QStringLiteral( "offsetX" ), QStringLiteral( "0" ) ).toDouble() );
    mapGrid->setOffsetY( gridElem.attribute( QStringLiteral( "offsetY" ), QStringLiteral( "0" ) ).toDouble() );
    mapGrid->setCrossLength( gridElem.attribute( QStringLiteral( "crossLength" ), QStringLiteral( "3" ) ).toDouble() );
    mapGrid->setFrameStyle( static_cast< QgsComposerMapGrid::FrameStyle >( gridElem.attribute( QStringLiteral( "gridFrameStyle" ), QStringLiteral( "0" ) ).toInt() ) );
    mapGrid->setFrameWidth( gridElem.attribute( QStringLiteral( "gridFrameWidth" ), QStringLiteral( "2.0" ) ).toDouble() );
    mapGrid->setFramePenSize( gridElem.attribute( QStringLiteral( "gridFramePenThickness" ), QStringLiteral( "0.5" ) ).toDouble() );
    mapGrid->setFramePenColor( QgsSymbolLayerUtils::decodeColor( gridElem.attribute( QStringLiteral( "framePenColor" ), QStringLiteral( "0,0,0" ) ) ) );
    mapGrid->setFrameFillColor1( QgsSymbolLayerUtils::decodeColor( gridElem.attribute( QStringLiteral( "frameFillColor1" ), QStringLiteral( "255,255,255,255" ) ) ) );
    mapGrid->setFrameFillColor2( QgsSymbolLayerUtils::decodeColor( gridElem.attribute( QStringLiteral( "frameFillColor2" ), QStringLiteral( "0,0,0,255" ) ) ) );
    mapGrid->setBlendMode( QgsPainting::getCompositionMode( static_cast< QgsPainting::BlendMode >( itemElem.attribute( QStringLiteral( "gridBlendMode" ), QStringLiteral( "0" ) ).toUInt() ) ) );
    QDomElement gridSymbolElem = gridElem.firstChildElement( QStringLiteral( "symbol" ) );
    QgsLineSymbol *lineSymbol = nullptr;
    if ( gridSymbolElem.isNull() )
    {
      //old project file, read penWidth /penColorRed, penColorGreen, penColorBlue
      lineSymbol = QgsLineSymbol::createSimple( QgsStringMap() );
      lineSymbol->setWidth( gridElem.attribute( QStringLiteral( "penWidth" ), QStringLiteral( "0" ) ).toDouble() );
      lineSymbol->setColor( QColor( gridElem.attribute( QStringLiteral( "penColorRed" ), QStringLiteral( "0" ) ).toInt(),
                                    gridElem.attribute( QStringLiteral( "penColorGreen" ), QStringLiteral( "0" ) ).toInt(),
                                    gridElem.attribute( QStringLiteral( "penColorBlue" ), QStringLiteral( "0" ) ).toInt() ) );
    }
    else
    {
      lineSymbol = QgsSymbolLayerUtils::loadSymbol<QgsLineSymbol>( gridSymbolElem, context );
    }
    mapGrid->setLineSymbol( lineSymbol );

    //annotation
    QDomNodeList annotationNodeList = gridElem.elementsByTagName( QStringLiteral( "Annotation" ) );
    if ( !annotationNodeList.isEmpty() )
    {
      QDomElement annotationElem = annotationNodeList.at( 0 ).toElement();
      mapGrid->setAnnotationEnabled( annotationElem.attribute( QStringLiteral( "show" ), QStringLiteral( "0" ) ) != QLatin1String( "0" ) );
      mapGrid->setAnnotationFormat( QgsComposerMapGrid::AnnotationFormat( annotationElem.attribute( QStringLiteral( "format" ), QStringLiteral( "0" ) ).toInt() ) );
      mapGrid->setAnnotationPosition( QgsComposerMapGrid::AnnotationPosition( annotationElem.attribute( QStringLiteral( "leftPosition" ), QStringLiteral( "0" ) ).toInt() ), QgsComposerMapGrid::Left );
      mapGrid->setAnnotationPosition( QgsComposerMapGrid::AnnotationPosition( annotationElem.attribute( QStringLiteral( "rightPosition" ), QStringLiteral( "0" ) ).toInt() ), QgsComposerMapGrid::Right );
      mapGrid->setAnnotationPosition( QgsComposerMapGrid::AnnotationPosition( annotationElem.attribute( QStringLiteral( "topPosition" ), QStringLiteral( "0" ) ).toInt() ), QgsComposerMapGrid::Top );
      mapGrid->setAnnotationPosition( QgsComposerMapGrid::AnnotationPosition( annotationElem.attribute( QStringLiteral( "bottomPosition" ), QStringLiteral( "0" ) ).toInt() ), QgsComposerMapGrid::Bottom );
      mapGrid->setAnnotationDirection( QgsComposerMapGrid::AnnotationDirection( annotationElem.attribute( QStringLiteral( "leftDirection" ), QStringLiteral( "0" ) ).toInt() ), QgsComposerMapGrid::Left );
      mapGrid->setAnnotationDirection( QgsComposerMapGrid::AnnotationDirection( annotationElem.attribute( QStringLiteral( "rightDirection" ), QStringLiteral( "0" ) ).toInt() ), QgsComposerMapGrid::Right );
      mapGrid->setAnnotationDirection( QgsComposerMapGrid::AnnotationDirection( annotationElem.attribute( QStringLiteral( "topDirection" ), QStringLiteral( "0" ) ).toInt() ), QgsComposerMapGrid::Top );
      mapGrid->setAnnotationDirection( QgsComposerMapGrid::AnnotationDirection( annotationElem.attribute( QStringLiteral( "bottomDirection" ), QStringLiteral( "0" ) ).toInt() ), QgsComposerMapGrid::Bottom );
      mapGrid->setAnnotationFrameDistance( annotationElem.attribute( QStringLiteral( "frameDistance" ), QStringLiteral( "0" ) ).toDouble() );
      QFont annotationFont;
      annotationFont.fromString( annotationElem.attribute( QStringLiteral( "font" ), QLatin1String( "" ) ) );
      mapGrid->setAnnotationFont( annotationFont );
      mapGrid->setAnnotationFontColor( QgsSymbolLayerUtils::decodeColor( itemElem.attribute( QStringLiteral( "fontColor" ), QStringLiteral( "0,0,0,255" ) ) ) );

      mapGrid->setAnnotationPrecision( annotationElem.attribute( QStringLiteral( "precision" ), QStringLiteral( "3" ) ).toInt() );
    }
    mGridStack->addGrid( mapGrid );
  }

  //load overview in old xml format
  QDomElement overviewFrameElem = itemElem.firstChildElement( QStringLiteral( "overviewFrame" ) );
  if ( !overviewFrameElem.isNull() )
  {
    QgsComposerMapOverview *mapOverview = new QgsComposerMapOverview( tr( "Overview %1" ).arg( mOverviewStack->size() + 1 ), this );

    mapOverview->setFrameMap( overviewFrameElem.attribute( QStringLiteral( "overviewFrameMap" ), QStringLiteral( "-1" ) ).toInt() );
    mapOverview->setBlendMode( QgsPainting::getCompositionMode( static_cast< QgsPainting::BlendMode >( overviewFrameElem.attribute( QStringLiteral( "overviewBlendMode" ), QStringLiteral( "0" ) ).toUInt() ) ) );
    mapOverview->setInverted( overviewFrameElem.attribute( QStringLiteral( "overviewInverted" ) ).compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0 );
    mapOverview->setCentered( overviewFrameElem.attribute( QStringLiteral( "overviewCentered" ) ).compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0 );

    QgsFillSymbol *fillSymbol = nullptr;
    QDomElement overviewFrameSymbolElem = overviewFrameElem.firstChildElement( QStringLiteral( "symbol" ) );
    if ( !overviewFrameSymbolElem.isNull() )
    {
      fillSymbol = QgsSymbolLayerUtils::loadSymbol<QgsFillSymbol>( overviewFrameSymbolElem, context );
      mapOverview->setFrameSymbol( fillSymbol );
    }
    mOverviewStack->addOverview( mapOverview );
  }

  //atlas
  QDomNodeList atlasNodeList = itemElem.elementsByTagName( QStringLiteral( "AtlasMap" ) );
  if ( !atlasNodeList.isEmpty() )
  {
    QDomElement atlasElem = atlasNodeList.at( 0 ).toElement();
    mAtlasDriven = ( atlasElem.attribute( QStringLiteral( "atlasDriven" ), QStringLiteral( "0" ) ) != QLatin1String( "0" ) );
    if ( atlasElem.hasAttribute( QStringLiteral( "fixedScale" ) ) ) // deprecated XML
    {
      mAtlasScalingMode = ( atlasElem.attribute( QStringLiteral( "fixedScale" ), QStringLiteral( "0" ) ) != QLatin1String( "0" ) ) ? Fixed : Auto;
    }
    else if ( atlasElem.hasAttribute( QStringLiteral( "scalingMode" ) ) )
    {
      mAtlasScalingMode = static_cast<AtlasScalingMode>( atlasElem.attribute( QStringLiteral( "scalingMode" ) ).toInt() );
    }
    mAtlasMargin = atlasElem.attribute( QStringLiteral( "margin" ), QStringLiteral( "0.1" ) ).toDouble();
  }

  //restore general composer item properties
  QDomNodeList composerItemList = itemElem.elementsByTagName( QStringLiteral( "ComposerItem" ) );
  if ( !composerItemList.isEmpty() )
  {
    QDomElement composerItemElem = composerItemList.at( 0 ).toElement();

    if ( !qgsDoubleNear( composerItemElem.attribute( QStringLiteral( "rotation" ), QStringLiteral( "0" ) ).toDouble(), 0.0 ) )
    {
      //in versions prior to 2.1 map rotation was stored in the rotation attribute
      mMapRotation = composerItemElem.attribute( QStringLiteral( "rotation" ), QStringLiteral( "0" ) ).toDouble();
    }

    _readXml( composerItemElem, doc );
  }

  updateBoundingRect();
  setUpdatesEnabled( true );

  emit itemChanged();
  return true;
}

QList<QgsMapLayer *> QgsComposerMap::layers() const
{
  return _qgis_listRefToRaw( mLayers );
}

void QgsComposerMap::setLayers( const QList<QgsMapLayer *> &layers )
{
  mLayers = _qgis_listRawToRef( layers );
}


void QgsComposerMap::setLayerStyleOverrides( const QMap<QString, QString> &overrides )
{
  if ( overrides == mLayerStyleOverrides )
    return;

  mLayerStyleOverrides = overrides;
  emit layerStyleOverridesChanged();  // associated legends may listen to this
}


void QgsComposerMap::storeCurrentLayerStyles()
{
  mLayerStyleOverrides.clear();
  Q_FOREACH ( const QgsMapLayerRef &layerRef, mLayers )
  {
    if ( QgsMapLayer *layer = layerRef.get() )
    {
      QgsMapLayerStyle style;
      style.readFromLayer( layer );
      mLayerStyleOverrides.insert( layer->id(), style.xmlData() );
    }
  }
}

void QgsComposerMap::layersAboutToBeRemoved( QList< QgsMapLayer * > layers )
{
  if ( !mLayers.isEmpty() || mLayerStyleOverrides.isEmpty() )
  {
    Q_FOREACH ( QgsMapLayer *layer, layers )
    {
      mLayerStyleOverrides.remove( layer->id() );
    }
    _qgis_removeLayers( mLayers, layers );
  }
}

QgsComposerMapGrid *QgsComposerMap::grid()
{
  if ( mGridStack->size() < 1 )
  {
    QgsComposerMapGrid *grid = new QgsComposerMapGrid( tr( "Grid %1" ).arg( 1 ), this );
    mGridStack->addGrid( grid );
  }
  return mGridStack->grid( 0 );
}

const QgsComposerMapGrid *QgsComposerMap::constFirstMapGrid() const
{
  return const_cast<QgsComposerMap *>( this )->grid();
}

QgsComposerMapOverview *QgsComposerMap::overview()
{
  if ( mOverviewStack->size() < 1 )
  {
    QgsComposerMapOverview *overview = new QgsComposerMapOverview( tr( "Overview %1" ).arg( 1 ), this );
    mOverviewStack->addOverview( overview );
  }
  return mOverviewStack->overview( 0 );
}

const QgsComposerMapOverview *QgsComposerMap::constFirstMapOverview() const
{
  return const_cast<QgsComposerMap *>( this )->overview();
}

QRectF QgsComposerMap::boundingRect() const
{
  return mCurrentRectangle;
}

void QgsComposerMap::updateBoundingRect()
{
  QRectF rectangle = rect();
  double frameExtension = mFrame ? pen().widthF() / 2.0 : 0.0;

  double topExtension = 0.0;
  double rightExtension = 0.0;
  double bottomExtension = 0.0;
  double leftExtension = 0.0;

  if ( mGridStack )
    mGridStack->calculateMaxGridExtension( topExtension, rightExtension, bottomExtension, leftExtension );

  topExtension = std::max( topExtension, frameExtension );
  rightExtension = std::max( rightExtension, frameExtension );
  bottomExtension = std::max( bottomExtension, frameExtension );
  leftExtension = std::max( leftExtension, frameExtension );

  rectangle.setLeft( rectangle.left() - leftExtension );
  rectangle.setRight( rectangle.right() + rightExtension );
  rectangle.setTop( rectangle.top() - topExtension );
  rectangle.setBottom( rectangle.bottom() + bottomExtension );
  if ( rectangle != mCurrentRectangle )
  {
    prepareGeometryChange();
    mCurrentRectangle = rectangle;
  }
}

void QgsComposerMap::setFrameStrokeWidth( const double strokeWidth )
{
  QgsComposerItem::setFrameStrokeWidth( strokeWidth );
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

void QgsComposerMap::mapPolygon( const QgsRectangle &extent, QPolygonF &poly ) const
{
  poly.clear();
  if ( qgsDoubleNear( mEvaluatedMapRotation, 0.0 ) )
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
  QgsPointXY rotationPoint( ( extent.xMaximum() + extent.xMinimum() ) / 2.0, ( extent.yMaximum() + extent.yMinimum() ) / 2.0 );
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

void QgsComposerMap::requestedExtent( QgsRectangle &extent ) const
{
  QgsRectangle newExtent = *currentMapExtent();
  if ( qgsDoubleNear( mEvaluatedMapRotation, 0.0 ) )
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

QgsExpressionContext QgsComposerMap::createExpressionContext() const
{
  QgsExpressionContext context = QgsComposerItem::createExpressionContext();

  //Can't utilize QgsExpressionContextUtils::mapSettingsScope as we don't always
  //have a QgsMapSettings object available when the context is required, so we manually
  //add the same variables here
  QgsExpressionContextScope *scope = new QgsExpressionContextScope( tr( "Map Settings" ) );

  //use QgsComposerItem's id, not map item's ID, since that is user-definable
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_id" ), QgsComposerItem::id(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_rotation" ), mMapRotation, true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_scale" ), scale(), true ) );

  QgsRectangle extent( *currentMapExtent() );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_extent" ), QVariant::fromValue( QgsGeometry::fromRect( extent ) ), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_extent_width" ), extent.width(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_extent_height" ), extent.height(), true ) );
  QgsGeometry centerPoint = QgsGeometry::fromPointXY( extent.center() );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_extent_center" ), QVariant::fromValue( centerPoint ), true ) );

  if ( mComposition )
  {
    QgsCoordinateReferenceSystem mapCrs = crs();
    scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_crs" ), mapCrs.authid(), true ) );
    scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_crs_definition" ), mapCrs.toProj4(), true ) );
    scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_units" ), QgsUnitTypes::toString( mapCrs.mapUnits() ), true ) );
  }

  context.appendScope( scope );

  return context;
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

void QgsComposerMap::refreshDataDefinedProperty( const QgsComposerObject::DataDefinedProperty property, const QgsExpressionContext *context )
{
  QgsExpressionContext scopedContext = createExpressionContext();
  const QgsExpressionContext *evalContext = context ? context : &scopedContext;

  //updates data defined properties and redraws item to match
  if ( property == QgsComposerObject::MapRotation || property == QgsComposerObject::MapScale ||
       property == QgsComposerObject::MapXMin || property == QgsComposerObject::MapYMin ||
       property == QgsComposerObject::MapXMax || property == QgsComposerObject::MapYMax ||
       property == QgsComposerObject::MapAtlasMargin ||
       property == QgsComposerObject::AllProperties )
  {
    QgsRectangle beforeExtent = *currentMapExtent();
    refreshMapExtents( evalContext );
    emit itemChanged();
    if ( *currentMapExtent() != beforeExtent )
    {
      emit extentChanged();
    }
  }

  //force redraw
  mCacheInvalidated = true;

  QgsComposerItem::refreshDataDefinedProperty( property, evalContext );
}

void QgsComposerMap::transformShift( double &xShift, double &yShift ) const
{
  double mmToMapUnits = 1.0 / mapUnitsToMM();
  double dxScaled = xShift * mmToMapUnits;
  double dyScaled = - yShift * mmToMapUnits;

  QgsComposerUtils::rotate( mEvaluatedMapRotation, dxScaled, dyScaled );

  xShift = dxScaled;
  yShift = dyScaled;
}

QPointF QgsComposerMap::mapToItemCoords( QPointF mapCoords ) const
{
  QPolygonF mapPoly = transformedMapPolygon();
  if ( mapPoly.empty() )
  {
    return QPointF( 0, 0 );
  }

  QgsRectangle tExtent = transformedExtent();
  QgsPointXY rotationPoint( ( tExtent.xMaximum() + tExtent.xMinimum() ) / 2.0, ( tExtent.yMaximum() + tExtent.yMinimum() ) / 2.0 );
  double dx = mapCoords.x() - rotationPoint.x();
  double dy = mapCoords.y() - rotationPoint.y();
  QgsComposerUtils::rotate( -mEvaluatedMapRotation, dx, dy );
  QgsPointXY backRotatedCoords( rotationPoint.x() + dx, rotationPoint.y() + dy );

  QgsRectangle unrotatedExtent = transformedExtent();
  double xItem = rect().width() * ( backRotatedCoords.x() - unrotatedExtent.xMinimum() ) / unrotatedExtent.width();
  double yItem = rect().height() * ( 1 - ( backRotatedCoords.y() - unrotatedExtent.yMinimum() ) / unrotatedExtent.height() );
  return QPointF( xItem, yItem );
}

void QgsComposerMap::drawAnnotations( QPainter *painter )
{
  if ( !mComposition || !mComposition->project() || !mDrawAnnotations )
  {
    return;
  }

  QList< QgsAnnotation * > annotations = mComposition->project()->annotationManager()->annotations();
  if ( annotations.isEmpty() )
    return;

  QgsRenderContext rc = QgsComposerUtils::createRenderContextForMap( this, painter );
  rc.setForceVectorOutput( true );
  rc.setExpressionContext( createExpressionContext() );
  QList< QgsMapLayer * > layers = layersToRender( &rc.expressionContext() );

  Q_FOREACH ( QgsAnnotation *annotation, annotations )
  {
    if ( !annotation || !annotation->isVisible() )
    {
      continue;
    }
    if ( annotation->mapLayer() && !layers.contains( annotation->mapLayer() ) )
      continue;

    drawAnnotation( annotation, rc );
  }
}

void QgsComposerMap::drawAnnotation( const QgsAnnotation *annotation, QgsRenderContext &context )
{
  if ( !annotation || !annotation->isVisible() || !context.painter() || !context.painter()->device() )
  {
    return;
  }

  context.painter()->save();
  context.painter()->setRenderHint( QPainter::Antialiasing, context.flags() & QgsRenderContext::Antialiasing );

  double itemX, itemY;
  if ( annotation->hasFixedMapPosition() )
  {
    QPointF mapPos = composerMapPosForItem( annotation );
    itemX = mapPos.x();
    itemY = mapPos.y();
  }
  else
  {
    itemX = annotation->relativePosition().x() * rect().width();
    itemY = annotation->relativePosition().y() * rect().height();
  }
  context.painter()->translate( itemX, itemY );

  //setup painter scaling to dots so that symbology is drawn to scale
  double dotsPerMM = context.painter()->device()->logicalDpiX() / 25.4;
  context.painter()->scale( 1 / dotsPerMM, 1 / dotsPerMM ); // scale painter from mm to dots

  annotation->render( context );
  context.painter()->restore();
}

QPointF QgsComposerMap::composerMapPosForItem( const QgsAnnotation *annotation ) const
{
  if ( !annotation )
    return QPointF( 0, 0 );

  double mapX = 0.0;
  double mapY = 0.0;

  mapX = annotation->mapPosition().x();
  mapY = annotation->mapPosition().y();
  QgsCoordinateReferenceSystem annotationCrs = annotation->mapPositionCrs();

  if ( annotationCrs != crs() )
  {
    //need to reproject
    QgsCoordinateTransform t( annotationCrs, crs() );
    double z = 0.0;
    t.transformInPlace( mapX, mapY, z );
  }

  return mapToItemCoords( QPointF( mapX, mapY ) );
}

void QgsComposerMap::assignFreeId()
{
  if ( !mComposition )
  {
    return;
  }

  const QgsComposerMap *existingMap = mComposition->getComposerMapById( mId );
  if ( !existingMap )
  {
    return; //keep mId as it is still available
  }

  int maxId = -1;
  QList<const QgsComposerMap *> mapList = mComposition->composerMapItems();
  QList<const QgsComposerMap *>::const_iterator mapIt = mapList.constBegin();
  for ( ; mapIt != mapList.constEnd(); ++mapIt )
  {
    if ( ( *mapIt )->id() > maxId )
    {
      maxId = ( *mapIt )->id();
    }
  }
  mId = maxId + 1;
  updateToolTip();
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

double QgsComposerMap::atlasMargin( const QgsComposerObject::PropertyValueType valueType )
{
  if ( valueType == QgsComposerObject::EvaluatedValue )
  {
    //evaluate data defined atlas margin

    //start with user specified margin
    double margin = mAtlasMargin;
    QgsExpressionContext context = createExpressionContext();

    bool ok = false;
    double ddMargin = mDataDefinedProperties.valueAsDouble( QgsComposerObject::MapAtlasMargin, context, 0.0, &ok );
    if ( ok )
    {
      //divide by 100 to convert to 0 -> 1.0 range
      margin = ddMargin / 100;
    }
    return margin;
  }
  else
  {
    return mAtlasMargin;
  }
}

