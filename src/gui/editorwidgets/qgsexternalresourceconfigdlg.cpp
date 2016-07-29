/***************************************************************************
    qgsexternalresourceconfigdlg.cpp
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

#include "qgsexternalresourceconfigdlg.h"
#include "qgsexternalresourcewidget.h"
#include "qgsproject.h"

#include <QFileDialog>
#include <QSettings>

class QgsExternalResourceWidgetWrapper;

QgsExternalResourceConfigDlg::QgsExternalResourceConfigDlg( QgsVectorLayer* vl, int fieldIdx, QWidget* parent )
    : QgsEditorConfigWidget( vl, fieldIdx, parent )
{
  setupUi( this );

  // By default, uncheck some options
  mUseLink->setChecked( false );
  mFullUrl->setChecked( false );
  mDocumentViewerGroupBox->setChecked( false );

  QString defpath = QgsProject::instance()->fileName().isEmpty() ? QDir::homePath() : QgsProject::instance()->fileInfo().absolutePath();

  mRootPath->setPlaceholderText( QSettings().value( "/UI/lastExternalResourceWidgetDefaultPath", QDir::toNativeSeparators( QDir::cleanPath( defpath ) ) ).toString() );

  // Add connection to button for choosing default path
  connect( mRootPathButton, SIGNAL( clicked() ), this, SLOT( chooseDefaultPath() ) );

  // Activate Relative Default Path option only if Default Path is set
  connect( mRootPath, SIGNAL( textChanged( const QString & ) ), this, SLOT( enableRelativeDefault() ) );

  // Dynamic GroupBox for relative paths option
  connect( mRelativeGroupBox, SIGNAL( toggled( bool ) ), this, SLOT( enableRelative( bool ) ) );

  // set ids for StorageTypeButtons
  mStorageButtonGroup->setId( mStoreFilesButton, QgsFileWidget::GetFile );
  mStorageButtonGroup->setId( mStoreDirsButton, QgsFileWidget::GetDirectory );
  mStoreFilesButton->setChecked( true );

  // set ids for RelativeButtons
  mRelativeButtonGroup->setId( mRelativeProject, QgsFileWidget::RelativeProject );
  mRelativeButtonGroup->setId( mRelativeDefault, QgsFileWidget::RelativeDefaultPath );
  mRelativeProject->setChecked( true );

  mDocumentViewerContentComboBox->addItem( tr( "Image" ), QgsExternalResourceWidget::Image );
  mDocumentViewerContentComboBox->addItem( tr( "Web view" ), QgsExternalResourceWidget::Web );
}

void QgsExternalResourceConfigDlg::chooseDefaultPath()
{
  QString dir;
  if ( !mRootPath->text().isEmpty() )
  {
    dir = mRootPath->text();
  }
  else
  {
    dir = QSettings().value( "/UI/lastExternalResourceWidgetDefaultPath", QDir::toNativeSeparators( QDir::cleanPath( QgsProject::instance()->fileInfo().absolutePath() ) ) ).toString();
  }

  QString rootName = QFileDialog::getExistingDirectory( this, tr( "Select a directory" ), dir, QFileDialog::ShowDirsOnly );

  if ( rootName.isNull() )
    return;

  mRootPath->setText( rootName );
}

void QgsExternalResourceConfigDlg::enableRelativeDefault()
{
  // Activate (or not) the RelativeDefault button if default path
  if ( mRelativeGroupBox->isChecked() )
    mRelativeDefault->setEnabled( !mRootPath->text().isEmpty() );

  // If no default path, RelativeProj button enabled by default
  if ( mRootPath->text().isEmpty() )
    mRelativeProject->toggle();
}

void QgsExternalResourceConfigDlg::enableRelative( bool state )
{
  if ( state )
  {
    mRelativeProject->setEnabled( true );
    if ( mRootPath->text().isEmpty() )
      mRelativeDefault->setEnabled( false );
    else
      mRelativeDefault->setEnabled( true );
  }
  else
  {
    mRelativeProject->setEnabled( false );
    mRelativeDefault->setEnabled( false );
  }
}


QgsEditorWidgetConfig QgsExternalResourceConfigDlg::config()
{
  QgsEditorWidgetConfig cfg;

  cfg.insert( "FileWidget", mFileWidgetGroupBox->isChecked() );
  cfg.insert( "FileWidgetButton", mFileWidgetButtonGroupBox->isChecked() );
  cfg.insert( "FileWidgetFilter", mFileWidgetFilterLineEdit->text() );

  if ( mUseLink->isChecked() )
  {
    cfg.insert( "UseLink", mUseLink->isChecked() );
    if ( mFullUrl->isChecked() )
      cfg.insert( "FullUrl", mFullUrl->isChecked() );
  }

  if ( !mRootPath->text().isEmpty() )
  {
    cfg.insert( "DefaultRoot", mRootPath->text() );
  }

  // Save Storage Mode
  cfg.insert( "StorageMode", mStorageButtonGroup->checkedId() );

  // Save Relative Paths option
  if ( mRelativeGroupBox->isChecked() )
  {
    cfg.insert( "RelativeStorage", mRelativeButtonGroup->checkedId() );
  }
  else
  {
    cfg.insert( "RelativeStorage", ( int )QgsFileWidget::Absolute );
  }

  if ( mDocumentViewerGroupBox->isChecked() )
  {
    cfg.insert( "DocumentViewer", mDocumentViewerContentComboBox->itemData( mDocumentViewerContentComboBox->currentIndex() ).toInt() );
    cfg.insert( "DocumentViewerHeight", mDocumentViewerHeight->value() );
    cfg.insert( "DocumentViewerWidth", mDocumentViewerWidth->value() );
  }
  else
  {
    cfg.insert( "DocumentViewer", ( int )QgsExternalResourceWidget::NoContent );
  }

  return cfg;
}


void QgsExternalResourceConfigDlg::setConfig( const QgsEditorWidgetConfig& config )
{
  if ( config.contains( "FileWidget" ) )
  {
    mFileWidgetGroupBox->setChecked( config.value( "FileWidget" ).toBool() );
  }
  if ( config.contains( "FileWidget" ) )
  {
    mFileWidgetButtonGroupBox->setChecked( config.value( "FileWidgetButton" ).toBool() );
  }
  if ( config.contains( "FileWidgetFilter" ) )
  {
    mFileWidgetFilterLineEdit->setText( config.value( "FileWidgetFilter" ).toString() );
  }

  if ( config.contains( "UseLink" ) )
  {
    mUseLink->setChecked( config.value( "UseLink" ).toBool() );
    if ( config.contains( "FullUrl" ) )
      mFullUrl->setChecked( true );
  }

  if ( config.contains( "DefaultRoot" ) )
  {
    mRootPath->setText( config.value( "DefaultRoot" ).toString() );
  }

  // relative storage
  if ( config.contains( "RelativeStorage" ) )
  {
    int relative = config.value( "RelativeStorage" ).toInt();
    if (( QgsFileWidget::RelativeStorage )relative == QgsFileWidget::Absolute )
    {
      mRelativeGroupBox->setChecked( false );
    }
    else
    {
      mRelativeGroupBox->setChecked( true );
      mRelativeButtonGroup->button( relative )->setChecked( true );
    }
  }

  // set storage mode
  if ( config.contains( "StorageMode" ) )
  {
    int mode = config.value( "StorageMode" ).toInt();
    mStorageButtonGroup->button( mode )->setChecked( true );
  }

  // Document viewer
  if ( config.contains( "DocumentViewer" ) )
  {
    QgsExternalResourceWidget::DocumentViewerContent content = ( QgsExternalResourceWidget::DocumentViewerContent )config.value( "DocumentViewer" ).toInt();
    mDocumentViewerGroupBox->setChecked( content != QgsExternalResourceWidget::NoContent );
    int idx = mDocumentViewerContentComboBox->findData( content );
    if ( idx >= 0 )
    {
      mDocumentViewerContentComboBox->setCurrentIndex( idx );
    }
    if ( config.contains( "DocumentViewerHeight" ) )
    {
      mDocumentViewerHeight->setValue( config.value( "DocumentViewerHeight" ).toInt() );
    }
    if ( config.contains( "DocumentViewerWidth" ) )
    {
      mDocumentViewerWidth->setValue( config.value( "DocumentViewerWidth" ).toInt() );
    }
  }
}
