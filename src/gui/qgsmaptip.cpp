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
#include "qgsexpression.h"
#include "qgslogger.h"
#include "qgssettings.h"
#include "qgswebview.h"
#include "qgswebframe.h"

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
  if ( !pMapCanvas->layers().contains( pLayer ) )
  {
    return;
  }

  // Show the maptip on the canvas
  QString tipText, lastTipText, tipHtml, bodyStyle, containerStyle,
          backgroundColor, strokeColor;

  delete mWidget;
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
  mWidget->setStyleSheet( QString(
                            ".QWidget{"
                            "border: 1px solid %1;"
                            "background-color: %2;}" ).arg(
                            strokeColor, backgroundColor ) );

  tipText = fetchFeature( pLayer, mapPosition, pMapCanvas );

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
                "white-space: nowrap;"
                "font: %2pt \"%3\";" ).arg( backgroundColor ).arg( mFontSize ).arg( mFontFamily );

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

  mWidget->move( pixelPosition.x(),
                 pixelPosition.y() );

  mWebView->setHtml( tipHtml );
  lastTipText = tipText;

  mWidget->show();
}

void QgsMapTip::resizeContent()
{
#if WITH_QTWEBKIT
  // Get the content size
  QWebElement container = mWebView->page()->mainFrame()->findFirstElement(
                            QStringLiteral( "#QgsWebViewContainer" ) );
  int width = container.geometry().width() + MARGIN_VALUE * 2;
  int height = container.geometry().height() + MARGIN_VALUE * 2;
  mWidget->resize( width, height );
#else
  mWebView->adjustSize();
#endif
}

void QgsMapTip::clear( QgsMapCanvas * )
{
  if ( !mMapTipVisible )
    return;

  mWebView->setHtml( QString() );
  mWidget->hide();

  // Reset the visible flag
  mMapTipVisible = false;
}

QString QgsMapTip::fetchFeature( QgsMapLayer *layer, QgsPointXY &mapPosition, QgsMapCanvas *mapCanvas )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
  if ( !vlayer )
    return QString();

  double searchRadius = QgsMapTool::searchRadiusMU( mapCanvas );

  QgsRectangle r;
  r.setXMinimum( mapPosition.x() - searchRadius );
  r.setYMinimum( mapPosition.y() - searchRadius );
  r.setXMaximum( mapPosition.x() + searchRadius );
  r.setYMaximum( mapPosition.y() + searchRadius );

  r = mapCanvas->mapSettings().mapToLayerCoordinates( layer, r );

  QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( vlayer ) );
  if ( mapCanvas )
    context.appendScope( QgsExpressionContextUtils::mapSettingsScope( mapCanvas->mapSettings() ) );

  QString mapTip = vlayer->mapTipTemplate();
  QString tipString;
  QgsExpression exp( vlayer->displayExpression() );
  QgsFeature feature;
  QgsFeatureRequest request = QgsFeatureRequest().setFilterRect( r ).setFlags( QgsFeatureRequest::ExactIntersect );
  if ( mapTip.isEmpty() )
  {
    exp.prepare( &context );
    request.setSubsetOfAttributes( exp.referencedColumns(), vlayer->fields() );
  }
  QgsFeatureIterator it = vlayer->getFeatures( request );
  QTime timer;
  timer.start();
  while ( it.nextFeature( feature ) )
  {
    context.setFeature( feature );
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

  return tipString;
}

void QgsMapTip::applyFontSettings()
{
  QgsSettings settings;
  QFont defaultFont = qApp->font();
  mFontSize = settings.value( QStringLiteral( "/qgis/stylesheet/fontPointSize" ), defaultFont.pointSize() ).toInt();
  mFontFamily = settings.value( QStringLiteral( "/qgis/stylesheet/fontFamily" ), defaultFont.family() ).toString();
}

// This slot handles all clicks
void QgsMapTip::onLinkClicked( const QUrl &url )
{
  QDesktopServices::openUrl( url );
}
