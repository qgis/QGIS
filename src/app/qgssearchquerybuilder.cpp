/***************************************************************************
    qgssearchquerybuilder.cpp  - Query builder for search strings
    ----------------------
    begin                : March 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include <QListView>
#include <QMessageBox>
#include <QStandardItem>
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgssearchquerybuilder.h"
#include "qgssearchstring.h"
#include "qgssearchtreenode.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgslogger.h"

QgsSearchQueryBuilder::QgsSearchQueryBuilder( QgsVectorLayer* layer,
    QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl ), mLayer( layer )
{
  setupUi( this );
  setupListViews();

  setWindowTitle( tr( "Search query builder" ) );

  QPushButton *pbn = new QPushButton( tr( "&Test" ) );
  buttonBox->addButton( pbn, QDialogButtonBox::ActionRole );
  connect( pbn, SIGNAL( clicked() ), this, SLOT( on_btnTest_clicked() ) );

  pbn = new QPushButton( tr( "&Clear" ) );
  buttonBox->addButton( pbn, QDialogButtonBox::ActionRole );
  connect( pbn, SIGNAL( clicked() ), this, SLOT( on_btnClear_clicked() ) );

  // disable unsupported operators
  btnIn->setHidden( true );
  btnNotIn->setHidden( true );
  btnPct->setHidden( true );

  // change to ~
  btnILike->setText( "~" );

  lblDataUri->setText( layer->name() );
  populateFields();
}

QgsSearchQueryBuilder::~QgsSearchQueryBuilder()
{
}


void QgsSearchQueryBuilder::populateFields()
{
  QgsDebugMsg( "entering." );
  const QgsFieldMap& fields = mLayer->pendingFields();
  for ( QgsFieldMap::const_iterator it = fields.begin(); it != fields.end(); ++it )
  {
    QString fieldName = it->name();
    mFieldMap[fieldName] = it.key();
    QStandardItem *myItem = new QStandardItem( fieldName );
    myItem->setEditable( false );
    mModelFields->insertRow( mModelFields->rowCount(), myItem );
  }
}

void QgsSearchQueryBuilder::setupListViews()
{
  QgsDebugMsg( "entering." );
  //Models
  mModelFields = new QStandardItemModel();
  mModelValues = new QStandardItemModel();
  lstFields->setModel( mModelFields );
  lstValues->setModel( mModelValues );
  // Modes
  lstFields->setViewMode( QListView::ListMode );
  lstValues->setViewMode( QListView::ListMode );
  lstFields->setSelectionBehavior( QAbstractItemView::SelectRows );
  lstValues->setSelectionBehavior( QAbstractItemView::SelectRows );
  // Performance tip since Qt 4.1
  lstFields->setUniformItemSizes( true );
  lstValues->setUniformItemSizes( true );
}

void QgsSearchQueryBuilder::getFieldValues( int limit )
{
  if ( !mLayer )
  {
    return;
  }
  // clear the values list
  mModelValues->clear();

  // determine the field type
  QString fieldName = mModelFields->data( lstFields->currentIndex() ).toString();
  int fieldIndex = mFieldMap[fieldName];
  QgsField field = mLayer->pendingFields()[fieldIndex];//provider->fields()[fieldIndex];
  bool numeric = ( field.type() == QVariant::Int || field.type() == QVariant::Double );

  QgsFeature feat;
  QString value;

  QgsAttributeList attrs;
  attrs.append( fieldIndex );

  mLayer->select( attrs, QgsRectangle(), false );

  lstValues->setCursor( Qt::WaitCursor );
  // Block for better performance
  mModelValues->blockSignals( true );
  lstValues->setUpdatesEnabled( false );

  /**MH: keep already inserted values in a set. Querying is much faster compared to QStandardItemModel::findItems*/
  QSet<QString> insertedValues;

  while ( mLayer->nextFeature( feat ) &&
          ( limit == 0 || mModelValues->rowCount() != limit ) )
  {
    const QgsAttributeMap& attributes = feat.attributeMap();
    value = attributes[fieldIndex].toString();

    if ( !numeric )
    {
      // put string in single quotes
      value = "'" + value + "'";
    }

    // add item only if it's not there already
    if ( !insertedValues.contains( value ) )
    {
      QStandardItem *myItem = new QStandardItem( value );
      myItem->setEditable( false );
      mModelValues->insertRow( mModelValues->rowCount(), myItem );
      insertedValues.insert( value );
    }
  }
  // Unblock for normal use
  mModelValues->blockSignals( false );
  lstValues->setUpdatesEnabled( true );
  // TODO: already sorted, signal emit to refresh model
  mModelValues->sort( 0 );
  lstValues->setCursor( Qt::ArrowCursor );
}

