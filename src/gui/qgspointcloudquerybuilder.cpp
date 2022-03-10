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
#include "qgspointcloudquerybuilder.h"
#include "qgslogger.h"
#include "qgsproject.h"
#include "qgssettings.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudexpression.h"
#include "qgsvectordataprovider.h"
#include "qgsapplication.h"
#include "qgshelp.h"
#include "qgsgui.h"

#include <QDomDocument>
#include <QDomElement>
#include <QFileDialog>
#include <QInputDialog>
#include <QListView>
#include <QMessageBox>
#include <QRegExp>
#include <QPushButton>
#include <QTextStream>


// constructor used when the query builder must make its own
// connection to the database
QgsPointCloudQueryBuilder::QgsPointCloudQueryBuilder( QgsPointCloudLayer *layer,
    QWidget *parent, Qt::WindowFlags fl )
  : QgsSubsetStringEditorInterface( parent, fl )
  , mPreviousFieldRow( -1 )
  , mLayer( layer )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  // disable values widgets for now
  groupBox2->setEnabled( false );
  connect( btnEqual, &QPushButton::clicked, this, &QgsPointCloudQueryBuilder::btnEqual_clicked );
  connect( btnLessThan, &QPushButton::clicked, this, &QgsPointCloudQueryBuilder::btnLessThan_clicked );
  connect( btnGreaterThan, &QPushButton::clicked, this, &QgsPointCloudQueryBuilder::btnGreaterThan_clicked );
  connect( btnIn, &QPushButton::clicked, this, &QgsPointCloudQueryBuilder::btnIn_clicked );
  connect( btnNotIn, &QPushButton::clicked, this, &QgsPointCloudQueryBuilder::btnNotIn_clicked );
  connect( lstFields, &QListView::clicked, this, &QgsPointCloudQueryBuilder::lstFields_clicked );
  connect( lstFields, &QListView::doubleClicked, this, &QgsPointCloudQueryBuilder::lstFields_doubleClicked );
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

  setupGuiViews();

  mOrigSubsetString = layer->subsetString();
  connect( layer, &QgsPointCloudLayer::subsetStringChanged, this, &QgsPointCloudQueryBuilder::layerSubsetStringChanged );
  layerSubsetStringChanged();

  lblDataUri->setText( tr( "Set provider filter on %1" ).arg( layer->name() ) );
  mTxtSql->setText( mOrigSubsetString );

  populateFields();
}

void QgsPointCloudQueryBuilder::showEvent( QShowEvent *event )
{
  mTxtSql->setFocus();
  QDialog::showEvent( event );
}

void QgsPointCloudQueryBuilder::populateFields()
{
  const QgsFields &fields = mLayer->dataProvider()->index()->attributes().toFields();
  mTxtSql->setFields( fields );
  for ( int idx = 0; idx < fields.count(); ++idx )
  {
    QStandardItem *myItem = new QStandardItem( fields.at( idx ).displayNameWithAlias() );
    myItem->setData( idx );
    myItem->setEditable( false );
    mModelFields->insertRow( mModelFields->rowCount(), myItem );
  }

  // All fields get ... setup
  setupLstFieldsModel();
}

void QgsPointCloudQueryBuilder::setupLstFieldsModel()
{
  lstFields->setModel( mModelFields );
}

void QgsPointCloudQueryBuilder::setupGuiViews()
{
  //Initialize the models
  mModelFields = new QStandardItemModel();

  // Modes
  lstFields->setViewMode( QListView::ListMode );
  lstFields->setSelectionBehavior( QAbstractItemView::SelectRows );
  // Performance tip since Qt 4.1
  lstFields->setUniformItemSizes( true );
  // Colored rows
  lstFields->setAlternatingRowColors( true );
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
    mLayer->setSubsetString( mTxtSql->text() );
    QMessageBox::information( this,
                              tr( "Query Result" ),
                              tr( "The expression was successfully parsed." ) );
  }
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

void QgsPointCloudQueryBuilder::btnEqual_clicked()
{
  mTxtSql->insertText( QStringLiteral( " = " ) );
  mTxtSql->setFocus();
}

void QgsPointCloudQueryBuilder::btnLessThan_clicked()
{
  mTxtSql->insertText( QStringLiteral( " < " ) );
  mTxtSql->setFocus();
}

void QgsPointCloudQueryBuilder::btnGreaterThan_clicked()
{
  mTxtSql->insertText( QStringLiteral( " > " ) );
  mTxtSql->setFocus();
}

void QgsPointCloudQueryBuilder::btnIn_clicked()
{
  mTxtSql->insertText( QStringLiteral( " IN " ) );
  mTxtSql->setFocus();
}

void QgsPointCloudQueryBuilder::btnNotIn_clicked()
{
  mTxtSql->insertText( QStringLiteral( " NOT IN " ) );
  mTxtSql->setFocus();
}

QString QgsPointCloudQueryBuilder::sql() const
{
  return mTxtSql->text();
}

void QgsPointCloudQueryBuilder::setSql( const QString &sqlStatement )
{
  mTxtSql->setText( sqlStatement );
}

void QgsPointCloudQueryBuilder::lstFields_clicked( const QModelIndex &index )
{
  if ( mPreviousFieldRow != index.row() )
  {
    mPreviousFieldRow = index.row();
  }
}

void QgsPointCloudQueryBuilder::lstFields_doubleClicked( const QModelIndex &index )
{
  mTxtSql->insertText( mModelFields->data( index ).toString() );
  mTxtSql->setFocus();
}

void QgsPointCloudQueryBuilder::btnLessEqual_clicked()
{
  mTxtSql->insertText( QStringLiteral( " <= " ) );
  mTxtSql->setFocus();
}

void QgsPointCloudQueryBuilder::btnGreaterEqual_clicked()
{
  mTxtSql->insertText( QStringLiteral( " >= " ) );
  mTxtSql->setFocus();
}

void QgsPointCloudQueryBuilder::btnNotEqual_clicked()
{
  mTxtSql->insertText( QStringLiteral( " != " ) );
  mTxtSql->setFocus();
}

void QgsPointCloudQueryBuilder::btnAnd_clicked()
{
  mTxtSql->insertText( QStringLiteral( " AND " ) );
  mTxtSql->setFocus();
}

void QgsPointCloudQueryBuilder::btnOr_clicked()
{
  mTxtSql->insertText( QStringLiteral( " OR " ) );
  mTxtSql->setFocus();
}

void QgsPointCloudQueryBuilder::clear()
{
  mTxtSql->clear();
  mLayer->setSubsetString( QString() );
}

void QgsPointCloudQueryBuilder::setDatasourceDescription( const QString &uri )
{
  lblDataUri->setText( uri );
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

void QgsPointCloudQueryBuilder::layerSubsetStringChanged()
{
  if ( mIgnoreLayerSubsetStringChangedSignal )
    return;
}
