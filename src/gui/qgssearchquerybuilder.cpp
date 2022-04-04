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

#include <QDomDocument>
#include <QDomElement>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QListView>
#include <QMessageBox>
#include <QStandardItem>
#include <QTextStream>

#include "qgssettings.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"
#include "qgsfields.h"
#include "qgssearchquerybuilder.h"
#include "qgsexpression.h"
#include "qgsvectorlayer.h"
#include "qgslogger.h"
#include "qgshelp.h"
#include "qgsexpressioncontextutils.h"
#include "qgsquerybuilder.h"


QgsSearchQueryBuilder::QgsSearchQueryBuilder( QgsVectorLayer *layer,
    QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
  , mLayer( layer )
{
  setupUi( this );
  connect( btnEqual, &QPushButton::clicked, this, &QgsSearchQueryBuilder::btnEqual_clicked );
  connect( btnLessThan, &QPushButton::clicked, this, &QgsSearchQueryBuilder::btnLessThan_clicked );
  connect( btnGreaterThan, &QPushButton::clicked, this, &QgsSearchQueryBuilder::btnGreaterThan_clicked );
  connect( btnLike, &QPushButton::clicked, this, &QgsSearchQueryBuilder::btnLike_clicked );
  connect( btnILike, &QPushButton::clicked, this, &QgsSearchQueryBuilder::btnILike_clicked );
  connect( btnPct, &QPushButton::clicked, this, &QgsSearchQueryBuilder::btnPct_clicked );
  connect( btnIn, &QPushButton::clicked, this, &QgsSearchQueryBuilder::btnIn_clicked );
  connect( btnNotIn, &QPushButton::clicked, this, &QgsSearchQueryBuilder::btnNotIn_clicked );
  connect( lstFields, &QListView::doubleClicked, this, &QgsSearchQueryBuilder::lstFields_doubleClicked );
  connect( lstValues, &QListView::doubleClicked, this, &QgsSearchQueryBuilder::lstValues_doubleClicked );
  connect( btnLessEqual, &QPushButton::clicked, this, &QgsSearchQueryBuilder::btnLessEqual_clicked );
  connect( btnGreaterEqual, &QPushButton::clicked, this, &QgsSearchQueryBuilder::btnGreaterEqual_clicked );
  connect( btnNotEqual, &QPushButton::clicked, this, &QgsSearchQueryBuilder::btnNotEqual_clicked );
  connect( btnAnd, &QPushButton::clicked, this, &QgsSearchQueryBuilder::btnAnd_clicked );
  connect( btnNot, &QPushButton::clicked, this, &QgsSearchQueryBuilder::btnNot_clicked );
  connect( btnOr, &QPushButton::clicked, this, &QgsSearchQueryBuilder::btnOr_clicked );
  connect( btnGetAllValues, &QPushButton::clicked, this, &QgsSearchQueryBuilder::btnGetAllValues_clicked );
  connect( btnSampleValues, &QPushButton::clicked, this, &QgsSearchQueryBuilder::btnSampleValues_clicked );
  setupListViews();
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsSearchQueryBuilder::showHelp );

  setWindowTitle( tr( "Search Query Builder" ) );

  QPushButton *pbn = new QPushButton( tr( "&Test" ) );
  buttonBox->addButton( pbn, QDialogButtonBox::ActionRole );
  connect( pbn, &QAbstractButton::clicked, this, &QgsSearchQueryBuilder::btnTest_clicked );

  pbn = new QPushButton( tr( "&Clear" ) );
  buttonBox->addButton( pbn, QDialogButtonBox::ActionRole );
  connect( pbn, &QAbstractButton::clicked, this, &QgsSearchQueryBuilder::btnClear_clicked );

  pbn = new QPushButton( tr( "&Save…" ) );
  buttonBox->addButton( pbn, QDialogButtonBox::ActionRole );
  pbn->setToolTip( tr( "Save query to an xml file" ) );
  connect( pbn, &QAbstractButton::clicked, this, &QgsSearchQueryBuilder::saveQuery );

  pbn = new QPushButton( tr( "&Load…" ) );
  buttonBox->addButton( pbn, QDialogButtonBox::ActionRole );
  pbn->setToolTip( tr( "Load query from xml file" ) );
  connect( pbn, &QAbstractButton::clicked, this, &QgsSearchQueryBuilder::loadQuery );

  if ( layer )
    lblDataUri->setText( layer->name() );
  populateFields();
}

