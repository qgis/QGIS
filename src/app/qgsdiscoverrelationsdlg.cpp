/***************************************************************************
    qgsdiscoverrelationsdlg.cpp
    ---------------------
    begin                : September 2016
    copyright            : (C) 2016 by Patrick Valsecchi
    email                : patrick dot valsecchi at camptocamp dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsdiscoverrelationsdlg.h"
#include "qgsvectorlayer.h"
#include "qgsrelationmanager.h"

#include <QPushButton>

QgsDiscoverRelationsDlg::QgsDiscoverRelationsDlg( const QList<QgsRelation> &existingRelations, const QList<QgsVectorLayer *> &layers, QWidget *parent )
  : QDialog( parent )
  , mLayers( layers )
{
  setupUi( this );

  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
  connect( mRelationsTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsDiscoverRelationsDlg::onSelectionChanged );

  mFoundRelations = QgsRelationManager::discoverRelations( existingRelations, layers );
  Q_FOREACH ( const QgsRelation &relation, mFoundRelations ) addRelation( relation );

  mRelationsTable->resizeColumnsToContents();

}

void QgsDiscoverRelationsDlg::addRelation( const QgsRelation &rel )
{
  const int row = mRelationsTable->rowCount();
  mRelationsTable->insertRow( row );
  mRelationsTable->setItem( row, 0, new QTableWidgetItem( rel.name() ) );
  mRelationsTable->setItem( row, 1, new QTableWidgetItem( rel.referencingLayer()->name() ) );
  mRelationsTable->setItem( row, 2, new QTableWidgetItem( rel.fieldPairs().at( 0 ).referencingField() ) );
  mRelationsTable->setItem( row, 3, new QTableWidgetItem( rel.referencedLayer()->name() ) );
  mRelationsTable->setItem( row, 4, new QTableWidgetItem( rel.fieldPairs().at( 0 ).referencedField() ) );
  if ( rel.strength() == QgsRelation::RelationStrength::Composition )
  {
    mRelationsTable->setItem( row, 5, new QTableWidgetItem( QStringLiteral( "Composition" ) ) );
  }
  else
  {
    mRelationsTable->setItem( row, 5, new QTableWidgetItem( QStringLiteral( "Association" ) ) );
  }

  mRelationsTable->item( row, 5 )->setToolTip( QStringLiteral( "Composition (child features will be copied too) or Association" ) );

}

QList<QgsRelation> QgsDiscoverRelationsDlg::relations() const
{
  QList<QgsRelation> result;
  Q_FOREACH ( const QModelIndex &row, mRelationsTable->selectionModel()->selectedRows() )
  {
    result.append( mFoundRelations.at( row.row() ) );
  }
  return result;
}

void QgsDiscoverRelationsDlg::onSelectionChanged()
{
  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( mRelationsTable->selectionModel()->hasSelection() );
}
