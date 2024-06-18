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
#include "qgsapplication.h"
#include "qgshelp.h"
#include "qgsgui.h"
#include "qgsfieldproxymodel.h"
#include "qgsfieldmodel.h"

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
QgsQueryBuilder::QgsQueryBuilder( QgsVectorLayer *layer,
                                  QWidget *parent, Qt::WindowFlags fl )
  : QgsSubsetStringEditorInterface( parent, fl )
  , mPreviousFieldRow( -1 )
  , mLayer( layer )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );
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

  QPushButton *pbn = new QPushButton( tr( "&Test" ) );
  buttonBox->addButton( pbn, QDialogButtonBox::ActionRole );
  connect( pbn, &QAbstractButton::clicked, this, &QgsQueryBuilder::test );

  pbn = new QPushButton( tr( "&Clear" ) );
  buttonBox->addButton( pbn, QDialogButtonBox::ActionRole );
  connect( pbn, &QAbstractButton::clicked, this, &QgsQueryBuilder::clear );

  pbn = new QPushButton( tr( "&Save…" ) );
  buttonBox->addButton( pbn, QDialogButtonBox::ActionRole );
  pbn->setToolTip( tr( "Save query to QQF file" ) );
  connect( pbn, &QAbstractButton::clicked, this, &QgsQueryBuilder::saveQuery );

  pbn = new QPushButton( tr( "&Load…" ) );
  buttonBox->addButton( pbn, QDialogButtonBox::ActionRole );
  pbn->setToolTip( tr( "Load query from QQF file" ) );
  connect( pbn, &QAbstractButton::clicked, this, &QgsQueryBuilder::loadQuery );

  setupGuiViews();

  mModelFields = new QgsFieldProxyModel();
  mModelFields->setFilters( QgsFieldProxyModel::Filter::AllTypes | QgsFieldProxyModel::Filter::OriginProvider );
  mModelFields->sourceFieldModel()->setLayer( layer );
  lstFields->setModel( mModelFields );

  mOrigSubsetString = layer->subsetString();
  connect( layer, &QgsVectorLayer::subsetStringChanged, this, &QgsQueryBuilder::layerSubsetStringChanged );
  layerSubsetStringChanged();

  lblDataUri->setText( tr( "Set provider filter on %1" ).arg( layer->name() ) );
  mTxtSql->setText( mOrigSubsetString );

  mFilterLineEdit->setShowSearchIcon( true );
  mFilterLineEdit->setPlaceholderText( tr( "Search…" ) );
  connect( mFilterLineEdit, &QgsFilterLineEdit::textChanged, this, &QgsQueryBuilder::onTextChanged );
}

void QgsQueryBuilder::showEvent( QShowEvent *event )
{
  mTxtSql->setFocus();
  QDialog::showEvent( event );
}

void QgsQueryBuilder::setupGuiViews()
{
  //Initialize the models
  mModelValues = new QStandardItemModel();
  mProxyValues = new QSortFilterProxyModel();
  mProxyValues->setSourceModel( mModelValues );
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
  lstValues->setModel( mProxyValues );
}

