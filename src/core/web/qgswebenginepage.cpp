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

#include "qgsconfig.h"
#include "qgswebenginepage.h"

#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsrunprocess.h"

#include <QCoreApplication>
#include <QDir>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainter>
#include <QSizeF>
#include <QString>
#include <QTemporaryDir>
#include <QWebEnginePage>

#include "moc_qgswebenginepage.cpp"

using namespace Qt::StringLiterals;

#ifdef HAVE_PDF4QT
#include "qgspdfrenderer.h"
#include <QTemporaryFile>
#else
#include "qgsexception.h"
#endif


//
// QgsHtmlRenderSettings
//

bool QgsHtmlRenderSettings::isEmpty() const
{
  return mBaseUrl.isEmpty() && mUserStylesheet.isEmpty() && mFeatureJson.isEmpty();
}

QVariantMap QgsHtmlRenderSettings::asMap() const
{
  QVariantMap res;
  if ( !baseUrl().isEmpty() )
  {
    res.insert( u"baseUrl"_s, QJsonValue::fromVariant( baseUrl() ) );
  }
  if ( !userStylesheet().isEmpty() )
  {
    res.insert( u"userStylesheet"_s, QJsonValue::fromVariant( userStylesheet() ) );
  }
  if ( !featureJson().isEmpty() )
  {
    res.insert( u"feature"_s, QJsonValue::fromVariant( featureJson() ) );
  }
  return res;
}


//
// QgsWebEnginePage
//

