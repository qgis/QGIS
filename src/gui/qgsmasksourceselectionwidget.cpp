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

#include "qgsmasksourceselectionwidget.h"

#include "qgsguiutils.h"
#include "qgslayertree.h"
#include "qgslayertreelayer.h"
#include "qgsnewnamedialog.h"
#include "qgsproject.h"
#include "qgsrenderer.h"
#include "qgsselectivemaskingsource.h"
#include "qgsselectivemaskingsourceset.h"
#include "qgsselectivemaskingsourcesetmanager.h"
#include "qgsselectivemaskingsourcesetmanagermodel.h"
#include "qgsstyleentityvisitor.h"
#include "qgssymbollayerutils.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerlabeling.h"

#include <QAction>
#include <QComboBox>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QPointer>
#include <QScreen>
#include <QString>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

#include "moc_qgsmasksourceselectionwidget.cpp"

using namespace Qt::StringLiterals;

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
  QVBoxLayout *vbox = new QVBoxLayout();
  vbox->setContentsMargins( 0, 0, 0, 0 );

  QHBoxLayout *hLayout = new QHBoxLayout();

  mSetComboBox = new QComboBox();
  mManagerModel = new QgsSelectiveMaskingSourceSetManagerModel( QgsProject::instance()->selectiveMaskingSourceSetManager(), this );
  mManagerModel->setAllowEmptyObject( true );
  mManagerProxyModel = new QgsSelectiveMaskingSourceSetManagerProxyModel( this );
  mManagerProxyModel->setSourceModel( mManagerModel );
  mSetComboBox->setModel( mManagerProxyModel );
  connect( mSetComboBox, qOverload< int >( &QComboBox::currentIndexChanged ), this, &QgsMaskSourceSelectionWidget::selectedSetChanged );

  mSetsToolButton = new QToolButton();
  mSetsToolButton->setText( u"…"_s );
  mSetsToolButton->setPopupMode( QToolButton::InstantPopup );

  QMenu *toolMenu = new QMenu( mSetsToolButton );
  connect( toolMenu, &QMenu::aboutToShow, this, [this] {
    const bool isCustomSelected = isCustomSet();
    mRemoveAction->setEnabled( !isCustomSelected );
    mRenameAction->setEnabled( !isCustomSelected );
  } );
  QAction *addAction = new QAction( tr( "New Selective Masking Set…" ), toolMenu );
  connect( addAction, &QAction::triggered, this, &QgsMaskSourceSelectionWidget::newSet );
  toolMenu->addAction( addAction );
  mRemoveAction = new QAction( tr( "Remove Set…" ), toolMenu );
  connect( mRemoveAction, &QAction::triggered, this, &QgsMaskSourceSelectionWidget::removeSet );
  toolMenu->addAction( mRemoveAction );
  mRenameAction = new QAction( tr( "Rename Set…" ), toolMenu );
  connect( mRenameAction, &QAction::triggered, this, &QgsMaskSourceSelectionWidget::renameSet );
  toolMenu->addAction( mRenameAction );
  mSetsToolButton->setMenu( toolMenu );

  hLayout->addWidget( new QLabel( tr( "Preset" ) ) );
  hLayout->addWidget( mSetComboBox, 1 );
  hLayout->addWidget( mSetsToolButton, 1 );
  vbox->addLayout( hLayout );

  mTree = new QTreeWidget( this );
  mTree->setHeaderHidden( true );

  connect( mTree, &QTreeWidget::itemChanged, this, &QgsMaskSourceSelectionWidget::onItemChanged );

  vbox->addWidget( mTree, 1 );

  setLayout( vbox );
}

void QgsMaskSourceSelectionWidget::update()
{
  mBlockChangedSignals++;
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

    auto layerItem = std::make_unique<QTreeWidgetItem>( mTree, QStringList() << layer->name() );
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
  mBlockChangedSignals--;
}