void QgsQueryBuilder::fillValues( const QString &field, int limit )
{
  // clear the model
  mModelValues->clear();

  const int fieldIndex = mLayer->fields().lookupField( field );

  // determine the field type
  QList<QVariant> values = qgis::setToList( mLayer->uniqueValues( fieldIndex, limit ) );
  std::sort( values.begin(), values.end() );

  const QString nullValue = QgsApplication::nullRepresentation();

  QgsDebugMsgLevel( QStringLiteral( "nullValue: %1" ).arg( nullValue ), 2 );

  const auto constValues = values;
  for ( const QVariant &var : constValues )
  {
    QString value;
    if ( QgsVariantUtils::isNull( var ) )
      value = nullValue;
    else if ( var.userType() == QMetaType::Type::QDate && mLayer->providerType() == QLatin1String( "ogr" ) && mLayer->storageType() == QLatin1String( "ESRI Shapefile" ) )
      value = var.toDate().toString( QStringLiteral( "yyyy/MM/dd" ) );
    else if ( var.userType() == QMetaType::Type::QVariantList || var.userType() == QMetaType::Type::QStringList )
    {
      const QVariantList list = var.toList();
      for ( const QVariant &val : list )
      {
        if ( !value.isEmpty() )
          value.append( ", " );
        value.append( QgsVariantUtils::isNull( val ) ? nullValue : val.toString() );
      }
    }
    else
      value = var.toString();

    QStandardItem *myItem = new QStandardItem( value );
    myItem->setEditable( false );
    myItem->setData( var, Qt::UserRole + 1 );
    mModelValues->insertRow( mModelValues->rowCount(), myItem );
    QgsDebugMsgLevel( QStringLiteral( "Value is null: %1\nvalue: %2" ).arg( QgsVariantUtils::isNull( var ) ).arg( QgsVariantUtils::isNull( var ) ? nullValue : var.toString() ), 2 );
  }
}

void QgsQueryBuilder::btnSampleValues_clicked()
{
  lstValues->setCursor( Qt::WaitCursor );

  const QString prevSubsetString = mLayer->subsetString();
  if ( mUseUnfilteredLayer->isChecked() && !prevSubsetString.isEmpty() )
  {
    mIgnoreLayerSubsetStringChangedSignal = true;
    mLayer->setSubsetString( QString() );
  }

  //Clear and fill the mModelValues
  fillValues( mModelFields->data( lstFields->currentIndex(), static_cast< int >( QgsFieldModel::CustomRole::FieldName ) ).toString(), 25 );

  if ( prevSubsetString != mLayer->subsetString() )
  {
    mLayer->setSubsetString( prevSubsetString );
    mIgnoreLayerSubsetStringChangedSignal = false;
  }

  lstValues->setCursor( Qt::ArrowCursor );
}

void QgsQueryBuilder::btnGetAllValues_clicked()
{
  lstValues->setCursor( Qt::WaitCursor );

  const QString prevSubsetString = mLayer->subsetString();
  if ( mUseUnfilteredLayer->isChecked() && !prevSubsetString.isEmpty() )
  {
    mIgnoreLayerSubsetStringChangedSignal = true;
    mLayer->setSubsetString( QString() );
  }

  //Clear and fill the mModelValues
  fillValues( mModelFields->data( lstFields->currentIndex(), static_cast< int >( QgsFieldModel::CustomRole::FieldName ) ).toString(), -1 );

  if ( prevSubsetString != mLayer->subsetString() )
  {
    mLayer->setSubsetString( prevSubsetString );
    mIgnoreLayerSubsetStringChangedSignal = false;
  }

  lstValues->setCursor( Qt::ArrowCursor );
}

void QgsQueryBuilder::test()
{
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
}