void QgsSearchQueryBuilder::populateFields()
{
  if ( !mLayer )
    return;

  const QgsFields &fields = mLayer->fields();
  for ( int idx = 0; idx < fields.count(); ++idx )
  {
    const QString fieldName = fields.at( idx ).name();
    mFieldMap[fieldName] = idx;
    QStandardItem *myItem = new QStandardItem( fieldName );
    myItem->setEditable( false );
    mModelFields->insertRow( mModelFields->rowCount(), myItem );
  }
}

void QgsSearchQueryBuilder::setupListViews()
{
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
  const QString fieldName = mModelFields->data( lstFields->currentIndex() ).toString();
  const int fieldIndex = mFieldMap[fieldName];
  const QgsField field = mLayer->fields().at( fieldIndex );//provider->fields().at( fieldIndex );
  const bool numeric = ( field.type() == QVariant::Int || field.type() == QVariant::Double );

  QgsFeature feat;
  QString value;

  QgsAttributeList attrs;
  attrs.append( fieldIndex );

  QgsFeatureIterator fit = mLayer->getFeatures( QgsFeatureRequest().setFlags( QgsFeatureRequest::NoGeometry ).setSubsetOfAttributes( attrs ) );

  lstValues->setCursor( Qt::WaitCursor );
  // Block for better performance
  mModelValues->blockSignals( true );
  lstValues->setUpdatesEnabled( false );

  // MH: keep already inserted values in a set. Querying is much faster compared to QStandardItemModel::findItems
  QSet<QString> insertedValues;

  while ( fit.nextFeature( feat ) &&
          ( limit == 0 || mModelValues->rowCount() != limit ) )
  {
    value = feat.attribute( fieldIndex ).toString();

    if ( !numeric )
    {
      // put string in single quotes and escape single quotes in the string
      value = '\'' + value.replace( '\'', QLatin1String( "''" ) ) + '\'';
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

void QgsSearchQueryBuilder::btnSampleValues_clicked()
{
  getFieldValues( 25 );
}

void QgsSearchQueryBuilder::btnGetAllValues_clicked()
{
  getFieldValues( 0 );
}

void QgsSearchQueryBuilder::btnTest_clicked()
{
  const long count = countRecords( mTxtSql->text() );

  // error?
  if ( count == -1 )
    return;

  QMessageBox::information( this, tr( "Test Query" ), tr( "Found %n matching feature(s).", "test result", count ) );
}

// This method tests the number of records that would be returned
long QgsSearchQueryBuilder::countRecords( const QString &searchString )
{
  QgsExpression search( searchString );
  if ( search.hasParserError() )
  {
    QMessageBox::critical( this, tr( "Query Result" ), search.parserErrorString() );
    return -1;
  }

  if ( !mLayer )
    return -1;

  const bool fetchGeom = search.needsGeometry();

  int count = 0;
  QgsFeature feat;

  QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( mLayer ) );

  if ( !search.prepare( &context ) )
  {
    QMessageBox::critical( this, tr( "Query Result" ), search.evalErrorString() );
    return -1;
  }

  QApplication::setOverrideCursor( Qt::WaitCursor );

  QgsFeatureIterator fit = mLayer->getFeatures( QgsFeatureRequest().setFlags( fetchGeom ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry ) );

  while ( fit.nextFeature( feat ) )
  {
    context.setFeature( feat );
    const QVariant value = search.evaluate( &context );
    if ( value.toInt() != 0 )
    {
      count++;
    }

    // check if there were errors during evaluating
    if ( search.hasEvalError() )
      break;
  }

  QApplication::restoreOverrideCursor();

  if ( search.hasEvalError() )
  {
    QMessageBox::critical( this, tr( "Query Result" ), search.evalErrorString() );
    return -1;
  }

  return count;
}


void QgsSearchQueryBuilder::btnOk_clicked()
{
  // if user hits OK and there is no query, skip the validation
  if ( mTxtSql->text().trimmed().length() > 0 )
  {
    accept();
    return;
  }

  // test the query to see if it will result in a valid layer
  const long numRecs = countRecords( mTxtSql->text() );
  if ( numRecs == -1 )
  {
    // error shown in countRecords
  }
  else if ( numRecs == 0 )
  {
    QMessageBox::warning( this, tr( "Query Result" ), tr( "The query you specified results in zero records being returned." ) );
  }
  else
  {
    accept();
  }

}

void QgsSearchQueryBuilder::btnEqual_clicked()
{
  mTxtSql->insertText( QStringLiteral( " = " ) );
}

void QgsSearchQueryBuilder::btnLessThan_clicked()
{
  mTxtSql->insertText( QStringLiteral( " < " ) );
}

void QgsSearchQueryBuilder::btnGreaterThan_clicked()
{
  mTxtSql->insertText( QStringLiteral( " > " ) );
}

void QgsSearchQueryBuilder::btnPct_clicked()
{
  mTxtSql->insertText( QStringLiteral( "%" ) );
}

void QgsSearchQueryBuilder::btnIn_clicked()
{
  mTxtSql->insertText( QStringLiteral( " IN " ) );
}

void QgsSearchQueryBuilder::btnNotIn_clicked()
{
  mTxtSql->insertText( QStringLiteral( " NOT IN " ) );
}

void QgsSearchQueryBuilder::btnLike_clicked()
{
  mTxtSql->insertText( QStringLiteral( " LIKE " ) );
}

QString QgsSearchQueryBuilder::searchString()
{
  return mTxtSql->text();
}

void QgsSearchQueryBuilder::setSearchString( const QString &searchString )
{
  mTxtSql->setText( searchString );
}

void QgsSearchQueryBuilder::lstFields_doubleClicked( const QModelIndex &index )
{
  mTxtSql->insertText( QgsExpression::quotedColumnRef( mModelFields->data( index ).toString() ) );
}

void QgsSearchQueryBuilder::lstValues_doubleClicked( const QModelIndex &index )
{
  mTxtSql->insertText( mModelValues->data( index ).toString() );
}

void QgsSearchQueryBuilder::btnLessEqual_clicked()
{
  mTxtSql->insertText( QStringLiteral( " <= " ) );
}

void QgsSearchQueryBuilder::btnGreaterEqual_clicked()
{
  mTxtSql->insertText( QStringLiteral( " >= " ) );
}

void QgsSearchQueryBuilder::btnNotEqual_clicked()
{
  mTxtSql->insertText( QStringLiteral( " != " ) );
}

void QgsSearchQueryBuilder::btnAnd_clicked()
{
  mTxtSql->insertText( QStringLiteral( " AND " ) );
}

void QgsSearchQueryBuilder::btnNot_clicked()
{
  mTxtSql->insertText( QStringLiteral( " NOT " ) );
}

void QgsSearchQueryBuilder::btnOr_clicked()
{
  mTxtSql->insertText( QStringLiteral( " OR " ) );
}

void QgsSearchQueryBuilder::btnClear_clicked()
{
  mTxtSql->clear();
}

void QgsSearchQueryBuilder::btnILike_clicked()
{
  mTxtSql->insertText( QStringLiteral( " ILIKE " ) );
}

void QgsSearchQueryBuilder::saveQuery()
{
  QgsQueryBuilder::saveQueryToFile( mTxtSql->text() );
}

void QgsSearchQueryBuilder::loadQuery()
{
  QString query;
  if ( QgsQueryBuilder::loadQueryFromFile( query ) )
  {
    mTxtSql->clear();
    mTxtSql->insertText( query );
  }
}

void QgsSearchQueryBuilder::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_vector/vector_properties.html#query-builder" ) );
}
