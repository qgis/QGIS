/***************************************************************************
    qgsrelationmanagerdialog.cpp
     --------------------------------------
    Date                 : 23.2.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdiscoverrelationsdialog.h"
#include "qgsrelationadddlg.h"
#include "qgsrelationmanagerdialog.h"
#include "qgsrelationmanager.h"
#include "qgsvectorlayer.h"

QgsRelationManagerDialog::QgsRelationManagerDialog( QgsRelationManager *relationMgr, QWidget *parent )
  : QWidget( parent )
  , Ui::QgsRelationManagerDialogBase()
  , mRelationManager( relationMgr )
{
  setupUi( this );
  connect( mBtnAddRelation, &QPushButton::clicked, this, &QgsRelationManagerDialog::mBtnAddRelation_clicked );
  connect( mBtnDiscoverRelations, &QPushButton::clicked, this, &QgsRelationManagerDialog::mBtnDiscoverRelations_clicked );
  connect( mBtnRemoveRelation, &QPushButton::clicked, this, &QgsRelationManagerDialog::mBtnRemoveRelation_clicked );

  mBtnRemoveRelation->setEnabled( false );
  connect( mRelationsTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsRelationManagerDialog::onSelectionChanged );
}

void QgsRelationManagerDialog::setLayers( const QList< QgsVectorLayer * > &layers )
{
  mLayers = layers;

  const QList<QgsRelation> &relations = mRelationManager->relations().values();

  const auto constRelations = relations;
  for ( const QgsRelation &rel : constRelations )
  {
    addRelation( rel );
  }

  mRelationsTable->sortByColumn( 0, Qt::AscendingOrder );
}

void QgsRelationManagerDialog::addRelation( const QgsRelation &rel )
{
  if ( ! rel.isValid() )
    return;

  QString referencingFields = rel.fieldPairs().at( 0 ).referencingField();
  QString referencedFields = rel.fieldPairs().at( 0 ).referencedField();
  for ( int i = 1; i < rel.fieldPairs().count(); i++ )
  {
    referencingFields.append( QStringLiteral( ", %1" ).arg( rel.fieldPairs().at( i ).referencingField() ) );
    referencedFields.append( QStringLiteral( ", %1" ).arg( rel.fieldPairs().at( i ).referencedField() ) );
  }

  mRelationsTable->setSortingEnabled( false );
  int row = mRelationsTable->rowCount();
  mRelationsTable->insertRow( row );

  QTableWidgetItem *item = new QTableWidgetItem( rel.name() );
  // Save relation in first column's item
  item->setData( Qt::UserRole, QVariant::fromValue<QgsRelation>( rel ) );
  mRelationsTable->setItem( row, 0, item );

  item = new QTableWidgetItem( rel.referencedLayer()->name() );
  item->setFlags( Qt::ItemIsEnabled );
  mRelationsTable->setItem( row, 1, item );

  item = new QTableWidgetItem( referencedFields );
  item->setFlags( Qt::ItemIsEnabled );
  mRelationsTable->setItem( row, 2, item );

  item = new QTableWidgetItem( rel.referencingLayer()->name() );
  item->setFlags( Qt::ItemIsEnabled );
  mRelationsTable->setItem( row, 3, item );

  item = new QTableWidgetItem( referencingFields );
  item->setFlags( Qt::ItemIsEnabled );
  mRelationsTable->setItem( row, 4, item );

  item = new QTableWidgetItem( rel.id() );
  item->setFlags( Qt::ItemIsEnabled );
  mRelationsTable->setItem( row, 5, item );

  if ( rel.strength() == QgsRelation::RelationStrength::Composition )
  {
    item = new QTableWidgetItem( QStringLiteral( "Composition" ) );
  }
  else
  {
    item = new QTableWidgetItem( QStringLiteral( "Association" ) );
  }
  item->setFlags( Qt::ItemIsEnabled );
  mRelationsTable->setItem( row, 6, item );

  mRelationsTable->setSortingEnabled( true );
}

void QgsRelationManagerDialog::mBtnAddRelation_clicked()
{
  QgsRelationAddDlg addDlg;

  if ( addDlg.exec() )
  {
    QgsRelation relation;

    relation.setReferencingLayer( addDlg.referencingLayerId() );
    relation.setReferencedLayer( addDlg.referencedLayerId() );
    QString relationId = addDlg.relationId();
    if ( addDlg.relationId().isEmpty() )
      relationId = QStringLiteral( "%1_%2_%3_%4" )
                   .arg( addDlg.referencingLayerId().left( 10 ),
                         addDlg.references().at( 0 ).first,
                         addDlg.referencedLayerId().left( 10 ),
                         addDlg.references().at( 0 ).second );

    QStringList existingNames;

    const auto rels { relations() };
    for ( const QgsRelation &rel : rels )
    {
      existingNames << rel.id();
    }

    QString tempId = relationId + "_%1";
    int suffix = 1;
    while ( existingNames.contains( relationId ) )
    {
      relationId = tempId.arg( suffix );
      ++suffix;
    }
    relation.setId( relationId );
    const auto references = addDlg.references();
    for ( const auto &reference : references )
      relation.addFieldPair( reference.first, reference.second );
    relation.setName( addDlg.relationName() );
    relation.setStrength( addDlg.relationStrength() );

    addRelation( relation );
  }
}

void QgsRelationManagerDialog::mBtnDiscoverRelations_clicked()
{
  QgsDiscoverRelationsDialog discoverDlg( relations(), mLayers, this );
  if ( discoverDlg.exec() )
  {
    const auto constRelations = discoverDlg.relations();
    for ( const QgsRelation &relation : constRelations )
    {
      addRelation( relation );
    }
  }
}

void QgsRelationManagerDialog::mBtnRemoveRelation_clicked()
{
  const QModelIndexList rows = mRelationsTable->selectionModel()->selectedRows();
  for ( int i = rows.size() - 1; i >= 0; --i )
  {
    mRelationsTable->removeRow( rows[i].row() );
  }
}

QList< QgsRelation > QgsRelationManagerDialog::relations()
{
  QList< QgsRelation > relations;

  int rows = mRelationsTable->rowCount();
  relations.reserve( rows );
  for ( int i = 0; i < rows; ++i )
  {
    QgsRelation relation = mRelationsTable->item( i, 0 )->data( Qt::UserRole ).value<QgsRelation>();
    // The name can be edited in the table, so apply this one
    relation.setName( mRelationsTable->item( i, 0 )->data( Qt::DisplayRole ).toString() );
    relations << relation;
  }

  return relations;
}

void QgsRelationManagerDialog::onSelectionChanged()
{
  mBtnRemoveRelation->setEnabled( mRelationsTable->selectionModel()->hasSelection() );
}
