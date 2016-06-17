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
#include "qgsmapcanvas.h"
#include "qgsmaptool.h"
#include "qgsvectorlayer.h"
#include "qgsexpression.h"
#include "qgslogger.h"
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
    : mWidget( nullptr ), mWebView( nullptr )
{
  // init the visible flag
  mMapTipVisible = false;
}

QgsMapTip::~QgsMapTip()
{

}

void QgsMapTip::showMapTip( QgsMapLayer *pLayer,
                            QgsPoint & mapPosition,
                            QPoint & thePixelPosition,
                            QgsMapCanvas *pMapCanvas )
{
  // Do the search using the active layer and the preferred label field for the
  // layer. The label field must be defined in the layer configuration
  // file/database. The code required to do this is similar to identify, except
  // we only want the first qualifying feature and we will only display the
  // field defined as the label field in the layer configuration file/database

  // Show the maptip on the canvas
  QString tipText, lastTipText, tipHtml, bodyStyle, containerStyle,
  backgroundColor, borderColor;

  delete mWidget;
  mWidget = new QWidget( pMapCanvas );
  mWebView = new QgsWebView( mWidget );


#if WITH_QTWEBKIT
  mWebView->page()->setLinkDelegationPolicy( QWebPage::DelegateAllLinks );//Handle link clicks by yourself
  mWebView->setContextMenuPolicy( Qt::NoContextMenu ); //No context menu is allowed if you don't need it
  connect( mWebView, SIGNAL( linkClicked( QUrl ) ), this, SLOT( onLinkClicked( QUrl ) ) );
#endif

  mWebView->page()->settings()->setAttribute(
    QWebSettings::DeveloperExtrasEnabled, true );
  mWebView->page()->settings()->setAttribute(
    QWebSettings::JavascriptEnabled, true );

  QHBoxLayout* layout = new QHBoxLayout;
  layout->addWidget( mWebView );

  mWidget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
  mWidget->setLayout( layout );

  //assure the map tip is never larger than half the map canvas
  const int MAX_WIDTH = pMapCanvas->geometry().width() / 2;
  const int MAX_HEIGHT = pMapCanvas->geometry().height() / 2;
  mWidget->setMaximumSize( MAX_WIDTH, MAX_HEIGHT );

  // start with 0 size,
  // the content will automatically make it grow up to MaximumSize
  mWidget->resize( 0, 0 );

  backgroundColor = mWidget->palette().base().color().name();
  borderColor = mWidget->palette().shadow().color().name();
  mWidget->setStyleSheet( QString(
                            ".QWidget{"
                            "border: 1px solid %1;"
                            "background-color: %2;}" ).arg(
                            borderColor, backgroundColor ) );

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
                "margin: 0;" ).arg( backgroundColor );

  containerStyle = QString(
                     "display: inline-block;"
                     "margin: 0px" );

  tipHtml = QString(
              "<html>"
              "<body style='%1'>"
              "<div id='QgsWebViewContainer' style='%2'>%3</div>"
              "</body>"
              "</html>" ).arg( bodyStyle, containerStyle, tipText );

  mWidget->move( thePixelPosition.x(),
                 thePixelPosition.y() );

  mWebView->setHtml( tipHtml );
  lastTipText = tipText;

  mWidget->show();

#if WITH_QTWEBKIT
  int scrollbarWidth = mWebView->page()->mainFrame()->scrollBarGeometry(
                         Qt::Vertical ).width();
  int scrollbarHeight = mWebView->page()->mainFrame()->scrollBarGeometry(
                          Qt::Horizontal ).height();

  if ( scrollbarWidth > 0 || scrollbarHeight > 0 )
  {
    // Get the content size
    QWebElement container = mWebView->page()->mainFrame()->findFirstElement(
                              "#QgsWebViewContainer" );
    int width = container.geometry().width() + 5 + scrollbarWidth;
    int height = container.geometry().height() + 5 + scrollbarHeight;

    mWidget->resize( width, height );
  }
#endif
}

void QgsMapTip::clear( QgsMapCanvas * )
{
  if ( !mMapTipVisible )
    return;

  mWebView->setHtml( QString() );
  mWidget->hide();

  // reset the visible flag
  mMapTipVisible = false;
}

QString QgsMapTip::fetchFeature( QgsMapLayer *layer, QgsPoint &mapPosition, QgsMapCanvas *mpMapCanvas )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
  if ( !vlayer )
    return QString();

  double searchRadius = QgsMapTool::searchRadiusMU( mpMapCanvas );

  QgsRectangle r;
  r.setXMinimum( mapPosition.x() - searchRadius );
  r.setYMinimum( mapPosition.y() - searchRadius );
  r.setXMaximum( mapPosition.x() + searchRadius );
  r.setYMaximum( mapPosition.y() + searchRadius );

  r = mpMapCanvas->mapSettings().mapToLayerCoordinates( layer, r );

  QgsFeature feature;

  if ( !vlayer->getFeatures( QgsFeatureRequest().setFilterRect( r ).setFlags( QgsFeatureRequest::ExactIntersect ) ).nextFeature( feature ) )
    return QString();

  int idx = vlayer->fieldNameIndex( vlayer->displayField() );
  if ( idx < 0 )
  {
    QgsExpressionContext context;
    context << QgsExpressionContextUtils::globalScope()
    << QgsExpressionContextUtils::projectScope()
    << QgsExpressionContextUtils::layerScope( vlayer );
    if ( mpMapCanvas )
      context.appendScope( QgsExpressionContextUtils::mapSettingsScope( mpMapCanvas->mapSettings() ) );

    context.setFeature( feature );
    return QgsExpression::replaceExpressionText( vlayer->displayField(), &context );
  }
  else
  {
    return feature.attribute( idx ).toString();
  }
}

//This slot handles all clicks
void QgsMapTip::onLinkClicked( const QUrl &url )
{
  QDesktopServices::openUrl( url );
}
