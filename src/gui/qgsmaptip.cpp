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
#include "qgsvectorlayertemporalproperties.h"
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

QgsMapTip::QgsMapTip()
{
  // Init the visible flag
  mMapTipVisible = false;

  // Init font-related values
  applyFontSettings();

  mDelayedClearTimer.setSingleShot( true );
  connect( &mDelayedClearTimer, &QTimer::timeout, this, [ = ]() {this->clear();} );
}

void QgsMapTip::showMapTip( QgsMapLayer *pLayer,
                            QgsPointXY &mapPosition,
                            QPoint &pixelPosition,
                            QgsMapCanvas *pMapCanvas )
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
  if ( mWidget && mWidget->underMouse() )
    return;

  // Show the maptip on the canvas
  QString tipText, lastTipText, tipHtml, bodyStyle, containerStyle,
          backgroundColor, strokeColor, textColor;

  if ( ! mWidget )
  {
    mWidget = new QWidget( pMapCanvas );
    mWidget->setContentsMargins( MARGIN_VALUE, MARGIN_VALUE, MARGIN_VALUE, MARGIN_VALUE );
    mWebView = new QgsWebView( mWidget );


#if WITH_QTWEBKIT
    mWebView->page()->setLinkDelegationPolicy( QWebPage::DelegateAllLinks );//Handle link clicks by yourself
    mWebView->setContextMenuPolicy( Qt::NoContextMenu ); //No context menu is allowed if you don't need it
    connect( mWebView, &QWebView::linkClicked, this, &QgsMapTip::onLinkClicked );
    connect( mWebView, &QWebView::loadFinished, this, [ = ]( bool ) { resizeContent(); } );
#endif

    mWebView->page()->settings()->setAttribute( QWebSettings::DeveloperExtrasEnabled, true );
    mWebView->page()->settings()->setAttribute( QWebSettings::JavascriptEnabled, true );
    mWebView->page()->settings()->setAttribute( QWebSettings::LocalStorageEnabled, true );

    // Disable scrollbars, avoid random resizing issues
    mWebView->page()->mainFrame()->setScrollBarPolicy( Qt::Horizontal, Qt::ScrollBarAlwaysOff );
    mWebView->page()->mainFrame()->setScrollBarPolicy( Qt::Vertical, Qt::ScrollBarAlwaysOff );

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins( 0, 0, 0, 0 );
    layout->addWidget( mWebView );

    mWidget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    mWidget->setLayout( layout );

    // Assure the map tip is never larger than half the map canvas
    const int MAX_WIDTH = pMapCanvas->geometry().width() / 2;
    const int MAX_HEIGHT = pMapCanvas->geometry().height() / 2;
    mWidget->setMaximumSize( MAX_WIDTH, MAX_HEIGHT );

    // Start with 0 size,
    // The content will automatically make it grow up to MaximumSize
    mWidget->resize( 0, 0 );

    backgroundColor = mWidget->palette().base().color().name();
    strokeColor = mWidget->palette().shadow().color().name();
    textColor = mWidget->palette().text().color().name();
    mWidget->setStyleSheet( QString(
                              ".QWidget{"
                              "border: 1px solid %1;"
                              "background-color: %2;}" ).arg(
                              strokeColor, backgroundColor ) );
  }

  // Only supported layer types here:
  switch ( pLayer->type() )
  {
    case QgsMapLayerType::VectorLayer:
      tipText = fetchFeature( pLayer, mapPosition, pMapCanvas );
      break;
    case QgsMapLayerType::RasterLayer:
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

  bodyStyle = QString(
                "background-color: %1;"
                "margin: 0;"
                "font: %2pt \"%3\";"
                "color: %4;" ).arg( backgroundColor ).arg( mFontSize ).arg( mFontFamily, textColor );

  containerStyle = QString(
                     "display: inline-block;"
                     "margin: 0px" );

  tipHtml = QString(
              "<html>"
              "<body style='%1'>"
              "<div id='QgsWebViewContainer' style='%2'>%3</div>"
              "</body>"
              "</html>" ).arg( bodyStyle, containerStyle, tipText );

  QgsDebugMsg( tipHtml );

  int cursorOffset = 0;
  // attempt to shift the tip away from the cursor.
  if ( QgsApplication::instance() )
  {
    // The following calculations are taken
    // from QgsApplication::getThemeCursor, and are used to calculate the correct cursor size
    // for both hi-dpi and non-hi-dpi screens.
    double scale = Qgis::UI_SCALE_FACTOR * QgsApplication::instance()->fontMetrics().height() / 32.0;
    cursorOffset = static_cast< int >( std::ceil( scale * 32 ) );
  }

  mWidget->move( pixelPosition.x() + cursorOffset, pixelPosition.y() );
  mWebView->setHtml( tipHtml );
  lastTipText = tipText;

  mWidget->show();

}

