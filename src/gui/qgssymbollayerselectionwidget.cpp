/***************************************************************************
    qgssymbollayerselectionwidget.h
    ---------------------
    begin                : July 2019
    copyright            : (C) 2019 by Hugo Mercier
    email                : hugo dot mercier at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QTreeWidget>
#include <QVBoxLayout>

#include "qgssymbollayerselectionwidget.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgslegendsymbolitem.h"
#include "symbology/qgsrenderer.h"
#include "qgsstyleentityvisitor.h"
#include "symbology/qgssymbollayerutils.h"
#include "qgsguiutils.h"
#include "qgslayertree.h"
#include "qgslayertreelayer.h"

QgsSymbolLayerSelectionWidget::QgsSymbolLayerSelectionWidget( QWidget *parent )
  : QWidget( parent )
{
  mTree = new QTreeWidget( this );
  mTree->setHeaderHidden( true );

  connect( mTree, &QTreeWidget::itemChanged, this, [&]( QTreeWidgetItem *, int ) { emit this->changed(); } );

  // place the tree in a layout
  QVBoxLayout *vbox = new QVBoxLayout();
  vbox->setContentsMargins( 0, 0, 0, 0 );
  vbox->addWidget( mTree );

  setLayout( vbox );
}

void QgsSymbolLayerSelectionWidget::setLayer( const QgsVectorLayer *layer )
{
  mLayer = layer;
  mItems.clear();
  mTree->clear();

  class TreeFillVisitor : public QgsStyleEntityVisitorInterface
  {
    public:
      TreeFillVisitor( QTreeWidgetItem *layerItem, const QgsVectorLayer *layer, QHash<QgsSymbolLayerId, QTreeWidgetItem *> &items ):
        mLayerItem( layerItem ), mLayer( layer ), mItems( items )
      {}

      bool visitEnter( const QgsStyleEntityVisitorInterface::Node &node ) override
      {
        if ( node.type != QgsStyleEntityVisitorInterface::NodeType::SymbolRule )
          return false;

        mCurrentIdentifier = node.identifier;
        mCurrentDescription = node.description;

        return true;
      }

      void visitSymbol( QTreeWidgetItem *rootItem, const QString &identifier, const QgsSymbol *symbol, QVector<int> rootPath )
      {
        for ( int idx = 0; idx < symbol->symbolLayerCount(); idx++ )
        {
          const QgsSymbolLayer *sl = symbol->symbolLayer( idx );
          // Skip mask symbol layers. It makes no sense to take them as mask targets.
          if ( sl->layerType() == "MaskMarker" )
            continue;

          const QgsSymbol *subSymbol = const_cast<QgsSymbolLayer *>( sl )->subSymbol();

          QVector<int> indexPath = rootPath;
          indexPath.append( idx );

          QTreeWidgetItem *slItem = new QTreeWidgetItem();
          const QIcon slIcon = QgsSymbolLayerUtils::symbolLayerPreviewIcon( sl, QgsUnitTypes::RenderMillimeters, QSize( iconSize, iconSize ), QgsMapUnitScale(), symbol->type() );
          slItem->setData( 0, Qt::UserRole, idx );
          slItem->setIcon( 0, slIcon );
          auto flags = slItem->flags();
          if ( ! subSymbol || subSymbol->symbolLayerCount() == 0 )
          {
            flags.setFlag( Qt::ItemIsUserCheckable, true );
            slItem->setCheckState( 0, Qt::Unchecked );
          }
          else
          {
            flags.setFlag( Qt::ItemIsUserCheckable, false );
          }
          slItem->setFlags( flags );
          rootItem->addChild( slItem );
          slItem->setExpanded( true );

          mItems[QgsSymbolLayerId( mCurrentIdentifier + identifier, indexPath )] = slItem;

          if ( subSymbol )
          {
            visitSymbol( slItem, identifier, subSymbol, indexPath );
          }
        }
      }

      bool visit( const QgsStyleEntityVisitorInterface::StyleLeaf &leaf ) override
      {
        if ( ! leaf.entity || leaf.entity->type() != QgsStyle::SymbolEntity )
          return true;

        const auto symbolEntity = static_cast<const QgsStyleSymbolEntity *>( leaf.entity );
        const QgsSymbol *symbol = symbolEntity->symbol();
        if ( ! symbol )
          return true;

        // either leaf.description or mCurrentDescription is defined
        QTreeWidgetItem *symbolItem = new QTreeWidgetItem( QStringList() << ( mCurrentDescription + leaf.description ) );
        const QIcon icon = QgsSymbolLayerUtils::symbolPreviewIcon( symbol, QSize( iconSize, iconSize ) );
        symbolItem->setData( 0, Qt::UserRole, mCurrentIdentifier );
        symbolItem->setIcon( 0, icon );
        mLayerItem->addChild( symbolItem );
        symbolItem->setExpanded( true );

        visitSymbol( symbolItem, leaf.identifier, symbol, {} );

        return true;
      }

      const int iconSize = QgsGuiUtils::scaleIconSize( 16 );
      QString mCurrentDescription;
      QString mCurrentIdentifier;
      QTreeWidgetItem *mLayerItem;
      const QgsVectorLayer *mLayer;
      QHash<QgsSymbolLayerId, QTreeWidgetItem *> &mItems;
  };

  // populate the tree
  if ( ! mLayer )
    return;
  if ( ! mLayer->renderer() )
    return;

  TreeFillVisitor visitor( mTree->invisibleRootItem(), mLayer, mItems );
  mLayer->renderer()->accept( &visitor );
}

QSet<QgsSymbolLayerId> QgsSymbolLayerSelectionWidget::selection() const
{
  QSet<QgsSymbolLayerId> sel;
  for ( auto it = mItems.begin(); it != mItems.end(); it++ )
  {
    if ( it.value()->checkState( 0 ) == Qt::Checked )
      sel.insert( it.key() );
  }
  return sel;
}

void QgsSymbolLayerSelectionWidget::setSelection( const QSet<QgsSymbolLayerId> &sel )
{
  // clear selection
  for ( auto it = mItems.begin(); it != mItems.end(); it++ )
  {
    if ( it.value()->flags() & Qt::ItemIsUserCheckable )
      it.value()->setCheckState( 0, Qt::Unchecked );
  }

  // apply selection passed in parameter
  for ( const QgsSymbolLayerId &lid : sel )
  {
    const auto it = mItems.find( lid );
    if ( it != mItems.end() )
      ( *it )->setCheckState( 0, Qt::Checked );
  }
}
