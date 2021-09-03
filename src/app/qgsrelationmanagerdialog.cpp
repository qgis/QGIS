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
#include "qgsrelationaddpolymorphicdialog.h"
#include "qgsrelationmanagerdialog.h"
#include "qgsrelationmanager.h"
#include "qgspolymorphicrelation.h"
#include "qgsvectorlayer.h"


#ifndef SIP_RUN
class RelationNameEditorDelegate: public QStyledItemDelegate
{
  public:
    RelationNameEditorDelegate( const QList<int> &editableColumns, QObject *parent = nullptr )
      : QStyledItemDelegate( parent )
      , mEditableColumns( editableColumns )
    {}

    virtual QWidget *createEditor( QWidget *parentWidget, const QStyleOptionViewItem &option, const QModelIndex &index ) const
    {
      if ( mEditableColumns.contains( index.column() ) )
        return QStyledItemDelegate::createEditor( parentWidget, option, index );

      return nullptr;
    }
  private:
    QList<int> mEditableColumns;
};
#endif

QgsRelationManagerDialog::QgsRelationManagerDialog( QgsRelationManager *relationMgr, QWidget *parent )
  : QWidget( parent )
  , Ui::QgsRelationManagerDialogBase()
  , mRelationManager( relationMgr )
{
  setupUi( this );

  mRelationsTree->setItemDelegate( new RelationNameEditorDelegate( QList<int>( {0} ), mRelationsTree ) );

  connect( mBtnAddRelation, &QPushButton::clicked, this, &QgsRelationManagerDialog::mBtnAddRelation_clicked );
  connect( mActionAddPolymorphicRelation, &QAction::triggered, this, &QgsRelationManagerDialog::mActionAddPolymorphicRelation_triggered );
  connect( mActionEditPolymorphicRelation, &QAction::triggered, this, &QgsRelationManagerDialog::mActionEditPolymorphicRelation_triggered );
  connect( mBtnDiscoverRelations, &QPushButton::clicked, this, &QgsRelationManagerDialog::mBtnDiscoverRelations_clicked );
  connect( mBtnRemoveRelation, &QPushButton::clicked, this, &QgsRelationManagerDialog::mBtnRemoveRelation_clicked );

  mBtnRemoveRelation->setEnabled( false );
  mBtnAddRelation->setPopupMode( QToolButton::MenuButtonPopup );
  mBtnAddRelation->addAction( mActionAddPolymorphicRelation );
  mBtnAddRelation->addAction( mActionEditPolymorphicRelation );

  connect( mRelationsTree->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsRelationManagerDialog::onSelectionChanged );
}

void QgsRelationManagerDialog::setLayers( const QList< QgsVectorLayer * > &layers )
{
  mLayers = layers;

  const QList<QgsRelation> &relations = mRelationManager->relations().values();
  for ( const QgsRelation &rel : relations )
  {
    // the generated relations for polymorphic relations should be ignored,
    // they are generated when the polymorphic relation is added to the table
    if ( rel.type() == QgsRelation::Generated )
      continue;

    addRelationPrivate( rel );
  }
  const QList<QgsPolymorphicRelation> &polymorphicRelations = mRelationManager->polymorphicRelations().values();
  for ( const QgsPolymorphicRelation &polymorphicRel : polymorphicRelations )
  {
    addPolymorphicRelation( polymorphicRel );
  }

  mRelationsTree->sortByColumn( 0, Qt::AscendingOrder );
}

bool QgsRelationManagerDialog::addRelation( const QgsRelation &rel )
{
  return addRelationPrivate( rel );
}

