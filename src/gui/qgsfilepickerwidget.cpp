/***************************************************************************
  qgsfilepickerwidget.cpp

 ---------------------
 begin                : 17.12.2015
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

#include "qgsfilepickerwidget.h"

#include <QLineEdit>
#include <QToolButton>
#include <QLabel>
#include <QFileDialog>
#include <QSettings>
#include <QGridLayout>
#include <QUrl>

#include "qgsfilterlineedit.h"
#include "qgslogger.h"
#include "qgsproject.h"

QgsFilePickerWidget::QgsFilePickerWidget( QWidget *parent )
    : QWidget( parent )
    , mFilePath( QString() )
    , mButtonVisible( true )
    , mUseLink( false )
    , mFullUrl( false )
    , mDefaultRoot( QString() )
    , mStorageMode( File )
{
  setBackgroundRole( QPalette::Window );
  setAutoFillBackground( true );

  QGridLayout* layout = new QGridLayout();
  layout->setMargin( 0 );

  // If displaying a hyperlink, use a QLabel
  mLinkLabel = new QLabel( this );
  // Make Qt opens the link with the OS defined viewer
  mLinkLabel->setOpenExternalLinks( true );
  // Label should always be enabled to be able to open
  // the link on read only mode.
  mLinkLabel->setEnabled( true );
  mLinkLabel->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
  mLinkLabel->setTextFormat( Qt::RichText );
  layout->addWidget( mLinkLabel, 0, 0 );
  mLinkLabel->hide(); // do not show by default

  // otherwise, use the traditional QLineEdit
  mLineEdit = new QgsFilterLineEdit( this );
  connect( mLineEdit, SIGNAL( textEdited( QString ) ), this, SLOT( textEdited( QString ) ) );
  layout->addWidget( mLineEdit, 1, 0 );

  mFilePickerButton = new QToolButton( this );
  mFilePickerButton->setText( "..." );
  connect( mFilePickerButton, SIGNAL( clicked() ), this, SLOT( openFileDialog() ) );
  layout->addWidget( mFilePickerButton, 0, 1, 2, 1 );

  setLayout( layout );
}

QString QgsFilePickerWidget::filePath()
{
  return mFilePath;
}

void QgsFilePickerWidget::setFilePath( QString path )
{
  mFilePath = path;
  mLineEdit->setText( path );
  mLinkLabel->setText( toUrl( path ) );
  emit fileChanged( mFilePath );
}

void QgsFilePickerWidget::setReadOnly( bool readOnly )
{
  mFilePickerButton->setEnabled( !readOnly );
  mLineEdit->setEnabled( !readOnly );
}

bool QgsFilePickerWidget::filePickerButtonVisible() const
{
  return mButtonVisible;
}

void QgsFilePickerWidget::setFilePickerButtonVisible( bool visible )
{
  mButtonVisible = visible;
  mFilePickerButton->setVisible( visible );
}

void QgsFilePickerWidget::textEdited( QString path )
{
  mFilePath = path;
  mLinkLabel->setText( toUrl( path ) );
  emit fileChanged( mFilePath );
}

bool QgsFilePickerWidget::useLink() const
{
  return mUseLink;
}

void QgsFilePickerWidget::setUseLink( bool useLink )
{
  mUseLink = useLink;
  mLinkLabel->setVisible( mUseLink );
  mLineEdit->setVisible( !mUseLink );
}

bool QgsFilePickerWidget::fullUrl() const
{
  return mFullUrl;
}

void QgsFilePickerWidget::setFullUrl( bool fullUrl )
{
  mFullUrl = fullUrl;
}

QString QgsFilePickerWidget::defaultRoot() const
{
  return mDefaultRoot;
}

void QgsFilePickerWidget::setDefaultRoot( QString defaultRoot )
{
  mDefaultRoot = defaultRoot;
}

QgsFilePickerWidget::StorageMode QgsFilePickerWidget::storageMode() const
{
  return mStorageMode;
}

void QgsFilePickerWidget::setStorageMode( QgsFilePickerWidget::StorageMode storageMode )
{
  mStorageMode = storageMode;
}

QgsFilePickerWidget::RelativeStorage QgsFilePickerWidget::relativeStorage()
{
  return mRelativeStorage;
}

void QgsFilePickerWidget::setRelativeStorage( QgsFilePickerWidget::RelativeStorage relativeStorage )
{
  mRelativeStorage = relativeStorage;
}

void QgsFilePickerWidget::openFileDialog()
{
  QSettings settings;
  QString oldPath;

  // If we use fixed default path
  if ( !mDefaultRoot.isEmpty() )
  {
    oldPath = QDir::cleanPath( mDefaultRoot );
  }
  // if we use a relative path option, we need to obtain the full path
  else if ( !mFilePath.isEmpty() )
  {
    oldPath = relativePath( mFilePath, false );
  }

  // If there is no valid value, find a default path to use
  QUrl theUrl = QUrl::fromUserInput( oldPath );
  if ( !theUrl.isValid() )
  {
    oldPath = settings.value( "/UI/lastExternalResourceWidgetDir", QDir::cleanPath( QgsProject::instance()->fileInfo().absolutePath() ) ).toString();
  }

  // Handle Storage
  QString fileName;
  if ( mStorageMode == File )
  {
    fileName = QFileDialog::getOpenFileName( this, tr( "Select a file" ), QFileInfo( oldPath ).absoluteFilePath() );
  }
  else if ( mStorageMode == Directory )
  {
    fileName = QFileDialog::getExistingDirectory( this, tr( "Select a directory" ), QFileInfo( oldPath ).absoluteFilePath(),  QFileDialog::ShowDirsOnly );
  }

  if ( fileName.isEmpty() )
    return;


  fileName = QDir::toNativeSeparators( QDir::cleanPath( QFileInfo( fileName ).absoluteFilePath() ) );
  // Store the last used path:

  if ( mStorageMode == File )
    settings.setValue( "/UI/lastFileNameWidgetDir", QFileInfo( fileName ).absolutePath() );
  else if ( mStorageMode == Directory )
    settings.setValue( "/UI/lastFileNameWidgetDir", fileName );

  // Handle relative Path storage
  fileName = relativePath( fileName, true );

  // Keep the new value
  setFilePath( fileName );
}


QString QgsFilePickerWidget::relativePath( QString filePath, bool removeRelative )
{
  QString RelativePath;
  if ( mRelativeStorage == RelativeProject )
  {
    RelativePath = QDir::toNativeSeparators( QDir::cleanPath( QgsProject::instance()->fileInfo().absolutePath() ) );
  }
  else if ( mRelativeStorage == RelativeDefaultPath && !mDefaultRoot.isEmpty() )
  {
    RelativePath = QDir::toNativeSeparators( QDir::cleanPath( mDefaultRoot ) );
  }
  if ( !RelativePath.isEmpty() )
  {
    if ( removeRelative )
      return QDir::cleanPath( QDir( RelativePath ).relativeFilePath( filePath ) );
    else
      return QDir::cleanPath( QDir( RelativePath ).filePath( filePath ) );
  }

  return filePath;
}


QString QgsFilePickerWidget::toUrl( const QString& value )
{
  QString rep;
  if ( value.isEmpty() )
  {
    rep =  QSettings().value( "qgis/nullValue", "NULL" ).toString();
  }

  QString urlStr = relativePath( value, false );
  QUrl theUrl = QUrl::fromUserInput( urlStr );
  if ( !theUrl.isValid() or !theUrl.isLocalFile() )
  {
    QgsDebugMsg( QString( "URL: %1 is not valid or not a local file !" ).arg( value ) );
    rep =  value;
  }

  QString filePath = theUrl.toString();
  if ( mFullUrl )
  {
    rep = QString( "<a href=\"%1\">%2</a>" ).arg( filePath, urlStr );
  }
  else
  {
    QString fileName = QFileInfo( urlStr ).fileName();
    rep = QString( "<a href=\"%1\">%2</a>" ).arg( filePath, fileName );
  }

  return rep;
}
