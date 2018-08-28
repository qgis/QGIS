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

#include "qgsloadstylefromdbdialog.h"
#include "qgslogger.h"
#include "qgisapp.h"
#include "qgssettings.h"

#include <QMessageBox>
#include <QVector>

QgsLoadStyleFromDBDialog::QgsLoadStyleFromDBDialog( QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );
  setWindowTitle( QStringLiteral( "Database styles manager" ) );

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

  connect( mRelatedTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsLoadStyleFromDBDialog::onRelatedTableSelectionChanged );
  connect( mOthersTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsLoadStyleFromDBDialog::onOthersTableSelectionChanged );
  connect( mRelatedTable, &QTableWidget::doubleClicked, this, &QDialog::accept );
  connect( mOthersTable, &QTableWidget::doubleClicked, this, &QDialog::accept );
  connect( mCancelButton, &QPushButton::clicked, this, &QDialog::reject );
  connect( mLoadButton, &QPushButton::clicked, this, &QDialog::accept );
  connect( mDeleteButton, &QPushButton::clicked, this, &QgsLoadStyleFromDBDialog::deleteStyleFromDB );

  setTabOrder( mRelatedTable, mOthersTable );
  setTabOrder( mOthersTable, mCancelButton );
  setTabOrder( mCancelButton, mDeleteButton );
  setTabOrder( mDeleteButton, mLoadButton );

  QgsSettings settings;
  restoreGeometry( settings.value( QStringLiteral( "Windows/loadStyleFromDb/geometry" ) ).toByteArray() );
}

QgsLoadStyleFromDBDialog::~QgsLoadStyleFromDBDialog()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/loadStyleFromDb/geometry" ), saveGeometry() );
}

void QgsLoadStyleFromDBDialog::initializeLists( const QStringList &ids, const QStringList &names, const QStringList &descriptions, int sectionLimit )
{
  mSectionLimit = sectionLimit;
  int relatedTableNOfCols = sectionLimit > 0 ? 2 : 1;
  int othersTableNOfCols = ids.count() - sectionLimit > 0 ? 2 : 1;
  QString twoColsHeader( QStringLiteral( "Name;Description" ) );
  QString oneColsHeader( QStringLiteral( "No styles found in the database" ) );
  QString relatedTableHeader = relatedTableNOfCols == 1 ? oneColsHeader : twoColsHeader;
  QString othersTableHeader = othersTableNOfCols == 1 ? oneColsHeader : twoColsHeader;

  mRelatedTable->setColumnCount( relatedTableNOfCols );
  mOthersTable->setColumnCount( othersTableNOfCols );
  mRelatedTable->setHorizontalHeaderLabels( relatedTableHeader.split( ';' ) );
  mOthersTable->setHorizontalHeaderLabels( othersTableHeader.split( ';' ) );
  mRelatedTable->setRowCount( sectionLimit );
  mOthersTable->setRowCount( ids.count() - sectionLimit );
  mRelatedTable->setDisabled( relatedTableNOfCols == 1 );
  mOthersTable->setDisabled( othersTableNOfCols == 1 );

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

QString QgsLoadStyleFromDBDialog::getSelectedStyleId()
{
  return mSelectedStyleId;
}

void QgsLoadStyleFromDBDialog::setLayer( QgsVectorLayer *l )
{
  mLayer = l;
  mDeleteButton->setVisible( mLayer->dataProvider()->isDeleteStyleFromDatabaseSupported() );
}

void QgsLoadStyleFromDBDialog::onRelatedTableSelectionChanged()
{
  selectionChanged( mRelatedTable );
  if ( mRelatedTable->selectionModel()->hasSelection() )
  {
    if ( mOthersTable->selectionModel()->hasSelection() )
    {
      disconnect( mOthersTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsLoadStyleFromDBDialog::onOthersTableSelectionChanged );
      QTableWidgetSelectionRange range( 0, 0, mOthersTable->rowCount() - 1, mOthersTable->columnCount() - 1 );
      mOthersTable->setRangeSelected( range, false );
      connect( mOthersTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsLoadStyleFromDBDialog::onOthersTableSelectionChanged );
    }
  }
}

void QgsLoadStyleFromDBDialog::onOthersTableSelectionChanged()
{
  selectionChanged( mOthersTable );
  if ( mOthersTable->selectionModel()->hasSelection() )
  {
    if ( mRelatedTable->selectionModel()->hasSelection() )
    {
      disconnect( mRelatedTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsLoadStyleFromDBDialog::onRelatedTableSelectionChanged );
      QTableWidgetSelectionRange range( 0, 0, mRelatedTable->rowCount() - 1, mRelatedTable->columnCount() - 1 );
      mRelatedTable->setRangeSelected( range, false );
      connect( mRelatedTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsLoadStyleFromDBDialog::onRelatedTableSelectionChanged );
    }
  }
}

void QgsLoadStyleFromDBDialog::selectionChanged( QTableWidget *styleTable )
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
}

void QgsLoadStyleFromDBDialog::deleteStyleFromDB()
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
