/***************************************************************************
  qgsvectorlayersavestyledialog.h
  --------------------------------------
  Date                 : September 2018
  Copyright            : (C) 2018 by Denis Rouzaud
  Email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QListWidgetItem>
#include <QMessageBox>

#include "qgsvectorlayersavestyledialog.h"
#include "qgsvectorlayer.h"
#include "qgssettings.h"
#include "qgshelp.h"
#include "qgsmaplayerstylecategoriesmodel.h"

QgsVectorLayerSaveStyleDialog::QgsVectorLayerSaveStyleDialog( QgsVectorLayer *layer, QWidget *parent )
  : QDialog( parent )
  , mLayer( layer )
{
  setupUi( this );
  QgsSettings settings;

  QString providerName = mLayer->providerType();
  if ( providerName == QLatin1String( "ogr" ) )
  {
    providerName = mLayer->dataProvider()->storageType();
    if ( providerName == QLatin1String( "GPKG" ) )
      providerName = QStringLiteral( "GeoPackage" );
  }

  QString myLastUsedDir = settings.value( QStringLiteral( "style/lastStyleDir" ), QDir::homePath() ).toString();

  // save style type combobox
  connect( mStyleTypeComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ), this, [ = ]( int )
  {
    QgsVectorLayerProperties::StyleType type = currentStyleType();
    mSaveToFileWidget->setVisible( type != QgsVectorLayerProperties::DB );
    mSaveToDbWidget->setVisible( type == QgsVectorLayerProperties::DB );
    mStyleCategoriesListView->setEnabled( type == QgsVectorLayerProperties::QML );
    mFileWidget->setFilter( type == QgsVectorLayerProperties::QML ? tr( "QGIS Layer Style File (*.qml)" ) : tr( "SLD File (*.sld)" ) );
    updateSaveButtonState();
  } );
  mStyleTypeComboBox->addItem( tr( "As QGIS QML style file" ), QgsVectorLayerProperties::QML );
  mStyleTypeComboBox->addItem( tr( "As SLD style file" ), QgsVectorLayerProperties::SLD );
  if ( mLayer->dataProvider()->isSaveAndLoadStyleToDatabaseSupported() )
    mStyleTypeComboBox->addItem( tr( "In database (%1)" ).arg( providerName ), QgsVectorLayerProperties::DB );

  // Save to DB setup
  connect( mDbStyleNameEdit, &QLineEdit::textChanged, this, &QgsVectorLayerSaveStyleDialog::updateSaveButtonState );
  mDbStyleDescriptionEdit->setTabChangesFocus( true );
  setTabOrder( mDbStyleNameEdit, mDbStyleDescriptionEdit );
  setTabOrder( mDbStyleDescriptionEdit, mDbStyleUseAsDefault );
  mDbStyleUIFileWidget->setDefaultRoot( myLastUsedDir );
  mDbStyleUIFileWidget->setFilter( tr( "Qt Designer UI file (*.ui)" ) );
  connect( mDbStyleUIFileWidget, &QgsFileWidget::fileChanged, this, &QgsVectorLayerSaveStyleDialog::readUiFileContent );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsVectorLayerSaveStyleDialog::showHelp );

  // save to file setup
  connect( mFileWidget, &QgsFileWidget::fileChanged, this, &QgsVectorLayerSaveStyleDialog::updateSaveButtonState );
  mFileWidget->setStorageMode( QgsFileWidget::SaveFile );
  mFileWidget->setDefaultRoot( myLastUsedDir );

  // fill style categories
  mModel = new QgsMapLayerStyleCategoriesModel( this );
  QgsMapLayer::StyleCategories lastStyleCategories = settings.flagValue( QStringLiteral( "style/lastStyleCategories" ), QgsMapLayer::AllStyleCategories );
  mModel->setCategories( lastStyleCategories );
  mStyleCategoriesListView->setModel( mModel );

  restoreGeometry( settings.value( QStringLiteral( "Windows/vectorLayerSaveStyle/geometry" ) ).toByteArray() );
  mStyleCategoriesListView->adjustSize();
}

void QgsVectorLayerSaveStyleDialog::accept()
{
  QgsSettings().setFlagValue( QStringLiteral( "style/lastStyleCategories" ), styleCategories() );
  QDialog::accept();
}

void QgsVectorLayerSaveStyleDialog::updateSaveButtonState()
{
  QgsVectorLayerProperties::StyleType type = currentStyleType();
  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( ( type == QgsVectorLayerProperties::DB && !mDbStyleNameEdit->text().isEmpty() ) ||
      ( type != QgsVectorLayerProperties::DB && !mFileWidget->filePath().isEmpty() ) );
}

QgsVectorLayerSaveStyleDialog::~QgsVectorLayerSaveStyleDialog()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/vectorLayerSaveStyle/geometry" ), saveGeometry() );
}

QgsVectorLayerSaveStyleDialog::SaveToDbSettings QgsVectorLayerSaveStyleDialog::saveToDbSettings() const
{
  SaveToDbSettings settings;
  settings.name = mDbStyleNameEdit->text();
  settings.description = mDbStyleDescriptionEdit->toPlainText();
  settings.isDefault = mDbStyleUseAsDefault->isChecked();
  settings.uiFileContent = mUiFileContent;
  return settings;
}

QString QgsVectorLayerSaveStyleDialog::outputFilePath() const
{
  return mFileWidget->filePath();
}

QgsMapLayer::StyleCategories QgsVectorLayerSaveStyleDialog::styleCategories() const
{
  return mModel->categories();
}

QgsVectorLayerProperties::StyleType QgsVectorLayerSaveStyleDialog::currentStyleType() const
{
  return mStyleTypeComboBox->currentData().value<QgsVectorLayerProperties::StyleType>();
}

void QgsVectorLayerSaveStyleDialog::readUiFileContent( const QString &filePath )
{
  QgsSettings myQSettings;  // where we keep last used filter in persistent state
  mUiFileContent = QString();

  if ( filePath.isNull() )
  {
    return;
  }

  QFileInfo myFI( filePath );
  QFile uiFile( myFI.filePath() );

  QString myPath = myFI.path();
  myQSettings.setValue( QStringLiteral( "style/lastStyleDir" ), myPath );

  if ( uiFile.open( QIODevice::ReadOnly ) )
  {
    QString content( uiFile.readAll() );
    QDomDocument doc;

    if ( !doc.setContent( content ) || doc.documentElement().tagName().compare( QLatin1String( "ui" ) ) )
    {
      QMessageBox::warning( this, tr( "Attach UI File" ),
                            tr( "The selected file does not appear to be a valid Qt Designer UI file." ) );
      return;
    }
    mUiFileContent = content;
  }
}

void QgsVectorLayerSaveStyleDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "introduction/general_tools.html#save-and-share-layer-properties" ) );
}
