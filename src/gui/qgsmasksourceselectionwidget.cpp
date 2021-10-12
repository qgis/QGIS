/***************************************************************************
    qgsmasksourceselectionwidget.cpp
    ---------------------
    begin                : September 2019
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

#include "qgsmasksourceselectionwidget.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgslegendsymbolitem.h"
#include "symbology/qgsrenderer.h"
#include "qgsstyleentityvisitor.h"
#include "symbology/qgssymbollayerutils.h"
#include "qgsguiutils.h"
#include "qgslayertree.h"
#include "qgslayertreelayer.h"
#include "qgsvectorlayerlabeling.h"
#include "qgsvectorlayerutils.h"
#include "symbology/qgsmasksymbollayer.h"

static void expandAll( QTreeWidgetItem *item )
{
  for ( int i = 0; i < item->childCount(); i++ )
    expandAll( item->child( i ) );
  item->setExpanded( true );
}

void printSymbolLayerId( const QgsSymbolLayerId &lid )
{
  std::cout << lid.symbolKey().toLocal8Bit().constData() << "/";
  QVector<int> path = lid.symbolLayerIndexPath();
  for ( int i = 0; i < path.size(); i++ )
  {
    std::cout << path[i] << "/";
  }
}

void printSymbolLayerRef( const QgsSymbolLayerReference &ref )
{
  std::cout << ref.layerId().toLocal8Bit().constData() << "/";
  printSymbolLayerId( ref.symbolLayerId() );
}

QgsMaskSourceSelectionWidget::QgsMaskSourceSelectionWidget( QWidget *parent )
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

void QgsMaskSourceSelectionWidget::update()
{
  mTree->clear();
  mItems.clear();

  class SymbolLayerFillVisitor : public QgsStyleEntityVisitorInterface
  {
    public:
      SymbolLayerFillVisitor( QTreeWidgetItem *layerItem, const QgsVectorLayer *layer, QHash<QgsSymbolLayerReference, QTreeWidgetItem *> &items ):
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

      bool visitSymbol( QTreeWidgetItem *rootItem, const QString &identifier, const QgsSymbol *symbol, QVector<int> rootPath )
      {
        bool ret = false;
        for ( int idx = 0; idx < symbol->symbolLayerCount(); idx++ )
        {
          QgsSymbolLayer *sl = const_cast<QgsSymbol *>( symbol )->symbolLayer( idx );
          QgsSymbol *subSymbol = sl->subSymbol();

          QVector<int> indexPath = rootPath;
          indexPath.append( idx );

          std::unique_ptr< QTreeWidgetItem > slItem = std::make_unique< QTreeWidgetItem >( rootItem );
          const QIcon slIcon = QgsSymbolLayerUtils::symbolLayerPreviewIcon( sl, QgsUnitTypes::RenderMillimeters, QSize( iconSize, iconSize ), QgsMapUnitScale(), symbol->type() );
          slItem->setIcon( 0, slIcon );
          if ( sl->layerType() == "MaskMarker" )
          {
            slItem->setText( 0, QObject::tr( "Mask symbol layer" ) );
            slItem->setFlags( slItem->flags() | Qt::ItemIsUserCheckable );
            slItem->setCheckState( 0, Qt::Unchecked );
          }

          if ( ( sl->layerType() == "MaskMarker" ) ||
               ( subSymbol && visitSymbol( slItem.get(), identifier, subSymbol, indexPath ) ) )
          {
            const QgsSymbolLayerReference ref( mLayer->id(), QgsSymbolLayerId( mCurrentIdentifier + identifier, indexPath ) );
            mItems[ref] = slItem.get();
            rootItem->addChild( slItem.release() );
            ret = true;
          }
        }
        return ret;
      }

      bool visit( const QgsStyleEntityVisitorInterface::StyleLeaf &leaf ) override
      {
        if ( ! leaf.entity || leaf.entity->type() != QgsStyle::SymbolEntity )
          return true;

        const auto symbolEntity = static_cast<const QgsStyleSymbolEntity *>( leaf.entity );
        const QgsSymbol *symbol = symbolEntity->symbol();
        if ( ! symbol )
          return true;

        std::unique_ptr< QTreeWidgetItem > symbolItem = std::make_unique< QTreeWidgetItem >( mLayerItem, QStringList() << ( mCurrentDescription + leaf.description ) );
        const QIcon icon = QgsSymbolLayerUtils::symbolPreviewIcon( symbol, QSize( iconSize, iconSize ) );
        symbolItem->setIcon( 0, icon );

        if ( visitSymbol( symbolItem.get(), leaf.identifier, symbol, {} ) )
          mLayerItem->addChild( symbolItem.release() );

        return true;
      }

      const int iconSize = QgsGuiUtils::scaleIconSize( 16 );
      QString mCurrentDescription;
      QString mCurrentIdentifier;
      QTreeWidgetItem *mLayerItem;
      const QgsVectorLayer *mLayer;
      QHash<QgsSymbolLayerReference, QTreeWidgetItem *> &mItems;
  };

  class LabelMasksVisitor : public QgsStyleEntityVisitorInterface
  {
    public:
      LabelMasksVisitor( QTreeWidgetItem *layerItem, const QgsVectorLayer *layer, QHash<QgsSymbolLayerReference, QTreeWidgetItem *> &items ):
        mLayerItem( layerItem ), mLayer( layer ), mItems( items )
      {}
      bool visitEnter( const QgsStyleEntityVisitorInterface::Node &node ) override
      {
        if ( node.type == QgsStyleEntityVisitorInterface::NodeType::SymbolRule )
        {
          currentRule = node.identifier;
          currentDescription = node.description;
          return true;
        }
        return false;
      }
      bool visit( const QgsStyleEntityVisitorInterface::StyleLeaf &leaf ) override
      {
        if ( leaf.entity && leaf.entity->type() == QgsStyle::LabelSettingsEntity )
        {
          auto labelSettingsEntity = static_cast<const QgsStyleLabelSettingsEntity *>( leaf.entity );
          if ( labelSettingsEntity->settings().format().mask().enabled() )
          {
            const QString maskTitle = currentRule.isEmpty()
                                      ? QObject::tr( "Label mask" )
                                      : QObject::tr( "Label mask for '%1' rule" ).arg( currentDescription );
            QTreeWidgetItem *slItem = new QTreeWidgetItem( mLayerItem, QStringList() << maskTitle );
            slItem->setFlags( slItem->flags() | Qt::ItemIsUserCheckable );
            slItem->setCheckState( 0, Qt::Unchecked );
            mLayerItem->addChild( slItem );
            mItems[QgsSymbolLayerReference( "__labels__" + mLayer->id(), { currentRule, 0 } )] = slItem;
          }
        }
        return true;
      }

      QHash<QString, QHash<QString, QSet<QgsSymbolLayerId>>> masks;
      // Current label rule, empty string for a simple labeling
      QString currentRule;
      QString currentDescription;
      QTreeWidgetItem *mLayerItem;
      const QgsVectorLayer *mLayer;
      QHash<QgsSymbolLayerReference, QTreeWidgetItem *> &mItems;
  };

  // populate the tree
  const auto layers = QgsProject::instance()->layerTreeRoot()->findLayers();
  for ( const QgsLayerTreeLayer *layerTreeLayer : layers )
  {
    const QgsMapLayer *layer = layerTreeLayer->layer();
    const QgsVectorLayer *vl = qobject_cast<const QgsVectorLayer *>( layer );
    if ( ! vl )
      continue;
    if ( ! vl->renderer() )
      continue;

    std::unique_ptr< QTreeWidgetItem > layerItem = std::make_unique< QTreeWidgetItem >( mTree, QStringList() << layer->name() );
    layerItem->setData( 0, Qt::UserRole, vl );

    if ( vl->labeling() )
    {
      LabelMasksVisitor lblVisitor( layerItem.get(), vl, mItems );
      vl->labeling()->accept( &lblVisitor );
    }

    SymbolLayerFillVisitor slVisitor( layerItem.get(), vl, mItems );
    vl->renderer()->accept( &slVisitor );

    if ( layerItem->childCount() > 0 )
      mTree->addTopLevelItem( layerItem.release() );
  }

  expandAll( mTree->invisibleRootItem() );
}

//! Returns the current selection
QList<QgsMaskSourceSelectionWidget::MaskSource> QgsMaskSourceSelectionWidget::selection() const
{
  QList<QgsMaskSourceSelectionWidget::MaskSource> sel;
  for ( auto it = mItems.begin(); it != mItems.end(); it++ )
  {
    if ( it.value()->checkState( 0 ) == Qt::Checked )
    {
      const QgsSymbolLayerReference &ref = it.key();
      QgsMaskSourceSelectionWidget::MaskSource source;
      source.isLabeling = ref.layerId().startsWith( "__labels__" );
      source.layerId = source.isLabeling ? ref.layerId().mid( 10 ) : ref.layerId();
      source.symbolLayerId = ref.symbolLayerId();
      sel.append( source );
    }
  }
  return sel;
}

//! Sets the symbol layer selection
void QgsMaskSourceSelectionWidget::setSelection( const QList<QgsMaskSourceSelectionWidget::MaskSource> &sel )
{
  // Clear current selection
  for ( auto it = mItems.begin(); it != mItems.end(); it++ )
  {
    it.value()->setCheckState( 0, Qt::Unchecked );
  }

  for ( const MaskSource &src : sel )
  {
    const QString layerId = ( src.isLabeling ? "__labels__" : "" ) + src.layerId;
    const auto it = mItems.find( QgsSymbolLayerReference( layerId, src.symbolLayerId ) );
    if ( it != mItems.end() )
    {
      it.value()->setCheckState( 0, Qt::Checked );
    }
  }
}

