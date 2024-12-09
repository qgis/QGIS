/***************************************************************************
  qgsmaplayersavestyledialog.h
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
#include <QPushButton>

#include "qgsmaplayersavestyledialog.h"
#include "moc_qgsmaplayersavestyledialog.cpp"
#include "qgssettings.h"
#include "qgshelp.h"
#include "qgsgui.h"
#include "qgsmaplayerstylecategoriesmodel.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsvectorlayer.h"

QgsMapLayerSaveStyleDialog::QgsMapLayerSaveStyleDialog( QgsMapLayer *layer, QWidget *parent )
  : QDialog( parent )
  , mLayer( layer )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  QgsSettings settings;

  const QString myLastUsedDir = settings.value( QStringLiteral( "style/lastStyleDir" ), QDir::homePath() ).toString();

  // save style type combobox
  connect( mStyleTypeComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, [=]( int ) {
    const QgsLayerPropertiesDialog::StyleType type = currentStyleType();
    mFileLabel->setVisible( type != QgsLayerPropertiesDialog::DatasourceDatabase && type != QgsLayerPropertiesDialog::UserDatabase );
    mFileWidget->setVisible( type != QgsLayerPropertiesDialog::DatasourceDatabase && type != QgsLayerPropertiesDialog::UserDatabase );
    mSaveToDbWidget->setVisible( type == QgsLayerPropertiesDialog::DatasourceDatabase );
    mSaveToSldWidget->setVisible( type == QgsLayerPropertiesDialog::SLD && layer->type() == Qgis::LayerType::Vector && static_cast<QgsVectorLayer *>( layer )->geometryType() == Qgis::GeometryType::Polygon );
    mStyleCategoriesListView->setEnabled( type != QgsLayerPropertiesDialog::SLD );
    mFileWidget->setFilter( type == QgsLayerPropertiesDialog::QML ? tr( "QGIS Layer Style File (*.qml)" ) : tr( "SLD File (*.sld)" ) );
    updateSaveButtonState();
  } );

  // Save to DB setup
  connect( mDbStyleNameEdit, &QLineEdit::textChanged, this, &QgsMapLayerSaveStyleDialog::updateSaveButtonState );
  mDbStyleDescriptionEdit->setTabChangesFocus( true );
  setTabOrder( mDbStyleNameEdit, mDbStyleDescriptionEdit );
  setTabOrder( mDbStyleDescriptionEdit, mDbStyleUseAsDefault );
  mDbStyleUIFileWidget->setDefaultRoot( myLastUsedDir );
  mDbStyleUIFileWidget->setFilter( tr( "Qt Designer UI file (*.ui)" ) );
  connect( mDbStyleUIFileWidget, &QgsFileWidget::fileChanged, this, &QgsMapLayerSaveStyleDialog::readUiFileContent );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsMapLayerSaveStyleDialog::showHelp );

  // save to file setup
  connect( mFileWidget, &QgsFileWidget::fileChanged, this, &QgsMapLayerSaveStyleDialog::updateSaveButtonState );
  mFileWidget->setStorageMode( QgsFileWidget::SaveFile );
  mFileWidget->setDefaultRoot( myLastUsedDir );
  connect( mFileWidget, &QgsFileWidget::fileChanged, this, [=]( const QString &path ) {
    QgsSettings settings;
    const QFileInfo tmplFileInfo( path );
    settings.setValue( QStringLiteral( "style/lastStyleDir" ), tmplFileInfo.absolutePath() );
  } );

  // fill style categories
  mModel = new QgsMapLayerStyleCategoriesModel( mLayer->type(), this );
  const QgsMapLayer::StyleCategories lastStyleCategories = settings.flagValue( QStringLiteral( "style/lastStyleCategories" ), QgsMapLayer::AllStyleCategories );
  mModel->setCategories( lastStyleCategories );
  mStyleCategoriesListView->setModel( mModel );
  mStyleCategoriesListView->setWordWrap( true );
  mStyleCategoriesListView->setItemDelegate( new QgsCategoryDisplayLabelDelegate( this ) );

  // select and deselect all categories
  connect( mSelectAllButton, &QPushButton::clicked, this, &QgsMapLayerSaveStyleDialog::selectAll );
  connect( mDeselectAllButton, &QPushButton::clicked, this, &QgsMapLayerSaveStyleDialog::deselectAll );
  connect( mInvertSelectionButton, &QPushButton::clicked, this, &QgsMapLayerSaveStyleDialog::invertSelection );

  mStyleCategoriesListView->adjustSize();

  setupMultipleStyles();
}

void QgsMapLayerSaveStyleDialog::invertSelection()
{
  for ( int i = 0; i < mModel->rowCount( QModelIndex() ); i++ )
  {
    QModelIndex index = mModel->index( i, 0 );
    Qt::CheckState currentState = Qt::CheckState( mModel->data( index, Qt::CheckStateRole ).toInt() );
    Qt::CheckState newState = ( currentState == Qt::Checked ) ? Qt::Unchecked : Qt::Checked;
    mModel->setData( index, newState, Qt::CheckStateRole );
  }
}

void QgsMapLayerSaveStyleDialog::selectAll()
{
  for ( int i = 0; i < mModel->rowCount( QModelIndex() ); i++ )
  {
    QModelIndex index = mModel->index( i, 0 );
    mModel->setData( index, Qt::Checked, Qt::CheckStateRole );
  }
}

void QgsMapLayerSaveStyleDialog::deselectAll()
{
  for ( int i = 0; i < mModel->rowCount( QModelIndex() ); i++ )
  {
    QModelIndex index = mModel->index( i, 0 );
    mModel->setData( index, Qt::Unchecked, Qt::CheckStateRole );
  }
}

void QgsMapLayerSaveStyleDialog::populateStyleComboBox()
{
  mStyleTypeComboBox->clear();
  mStyleTypeComboBox->addItem( tr( "As QGIS QML style file" ), QgsLayerPropertiesDialog::QML );
  mStyleTypeComboBox->addItem( tr( "As SLD style file" ), QgsLayerPropertiesDialog::SLD );

  if ( mLayer->dataProvider()->styleStorageCapabilities().testFlag( Qgis::ProviderStyleStorageCapability::SaveToDatabase ) )
    mStyleTypeComboBox->addItem( tr( "In datasource database" ), QgsLayerPropertiesDialog::DatasourceDatabase );

  if ( mSaveOnlyCurrentStyle )
    mStyleTypeComboBox->addItem( tr( "As default in local user database" ), QgsLayerPropertiesDialog::UserDatabase );
}

void QgsMapLayerSaveStyleDialog::accept()
{
  QgsSettings().setFlagValue( QStringLiteral( "style/lastStyleCategories" ), styleCategories() );
  QDialog::accept();
}

void QgsMapLayerSaveStyleDialog::updateSaveButtonState()
{
  const QgsLayerPropertiesDialog::StyleType type = currentStyleType();
  bool enabled { false };
  switch ( type )
  {
    case QgsLayerPropertiesDialog::DatasourceDatabase:
      if ( saveOnlyCurrentStyle() )
      {
        enabled = !mDbStyleNameEdit->text().isEmpty();
      }
      else
      {
        enabled = true;
      }
      break;
    case QgsLayerPropertiesDialog::QML:
    case QgsLayerPropertiesDialog::SLD:
      enabled = !mFileWidget->filePath().isEmpty();
      break;
    case QgsLayerPropertiesDialog::UserDatabase:
      enabled = true;
      break;
  }
  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( enabled );
}

QgsMapLayerSaveStyleDialog::SaveToDbSettings QgsMapLayerSaveStyleDialog::saveToDbSettings() const
{
  SaveToDbSettings settings;
  settings.name = mDbStyleNameEdit->text();
  settings.description = mDbStyleDescriptionEdit->toPlainText();
  settings.isDefault = mDbStyleUseAsDefault->isChecked();
  settings.uiFileContent = mUiFileContent;
  return settings;
}

QString QgsMapLayerSaveStyleDialog::outputFilePath() const
{
  return mFileWidget->filePath();
}

QgsMapLayer::StyleCategories QgsMapLayerSaveStyleDialog::styleCategories() const
{
  return mModel->categories();
}

QgsLayerPropertiesDialog::StyleType QgsMapLayerSaveStyleDialog::currentStyleType() const
{
  return mStyleTypeComboBox->currentData().value<QgsLayerPropertiesDialog::StyleType>();
}

void QgsMapLayerSaveStyleDialog::readUiFileContent( const QString &filePath )
{
  QgsSettings myQSettings; // where we keep last used filter in persistent state
  mUiFileContent = QString();

  if ( filePath.isNull() )
  {
    return;
  }

  const QFileInfo myFI( filePath );
  QFile uiFile( myFI.filePath() );

  const QString myPath = myFI.path();
  myQSettings.setValue( QStringLiteral( "style/lastStyleDir" ), myPath );

  if ( uiFile.open( QIODevice::ReadOnly ) )
  {
    const QString content( uiFile.readAll() );
    QDomDocument doc;

    if ( !doc.setContent( content ) || doc.documentElement().tagName().compare( QLatin1String( "ui" ) ) )
    {
      QMessageBox::warning( this, tr( "Attach UI File" ), tr( "The selected file does not appear to be a valid Qt Designer UI file." ) );
      return;
    }
    mUiFileContent = content;
  }
}

void QgsMapLayerSaveStyleDialog::setupMultipleStyles()
{
  // Show/hide part of the UI according to multiple style support
  if ( !mSaveOnlyCurrentStyle )
  {
    const QgsMapLayerStyleManager *styleManager { mLayer->styleManager() };
    const QStringList constStyles = styleManager->styles();
    for ( const QString &name : constStyles )
    {
      QListWidgetItem *item = new QListWidgetItem( name, mStylesWidget );
      item->setCheckState( Qt::CheckState::Checked );
      // Highlight the current style
      if ( name == styleManager->currentStyle() )
      {
        item->setToolTip( tr( "Current style" ) );
        QFont font { item->font() };
        font.setItalic( true );
        item->setFont( font );
      }
      mStylesWidget->addItem( item );
    }
    mDbStyleNameEdit->setToolTip( tr( "Leave blank to use style names or set the base name (an incremental number will be automatically appended)" ) );
  }
  else
  {
    mDbStyleNameEdit->setToolTip( QString() );
  }

  mStylesWidget->setVisible( !mSaveOnlyCurrentStyle );
  mStylesWidgetLabel->setVisible( !mSaveOnlyCurrentStyle );

  mDbStyleDescriptionEdit->setVisible( mSaveOnlyCurrentStyle );
  descriptionLabel->setVisible( mSaveOnlyCurrentStyle );
  mDbStyleUseAsDefault->setVisible( mSaveOnlyCurrentStyle );

  populateStyleComboBox();
}

bool QgsMapLayerSaveStyleDialog::saveOnlyCurrentStyle() const
{
  return mSaveOnlyCurrentStyle;
}

void QgsMapLayerSaveStyleDialog::setSaveOnlyCurrentStyle( bool saveOnlyCurrentStyle )
{
  if ( mSaveOnlyCurrentStyle != saveOnlyCurrentStyle )
  {
    mSaveOnlyCurrentStyle = saveOnlyCurrentStyle;
    setupMultipleStyles();
  }
}

const QListWidget *QgsMapLayerSaveStyleDialog::stylesWidget()
{
  return mStylesWidget;
}

Qgis::SldExportOptions QgsMapLayerSaveStyleDialog::sldExportOptions() const
{
  Qgis::SldExportOptions options;

  if ( mStyleTypeComboBox->currentData() == QgsLayerPropertiesDialog::SLD && mSldExportPng->isChecked() )
  {
    options.setFlag( Qgis::SldExportOption::Png );
  }
  return options;
}

void QgsMapLayerSaveStyleDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "introduction/general_tools.html#save-and-share-layer-properties" ) );
}
