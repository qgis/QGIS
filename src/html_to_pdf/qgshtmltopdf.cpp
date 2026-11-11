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
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QFileInfo>
#include <QDir>

#include <iostream>

QgsHtmlToPdf::QgsHtmlToPdf()
{
  mPage.settings()->setAttribute( QWebEngineSettings::TouchEventsApiEnabled, false );

  connect( &mPage, &QWebEnginePage::loadFinished, this, &QgsHtmlToPdf::onLoadFinished );
  connect( &mPage, &QWebEnginePage::loadProgress, this, []( int progress ) { std::cout << progress << std::endl; } );
  connect( &mPage, &QWebEnginePage::loadingChanged, this, [this]( const QWebEngineLoadingInfo &info ) {
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
      mReturnCode = ReturnCode::LoadFailed;
    }
    else if ( info.status() == QWebEngineLoadingInfo::LoadStartedStatus )
    {
      std::cout << "[NETWORK] Loading initiated...\n";
    }
  } );
  connect( &mPage, &QWebEnginePage::permissionRequested, this, []( QWebEnginePermission permissionRequest ) { permissionRequest.deny(); } );

  connect( &mPage, &QWebEnginePage::pdfPrintingFinished, this, &QgsHtmlToPdf::onPdfFinished );
}

void QgsHtmlToPdf::run( const QString &url, const QString &pdfFileName, int width, const QString &userAgent )
{
  mPdfFileName = pdfFileName;
  mWidth = width;
  mUserAgent = userAgent;

  if ( !mUserAgent.isEmpty() )
  {
    mPage.profile()->setHttpUserAgent( mUserAgent );
  }

  const QFileInfo fi( url );
  const QUrl resolvedUrl = fi.exists() ? QUrl::fromLocalFile( fi.absoluteFilePath() ) : QUrl::fromUserInput( url, QDir::currentPath(), QUrl::AssumeLocalFile );

  mPage.load( resolvedUrl );
}

void QgsHtmlToPdf::onLoadFinished( bool ok )
{
  if ( !ok )
  {
    mReturnCode = LoadFailed;
    emit finished( mReturnCode );
    return;
  }

  // a whole lot of unreliable approaches to try to constrain the page to a desired width
  // and then get the actual CONTENT height. The current iteration works ok for basic HTML... but
  // not so well if there's fancy javascript involved which changes the page height. Feel free to
  // revise as better approaches are available...!
  const QString jsTemplate = QString( uR"js(
(() => {
      // try to clear page margins
      const style = document.createElement('style');
      style.textContent = `
        @page { margin: 0 !important; size: auto; }
        html, body {
          margin: 0 !important;
          padding: 0 !important;
        }
        *, *::before, *::after {
          break-inside: auto !important;
          page-break-inside: auto !important;
          break-before: auto !important;
          page-break-before: auto !important;
          break-after: auto !important;
          page-break-after: auto !important;
        }
      `;
      (document.head || document.documentElement).appendChild(style);

      const targetWidth = %1;

      if (targetWidth > 0 && document.body) {
        document.documentElement.style.width = targetWidth + 'px';
        document.body.style.width = targetWidth + 'px';
        document.documentElement.style.boxSizing = 'border-box';
        document.body.style.boxSizing = 'border-box';
        void document.body.offsetHeight;
      }

      // try to work out actual document height at this width... not very reliable ;)
      let maxBottom = 0;
      let maxRight = 0;
      if (document.body) {
        const elements = document.body.querySelectorAll('*');
        for (let i = 0; i < elements.length; i++) {
          const rect = elements[i].getBoundingClientRect();
          if (rect.width > 0 && rect.height > 0) {
            const bottom = rect.bottom + window.scrollY;
            const right = rect.right + window.scrollX;
            if (bottom > maxBottom) maxBottom = bottom;
            if (right > maxRight) maxRight = right;
          }
        }
      }

      const bodyRect = document.body ? document.body.getBoundingClientRect() : { width: 0, height: 0 };
      const contentHeight = Math.ceil(Math.max(
        maxBottom,
        bodyRect.height,
        document.body ? document.body.scrollHeight : 0,
        document.documentElement.scrollHeight,
        document.documentElement.offsetHeight
      ));

      const contentWidth = targetWidth > 0 ? targetWidth : Math.ceil(Math.max(
        maxRight,
        bodyRect.width,
        document.documentElement.scrollWidth,
        document.documentElement.offsetWidth
      ));

      return [contentWidth, contentHeight];
    })()
  )js"_s )
                               .arg( mWidth );

  mPage.runJavaScript( jsTemplate, [this]( QVariant res ) { onLoadJsFinished( res ); } );
}

void QgsHtmlToPdf::onLoadJsFinished( const QVariant &res )
{
  const QVariantList list = res.toList();
  if ( list.size() < 2 )
  {
    mReturnCode = ReturnCode::LoadJsFailed;
    emit finished( mReturnCode );
    return;
  }

  const int cssWidth = std::max( 1, list.value( 0 ).toInt() );
  constexpr int PIXEL_BUFFER = 2;
  const int cssHeight = std::max( 1, list.value( 1 ).toInt() ) + PIXEL_BUFFER;

  constexpr double CSS_PX_TO_PT = 72.0 / 96.0;

  const double wPoints = cssWidth * CSS_PX_TO_PT;
  const double hPoints = cssHeight * CSS_PX_TO_PT;

  const bool isLandscape = wPoints > hPoints;

  const QSizeF pageSizePoints = isLandscape ? QSizeF( hPoints, wPoints ) : QSizeF( wPoints, hPoints );

  const QPageLayout
    layout( QPageSize( pageSizePoints, QPageSize::Point ), isLandscape ? QPageLayout::Landscape : QPageLayout::Portrait, QMarginsF( 0, 0, 0, 0 ), QPageLayout::Point, QMarginsF( 0, 0, 0, 0 ) );

  mPage.printToPdf( mPdfFileName, layout );
}

void QgsHtmlToPdf::onPdfFinished( const QString &, bool ok )
{
  if ( !ok )
  {
    mReturnCode = ReturnCode::PrintFailed;
  }

  emit finished( mReturnCode );
}


//
// QgsLoggingWebEnginePage
//

void QgsLoggingWebEnginePage::javaScriptConsoleMessage( JavaScriptConsoleMessageLevel level, const QString &message, int lineNumber, const QString &sourceID )
{
  switch ( level )
  {
    case QWebEnginePage::InfoMessageLevel:
    case QWebEnginePage::WarningMessageLevel:
      std::cout << " [JS CONSOLE] " << message.toStdString() << " (Line: " << lineNumber << ", Source: " << sourceID.toStdString() << ")\n";
      break;
    case QWebEnginePage::ErrorMessageLevel:
      std::cerr << " [JS CONSOLE ERROR] " << message.toStdString() << " (Line: " << lineNumber << ", Source: " << sourceID.toStdString() << ")\n";
      break;
  }
}
