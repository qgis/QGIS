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
#include "qgsquerybuilder.h"
#include "qgslogger.h"
#include <QListView>
#include <QMessageBox>
#include <QRegExp>
#include <QPushButton>
#include <QSettings>
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"

// constructor used when the query builder must make its own
// connection to the database
QgsQueryBuilder::QgsQueryBuilder( QgsVectorLayer *layer,
                                  QWidget *parent, Qt::WindowFlags fl )
    : QDialog( parent, fl )
    , mPreviousFieldRow( -1 )
    , mLayer( layer )
{
  setupUi( this );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/QueryBuilder/geometry" ).toByteArray() );

  QPushButton *pbn = new QPushButton( tr( "&Test" ) );
  buttonBox->addButton( pbn, QDialogButtonBox::ActionRole );
  connect( pbn, SIGNAL( clicked() ), this, SLOT( test() ) );

  pbn = new QPushButton( tr( "&Clear" ) );
  buttonBox->addButton( pbn, QDialogButtonBox::ActionRole );
  connect( pbn, SIGNAL( clicked() ), this, SLOT( clear() ) );

  setupGuiViews();

  mOrigSubsetString = layer->subsetString();

  mUseUnfilteredLayer->setDisabled( mLayer->subsetString().isEmpty() );

  lblDataUri->setText( tr( "Set provider filter on %1" ).arg( layer->name() ) );
  txtSQL->setText( mOrigSubsetString );

  populateFields();
}

QgsQueryBuilder::~QgsQueryBuilder()
{
  QSettings settings;
  settings.setValue( "/Windows/QueryBuilder/geometry", saveGeometry() );
}

void QgsQueryBuilder::showEvent( QShowEvent *event )
{
  txtSQL->setFocus();
  QDialog::showEvent( event );
}

