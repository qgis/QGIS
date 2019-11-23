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
#include <QGridLayout>
#include <QUrl>
#include <QDropEvent>

#include "qgssettings.h"
#include "qgsfilterlineedit.h"
#include "qgslogger.h"
#include "qgsproject.h"
#include "qgsapplication.h"
#include "qgsfileutils.h"
#include "qgsmimedatautils.h"

QgsFileWidget::QgsFileWidget( QWidget *parent )
  : QWidget( parent )
{
  setBackgroundRole( QPalette::Window );
  setAutoFillBackground( true );

  mLayout = new QHBoxLayout();
  mLayout->setMargin( 0 );

  // If displaying a hyperlink, use a QLabel
  mLinkLabel = new QLabel( this );
  // Make Qt opens the link with the OS defined viewer
  mLinkLabel->setOpenExternalLinks( true );
  // Label should always be enabled to be able to open
  // the link on read only mode.
  mLinkLabel->setEnabled( true );
  mLinkLabel->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
  mLinkLabel->setTextFormat( Qt::RichText );
  mLinkLabel->hide(); // do not show by default

  // otherwise, use the traditional QLineEdit subclass
  mLineEdit = new QgsFileDropEdit( this );
  mLineEdit->setDragEnabled( true );
  mLineEdit->setToolTip( tr( "Full path to the file(s), including name and extension" ) );
  connect( mLineEdit, &QLineEdit::textChanged, this, &QgsFileWidget::textEdited );
  mLayout->addWidget( mLineEdit );

  mFileWidgetButton = new QToolButton( this );
  mFileWidgetButton->setText( QChar( 0x2026 ) );
  mFileWidgetButton->setToolTip( tr( "Browse" ) );
  connect( mFileWidgetButton, &QAbstractButton::clicked, this, &QgsFileWidget::openFileDialog );
  mLayout->addWidget( mFileWidgetButton );

  setLayout( mLayout );
}

QString QgsFileWidget::filePath()
{
  return mFilePath;
}

QStringList QgsFileWidget::splitFilePaths( const QString &path )
{
  QStringList paths;
  const QStringList pathParts = path.split( QRegExp( "\"\\s+\"" ), QString::SkipEmptyParts );
  for ( const auto &pathsPart : pathParts )
  {
    QString cleaned = pathsPart;
    cleaned.remove( QRegExp( "(^\\s*\")|(\"\\s*)" ) );
    paths.append( cleaned );
  }
  return paths;
}