void QgsSearchQueryBuilder::on_btnSampleValues_clicked()
{
  getFieldValues( 25 );
}

void QgsSearchQueryBuilder::on_btnGetAllValues_clicked()
{
  getFieldValues( 0 );
}

void QgsSearchQueryBuilder::on_btnTest_clicked()
{
  long count = countRecords( txtSQL->toPlainText() );

  // error?
  if ( count == -1 )
    return;

  QMessageBox::information( this, tr( "Search results" ), tr( "Found %n matching feature(s).", "test result", count ) );
}

// This method tests the number of records that would be returned
long QgsSearchQueryBuilder::countRecords( QString searchString )
{
  QgsSearchString search;
  if ( !search.setString( searchString ) )
  {
    QMessageBox::critical( this, tr( "Search string parsing error" ), search.parserErrorMsg() );
    return -1;
  }

  QgsSearchTreeNode* searchTree = search.tree();
  if ( searchTree == NULL )
  {
    // entered empty search string
    return mLayer->featureCount();
  }

  bool fetchGeom = searchTree->needsGeometry();

  QApplication::setOverrideCursor( Qt::WaitCursor );

  int count = 0;
  QgsFeature feat;
  QgsVectorDataProvider* provider = mLayer->dataProvider();
  const QgsFieldMap& fields = provider->fields();
  QgsAttributeList allAttributes = provider->attributeIndexes();

  provider->select( allAttributes, QgsRectangle(), fetchGeom );

  while ( provider->nextFeature( feat ) )
  {
    if ( searchTree->checkAgainst( fields, feat.attributeMap(), feat.geometry() ) )
    {
      count++;
    }

    // check if there were errors during evaulating
    if ( searchTree->hasError() )
      break;
  }

  QApplication::restoreOverrideCursor();

  return count;
}


void QgsSearchQueryBuilder::on_btnOk_clicked()
{
  // if user hits Ok and there is no query, skip the validation
  if ( txtSQL->toPlainText().trimmed().length() > 0 )
  {
    accept();
    return;
  }

  // test the query to see if it will result in a valid layer
  long numRecs = countRecords( txtSQL->toPlainText() );
  if ( numRecs == -1 )
  {
    // error shown in countRecords
  }
  else if ( numRecs == 0 )
  {
    QMessageBox::warning( this, tr( "No Records" ), tr( "The query you specified results in zero records being returned." ) );
  }
  else
  {
    accept();
  }

}

void QgsSearchQueryBuilder::on_btnEqual_clicked()
{
  txtSQL->insertPlainText( " = " );
}

void QgsSearchQueryBuilder::on_btnLessThan_clicked()
{
  txtSQL->insertPlainText( " < " );
}

void QgsSearchQueryBuilder::on_btnGreaterThan_clicked()
{
  txtSQL->insertPlainText( " > " );
}

void QgsSearchQueryBuilder::on_btnLike_clicked()
{
  txtSQL->insertPlainText( " LIKE " );
}

QString QgsSearchQueryBuilder::searchString()
{
  return txtSQL->toPlainText();
}

void QgsSearchQueryBuilder::setSearchString( QString searchString )
{
  txtSQL->setPlainText( searchString );
}

void QgsSearchQueryBuilder::on_lstFields_doubleClicked( const QModelIndex &index )
{
  txtSQL->insertPlainText( mModelFields->data( index ).toString() );
}

void QgsSearchQueryBuilder::on_lstValues_doubleClicked( const QModelIndex &index )
{
  txtSQL->insertPlainText( mModelValues->data( index ).toString() );
}

void QgsSearchQueryBuilder::on_btnLessEqual_clicked()
{
  txtSQL->insertPlainText( " <= " );
}

void QgsSearchQueryBuilder::on_btnGreaterEqual_clicked()
{
  txtSQL->insertPlainText( " >= " );
}

void QgsSearchQueryBuilder::on_btnNotEqual_clicked()
{
  txtSQL->insertPlainText( " != " );
}

void QgsSearchQueryBuilder::on_btnAnd_clicked()
{
  txtSQL->insertPlainText( " AND " );
}

void QgsSearchQueryBuilder::on_btnNot_clicked()
{
  txtSQL->insertPlainText( " NOT " );
}

void QgsSearchQueryBuilder::on_btnOr_clicked()
{
  txtSQL->insertPlainText( " OR " );
}

void QgsSearchQueryBuilder::on_btnClear_clicked()
{
  txtSQL->clear();
}

void QgsSearchQueryBuilder::on_btnILike_clicked()
{
  //txtSQL->insertPlainText(" ILIKE ");
  txtSQL->insertPlainText( " ~ " );
}

