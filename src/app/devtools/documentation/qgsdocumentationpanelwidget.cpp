/***************************************************************************
    qgsdocumentationpanelwidget.cpp
    -------------------------
    begin                : October 2024
    copyright            : (C) 2024 by Yoann Quenach de Quivillic
    email                : yoann dot quenach at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdocumentationpanelwidget.h"
#include "moc_qgsdocumentationpanelwidget.cpp"
#include "qgisapp.h"
#include <QVBoxLayout>

#ifdef HAVE_WEBENGINE
#include <QtWebEngineWidgets/QWebEngineView>
#else
#include "qgswebview.h"
#endif


//
// QgsDocumentationPanelWidget
//

QgsDocumentationPanelWidget::QgsDocumentationPanelWidget( QWidget *parent )
  : QgsDevToolWidget( parent )
{
  setupUi( this );
#ifdef HAVE_WEBENGINE
  mWebView = new QWebEngineView( this );
#else
  mWebView = new QgsWebView( this );
#endif

  mWebViewContainer->layout()->addWidget( mWebView );

  connect( mPyQgisHomeButton, &QToolButton::clicked, this, [] {QgisApp::instance()->showApiDocumentation( Qgis::DocumentationApi::PyQgis, Qgis::DocumentationBrowser::DeveloperToolsPanel );} );
  connect( mQtHomeButton, &QToolButton::clicked, this, [] {QgisApp::instance()->showApiDocumentation( Qgis::DocumentationApi::Qt, Qgis::DocumentationBrowser::DeveloperToolsPanel );} );
  connect( mOpenUrlButton, &QToolButton::clicked, this, [this] {QgisApp::instance()->openURL( mWebView->url().toString(), false );} );

}

void QgsDocumentationPanelWidget::showUrl( const QUrl &url )
{
  if ( mWebView->url() != url )
  {
    mWebView->load( url );
  }
}
