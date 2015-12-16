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

#include <QGridLayout>
#include <QVariant>
#include <QSettings>
#ifdef WITH_QTWEBKIT
#include <QWebView>
#endif


QgsExternalResourceWidget::QgsExternalResourceWidget( QWidget *parent )
    : QWidget( parent )
    , mFilePickerVisible( true )
    , mDocumentViewerContent( NoContent )
    , mDocumentViewerHeight( 0 )
    , mDocumentViewerWidth( 0 )

{
  setBackgroundRole( QPalette::Window );
  setAutoFillBackground( true );

  QGridLayout* layout = new QGridLayout();
  layout->setMargin( 0 );

  mFilePicker = new QgsFilePickerWidget( this );
  layout->addWidget( mFilePicker, 0, 0 );
  mFilePicker->setVisible( mFilePickerVisible );

  mPixmapLabel = new QgsPixmapLabel( this );
  layout->addWidget( mPixmapLabel, 1, 0 );

#ifdef WITH_QTWEBKIT
  mWebView = new QWebView( this );
  layout->addWidget( mWebView, 2, 0 );
#endif

  updateDocumentViewer();

  setLayout( layout );

  connect( mFilePicker, SIGNAL( fileChanged( QString ) ), this, SLOT( loadDocument( QString ) ) );
}

QVariant QgsExternalResourceWidget::documentPath()
{
  QString path = mFilePicker->filePath();
  if ( path.isNull() || path == QSettings().value( "qgis/nullValue", "NULL" ) )
  {
    return QVariant();
  }
  else
  {
    return path;
  }
}

void QgsExternalResourceWidget::setDocumentPath( QVariant value )
{
  QString path;
  if ( value.isNull() )
  {
    path =  QSettings().value( "qgis/nullValue", "NULL" ).toString();
  }
  else
  {
    path = value.toString();
  }
  mFilePicker->setFilePath( path );
}

QgsFilePickerWidget*QgsExternalResourceWidget::filePickerwidget()
{
  return mFilePicker;
}

bool QgsExternalResourceWidget::filePickerVisible() const
{
  return mFilePickerVisible;
}

void QgsExternalResourceWidget::setFilePickerVisible( bool visible )
{
  mFilePickerVisible = visible;
  mFilePicker->setVisible( visible );
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
  mFilePicker->setReadOnly( readOnly );
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

    if ( pm )
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

void QgsExternalResourceWidget::loadDocument( QString path )
{
  if ( path.isNull() )
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
    }
  }


#ifdef WITH_QTWEBKIT
  if ( mDocumentViewerContent == Web )
  {
    mWebView->setUrl( QUrl( path ) );
  }
#endif

  if ( mDocumentViewerContent == Image )
  {
    QPixmap pm( path );
    if ( !pm.isNull() )
    {
      mPixmapLabel->setPixmap( pm );

      updateDocumentViewer();
    }
  }
}


