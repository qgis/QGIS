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
    QString fieldName = fields.at( idx ).name();
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
  QString fieldName = mModelFields->data( lstFields->currentIndex() ).toString();
  int fieldIndex = mFieldMap[fieldName];
  QgsField field = mLayer->fields().at( fieldIndex );//provider->fields().at( fieldIndex );
  bool numeric = ( field.type() == QVariant::Int || field.type() == QVariant::Double );

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
  long count = countRecords( txtSQL->text() );

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

  bool fetchGeom = search.needsGeometry();

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
    QVariant value = search.evaluate( &context );
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
  if ( txtSQL->text().trimmed().length() > 0 )
  {
    accept();
    return;
  }

  // test the query to see if it will result in a valid layer
  long numRecs = countRecords( txtSQL->text() );
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
  txtSQL->insertText( QStringLiteral( " = " ) );
}

void QgsSearchQueryBuilder::btnLessThan_clicked()
{
  txtSQL->insertText( QStringLiteral( " < " ) );
}

void QgsSearchQueryBuilder::btnGreaterThan_clicked()
{
  txtSQL->insertText( QStringLiteral( " > " ) );
}

void QgsSearchQueryBuilder::btnPct_clicked()
{
  txtSQL->insertText( QStringLiteral( "%" ) );
}

void QgsSearchQueryBuilder::btnIn_clicked()
{
  txtSQL->insertText( QStringLiteral( " IN " ) );
}

void QgsSearchQueryBuilder::btnNotIn_clicked()
{
  txtSQL->insertText( QStringLiteral( " NOT IN " ) );
}

void QgsSearchQueryBuilder::btnLike_clicked()
{
  txtSQL->insertText( QStringLiteral( " LIKE " ) );
}

QString QgsSearchQueryBuilder::searchString()
{
  return txtSQL->text();
}

void QgsSearchQueryBuilder::setSearchString( const QString &searchString )
{
  txtSQL->setText( searchString );
}

void QgsSearchQueryBuilder::lstFields_doubleClicked( const QModelIndex &index )
{
  txtSQL->insertText( QgsExpression::quotedColumnRef( mModelFields->data( index ).toString() ) );
}

void QgsSearchQueryBuilder::lstValues_doubleClicked( const QModelIndex &index )
{
  txtSQL->insertText( mModelValues->data( index ).toString() );
}

void QgsSearchQueryBuilder::btnLessEqual_clicked()
{
  txtSQL->insertText( QStringLiteral( " <= " ) );
}

void QgsSearchQueryBuilder::btnGreaterEqual_clicked()
{
  txtSQL->insertText( QStringLiteral( " >= " ) );
}

void QgsSearchQueryBuilder::btnNotEqual_clicked()
{
  txtSQL->insertText( QStringLiteral( " != " ) );
}

void QgsSearchQueryBuilder::btnAnd_clicked()
{
  txtSQL->insertText( QStringLiteral( " AND " ) );
}

void QgsSearchQueryBuilder::btnNot_clicked()
{
  txtSQL->insertText( QStringLiteral( " NOT " ) );
}

void QgsSearchQueryBuilder::btnOr_clicked()
{
  txtSQL->insertText( QStringLiteral( " OR " ) );
}

void QgsSearchQueryBuilder::btnClear_clicked()
{
  txtSQL->clear();
}

void QgsSearchQueryBuilder::btnILike_clicked()
{
  txtSQL->insertText( QStringLiteral( " ILIKE " ) );
}

void QgsSearchQueryBuilder::saveQuery()
{
  QgsSettings s;
  QString lastQueryFileDir = s.value( QStringLiteral( "/UI/lastQueryFileDir" ), QDir::homePath() ).toString();
  //save as qqt (QGIS query file)
  QString saveFileName = QFileDialog::getSaveFileName( nullptr, tr( "Save Query to File" ), lastQueryFileDir, QStringLiteral( "*.qqf" ) );
  if ( saveFileName.isNull() )
  {
    return;
  }

  if ( !saveFileName.endsWith( QLatin1String( ".qqf" ), Qt::CaseInsensitive ) )
  {
    saveFileName += QLatin1String( ".qqf" );
  }

  QFile saveFile( saveFileName );
  if ( !saveFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    QMessageBox::critical( nullptr, tr( "Save Query to File" ), tr( "Could not open file for writing." ) );
    return;
  }

  QDomDocument xmlDoc;
  QDomElement queryElem = xmlDoc.createElement( QStringLiteral( "Query" ) );
  QDomText queryTextNode = xmlDoc.createTextNode( txtSQL->text() );
  queryElem.appendChild( queryTextNode );
  xmlDoc.appendChild( queryElem );

  QTextStream fileStream( &saveFile );
  xmlDoc.save( fileStream, 2 );

  QFileInfo fi( saveFile );
  s.setValue( QStringLiteral( "/UI/lastQueryFileDir" ), fi.absolutePath() );
}

