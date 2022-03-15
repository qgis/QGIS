/***************************************************************************
    qgspointcloudquerybuilder.cpp  - Query builder for point cloud layers
    -----------------------------
    begin                : March 2022
    copyright            : (C) 2022 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgspointcloudquerybuilder.h"
#include "qgssettings.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudexpression.h"
#include "qgshelp.h"
#include "qgsgui.h"

#include <QDomDocument>
#include <QDomElement>
#include <QFileDialog>
#include <QInputDialog>
#include <QListView>
#include <QMessageBox>
#include <QPushButton>
#include <QTextStream>


// constructor used when the query builder must make its own
// connection to the database
QgsPointCloudQueryBuilder::QgsPointCloudQueryBuilder( QgsPointCloudLayer *layer,
    QWidget *parent, Qt::WindowFlags fl )
  : QgsSubsetStringEditorInterface( parent, fl )
  , mLayer( layer )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  setupGuiViews();
  populateAttributes();

  connect( lstAttributes->selectionModel(), &QItemSelectionModel::currentChanged, this, &QgsPointCloudQueryBuilder::lstAttributes_currentChanged );
  connect( lstAttributes, &QListView::doubleClicked, this, &QgsPointCloudQueryBuilder::lstAttributes_doubleClicked );
  connect( lstValues, &QListView::doubleClicked, this, &QgsPointCloudQueryBuilder::lstValues_doubleClicked );
  connect( btnEqual, &QPushButton::clicked, this, &QgsPointCloudQueryBuilder::btnEqual_clicked );
  connect( btnLessThan, &QPushButton::clicked, this, &QgsPointCloudQueryBuilder::btnLessThan_clicked );
  connect( btnGreaterThan, &QPushButton::clicked, this, &QgsPointCloudQueryBuilder::btnGreaterThan_clicked );
  connect( btnIn, &QPushButton::clicked, this, &QgsPointCloudQueryBuilder::btnIn_clicked );
  connect( btnNotIn, &QPushButton::clicked, this, &QgsPointCloudQueryBuilder::btnNotIn_clicked );
  connect( btnLessEqual, &QPushButton::clicked, this, &QgsPointCloudQueryBuilder::btnLessEqual_clicked );
  connect( btnGreaterEqual, &QPushButton::clicked, this, &QgsPointCloudQueryBuilder::btnGreaterEqual_clicked );
  connect( btnNotEqual, &QPushButton::clicked, this, &QgsPointCloudQueryBuilder::btnNotEqual_clicked );
  connect( btnAnd, &QPushButton::clicked, this, &QgsPointCloudQueryBuilder::btnAnd_clicked );
  connect( btnOr, &QPushButton::clicked, this, &QgsPointCloudQueryBuilder::btnOr_clicked );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsPointCloudQueryBuilder::showHelp );

  QPushButton *pbn = new QPushButton( tr( "&Test" ) );
  buttonBox->addButton( pbn, QDialogButtonBox::ActionRole );
  connect( pbn, &QAbstractButton::clicked, this, &QgsPointCloudQueryBuilder::test );

  pbn = new QPushButton( tr( "&Clear" ) );
  buttonBox->addButton( pbn, QDialogButtonBox::ActionRole );
  connect( pbn, &QAbstractButton::clicked, this, &QgsPointCloudQueryBuilder::clear );

  pbn = new QPushButton( tr( "&Save…" ) );
  buttonBox->addButton( pbn, QDialogButtonBox::ActionRole );
  pbn->setToolTip( tr( "Save query to QQF file" ) );
  connect( pbn, &QAbstractButton::clicked, this, &QgsPointCloudQueryBuilder::saveQuery );

  pbn = new QPushButton( tr( "&Load…" ) );
  buttonBox->addButton( pbn, QDialogButtonBox::ActionRole );
  pbn->setToolTip( tr( "Load query from QQF file" ) );
  connect( pbn, &QAbstractButton::clicked, this, &QgsPointCloudQueryBuilder::loadQuery );

  mOrigSubsetString = layer->subsetString();

  lblDataUri->setText( tr( "Set provider filter on %1" ).arg( layer->name() ) );
  mTxtSql->setText( mOrigSubsetString );
}

void QgsPointCloudQueryBuilder::showEvent( QShowEvent *event )
{
  mTxtSql->setFocus();
  QDialog::showEvent( event );
}


void QgsPointCloudQueryBuilder::setupGuiViews()
{
  //Initialize the models
  mModelAttributes = new QStandardItemModel();
  mModelValues = new QStandardItemModel();

  // Modes
  lstAttributes->setViewMode( QListView::ListMode );
  lstValues->setViewMode( QListView::ListMode );
  lstAttributes->setSelectionBehavior( QAbstractItemView::SelectRows );
  lstValues->setSelectionBehavior( QAbstractItemView::SelectRows );
  lstAttributes->setEditTriggers( QAbstractItemView::NoEditTriggers );
  lstValues->setEditTriggers( QAbstractItemView::NoEditTriggers );
  // Performance tip since Qt 4.1
  lstAttributes->setUniformItemSizes( true );
  lstValues->setUniformItemSizes( true );
  // Colored rows
  lstAttributes->setAlternatingRowColors( true );
  lstValues->setAlternatingRowColors( true );

  lstAttributes->setModel( mModelAttributes );
  lstValues->setModel( mModelValues );
}

void QgsPointCloudQueryBuilder::populateAttributes()
{
  const QgsFields &fields = mLayer->dataProvider()->index()->attributes().toFields();
  mTxtSql->setFields( fields );
  for ( int idx = 0; idx < fields.count(); ++idx )
  {
    QStandardItem *myItem = new QStandardItem( fields.at( idx ).displayNameWithAlias() );
    mModelAttributes->insertRow( mModelAttributes->rowCount(), myItem );
  }
}

void QgsPointCloudQueryBuilder::lstAttributes_currentChanged( const QModelIndex &current, const QModelIndex &previous )
{
  Q_UNUSED( previous )

  mModelValues->clear();
  const QString attribute = current.data().toString();
  if ( attribute.compare( QLatin1String( "Classification" ), Qt::CaseInsensitive ) == 0 )
  {
    const QMap<int, QString> codes = QgsPointCloudDataProvider::translatedLasClassificationCodes();
    for ( int i = 0; i <= 18; ++i )
    {
      QStandardItem *item = new QStandardItem( QString( "%1: %2" ).arg( i ).arg( codes.value( i ) ) );
      item->setData( i, Qt::UserRole );
      mModelValues->insertRow( mModelValues->rowCount(), item );
    }
  }
  else
  {
    QVariant minimum = mLayer->dataProvider()->metadataStatistic( attribute, QgsStatisticalSummary::Min );
    QVariant maximum = mLayer->dataProvider()->metadataStatistic( attribute, QgsStatisticalSummary::Max );
    QVariant mean = mLayer->dataProvider()->metadataStatistic( attribute, QgsStatisticalSummary::Mean );
    QVariant stddev = mLayer->dataProvider()->metadataStatistic( attribute, QgsStatisticalSummary::StDev );

    QStandardItem *item1 = new QStandardItem( QLatin1String( "Minimum: %1" ).arg( minimum.toString() ) );
    item1->setData( minimum, Qt::UserRole );
    mModelValues->insertRow( mModelValues->rowCount(), item1 );
    QStandardItem *tem2 = new QStandardItem( QLatin1String( "Maximum: %1" ).arg( maximum.toString() ) );
    tem2->setData( maximum, Qt::UserRole );
    mModelValues->insertRow( mModelValues->rowCount(), tem2 );
    QStandardItem *item3 = new QStandardItem( QLatin1String( "Mean: %1" ).arg( mean.toString() ) );
    item3->setData( mean, Qt::UserRole );
    mModelValues->insertRow( mModelValues->rowCount(), item3 );
    QStandardItem *item4 = new QStandardItem( QLatin1String( "StdDev: %1" ).arg( stddev.toString() ) );
    item4->setData( stddev, Qt::UserRole );
    mModelValues->insertRow( mModelValues->rowCount(), item4 );
  }
}

void QgsPointCloudQueryBuilder::lstAttributes_doubleClicked( const QModelIndex &index )
{
  mTxtSql->insertText( QLatin1String( "%1 " ).arg( mModelAttributes->data( index ).toString() ) );
  mTxtSql->setFocus();
}

void QgsPointCloudQueryBuilder::lstValues_doubleClicked( const QModelIndex &index )
{
  mTxtSql->insertText( QLatin1String( "%1 " ).arg( mModelValues->data( index, Qt::UserRole ).toString() ) );
  mTxtSql->setFocus();
}

void QgsPointCloudQueryBuilder::btnEqual_clicked()
{
  mTxtSql->insertText( QStringLiteral( "= " ) );
  mTxtSql->setFocus();
}

void QgsPointCloudQueryBuilder::btnLessThan_clicked()
{
  mTxtSql->insertText( QStringLiteral( "< " ) );
  mTxtSql->setFocus();
}

void QgsPointCloudQueryBuilder::btnGreaterThan_clicked()
{
  mTxtSql->insertText( QStringLiteral( "> " ) );
  mTxtSql->setFocus();
}

void QgsPointCloudQueryBuilder::btnIn_clicked()
{
  mTxtSql->insertText( QStringLiteral( "IN () " ) );
  int i, j;
  mTxtSql->getCursorPosition( &i, &j );
  mTxtSql->setCursorPosition( i, j - 2 );
  mTxtSql->setFocus();
}

void QgsPointCloudQueryBuilder::btnNotIn_clicked()
{
  mTxtSql->insertText( QStringLiteral( "NOT IN () " ) );
  int i, j;
  mTxtSql->getCursorPosition( &i, &j );
  mTxtSql->setCursorPosition( i, j - 2 );
  mTxtSql->setFocus();
}

void QgsPointCloudQueryBuilder::btnLessEqual_clicked()
{
  mTxtSql->insertText( QStringLiteral( "<= " ) );
  mTxtSql->setFocus();
}

void QgsPointCloudQueryBuilder::btnGreaterEqual_clicked()
{
  mTxtSql->insertText( QStringLiteral( ">= " ) );
  mTxtSql->setFocus();
}

void QgsPointCloudQueryBuilder::btnNotEqual_clicked()
{
  mTxtSql->insertText( QStringLiteral( "!= " ) );
  mTxtSql->setFocus();
}

void QgsPointCloudQueryBuilder::btnAnd_clicked()
{
  mTxtSql->insertText( QStringLiteral( "AND " ) );
  mTxtSql->setFocus();
}

void QgsPointCloudQueryBuilder::btnOr_clicked()
{
  mTxtSql->insertText( QStringLiteral( "OR " ) );
  mTxtSql->setFocus();
}

void QgsPointCloudQueryBuilder::accept()
{
  if ( mTxtSql->text() != mOrigSubsetString )
  {
    if ( !mLayer->setSubsetString( mTxtSql->text() ) )
    {
      QMessageBox::warning( this, tr( "Query Result" ), tr( "Error in query. The subset string could not be set." ) );
      return;
    }
  }

  QDialog::accept();
}

void QgsPointCloudQueryBuilder::reject()
{
  if ( mLayer->subsetString() != mOrigSubsetString )
    mLayer->setSubsetString( mOrigSubsetString );

  QDialog::reject();
}

void QgsPointCloudQueryBuilder::test()
{
  QgsPointCloudExpression expression( mTxtSql->text() );
  if ( !expression.isValid() && !mTxtSql->text().isEmpty() )
  {
    QMessageBox::warning( this,
                          tr( "Query Result" ),
                          tr( "An error occurred while parsing the expression:\n%1" ).arg( expression.parserErrorString() ) );
  }
  else
  {
    const QSet<QString> attributes = expression.referencedAttributes();
    int offset;
    for ( const auto &attribute : attributes )
    {
      if ( mLayer->dataProvider() &&
           !mLayer->dataProvider()->attributes().find( attribute, offset ) )
      {
        QMessageBox::warning( this,
                              tr( "Query Result" ),
                              tr( "\"%1\" not recognized as an available attribute." ).arg( attribute ) );
        return;
      }
    }
    mLayer->setSubsetString( mTxtSql->text() );
    QMessageBox::information( this,
                              tr( "Query Result" ),
                              tr( "The expression was successfully parsed." ) );
  }
}

void QgsPointCloudQueryBuilder::clear()
{
  mTxtSql->clear();
  mLayer->setSubsetString( QString() );
}

void QgsPointCloudQueryBuilder::showHelp()
{
  // TODO: No pointcloud help page yet
  //QgsHelp::openHelp( QStringLiteral( "working_with_vector/vector_properties.html#query-builder" ) );
}

void QgsPointCloudQueryBuilder::saveQuery()
{
  QgsSettings s;
  const QString lastQueryFileDir = s.value( QStringLiteral( "/UI/lastQueryFileDir" ), QDir::homePath() ).toString();
  //save as qqf (QGIS query file)
  QString saveFileName = QFileDialog::getSaveFileName( nullptr, tr( "Save Query to File" ), lastQueryFileDir, tr( "Query files (*.qqf *.QQF)" ) );
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
  const QDomText queryTextNode = xmlDoc.createTextNode( mTxtSql->text() );
  queryElem.appendChild( queryTextNode );
  xmlDoc.appendChild( queryElem );

  QTextStream fileStream( &saveFile );
  xmlDoc.save( fileStream, 2 );

  const QFileInfo fi( saveFile );
  s.setValue( QStringLiteral( "/UI/lastQueryFileDir" ), fi.absolutePath() );
}

void QgsPointCloudQueryBuilder::loadQuery()
{
  const QgsSettings s;
  const QString lastQueryFileDir = s.value( QStringLiteral( "/UI/lastQueryFileDir" ), QDir::homePath() ).toString();

  const QString queryFileName = QFileDialog::getOpenFileName( nullptr, tr( "Load Query from File" ), lastQueryFileDir, tr( "Query files" ) + " (*.qqf);;" + tr( "All files" ) + " (*)" );
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

  const QDomElement queryElem = queryDoc.firstChildElement( QStringLiteral( "Query" ) );
  if ( queryElem.isNull() )
  {
    QMessageBox::critical( nullptr, tr( "Load Query from File" ), tr( "File is not a valid query document." ) );
    return;
  }

  const QString query = queryElem.text();

  //TODO: test if all the attributes are valid
  const QgsPointCloudExpression search( query );
  if ( search.hasParserError() )
  {
    QMessageBox::critical( this, tr( "Query Result" ), search.parserErrorString() );
    return;
  }

  mTxtSql->clear();
  mTxtSql->insertText( query );
}
