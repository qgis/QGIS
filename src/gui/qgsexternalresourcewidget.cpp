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

#include <QDir>
#include <QGridLayout>
#include <QVariant>
#include <QSettings>
#ifdef WITH_QTWEBKIT
#include <QWebView>
#endif


QgsExternalResourceWidget::QgsExternalResourceWidget( QWidget *parent )
    : QWidget( parent )
    , mFileWidgetVisible( true )
    , mDocumentViewerContent( NoContent )
    , mDocumentViewerHeight( 0 )
    , mDocumentViewerWidth( 0 )
    , mRelativeStorage( QgsFileWidget::Absolute )

{
  setBackgroundRole( QPalette::Window );
  setAutoFillBackground( true );

  QGridLayout* layout = new QGridLayout();
  layout->setMargin( 0 );

  mFileWidget = new QgsFileWidget( this );
  layout->addWidget( mFileWidget, 0, 0 );
  mFileWidget->setVisible( mFileWidgetVisible );

  mPixmapLabel = new QgsPixmapLabel( this );
  layout->addWidget( mPixmapLabel, 1, 0 );

#ifdef WITH_QTWEBKIT
  mWebView = new QWebView( this );
  layout->addWidget( mWebView, 2, 0 );
#endif

  updateDocumentViewer();

  setLayout( layout );

  connect( mFileWidget, SIGNAL( fileChanged( QString ) ), this, SLOT( loadDocument( QString ) ) );
}

QVariant QgsExternalResourceWidget::documentPath( QVariant::Type type ) const
{
  QString path = mFileWidget->filePath();
  if ( path.isEmpty() )
  {
    return QVariant( type );
  }
  else
  {
    return path;
  }
}

void QgsExternalResourceWidget::setDocumentPath( const QVariant& path )
{
  mFileWidget->setFilePath( path.toString() );
}

QgsFileWidget*QgsExternalResourceWidget::fileWidget()
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
  updateDocumentViewer();
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
#ifdef WITH_QTWEBKIT
  mWebView->setVisible( mDocumentViewerContent == Web );
#endif

  mPixmapLabel->setVisible( mDocumentViewerContent == Image );

  if ( mDocumentViewerContent == Image )
  {
    const QPixmap* pm = mPixmapLabel->pixmap();

    if ( !pm || pm->isNull() )
    {
      mPixmapLabel->setMinimumSize( QSize( 0, 0 ) );
    }
    else
    {
      QSize size( mDocumentViewerWidth, mDocumentViewerHeight );
      if ( size.width() == 0 && size.height() > 0 )
      {
        size.setWidth( size.height() * pm->size().width() / pm->size().height() );
      }
      else if ( size.width() > 0 && size.height() == 0 )
      {
        size.setHeight( size.width() * pm->size().height() / pm->size().width() );
      }

      if ( size.width() != 0 || size.height() != 0 )
      {
        mPixmapLabel->setMinimumSize( size );
        mPixmapLabel->setMaximumSize( size );
      }
    }
  }
}

QString QgsExternalResourceWidget::resolvePath( const QString& path )
{
  switch ( mRelativeStorage )
  {
    case QgsFileWidget::Absolute:
      return path;
      break;
    case QgsFileWidget::RelativeProject:
      return QgsProject::instance()->fileInfo().dir().filePath( path );
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

void QgsExternalResourceWidget::setDefaultRoot( const QString& defaultRoot )
{
  mFileWidget->setDefaultRoot( defaultRoot );
  mDefaultRoot = defaultRoot;
}

QgsFileWidget::RelativeStorage QgsExternalResourceWidget::relativeStorage() const
{
  return mRelativeStorage;
}

void QgsExternalResourceWidget::setRelativeStorage( const QgsFileWidget::RelativeStorage& relativeStorage )
{
  mFileWidget->setRelativeStorage( relativeStorage );
  mRelativeStorage = relativeStorage;
}

void QgsExternalResourceWidget::loadDocument( const QString& path )
{
  QString resolvedPath;

  if ( path.isEmpty() )
  {
#ifdef WITH_QTWEBKIT
    if ( mDocumentViewerContent == Web )
    {
      mWebView->setUrl( QUrl( "about:blank" ) );
    }
#endif
    if ( mDocumentViewerContent == Image )
    {
      mPixmapLabel->clear();
      updateDocumentViewer();
    }
  }
  else
  {
    resolvedPath = resolvePath( path );

#ifdef WITH_QTWEBKIT
    if ( mDocumentViewerContent == Web )
    {
      mWebView->setUrl( QUrl( resolvedPath ) );
    }
#endif

    if ( mDocumentViewerContent == Image )
    {
      QPixmap pm( resolvedPath );
      mPixmapLabel->setPixmap( pm );
      updateDocumentViewer();
    }
  }
}


