/***************************************************************************
                qgsquerybuilder.cpp - Query Builder
                     --------------------------------------
               Date                 : 2004-11-19
               Copyright            : (C) 2004 by Gary E.Sherman
               Email                : sherman at mrcc.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */
#include "qgsquerybuilder.h"
#include "qgslogger.h"
#include <QListView>
#include <QMessageBox>
#include <QRegExp>
#include "qgslogger.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"

#if QT_VERSION < 0x040300
#define toPlainText() text()
#endif


// constructor used when the query builder must make its own
// connection to the database
QgsQueryBuilder::QgsQueryBuilder( QgsVectorLayer *layer,
                                  QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl ), mLayer( layer )
{
  setupUi( this );
  setupGuiViews();

  mOrigSubsetString = layer->subsetString();

  lblDataUri->setText( layer->publicSource() );
  populateFields();
}

QgsQueryBuilder::~QgsQueryBuilder()
{
}

void QgsQueryBuilder::populateFields()
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

void QgsQueryBuilder::setupLstFieldsModel()
{
  lstFields->setModel( mModelFields );
}

void QgsQueryBuilder::setupGuiViews()
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

void QgsQueryBuilder::fillValues( int idx, QString subsetString, int limit )
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
  mLayer->dataProvider()->uniqueValues( idx, values, limit );

  for ( int i = 0; i < values.size(); i++ )
  {
    QStandardItem *myItem = new QStandardItem( values[i].toString() );
    myItem->setEditable( false );
    mModelValues->insertRow( mModelValues->rowCount(), myItem );
  }
}

void QgsQueryBuilder::on_btnSampleValues_clicked()
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

void QgsQueryBuilder::on_btnGetAllValues_clicked()
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

void QgsQueryBuilder::on_btnTest_clicked()
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

void QgsQueryBuilder::on_btnOk_clicked()
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

  accept();
}

void QgsQueryBuilder::on_btnCancel_clicked()
{
  if ( mLayer->subsetString() != mOrigSubsetString )
    mLayer->setSubsetString( mOrigSubsetString );

  reject();
}

void QgsQueryBuilder::on_btnEqual_clicked()
{
  txtSQL->insertPlainText( " = " );
}

void QgsQueryBuilder::on_btnLessThan_clicked()
{
  txtSQL->insertPlainText( " < " );
}

void QgsQueryBuilder::on_btnGreaterThan_clicked()
{
  txtSQL->insertPlainText( " > " );
}

void QgsQueryBuilder::on_btnPct_clicked()
{
  txtSQL->insertPlainText( "%" );
}

void QgsQueryBuilder::on_btnIn_clicked()
{
  txtSQL->insertPlainText( " IN " );
}

void QgsQueryBuilder::on_btnNotIn_clicked()
{
  txtSQL->insertPlainText( " NOT IN " );
}

void QgsQueryBuilder::on_btnLike_clicked()
{
  txtSQL->insertPlainText( " LIKE " );
}

QString QgsQueryBuilder::sql()
{
  return txtSQL->toPlainText();
}

void QgsQueryBuilder::setSql( QString sqlStatement )
{
  txtSQL->setText( sqlStatement );
}

void QgsQueryBuilder::on_lstFields_clicked( const QModelIndex &index )
{
  if ( mPreviousFieldRow != index.row() )
  {
    mPreviousFieldRow = index.row();

    btnSampleValues->setEnabled( true );
    btnGetAllValues->setEnabled( true );

    mModelValues->clear();
  }
}

void QgsQueryBuilder::on_lstFields_doubleClicked( const QModelIndex &index )
{
  txtSQL->insertPlainText( "\"" + mLayer->pendingFields()[ mModelFields->data( index, Qt::UserRole+1 ).toInt()].name() + "\"" );
}

void QgsQueryBuilder::on_lstValues_doubleClicked( const QModelIndex &index )
{
  txtSQL->insertPlainText( "'" + mModelValues->data( index ).toString() + "'" );
}

void QgsQueryBuilder::on_btnLessEqual_clicked()
{
  txtSQL->insertPlainText( " <= " );
}

void QgsQueryBuilder::on_btnGreaterEqual_clicked()
{
  txtSQL->insertPlainText( " >= " );
}

void QgsQueryBuilder::on_btnNotEqual_clicked()
{
  txtSQL->insertPlainText( " != " );
}

void QgsQueryBuilder::on_btnAnd_clicked()
{
  txtSQL->insertPlainText( " AND " );
}

void QgsQueryBuilder::on_btnNot_clicked()
{
  txtSQL->insertPlainText( " NOT " );
}

void QgsQueryBuilder::on_btnOr_clicked()
{
  txtSQL->insertPlainText( " OR " );
}

void QgsQueryBuilder::on_btnClear_clicked()
{
  txtSQL->clear();
}

void QgsQueryBuilder::on_btnILike_clicked()
{
  txtSQL->insertPlainText( " ILIKE " );
}
void QgsQueryBuilder::setDatasourceDescription( QString uri )
{
  lblDataUri->setText( uri );
}
