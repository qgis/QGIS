/***************************************************************************
    qgsmaptips.cpp  -  Query a layer and show a maptip on the canvas
    ---------------------
    begin                : October 2007
    copyright            : (C) 2007 by Gary Sherman
    email                : sherman @ mrcc dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
// QGIS includes
#include "qgsfeatureiterator.h"
#include "qgsmapcanvas.h"
#include "qgsmaptool.h"
#include "qgsvectorlayer.h"
#include "qgsrasterlayer.h"
#include "qgsexpression.h"
#include "qgslogger.h"
#include "qgssettings.h"
#include "qgswebview.h"
#include "qgswebframe.h"
#include "qgsapplication.h"
#include "qgsrenderer.h"
#include "qgsexpressioncontextutils.h"
#include "qgsmaplayertemporalproperties.h"
#include "qgsrendercontext.h"
#include "qgsmapcanvasutils.h"

// Qt includes
#include <QPoint>
#include <QToolTip>
#include <QSettings>
#include <QLabel>
#include <QDesktopServices>
#if WITH_QTWEBKIT
#include <QWebElement>
#endif
#include <QHBoxLayout>

#include "qgsmaptip.h"
#include "moc_qgsmaptip.cpp"


const QString QgsMapTip::sMapTipTemplate = "<html>\n"
                                           "  <head>\n"
                                           "    <style>\n"
                                           "    body {\n"
                                           "        margin: 0;\n"
                                           "        font: %1pt \"%2\";\n"
                                           "        color: %3;\n"
                                           "        width: %4px;\n"
                                           "    }\n"
                                           "    #QgsWebViewContainer {\n"
                                           "        background-color: %5;\n"
                                           "        border: 1px solid %6;\n"
                                           "        display: inline-block;\n"
                                           "        margin: 0\n"
                                           "    }\n"
                                           "    #QgsWebViewContainerInner {\n"
                                           "        margin: 5px\n"
                                           "    }\n"
                                           "    </style>\n"
                                           "  </head>\n"
                                           "  <body>\n"
                                           "    <div id='QgsWebViewContainer'>\n"
                                           "      <div id='QgsWebViewContainerInner'>\n"
                                           "      %7\n"
                                           "      </div>\n"
                                           "    </div>\n"
                                           "  </body>\n"
                                           "</html>\n";


QgsMapTip::QgsMapTip()
{
  // Init the visible flag
  mMapTipVisible = false;

  mDelayedClearTimer.setSingleShot( true );
  connect( &mDelayedClearTimer, &QTimer::timeout, this, [=]() { this->clear(); } );
}

void QgsMapTip::showMapTip( QgsMapLayer *pLayer, QgsPointXY &mapPosition, const QPoint &pixelPosition, QgsMapCanvas *pMapCanvas )
{
  // Do the search using the active layer and the preferred label field for the
  // layer. The label field must be defined in the layer configuration
  // file/database. The code required to do this is similar to identify, except
  // we only want the first qualifying feature and we will only display the
  // field defined as the label field in the layer configuration file/database

  // Do not render map tips if the layer is not visible
  if ( !pMapCanvas->layers( true ).contains( pLayer ) )
  {
    return;
  }

  // Do not render a new map tip when the mouse hovers an existing one
  if ( mWebView && mWebView->underMouse() )
  {
    return;
  }

  // Show the maptip on the canvas
  QString tipText, lastTipText, tipHtml;

  if ( !mWebView )
  {
    mWebView = new QgsWebView( pMapCanvas );
    // Make the webwiew transparent

    // Setting the background color to 'transparent' does not play nice
    // with webkit scrollbars, that are rendered as black rectangles (#54683)
    QColor transparentColor = mWebView->palette().color( QPalette::Window );
    transparentColor.setAlpha( 0 );
    mWebView->setStyleSheet( QString( "background:%1;" ).arg( transparentColor.name( QColor::HexArgb ) ) );


#if WITH_QTWEBKIT
    mWebView->page()->setLinkDelegationPolicy( QWebPage::DelegateAllLinks ); //Handle link clicks by yourself
    mWebView->setContextMenuPolicy( Qt::NoContextMenu );                     //No context menu is allowed if you don't need it
    connect( mWebView, &QWebView::linkClicked, this, &QgsMapTip::onLinkClicked );
    connect( mWebView, &QWebView::loadFinished, this, [=]( bool ) { resizeAndMoveToolTip(); } );
#endif

    mWebView->page()->settings()->setAttribute( QWebSettings::DeveloperExtrasEnabled, true );
    mWebView->page()->settings()->setAttribute( QWebSettings::JavascriptEnabled, true );
    mWebView->page()->settings()->setAttribute( QWebSettings::LocalStorageEnabled, true );

    // Disable scrollbars, avoid random resizing issues
    mWebView->page()->mainFrame()->setScrollBarPolicy( Qt::Horizontal, Qt::ScrollBarAlwaysOff );
    mWebView->page()->mainFrame()->setScrollBarPolicy( Qt::Vertical, Qt::ScrollBarAlwaysOff );
  }

  // Only supported layer types here:
  switch ( pLayer->type() )
  {
    case Qgis::LayerType::Vector:
      tipText = fetchFeature( pLayer, mapPosition, pMapCanvas );
      break;
    case Qgis::LayerType::Raster:
      tipText = fetchRaster( pLayer, mapPosition, pMapCanvas );
      break;
    default:
      break;
  }

  mMapTipVisible = !tipText.isEmpty();
  if ( !mMapTipVisible )
  {
    clear();
    return;
  }

  if ( tipText == lastTipText )
  {
    return;
  }

  // Compute offset from the cursor position
  int cursorOffset = 0;
  if ( QgsApplication::instance() )
  {
    // The following calculations are taken
    // from QgsApplication::getThemeCursor, and are used to calculate the correct cursor size
    // for both hi-dpi and non-hi-dpi screens.
    double scale = Qgis::UI_SCALE_FACTOR * QgsApplication::instance()->fontMetrics().height() / 32.0;
    cursorOffset = static_cast<int>( std::ceil( scale * 32 ) );
  }

  // Ensures the map tip is never larger than the available space
  const int MAX_WIDTH = std::max( pixelPosition.x(), pMapCanvas->width() - pixelPosition.x() ) - cursorOffset - 5;
  const int MAX_HEIGHT = std::max( pixelPosition.y(), pMapCanvas->height() - pixelPosition.y() ) - 5;

  mWebView->setMaximumSize( MAX_WIDTH, MAX_HEIGHT );

  tipHtml = QgsMapTip::htmlText( tipText, MAX_WIDTH );

  QgsDebugMsgLevel( tipHtml, 2 );

  mPosition = pixelPosition;
  mMapCanvas = pMapCanvas;
  mWebView->setHtml( tipHtml );
  lastTipText = tipText;

#if !WITH_QTWEBKIT
  resizeAndMoveToolTip();
#endif
}

void QgsMapTip::resizeAndMoveToolTip()
{
#if WITH_QTWEBKIT
  // Get the content size
  const QWebElement container = mWebView->page()->mainFrame()->findFirstElement(
    QStringLiteral( "#QgsWebViewContainer" )
  );
  const int width = container.geometry().width();
  const int height = container.geometry().height();
  mWebView->resize( width, height );
#else
  mWebView->adjustSize();
#endif

  int cursorOffset = 0;
  // attempt to shift the tip away from the cursor.
  if ( QgsApplication::instance() )
  {
    // The following calculations are taken
    // from QgsApplication::getThemeCursor, and are used to calculate the correct cursor size
    // for both hi-dpi and non-hi-dpi screens.
    double scale = Qgis::UI_SCALE_FACTOR * QgsApplication::instance()->fontMetrics().height() / 32.0;
    cursorOffset = static_cast<int>( std::ceil( scale * 32 ) );
  }

  if ( !mMapCanvas )
  {
    mWebView->move( mPosition );
    mWebView->show();
    return;
  }

  // Check if there is enough space to the right of the cursor
  int availableWidthRight = mMapCanvas->width() - mPosition.x() - cursorOffset;
  int availableWidthLeft = mPosition.x() - cursorOffset;
  int availableHeightBottom = mMapCanvas->height() - mPosition.y();
  int availableHeightTop = mPosition.y();
  int x, y;
  // If there is enough space on the right, or more space on the right than on the left, move the map tip to the right of the cursor
  if ( mWebView->width() < availableWidthRight || availableWidthRight > availableWidthLeft )
  {
    x = mPosition.x() + cursorOffset;
  }
  // Otherwise, move the map tip to the left of the cursor
  else
  {
    x = mPosition.x() - mWebView->width() - cursorOffset;
  }

  // If there is enough space on the bottom, or more space on the bottom than on the top, move the map tip to the bottom of the cursor
  if ( mWebView->height() < availableHeightBottom || availableHeightBottom > availableHeightTop )
  {
    y = mPosition.y();
  }
  // Otherwise, move the map tip to the top of the cursor
  else
  {
    y = mPosition.y() - mWebView->height();
  }
  mWebView->move( x, y );
  mWebView->show();
}

void QgsMapTip::clear( QgsMapCanvas *, int msDelay )
{
  if ( !mMapTipVisible )
  {
    return;
  }

  // Skip clearing the map tip if the user interacts with it or the timer still runs
  if ( mDelayedClearTimer.isActive() || mWebView->underMouse() )
  {
    return;
  }

  if ( msDelay > 0 )
  {
    mDelayedClearTimer.start( msDelay );
    return;
  }
  mWebView->setHtml( QString() );
  mWebView->hide();

  // Reset the visible flag
  mMapTipVisible = false;
}

QString QgsMapTip::fetchFeature( QgsMapLayer *layer, QgsPointXY &mapPosition, QgsMapCanvas *mapCanvas )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
  if ( !vlayer || !vlayer->isSpatial() || !vlayer->mapTipsEnabled() )
  {
    return QString();
  }

  if ( !layer->isInScaleRange( mapCanvas->mapSettings().scale() ) || ( mapCanvas->mapSettings().isTemporal() && layer->temporalProperties() && !layer->temporalProperties()->isVisibleInTemporalRange( mapCanvas->temporalRange() ) ) )
  {
    return QString();
  }

  const double searchRadius = QgsMapTool::searchRadiusMU( mapCanvas );

  QgsRectangle r;
  r.setXMinimum( mapPosition.x() - searchRadius );
  r.setYMinimum( mapPosition.y() - searchRadius );
  r.setXMaximum( mapPosition.x() + searchRadius );
  r.setYMaximum( mapPosition.y() + searchRadius );

  r = mapCanvas->mapSettings().mapToLayerCoordinates( layer, r );

  QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( vlayer ) );
  context.appendScope( QgsExpressionContextUtils::mapSettingsScope( mapCanvas->mapSettings() ) );
  context.appendScope( QgsExpressionContextUtils::mapLayerPositionScope( r.center() ) );

  const QString canvasFilter = QgsMapCanvasUtils::filterForLayer( mapCanvas, vlayer );
  if ( canvasFilter == QLatin1String( "FALSE" ) )
  {
    return QString();
  }

  const QString mapTip = vlayer->mapTipTemplate();
  QString tipString;
  QgsExpression exp( vlayer->displayExpression() );
  QgsFeature feature;

  QgsFeatureRequest request;
  request.setFilterRect( r );
  request.setFlags( Qgis::FeatureRequestFlag::ExactIntersect );
  if ( !canvasFilter.isEmpty() )
  {
    request.setFilterExpression( canvasFilter );
  }

  if ( mapTip.isEmpty() )
  {
    exp.prepare( &context );
    request.setSubsetOfAttributes( exp.referencedColumns(), vlayer->fields() );
  }

  QgsRenderContext renderCtx = QgsRenderContext::fromMapSettings( mapCanvas->mapSettings() );
  renderCtx.setExpressionContext( mapCanvas->createExpressionContext() );
  renderCtx.expressionContext() << QgsExpressionContextUtils::layerScope( vlayer );

  bool filter = false;
  std::unique_ptr<QgsFeatureRenderer> renderer;
  if ( vlayer->renderer() )
  {
    renderer.reset( vlayer->renderer()->clone() );
    renderer->startRender( renderCtx, vlayer->fields() );
    filter = renderer->capabilities() & QgsFeatureRenderer::Filter;

    const QString filterExpression = renderer->filter( vlayer->fields() );
    if ( !filterExpression.isEmpty() )
    {
      request.combineFilterExpression( filterExpression );
    }
  }
  request.setExpressionContext( renderCtx.expressionContext() );

  QgsFeatureIterator it = vlayer->getFeatures( request );
  QElapsedTimer timer;
  timer.start();
  while ( it.nextFeature( feature ) )
  {
    context.setFeature( feature );

    renderCtx.expressionContext().setFeature( feature );
    if ( filter && renderer && !renderer->willRenderFeature( feature, renderCtx ) )
    {
      continue;
    }

    if ( !mapTip.isEmpty() )
    {
      tipString = QgsExpression::replaceExpressionText( mapTip, &context );
    }
    else
    {
      tipString = exp.evaluate( &context ).toString();
    }

    if ( !tipString.isEmpty() || timer.elapsed() >= 1000 )
    {
      break;
    }
  }

  if ( renderer )
  {
    renderer->stopRender( renderCtx );
  }

  return tipString;
}

QString QgsMapTip::fetchRaster( QgsMapLayer *layer, QgsPointXY &mapPosition, QgsMapCanvas *mapCanvas )
{
  QgsRasterLayer *rlayer = qobject_cast<QgsRasterLayer *>( layer );
  if ( !rlayer || !rlayer->mapTipsEnabled() )
  {
    return QString();
  }

  if ( !layer->isInScaleRange( mapCanvas->mapSettings().scale() ) || ( mapCanvas->mapSettings().isTemporal() && !layer->temporalProperties()->isVisibleInTemporalRange( mapCanvas->temporalRange() ) ) )
  {
    return QString();
  }

  if ( rlayer->mapTipTemplate().isEmpty() )
  {
    return QString();
  }

  const QgsPointXY mappedPosition { mapCanvas->mapSettings().mapToLayerCoordinates( layer, mapPosition ) };

  if ( !layer->extent().contains( mappedPosition ) )
  {
    return QString();
  }

  QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) );
  context.appendScope( QgsExpressionContextUtils::mapSettingsScope( mapCanvas->mapSettings() ) );
  context.appendScope( QgsExpressionContextUtils::mapLayerPositionScope( mappedPosition ) );
  return QgsExpression::replaceExpressionText( rlayer->mapTipTemplate(), &context );
}

QString QgsMapTip::htmlText( const QString &text, int maxWidth )
{
  const QgsSettings settings;
  const QFont defaultFont = qApp->font();
  const int fontSize = defaultFont.pointSize();
  const QString fontFamily = defaultFont.family();
  const QString backgroundColor = QgsApplication::palette().base().color().name();
  const QString strokeColor = QgsApplication::palette().shadow().color().name();
  const QString textColor = QgsApplication::palette().toolTipText().color().name();
  return sMapTipTemplate.arg( fontSize ).arg( fontFamily ).arg( textColor ).arg( maxWidth == -1 ? "" : QString::number( maxWidth ) ).arg( backgroundColor ).arg( strokeColor ).arg( text );
}

// This slot handles all clicks
void QgsMapTip::onLinkClicked( const QUrl &url )
{
  QDesktopServices::openUrl( url );
}


QString QgsMapTip::vectorMapTipPreviewText( QgsMapLayer *layer, QgsMapCanvas *mapCanvas, const QString &mapTemplate, const QString &displayExpression )
{
  // Only spatial layers can have map tips
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
  if ( !mapCanvas || !vlayer || !vlayer->isSpatial() )
    return QString();

  // If no map tip template or display expression is set, return an empty string
  if ( mapTemplate.isEmpty() && displayExpression.isEmpty() )
    return QString();

  // Create an expression context
  QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( vlayer ) );
  context.appendScope( QgsExpressionContextUtils::mapSettingsScope( mapCanvas->mapSettings() ) );

  // Get the first feature if any, and add it to the expression context
  QgsFeature previewFeature;
  if ( vlayer->featureCount() > 0 )
  {
    QgsFeatureIterator it = vlayer->getFeatures( QgsFeatureRequest().setLimit( 1 ) );
    it.nextFeature( previewFeature );
  }
  else
  {
    previewFeature = QgsFeature( vlayer->fields() );
  }
  context.setFeature( previewFeature );

  // Generate the map tip text from the context and the mapTipTemplate/displayExpression
  QString tipText;
  if ( mapTemplate.isEmpty() )
  {
    QgsExpression exp( displayExpression );
    exp.prepare( &context );
    tipText = exp.evaluate( &context ).toString();
  }
  else
  {
    tipText = QgsExpression::replaceExpressionText( mapTemplate, &context );
  }

  // Insert the map tip text into the html template
  return QgsMapTip::htmlText( tipText, mapCanvas->width() / 2 );
}

QString QgsMapTip::rasterMapTipPreviewText( QgsMapLayer *layer, QgsMapCanvas *mapCanvas, const QString &mapTemplate )
{
  QgsRasterLayer *rlayer = qobject_cast<QgsRasterLayer *>( layer );
  if ( !mapCanvas || !rlayer || mapTemplate.isEmpty() )
  {
    return QString();
  }

  // Create an expression context
  QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) );
  context.appendScope( QgsExpressionContextUtils::mapSettingsScope( mapCanvas->mapSettings() ) );

  // Get the position of the center of the layer, and add it to the expression context
  const QgsPointXY mappedPosition { layer->extent().center() };
  context.appendScope( QgsExpressionContextUtils::mapLayerPositionScope( mappedPosition ) );

  // Generate the map tip text from the context and the mapTipTemplate
  const QString tipText = QgsExpression::replaceExpressionText( mapTemplate, &context );

  // Insert the map tip text into the html template
  return QgsMapTip::htmlText( tipText, mapCanvas->width() / 2 );
}
