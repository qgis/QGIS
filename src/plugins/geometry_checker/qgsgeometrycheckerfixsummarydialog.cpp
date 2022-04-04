/***************************************************************************
 *  qgsgeometrycheckerfixsummarydialog.cpp                                 *
 *  -------------------                                                    *
 *  copyright            : (C) 2015 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeometrycheckcontext.h"
#include "qgsgeometrycheckerfixsummarydialog.h"
#include "qgsgeometrychecker.h"
#include "qgsgeometrycheck.h"
#include "qgsfeaturepool.h"
#include "qgisinterface.h"
#include "qgsmapcanvas.h"
#include "qgsvectorlayer.h"
#include "qgsgeometrycheckerror.h"

QgsGeometryCheckerFixSummaryDialog::QgsGeometryCheckerFixSummaryDialog( const Statistics &stats,
    QgsGeometryChecker *checker,
    QWidget *parent )
  : QDialog( parent )
  , mChecker( checker )
{
  ui.setupUi( this );

  ui.groupBoxFixedErrors->setTitle( tr( "%n error(s) were fixed", nullptr, stats.fixedErrors.size() ) );
  ui.groupBoxNewErrors->setTitle( tr( "%n new error(s) were found", nullptr, stats.newErrors.count() ) );
  ui.groupBoxNotFixed->setTitle( tr( "%n error(s) were not fixed", nullptr, stats.failedErrors.count() ) );
  ui.groupBoxObsoleteErrors->setTitle( tr( "%n error(s) are obsolete", nullptr, stats.obsoleteErrors.count() ) );

  for ( QgsGeometryCheckError *error : stats.fixedErrors )
  {
    addError( ui.tableWidgetFixedErrors, error );
  }
  for ( QgsGeometryCheckError *error : stats.newErrors )
  {
    addError( ui.tableWidgetNewErrors, error );
  }
  for ( QgsGeometryCheckError *error : stats.failedErrors )
  {
    addError( ui.tableWidgetNotFixed, error );
  }
  for ( QgsGeometryCheckError *error : stats.obsoleteErrors )
  {
    addError( ui.tableWidgetObsoleteErrors, error );
  }

  setupTable( ui.tableWidgetFixedErrors );
  setupTable( ui.tableWidgetNewErrors );
  setupTable( ui.tableWidgetNotFixed );
  setupTable( ui.tableWidgetObsoleteErrors );

  ui.plainTextEditMessages->setPlainText( checker->getMessages().join( QLatin1Char( '\n' ) ) );

  ui.groupBoxFixedErrors->setVisible( !stats.fixedErrors.isEmpty() );
  ui.groupBoxNewErrors->setVisible( !stats.newErrors.isEmpty() );
  ui.groupBoxNotFixed->setVisible( !stats.failedErrors.isEmpty() );
  ui.groupBoxObsoleteErrors->setVisible( !stats.obsoleteErrors.isEmpty() );
  ui.groupBoxMessages->setVisible( !checker->getMessages().isEmpty() );
}

void QgsGeometryCheckerFixSummaryDialog::addError( QTableWidget *table, QgsGeometryCheckError *error )
{
  const bool sortingWasEnabled = table->isSortingEnabled();
  if ( sortingWasEnabled )
    table->setSortingEnabled( false );

  const int prec = 7 - std::floor( std::max( 0., std::log10( std::max( error->location().x(), error->location().y() ) ) ) );
  const QString posStr = QStringLiteral( "%1, %2" ).arg( error->location().x(), 0, 'f', prec ).arg( error->location().y(), 0, 'f', prec );

  const int row = table->rowCount();
  table->insertRow( row );
  table->setItem( row, 0, new QTableWidgetItem( !error->layerId().isEmpty() ? mChecker->featurePools()[error->layerId()]->layer()->name() : "" ) );
  QTableWidgetItem *idItem = new QTableWidgetItem();
  idItem->setData( Qt::EditRole, error->featureId() != FID_NULL ? QVariant( error->featureId() ) : QVariant() );
  table->setItem( row, 1, idItem );
  table->setItem( row, 2, new QTableWidgetItem( error->description() ) );
  table->setItem( row, 3, new QTableWidgetItem( posStr ) );
  QTableWidgetItem *valueItem = new QTableWidgetItem();
  valueItem->setData( Qt::EditRole, error->value() );
  table->setItem( row, 4, valueItem );
  table->item( row, 0 )->setData( Qt::UserRole, QVariant::fromValue( reinterpret_cast<void *>( error ) ) );

  if ( sortingWasEnabled )
    table->setSortingEnabled( true );
}

void QgsGeometryCheckerFixSummaryDialog::setupTable( QTableWidget *table )
{
  table->resizeColumnToContents( 0 );
  table->resizeColumnToContents( 1 );
  table->horizontalHeader()->setSectionResizeMode( 2, QHeaderView::Stretch );
  table->horizontalHeader()->setSectionResizeMode( 3, QHeaderView::Stretch );
  table->horizontalHeader()->setSectionResizeMode( 4, QHeaderView::Stretch );
  table->setEditTriggers( QAbstractItemView::NoEditTriggers );
  table->setSelectionBehavior( QAbstractItemView::SelectRows );
  table->setSelectionMode( QAbstractItemView::SingleSelection );

  table->horizontalHeader()->setSortIndicator( 0, Qt::AscendingOrder );
  table->setSortingEnabled( true );

  connect( table->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsGeometryCheckerFixSummaryDialog::onTableSelectionChanged );
}

void QgsGeometryCheckerFixSummaryDialog::onTableSelectionChanged( const QItemSelection &newSel, const QItemSelection & /*oldSel*/ )
{
  QItemSelectionModel *selModel = qobject_cast<QItemSelectionModel *>( QObject::sender() );
  const QAbstractItemModel *model = selModel->model();

  for ( QTableWidget *table : {ui.tableWidgetFixedErrors, ui.tableWidgetNewErrors, ui.tableWidgetNotFixed, ui.tableWidgetObsoleteErrors} )
  {
    if ( table->selectionModel() != selModel )
    {
      table->selectionModel()->blockSignals( true );
      table->clearSelection();
      table->selectionModel()->blockSignals( false );
    }
  }

  if ( !newSel.isEmpty() && !newSel.first().indexes().isEmpty() )
  {
    const QModelIndex idx = newSel.first().indexes().first();
    QgsGeometryCheckError *error = reinterpret_cast<QgsGeometryCheckError *>( model->data( model->index( idx.row(), 0 ), Qt::UserRole ).value<void *>() );
    emit errorSelected( error );
  }
}
