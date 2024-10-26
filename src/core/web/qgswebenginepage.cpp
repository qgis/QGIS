/***************************************************************************
                          qgswebenginepage.h
                             -------------------
    begin                : December 2023
    copyright            : (C) 2023 by Nyall Dawson
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

#include "qgswebenginepage.h"
#include "moc_qgswebenginepage.cpp"
#include "qgsconfig.h"
#include <QWebEnginePage>
#include <QEventLoop>
#include <QSizeF>

#ifdef HAVE_PDF4QT
#include "qgspdfrenderer.h"
#include <QTemporaryFile>
#else
#include "qgsexception.h"
#endif

QgsWebEnginePage::QgsWebEnginePage( QObject *parent )
  : QObject( parent )
  , mPage{ std::make_unique< QWebEnginePage >() }
{
  // proxy some signals from the page
  connect( mPage.get(), &QWebEnginePage::loadStarted, this, &QgsWebEnginePage::loadStarted );
  connect( mPage.get(), &QWebEnginePage::loadProgress, this, &QgsWebEnginePage::loadProgress );
  connect( mPage.get(), &QWebEnginePage::loadFinished, this, &QgsWebEnginePage::loadFinished );
}

QgsWebEnginePage::~QgsWebEnginePage() = default;

QWebEnginePage *QgsWebEnginePage::page()
{
  return mPage.get();
}

bool QgsWebEnginePage::setContent( const QByteArray &data, const QString &mimeType, const QUrl &baseUrl, bool blocking )
{
  mCachedSize = QSize();
  if ( blocking )
  {
    QEventLoop loop;
    bool finished = false;
    bool result = true;
    connect( mPage.get(), &QWebEnginePage::loadFinished, &loop, [&loop, &finished, &result]( bool ok )
    {
      finished = true;
      result = ok;
      loop.exit();
    } );
    mPage->setContent( data, mimeType, baseUrl );
    if ( !finished )
    {
      loop.exec( QEventLoop::ExcludeUserInputEvents );
    }
    if ( result )
      handlePostBlockingLoadOperations();
    return result;
  }
  else
  {
    mPage->setContent( data, mimeType, baseUrl );
    return true;
  }
}

bool QgsWebEnginePage::setHtml( const QString &html, const QUrl &baseUrl, bool blocking )
{
  mCachedSize = QSize();
  if ( blocking )
  {
    QEventLoop loop;
    bool finished = false;
    bool result = true;
    connect( mPage.get(), &QWebEnginePage::loadFinished, &loop, [&loop, &finished, &result]( bool ok )
    {
      finished = true;
      result = ok;
      loop.exit();
    } );
    mPage->setHtml( html, baseUrl );
    if ( !finished )
    {
      loop.exec( QEventLoop::ExcludeUserInputEvents );
    }
    if ( result )
      handlePostBlockingLoadOperations();

    return result;
  }
  else
  {
    mPage->setHtml( html, baseUrl );
    return true;
  }
}

bool QgsWebEnginePage::setUrl( const QUrl &url, bool blocking )
{
  mCachedSize = QSize();
  if ( blocking )
  {
    QEventLoop loop;
    bool finished = false;
    bool result = true;
    connect( mPage.get(), &QWebEnginePage::loadFinished, &loop, [&loop, &finished, &result]( bool ok )
    {
      finished = true;
      result = ok;
      loop.exit();
    } );
    mPage->setUrl( url );
    if ( !finished )
    {
      loop.exec( QEventLoop::ExcludeUserInputEvents );
    }
    if ( result )
      handlePostBlockingLoadOperations();

    return result;
  }
  else
  {
    mPage->setUrl( url );
    return true;
  }
}

QSize QgsWebEnginePage::documentSize() const
{
  if ( mCachedSize.isValid() )
    return mCachedSize;

  QEventLoop loop;
  int width = -1;
  int height = -1;
  bool finished = false;
  mPage->runJavaScript( "[document.documentElement.scrollWidth, document.documentElement.scrollHeight];", [&width, &height, &loop, &finished]( QVariant result )
  {
    width = result.toList().value( 0 ).toInt();
    height = result.toList().value( 1 ).toInt();
    finished = true;
    loop.exit();
  } );
  if ( !finished )
  {
    loop.exec( QEventLoop::ExcludeUserInputEvents );
  }


  mCachedSize = QSize( width, height );
  return mCachedSize;
}

void QgsWebEnginePage::handlePostBlockingLoadOperations()
{
  // Following a blocking content load, do some other quick calculations which involve local event loops.
  // This allows callers to avoid having to make another later call to a method which would other involve a local event loop.
  QEventLoop loop;
  int width = 0;
  int height = 0;
  bool finished = false;
  mPage->runJavaScript( "[document.documentElement.scrollWidth, document.documentElement.scrollHeight];", [&width, &height, &loop, &finished]( QVariant result )
  {
    width = result.toList().value( 0 ).toInt();
    height = result.toList().value( 1 ).toInt();
    finished = true;
    loop.exit();
  } );
  if ( !finished )
  {
    loop.exec( QEventLoop::ExcludeUserInputEvents );
  }

  mCachedSize = QSize( width, height );
}

#ifdef HAVE_PDF4QT
bool QgsWebEnginePage::render( QPainter *painter, const QRectF &painterRect )
{
  const QSize actualSize = documentSize();

  // TODO -- is this ALWAYS 96?
  static constexpr double dpi = 96.0;
  const QSizeF pageSize = QSizeF( actualSize.width() / dpi, actualSize.height() / dpi );

  QEventLoop loop;
  bool finished = false;
  bool printOk = false;
  QString renderedPdfPath;
  connect( mPage.get(), &QWebEnginePage::pdfPrintingFinished, &loop, [&loop, &finished, &printOk, &renderedPdfPath]( const QString & pdfPath, bool success )
  {
    finished = true;
    renderedPdfPath = pdfPath;
    printOk = success;
    loop.exit();
  } );

  // generate file name for temporary intermediate PDF file
  QTemporaryFile f;
  f.open();
  f.close();

  const QPageLayout layout = QPageLayout( QPageSize( pageSize, QPageSize::Inch ),
                                          QPageLayout::Portrait, QMarginsF( 0, 0, 0, 0 ),
                                          QPageLayout::Inch, QMarginsF( 0, 0, 0, 0 ) );
  mPage->printToPdf( f.fileName(), layout );

  if ( !finished )
  {
    loop.exec( QEventLoop::ExcludeUserInputEvents );
  }

  if ( printOk )
  {
    QgsPdfRenderer renderer( renderedPdfPath );
    renderer.render( painter, painterRect, 0 );
  }
  return printOk;
}
#else
bool QgsWebEnginePage::render( QPainter *, const QRectF & )
{
  throw QgsNotSupportedException( QObject::tr( "Rendering web pages requires a QGIS build with PDF4Qt library support" ) );
}
#endif
