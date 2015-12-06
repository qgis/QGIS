/***************************************************************************
    qgsfilenamewidgetwrapper.cpp
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

#include "qgsfilenamewidgetwrapper.h"

#include "qgsfilterlineedit.h"
#include "qgsproject.h"

#include <QFileDialog>
#include <QSettings>
#include <QGridLayout>
#include <QUrl>


/*

DONE

* FileName should be reserved to String fields (and not Int/Double/Dates).
* Find the good regexp to match value.
* Links should be enabled even in non modification mode.
* In Labelmode, Button should not be increased in size.
* Keep the last used path in a QSetting.
* Use the last path used for new empty values.
* If data is not a filePath, just show the value !
* Test with files with a space in their names.
* Use QUrl or something to forge the URL.
* feature: store path relatively to the default PATH.
* feature: store path relatively to the project PATH.

*/
QgsFileNameWidgetWrapper::QgsFileNameWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent )
    : QgsEditorWidgetWrapper( vl, fieldIdx, editor, parent )
    , mLineEdit( nullptr )
    , mPushButton( nullptr )
    , mLabel( nullptr )
{
}

QVariant QgsFileNameWidgetWrapper::value() const
{
  if ( mLineEdit )
  {
    if ( mLineEdit->text().isEmpty() || mLineEdit->text() == QSettings().value( "qgis/nullValue", "NULL" ).toString() )
      return QVariant( field().type() );
    return mLineEdit->text();
  }

  return mValue;
}

bool QgsFileNameWidgetWrapper::valid() const
{
  return mLineEdit || mLabel;
}

QWidget* QgsFileNameWidgetWrapper::createWidget( QWidget* parent )
{
  QWidget* container = new QWidget( parent );
  container->setBackgroundRole( QPalette::Window );
  container->setAutoFillBackground( true );
  QPushButton* pbn = new QPushButton( tr( "..." ), container );
  pbn->setObjectName( "FileChooserButton" );
  QGridLayout* layout = new QGridLayout();
  layout->setMargin( 0 );

  // If we want to display a hyperlink, use a QLabel
  if ( config( "UseLink" ).toBool() )
  {
    QLabel* labelLink = new QLabel( container );
    labelLink->setObjectName( "UrlLabel" );
    // Make Qt opens the link with the OS defined viewer
    labelLink->setOpenExternalLinks( true );
    // Label should always be enabled to be able to open
    // the link on read only mode.
    labelLink->setEnabled( true );
    labelLink->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
    labelLink->setTextFormat( Qt::RichText );
    layout->addWidget( labelLink, 0, 0 );
  }
  // Instead, use the traditional QLineEdit
  else
  {
    QLineEdit* le = new QgsFilterLineEdit( container );
    layout->addWidget( le, 0, 0 );
  }
  layout->addWidget( pbn, 0, 1 );
  container->setLayout( layout );

  return container;
}

void QgsFileNameWidgetWrapper::initWidget( QWidget* editor )
{
  mValue = QString(); // initialize value with an empty QString
  mLineEdit = qobject_cast<QLineEdit*>( editor );
  if ( !mLineEdit )
  {
    mLineEdit = editor->findChild<QLineEdit*>();
  }

  mPushButton = editor->findChild<QPushButton*>( "FileChooserButton" );

  if ( mPushButton )
    connect( mPushButton, SIGNAL( clicked() ), this, SLOT( selectFileName() ) );

  mLabel = qobject_cast<QLabel*>( editor );
  if ( !mLabel )
  {
    mLabel = editor->findChild<QLabel*>( "UrlLabel" );
  }

  if ( mLineEdit )
  {
    QgsFilterLineEdit* fle = qobject_cast<QgsFilterLineEdit*>( editor );
    if ( fle )
    {
      fle->setNullValue( QSettings().value( "qgis/nullValue", "NULL" ).toString() );
    }

    connect( mLineEdit, SIGNAL( textChanged( QString ) ), this, SLOT( valueChanged( QString ) ) );
  }
}

void QgsFileNameWidgetWrapper::setValue( const QVariant& value )
{
  mValue = value;
  if ( mLineEdit )
  {
    if ( value.isNull() )
    {
      mLineEdit->setText( QSettings().value( "qgis/nullValue", "NULL" ).toString() );
    }
    else
    {
      mLineEdit->setText( value.toString() );
    }
  }

  if ( mLabel )
  {
    mLabel->setText( toUrl( value ) );
    valueChanged( value.toString() ); // emit signal that value has changed
  }

}