QgsSelectiveMaskingSourceSet QgsMaskSourceSelectionWidget::sourceSet() const
{
  QgsSelectiveMaskingSourceSet set;

  const QModelIndex selectedIndex = mManagerProxyModel->mapToSource( mManagerProxyModel->index( mSetComboBox->currentIndex(), 0 ) );
  const bool isCustomSet = mManagerModel->data( selectedIndex, static_cast< int >( QgsSelectiveMaskingSourceSetManagerModel::CustomRole::IsEmptyObject ) ).toBool();
  if ( !isCustomSet )
  {
    set.setName( mManagerModel->data( selectedIndex, Qt::DisplayRole ).toString() );
    set.setId( mManagerModel->data( selectedIndex, static_cast< int >( QgsSelectiveMaskingSourceSetManagerModel::CustomRole::SetId ) ).toString() );
  }

  for ( auto it = mItems.begin(); it != mItems.end(); it++ )
  {
    if ( it.value()->checkState( 0 ) == Qt::Checked )
    {
      const QgsSymbolLayerReference &ref = it.key();
      QgsSelectiveMaskSource source;
      const bool isLabeling = ref.layerId().startsWith( "__labels__" );
      if ( isLabeling )
      {
        source.setSourceType( Qgis::SelectiveMaskSourceType::Label );
        source.setLayerId( ref.layerId().mid( 10 ) );
      }
      else
      {
        source.setSourceType( Qgis::SelectiveMaskSourceType::SymbolLayer );
        source.setLayerId( ref.layerId() );
      }
      source.setSourceId( ref.symbolLayerIdV2() );
      set.append( source );
    }
  }
  return set;
}

void QgsMaskSourceSelectionWidget::setSourceSet( const QgsSelectiveMaskingSourceSet &set )
{
  // Clear current selection
  mBlockChangedSignals++;
  for ( auto it = mItems.begin(); it != mItems.end(); it++ )
  {
    it.value()->setCheckState( 0, Qt::Unchecked );
  }

  const QVector< QgsSelectiveMaskSource > sources = set.sources();
  for ( const QgsSelectiveMaskSource &source : sources )
  {
    QString layerId;
    switch ( source.sourceType() )
    {
      case Qgis::SelectiveMaskSourceType::SymbolLayer:
        layerId = source.layerId();
        break;
      case Qgis::SelectiveMaskSourceType::Label:
        layerId = u"__labels__%1"_s.arg( source.layerId() );
        break;
    }
    const auto it = mItems.find( QgsSymbolLayerReference( layerId, source.sourceId() ) );
    if ( it != mItems.end() )
    {
      it.value()->setCheckState( 0, Qt::Checked );
    }
  }
  if ( !set.isValid() )
  {
    mSetComboBox->setCurrentIndex( 0 );
  }
  else
  {
    mSetComboBox->setCurrentIndex( mSetComboBox->findText( set.name() ) );
  }
  mBlockChangedSignals--;
}

void QgsMaskSourceSelectionWidget::selectedSetChanged()
{
  mBlockChangedSignals++;
  if ( !isCustomSet() )
  {
    const QModelIndex selectedIndex = mManagerProxyModel->mapToSource( mManagerProxyModel->index( mSetComboBox->currentIndex(), 0 ) );
    const QString selectedSetId = mManagerModel->data( selectedIndex, static_cast< int >( QgsSelectiveMaskingSourceSetManagerModel::CustomRole::SetId ) ).toString();
    const QgsSelectiveMaskingSourceSet set = QgsProject::instance()->selectiveMaskingSourceSetManager()->setById( selectedSetId );
    setSourceSet( set );
  }
  mBlockChangedSignals--;

  emitChanged();
}

void QgsMaskSourceSelectionWidget::emitChanged()
{
  if ( !mBlockChangedSignals )
  {
    emit changed();
  }
}

