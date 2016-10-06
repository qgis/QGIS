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

#include "qgsgeometrycheckerfixsummarydialog.h"
#include "../checks/qgsgeometrycheck.h"
#include "qgisinterface.h"
#include "qgsmapcanvas.h"

QgsGeometryCheckerFixSummaryDialog::QgsGeometryCheckerFixSummaryDialog( QgisInterface* iface, QgsVectorLayer* layer, const Statistics& stats, const QStringList &messages, QWidget *parent )
    : QDialog( parent )
    , mIface( iface )
    , mLayer( layer )
{
  ui.setupUi( this );

  ui.groupBoxFixedErrors->setTitle( tr( "%1 errors were fixed" ).arg( stats.fixedErrors.size() ) );
  ui.groupBoxNewErrors->setTitle( tr( "%1 new errors were found" ).arg( stats.newErrors.count() ) );
  ui.groupBoxNotFixed->setTitle( tr( "%1 errors were not fixed" ).arg( stats.failedErrors.count() ) );
  ui.groupBoxObsoleteErrors->setTitle( tr( "%1 errors are obsolete" ).arg( stats.obsoleteErrors.count() ) );

  Q_FOREACH ( QgsGeometryCheckError* error, stats.fixedErrors )
  {
    addError( ui.tableWidgetFixedErrors, error );
  }
  Q_FOREACH ( QgsGeometryCheckError* error, stats.newErrors )
  {
    addError( ui.tableWidgetNewErrors, error );
  }
  Q_FOREACH ( QgsGeometryCheckError* error, stats.failedErrors )
  {
    addError( ui.tableWidgetNotFixed, error );
  }
  Q_FOREACH ( QgsGeometryCheckError* error, stats.obsoleteErrors )
  {
    addError( ui.tableWidgetObsoleteErrors, error );
  }

  setupTable( ui.tableWidgetFixedErrors );
  setupTable( ui.tableWidgetNewErrors );
  setupTable( ui.tableWidgetNotFixed );
  setupTable( ui.tableWidgetObsoleteErrors );

  ui.plainTextEditMessages->setPlainText( messages.join( "\n" ) );

  ui.groupBoxFixedErrors->setVisible( !stats.fixedErrors.isEmpty() );
  ui.groupBoxNewErrors->setVisible( !stats.newErrors.isEmpty() );
  ui.groupBoxNotFixed->setVisible( !stats.failedErrors.isEmpty() );
  ui.groupBoxObsoleteErrors->setVisible( !stats.obsoleteErrors.isEmpty() );
  ui.groupBoxMessages->setVisible( !messages.isEmpty() );
}

void QgsGeometryCheckerFixSummaryDialog::addError( QTableWidget* table, QgsGeometryCheckError* error )
{
  int prec = 7 - std::floor( qMax( 0., std::log10( qMax( error->location().x(), error->location().y() ) ) ) );
  QString posStr = QString( "%1, %2" ).arg( error->location().x(), 0, 'f', prec ).arg( error->location().y(), 0, 'f', prec );
  double layerToMap = mIface->mapCanvas()->mapSettings().layerToMapUnits( mLayer );
  QVariant value;
  if ( error->valueType() == QgsGeometryCheckError::ValueLength )
  {
    value = QVariant::fromValue( error->value().toDouble() * layerToMap );
  }
  else if ( error->valueType() == QgsGeometryCheckError::ValueArea )
  {
    value = QVariant::fromValue( error->value().toDouble() * layerToMap * layerToMap );
  }
  else
  {
    value = error->value();
  }

  int row = table->rowCount();
  table->insertRow( row );
  QTableWidgetItem* idItem = new QTableWidgetItem();
  idItem->setData( Qt::EditRole, error->featureId() != FEATUREID_NULL ? QVariant( error->featureId() ) : QVariant() );
  idItem->setData( Qt::UserRole, QVariant::fromValue( reinterpret_cast<void*>( error ) ) );
  table->setItem( row, 0, idItem );
  table->setItem( row, 1, new QTableWidgetItem( error->description() ) );
  table->setItem( row, 2, new QTableWidgetItem( posStr ) );
  QTableWidgetItem* valueItem = new QTableWidgetItem();
  valueItem->setData( Qt::EditRole, value );
  table->setItem( row, 3, valueItem );
}

void QgsGeometryCheckerFixSummaryDialog::setupTable( QTableWidget* table )
{
  table->resizeColumnToContents( 0 );
  table->horizontalHeader()->setResizeMode( 1, QHeaderView::Stretch );
  table->horizontalHeader()->setResizeMode( 2, QHeaderView::Stretch );
  table->horizontalHeader()->setResizeMode( 3, QHeaderView::Stretch );
  table->horizontalHeader()->setResizeMode( 4, QHeaderView::Stretch );

  table->setEditTriggers( QAbstractItemView::NoEditTriggers );
  table->setSelectionBehavior( QAbstractItemView::SelectRows );
  table->setSelectionMode( QAbstractItemView::SingleSelection );

  table->horizontalHeader()->setSortIndicator( 0, Qt::AscendingOrder );
  table->setSortingEnabled( true );

  connect( table->selectionModel(), SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ), this, SLOT( onTableSelectionChanged( QItemSelection, QItemSelection ) ) );
}

void QgsGeometryCheckerFixSummaryDialog::onTableSelectionChanged( const QItemSelection &newSel, const QItemSelection & /*oldSel*/ )
{
  const QAbstractItemModel* model = qobject_cast<QItemSelectionModel*>( QObject::sender() )->model();

  Q_FOREACH ( QTableWidget* table, QList<QTableWidget*>() << ui.tableWidgetFixedErrors << ui.tableWidgetNewErrors << ui.tableWidgetNotFixed << ui.tableWidgetObsoleteErrors )
    if ( table->model() != model )
    {
      table->selectionModel()->blockSignals( true );
      table->clearSelection();
      table->selectionModel()->blockSignals( false );
    }

  if ( !newSel.isEmpty() && !newSel.first().indexes().isEmpty() )
  {
    QModelIndex idx = newSel.first().indexes().first();
    QgsGeometryCheckError* error = reinterpret_cast<QgsGeometryCheckError*>( model->data( model->index( idx.row(), 0 ), Qt::UserRole ).value<void*>() );
    emit errorSelected( error );
  }
}
