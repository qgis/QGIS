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
#include "qgsquerybuilder.h"

#include <QDomDocument>
#include <QDomElement>
#include <QFileDialog>
#include <QListView>
#include <QMessageBox>
#include <QPushButton>
#include <QTextStream>


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
    const QgsPointCloudStatistics stats = mLayer->statistics();
    double value = stats.minimum( attribute );
    QString valueString = std::isnan( value ) ? tr( "n/a" ) : QString::number( value );
    QStandardItem *item = new QStandardItem( tr( "Minimum: %1" ).arg( valueString ) );
    item->setData( value, Qt::UserRole );
    mModelValues->insertRow( mModelValues->rowCount(), item );

    value = stats.maximum( attribute );
    valueString = std::isnan( value ) ? tr( "n/a" ) : QString::number( value );
    item = new QStandardItem( tr( "Maximum: %1" ).arg( valueString ) );
    item->setData( value, Qt::UserRole );
    mModelValues->insertRow( mModelValues->rowCount(), item );

    value = stats.mean( attribute );
    valueString = std::isnan( value ) ? tr( "n/a" ) : QString::number( value );
    item = new QStandardItem( tr( "Mean: %1" ).arg( valueString ) );
    item->setData( value, Qt::UserRole );
    mModelValues->insertRow( mModelValues->rowCount(), item );

    value = stats.stDev( attribute );
    valueString = std::isnan( value ) ? tr( "n/a" ) : QString::number( value );
    item = new QStandardItem( tr( "StdDev: %1" ).arg( valueString ) );
    item->setData( value, Qt::UserRole );
    mModelValues->insertRow( mModelValues->rowCount(), item );
  }
}

void QgsPointCloudQueryBuilder::lstAttributes_doubleClicked( const QModelIndex &index )
{
  mTxtSql->insertText( QStringLiteral( "%1 " ).arg( mModelAttributes->data( index ).toString() ) );
  mTxtSql->setFocus();
}

void QgsPointCloudQueryBuilder::lstValues_doubleClicked( const QModelIndex &index )
{
  mTxtSql->insertText( QStringLiteral( "%1 " ).arg( mModelValues->data( index, Qt::UserRole ).toString() ) );
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

void QgsPointCloudQueryBuilder::saveQuery()
{
  const bool ok = QgsQueryBuilder::saveQueryToFile( mTxtSql->text() );
  Q_UNUSED( ok )
}

void QgsPointCloudQueryBuilder::loadQuery()
{
  QString subset;
  if ( QgsQueryBuilder::loadQueryFromFile( subset ) )
  {
    mTxtSql->clear();
    mTxtSql->insertText( subset );
  }
}
