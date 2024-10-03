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
#include "qgswebview.h"
#include "qgisapp.h"
#include <QVBoxLayout>

//
// QgsDocumentationPanelWidget
//

QgsDocumentationPanelWidget::QgsDocumentationPanelWidget( QWidget *parent )
  : QgsDevToolWidget( parent )
{
  setupUi( this );


  connect( mCppHomeButton, &QToolButton::clicked, this, [this] {QgisApp::instance()->showApiDocumentation( QStringLiteral( "qgis" ), false, true );} );
  connect( mPythonHomeButton, &QToolButton::clicked, this, [this] {QgisApp::instance()->showApiDocumentation( QStringLiteral( "qgis" ), true, true );} );
  connect( mQtHomeButton, &QToolButton::clicked, this, [this] {QgisApp::instance()->showApiDocumentation( QStringLiteral( "qt" ), false, true );} );
}

void QgsDocumentationPanelWidget::showUrl( const QUrl &url )
{
  mWebView->setUrl( url );
}