void QgsMapTip::resizeContent()
{
#if WITH_QTWEBKIT
  // Get the content size
  const QWebElement container = mWebView->page()->mainFrame()->findFirstElement(
                                  QStringLiteral( "#QgsWebViewContainer" ) );
  const int width = container.geometry().width() + MARGIN_VALUE * 2;
  const int height = container.geometry().height() + MARGIN_VALUE * 2;
  mWidget->resize( width, height );
#else
  mWebView->adjustSize();
#endif
}

void QgsMapTip::clear( QgsMapCanvas *, int msDelay )
{
  if ( !mMapTipVisible )
    return;

  // Skip clearing the map tip if the user interacts with it or the timer still runs
  if ( mDelayedClearTimer.isActive() || mWidget->underMouse() )
    return;

  if ( msDelay > 0 )
  {
    mDelayedClearTimer.start( msDelay );
    return;
  }
  mWebView->setHtml( QString() );
  mWidget->hide();

  // Reset the visible flag
  mMapTipVisible = false;
}

QString QgsMapTip::fetchFeature( QgsMapLayer *layer, QgsPointXY &mapPosition, QgsMapCanvas *mapCanvas )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
  if ( !vlayer || !vlayer->isSpatial() )
    return QString();

  if ( !layer->isInScaleRange( mapCanvas->mapSettings().scale() ) ||
       ( mapCanvas->mapSettings().isTemporal() && !layer->temporalProperties()->isVisibleInTemporalRange( mapCanvas->temporalRange() ) ) )
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
  if ( canvasFilter ==  QLatin1String( "FALSE" ) )
    return QString();

  const QString mapTip = vlayer->mapTipTemplate();
  QString tipString;
  QgsExpression exp( vlayer->displayExpression() );
  QgsFeature feature;

  QgsFeatureRequest request;
  request.setFilterRect( r );
  request.setFlags( QgsFeatureRequest::ExactIntersect );
  if ( !canvasFilter.isEmpty() )
    request.setFilterExpression( canvasFilter );

  if ( mapTip.isEmpty() )
  {
    exp.prepare( &context );
    request.setSubsetOfAttributes( exp.referencedColumns(), vlayer->fields() );
  }

  QgsRenderContext renderCtx = QgsRenderContext::fromMapSettings( mapCanvas->mapSettings() );
  renderCtx.setExpressionContext( mapCanvas->createExpressionContext() );
  renderCtx.expressionContext() << QgsExpressionContextUtils::layerScope( vlayer );

  bool filter = false;
  std::unique_ptr< QgsFeatureRenderer > renderer;
  if ( vlayer->renderer() )
  {
    renderer.reset( vlayer->renderer()->clone() );
    renderer->startRender( renderCtx, vlayer->fields() );
    filter = renderer->capabilities() & QgsFeatureRenderer::Filter;

    const QString filterExpression = renderer->filter( vlayer->fields() );
    if ( ! filterExpression.isEmpty() )
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
    renderer->stopRender( renderCtx );

  return tipString;
}

QString QgsMapTip::fetchRaster( QgsMapLayer *layer, QgsPointXY &mapPosition, QgsMapCanvas *mapCanvas )
{
  QgsRasterLayer *rlayer = qobject_cast<QgsRasterLayer *>( layer );
  if ( !rlayer )
    return QString();

  if ( !layer->isInScaleRange( mapCanvas->mapSettings().scale() ) ||
       ( mapCanvas->mapSettings().isTemporal() && !layer->temporalProperties()->isVisibleInTemporalRange( mapCanvas->temporalRange() ) ) )
  {
    return QString();
  }

  const QgsPointXY mappedPosition { mapCanvas->mapSettings().mapToLayerCoordinates( layer, mapPosition ) };

  if ( ! layer->extent().contains( mappedPosition ) )
  {
    return QString( );
  }

  QString tipText { rlayer->mapTipTemplate() };

  if ( ! tipText.isEmpty() )
  {
    QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) );
    context.appendScope( QgsExpressionContextUtils::mapSettingsScope( mapCanvas->mapSettings() ) );
    context.appendScope( QgsExpressionContextUtils::mapLayerPositionScope( mappedPosition ) );
    tipText = QgsExpression::replaceExpressionText( tipText, &context );
  }

  return tipText;
}

void QgsMapTip::applyFontSettings()
{
  const QgsSettings settings;
  const QFont defaultFont = qApp->font();
  mFontSize = settings.value( QStringLiteral( "/qgis/stylesheet/fontPointSize" ), defaultFont.pointSize() ).toInt();
  mFontFamily = settings.value( QStringLiteral( "/qgis/stylesheet/fontFamily" ), defaultFont.family() ).toString();
}

// This slot handles all clicks
void QgsMapTip::onLinkClicked( const QUrl &url )
{
  QDesktopServices::openUrl( url );
}
