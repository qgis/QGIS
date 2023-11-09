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
#include "qgsdiscoverrelationsdialog.h"
#include "qgsvectorlayer.h"
#include "qgsrelationmanager.h"
#include "qgshelp.h"

#include <QPushButton>

QgsDiscoverRelationsDialog::QgsDiscoverRelationsDialog( const QList<QgsRelation> &existingRelations, const QList<QgsVectorLayer *> &layers, QWidget *parent )
  : QDialog( parent )
  , mLayers( layers )
{
  setupUi( this );

  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
  mButtonBox->addButton( QDialogButtonBox::Help );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, [ = ]
  {
    QgsHelp::openHelp( QStringLiteral( "working_with_vector/attribute_table.html#defining-1-n-relation" ) );
  } );
  connect( mRelationsTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsDiscoverRelationsDialog::onSelectionChanged );

  mFoundRelations = QgsRelationManager::discoverRelations( existingRelations, layers );
  for ( const QgsRelation &relation : std::as_const( mFoundRelations ) )
    addRelation( relation );

  mRelationsTable->resizeColumnsToContents();
}

void QgsDiscoverRelationsDialog::addRelation( const QgsRelation &rel )
{
  QString referencingFields, referencedFields;
  for ( int i = 0; i < rel.fieldPairs().count(); i++ )
  {
    referencingFields.append( QStringLiteral( "%1%2" ).arg( ( referencingFields.isEmpty() ? "" : ", " ),
                              rel.fieldPairs().at( i ).referencingField() ) );
    referencedFields.append( QStringLiteral( "%1%2" ).arg( ( referencedFields.isEmpty() ? "" : ", " ),
                             rel.fieldPairs().at( i ).referencedField() ) );
  }

  const int row = mRelationsTable->rowCount();
  mRelationsTable->insertRow( row );
  mRelationsTable->setItem( row, 0, new QTableWidgetItem( rel.name() ) );
  mRelationsTable->setItem( row, 1, new QTableWidgetItem( rel.referencingLayer()->name() ) );
  mRelationsTable->setItem( row, 2, new QTableWidgetItem( referencingFields ) );
  mRelationsTable->setItem( row, 3, new QTableWidgetItem( rel.referencedLayer()->name() ) );
  mRelationsTable->setItem( row, 4, new QTableWidgetItem( referencedFields ) );
  switch ( rel.strength() )
  {
    case Qgis::RelationshipStrength::Composition:
    {
      mRelationsTable->setItem( row, 5, new QTableWidgetItem( QStringLiteral( "Composition" ) ) );
      break;
    }
    case Qgis::RelationshipStrength::Association:
    {
      mRelationsTable->setItem( row, 5, new QTableWidgetItem( QStringLiteral( "Association" ) ) );
      break;
    }
  }

  mRelationsTable->item( row, 5 )->setToolTip( QStringLiteral( "Composition (child features will also be copied and deleted) or Association" ) );
}

QList<QgsRelation> QgsDiscoverRelationsDialog::relations() const
{
  QList<QgsRelation> result;
  const auto constSelectedRows = mRelationsTable->selectionModel()->selectedRows();
  for ( const QModelIndex &row : constSelectedRows )
  {
    result.append( mFoundRelations.at( row.row() ) );
  }
  return result;
}

void QgsDiscoverRelationsDialog::onSelectionChanged()
{
  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( mRelationsTable->selectionModel()->hasSelection() );
}