void QgsQueryBuilder::accept()
{
  if ( mTxtSql->text() != mOrigSubsetString )
  {
    if ( !mLayer->setSubsetString( mTxtSql->text() ) )
    {
      //error in query - show the problem
      if ( mLayer->dataProvider()->hasErrors() )
      {
        QMessageBox::warning( this,
                              tr( "Query Result" ),
                              tr( "An error occurred when executing the query." )
                              + tr( "\nThe data provider said:\n%1" ).arg( mLayer->dataProvider()->errors().join( QLatin1Char( '\n' ) ) ) );
        mLayer->dataProvider()->clearErrors();
      }
      else
      {
        QMessageBox::warning( this, tr( "Query Result" ), tr( "Error in query. The subset string could not be set." ) );
      }

      return;
    }
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
  mTxtSql->insertText( QStringLiteral( " = " ) );
  mTxtSql->setFocus();
}

void QgsQueryBuilder::btnLessThan_clicked()
{
  mTxtSql->insertText( QStringLiteral( " < " ) );
  mTxtSql->setFocus();
}

void QgsQueryBuilder::btnGreaterThan_clicked()
{
  mTxtSql->insertText( QStringLiteral( " > " ) );
  mTxtSql->setFocus();
}

void QgsQueryBuilder::btnPct_clicked()
{
  mTxtSql->insertText( QStringLiteral( "%" ) );
  mTxtSql->setFocus();
}

void QgsQueryBuilder::btnIn_clicked()
{
  mTxtSql->insertText( QStringLiteral( " IN " ) );
  mTxtSql->setFocus();
}

void QgsQueryBuilder::btnNotIn_clicked()
{
  mTxtSql->insertText( QStringLiteral( " NOT IN " ) );
  mTxtSql->setFocus();
}

void QgsQueryBuilder::btnLike_clicked()
{
  mTxtSql->insertText( QStringLiteral( " LIKE " ) );
  mTxtSql->setFocus();
}

QString QgsQueryBuilder::sql() const
{
  return mTxtSql->text();
}

void QgsQueryBuilder::setSql( const QString &sqlStatement )
{
  mTxtSql->setText( sqlStatement );
}

void QgsQueryBuilder::lstFields_clicked( const QModelIndex &index )
{
  if ( mPreviousFieldRow != index.row() )
  {
    mPreviousFieldRow = index.row();

    btnSampleValues->setEnabled( true );
    btnGetAllValues->setEnabled( true );

    mModelValues->clear();
    mFilterLineEdit->clear();
  }
}

void QgsQueryBuilder::lstFields_doubleClicked( const QModelIndex &index )
{
  mTxtSql->insertText( '\"' + mModelFields->data( index, static_cast< int >( QgsFieldModel::CustomRole::FieldName ) ).toString() + '\"' );
  mTxtSql->setFocus();
}

void QgsQueryBuilder::lstValues_doubleClicked( const QModelIndex &index )
{
  const QVariant value = index.data( Qt::UserRole + 1 );
  if ( QgsVariantUtils::isNull( value ) )
    mTxtSql->insertText( QStringLiteral( "NULL" ) );
  else if ( value.userType() == QMetaType::Type::QDate && mLayer->providerType() == QLatin1String( "ogr" ) && mLayer->storageType() == QLatin1String( "ESRI Shapefile" ) )
    mTxtSql->insertText( '\'' + value.toDate().toString( QStringLiteral( "yyyy/MM/dd" ) ) + '\'' );
  else if ( value.userType() == QMetaType::Type::Int || value.userType() == QMetaType::Type::Double || value.userType() == QMetaType::Type::LongLong || value.userType() == QMetaType::Type::Bool )
    mTxtSql->insertText( value.toString() );
  else
    mTxtSql->insertText( '\'' + value.toString().replace( '\'', QLatin1String( "''" ) ) + '\'' );

  mTxtSql->setFocus();
}

void QgsQueryBuilder::btnLessEqual_clicked()
{
  mTxtSql->insertText( QStringLiteral( " <= " ) );
  mTxtSql->setFocus();
}

void QgsQueryBuilder::btnGreaterEqual_clicked()
{
  mTxtSql->insertText( QStringLiteral( " >= " ) );
  mTxtSql->setFocus();
}

void QgsQueryBuilder::btnNotEqual_clicked()
{
  mTxtSql->insertText( QStringLiteral( " != " ) );
  mTxtSql->setFocus();
}

void QgsQueryBuilder::btnAnd_clicked()
{
  mTxtSql->insertText( QStringLiteral( " AND " ) );
  mTxtSql->setFocus();
}

void QgsQueryBuilder::btnNot_clicked()
{
  mTxtSql->insertText( QStringLiteral( " NOT " ) );
  mTxtSql->setFocus();
}

void QgsQueryBuilder::btnOr_clicked()
{
  mTxtSql->insertText( QStringLiteral( " OR " ) );
  mTxtSql->setFocus();
}

void QgsQueryBuilder::onTextChanged( const QString &text )
{
  mProxyValues->setFilterCaseSensitivity( Qt::CaseInsensitive );
  mProxyValues->setFilterWildcard( text );
}

void QgsQueryBuilder::clear()
{
  mTxtSql->clear();
  mLayer->setSubsetString( QString() );
}

void QgsQueryBuilder::btnILike_clicked()
{
  mTxtSql->insertText( QStringLiteral( " ILIKE " ) );
  mTxtSql->setFocus();
}

void QgsQueryBuilder::setDatasourceDescription( const QString &uri )
{
  lblDataUri->setText( uri );
}

void QgsQueryBuilder::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_vector/vector_properties.html#query-builder" ) );
}

