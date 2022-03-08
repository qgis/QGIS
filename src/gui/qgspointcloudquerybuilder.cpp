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
  connect( btnEqual, &QPushButton::clicked, this, &QgsPointCloudQueryBuilder::btnEqual_clicked );
  connect( btnLessThan, &QPushButton::clicked, this, &QgsPointCloudQueryBuilder::btnLessThan_clicked );
  connect( btnGreaterThan, &QPushButton::clicked, this, &QgsPointCloudQueryBuilder::btnGreaterThan_clicked );
  connect( btnPct, &QPushButton::clicked, this, &QgsPointCloudQueryBuilder::btnPct_clicked );
  connect( btnIn, &QPushButton::clicked, this, &QgsPointCloudQueryBuilder::btnIn_clicked );
  connect( btnNotIn, &QPushButton::clicked, this, &QgsPointCloudQueryBuilder::btnNotIn_clicked );
  connect( lstFields, &QListView::clicked, this, &QgsPointCloudQueryBuilder::lstFields_clicked );
  connect( lstFields, &QListView::doubleClicked, this, &QgsPointCloudQueryBuilder::lstFields_doubleClicked );
  connect( btnLessEqual, &QPushButton::clicked, this, &QgsPointCloudQueryBuilder::btnLessEqual_clicked );
  connect( btnGreaterEqual, &QPushButton::clicked, this, &QgsPointCloudQueryBuilder::btnGreaterEqual_clicked );
  connect( btnNotEqual, &QPushButton::clicked, this, &QgsPointCloudQueryBuilder::btnNotEqual_clicked );
  connect( btnAnd, &QPushButton::clicked, this, &QgsPointCloudQueryBuilder::btnAnd_clicked );
  connect( btnNot, &QPushButton::clicked, this, &QgsPointCloudQueryBuilder::btnNot_clicked );
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
  /*
  const QgsFields &fields = mLayer->fields();
  mTxtSql->setFields( fields );
  for ( int idx = 0; idx < fields.count(); ++idx )
  {
    if ( fields.fieldOrigin( idx ) != QgsFields::OriginProvider )
    {
      // only consider native fields
      continue;
    }
    QStandardItem *myItem = new QStandardItem( fields.at( idx ).displayNameWithAlias() );
    myItem->setData( idx );
    myItem->setEditable( false );
    mModelFields->insertRow( mModelFields->rowCount(), myItem );
  }

  // All fields get ... setup
  setupLstFieldsModel();
  */
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
  /*
  // test the sql statement to see if it works
  // by counting the number of records that would be
  // returned

  if ( mLayer->setSubsetString( mTxtSql->text() ) )
  {
    const long long featureCount { mLayer->featureCount() };
    // Check for errors
    if ( featureCount < 0 )
    {
      QMessageBox::warning( this,
                            tr( "Query Result" ),
                            tr( "An error occurred when executing the query, please check the expression syntax." ) );
    }
    else
    {
      QMessageBox::information( this,
                                tr( "Query Result" ),
                                tr( "The where clause returned %n row(s).", "returned test rows", featureCount ) );
    }
  }
  else if ( mLayer->dataProvider()->hasErrors() )
  {
    QMessageBox::warning( this,
                          tr( "Query Result" ),
                          tr( "An error occurred when executing the query." )
                          + tr( "\nThe data provider said:\n%1" ).arg( mLayer->dataProvider()->errors().join( QLatin1Char( '\n' ) ) ) );
    mLayer->dataProvider()->clearErrors();
  }
  else
  {
    QMessageBox::warning( this,
                          tr( "Query Result" ),
                          tr( "An error occurred when executing the query." ) );
  }
  */
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

void QgsPointCloudQueryBuilder::btnPct_clicked()
{
  mTxtSql->insertText( QStringLiteral( "%" ) );
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
  /*
  mTxtSql->insertText( '\"' + mLayer->fields().at( mModelFields->data( index, Qt::UserRole + 1 ).toInt() ).name() + '\"' );
  mTxtSql->setFocus();
  */
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

void QgsPointCloudQueryBuilder::btnNot_clicked()
{
  mTxtSql->insertText( QStringLiteral( " NOT " ) );
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
  QgsHelp::openHelp( QStringLiteral( "working_with_vector/vector_properties.html#query-builder" ) );
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
  const QgsExpression search( query );
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
