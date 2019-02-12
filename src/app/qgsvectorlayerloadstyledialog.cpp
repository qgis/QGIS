/***************************************************************************
    qgsloadstylefromdbdialog.cpp
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

#include <QMessageBox>
#include <QVector>

#include "qgsvectorlayerloadstyledialog.h"
#include "qgslogger.h"
#include "qgisapp.h"
#include "qgssettings.h"
#include "qgsvectorlayerproperties.h"
#include "qgsmaplayerstylecategoriesmodel.h"
#include "qgsmessagebar.h"
#include "qgsapplication.h"


QgsVectorLayerLoadStyleDialog::QgsVectorLayerLoadStyleDialog( QgsVectorLayer *layer, QWidget *parent )
  : QDialog( parent )
  , mLayer( layer )
{
  setupUi( this );
  setWindowTitle( QStringLiteral( "Database styles manager" ) );

  mDeleteButton = mButtonBox->button( QDialogButtonBox::StandardButton::Close );
  mDeleteButton->setText( tr( "Delete Style" ) );
  mDeleteButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionDeleteSelected.svg" ) ) );
  mLoadButton = mButtonBox->button( QDialogButtonBox::StandardButton::Open );
  mLoadButton->setText( tr( "Load Style" ) );
  mCancelButton = mButtonBox->button( QDialogButtonBox::StandardButton::Cancel );

  QgsSettings settings;

  QString providerName = mLayer->providerType();
  if ( providerName == QLatin1String( "ogr" ) )
  {
    providerName = mLayer->dataProvider()->storageType();
    if ( providerName == QLatin1String( "GPKG" ) )
      providerName = QStringLiteral( "GeoPackage" );
  }

  QString myLastUsedDir = settings.value( QStringLiteral( "style/lastStyleDir" ), QDir::homePath() ).toString();

  // load style type combobox
  connect( mStyleTypeComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ), this, [ = ]( int )
  {
    QgsVectorLayerProperties::StyleType type = currentStyleType();
    mFromFileWidget->setVisible( type != QgsVectorLayerProperties::DB );
    mFromDbWidget->setVisible( type == QgsVectorLayerProperties::DB );
    mDeleteButton->setVisible( type == QgsVectorLayerProperties::DB && mLayer->dataProvider()->isDeleteStyleFromDatabaseSupported() );
    mStyleCategoriesListView->setEnabled( currentStyleType() != QgsVectorLayerProperties::SLD );
    updateLoadButtonState();
  } );
  mStyleTypeComboBox->addItem( tr( "from file" ), QgsVectorLayerProperties::QML ); // QML is used as entry, but works for SLD too, see currentStyleType()
  if ( mLayer->dataProvider()->isSaveAndLoadStyleToDatabaseSupported() )
    mStyleTypeComboBox->addItem( tr( "from database (%1)" ).arg( providerName ), QgsVectorLayerProperties::DB );

  // fill style categories
  mModel = new QgsMapLayerStyleCategoriesModel( this );
  QgsMapLayer::StyleCategories lastStyleCategories = settings.flagValue( QStringLiteral( "style/lastStyleCategories" ), QgsMapLayer::AllStyleCategories );
  mModel->setCategories( lastStyleCategories );
  mStyleCategoriesListView->setModel( mModel );

  // load from file setup
  mFileWidget->setFilter( tr( "QGIS Layer Style File, SLD File" ) + QStringLiteral( " (*.qml *.sld)" ) );
  mFileWidget->setStorageMode( QgsFileWidget::GetFile );
  mFileWidget->setDefaultRoot( myLastUsedDir );
  connect( mFileWidget, &QgsFileWidget::fileChanged, this, [ = ]( const QString & path )
  {
    mStyleCategoriesListView->setEnabled( currentStyleType() != QgsVectorLayerProperties::SLD );
    QgsSettings settings;
    QFileInfo tmplFileInfo( path );
    settings.setValue( QStringLiteral( "style/lastStyleDir" ), tmplFileInfo.absolutePath() );

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
  connect( mRelatedTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsVectorLayerLoadStyleDialog::onRelatedTableSelectionChanged );
  connect( mOthersTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsVectorLayerLoadStyleDialog::onOthersTableSelectionChanged );
  connect( mRelatedTable, &QTableWidget::doubleClicked, this, &QDialog::accept );
  connect( mOthersTable, &QTableWidget::doubleClicked, this, &QDialog::accept );
  connect( mCancelButton, &QPushButton::clicked, this, &QDialog::reject );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, &QgsVectorLayerLoadStyleDialog::showHelp );
  connect( mLoadButton, &QPushButton::clicked, this, &QDialog::accept );
  connect( mDeleteButton, &QPushButton::clicked, this, &QgsVectorLayerLoadStyleDialog::deleteStyleFromDB );
  setTabOrder( mRelatedTable, mOthersTable );

  restoreGeometry( settings.value( QStringLiteral( "Windows/vectorLayerLoadStyle/geometry" ) ).toByteArray() );
  mStyleCategoriesListView->adjustSize();
}

QgsVectorLayerLoadStyleDialog::~QgsVectorLayerLoadStyleDialog()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/vectorLayerLoadStyle/geometry" ), saveGeometry() );
}

QgsMapLayer::StyleCategories QgsVectorLayerLoadStyleDialog::styleCategories() const
{
  return mModel->categories();
}

QgsVectorLayerProperties::StyleType QgsVectorLayerLoadStyleDialog::currentStyleType() const
{
  QgsVectorLayerProperties::StyleType type = mStyleTypeComboBox->currentData().value<QgsVectorLayerProperties::StyleType>();
  if ( type == QgsVectorLayerProperties::QML )
  {
    QFileInfo fi( mFileWidget->filePath() );
    if ( fi.exists() && fi.suffix().compare( QStringLiteral( "sld" ), Qt::CaseInsensitive ) == 0 )
      type = QgsVectorLayerProperties::SLD;
  }
  return type;
}

QString QgsVectorLayerLoadStyleDialog::filePath() const
{
  return mFileWidget->filePath();
}

void QgsVectorLayerLoadStyleDialog::initializeLists( const QStringList &ids, const QStringList &names, const QStringList &descriptions, int sectionLimit )
{
  // -1 means no ids
  mSectionLimit = sectionLimit;
  int relatedTableNOfCols = sectionLimit > 0 ? 2 : 1;
  int othersTableNOfCols = ( sectionLimit >= 0 && ids.count() - sectionLimit > 0 ) ? 2 : 1;
  QString twoColsHeader( QStringLiteral( "Name;Description" ) );
  QString oneColsHeader( QStringLiteral( "No styles found in the database" ) );
  QString relatedTableHeader = relatedTableNOfCols == 1 ? oneColsHeader : twoColsHeader;
  QString othersTableHeader = othersTableNOfCols == 1 ? oneColsHeader : twoColsHeader;

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
      int j = i - sectionLimit;
      QTableWidgetItem *item = new QTableWidgetItem( names.value( i, QString() ) );
      item->setData( Qt::UserRole, ids[i] );
      mOthersTable->setItem( j, 0, item );
      mOthersTable->setItem( j, 1, new QTableWidgetItem( descriptions.value( i, QString() ) ) );
    }
  }
}

QString QgsVectorLayerLoadStyleDialog::selectedStyleId()
{
  return mSelectedStyleId;
}

void QgsVectorLayerLoadStyleDialog::onRelatedTableSelectionChanged()
{
  selectionChanged( mRelatedTable );
  if ( mRelatedTable->selectionModel()->hasSelection() )
  {
    if ( mOthersTable->selectionModel()->hasSelection() )
    {
      disconnect( mOthersTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsVectorLayerLoadStyleDialog::onOthersTableSelectionChanged );
      QTableWidgetSelectionRange range( 0, 0, mOthersTable->rowCount() - 1, mOthersTable->columnCount() - 1 );
      mOthersTable->setRangeSelected( range, false );
      connect( mOthersTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsVectorLayerLoadStyleDialog::onOthersTableSelectionChanged );
    }
  }
}

void QgsVectorLayerLoadStyleDialog::onOthersTableSelectionChanged()
{
  selectionChanged( mOthersTable );
  if ( mOthersTable->selectionModel()->hasSelection() )
  {
    if ( mRelatedTable->selectionModel()->hasSelection() )
    {
      disconnect( mRelatedTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsVectorLayerLoadStyleDialog::onRelatedTableSelectionChanged );
      QTableWidgetSelectionRange range( 0, 0, mRelatedTable->rowCount() - 1, mRelatedTable->columnCount() - 1 );
      mRelatedTable->setRangeSelected( range, false );
      connect( mRelatedTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsVectorLayerLoadStyleDialog::onRelatedTableSelectionChanged );
    }
  }
}

void QgsVectorLayerLoadStyleDialog::selectionChanged( QTableWidget *styleTable )
{
  QTableWidgetItem *item = nullptr;
  QList<QTableWidgetItem *> selected = styleTable->selectedItems();

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

void QgsVectorLayerLoadStyleDialog::accept()
{
  QgsSettings().setFlagValue( QStringLiteral( "style/lastStyleCategories" ), styleCategories() );
  QDialog::accept();
}

void QgsVectorLayerLoadStyleDialog::deleteStyleFromDB()
{
  QString msgError;
  QString opInfo = QObject::tr( "Delete style %1 from %2" ).arg( mSelectedStyleName, mLayer->providerType() );

  if ( QMessageBox::question( nullptr, QObject::tr( "Delete Style" ),
                              QObject::tr( "Are you sure you want to delete the style %1?" ).arg( mSelectedStyleName ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  mLayer->deleteStyleFromDatabase( mSelectedStyleId, msgError );
  if ( !msgError.isNull() )
  {
    QgsDebugMsg( opInfo + " failed." );
    QgisApp::instance()->messageBar()->pushMessage( opInfo, tr( "%1: fail. %2" ).arg( opInfo, msgError ), Qgis::Warning, QgisApp::instance()->messageTimeout() );
  }
  else
  {
    QgisApp::instance()->messageBar()->pushMessage( opInfo, tr( "%1: success" ).arg( opInfo ), Qgis::Info, QgisApp::instance()->messageTimeout() );

    //Delete all rows from the UI table widgets
    mRelatedTable->setRowCount( 0 );
    mOthersTable->setRowCount( 0 );

    //Fill UI widgets again from DB. Other users might have change the styles meanwhile.
    QString errorMsg;
    QStringList ids, names, descriptions;
    //get the list of styles in the db
    int sectionLimit = mLayer->listStylesInDatabase( ids, names, descriptions, errorMsg );
    if ( !errorMsg.isNull() )
    {
      QgisApp::instance()->messageBar()->pushMessage( tr( "Error occurred while retrieving styles from database" ), errorMsg, Qgis::Warning, QgisApp::instance()->messageTimeout() );
    }
    else
    {
      initializeLists( ids, names, descriptions, sectionLimit );
    }
  }
}

void QgsVectorLayerLoadStyleDialog::updateLoadButtonState()
{
  QgsVectorLayerProperties::StyleType type = currentStyleType();
  mLoadButton->setEnabled( ( type == QgsVectorLayerProperties::DB
                             && ( mRelatedTable->selectionModel()->hasSelection() || mOthersTable->selectionModel()->hasSelection()
                                ) ) ||
                           ( type != QgsVectorLayerProperties::DB && !mFileWidget->filePath().isEmpty() ) );
}

void QgsVectorLayerLoadStyleDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "introduction/general_tools.html#save-and-share-layer-properties" ) );
}
