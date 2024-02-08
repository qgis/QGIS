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
#include "qgsconfig.h"
#include <QWebEnginePage>

#ifdef HAVE_PDF4QT
#include "qgspdfrenderer.h"
#include <QEventLoop>
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

void QgsWebEnginePage::setContent( const QByteArray &data, const QString &mimeType, const QUrl &baseUrl )
{
  mPage->setContent( data, mimeType, baseUrl );
}

void QgsWebEnginePage::setHtml( const QString &html, const QUrl &baseUrl )
{
  mPage->setHtml( html, baseUrl );
}

void QgsWebEnginePage::setUrl( const QUrl &url )
{
  mPage->setUrl( url );
}

#ifdef HAVE_PDF4QT
bool QgsWebEnginePage::render( QPainter *painter, const QRectF &painterRect )
{
  // TODO -- page size

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
  mPage->printToPdf( f.fileName() );

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
