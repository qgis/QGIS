/***************************************************************************
    qgsmaplayerloadstyledialog.cpp
    ---------------------
    begin                : April 2013
    copyright            : (C) 2013 by Emilio Loi
    email                : loi at faunalia dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaplayerloadstyledialog.h"

#include "qgsapplication.h"
#include "qgsgui.h"
#include "qgshelp.h"
#include "qgslayerpropertiesdialog.h"
#include "qgslogger.h"
#include "qgsmaplayerstylecategoriesmodel.h"
#include "qgssettings.h"

#include <QMessageBox>
#include <QVector>

#include "moc_qgsmaplayerloadstyledialog.cpp"

QgsMapLayerLoadStyleDialog::QgsMapLayerLoadStyleDialog( QgsMapLayer *layer, QWidget *parent )
  : QDialog( parent )
  , mLayer( layer )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );
  setWindowTitle( tr( "Database Styles Manager" ) );

  mDeleteButton = mButtonBox->button( QDialogButtonBox::StandardButton::Close );
  mDeleteButton->setText( tr( "Delete Style" ) );
  mDeleteButton->setIcon( QgsApplication::getThemeIcon( u"/mActionDeleteSelected.svg"_s ) );
  mLoadButton = mButtonBox->button( QDialogButtonBox::StandardButton::Open );
  mLoadButton->setText( tr( "Load Style" ) );
  mCancelButton = mButtonBox->button( QDialogButtonBox::StandardButton::Cancel );

  QgsSettings settings;

  const QString myLastUsedDir = settings.value( u"style/lastStyleDir"_s, QDir::homePath() ).toString();

  // load style type combobox
  connect( mStyleTypeComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, [this]( int ) {
    const QgsLayerPropertiesDialog::StyleType type = currentStyleType();
    mFileLabel->setVisible( type != QgsLayerPropertiesDialog::StyleType::DatasourceDatabase && type != QgsLayerPropertiesDialog::StyleType::UserDatabase );
    mFileWidget->setVisible( type != QgsLayerPropertiesDialog::StyleType::DatasourceDatabase && type != QgsLayerPropertiesDialog::StyleType::UserDatabase );
    mFromDbWidget->setVisible( type == QgsLayerPropertiesDialog::StyleType::DatasourceDatabase );
    mDeleteButton->setVisible( type == QgsLayerPropertiesDialog::StyleType::DatasourceDatabase && mLayer->dataProvider()->styleStorageCapabilities().testFlag( Qgis::ProviderStyleStorageCapability::DeleteFromDatabase ) );

    mStyleCategoriesListView->setEnabled( currentStyleType() != QgsLayerPropertiesDialog::StyleType::SLD );
    updateLoadButtonState();
  } );
  mStyleTypeComboBox->addItem( tr( "From file" ), QgsLayerPropertiesDialog::QML ); // QML is used as entry, but works for SLD too, see currentStyleType()
  mStyleTypeComboBox->addItem( tr( "Default from local database" ), QgsLayerPropertiesDialog::UserDatabase );

  if ( mLayer->dataProvider()->styleStorageCapabilities().testFlag( Qgis::ProviderStyleStorageCapability::LoadFromDatabase ) )
  {
    mStyleTypeComboBox->addItem( tr( "From datasource database" ), QgsLayerPropertiesDialog::StyleType::DatasourceDatabase );
    if ( settings.value( u"style/lastLoadStyleTypeSelection"_s ) == QgsLayerPropertiesDialog::StyleType::DatasourceDatabase )
    {
      mStyleTypeComboBox->setCurrentIndex( mStyleTypeComboBox->findData( QgsLayerPropertiesDialog::StyleType::DatasourceDatabase ) );
    }
  }

  // fill style categories
  mModel = new QgsMapLayerStyleCategoriesModel( mLayer->type(), this );
  const QgsMapLayer::StyleCategories lastStyleCategories = settings.flagValue( u"style/lastStyleCategories"_s, QgsMapLayer::AllStyleCategories );
  mModel->setCategories( lastStyleCategories );
  mStyleCategoriesListView->setModel( mModel );
  mStyleCategoriesListView->setWordWrap( true );
  mStyleCategoriesListView->setItemDelegate( new QgsCategoryDisplayLabelDelegate( this ) );

  // load from file setup
  switch ( mLayer->type() )
  {
    case Qgis::LayerType::Vector:
      mFileWidget->setFilter( tr( "QGIS Layer Style File, SLD File" ) + u" (*.qml *.sld)"_s );
      break;

    case Qgis::LayerType::VectorTile:
      mFileWidget->setFilter( tr( "All Styles" ) + u" (*.qml *.json);;"_s + tr( "QGIS Layer Style File" ) + u" (*.qml);;"_s + tr( "MapBox GL Style JSON File" ) + u" (*.json)"_s );
      break;

    case Qgis::LayerType::Raster:
    case Qgis::LayerType::Mesh:
    case Qgis::LayerType::Annotation:
    case Qgis::LayerType::Plugin:
    case Qgis::LayerType::PointCloud:
    case Qgis::LayerType::Group:
    case Qgis::LayerType::TiledScene:
      break;
  }

  mFileWidget->setStorageMode( QgsFileWidget::GetFile );
  mFileWidget->setDefaultRoot( myLastUsedDir );
  connect( mFileWidget, &QgsFileWidget::fileChanged, this, [this]( const QString &path ) {
    mStyleCategoriesListView->setEnabled( currentStyleType() != QgsLayerPropertiesDialog::SLD );
    QgsSettings settings;
    const QFileInfo tmplFileInfo( path );
    settings.setValue( u"style/lastStyleDir"_s, tmplFileInfo.absolutePath() );

    updateLoadButtonState();
  } );

  // load from DB
  mLoadButton->setDisabled( true );
  mDeleteButton->setDisabled( true );
  mRelatedTable->setEditTriggers( QTableWidget::NoEditTriggers );
  mRelatedTable->horizontalHeader()->setStretchLastSection( true );
  mRelatedTable->setSelectionBehavior( QTableWidget::SelectRows );
  mRelatedTable->verticalHeader()->setVisible( false );
  mOthersTable->setEditTriggers( QTableWidget::NoEditTriggers );
  mOthersTable->horizontalHeader()->setStretchLastSection( true );
  mOthersTable->setSelectionBehavior( QTableWidget::SelectRows );
  mOthersTable->verticalHeader()->setVisible( false );
  connect( mRelatedTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsMapLayerLoadStyleDialog::onRelatedTableSelectionChanged );
  connect( mOthersTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsMapLayerLoadStyleDialog::onOthersTableSelectionChanged );
  connect( mRelatedTable, &QTableWidget::doubleClicked, this, &QDialog::accept );
  connect( mOthersTable, &QTableWidget::doubleClicked, this, &QDialog::accept );
  connect( mCancelButton, &QPushButton::clicked, this, &QDialog::reject );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, &QgsMapLayerLoadStyleDialog::showHelp );
  connect( mLoadButton, &QPushButton::clicked, this, &QDialog::accept );
  connect( mDeleteButton, &QPushButton::clicked, this, &QgsMapLayerLoadStyleDialog::deleteStyleFromDB );
  connect( this, &QgsMapLayerLoadStyleDialog::rejected, [this] {
    QgsSettings().setValue( u"style/lastLoadStyleTypeSelection"_s, currentStyleType() );
  } );

  setTabOrder( mRelatedTable, mOthersTable );

  mStyleCategoriesListView->adjustSize();

  // select and deselect all categories
  connect( mSelectAllButton, &QPushButton::clicked, this, &QgsMapLayerLoadStyleDialog::selectAll );
  connect( mDeselectAllButton, &QPushButton::clicked, this, &QgsMapLayerLoadStyleDialog::deselectAll );
  connect( mInvertSelectionButton, &QPushButton::clicked, this, &QgsMapLayerLoadStyleDialog::invertSelection );
}

void QgsMapLayerLoadStyleDialog::invertSelection()
{
  for ( int i = 0; i < mModel->rowCount( QModelIndex() ); i++ )
  {
    QModelIndex index = mModel->index( i, 0 );
    Qt::CheckState currentState = Qt::CheckState( mModel->data( index, Qt::CheckStateRole ).toInt() );
    Qt::CheckState newState = ( currentState == Qt::Checked ) ? Qt::Unchecked : Qt::Checked;
    mModel->setData( index, newState, Qt::CheckStateRole );
  }
}

void QgsMapLayerLoadStyleDialog::selectAll()
{
  for ( int i = 0; i < mModel->rowCount( QModelIndex() ); i++ )
  {
    QModelIndex index = mModel->index( i, 0 );
    mModel->setData( index, Qt::Checked, Qt::CheckStateRole );
  }
}

void QgsMapLayerLoadStyleDialog::deselectAll()
{
  for ( int i = 0; i < mModel->rowCount( QModelIndex() ); i++ )
  {
    QModelIndex index = mModel->index( i, 0 );
    mModel->setData( index, Qt::Unchecked, Qt::CheckStateRole );
  }
}

QgsMapLayer::StyleCategories QgsMapLayerLoadStyleDialog::styleCategories() const
{
  return mModel->categories();
}

QgsLayerPropertiesDialog::StyleType QgsMapLayerLoadStyleDialog::currentStyleType() const
{
  QgsLayerPropertiesDialog::StyleType type = mStyleTypeComboBox->currentData().value<QgsLayerPropertiesDialog::StyleType>();
  if ( type == QgsLayerPropertiesDialog::QML )
  {
    const QFileInfo fi( mFileWidget->filePath() );
    if ( fi.exists() && fi.suffix().compare( u"sld"_s, Qt::CaseInsensitive ) == 0 )
      type = QgsLayerPropertiesDialog::SLD;
  }
  return type;
}

QString QgsMapLayerLoadStyleDialog::fileExtension() const
{
  return QFileInfo( mFileWidget->filePath() ).suffix();
}

QString QgsMapLayerLoadStyleDialog::filePath() const
{
  return mFileWidget->filePath();
}

void QgsMapLayerLoadStyleDialog::initializeLists( const QStringList &ids, const QStringList &names, const QStringList &descriptions, int sectionLimit )
{
  // -1 means no ids
  mSectionLimit = sectionLimit;
  const int relatedTableNOfCols = sectionLimit > 0 ? 2 : 1;
  const int othersTableNOfCols = ( sectionLimit >= 0 && ids.count() - sectionLimit > 0 ) ? 2 : 1;
  const QString twoColsHeader( u"Name;Description"_s );
  const QString oneColsHeader( u"No styles found in the database"_s );
  const QString relatedTableHeader = relatedTableNOfCols == 1 ? oneColsHeader : twoColsHeader;
  const QString othersTableHeader = othersTableNOfCols == 1 ? oneColsHeader : twoColsHeader;

  mRelatedTable->setColumnCount( relatedTableNOfCols );
  mOthersTable->setColumnCount( othersTableNOfCols );
  mRelatedTable->setHorizontalHeaderLabels( relatedTableHeader.split( ';' ) );
  mOthersTable->setHorizontalHeaderLabels( othersTableHeader.split( ';' ) );
  mRelatedTable->setRowCount( sectionLimit );
  mOthersTable->setRowCount( sectionLimit >= 0 ? ( ids.count() - sectionLimit ) : 0 );
  mRelatedTable->setDisabled( relatedTableNOfCols == 1 );
  mOthersTable->setDisabled( othersTableNOfCols == 1 );

  if ( sectionLimit >= 0 )
  {
    for ( int i = 0; i < sectionLimit; i++ )
    {
      QTableWidgetItem *item = new QTableWidgetItem( names.value( i, QString() ) );
      item->setData( Qt::UserRole, ids[i] );
      mRelatedTable->setItem( i, 0, item );
      mRelatedTable->setItem( i, 1, new QTableWidgetItem( descriptions.value( i, QString() ) ) );
    }
    for ( int i = sectionLimit; i < ids.count(); i++ )
    {
      const int j = i - sectionLimit;
      QTableWidgetItem *item = new QTableWidgetItem( names.value( i, QString() ) );
      item->setData( Qt::UserRole, ids[i] );
      mOthersTable->setItem( j, 0, item );
      mOthersTable->setItem( j, 1, new QTableWidgetItem( descriptions.value( i, QString() ) ) );
    }
  }
}

QString QgsMapLayerLoadStyleDialog::selectedStyleId()
{
  return mSelectedStyleId;
}

void QgsMapLayerLoadStyleDialog::onRelatedTableSelectionChanged()
{
  selectionChanged( mRelatedTable );
  if ( mRelatedTable->selectionModel()->hasSelection() )
  {
    if ( mOthersTable->selectionModel()->hasSelection() )
    {
      disconnect( mOthersTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsMapLayerLoadStyleDialog::onOthersTableSelectionChanged );
      const QTableWidgetSelectionRange range( 0, 0, mOthersTable->rowCount() - 1, mOthersTable->columnCount() - 1 );
      mOthersTable->setRangeSelected( range, false );
      connect( mOthersTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsMapLayerLoadStyleDialog::onOthersTableSelectionChanged );
    }
  }
}

void QgsMapLayerLoadStyleDialog::onOthersTableSelectionChanged()
{
  selectionChanged( mOthersTable );
  if ( mOthersTable->selectionModel()->hasSelection() )
  {
    if ( mRelatedTable->selectionModel()->hasSelection() )
    {
      disconnect( mRelatedTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsMapLayerLoadStyleDialog::onRelatedTableSelectionChanged );
      const QTableWidgetSelectionRange range( 0, 0, mRelatedTable->rowCount() - 1, mRelatedTable->columnCount() - 1 );
      mRelatedTable->setRangeSelected( range, false );
      connect( mRelatedTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsMapLayerLoadStyleDialog::onRelatedTableSelectionChanged );
    }
  }
}

void QgsMapLayerLoadStyleDialog::selectionChanged( QTableWidget *styleTable )
{
  QTableWidgetItem *item = nullptr;
  const QList<QTableWidgetItem *> selected = styleTable->selectedItems();

  if ( !selected.isEmpty() )
  {
    item = selected.at( 0 );
    mSelectedStyleName = item->text();
    mSelectedStyleId = item->data( Qt::UserRole ).toString();
    mLoadButton->setEnabled( true );
    mDeleteButton->setEnabled( true );
  }
  else
  {
    mSelectedStyleName.clear();
    mSelectedStyleId.clear();
    mLoadButton->setEnabled( false );
    mDeleteButton->setEnabled( false );
  }

  updateLoadButtonState();
}

void QgsMapLayerLoadStyleDialog::accept()
{
  QgsSettings settings;
  settings.setFlagValue( u"style/lastStyleCategories"_s, styleCategories() );
  settings.setValue( u"style/lastLoadStyleTypeSelection"_s, currentStyleType() );
  QDialog::accept();
}

void QgsMapLayerLoadStyleDialog::deleteStyleFromDB()
{
  QString msgError;
  const QString opInfo = QObject::tr( "Delete style %1 from %2" ).arg( mSelectedStyleName, mLayer->providerType() );

  if ( QMessageBox::question( nullptr, QObject::tr( "Delete Style" ), QObject::tr( "Are you sure you want to delete the style %1?" ).arg( mSelectedStyleName ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  mLayer->deleteStyleFromDatabase( mSelectedStyleId, msgError );
  if ( !msgError.isNull() )
  {
    QgsDebugError( opInfo + " failed." );
    QMessageBox::warning( this, opInfo, tr( "%1: fail. %2" ).arg( opInfo, msgError ) );
  }
  else
  {
    //    QgisApp::instance()->messageBar()->pushMessage( opInfo, tr( "%1: success" ).arg( opInfo ), Qgis::MessageLevel::Info, QgisApp::instance()->messageTimeout() );

    //Delete all rows from the UI table widgets
    mRelatedTable->setRowCount( 0 );
    mOthersTable->setRowCount( 0 );

    //Fill UI widgets again from DB. Other users might have changed the styles meanwhile.
    QString errorMsg;
    QStringList ids, names, descriptions;
    //get the list of styles in the db
    const int sectionLimit = mLayer->listStylesInDatabase( ids, names, descriptions, errorMsg );
    if ( !errorMsg.isNull() )
    {
      QMessageBox::warning( this, tr( "Error occurred while retrieving styles from database" ), errorMsg );
    }
    else
    {
      initializeLists( ids, names, descriptions, sectionLimit );
    }
  }
}

void QgsMapLayerLoadStyleDialog::updateLoadButtonState()
{
  const QgsLayerPropertiesDialog::StyleType type = currentStyleType();
  mLoadButton->setEnabled( ( type == QgsLayerPropertiesDialog::DatasourceDatabase && ( mRelatedTable->selectionModel()->hasSelection() || mOthersTable->selectionModel()->hasSelection() ) ) || ( type != QgsLayerPropertiesDialog::DatasourceDatabase && !mFileWidget->filePath().isEmpty() ) || type == QgsLayerPropertiesDialog::UserDatabase );
}

void QgsMapLayerLoadStyleDialog::showHelp()
{
  QgsHelp::openHelp( u"introduction/general_tools.html#save-and-share-layer-properties"_s );
}
