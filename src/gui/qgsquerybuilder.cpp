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
#include "qgssettings.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"

#include <QListView>
#include <QMessageBox>
#include <QRegExp>
#include <QPushButton>

// constructor used when the query builder must make its own
// connection to the database
QgsQueryBuilder::QgsQueryBuilder( QgsVectorLayer *layer,
                                  QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
  , mPreviousFieldRow( -1 )
  , mLayer( layer )
{
  setupUi( this );
  connect( btnEqual, &QPushButton::clicked, this, &QgsQueryBuilder::btnEqual_clicked );
  connect( btnLessThan, &QPushButton::clicked, this, &QgsQueryBuilder::btnLessThan_clicked );
  connect( btnGreaterThan, &QPushButton::clicked, this, &QgsQueryBuilder::btnGreaterThan_clicked );
  connect( btnPct, &QPushButton::clicked, this, &QgsQueryBuilder::btnPct_clicked );
  connect( btnIn, &QPushButton::clicked, this, &QgsQueryBuilder::btnIn_clicked );
  connect( btnNotIn, &QPushButton::clicked, this, &QgsQueryBuilder::btnNotIn_clicked );
  connect( btnLike, &QPushButton::clicked, this, &QgsQueryBuilder::btnLike_clicked );
  connect( btnILike, &QPushButton::clicked, this, &QgsQueryBuilder::btnILike_clicked );
  connect( lstFields, &QListView::clicked, this, &QgsQueryBuilder::lstFields_clicked );
  connect( lstFields, &QListView::doubleClicked, this, &QgsQueryBuilder::lstFields_doubleClicked );
  connect( lstValues, &QListView::doubleClicked, this, &QgsQueryBuilder::lstValues_doubleClicked );
  connect( btnLessEqual, &QPushButton::clicked, this, &QgsQueryBuilder::btnLessEqual_clicked );
  connect( btnGreaterEqual, &QPushButton::clicked, this, &QgsQueryBuilder::btnGreaterEqual_clicked );
  connect( btnNotEqual, &QPushButton::clicked, this, &QgsQueryBuilder::btnNotEqual_clicked );
  connect( btnAnd, &QPushButton::clicked, this, &QgsQueryBuilder::btnAnd_clicked );
  connect( btnNot, &QPushButton::clicked, this, &QgsQueryBuilder::btnNot_clicked );
  connect( btnOr, &QPushButton::clicked, this, &QgsQueryBuilder::btnOr_clicked );
  connect( btnGetAllValues, &QPushButton::clicked, this, &QgsQueryBuilder::btnGetAllValues_clicked );
  connect( btnSampleValues, &QPushButton::clicked, this, &QgsQueryBuilder::btnSampleValues_clicked );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsQueryBuilder::showHelp );

  QgsSettings settings;
  restoreGeometry( settings.value( QStringLiteral( "Windows/QueryBuilder/geometry" ) ).toByteArray() );

  QPushButton *pbn = new QPushButton( tr( "&Test" ) );
  buttonBox->addButton( pbn, QDialogButtonBox::ActionRole );
  connect( pbn, &QAbstractButton::clicked, this, &QgsQueryBuilder::test );

  pbn = new QPushButton( tr( "&Clear" ) );
  buttonBox->addButton( pbn, QDialogButtonBox::ActionRole );
  connect( pbn, &QAbstractButton::clicked, this, &QgsQueryBuilder::clear );

  setupGuiViews();

  mOrigSubsetString = layer->subsetString();

  mUseUnfilteredLayer->setDisabled( mLayer->subsetString().isEmpty() );

  lblDataUri->setText( tr( "Set provider filter on %1" ).arg( layer->name() ) );
  txtSQL->setText( mOrigSubsetString );

  populateFields();
}

QgsQueryBuilder::~QgsQueryBuilder()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/QueryBuilder/geometry" ), saveGeometry() );
}

void QgsQueryBuilder::showEvent( QShowEvent *event )
{
  txtSQL->setFocus();
  QDialog::showEvent( event );
}