QgsWebEnginePage::QgsWebEnginePage( QObject *parent )
  : QObject( parent )
  , mPage { std::make_unique< QWebEnginePage >() }
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
    connect( mPage.get(), &QWebEnginePage::loadFinished, &loop, [&loop, &finished, &result]( bool ok ) {
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
    connect( mPage.get(), &QWebEnginePage::loadFinished, &loop, [&loop, &finished, &result]( bool ok ) {
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
    connect( mPage.get(), &QWebEnginePage::loadFinished, &loop, [&loop, &finished, &result]( bool ok ) {
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
  mPage->runJavaScript( "[document.documentElement.scrollWidth, document.documentElement.scrollHeight];", [&width, &height, &loop, &finished]( QVariant result ) {
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
  mPage->runJavaScript( "[document.documentElement.scrollWidth, document.documentElement.scrollHeight];", [&width, &height, &loop, &finished]( QVariant result ) {
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
  connect( mPage.get(), &QWebEnginePage::pdfPrintingFinished, &loop, [&loop, &finished, &printOk, &renderedPdfPath]( const QString &pdfPath, bool success ) {
    finished = true;
    renderedPdfPath = pdfPath;
    printOk = success;
    loop.exit();
  } );

  // generate file name for temporary intermediate PDF file
  QTemporaryFile f;
  if ( !f.open() )
    return false;

  f.close();

  const QPageLayout layout = QPageLayout( QPageSize( pageSize, QPageSize::Inch ), QPageLayout::Portrait, QMarginsF( 0, 0, 0, 0 ), QPageLayout::Inch, QMarginsF( 0, 0, 0, 0 ) );
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

QString QgsWebEnginePage::htmlToPdfExecutable()
{
  QString executable = QProcessEnvironment::systemEnvironment().value( u"QGIS_HTML_TO_PDF_EXECUTABLE"_s );
  if ( executable.isEmpty() )
  {
#if defined( Q_OS_WIN )
    executable = QgsApplication::libexecPath() + "qgis_html_to_pdf.exe";
#else
    executable = QgsApplication::libexecPath() + "qgis_html_to_pdf";
#endif
  }
  return executable;
}

QString QgsWebEnginePage::htmlToPdfReturnCodeToString( int code )
{
  // matches values in QgsHtmlToPdf::ResultCode
  switch ( code )
  {
    case 0:
      return QString();

    case 1:
    {
      return tr( "Loading HTML failed" );
    }

    case 2:
    {
      return tr( "Executing Javascript initialization code failed" );
    }

    case 3:
    {
      return tr( "Printing HTML to PDF failed" );
    }

    case 4:
    {
      return tr( "Loading settings JSON failed" );
    }

    default:
    {
      return tr( "Unknown error" );
    }
  }
  BUILTIN_UNREACHABLE
}

bool QgsWebEnginePage::doPrintToPdf( const QStringList &args, const QgsHtmlRenderSettings &settings, QString &errorMessage, QgsFeedback *feedback )
{
  const QString executable = htmlToPdfExecutable();
  if ( executable.isEmpty() || !QFile::exists( executable ) )
  {
    errorMessage = tr( "qgis_html_to_pdf utility not found" );
    return false;
  }

  const QTemporaryDir tempDir;

  QStringList allArgs = args;
  if ( !settings.isEmpty() )
  {
    const QString settingsFilename = tempDir.filePath( u"settings.json"_s );
    allArgs.append( u"--settings=%1"_s.arg( settingsFilename ) );

    QJsonObject settingsObj = QJsonObject::fromVariantMap( settings.asMap() );
    QJsonDocument jsonDocument( settingsObj );
    QFile settingsFile( settingsFilename );
    if ( !settingsFile.open( QFile::WriteOnly | QFile::Truncate ) )
    {
      errorMessage = tr( "writing settings to temporary file failed" );
      return false;
    }

    settingsFile.write( jsonDocument.toJson() );
    settingsFile.close();
  }

  QgsBlockingProcess process( executable, allArgs );
  process.setStdErrHandler( []( const QByteArray &ba ) { QgsDebugError( ba ); } );
  process.setStdOutHandler( []( const QByteArray &ba ) { QgsDebugMsgLevel( ba, 2 ); } );
  const int returnCode = process.run( feedback );
  errorMessage = htmlToPdfReturnCodeToString( returnCode );
  return returnCode == 0;
}

bool QgsWebEnginePage::printToPdfBlocking( const QString &html, const QString &pdfFileName, QString &errorMessage, int width, const QgsHtmlRenderSettings &settings, QgsFeedback *feedback )
{
  const QString executable = htmlToPdfExecutable();
  if ( executable.isEmpty() || !QFile::exists( executable ) )
  {
    errorMessage = tr( "qgis_html_to_pdf utility not found" );
    return false;
  }

  const QTemporaryDir tempDir;
  const QString outputFilename = tempDir.filePath( u"content.html"_s );
  QFile outputFile( outputFilename );
  if ( !outputFile.open( QFile::WriteOnly | QFile::Truncate ) )
  {
    errorMessage = tr( "writing HTML to temporary file failed" );
    return false;
  }

  outputFile.write( html.toUtf8() );
  outputFile.close();

  const QStringList args { outputFilename, pdfFileName, u"--width=%1"_s.arg( width ) };
  return doPrintToPdf( args, settings, errorMessage, feedback );
}

bool QgsWebEnginePage::printToPdfBlocking( const QUrl &url, const QString &pdfFileName, QString &errorMessage, int width, const QgsHtmlRenderSettings &settings, QgsFeedback *feedback )
{
  const QString executable = htmlToPdfExecutable();
  if ( executable.isEmpty() || !QFile::exists( executable ) )
  {
    errorMessage = tr( "qgis_html_to_pdf utility not found" );
    return false;
  }

  const QStringList args { url.toString(), pdfFileName, u"--width=%1"_s.arg( width ) };
  return doPrintToPdf( args, settings, errorMessage, feedback );
}

bool QgsWebEnginePage::renderBlocking(
  const QString &html, QPainter *painter, const QRectF &painterRect, QString &errorMessage, QSizeF &renderedSize, const QgsHtmlRenderSettings &settings, QgsFeedback *feedback
)
{
#ifdef HAVE_PDF4QT
  const QTemporaryDir tempDir;
  const QString renderedPdfPath = tempDir.filePath( u"content.pdf"_s );

  const double dpi = ( painter && painter->device() ) ? painter->device()->logicalDpiX() : 96.0;
  const int cssWidth = std::max( 1, static_cast< int >( std::round( painterRect.width() * 96.0 / dpi ) ) );

  if ( !printToPdfBlocking( html, renderedPdfPath, errorMessage, cssWidth, settings, feedback ) )
  {
    return false;
  }

  QgsPdfRenderer renderer( renderedPdfPath );
  return renderBlockingInternal( painter, renderer, errorMessage, painterRect, renderedSize );
#else
  ( void ) html;
  ( void ) painter;
  ( void ) painterRect;
  ( void ) errorMessage;
  ( void ) settings;
  ( void ) feedback;
  ( void ) renderedSize;
  throw QgsNotSupportedException( QObject::tr( "Rendering HTML requires a QGIS build with PDF4Qt library support" ) );
#endif
}

bool QgsWebEnginePage::renderBlocking( const QUrl &url, QPainter *painter, const QRectF &painterRect, QString &errorMessage, QSizeF &renderedSize, const QgsHtmlRenderSettings &settings, QgsFeedback *feedback )
{
#ifdef HAVE_PDF4QT
  const QTemporaryDir tempDir;
  const QString renderedPdfPath = tempDir.filePath( u"content.pdf"_s );

  const double dpi = ( painter && painter->device() ) ? painter->device()->logicalDpiX() : 96.0;
  const int cssWidth = std::max( 1, static_cast< int >( std::round( painterRect.width() * 96.0 / dpi ) ) );

  if ( !printToPdfBlocking( url, renderedPdfPath, errorMessage, cssWidth, settings, feedback ) )
  {
    return false;
  }

  QgsPdfRenderer renderer( renderedPdfPath );
  return renderBlockingInternal( painter, renderer, errorMessage, painterRect, renderedSize );
#else
  ( void ) url;
  ( void ) painter;
  ( void ) painterRect;
  ( void ) errorMessage;
  ( void ) feedback;
  ( void ) renderedSize;
  ( void ) settings;
  throw QgsNotSupportedException( QObject::tr( "Rendering HTML requires a QGIS build with PDF4Qt library support" ) );
#endif
}

#ifdef HAVE_PDF4QT
bool QgsWebEnginePage::renderBlockingInternal( QPainter *painter, QgsPdfRenderer &renderer, QString &errorMessage, const QRectF &painterRect, QSizeF &renderedSize )
{
  if ( renderer.pageCount() < 1 )
  {
    errorMessage = tr( "Generated PDF contains no pages" );
    return false;
  }

  const QRectF mediaBox = renderer.pageMediaBox( 0 );
  if ( mediaBox.isEmpty() || mediaBox.width() <= 0 )
    return false;

  // Calculate target height preserving the content's natural aspect ratio
  const double aspectRatio = mediaBox.height() / mediaBox.width();
  const double targetHeight = painterRect.width() * aspectRatio;

  const QRectF targetRect( painterRect.left(), painterRect.top(), painterRect.width(), targetHeight );
  renderedSize = targetRect.size();
  return renderer.render( painter, targetRect, 0 );
}
#endif
