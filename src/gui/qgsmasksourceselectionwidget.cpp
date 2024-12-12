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
#include <QPointer>
#include <QScreen>

#include "qgsmasksourceselectionwidget.h"
#include "moc_qgsmasksourceselectionwidget.cpp"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "symbology/qgsrenderer.h"
#include "qgsstyleentityvisitor.h"
#include "symbology/qgssymbollayerutils.h"
#include "qgsguiutils.h"
#include "qgslayertree.h"
#include "qgslayertreelayer.h"
#include "qgsvectorlayerlabeling.h"

static void expandAll( QTreeWidgetItem *item )
{
  for ( int i = 0; i < item->childCount(); i++ )
    expandAll( item->child( i ) );
  item->setExpanded( true );
}

void printSymbolLayerRef( const QgsSymbolLayerReference &ref )
{
  std::cout << ref.layerId().toLocal8Bit().constData() << "/" << ref.symbolLayerIdV2().toLocal8Bit().constData();
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
      SymbolLayerFillVisitor( QTreeWidgetItem *layerItem, const QgsVectorLayer *layer, QHash<QgsSymbolLayerReference, QTreeWidgetItem *> &items, QScreen *screen )
        : mLayerItem( layerItem )
        , mLayer( layer )
        , mItems( items )
        , mScreen( screen )
      {}

      bool visitEnter( const QgsStyleEntityVisitorInterface::Node &node ) override
      {
        if ( node.type != QgsStyleEntityVisitorInterface::NodeType::SymbolRule )
          return false;

        mCurrentDescription = node.description;

        return true;
      }

      struct TreeNode
      {
          TreeNode( const QgsSymbol *_symbol, const QgsSymbolLayer *_sl = nullptr )
            : sl( _sl ), symbol( _symbol ) {};

          const QgsSymbolLayer *sl = nullptr;
          const QgsSymbol *symbol = nullptr;
          QList<TreeNode> children;
      };


      bool visitSymbol( TreeNode &parent, const QString &identifier, const QgsSymbol *symbol, QVector<int> rootPath )
      {
        bool ret = false;
        for ( int idx = 0; idx < symbol->symbolLayerCount(); idx++ )
        {
          QgsSymbolLayer *sl = const_cast<QgsSymbol *>( symbol )->symbolLayer( idx );
          QgsSymbol *subSymbol = sl->subSymbol();

          QVector<int> indexPath = rootPath;
          indexPath.append( idx );

          TreeNode node( symbol, sl );
          if ( ( sl->layerType() == "MaskMarker" ) || ( subSymbol && visitSymbol( node, identifier, subSymbol, indexPath ) ) )
          {
            ret = true;
            parent.children << node;
          }
        }
        return ret;
      }

      bool visit( const QgsStyleEntityVisitorInterface::StyleLeaf &leaf ) override
      {
        if ( !leaf.entity || leaf.entity->type() != QgsStyle::SymbolEntity )
          return true;

        const auto symbolEntity = static_cast<const QgsStyleSymbolEntity *>( leaf.entity );
        const QgsSymbol *symbol = symbolEntity->symbol();
        if ( !symbol )
          return true;

        TreeNode node( symbol );
        if ( visitSymbol( node, leaf.identifier, symbol, {} ) )
          createItems( leaf.description, mLayerItem, node );

        return true;
      }

      void createItems( const QString &leafDescription, QTreeWidgetItem *rootItem, const TreeNode &node )
      {
        QTreeWidgetItem *item = nullptr;
        // root symbol node
        if ( !node.sl )
        {
          item = new QTreeWidgetItem( rootItem, QStringList() << ( mCurrentDescription + leafDescription ) );
          const QIcon icon = QgsSymbolLayerUtils::symbolPreviewIcon( node.symbol, QSize( iconSize, iconSize ), 0, nullptr, QgsScreenProperties( mScreen.data() ) );
          item->setIcon( 0, icon );
        }
        // symbol layer node
        else
        {
          item = new QTreeWidgetItem( rootItem );
          const QIcon slIcon = QgsSymbolLayerUtils::symbolLayerPreviewIcon( node.sl, Qgis::RenderUnit::Millimeters, QSize( iconSize, iconSize ), QgsMapUnitScale(), node.symbol->type(), nullptr, QgsScreenProperties( mScreen.data() ) );
          item->setIcon( 0, slIcon );
          if ( node.sl->layerType() == "MaskMarker" )
          {
            item->setText( 0, QObject::tr( "Mask symbol layer" ) );
            item->setFlags( item->flags() | Qt::ItemIsUserCheckable );
            item->setCheckState( 0, Qt::Unchecked );

            const QgsSymbolLayerReference ref( mLayer->id(), node.sl->id() );
            mItems[ref] = item;
          }
        }

        rootItem->addChild( item );

        for ( TreeNode child : node.children )
          createItems( leafDescription, item, child );
      };

      const int iconSize = QgsGuiUtils::scaleIconSize( 16 );
      QString mCurrentDescription;
      QTreeWidgetItem *mLayerItem;
      const QgsVectorLayer *mLayer;
      QHash<QgsSymbolLayerReference, QTreeWidgetItem *> &mItems;
      QPointer<QScreen> mScreen;
  };

  class LabelMasksVisitor : public QgsStyleEntityVisitorInterface
  {
    public:
      LabelMasksVisitor( QTreeWidgetItem *layerItem, const QgsVectorLayer *layer, QHash<QgsSymbolLayerReference, QTreeWidgetItem *> &items )
        : mLayerItem( layerItem ), mLayer( layer ), mItems( items )
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
            mItems[QgsSymbolLayerReference( "__labels__" + mLayer->id(), currentRule )] = slItem;
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
    QgsMapLayer *layer = layerTreeLayer->layer();
    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer );
    if ( !vl )
      continue;
    if ( !vl->renderer() )
      continue;

    std::unique_ptr<QTreeWidgetItem> layerItem = std::make_unique<QTreeWidgetItem>( mTree, QStringList() << layer->name() );
    layerItem->setData( 0, Qt::UserRole, QVariant::fromValue( vl ) );

    if ( vl->labeling() )
    {
      LabelMasksVisitor lblVisitor( layerItem.get(), vl, mItems );
      vl->labeling()->accept( &lblVisitor );
    }

    SymbolLayerFillVisitor slVisitor( layerItem.get(), vl, mItems, screen() );
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
      source.symbolLayerId = ref.symbolLayerIdV2();
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