int QgsRelationManagerDialog::addPolymorphicRelation( const QgsPolymorphicRelation &polyRel )
{
  int itemsAdded = 0;
  if ( ! polyRel.isValid() )
    return itemsAdded;

  QString referencingFields;
  QString referencedFields;

  for ( int i = 0; i < polyRel.fieldPairs().count(); i++ )
  {
    if ( i != 0 )
    {
      referencingFields += QLatin1String( ", " );
      referencedFields += QLatin1String( ", " );
    }

    referencingFields += polyRel.fieldPairs().at( i ).referencingField();
    referencedFields += polyRel.fieldPairs().at( i ).referencedField();
  }

  mRelationsTree->setSortingEnabled( false );
  const int row = mRelationsTree->topLevelItemCount();
  QTreeWidgetItem *item = new QTreeWidgetItem();

  mRelationsTree->insertTopLevelItem( row, item );

  // Save relation in first column's item
  item->setExpanded( true );
  item->setFlags( item->flags() | Qt::ItemIsEditable );
  item->setData( 0, Qt::UserRole, QVariant::fromValue<QgsPolymorphicRelation>( polyRel ) );
  item->setText( 0, polyRel.name() );
  item->setText( 1, polyRel.referencingLayer()->name() );
  item->setText( 2, referencedFields );
  item->setText( 3, QStringLiteral( "as in \"%1\".\"%2\"" ).arg( polyRel.referencingLayer()->name(), polyRel.referencedLayerField() ) );
  item->setText( 4, referencingFields );
  item->setText( 5, polyRel.id() );
  item->setText( 6, polyRel.strength() == QgsRelation::RelationStrength::Composition
                 ? QStringLiteral( "Composition" )
                 : QStringLiteral( "Association" ) );

  const QList<QgsRelation> generatedRelations = polyRel.generateRelations();
  for ( const QgsRelation &generatedRelation : generatedRelations )
  {
    if ( addRelationPrivate( generatedRelation, item ) )
      itemsAdded++;
  }

  mRelationsTree->setSortingEnabled( true );

  return itemsAdded;
}