void QgsSearchQueryBuilder::loadQuery()
{
  QgsSettings s;
  QString lastQueryFileDir = s.value( QStringLiteral( "/UI/lastQueryFileDir" ), QDir::homePath() ).toString();

  QString queryFileName = QFileDialog::getOpenFileName( nullptr, tr( "Load Query from File" ), lastQueryFileDir, tr( "Query files" ) + " (*.qqf);;" + tr( "All files" ) + " (*)" );
  if ( queryFileName.isNull() )
  {
    return;
  }

  QFile queryFile( queryFileName );
  if ( !queryFile.open( QIODevice::ReadOnly ) )
  {
    QMessageBox::critical( nullptr, tr( "Load Query from File" ), tr( "Could not open file for reading." ) );
    return;
  }
  QDomDocument queryDoc;
  if ( !queryDoc.setContent( &queryFile ) )
  {
    QMessageBox::critical( nullptr, tr( "Load Query from File" ), tr( "File is not a valid xml document." ) );
    return;
  }

  QDomElement queryElem = queryDoc.firstChildElement( QStringLiteral( "Query" ) );
  if ( queryElem.isNull() )
  {
    QMessageBox::critical( nullptr, tr( "Load Query from File" ), tr( "File is not a valid query document." ) );
    return;
  }

  QString query = queryElem.text();

  //todo: test if all the attributes are valid
  QgsExpression search( query );
  if ( search.hasParserError() )
  {
    QMessageBox::critical( this, tr( "Query Result" ), search.parserErrorString() );
    return;
  }

  QString newQueryText = query;

#if 0
  // TODO: implement with visitor pattern in QgsExpression

  QStringList attributes = searchTree->referencedColumns();
  QMap< QString, QString> attributesToReplace;
  QStringList existingAttributes;

  //get all existing fields
  QMap<QString, int>::const_iterator fieldIt = mFieldMap.constBegin();
  for ( ; fieldIt != mFieldMap.constEnd(); ++fieldIt )
  {
    existingAttributes.push_back( fieldIt.key() );
  }

  //if a field does not exist, ask what field should be used instead
  QStringList::const_iterator attIt = attributes.constBegin();
  for ( ; attIt != attributes.constEnd(); ++attIt )
  {
    //test if attribute is there
    if ( !mFieldMap.contains( attIt ) )
    {
      bool ok;
      QString replaceAttribute = QInputDialog::getItem( 0, tr( "Select Attribute" ), tr( "There is no attribute '%1' in the current vector layer. Please select an existing attribute." ).arg( *attIt ),
                                 existingAttributes, 0, false, &ok );
      if ( !ok || replaceAttribute.isEmpty() )
      {
        return;
      }
      attributesToReplace.insert( *attIt, replaceAttribute );
    }
  }

  //Now replace all the string in the query
  QList<QgsSearchTreeNode *> columnRefList = searchTree->columnRefNodes();
  QList<QgsSearchTreeNode *>::iterator columnIt = columnRefList.begin();
  for ( ; columnIt != columnRefList.end(); ++columnIt )
  {
    QMap< QString, QString>::const_iterator replaceIt = attributesToReplace.find( ( *columnIt )->columnRef() );
    if ( replaceIt != attributesToReplace.constEnd() )
    {
      ( *columnIt )->setColumnRef( replaceIt.value() );
    }
  }

  if ( attributesToReplace.size() > 0 )
  {
    newQueryText = query;
  }
#endif

  txtSQL->clear();
  txtSQL->insertText( newQueryText );
}

void QgsSearchQueryBuilder::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_vector/vector_properties.html#query-builder" ) );
}
