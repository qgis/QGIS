/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsloadstylefromdbdialog.h"
#include "qgslogger.h"

#include <QMessageBox>
#include <QVector>

QgsLoadStyleFromDBDialog::QgsLoadStyleFromDBDialog( QWidget *parent )
    : QDialog( parent )
{
  setupUi( this );
  setWindowTitle( "Load style from database" );
  mSelectedStyleId = tr( "" );

  mLoadButton->setDisabled( true );
  mRelatedTable->setEditTriggers( QTableWidget::NoEditTriggers );
  mRelatedTable->horizontalHeader()->setStretchLastSection( true );
  mRelatedTable->setSelectionBehavior( QTableWidget::SelectRows );
  mRelatedTable->verticalHeader()->setVisible( false );

  mOthersTable->setEditTriggers( QTableWidget::NoEditTriggers );
  mOthersTable->horizontalHeader()->setStretchLastSection( true );
  mOthersTable->setSelectionBehavior( QTableWidget::SelectRows );
  mOthersTable->verticalHeader()->setVisible( false );

  connect( mRelatedTable, SIGNAL( cellClicked( int, int ) ), this, SLOT( cellSelectedRelatedTable( int ) ) );
  connect( mOthersTable, SIGNAL( cellClicked( int, int ) ), this, SLOT( cellSelectedOthersTable( int ) ) );
  connect( mRelatedTable, SIGNAL( doubleClicked( QModelIndex ) ),
           this, SLOT( accept() ) );
  connect( mOthersTable, SIGNAL( doubleClicked( QModelIndex ) ),
           this, SLOT( accept() ) );
  connect( mCancelButton, SIGNAL( clicked() ), this, SLOT( reject() ) );
  connect( mLoadButton, SIGNAL( clicked() ), this, SLOT( accept() ) );

  setTabOrder( mRelatedTable, mOthersTable );
  setTabOrder( mOthersTable, mCancelButton );
  setTabOrder( mCancelButton, mLoadButton );

}

void QgsLoadStyleFromDBDialog::initializeLists( QStringList ids, QStringList names, QStringList descriptions, int sectionLimit )
{
  mIds = ids;
  mNames = names;
  mDescriptions = descriptions;
  mSectionLimit = sectionLimit;
  int relatedTableNOfCols = ( sectionLimit > 0 ) ? 2 : 1;
  int othersTableNOfCols = ( ids.count() - sectionLimit > 0 ) ? 2 : 1;
  QString twoColsHeader( "Name;Description" );
  QString oneColsHeader( "No styles found in the database" );
  QString relatedTableHeader = ( relatedTableNOfCols == 1 ) ? oneColsHeader : twoColsHeader;
  QString othersTableHeader = ( othersTableNOfCols == 1 ) ? oneColsHeader : twoColsHeader;

  mRelatedTable->setColumnCount( relatedTableNOfCols );
  mOthersTable->setColumnCount( othersTableNOfCols );
  mRelatedTable->setHorizontalHeaderLabels( relatedTableHeader.split( ";" ) );
  mOthersTable->setHorizontalHeaderLabels( othersTableHeader.split( ";" ) );
  mRelatedTable->setRowCount( sectionLimit );
  mOthersTable->setRowCount( ids.count() - sectionLimit );
  mRelatedTable->setDisabled(( relatedTableNOfCols == 1 ) );
  mOthersTable->setDisabled(( othersTableNOfCols == 1 ) );

  for ( int i = 0; i < sectionLimit; i++ )
  {
    mRelatedTable->setItem( i, 0, new QTableWidgetItem( names.value( i, "" ) ) );
    mRelatedTable->setItem( i, 1, new QTableWidgetItem( descriptions.value( i, "" ) ) );
  }
  for ( int i = sectionLimit; i < ids.count(); i++ )
  {
    int j = i - sectionLimit;
    mOthersTable->setItem( j, 0, new QTableWidgetItem( names.value( i, "" ) ) );
    mOthersTable->setItem( j, 1, new QTableWidgetItem( descriptions.value( i, "" ) ) );
  }
}

QString QgsLoadStyleFromDBDialog::getSelectedStyleId()
{
  return mSelectedStyleId;
}

void QgsLoadStyleFromDBDialog::cellSelectedRelatedTable( int r )
{
  mLoadButton->setEnabled( true );
  mSelectedStyleId = mIds.value( r );
}

void QgsLoadStyleFromDBDialog::cellSelectedOthersTable( int r )
{
  mLoadButton->setEnabled( true );
  mSelectedStyleId = mIds.value( r + mSectionLimit );
}
