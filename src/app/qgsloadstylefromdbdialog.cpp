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

#include <QSettings>
#include <QMessageBox>
#include <QVector>

QgsLoadStyleFromDBDialog::QgsLoadStyleFromDBDialog( QWidget *parent )
    : QDialog( parent )
    , mSectionLimit( 0 )
{
  setupUi( this );
  setWindowTitle( QStringLiteral( "Database styles manager" ) );
  mSelectedStyleId = "";

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

  connect( mRelatedTable, SIGNAL( itemSelectionChanged() ), this, SLOT( relatedTableSelectionChanged() ) );
  connect( mOthersTable, SIGNAL( itemSelectionChanged() ), this, SLOT( otherTableSelectionChanged() ) );
  connect( mRelatedTable, SIGNAL( doubleClicked( QModelIndex ) ), this, SLOT( accept() ) );
  connect( mOthersTable, SIGNAL( doubleClicked( QModelIndex ) ), this, SLOT( accept() ) );
  connect( mCancelButton, SIGNAL( clicked() ), this, SLOT( reject() ) );
  connect( mDeleteButton, SIGNAL( clicked() ), this, SLOT( deleteStyleFromDB() ) );
  connect( mLoadButton, SIGNAL( clicked() ), this, SLOT( accept() ) );

  setTabOrder( mRelatedTable, mOthersTable );
  setTabOrder( mOthersTable, mCancelButton );
  setTabOrder( mCancelButton, mDeleteButton );
  setTabOrder( mDeleteButton, mLoadButton );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/loadStyleFromDb/geometry" ).toByteArray() );

}

QgsLoadStyleFromDBDialog::~QgsLoadStyleFromDBDialog()
{
  QSettings settings;
  settings.setValue( "/Windows/loadStyleFromDb/geometry", saveGeometry() );
}

void QgsLoadStyleFromDBDialog::initializeLists( const QStringList& ids, const QStringList& names, const QStringList& descriptions, int sectionLimit )
{
  mSectionLimit = sectionLimit;
  int relatedTableNOfCols = sectionLimit > 0 ? 2 : 1;
  int othersTableNOfCols = ids.count() - sectionLimit > 0 ? 2 : 1;
  QString twoColsHeader( "Name;Description" );
  QString oneColsHeader( "No styles found in the database" );
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
    QTableWidgetItem *item = new QTableWidgetItem( names.value( i, "" ) );
    item->setData( Qt::UserRole, ids[i] );
    mRelatedTable->setItem( i, 0, item );
    mRelatedTable->setItem( i, 1, new QTableWidgetItem( descriptions.value( i, "" ) ) );
  }
  for ( int i = sectionLimit; i < ids.count(); i++ )
  {
    int j = i - sectionLimit;
    QTableWidgetItem *item = new QTableWidgetItem( names.value( i, "" ) );
    item->setData( Qt::UserRole, ids[i] );
    mOthersTable->setItem( j, 0, item );
    mOthersTable->setItem( j, 1, new QTableWidgetItem( descriptions.value( i, "" ) ) );
  }
}

QString QgsLoadStyleFromDBDialog::getSelectedStyleId()
{
  return mSelectedStyleId;
}

void QgsLoadStyleFromDBDialog::setLayer( QgsVectorLayer *l )
{
  mLayer = l;
  if ( mLayer->dataProvider()->isDeleteStyleFromDBSupported() )
  {
    //QgsDebugMsg( "QgsLoadStyleFromDBDialog::setLayer → The dataProvider supports isDeleteStyleFromDBSupported" );
    mDeleteButton->setVisible( true );
  }
  else
  {
    // QgsDebugMsg( "QgsLoadStyleFromDBDialog::setLayer → The dataProvider does not supports isDeleteStyleFromDBSupported" );
    mDeleteButton->setVisible( false );
  }
}

void QgsLoadStyleFromDBDialog::relatedTableSelectionChanged()
{
  selectionChanged( mRelatedTable );
  //deselect any other row on the other table widget
  QTableWidgetSelectionRange range( 0, 0, mOthersTable->rowCount() - 1, mOthersTable->columnCount() - 1 );
  mOthersTable->setRangeSelected( range, false );
}

void QgsLoadStyleFromDBDialog::otherTableSelectionChanged()
{
  selectionChanged( mOthersTable );
  //deselect any other row on the other table widget
  QTableWidgetSelectionRange range( 0, 0, mRelatedTable->rowCount() - 1, mRelatedTable->columnCount() - 1 );
  mRelatedTable->setRangeSelected( range, false );
}

void QgsLoadStyleFromDBDialog::selectionChanged( QTableWidget *styleTable )
{
  QTableWidgetItem *item;
  QList<QTableWidgetItem *> selected = styleTable->selectedItems();

  if ( selected.count() > 0 )
  {
    item = selected.at( 0 );
    mSelectedStyleName = item->text();
    mSelectedStyleId = item->data( Qt::UserRole ).toString();
    mLoadButton->setEnabled( true );
    mDeleteButton->setEnabled( true );
  }
  else
  {
    mSelectedStyleName = "";
    mSelectedStyleId = "";
    mLoadButton->setEnabled( false );
    mDeleteButton->setEnabled( false );
  }
}

void QgsLoadStyleFromDBDialog::deleteStyleFromDB()
{
  QString uri, msgError;
  QString opInfo = QObject::tr( "Delete style %1 from %2" ).arg( mSelectedStyleName, mLayer->providerType() );

  if ( QMessageBox::question( nullptr, QObject::tr( "Delete style" ),
                              QObject::tr( "Are you sure you want to delete the style %1?" ).arg( mSelectedStyleName ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  uri = mLayer->dataProvider()->dataSourceUri();
  //  mLayer->dataProvider()->deleteStyleById( uri, mSelectedStyleId, msgError );
  mLayer->deleteStyleFromDatabase( mSelectedStyleId, msgError );

  if ( !msgError.isNull() )
  {
    QgsDebugMsg( opInfo + " failed." );
    QgisApp::instance()->messageBar()->pushMessage( opInfo , tr( "%1: fail. %2" ).arg( opInfo, msgError ), QgsMessageBar::WARNING, QgisApp::instance()->messageTimeout() );
  }
  else
  {
    QgisApp::instance()->messageBar()->pushMessage( opInfo , tr( "%1: success" ).arg( opInfo ), QgsMessageBar::INFO, QgisApp::instance()->messageTimeout() );

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
      QgisApp::instance()->messageBar()->pushMessage( tr( "Error occurred retrieving styles from database" ), errorMsg, QgsMessageBar::WARNING, QgisApp::instance()->messageTimeout() );
    }
    else
    {
      initializeLists( ids, names, descriptions, sectionLimit );
    }
  }
}