bool QgsRelationManagerDialog::addRelationPrivate( const QgsRelation &rel, QTreeWidgetItem *parentItem )
{
  if ( ! rel.isValid() )
    return false;

  QString referencingFields = rel.fieldPairs().at( 0 ).referencingField();
  QString referencedFields = rel.fieldPairs().at( 0 ).referencedField();
  for ( int i = 1; i < rel.fieldPairs().count(); i++ )
  {
    referencingFields.append( QStringLiteral( ", %1" ).arg( rel.fieldPairs().at( i ).referencingField() ) );
    referencedFields.append( QStringLiteral( ", %1" ).arg( rel.fieldPairs().at( i ).referencedField() ) );
  }

  mRelationsTree->setSortingEnabled( false );
  QTreeWidgetItem *item = new QTreeWidgetItem();

  switch ( rel.type() )
  {
    case QgsRelation::Normal:
      item->setFlags( item->flags() | Qt::ItemIsEditable );
      mRelationsTree->invisibleRootItem()->addChild( item );
      break;
    case QgsRelation::Generated:
      Q_ASSERT( parentItem );
      item->setFlags( item->flags() & ~Qt::ItemIsSelectable );
      parentItem->addChild( item );
      break;
  }

  // Save relation in first column's item
  item->setData( 0, Qt::UserRole, QVariant::fromValue<QgsRelation>( rel ) );

  item->setText( 0, rel.name() );
  item->setText( 1, rel.referencedLayer()->name() );
  item->setText( 2, referencedFields );
  item->setText( 3, rel.referencingLayer()->name() );
  item->setText( 4, referencingFields );
  item->setText( 5, rel.id() );
  item->setText( 6, rel.strength() == QgsRelation::RelationStrength::Composition
                 ? QStringLiteral( "Composition" )
                 : QStringLiteral( "Association" ) );

  mRelationsTree->setSortingEnabled( true );

  return true;
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

    const QString tempId = relationId + "_%1";
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

void QgsRelationManagerDialog::mActionAddPolymorphicRelation_triggered()
{
  QgsRelationAddPolymorphicDialog addDlg( false );

  if ( addDlg.exec() )
  {
    QgsPolymorphicRelation relation;
    relation.setReferencingLayer( addDlg.referencingLayerId() );
    relation.setReferencedLayerField( addDlg.referencedLayerField() );
    relation.setReferencedLayerExpression( addDlg.referencedLayerExpression() );
    relation.setReferencedLayerIds( addDlg.referencedLayerIds() );
    relation.setRelationStrength( addDlg.relationStrength() );

    const auto references = addDlg.fieldPairs();
    for ( const auto &reference : references )
      relation.addFieldPair( reference.first, reference.second );

    const QString relationId = addDlg.relationId();

    if ( relationId.isEmpty() )
      relation.generateId();
    else
      relation.setId( relationId );

    addPolymorphicRelation( relation );
  }
}

void QgsRelationManagerDialog::mActionEditPolymorphicRelation_triggered()
{
  QgsRelationAddPolymorphicDialog addDlg( true );
  const QModelIndexList rows = mRelationsTree->selectionModel()->selectedRows();

  if ( rows.size() != 1 )
    return;

  const QgsPolymorphicRelation polyRel = mRelationsTree->topLevelItem( rows[0].row() )->data( 0, Qt::UserRole ).value<QgsPolymorphicRelation>();
  addDlg.setPolymorphicRelation( polyRel );

  if ( addDlg.exec() )
  {
    QgsPolymorphicRelation relation;
    relation.setReferencingLayer( addDlg.referencingLayerId() );
    relation.setReferencedLayerField( addDlg.referencedLayerField() );
    relation.setReferencedLayerExpression( addDlg.referencedLayerExpression() );
    relation.setReferencedLayerIds( addDlg.referencedLayerIds() );
    relation.setRelationStrength( addDlg.relationStrength() );

    const auto references = addDlg.fieldPairs();
    for ( const auto &reference : references )
      relation.addFieldPair( reference.first, reference.second );

    const QString relationId = addDlg.relationId();

    if ( relationId.isEmpty() )
      relation.generateId();
    else
      relation.setId( relationId );

    for ( int i = 0, l = mRelationsTree->topLevelItemCount(); i < l; i++ )
    {
      mRelationsTree->topLevelItem( i )->data( 0, Qt::UserRole );
      const QgsPolymorphicRelation currentPolyRel = mRelationsTree->topLevelItem( i )->data( 0, Qt::UserRole ).value<QgsPolymorphicRelation>();

      if ( currentPolyRel.id() == polyRel.id() )
      {
        // we safely assume that polymorphic relations are always a top level item
        mRelationsTree->takeTopLevelItem( i );
        break;
      }
    }

    addPolymorphicRelation( relation );
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
  const QModelIndexList rows = mRelationsTree->selectionModel()->selectedRows();
  for ( int i = rows.size() - 1; i >= 0; --i )
  {
    mRelationsTree->takeTopLevelItem( rows[i].row() );
  }
}

QList< QgsRelation > QgsRelationManagerDialog::relations()
{
  QList< QgsRelation > relations;

  const int rows = mRelationsTree->topLevelItemCount();
  relations.reserve( rows );
  for ( int i = 0; i < rows; ++i )
  {
    QTreeWidgetItem *item = mRelationsTree->topLevelItem( i );

    if ( item->data( 0, Qt::UserRole ).typeName() != QLatin1String( "QgsRelation" ) )
      continue;

    QgsRelation relation = item->data( 0, Qt::UserRole ).value<QgsRelation>();
    // The name can be edited in the table, so apply this one
    relation.setName( item->data( 0, Qt::DisplayRole ).toString() );
    relations << relation;
  }

  return relations;
}

QList< QgsPolymorphicRelation > QgsRelationManagerDialog::polymorphicRelations()
{
  QList< QgsPolymorphicRelation > relations;

  const int rows = mRelationsTree->topLevelItemCount();
  relations.reserve( rows );
  for ( int i = 0; i < rows; ++i )
  {
    QTreeWidgetItem *item = mRelationsTree->topLevelItem( i );

    if ( item->data( 0, Qt::UserRole ).typeName() != QLatin1String( "QgsPolymorphicRelation" ) )
      continue;

    QgsPolymorphicRelation relation = item->data( 0, Qt::UserRole ).value<QgsPolymorphicRelation>();
    // The name can be edited in the table, so apply this one
    relation.setName( item->data( 0, Qt::DisplayRole ).toString() );
    relations << relation;
  }

  return relations;
}

void QgsRelationManagerDialog::onSelectionChanged()
{
  mBtnRemoveRelation->setEnabled( ! mRelationsTree->selectionModel()->selectedRows().isEmpty() );

  const QModelIndexList rows = mRelationsTree->selectionModel()->selectedRows();
  const bool isEditPolymorphicRelationEnabled = (
        rows.size() == 1
        && mRelationsTree->topLevelItem( rows[0].row() )->data( 0, Qt::UserRole ).value<QgsPolymorphicRelation>().isValid()
      );
  mActionEditPolymorphicRelation->setEnabled( isEditPolymorphicRelationEnabled );
}
