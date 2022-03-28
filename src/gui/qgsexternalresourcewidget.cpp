/***************************************************************************
  qgsexternalresourcewidget.cpp

 ---------------------
 begin                : 16.12.2015
 copyright            : (C) 2015 by Denis Rouzaud
 email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsexternalresourcewidget.h"
#include "qgspixmaplabel.h"
#include "qgsproject.h"
#include "qgsapplication.h"
#include "qgsnetworkaccessmanager.h"
#include "qgstaskmanager.h"
#include "qgsexternalstorage.h"
#include "qgsmessagebar.h"
#include "qgsexternalstoragefilewidget.h"

#include <QDir>
#include <QGridLayout>
#include <QVariant>
#include <QSettings>
#include <QImageReader>
#include <QToolButton>
#include <QMimeType>
#include <QMimeDatabase>
#include <QMovie>
#ifdef WITH_QTWEBKIT
#include <QWebView>
#endif

QgsExternalResourceWidget::QgsExternalResourceWidget( QWidget *parent )
  : QWidget( parent )
{
  setBackgroundRole( QPalette::Window );
  setAutoFillBackground( true );

  QGridLayout *layout = new QGridLayout();
  layout->setContentsMargins( 0, 0, 0, 0 );

  mFileWidget = new QgsExternalStorageFileWidget( this );
  layout->addWidget( mFileWidget, 0, 0 );
  mFileWidget->setVisible( mFileWidgetVisible );

  mPixmapLabel = new QgsPixmapLabel( this );
  layout->addWidget( mPixmapLabel, 1, 0 );

#ifdef WITH_QTWEBKIT
  mWebView = new QWebView( this );
  mWebView->setAcceptDrops( false );
  layout->addWidget( mWebView, 2, 0 );
#endif

  mLoadingLabel = new QLabel( this );
  layout->addWidget( mLoadingLabel, 3, 0 );
  mLoadingMovie = new QMovie( QgsApplication::iconPath( QStringLiteral( "/mIconLoading.gif" ) ), QByteArray(), this );
  mLoadingMovie->setScaledSize( QSize( 32, 32 ) );
  mLoadingLabel->setMovie( mLoadingMovie );

  mErrorLabel = new QLabel( this );
  layout->addWidget( mErrorLabel, 4, 0 );
  mErrorLabel->setPixmap( QPixmap( QgsApplication::iconPath( QStringLiteral( "/mIconWarning.svg" ) ) ) );

  updateDocumentViewer();

  setLayout( layout );

  connect( mFileWidget, &QgsFileWidget::fileChanged, this, &QgsExternalResourceWidget::loadDocument );
  connect( mFileWidget, &QgsFileWidget::fileChanged, this, &QgsExternalResourceWidget::valueChanged );
}

QVariant QgsExternalResourceWidget::documentPath( QVariant::Type type ) const
{
  const QString path = mFileWidget->filePath();
  if ( path.isEmpty() || path == QgsApplication::nullRepresentation() )
  {
    return QVariant( type );
  }
  else
  {
    return path;
  }
}

void QgsExternalResourceWidget::setDocumentPath( const QVariant &path )
{
  mFileWidget->setFilePath( path.toString() );
}

QgsExternalStorageFileWidget *QgsExternalResourceWidget::fileWidget()
{
  return mFileWidget;
}

bool QgsExternalResourceWidget::fileWidgetVisible() const
{
  return mFileWidgetVisible;
}

void QgsExternalResourceWidget::setFileWidgetVisible( bool visible )
{
  mFileWidgetVisible = visible;
  mFileWidget->setVisible( visible );
}

QgsExternalResourceWidget::DocumentViewerContent QgsExternalResourceWidget::documentViewerContent() const
{
  return mDocumentViewerContent;
}

void QgsExternalResourceWidget::setDocumentViewerContent( QgsExternalResourceWidget::DocumentViewerContent content )
{
  mDocumentViewerContent = content;
  if ( mDocumentViewerContent != Image )
    updateDocumentViewer();
  loadDocument( mFileWidget->filePath() );
}

int QgsExternalResourceWidget::documentViewerHeight() const
{
  return mDocumentViewerHeight;
}

void QgsExternalResourceWidget::setDocumentViewerHeight( int height )
{
  mDocumentViewerHeight = height;
  updateDocumentViewer();
}

int QgsExternalResourceWidget::documentViewerWidth() const
{
  return mDocumentViewerWidth;
}

void QgsExternalResourceWidget::setDocumentViewerWidth( int width )
{
  mDocumentViewerWidth = width;
  updateDocumentViewer();
}

void QgsExternalResourceWidget::setReadOnly( bool readOnly )
{
  mFileWidget->setReadOnly( readOnly );
}

void QgsExternalResourceWidget::updateDocumentViewer()
{
  mErrorLabel->setVisible( false );
  mLoadingLabel->setVisible( false );
  mLoadingMovie->stop();

#ifdef WITH_QTWEBKIT
  mWebView->setVisible( mDocumentViewerContent == Web );
#endif

  mPixmapLabel->setVisible( mDocumentViewerContent == Image );

  if ( mDocumentViewerContent == Image )
  {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    const QPixmap pm = mPixmapLabel->pixmap() ? *mPixmapLabel->pixmap() : QPixmap();
#else
    const QPixmap pm = mPixmapLabel->pixmap();
#endif

    if ( !pm || pm.isNull() )
    {
      mPixmapLabel->setMinimumSize( QSize( 0, 0 ) );
    }
    else
    {
      QSize size( mDocumentViewerWidth, mDocumentViewerHeight );
      if ( size.width() == 0 && size.height() > 0 )
      {
        size.setWidth( size.height() * pm.size().width() / pm.size().height() );
      }
      else if ( size.width() > 0 && size.height() == 0 )
      {
        size.setHeight( size.width() * pm.size().height() / pm.size().width() );
      }

      if ( size.width() != 0 || size.height() != 0 )
      {
        mPixmapLabel->setMinimumSize( size );
        mPixmapLabel->setMaximumSize( size );
      }
    }
  }
}

QString QgsExternalResourceWidget::resolvePath( const QString &path )
{
  switch ( mRelativeStorage )
  {
    case QgsFileWidget::Absolute:
      return path;
      break;
    case QgsFileWidget::RelativeProject:
      return QFileInfo( QgsProject::instance()->absoluteFilePath() ).dir().filePath( path );
      break;
    case QgsFileWidget::RelativeDefaultPath:
      return QDir( mDefaultRoot ).filePath( path );
      break;
  }
  return QString(); // avoid warnings
}

QString QgsExternalResourceWidget::defaultRoot() const
{
  return mDefaultRoot;
}

void QgsExternalResourceWidget::setDefaultRoot( const QString &defaultRoot )
{
  mFileWidget->setDefaultRoot( defaultRoot );
  mDefaultRoot = defaultRoot;
}

QgsFileWidget::RelativeStorage QgsExternalResourceWidget::relativeStorage() const
{
  return mRelativeStorage;
}

void QgsExternalResourceWidget::setRelativeStorage( QgsFileWidget::RelativeStorage relativeStorage )
{
  mFileWidget->setRelativeStorage( relativeStorage );
  mRelativeStorage = relativeStorage;
}

void QgsExternalResourceWidget::setStorageType( const QString &storageType )
{
  mFileWidget->setStorageType( storageType );
}

QString QgsExternalResourceWidget::storageType() const
{
  return mFileWidget->storageType();
}

void QgsExternalResourceWidget::setStorageAuthConfigId( const QString &authCfg )
{
  mFileWidget->setStorageAuthConfigId( authCfg );
}

QString QgsExternalResourceWidget::storageAuthConfigId() const
{
  return mFileWidget->storageAuthConfigId();
}

void QgsExternalResourceWidget::setMessageBar( QgsMessageBar *messageBar )
{
  mFileWidget->setMessageBar( messageBar );
}

QgsMessageBar *QgsExternalResourceWidget::messageBar() const
{
  return mFileWidget->messageBar();
}

void QgsExternalResourceWidget::updateDocumentContent( const QString &filePath )
{
#ifdef WITH_QTWEBKIT
  if ( mDocumentViewerContent == Web )
  {
    mWebView->load( QUrl::fromEncoded( filePath.toUtf8() ) );
    mWebView->page()->settings()->setAttribute( QWebSettings::LocalStorageEnabled, true );
  }
#endif

  if ( mDocumentViewerContent == Image )
  {
    // use an image reader to ensure image orientation and transforms are correctly handled
    QImageReader ir( filePath );
    ir.setAutoTransform( true );
    const QPixmap pm = QPixmap::fromImage( ir.read() );
    if ( !pm.isNull() )
      mPixmapLabel->setPixmap( pm );
    else
      mPixmapLabel->clear();
  }

  updateDocumentViewer();
}

void QgsExternalResourceWidget::clearContent()
{
#ifdef WITH_QTWEBKIT
  if ( mDocumentViewerContent == Web )
  {
    mWebView->load( QUrl( QStringLiteral( "about:blank" ) ) );
  }
#endif
  if ( mDocumentViewerContent == Image )
  {
    mPixmapLabel->clear();
  }

  updateDocumentViewer();
}

void QgsExternalResourceWidget::loadDocument( const QString &path )
{
  if ( path.isEmpty() || path == QgsApplication::nullRepresentation() )
  {
    if ( mFileWidget->externalStorage() && mContent )
    {
      mContent->cancel();
      mContent.clear();
    }

    clearContent();
  }
  else if ( mDocumentViewerContent != NoContent )
  {
    const QString resolvedPath = resolvePath( path );

    if ( mFileWidget->externalStorage() )
    {
      if ( mContent )
      {
        mContent->cancel();
      }

      mContent = mFileWidget->externalStorage()->fetch( resolvedPath, storageAuthConfigId() );

#ifdef WITH_QTWEBKIT
      mWebView->setVisible( false );
#endif
      mPixmapLabel->setVisible( false );
      mErrorLabel->setVisible( false );
      mLoadingLabel->setVisible( true );
      mLoadingMovie->start();
      connect( mContent, &QgsExternalStorageFetchedContent::fetched, this, &QgsExternalResourceWidget::onFetchFinished );
      connect( mContent, &QgsExternalStorageFetchedContent::errorOccurred, this, &QgsExternalResourceWidget::onFetchFinished );
      connect( mContent, &QgsExternalStorageFetchedContent::canceled, this, &QgsExternalResourceWidget::onFetchFinished );

      mContent->fetch();
    }
    else
    {
      updateDocumentContent( resolvedPath );
    }
  }
}

void QgsExternalResourceWidget::onFetchFinished()
{
  QgsExternalStorageFetchedContent *content = qobject_cast<QgsExternalStorageFetchedContent *>( sender() );

  if ( content == mContent && mContent->status() == Qgis::ContentStatus::Failed )
  {
#ifdef WITH_QTWEBKIT
    mWebView->setVisible( false );
#endif
    mPixmapLabel->setVisible( false );
    mLoadingLabel->setVisible( false );
    mLoadingMovie->stop();
    mErrorLabel->setVisible( true );

    if ( messageBar() )
    {
      messageBar()->pushWarning( tr( "Fetching External Resource" ),
                                 tr( "Error while fetching external resource '%1' : %2" ).arg(
                                   mFileWidget->filePath(), mContent->errorString() ) );
    }
  }
  else if ( content == mContent && mContent->status() == Qgis::ContentStatus::Finished )
  {
    const QString filePath = mDocumentViewerContent == Web
                             ? QUrl::fromLocalFile( mContent->filePath() ).toString()
                             : mContent->filePath();

    updateDocumentContent( filePath );
  }

  content->deleteLater();
}
