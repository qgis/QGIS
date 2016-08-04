/***************************************************************************
    qgsphotowidgetwrapper.cpp
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsphotowidgetwrapper.h"
#include "qgspixmaplabel.h"
#include "qgsproject.h"

#include <QGridLayout>
#include <QFileDialog>
#include <QSettings>
#include <QUrl>

#include "qgsfilterlineedit.h"

QgsPhotoWidgetWrapper::QgsPhotoWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent )
    :  QgsEditorWidgetWrapper( vl, fieldIdx, editor, parent )
    , mPhotoLabel( nullptr )
    , mPhotoPixmapLabel( nullptr )
    , mLineEdit( nullptr )
    , mButton( nullptr )
{
#ifdef WITH_QTWEBKIT
  mWebView = nullptr;
#endif
}

void QgsPhotoWidgetWrapper::selectFileName()
{
  if ( !mLineEdit )
    return;

  QString fileName = QFileDialog::getOpenFileName( nullptr, tr( "Select a picture" ), QFileInfo( mLineEdit->text() ).absolutePath() );

  if ( fileName.isNull() )
    return;

  QString projPath = QDir::toNativeSeparators( QDir::cleanPath( QgsProject::instance()->fileInfo().absolutePath() ) );
  QString filePath = QDir::toNativeSeparators( QDir::cleanPath( QFileInfo( fileName ).absoluteFilePath() ) );

  if ( filePath.startsWith( projPath ) )
    filePath = QDir( projPath ).relativeFilePath( filePath );

  mLineEdit->setText( filePath );
}

void QgsPhotoWidgetWrapper::loadPixmap( const QString& fileName )
{
  if ( fileName.isEmpty() )
  {
#ifdef WITH_QTWEBKIT
    if ( mWebView )
    {
      mWebView->setUrl( QString() );
    }
#endif
    clearPicture();
    return;
  }

  QString filePath = fileName;

  if ( QUrl( fileName ).isRelative() )
    filePath = QDir( QgsProject::instance()->fileInfo().absolutePath() ).filePath( fileName );

#ifdef WITH_QTWEBKIT
  if ( mWebView )
  {
    mWebView->setUrl( filePath );
  }
#endif

  QPixmap pm( filePath );
  if ( !pm.isNull() && mPhotoLabel )
  {
    QSize size( config( "Width" ).toInt(), config( "Height" ).toInt() );
    if ( size.width() == 0 && size.height() > 0 )
    {
      size.setWidth( size.height() * pm.size().width() / pm.size().height() );
    }
    else if ( size.width() > 0 && size.height() == 0 )
    {
      size.setHeight( size.width() * pm.size().height() / pm.size().width() );
    }

    if ( mPhotoPixmapLabel )
    {
      mPhotoPixmapLabel->setPixmap( pm );

      if ( size.width() != 0 || size.height() != 0 )
      {
        mPhotoPixmapLabel->setMinimumSize( size );
        mPhotoPixmapLabel->setMaximumSize( size );
      }
    }
    else // mPhotoLabel is checked in the outer if branch
    {
      mPhotoLabel->setMinimumSize( size );
      pm = pm.scaled( size, Qt::KeepAspectRatio, Qt::SmoothTransformation );
      mPhotoLabel->setPixmap( pm );
    }
  }
  else
  {
    clearPicture();
  }
}

void QgsPhotoWidgetWrapper::clearPicture()
{
  if ( mPhotoLabel )
  {
    mPhotoLabel->clear();
    mPhotoLabel->setMinimumSize( QSize( 0, 0 ) );

    if ( mPhotoPixmapLabel )
      mPhotoPixmapLabel->setPixmap( QPixmap() );
    else
      mPhotoLabel->setPixmap( QPixmap() );
  }
}

QVariant QgsPhotoWidgetWrapper::value() const
{
  QVariant v;

  if ( mLineEdit )
  {
    if ( mLineEdit->text() == QSettings().value( "qgis/nullValue", "NULL" ).toString() )
      v = QVariant( QVariant::String );
    else
      v = mLineEdit->text();
  }

  return v;
}

void QgsPhotoWidgetWrapper::showIndeterminateState()
{
  if ( mLineEdit )
  {
    whileBlocking( mLineEdit )->clear();
  }
  if ( mPhotoLabel )
    mPhotoLabel->clear();
  if ( mPhotoPixmapLabel )
    mPhotoPixmapLabel->clear();
}

QWidget* QgsPhotoWidgetWrapper::createWidget( QWidget* parent )
{
  QWidget* container = new QWidget( parent );
  QGridLayout* layout = new QGridLayout();
  QgsFilterLineEdit* le = new QgsFilterLineEdit();
  QgsPixmapLabel* label = new QgsPixmapLabel();
  label->setObjectName( "PhotoLabel" );
  QPushButton* pb = new QPushButton( tr( "..." ) );
  pb->setObjectName( "FileChooserButton" );

  layout->addWidget( label, 0, 0, 1, 2 );
  layout->addWidget( le, 1, 0 );
  layout->addWidget( pb, 1, 1 );
  layout->setMargin( 0 );
  layout->setContentsMargins( 0, 0, 0, 0 );

  container->setLayout( layout );

  return container;
}

void QgsPhotoWidgetWrapper::initWidget( QWidget* editor )
{
  QWidget* container;

  mLineEdit = qobject_cast<QLineEdit*>( editor );
#ifdef WITH_QTWEBKIT
  mWebView = qobject_cast<QWebView*>( editor );
#endif

  if ( mLineEdit )
  {
    container = mLineEdit->parentWidget();
  }
#ifdef WITH_QTWEBKIT
  else if ( mWebView )
  {
    container = mWebView->parentWidget();
    mLineEdit = container->findChild<QLineEdit*>();
  }
#endif
  else
  {
    container = editor;
    mLineEdit = container->findChild<QLineEdit*>();
  }

  mButton = container->findChild<QPushButton*>( "FileChooserButton" );
  if ( !mButton )
    mButton = container->findChild<QPushButton*>();

  mPhotoLabel = container->findChild<QLabel*>( "PhotoLabel" );
  if ( !mPhotoLabel )
    mPhotoLabel = container->findChild<QLabel*>();

  mPhotoPixmapLabel = qobject_cast<QgsPixmapLabel*>( mPhotoLabel );
  if ( mButton )
    connect( mButton, SIGNAL( clicked() ), this, SLOT( selectFileName() ) );

  if ( mLineEdit )
  {

    QgsFilterLineEdit *fle = qobject_cast<QgsFilterLineEdit*>( mLineEdit );
    if ( fle )
    {
      fle->setNullValue( QSettings().value( "qgis/nullValue", "NULL" ).toString() );
    }

    connect( mLineEdit, SIGNAL( textChanged( QString ) ), this, SLOT( valueChanged( QString ) ) );
    connect( mLineEdit, SIGNAL( textChanged( QString ) ), this, SLOT( loadPixmap( QString ) ) );
  }
}

bool QgsPhotoWidgetWrapper::valid() const
{
#ifdef WITH_QTWEBKIT
  return mPhotoLabel || mLineEdit || mButton || mWebView;
#else
  return mPhotoLabel || mLineEdit || mButton;
#endif
}

void QgsPhotoWidgetWrapper::setValue( const QVariant& value )
{
  if ( mLineEdit )
  {
    if ( value.isNull() )
    {
      whileBlocking( mLineEdit )->setText( QSettings().value( "qgis/nullValue", "NULL" ).toString() );
      clearPicture();
    }
    else
      mLineEdit->setText( value.toString() );
  }
  else
  {
    loadPixmap( value.toString() );
  }
}

void QgsPhotoWidgetWrapper::setEnabled( bool enabled )
{
  if ( mLineEdit )
    mLineEdit->setEnabled( enabled );

  if ( mButton )
    mButton->setEnabled( enabled );
}

void QgsPhotoWidgetWrapper::updateConstraintWidgetStatus( bool constraintValid )
{
  if ( mLineEdit )
  {
    if ( constraintValid )
      mLineEdit->setStyleSheet( QString() );
    else
    {
      mLineEdit->setStyleSheet( "QgsFilterLineEdit { background-color: #dd7777; }" );
    }
  }
}
