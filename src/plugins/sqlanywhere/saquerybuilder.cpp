/***************************************************************************
  saquerybuilder.cpp
  Query builder for layers backed by SQL Anywhere database.
  -------------------
    begin                : Dec 2010
    copyright            : (C) 2010 by iAnywhere Solutions, Inc.
    author               : David DeHaan
    email                : ddehaan at sybase dot com

 This class was copied and modified from QgsQueryBuilder because that
 class is not accessible to QGIS plugins.  Therefore, the author gratefully
 acknowledges the following copyright on the original content:
    qgsquerybuilder.cpp
    Date                 : 2004-11-19
    Copyright            : (C) 2004 by Gary E.Sherman
    Email                : sherman at mrcc.com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "saquerybuilder.h"

#include "qgslogger.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"

#include <QListView>
#include <QMessageBox>
#include <QRegExp>
#include <QPushButton>

// constructor used when the query builder must make its own
// connection to the database
SaQueryBuilder::SaQueryBuilder( QgsVectorLayer *layer,
                                QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl ), mLayer( layer )
{
  setupUi( this );
  connect( buttonBox, SIGNAL( helpRequested() ), this, SLOT( helpClicked() ) );

  QPushButton *pbn = new QPushButton( tr( "&Test" ) );
  buttonBox->addButton( pbn, QDialogButtonBox::ActionRole );
  connect( pbn, SIGNAL( clicked() ), this, SLOT( test() ) );

  pbn = new QPushButton( tr( "&Clear" ) );
  buttonBox->addButton( pbn, QDialogButtonBox::ActionRole );
  connect( pbn, SIGNAL( clicked() ), this, SLOT( clear() ) );

  // remove the ILIKE button since ILIKE is a PostgreSQL special
  // not supported by SQL Anywhere
  btnILike->setVisible( false );

  setupGuiViews();

  mOrigSubsetString = layer->subsetString();

  lblDataUri->setText( layer->publicSource() );
  txtSQL->setText( mOrigSubsetString );

  populateFields();
}

SaQueryBuilder::~SaQueryBuilder()
{
}

void SaQueryBuilder::populateFields()
{
  for ( QgsFieldMap::const_iterator it = mLayer->pendingFields().begin(); it != mLayer->pendingFields().end(); it++ )
  {
    QStandardItem *myItem = new QStandardItem( it->name() );
    myItem->setData( it.key() );
    myItem->setEditable( false );
    mModelFields->insertRow( mModelFields->rowCount(), myItem );
  }

  // All fields get ... setup
  setupLstFieldsModel();
}

void SaQueryBuilder::setupLstFieldsModel()
{
  lstFields->setModel( mModelFields );
}

void SaQueryBuilder::setupGuiViews()
{
  //Initialize the models
  mModelFields = new QStandardItemModel();
  mModelValues = new QStandardItemModel();
  // Modes
  lstFields->setViewMode( QListView::ListMode );
  lstValues->setViewMode( QListView::ListMode );
  lstFields->setSelectionBehavior( QAbstractItemView::SelectRows );
  lstValues->setSelectionBehavior( QAbstractItemView::SelectRows );
  // Performance tip since Qt 4.1
  lstFields->setUniformItemSizes( true );
  lstValues->setUniformItemSizes( true );
  // Colored rows
  lstFields->setAlternatingRowColors( true );
  lstValues->setAlternatingRowColors( true );
}

void SaQueryBuilder::fillValues( int idx, QString subsetString, int limit )
{
  // clear the model
  mModelValues->clear();

  if ( !mLayer->setSubsetString( subsetString ) )
  {
    QMessageBox::information( this, tr( "Invalid Query" ), tr( "Setting the query failed" ) );
    return;
  }

  // determine the field type
  QList<QVariant> values;
  mLayer->uniqueValues( idx, values, limit );

  for ( int i = 0; i < values.size(); i++ )
  {
    QStandardItem *myItem = new QStandardItem( values[i].toString() );
    myItem->setEditable( false );
    mModelValues->insertRow( mModelValues->rowCount(), myItem );
  }
}

void SaQueryBuilder::on_btnSampleValues_clicked()
{
  lstValues->setCursor( Qt::WaitCursor );

  //delete connection mModelValues and lstValues
  QStandardItemModel *tmp = new QStandardItemModel();
  lstValues->setModel( tmp );
  //Clear and fill the mModelValues
  fillValues( mModelFields->data( lstFields->currentIndex(), Qt::UserRole + 1 ).toInt(), mOrigSubsetString, 25 );
  lstValues->setModel( mModelValues );
  lstValues->setCursor( Qt::ArrowCursor );
  //delete the tmp
  delete tmp;

}

void SaQueryBuilder::on_btnGetAllValues_clicked()
{
  lstValues->setCursor( Qt::WaitCursor );

  //delete connection mModelValues and lstValues
  QStandardItemModel *tmp = new QStandardItemModel();
  lstValues->setModel( tmp );
  //Clear and fill the mModelValues
  fillValues( mModelFields->data( lstFields->currentIndex(), Qt::UserRole + 1 ).toInt(), mOrigSubsetString, -1 );
  lstValues->setModel( mModelValues );
  lstValues->setCursor( Qt::ArrowCursor );
  //delete the tmp
  delete tmp;
}

void SaQueryBuilder::test()
{
  // test the sql statement to see if it works
  // by counting the number of records that would be
  // returned

  // if there is no sql, issue a warning
  if ( txtSQL->toPlainText().isEmpty() )
  {
    QMessageBox::information( this,
                              tr( "No Query" ),
                              tr( "You must create a query before you can test it" ) );
  }
  else if ( mLayer->setSubsetString( txtSQL->toPlainText() ) )
  {
    QMessageBox::information( this,
                              tr( "Query Result" ),
                              tr( "The where clause returned %n row(s).", "returned test rows", mLayer->featureCount() ) );
  }
  else
  {
    QMessageBox::warning( this,
                          tr( "Query Failed" ),
                          tr( "An error occurred when executing the query" ) );
  }
}

// Slot for showing help
void SaQueryBuilder::helpClicked()
{
  // QgsContextHelp::run( context_id );
}

void SaQueryBuilder::accept()
{
  // if user hits Ok and there is no query, skip the validation
  if ( !txtSQL->toPlainText().trimmed().isEmpty() )
  {
    if ( !mLayer->setSubsetString( txtSQL->toPlainText() ) )
    {
      //error in query - show the problem
      QMessageBox::warning( this, tr( "Error in Query" ), tr( "The subset string could not be set" ) );
      return;
    }
  }

  QDialog::accept();
}

void SaQueryBuilder::reject()
{
  if ( mLayer->subsetString() != mOrigSubsetString )
    mLayer->setSubsetString( mOrigSubsetString );

  QDialog::reject();
}

void SaQueryBuilder::on_btnEqual_clicked()
{
  txtSQL->insertPlainText( " = " );
}

void SaQueryBuilder::on_btnLessThan_clicked()
{
  txtSQL->insertPlainText( " < " );
}

void SaQueryBuilder::on_btnGreaterThan_clicked()
{
  txtSQL->insertPlainText( " > " );
}

void SaQueryBuilder::on_btnPct_clicked()
{
  txtSQL->insertPlainText( "%" );
}

void SaQueryBuilder::on_btnIn_clicked()
{
  txtSQL->insertPlainText( " IN " );
}

void SaQueryBuilder::on_btnNotIn_clicked()
{
  txtSQL->insertPlainText( " NOT IN " );
}

void SaQueryBuilder::on_btnLike_clicked()
{
  txtSQL->insertPlainText( " LIKE " );
}

QString SaQueryBuilder::sql()
{
  return txtSQL->toPlainText();
}

void SaQueryBuilder::setSql( QString sqlStatement )
{
  txtSQL->setText( sqlStatement );
}

void SaQueryBuilder::on_lstFields_clicked( const QModelIndex &index )
{
  if ( mPreviousFieldRow != index.row() )
  {
    mPreviousFieldRow = index.row();

    btnSampleValues->setEnabled( true );
    btnGetAllValues->setEnabled( true );

    mModelValues->clear();
  }
}

void SaQueryBuilder::on_lstFields_doubleClicked( const QModelIndex &index )
{
  txtSQL->insertPlainText( "\"" + mLayer->pendingFields()[ mModelFields->data( index, Qt::UserRole+1 ).toInt()].name() + "\"" );
}

void SaQueryBuilder::on_lstValues_doubleClicked( const QModelIndex &index )
{
  txtSQL->insertPlainText( "'" + mModelValues->data( index ).toString() + "'" );
}

void SaQueryBuilder::on_btnLessEqual_clicked()
{
  txtSQL->insertPlainText( " <= " );
}

void SaQueryBuilder::on_btnGreaterEqual_clicked()
{
  txtSQL->insertPlainText( " >= " );
}

void SaQueryBuilder::on_btnNotEqual_clicked()
{
  txtSQL->insertPlainText( " != " );
}

void SaQueryBuilder::on_btnAnd_clicked()
{
  txtSQL->insertPlainText( " AND " );
}

void SaQueryBuilder::on_btnNot_clicked()
{
  txtSQL->insertPlainText( " NOT " );
}

void SaQueryBuilder::on_btnOr_clicked()
{
  txtSQL->insertPlainText( " OR " );
}

void SaQueryBuilder::clear()
{
  txtSQL->clear();
}

void SaQueryBuilder::on_btnILike_clicked()
{
  txtSQL->insertPlainText( " ILIKE " );
}
void SaQueryBuilder::setDatasourceDescription( QString uri )
{
  lblDataUri->setText( uri );
}
