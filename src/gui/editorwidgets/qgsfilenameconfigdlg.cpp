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

  // Add connection to button for choosing default path
  connect( mRootPathButton, SIGNAL( clicked() ), this, SLOT( chooseDefaultPath() ) );

  // Activate Relative Default Path option only if Default Path is set
  connect( mRootPath, SIGNAL( textChanged( const QString &) ), this, SLOT( enableRelativeDefault() ) );

  // Disable Relative Project path if Relative Default Path is checked
  connect( mRelativeDefault, SIGNAL( stateChanged( int ) ), this, SLOT( enableRelativeProj( int ) ) );

  // Disable Relative Default Path if Relative Project path is checked
  connect( mRelativeProj, SIGNAL( stateChanged( int ) ), this, SLOT( enableRelativeDefault() ) );

  // set ids for StorageTypeButtons
  mStorageButtonGroup->setId( mStoreFilesButton, 0 );
  mStorageButtonGroup->setId( mStoreDirsButton, 1 );

  // First button is toggled by default
  mStorageButtonGroup->button( 0 )->toggle();
}

// Choose a base directory for rootPath
void QgsFileNameConfigDlg::chooseDefaultPath()
{
  QString dir = QSettings().value( "/UI/lastFileNameWidgetDir", QDir::homePath() ).toString();
  QString rootName = QFileDialog::getExistingDirectory( this, tr( "Select a directory" ), dir, QFileDialog::ShowDirsOnly );

  if ( rootName.isNull() )
    return;

  mRootPath->setText( rootName );
}

// Dynamic options
void QgsFileNameConfigDlg::enableRelativeDefault()
{
  if ( !mRelativeProj->isChecked() )
  {
    if ( mRootPath->text().isEmpty() )
    {
      mRelativeDefault->setCheckState( Qt::Unchecked );
      mRelativeProj->setEnabled( true );
    }
    mRelativeDefault->setEnabled( !mRootPath->text().isEmpty() );
  }
  else
  {
    mRelativeDefault->setEnabled( false );
  }
}

// Dynamic activation
void QgsFileNameConfigDlg::enableRelativeProj( int state )
{
  if ( state == Qt::Checked )
    mRelativeProj->setEnabled( false );
  else if ( state == Qt::Unchecked )
    mRelativeProj->setEnabled( true );
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
  if ( mRelativeDefault->isChecked() )
    cfg.insert( "RelativeStorage", "Default" );
  if ( mRelativeProj->isChecked() )
    cfg.insert( "RelativeStorage", "Project" );

  // Save Storage Mode
  int but = mStorageButtonGroup->checkedId();
  if ( but != -1 )
  {
    if ( but == 0 )
      cfg.insert( "StorageMode", "Files" );
    else if ( but == 1 )
      cfg.insert( "StorageMode", "Dirs" );
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
    if ( config.value( "RelativeStorage") == "Default" )
      mRelativeDefault->setCheckState( Qt::Checked );
    else if ( config.value( "RelativeStorage") == "Project" )
      mRelativeProj->setCheckState( Qt::Checked );
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