void QgsQueryBuilder::saveQuery()
{
  const bool ok = saveQueryToFile( mTxtSql->text() );
  Q_UNUSED( ok )
}

bool QgsQueryBuilder::saveQueryToFile( const QString &subset )
{
  QgsSettings s;
  const QString lastQueryFileDir = s.value( QStringLiteral( "/UI/lastQueryFileDir" ), QDir::homePath() ).toString();
  //save as qqf (QGIS query file)
  QString saveFileName = QFileDialog::getSaveFileName( nullptr, tr( "Save Query to File" ), lastQueryFileDir, tr( "Query files (*.qqf *.QQF)" ) );
  if ( saveFileName.isNull() )
  {
    return false;
  }

  if ( !saveFileName.endsWith( QLatin1String( ".qqf" ), Qt::CaseInsensitive ) )
  {
    saveFileName += QLatin1String( ".qqf" );
  }

  QFile saveFile( saveFileName );
  if ( !saveFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    QMessageBox::critical( nullptr, tr( "Save Query to File" ), tr( "Could not open file for writing." ) );
    return false ;
  }

  QDomDocument xmlDoc;
  QDomElement queryElem = xmlDoc.createElement( QStringLiteral( "Query" ) );
  const QDomText queryTextNode = xmlDoc.createTextNode( subset );
  queryElem.appendChild( queryTextNode );
  xmlDoc.appendChild( queryElem );

  QTextStream fileStream( &saveFile );
  xmlDoc.save( fileStream, 2 );

  const QFileInfo fi( saveFile );
  s.setValue( QStringLiteral( "/UI/lastQueryFileDir" ), fi.absolutePath() );
  return true;
}

void QgsQueryBuilder::loadQuery()
{
  QString subset;
  if ( loadQueryFromFile( subset ) )
  {
    mTxtSql->clear();
    mTxtSql->insertText( subset );
  }
}

bool QgsQueryBuilder::loadQueryFromFile( QString &subset )
{
  const QgsSettings s;
  const QString lastQueryFileDir = s.value( QStringLiteral( "/UI/lastQueryFileDir" ), QDir::homePath() ).toString();

  const QString queryFileName = QFileDialog::getOpenFileName( nullptr, tr( "Load Query from File" ), lastQueryFileDir, tr( "Query files" ) + " (*.qqf);;" + tr( "All files" ) + " (*)" );
  if ( queryFileName.isNull() )
  {
    return false;
  }

  QFile queryFile( queryFileName );
  if ( !queryFile.open( QIODevice::ReadOnly ) )
  {
    QMessageBox::critical( nullptr, tr( "Load Query from File" ), tr( "Could not open file for reading." ) );
    return false;
  }
  QDomDocument queryDoc;
  if ( !queryDoc.setContent( &queryFile ) )
  {
    QMessageBox::critical( nullptr, tr( "Load Query from File" ), tr( "File is not a valid xml document." ) );
    return false;
  }

  const QDomElement queryElem = queryDoc.firstChildElement( QStringLiteral( "Query" ) );
  if ( queryElem.isNull() )
  {
    QMessageBox::critical( nullptr, tr( "Load Query from File" ), tr( "File is not a valid query document." ) );
    return false;
  }

  subset = queryElem.text();
  return true;
}

void QgsQueryBuilder::layerSubsetStringChanged()
{
  if ( mIgnoreLayerSubsetStringChangedSignal )
    return;
  mUseUnfilteredLayer->setDisabled( mLayer->subsetString().isEmpty() );
}
