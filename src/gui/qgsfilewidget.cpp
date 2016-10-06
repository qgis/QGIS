/***************************************************************************
  qgsfilewidget.cpp

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

#include "qgsfilewidget.h"

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

QgsFileWidget::QgsFileWidget( QWidget *parent )
    : QWidget( parent )
    , mFilePath( QString() )
    , mButtonVisible( true )
    , mUseLink( false )
    , mFullUrl( false )
    , mDialogTitle( QString() )
    , mFilter( QString() )
    , mDefaultRoot( QString() )
    , mStorageMode( GetFile )
    , mRelativeStorage( Absolute )
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
  connect( mLineEdit, SIGNAL( textChanged( QString ) ), this, SLOT( textEdited( QString ) ) );
  layout->addWidget( mLineEdit, 1, 0 );

  mFileWidgetButton = new QToolButton( this );
  mFileWidgetButton->setText( "..." );
  connect( mFileWidgetButton, SIGNAL( clicked() ), this, SLOT( openFileDialog() ) );
  layout->addWidget( mFileWidgetButton, 0, 1, 2, 1 );

  setLayout( layout );
}

QgsFileWidget::~QgsFileWidget()
{
}

QString QgsFileWidget::filePath()
{
  return mFilePath;
}

void QgsFileWidget::setFilePath( QString path )
{
  if ( path == QSettings().value( "qgis/nullValue", "NULL" ) )
  {
    path = "";
  }

  //will trigger textEdited slot
  mLineEdit->setValue( path );
}

void QgsFileWidget::setReadOnly( bool readOnly )
{
  mFileWidgetButton->setEnabled( !readOnly );
  mLineEdit->setEnabled( !readOnly );
}

QString QgsFileWidget::dialogTitle() const
{
  return mDialogTitle;
}

void QgsFileWidget::setDialogTitle( const QString& title )
{
  mDialogTitle = title;
}

QString QgsFileWidget::filter() const
{
  return mFilter;
}

void QgsFileWidget::setFilter( const QString& filters )
{
  mFilter = filters;
}

bool QgsFileWidget::fileWidgetButtonVisible() const
{
  return mButtonVisible;
}

void QgsFileWidget::setFileWidgetButtonVisible( bool visible )
{
  mButtonVisible = visible;
  mFileWidgetButton->setVisible( visible );
}

void QgsFileWidget::textEdited( const QString& path )
{
  mFilePath = path;
  mLinkLabel->setText( toUrl( path ) );
  emit fileChanged( mFilePath );
}

bool QgsFileWidget::useLink() const
{
  return mUseLink;
}

void QgsFileWidget::setUseLink( bool useLink )
{
  mUseLink = useLink;
  mLinkLabel->setVisible( mUseLink );
  mLineEdit->setVisible( !mUseLink );
}

bool QgsFileWidget::fullUrl() const
{
  return mFullUrl;
}

void QgsFileWidget::setFullUrl( bool fullUrl )
{
  mFullUrl = fullUrl;
}

QString QgsFileWidget::defaultRoot() const
{
  return mDefaultRoot;
}

void QgsFileWidget::setDefaultRoot( const QString& defaultRoot )
{
  mDefaultRoot = defaultRoot;
}

QgsFileWidget::StorageMode QgsFileWidget::storageMode() const
{
  return mStorageMode;
}

void QgsFileWidget::setStorageMode( QgsFileWidget::StorageMode storageMode )
{
  mStorageMode = storageMode;
}

QgsFileWidget::RelativeStorage QgsFileWidget::relativeStorage() const
{
  return mRelativeStorage;
}

void QgsFileWidget::setRelativeStorage( QgsFileWidget::RelativeStorage relativeStorage )
{
  mRelativeStorage = relativeStorage;
}

void QgsFileWidget::openFileDialog()
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
    QString defPath = QDir::cleanPath( QgsProject::instance()->fileInfo().absolutePath() );
    if ( defPath.isEmpty() )
    {
      defPath = QDir::homePath();
    }
    oldPath = settings.value( "/UI/lastExternalResourceWidgetDefaultPath", defPath ).toString();
  }

  // Handle Storage
  QString fileName;
  QString title;
  if ( mStorageMode == GetFile )
  {
    title = !mDialogTitle.isEmpty() ? mDialogTitle : tr( "Select a file" );
    fileName = QFileDialog::getOpenFileName( this, title, QFileInfo( oldPath ).absoluteFilePath(), mFilter );
  }
  else if ( mStorageMode == GetDirectory )
  {
    title = !mDialogTitle.isEmpty() ? mDialogTitle : tr( "Select a directory" );
    fileName = QFileDialog::getExistingDirectory( this, title, QFileInfo( oldPath ).absoluteFilePath(),  QFileDialog::ShowDirsOnly );
  }

  if ( fileName.isEmpty() )
    return;


  fileName = QDir::toNativeSeparators( QDir::cleanPath( QFileInfo( fileName ).absoluteFilePath() ) );
  // Store the last used path:

  if ( mStorageMode == GetFile )
  {
    settings.setValue( "/UI/lastFileNameWidgetDir", QFileInfo( fileName ).absolutePath() );
  }
  else if ( mStorageMode == GetDirectory )
  {
    settings.setValue( "/UI/lastFileNameWidgetDir", fileName );
  }

  // Handle relative Path storage
  fileName = relativePath( fileName, true );

  // Keep the new value
  setFilePath( fileName );
}


QString QgsFileWidget::relativePath( const QString& filePath, bool removeRelative ) const
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
    {
      return QDir::cleanPath( QDir( RelativePath ).relativeFilePath( filePath ) );
    }
    else
    {
      return QDir::cleanPath( QDir( RelativePath ).filePath( filePath ) );
    }
  }

  return filePath;
}


QString QgsFileWidget::toUrl( const QString& path ) const
{
  QString rep;
  if ( path.isEmpty() )
  {
    return QSettings().value( "qgis/nullValue", "NULL" ).toString();
  }

  QString urlStr = relativePath( path, false );
  QUrl url = QUrl::fromUserInput( urlStr );
  if ( !url.isValid() || !url.isLocalFile() )
  {
    QgsDebugMsg( QString( "URL: %1 is not valid or not a local file!" ).arg( path ) );
    rep =  path;
  }

  QString pathStr = url.toString();
  if ( mFullUrl )
  {
    rep = QString( "<a href=\"%1\">%2</a>" ).arg( pathStr, path );
  }
  else
  {
    QString fileName = QFileInfo( urlStr ).fileName();
    rep = QString( "<a href=\"%1\">%2</a>" ).arg( pathStr, fileName );
  }

  return rep;
}
