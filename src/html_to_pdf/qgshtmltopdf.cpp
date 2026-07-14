/***************************************************************************
                          qgshtmltopdf.cpp
                          -------------------
    begin                : July 2026
    copyright            : (C) 2026 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgshtmltopdf.h"

#include <QString>

#include "moc_qgshtmltopdf.cpp"

using namespace Qt::StringLiterals;

#include <QObject>
#include <QLibrary>
#include <QWebEnginePage>
#include <QEventLoop>
#include <QCoreApplication>
#include <QUrl>
#include <QWebEngineCertificateError>
#include <QWebEngineNavigationRequest>
#include <QWebEngineLoadingInfo>

#include <iostream>

QgsHtmlToPdf::QgsHtmlToPdf()
{
  // avoid locks if page requests a prompt:
  mPage.runJavaScript( u"window.alert = function(){}; window.confirm = function(){return true;}; window.prompt = function(){return '';};"_s );

  connect( &mPage, &QWebEnginePage::loadFinished, this, &QgsHtmlToPdf::onLoadFinished );
  connect( &mPage, &QWebEnginePage::loadProgress, this, []( int progress ) { std::cout << progress << std::endl; } );
  connect( &mPage, &QWebEnginePage::loadingChanged, this, []( const QWebEngineLoadingInfo &info ) {
    if ( info.status() == QWebEngineLoadingInfo::LoadFailedStatus )
    {
      std::cerr
        << "[NETWORK ERROR] Failed to load URL: "
        << info.url().toString().toStdString()
        << "\n"
        << "  Domain Error Code: "
        << info.errorCode()
        << "\n"
        << "  Error String: "
        << info.errorString().toStdString()
        << "\n";
    }
    else if ( info.status() == QWebEngineLoadingInfo::LoadStartedStatus )
    {
      std::cerr << "[NETWORK] Loading initiated...\n";
    }
  } );
  connect( &mPage, &QWebEnginePage::permissionRequested, this, []( QWebEnginePermission permissionRequest ) { permissionRequest.deny(); } );

  connect( &mPage, &QWebEnginePage::pdfPrintingFinished, this, &QgsHtmlToPdf::onPdfFinished );
}

void QgsHtmlToPdf::run( const QString &url, const QString &pdfFileName )
{
  mPdfFileName = pdfFileName;

  mPage.load( QUrl( url ) );
}

void QgsHtmlToPdf::onLoadFinished( bool ok )
{
  if ( !ok )
  {
    emit finished( 1 );
    return;
  }

  mPage.runJavaScript( u"[document.documentElement.scrollWidth, document.documentElement.scrollHeight];"_s, [this]( QVariant res ) { onJsFinished( res ); } );
}

void QgsHtmlToPdf::onJsFinished( const QVariant &res )
{
  const QVariantList list = res.toList();
  if ( list.size() < 2 )
  {
    emit finished( 1 );
    return;
  }

  int width = list.value( 0 ).toInt();
  int height = list.value( 1 ).toInt();

  const QSizeF pageSize = QSizeF( width / RENDER_TO_PDF_DPI, height / RENDER_TO_PDF_DPI );
  const QPageLayout layout = QPageLayout( QPageSize( pageSize, QPageSize::Inch ), QPageLayout::Portrait, QMarginsF( 0, 0, 0, 0 ), QPageLayout::Inch, QMarginsF( 0, 0, 0, 0 ) );

  mPage.printToPdf( mPdfFileName, layout );
}

void QgsHtmlToPdf::onPdfFinished( const QString &, bool ok )
{
  emit finished( ok ? 0 : 1 );
}


//
// QgsLoggingWebEnginePage
//

void QgsLoggingWebEnginePage::javaScriptConsoleMessage( JavaScriptConsoleMessageLevel level, const QString &message, int lineNumber, const QString &sourceID )
{
  std::cerr << " [JS CONSOLE] " << ( level == ErrorMessageLevel ? "ERROR: " : "WARN/INFO: " ) << message.toStdString() << " (Line: " << lineNumber << ", Source: " << sourceID.toStdString() << ")\n";
}