void QgsQueryBuilder::populateFields()
{
  const QgsFields &fields = mLayer->fields();
  for ( int idx = 0; idx < fields.count(); ++idx )
  {
    if ( fields.fieldOrigin( idx ) != QgsFields::OriginProvider )
    {
      // only consider native fields
      continue;
    }
    QStandardItem *myItem = new QStandardItem( fields.at( idx ).name() );
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
  QSet<QVariant> values = mLayer->uniqueValues( idx, limit );

  QString nullValue = QgsApplication::nullRepresentation();

  QgsDebugMsg( QString( "nullValue: %1" ).arg( nullValue ) );

  Q_FOREACH ( const QVariant &var, values )
  {
    QString value;
    if ( var.isNull() )
      value = nullValue;
    else if ( var.type() == QVariant::Date && mLayer->providerType() == QLatin1String( "ogr" ) && mLayer->storageType() == QLatin1String( "ESRI Shapefile" ) )
      value = var.toDate().toString( QStringLiteral( "yyyy/MM/dd" ) );
    else
      value = var.toString();

    QStandardItem *myItem = new QStandardItem( value );
    myItem->setEditable( false );
    myItem->setData( var, Qt::UserRole + 1 );
    mModelValues->insertRow( mModelValues->rowCount(), myItem );
    QgsDebugMsg( QString( "Value is null: %1\nvalue: %2" ).arg( var.isNull() ).arg( var.isNull() ? nullValue : var.toString() ) );
  }
  mModelValues->sort( 0 );
}

void QgsQueryBuilder::btnSampleValues_clicked()
{
  lstValues->setCursor( Qt::WaitCursor );

  QString prevSubsetString = mLayer->subsetString();
  if ( mUseUnfilteredLayer->isChecked() && !prevSubsetString.isEmpty() )
  {
    mLayer->setSubsetString( QLatin1String( "" ) );
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

void QgsQueryBuilder::btnGetAllValues_clicked()
{
  lstValues->setCursor( Qt::WaitCursor );

  QString prevSubsetString = mLayer->subsetString();
  if ( mUseUnfilteredLayer->isChecked() && !prevSubsetString.isEmpty() )
  {
    mLayer->setSubsetString( QLatin1String( "" ) );
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
                          + tr( "\nThe data provider said:\n%1" ).arg( mLayer->dataProvider()->errors().join( QStringLiteral( "\n" ) ) ) );
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
                            + tr( "\nThe data provider said:\n%1" ).arg( mLayer->dataProvider()->errors().join( QStringLiteral( "\n" ) ) ) );
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

void QgsQueryBuilder::btnEqual_clicked()
{
  txtSQL->insertText( QStringLiteral( " = " ) );
  txtSQL->setFocus();
}

void QgsQueryBuilder::btnLessThan_clicked()
{
  txtSQL->insertText( QStringLiteral( " < " ) );
  txtSQL->setFocus();
}

void QgsQueryBuilder::btnGreaterThan_clicked()
{
  txtSQL->insertText( QStringLiteral( " > " ) );
  txtSQL->setFocus();
}

void QgsQueryBuilder::btnPct_clicked()
{
  txtSQL->insertText( QStringLiteral( "%" ) );
  txtSQL->setFocus();
}

void QgsQueryBuilder::btnIn_clicked()
{
  txtSQL->insertText( QStringLiteral( " IN " ) );
  txtSQL->setFocus();
}

void QgsQueryBuilder::btnNotIn_clicked()
{
  txtSQL->insertText( QStringLiteral( " NOT IN " ) );
  txtSQL->setFocus();
}

void QgsQueryBuilder::btnLike_clicked()
{
  txtSQL->insertText( QStringLiteral( " LIKE " ) );
  txtSQL->setFocus();
}

QString QgsQueryBuilder::sql()
{
  return txtSQL->text();
}

void QgsQueryBuilder::setSql( const QString &sqlStatement )
{
  txtSQL->setText( sqlStatement );
}

void QgsQueryBuilder::lstFields_clicked( const QModelIndex &index )
{
  if ( mPreviousFieldRow != index.row() )
  {
    mPreviousFieldRow = index.row();

    btnSampleValues->setEnabled( true );
    btnGetAllValues->setEnabled( true );

    mModelValues->clear();
  }
}

void QgsQueryBuilder::lstFields_doubleClicked( const QModelIndex &index )
{
  txtSQL->insertText( '\"' + mLayer->fields().at( mModelFields->data( index, Qt::UserRole + 1 ).toInt() ).name() + '\"' );
  txtSQL->setFocus();
}

void QgsQueryBuilder::lstValues_doubleClicked( const QModelIndex &index )
{
  QVariant value = mModelValues->data( index, Qt::UserRole + 1 );
  if ( value.isNull() )
    txtSQL->insertText( QStringLiteral( "NULL" ) );
  else if ( value.type() == QVariant::Date && mLayer->providerType() == QLatin1String( "ogr" ) && mLayer->storageType() == QLatin1String( "ESRI Shapefile" ) )
    txtSQL->insertText( '\'' + value.toDate().toString( QStringLiteral( "yyyy/MM/dd" ) ) + '\'' );
  else if ( value.type() == QVariant::Int || value.type() == QVariant::Double || value.type() == QVariant::LongLong )
    txtSQL->insertText( value.toString() );
  else
    txtSQL->insertText( '\'' + value.toString().replace( '\'', QLatin1String( "''" ) ) + '\'' );

  txtSQL->setFocus();
}

void QgsQueryBuilder::btnLessEqual_clicked()
{
  txtSQL->insertText( QStringLiteral( " <= " ) );
  txtSQL->setFocus();
}

void QgsQueryBuilder::btnGreaterEqual_clicked()
{
  txtSQL->insertText( QStringLiteral( " >= " ) );
  txtSQL->setFocus();
}

void QgsQueryBuilder::btnNotEqual_clicked()
{
  txtSQL->insertText( QStringLiteral( " != " ) );
  txtSQL->setFocus();
}

void QgsQueryBuilder::btnAnd_clicked()
{
  txtSQL->insertText( QStringLiteral( " AND " ) );
  txtSQL->setFocus();
}

void QgsQueryBuilder::btnNot_clicked()
{
  txtSQL->insertText( QStringLiteral( " NOT " ) );
  txtSQL->setFocus();
}

void QgsQueryBuilder::btnOr_clicked()
{
  txtSQL->insertText( QStringLiteral( " OR " ) );
  txtSQL->setFocus();
}

void QgsQueryBuilder::clear()
{
  txtSQL->clear();
  mLayer->setSubsetString( QLatin1String( "" ) );
  mUseUnfilteredLayer->setDisabled( true );
}

void QgsQueryBuilder::btnILike_clicked()
{
  txtSQL->insertText( QStringLiteral( " ILIKE " ) );
  txtSQL->setFocus();
}

void QgsQueryBuilder::setDatasourceDescription( const QString &uri )
{
  lblDataUri->setText( uri );
}

void QgsQueryBuilder::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_vector/vector_properties.html#query-builder" ) );
}