void QgsQueryBuilder::populateFields()
{
  const QgsFields& fields = mLayer->fields();
  for ( int idx = 0; idx < fields.count(); ++idx )
  {
    if ( fields.fieldOrigin( idx ) != QgsFields::OriginProvider )
    {
      // only consider native fields
      continue;
    }
    QStandardItem *myItem = new QStandardItem( fields[idx].name() );
    myItem->setData( idx );
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

void QgsQueryBuilder::fillValues( int idx, int limit )
{
  // clear the model
  mModelValues->clear();

  // determine the field type
  QList<QVariant> values;
  mLayer->uniqueValues( idx, values, limit );

  QSettings settings;
  QString nullValue = settings.value( "qgis/nullValue", "NULL" ).toString();

  QgsDebugMsg( QString( "nullValue: %1" ).arg( nullValue ) );

  for ( int i = 0; i < values.size(); i++ )
  {
    QString value;
    if ( values[i].isNull() )
      value = nullValue;
    else if ( values[i].type() == QVariant::Date && mLayer->providerType() == "ogr" && mLayer->storageType() == "ESRI Shapefile" )
      value = values[i].toDate().toString( "yyyy/MM/dd" );
    else
      value = values[i].toString();

    QStandardItem *myItem = new QStandardItem( value );
    myItem->setEditable( false );
    myItem->setData( values[i], Qt::UserRole + 1 );
    mModelValues->insertRow( mModelValues->rowCount(), myItem );
    QgsDebugMsg( QString( "Value is null: %1\nvalue: %2" ).arg( values[i].isNull() ).arg( values[i].isNull() ? nullValue : values[i].toString() ) );
  }
}

void QgsQueryBuilder::on_btnSampleValues_clicked()
{
  lstValues->setCursor( Qt::WaitCursor );

  QString prevSubsetString = mLayer->subsetString();
  if ( mUseUnfilteredLayer->isChecked() && !prevSubsetString.isEmpty() )
  {
    mLayer->setSubsetString( "" );
  }

  //delete connection mModelValues and lstValues
  QStandardItemModel *tmp = new QStandardItemModel();
  lstValues->setModel( tmp );
  //Clear and fill the mModelValues
  fillValues( mModelFields->data( lstFields->currentIndex(), Qt::UserRole + 1 ).toInt(), 25 );
  lstValues->setModel( mModelValues );
  //delete the tmp
  delete tmp;

  if ( prevSubsetString != mLayer->subsetString() )
  {
    mLayer->setSubsetString( prevSubsetString );
  }

  lstValues->setCursor( Qt::ArrowCursor );
}

void QgsQueryBuilder::on_btnGetAllValues_clicked()
{
  lstValues->setCursor( Qt::WaitCursor );

  QString prevSubsetString = mLayer->subsetString();
  if ( mUseUnfilteredLayer->isChecked() && !prevSubsetString.isEmpty() )
  {
    mLayer->setSubsetString( "" );
  }

  //delete connection mModelValues and lstValues
  QStandardItemModel *tmp = new QStandardItemModel();
  lstValues->setModel( tmp );
  //Clear and fill the mModelValues
  fillValues( mModelFields->data( lstFields->currentIndex(), Qt::UserRole + 1 ).toInt(), -1 );
  lstValues->setModel( mModelValues );
  //delete the tmp
  delete tmp;

  if ( prevSubsetString != mLayer->subsetString() )
  {
    mLayer->setSubsetString( prevSubsetString );
  }

  lstValues->setCursor( Qt::ArrowCursor );
}

void QgsQueryBuilder::test()
{
  // test the sql statement to see if it works
  // by counting the number of records that would be
  // returned

  if ( mLayer->setSubsetString( txtSQL->text() ) )
  {
    mUseUnfilteredLayer->setDisabled( mLayer->subsetString().isEmpty() );

    QMessageBox::information( this,
                              tr( "Query Result" ),
                              tr( "The where clause returned %n row(s).", "returned test rows", mLayer->featureCount() ) );
  }
  else if ( mLayer->dataProvider()->hasErrors() )
  {
    QMessageBox::warning( this,
                          tr( "Query Failed" ),
                          tr( "An error occurred when executing the query." )
                          + tr( "\nThe data provider said:\n%1" ).arg( mLayer->dataProvider()->errors().join( "\n" ) ) );
    mLayer->dataProvider()->clearErrors();
  }
  else
  {
    QMessageBox::warning( this,
                          tr( "Query Failed" ),
                          tr( "An error occurred when executing the query." ) );
  }
}

void QgsQueryBuilder::accept()
{
  if ( !mLayer->setSubsetString( txtSQL->text() ) )
  {
    //error in query - show the problem
    if ( mLayer->dataProvider()->hasErrors() )
    {
      QMessageBox::warning( this,
                            tr( "Query Failed" ),
                            tr( "An error occurred when executing the query." )
                            + tr( "\nThe data provider said:\n%1" ).arg( mLayer->dataProvider()->errors().join( "\n" ) ) );
      mLayer->dataProvider()->clearErrors();
    }
    else
    {
      QMessageBox::warning( this, tr( "Error in Query" ), tr( "The subset string could not be set" ) );
    }

    return;
  }

  QDialog::accept();
}

void QgsQueryBuilder::reject()
{
  if ( mLayer->subsetString() != mOrigSubsetString )
    mLayer->setSubsetString( mOrigSubsetString );

  QDialog::reject();
}

void QgsQueryBuilder::on_btnEqual_clicked()
{
  txtSQL->insertText( " = " );
  txtSQL->setFocus();
}

void QgsQueryBuilder::on_btnLessThan_clicked()
{
  txtSQL->insertText( " < " );
  txtSQL->setFocus();
}

void QgsQueryBuilder::on_btnGreaterThan_clicked()
{
  txtSQL->insertText( " > " );
  txtSQL->setFocus();
}

void QgsQueryBuilder::on_btnPct_clicked()
{
  txtSQL->insertText( "%" );
  txtSQL->setFocus();
}

void QgsQueryBuilder::on_btnIn_clicked()
{
  txtSQL->insertText( " IN " );
  txtSQL->setFocus();
}

void QgsQueryBuilder::on_btnNotIn_clicked()
{
  txtSQL->insertText( " NOT IN " );
  txtSQL->setFocus();
}

void QgsQueryBuilder::on_btnLike_clicked()
{
  txtSQL->insertText( " LIKE " );
  txtSQL->setFocus();
}

QString QgsQueryBuilder::sql()
{
  return txtSQL->text();
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
  txtSQL->insertText( "\"" + mLayer->fields()[ mModelFields->data( index, Qt::UserRole+1 ).toInt()].name() + "\"" );
  txtSQL->setFocus();
}

void QgsQueryBuilder::on_lstValues_doubleClicked( const QModelIndex &index )
{
  QVariant value = mModelValues->data( index, Qt::UserRole + 1 );
  if ( value.isNull() )
    txtSQL->insertText( "NULL" );
  else if ( value.type() == QVariant::Date && mLayer->providerType() == "ogr" && mLayer->storageType() == "ESRI Shapefile" )
    txtSQL->insertText( "'" + value.toDate().toString( "yyyy/MM/dd" ) + "'" );
  else if ( value.type() == QVariant::Int || value.type() == QVariant::Double || value.type() == QVariant::LongLong )
    txtSQL->insertText( value.toString() );
  else
    txtSQL->insertText( "'" + value.toString().replace( "'", "''" ) + "'" );

  txtSQL->setFocus();
}

void QgsQueryBuilder::on_btnLessEqual_clicked()
{
  txtSQL->insertText( " <= " );
  txtSQL->setFocus();
}

void QgsQueryBuilder::on_btnGreaterEqual_clicked()
{
  txtSQL->insertText( " >= " );
  txtSQL->setFocus();
}

void QgsQueryBuilder::on_btnNotEqual_clicked()
{
  txtSQL->insertText( " != " );
  txtSQL->setFocus();
}

void QgsQueryBuilder::on_btnAnd_clicked()
{
  txtSQL->insertText( " AND " );
  txtSQL->setFocus();
}

void QgsQueryBuilder::on_btnNot_clicked()
{
  txtSQL->insertText( " NOT " );
  txtSQL->setFocus();
}

void QgsQueryBuilder::on_btnOr_clicked()
{
  txtSQL->insertText( " OR " );
  txtSQL->setFocus();
}

void QgsQueryBuilder::clear()
{
  txtSQL->clear();
  mLayer->setSubsetString( "" );
  mUseUnfilteredLayer->setDisabled( true );
}

void QgsQueryBuilder::on_btnILike_clicked()
{
  txtSQL->insertText( " ILIKE " );
  txtSQL->setFocus();
}

void QgsQueryBuilder::setDatasourceDescription( QString uri )
{
  lblDataUri->setText( uri );
}
