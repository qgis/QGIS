/***************************************************************************
    qgsfilenameconfigdlg.cpp
     --------------------------------------
    Date                 : 2015-11-26
    Copyright            : (C) 2015 Médéric Ribreux
    Email                : mederic.ribreux at medspx dot fr
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfilenameconfigdlg.h"
#include "qgsproject.h"

#include <QFileDialog>
#include <QSettings>

class QgsFileNameWidgetWrapper;

QgsFileNameConfigDlg::QgsFileNameConfigDlg( QgsVectorLayer* vl, int fieldIdx, QWidget* parent )
    : QgsEditorConfigWidget( vl, fieldIdx, parent )
{
  setupUi( this );

  // By default, uncheck some options
  mUseLink->setChecked( false );
  mFullUrl->setChecked( false );
  mRootPath->setPlaceholderText( QSettings().value( "/UI/lastFileNameWidgetDir", QDir::toNativeSeparators( QDir::cleanPath( QgsProject::instance()->fileInfo().absolutePath() ) ) ).toString() );

  // Add connection to button for choosing default path
  connect( mRootPathButton, SIGNAL( clicked() ), this, SLOT( chooseDefaultPath() ) );

  // Activate Relative Default Path option only if Default Path is set
  connect( mRootPath, SIGNAL( textChanged( const QString & ) ), this, SLOT( enableRelativeDefault() ) );

  // Dynamic GroupBox for relative paths option
  connect( mRelativeGroupBox, SIGNAL( toggled( bool ) ), this, SLOT( enableRelative( bool ) ) );

  // set ids for StorageTypeButtons
  mStorageButtonGroup->setId( mStoreFilesButton, 0 );
  mStorageButtonGroup->setId( mStoreDirsButton, 1 );

  // First button is toggled by default
  mStorageButtonGroup->button( 0 )->toggle();

  // set ids for RelativeButtons
  mRelativeButtonGroup->setId( mRelativeProj, 0 );
  mRelativeButtonGroup->setId( mRelativeDefault, 1 );

  mRelativeButtonGroup->button( 0 )->toggle();
}

// Choose a base directory for rootPath
void QgsFileNameConfigDlg::chooseDefaultPath()
{
  QString dir;
  if ( !mRootPath->text().isEmpty() )
    dir = mRootPath->text();
  else
    dir = QSettings().value( "/UI/lastFileNameWidgetDir", QDir::toNativeSeparators( QDir::cleanPath( QgsProject::instance()->fileInfo().absolutePath() ) ) ).toString();

  QString rootName = QFileDialog::getExistingDirectory( this, tr( "Select a directory" ), dir, QFileDialog::ShowDirsOnly );

  if ( rootName.isNull() )
    return;

  mRootPath->setText( rootName );
}

// Modify RelativeDefault according to mRootPath content
void QgsFileNameConfigDlg::enableRelativeDefault()
{
  // Activate (or not) the RelativeDefault button if default path
  if ( mRelativeGroupBox->isChecked() )
    mRelativeDefault->setEnabled( !mRootPath->text().isEmpty() );

  // If no default path, RelativeProj button enabled by default
  if ( mRootPath->text().isEmpty() )
    mRelativeProj->toggle();
}

// Dynamic activation of RelativeGroupBox
void QgsFileNameConfigDlg::enableRelative( bool state )
{
  if ( state )
  {
    mRelativeProj->setEnabled( true );
    if ( mRootPath->text().isEmpty() )
      mRelativeDefault->setEnabled( false );
    else
      mRelativeDefault->setEnabled( true );
  }
  else
  {
    mRelativeProj->setEnabled( false );
    mRelativeDefault->setEnabled( false );
  }
}

// All non mandatory options are not stored to reduce
// the file size of the project file !
QgsEditorWidgetConfig QgsFileNameConfigDlg::config()
{
  QgsEditorWidgetConfig cfg;

  if ( mUseLink->isChecked() )
  {
    cfg.insert( "UseLink", mUseLink->isChecked() );
    if ( mFullUrl->isChecked() )
      cfg.insert( "FullUrl", mFullUrl->isChecked() );
  }

  if ( !mRootPath->text().isEmpty() )
    cfg.insert( "DefaultRoot", mRootPath->text() );

  // Save Storage Mode
  int but = mStorageButtonGroup->checkedId();
  if ( but != -1 )
  {
    if ( but == 0 )
      cfg.insert( "StorageMode", "Files" );
    else if ( but == 1 )
      cfg.insert( "StorageMode", "Dirs" );
  }

  // Save Relative Paths option
  if ( mRelativeGroupBox->isChecked() )
  {
    but = mRelativeButtonGroup->checkedId();
    if ( but != -1 )
    {
      if ( but == 0 )
        cfg.insert( "RelativeStorage", "Project" );
      else if ( but == 1 )
        cfg.insert( "RelativeStorage", "Default" );
    }
  }

  return cfg;
}

// Set the configuration on the widget
void QgsFileNameConfigDlg::setConfig( const QgsEditorWidgetConfig& config )
{
  if ( config.contains( "UseLink" ) )
  {
    mUseLink->setChecked( true );
    if ( config.contains( "FullUrl" ) )
      mFullUrl->setChecked( true );
  }

  if ( config.contains( "DefaultRoot" ) )
    mRootPath->setText( config.value( "DefaultRoot" ).toString() );

  if ( config.contains( "RelativeStorage" ) )
  {
    mRelativeGroupBox->setChecked( true );
    if ( config.value( "RelativeStorage" ) == "Default" )
      mRelativeDefault->toggle();
    else if ( config.value( "RelativeStorage" ) == "Project" )
      mRelativeProj->toggle();
  }

  // set storage mode
  if ( config.contains( "StorageMode" ) )
  {
    if ( config.value( "StorageMode" ) == "Files" )
      mStorageButtonGroup->button( 0 )->toggle();
    else if ( config.value( "StorageMode" ) == "Dirs" )
      mStorageButtonGroup->button( 1 )->toggle();
  }
}