void QgsFileWidget::setFilePath( QString path )
{
  if ( path == QgsApplication::nullRepresentation() )
  {
    path.clear();
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

void QgsFileWidget::setDialogTitle( const QString &title )
{
  mDialogTitle = title;
}

QString QgsFileWidget::filter() const
{
  return mFilter;
}

void QgsFileWidget::setFilter( const QString &filters )
{
  mFilter = filters;
  mLineEdit->setFilters( filters );
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

void QgsFileWidget::textEdited( const QString &path )
{
  mFilePath = path;
  mLinkLabel->setText( toUrl( path ) );
  // Show tooltip if multiple files are selected
  if ( path.contains( QStringLiteral( "\" \"" ) ) )
  {
    mLineEdit->setToolTip( tr( "Selected files:<br><ul><li>%1</li></ul><br>" ).arg( splitFilePaths( path ).join( QStringLiteral( "</li><li>" ) ) ) );
  }
  else
  {
    mLineEdit->setToolTip( QString() );
  }
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
  if ( mUseLink )
  {
    mLayout->removeWidget( mLineEdit );
    mLayout->insertWidget( 0, mLinkLabel );
  }
  else
  {
    mLayout->removeWidget( mLinkLabel );
    mLayout->insertWidget( 0, mLineEdit );
  }
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

void QgsFileWidget::setDefaultRoot( const QString &defaultRoot )
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
  mLineEdit->setStorageMode( storageMode );
}

QgsFileWidget::RelativeStorage QgsFileWidget::relativeStorage() const
{
  return mRelativeStorage;
}

void QgsFileWidget::setRelativeStorage( QgsFileWidget::RelativeStorage relativeStorage )
{
  mRelativeStorage = relativeStorage;
}

QgsFilterLineEdit *QgsFileWidget::lineEdit()
{
  return mLineEdit;
}

void QgsFileWidget::openFileDialog()
{
  QgsSettings settings;
  QString oldPath;

  // if we use a relative path option, we need to obtain the full path
  // first choice is the current file path, if one is entered
  if ( !mFilePath.isEmpty() )
  {
    oldPath = relativePath( mFilePath, false );
  }
  // If we use fixed default path
  // second choice is the default root
  else if ( !mDefaultRoot.isEmpty() )
  {
    oldPath = QDir::cleanPath( mDefaultRoot );
  }

  // If there is no valid value, find a default path to use
  QUrl url = QUrl::fromUserInput( oldPath );
  if ( !url.isValid() )
  {
    QString defPath = QDir::cleanPath( QFileInfo( QgsProject::instance()->absoluteFilePath() ).path() );
    if ( defPath.isEmpty() )
    {
      defPath = QDir::homePath();
    }
    oldPath = settings.value( QStringLiteral( "UI/lastFileNameWidgetDir" ), defPath ).toString();
  }

  // Handle Storage
  QString fileName;
  QStringList fileNames;
  QString title;

  emit blockEvents( true );
  switch ( mStorageMode )
  {
    case GetFile:
      title = !mDialogTitle.isEmpty() ? mDialogTitle : tr( "Select a file" );
      fileName = QFileDialog::getOpenFileName( this, title, QFileInfo( oldPath ).absoluteFilePath(), mFilter, &mSelectedFilter );
      break;
    case GetMultipleFiles:
      title = !mDialogTitle.isEmpty() ? mDialogTitle : tr( "Select one or more files" );
      fileNames = QFileDialog::getOpenFileNames( this, title, QFileInfo( oldPath ).absoluteFilePath(), mFilter, &mSelectedFilter );
      break;
    case GetDirectory:
      title = !mDialogTitle.isEmpty() ? mDialogTitle : tr( "Select a directory" );
      fileName = QFileDialog::getExistingDirectory( this, title, QFileInfo( oldPath ).absoluteFilePath(),  QFileDialog::ShowDirsOnly );
      break;
    case SaveFile:
    {
      title = !mDialogTitle.isEmpty() ? mDialogTitle : tr( "Create or select a file" );
      if ( !confirmOverwrite() )
      {
        fileName = QFileDialog::getSaveFileName( this, title, QFileInfo( oldPath ).absoluteFilePath(), mFilter, &mSelectedFilter, QFileDialog::DontConfirmOverwrite );
      }
      else
      {
        fileName = QFileDialog::getSaveFileName( this, title, QFileInfo( oldPath ).absoluteFilePath(), mFilter, &mSelectedFilter );
      }

      // make sure filename ends with filter. This isn't automatically done by
      // getSaveFileName on some platforms (e.g. gnome)
      fileName = QgsFileUtils::addExtensionFromFilter( fileName, mSelectedFilter );
    }
    break;
  }
  emit blockEvents( false );

  if ( fileName.isEmpty() && fileNames.isEmpty( ) )
    return;

  if ( mStorageMode != GetMultipleFiles )
  {
    fileName = QDir::toNativeSeparators( QDir::cleanPath( QFileInfo( fileName ).absoluteFilePath() ) );
  }
  else
  {
    for ( int i = 0; i < fileNames.length(); i++ )
    {
      fileNames.replace( i, QDir::toNativeSeparators( QDir::cleanPath( QFileInfo( fileNames.at( i ) ).absoluteFilePath() ) ) );
    }
  }

  // Store the last used path:
  switch ( mStorageMode )
  {
    case GetFile:
    case SaveFile:
      settings.setValue( QStringLiteral( "UI/lastFileNameWidgetDir" ), QFileInfo( fileName ).absolutePath() );
      break;
    case GetDirectory:
      settings.setValue( QStringLiteral( "UI/lastFileNameWidgetDir" ), fileName );
      break;
    case GetMultipleFiles:
      settings.setValue( QStringLiteral( "UI/lastFileNameWidgetDir" ), QFileInfo( fileNames.first( ) ).absolutePath() );
      break;
  }

  // Handle relative Path storage
  if ( mStorageMode != GetMultipleFiles )
  {
    fileName = relativePath( fileName, true );
    setFilePath( fileName );
  }
  else
  {
    for ( int i = 0; i < fileNames.length(); i++ )
    {
      fileNames.replace( i, relativePath( fileNames.at( i ), true ) );
    }
    if ( fileNames.length() > 1 )
    {
      setFilePath( QStringLiteral( "\"%1\"" ).arg( fileNames.join( QStringLiteral( "\" \"" ) ) ) );
    }
    else
    {
      setFilePath( fileNames.first( ) );
    }
  }
}


QString QgsFileWidget::relativePath( const QString &filePath, bool removeRelative ) const
{
  QString RelativePath;
  if ( mRelativeStorage == RelativeProject )
  {
    RelativePath = QDir::toNativeSeparators( QDir::cleanPath( QFileInfo( QgsProject::instance()->absoluteFilePath() ).path() ) );
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


QString QgsFileWidget::toUrl( const QString &path ) const
{
  QString rep;
  if ( path.isEmpty() )
  {
    return QgsApplication::nullRepresentation();
  }

  QString urlStr = relativePath( path, false );
  QUrl url = QUrl::fromUserInput( urlStr );
  if ( !url.isValid() || !url.isLocalFile() )
  {
    QgsDebugMsg( QStringLiteral( "URL: %1 is not valid or not a local file!" ).arg( path ) );
    rep = path;
  }

  QString pathStr = url.toString();
  if ( mFullUrl )
  {
    rep = QStringLiteral( "<a href=\"%1\">%2</a>" ).arg( pathStr, path );
  }
  else
  {
    QString fileName = QFileInfo( urlStr ).fileName();
    rep = QStringLiteral( "<a href=\"%1\">%2</a>" ).arg( pathStr, fileName );
  }

  return rep;
}



///@cond PRIVATE


QgsFileDropEdit::QgsFileDropEdit( QWidget *parent )
  : QgsFilterLineEdit( parent )
{
  mDragActive = false;
  setAcceptDrops( true );
}

void QgsFileDropEdit::setFilters( const QString &filters )
{
  mAcceptableExtensions.clear();

  if ( filters.contains( QStringLiteral( "*.*" ) ) )
    return; // everything is allowed!

  QRegularExpression rx( QStringLiteral( "\\*\\.(\\w+)" ) );
  QRegularExpressionMatchIterator i = rx.globalMatch( filters );
  while ( i.hasNext() )
  {
    QRegularExpressionMatch match = i.next();
    if ( match.hasMatch() )
    {
      mAcceptableExtensions << match.captured( 1 ).toLower();
    }
  }
}

QString QgsFileDropEdit::acceptableFilePath( QDropEvent *event ) const
{
  QStringList rawPaths;
  QStringList paths;
  if ( event->mimeData()->hasUrls() )
  {
    const QList< QUrl > urls = event->mimeData()->urls();
    rawPaths.reserve( urls.count() );
    for ( const QUrl &url : urls )
    {
      const QString local =  url.toLocalFile();
      if ( !rawPaths.contains( local ) )
        rawPaths.append( local );
    }
  }

  QgsMimeDataUtils::UriList lst = QgsMimeDataUtils::decodeUriList( event->mimeData() );
  for ( const QgsMimeDataUtils::Uri &u : lst )
  {
    if ( !rawPaths.contains( u.uri ) )
      rawPaths.append( u.uri );
  }

  if ( !event->mimeData()->text().isEmpty() && !rawPaths.contains( event->mimeData()->text() ) )
    rawPaths.append( event->mimeData()->text() );

  paths.reserve( rawPaths.count() );
  for ( const QString &path : qgis::as_const( rawPaths ) )
  {
    QFileInfo file( path );
    switch ( mStorageMode )
    {
      case QgsFileWidget::GetFile:
      case QgsFileWidget::GetMultipleFiles:
      case QgsFileWidget::SaveFile:
      {
        if ( file.isFile() && ( mAcceptableExtensions.isEmpty() || mAcceptableExtensions.contains( file.suffix(), Qt::CaseInsensitive ) ) )
          paths.append( file.filePath() );

        break;
      }

      case QgsFileWidget::GetDirectory:
      {
        if ( file.isDir() )
          paths.append( file.filePath() );
        else if ( file.isFile() )
        {
          // folder mode, but a file dropped. So get folder name from file
          paths.append( file.absolutePath() );
        }

        break;
      }
    }
  }

  if ( paths.size() > 1 )
  {
    return QStringLiteral( "\"%1\"" ).arg( paths.join( QStringLiteral( "\" \"" ) ) );
  }
  else if ( paths.size() == 1 )
  {
    return paths.first();
  }
  else
  {
    return QString();
  }
}

void QgsFileDropEdit::dragEnterEvent( QDragEnterEvent *event )
{
  QString filePath = acceptableFilePath( event );
  if ( !filePath.isEmpty() )
  {
    event->acceptProposedAction();
    mDragActive = true;
    update();
  }
  else
  {
    event->ignore();
  }
}

void QgsFileDropEdit::dragLeaveEvent( QDragLeaveEvent *event )
{
  QgsFilterLineEdit::dragLeaveEvent( event );
  event->accept();
  mDragActive = false;
  update();
}

void QgsFileDropEdit::dropEvent( QDropEvent *event )
{
  QString filePath = acceptableFilePath( event );
  if ( !filePath.isEmpty() )
  {
    setText( filePath );
    selectAll();
    setFocus( Qt::MouseFocusReason );
    event->acceptProposedAction();
    mDragActive = false;
    update();
  }
}

void QgsFileDropEdit::paintEvent( QPaintEvent *e )
{
  QgsFilterLineEdit::paintEvent( e );
  if ( mDragActive )
  {
    QPainter p( this );
    int width = 2;  // width of highlight rectangle inside frame
    p.setPen( QPen( palette().highlight(), width ) );
    QRect r = rect().adjusted( width, width, -width, -width );
    p.drawRect( r );
  }
}

///@endcond