void QgsMaskSourceSelectionWidget::onItemChanged()
{
  if ( mBlockChangedSignals )
    return;

  if ( !isCustomSet() )
  {
    const QgsSelectiveMaskingSourceSet set = sourceSet();
    QgsProject::instance()->selectiveMaskingSourceSetManager()->updateSet( set );
  }
  emitChanged();
}

void QgsMaskSourceSelectionWidget::newSet()
{
  QString newTitle = QgsProject::instance()->selectiveMaskingSourceSetManager()->generateUniqueTitle();

  QStringList existingNames;
  const QVector<QgsSelectiveMaskingSourceSet> sets = QgsProject::instance()->selectiveMaskingSourceSetManager()->sets();
  existingNames.reserve( sets.size() );
  for ( const QgsSelectiveMaskingSourceSet &set : sets )
  {
    existingNames << set.name();
  }

  QgsNewNameDialog dlg( tr( "selective masking set" ), newTitle, QStringList(), existingNames, Qt::CaseSensitive, this );
  dlg.setWindowTitle( tr( "Create Selective Masking Set" ) );
  dlg.setHintString( tr( "Enter a unique selective masking set name" ) );
  dlg.setOverwriteEnabled( false );
  dlg.setAllowEmptyName( false );
  dlg.setConflictingNameWarning( tr( "A masking set with this name already exists." ) );

  if ( dlg.exec() != QDialog::Accepted )
  {
    return;
  }

  QgsSelectiveMaskingSourceSet newSet;
  newSet.setName( dlg.name() );
  QgsProject::instance()->selectiveMaskingSourceSetManager()->addSet( newSet );
  mSetComboBox->setCurrentIndex( mSetComboBox->findText( newSet.name() ) );
}

void QgsMaskSourceSelectionWidget::removeSet()
{
  if ( isCustomSet() )
    return;

  const QString setName = mSetComboBox->currentText();
  if ( QMessageBox::warning( this, tr( "Remove Selective Masking Set" ), tr( "Do you really want to remove the selective masking set “%1”?" ).arg( setName ), QMessageBox::Ok | QMessageBox::Cancel ) != QMessageBox::Ok )
  {
    return;
  }

  mSetComboBox->setCurrentIndex( 0 );
  QgsProject::instance()->selectiveMaskingSourceSetManager()->removeSet( setName );
}

void QgsMaskSourceSelectionWidget::renameSet()
{
  if ( isCustomSet() )
    return;

  const QString setName = mSetComboBox->currentText();

  QStringList existingNames;
  const QVector<QgsSelectiveMaskingSourceSet> sets = QgsProject::instance()->selectiveMaskingSourceSetManager()->sets();
  existingNames.reserve( sets.size() );
  for ( const QgsSelectiveMaskingSourceSet &set : sets )
  {
    if ( set.name() != setName )
      existingNames << set.name();
  }

  QgsNewNameDialog dlg( tr( "selective masking set" ), setName, QStringList(), existingNames, Qt::CaseSensitive, this );
  dlg.setWindowTitle( tr( "Rename Selective Masking Set" ) );
  dlg.setHintString( tr( "Enter a unique selective masking set name" ) );
  dlg.setOverwriteEnabled( false );
  dlg.setAllowEmptyName( false );
  dlg.setConflictingNameWarning( tr( "A masking set with this name already exists." ) );

  if ( dlg.exec() != QDialog::Accepted )
  {
    return;
  }

  QgsProject::instance()->selectiveMaskingSourceSetManager()->renameSet( setName, dlg.name() );
}

bool QgsMaskSourceSelectionWidget::isCustomSet() const
{
  const QModelIndex selectedIndex = mManagerProxyModel->mapToSource( mManagerProxyModel->index( mSetComboBox->currentIndex(), 0 ) );
  return mManagerModel->data( selectedIndex, static_cast< int >( QgsSelectiveMaskingSourceSetManagerModel::CustomRole::IsEmptyObject ) ).toBool();
}