void QgsFileNameWidgetWrapper::setEnabled( bool enabled )
{
  if ( mLineEdit )
    mLineEdit->setReadOnly( !enabled );

  if ( mPushButton )
    mPushButton->setEnabled( enabled );
}

// Open a file/dir selector and affect the value to the widget
void QgsFileNameWidgetWrapper::selectFileName()
{
  QSettings settings;
  QString oldPath;

  // If we use fixed default path
  if ( config().contains( "DefaultRoot" ) )
    oldPath = QDir::cleanPath( config( "DefaultRoot").toString() );
  // if we use a relative path option, we need to obtain the full path
  else if ( !value().isNull() )
    oldPath = relativePath( value().toString(), false );
  else
  {
    // If there is no valid value, find a default path to use
    QUrl theUrl = QUrl::fromUserInput( oldPath );
    if ( !theUrl.isValid() )
      oldPath = settings.value( "/UI/lastFileNameWidgetDir", QDir::cleanPath( QgsProject::instance()->fileInfo().absolutePath() ) ).toString();
  }

  // Handle Storage
  QString fileName;
  if ( config().contains( "StorageMode" ) )
  {
    if ( config( "StorageMode" ).toString() == "Files" )
      fileName = QFileDialog::getOpenFileName( widget(), tr( "Select a file" ), QFileInfo( oldPath ).absoluteFilePath() );
    else if ( config( "StorageMode" ).toString() == "Dirs" )
      fileName = QFileDialog::getExistingDirectory( widget(), tr( "Select a directory" ), QFileInfo( oldPath ).absoluteFilePath(),  QFileDialog::ShowDirsOnly );
    else
      return;
  }

  if ( fileName.isNull() )
    return;


  fileName = QDir::toNativeSeparators( QDir::cleanPath( QFileInfo( fileName ).absoluteFilePath() ) );
  // Store the last used path:
  if ( config( "StorageMode" ).toString() == "Files" )
    settings.setValue( "/UI/lastFileNameWidgetDir", QFileInfo( fileName ).absolutePath() );
  else if ( config( "StorageMode" ).toString() == "Dirs" )
    settings.setValue( "/UI/lastFileNameWidgetDir", fileName );

  // Handle relative Path storage
  fileName = relativePath( fileName, true );

  // Keep the new value
  setValue( fileName );
}

// Returns a filePath with relative path options applied (or not) !
QString QgsFileNameWidgetWrapper::relativePath( QString filePath, bool removeRelative )
{
  if ( config().contains( "RelativeStorage" ) )
  {
    QString RelativePath;
    if ( config( "RelativeStorage" ).toString() == "Project" )
      RelativePath = QDir::toNativeSeparators( QDir::cleanPath( QgsProject::instance()->fileInfo().absolutePath() ) );
    else if ( config( "RelativeStorage" ).toString() == "Default" && config().contains( "DefaultRoot" ) )
      RelativePath = QDir::toNativeSeparators( QDir::cleanPath( config( "DefaultRoot" ).toString() ) );
    if ( !RelativePath.isEmpty() )
    {
      if ( removeRelative )
        return QDir::cleanPath( QDir( RelativePath ).relativeFilePath( filePath ) );
      else
        return QDir::cleanPath( QDir( RelativePath ).filePath( filePath ) );
    }
  }
  return filePath;
}

// Transforms a filepath into a URL
QString QgsFileNameWidgetWrapper::toUrl( const QVariant& value )
{
  QString rep;
  if ( value.isNull() )
    return QSettings().value( "qgis/nullValue", "NULL" ).toString();

  QString urlStr = relativePath( value.toString(), false );
  QUrl theUrl = QUrl::fromUserInput( urlStr );
  if ( !theUrl.isValid() or !theUrl.isLocalFile() )
  {
    QgsDebugMsg( QString( "URL: %1 is not valid or not a local file !" ).arg( value.toString() ) );
    return value.toString();
  }

  QString filePath = theUrl.toString();
  if ( config( "FullUrl" ).toBool() )
  {
    rep = QString( "<a href=\"%1\">%2</a>" ).arg( filePath, urlStr );
    return rep;
  }
  else
  {
    QString fileName = QFileInfo( urlStr ).fileName();
    rep = QString( "<a href=\"%1\">%2</a>" ).arg( filePath, fileName );
    return rep;
  }
}
