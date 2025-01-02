/***************************************************************************
                         qgsdxfexportdialog.cpp
                         ----------------------
    begin                : September 2013
    copyright            : (C) 2013 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdxfexportdialog.h"
#include "moc_qgsdxfexportdialog.cpp"

#include "qgsmaplayer.h"
#include "qgslayertree.h"
#include "qgslayertreegroup.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgshelp.h"
#include "qgis.h"
#include "qgsfieldcombobox.h"
#include "qgisapp.h"
#include "qgsmapthemecollection.h"
#include "qgsmapcanvas.h"
#include "qgssettings.h"
#include "qgsgui.h"

#include <QFileDialog>
#include <QPushButton>
#include <QMessageBox>

const int LAYER_COL = 0;
const int OUTPUT_LAYER_ATTRIBUTE_COL = 1;
const int ALLOW_DD_SYMBOL_BLOCKS_COL = 2;
const int MAXIMUM_DD_SYMBOL_BLOCKS_COL = 3;

FieldSelectorDelegate::FieldSelectorDelegate( QObject *parent )
  : QItemDelegate( parent )
{
}

QWidget *FieldSelectorDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( option )

  QgsVectorLayer *vl = indexToLayer( index.model(), index );
  if ( !vl )
    return nullptr;

  if ( index.column() == LAYER_COL )
  {
    QgsFilterLineEdit *le = new QgsFilterLineEdit( parent, vl->name() );

    return le;
  }
  else if ( index.column() == OUTPUT_LAYER_ATTRIBUTE_COL )
  {
    QgsFieldComboBox *w = new QgsFieldComboBox( parent );
    w->setLayer( vl );
    w->setAllowEmptyFieldName( true );
    return w;
  }
  else if ( index.column() == ALLOW_DD_SYMBOL_BLOCKS_COL )
  {
    return nullptr;
  }
  else if ( index.column() == MAXIMUM_DD_SYMBOL_BLOCKS_COL )
  {
    QLineEdit *le = new QLineEdit( parent );
    le->setValidator( new QIntValidator( le ) );
    return le;
  }

  return nullptr;
}

void FieldSelectorDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  QgsVectorLayer *vl = indexToLayer( index.model(), index );
  if ( !vl )
    return;

  if ( index.column() == LAYER_COL )
  {
    QgsFilterLineEdit *le = qobject_cast<QgsFilterLineEdit *>( editor );
    if ( le )
    {
      le->setText( index.data().toString() );
    }
  }
  else if ( index.column() == OUTPUT_LAYER_ATTRIBUTE_COL )
  {
    QgsFieldComboBox *fcb = qobject_cast<QgsFieldComboBox *>( editor );
    if ( !fcb )
      return;

    int idx = attributeIndex( index.model(), vl );
    if ( vl->fields().exists( idx ) )
    {
      fcb->setField( vl->fields().at( idx ).name() );
    }
  }
  else if ( index.column() == MAXIMUM_DD_SYMBOL_BLOCKS_COL )
  {
    QLineEdit *le = qobject_cast<QLineEdit *>( editor );
    if ( le )
    {
      le->setText( index.data().toString() );
    }
  }
}

void FieldSelectorDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QgsVectorLayer *vl = indexToLayer( index.model(), index );
  if ( !vl )
    return;

  if ( index.column() == LAYER_COL )
  {
    QgsFilterLineEdit *le = qobject_cast<QgsFilterLineEdit *>( editor );
    if ( le )
    {
      model->setData( index, le->text() );
    }
  }
  else if ( index.column() == OUTPUT_LAYER_ATTRIBUTE_COL )
  {
    QgsFieldComboBox *fcb = qobject_cast<QgsFieldComboBox *>( editor );
    if ( !fcb )
      return;

    model->setData( index, vl->fields().lookupField( fcb->currentField() ) );
  }
  else if ( index.column() == MAXIMUM_DD_SYMBOL_BLOCKS_COL )
  {
    QLineEdit *le = qobject_cast<QLineEdit *>( editor );
    if ( le )
    {
      model->setData( index, le->text().toInt() );
    }
  }
}

QgsVectorLayer *FieldSelectorDelegate::indexToLayer( const QAbstractItemModel *model, const QModelIndex &index ) const
{
  const QgsLayerTreeProxyModel *proxy = qobject_cast<const QgsLayerTreeProxyModel *>( model );
  Q_ASSERT( proxy );

  const QgsVectorLayerAndAttributeModel *m = qobject_cast<const QgsVectorLayerAndAttributeModel *>( proxy->sourceModel() );
  Q_ASSERT( m );

  return m->vectorLayer( proxy->mapToSource( index ) );
}

int FieldSelectorDelegate::attributeIndex( const QAbstractItemModel *model, const QgsVectorLayer *vl ) const
{
  const QgsLayerTreeProxyModel *proxy = qobject_cast<const QgsLayerTreeProxyModel *>( model );
  Q_ASSERT( proxy );

  const QgsVectorLayerAndAttributeModel *m = qobject_cast<const QgsVectorLayerAndAttributeModel *>( proxy->sourceModel() );
  Q_ASSERT( m );

  return m->attributeIndex( vl );
}

QgsVectorLayerAndAttributeModel::QgsVectorLayerAndAttributeModel( QgsLayerTree *rootNode, QObject *parent )
  : QgsLayerTreeModel( rootNode, parent )
{
  //init mCreateDDBlockInfo, mDDBlocksMaxNumberOfClasses
  QSet<QString> layerIds;
  retrieveAllLayers( rootNode, layerIds );
  for ( const auto &id : std::as_const( layerIds ) )
  {
    const QgsVectorLayer *vLayer = qobject_cast<const QgsVectorLayer *>( QgsProject::instance()->mapLayer( id ) );
    if ( vLayer )
    {
      mCreateDDBlockInfo[vLayer] = DEFAULT_DXF_DATA_DEFINED_BLOCKS;
      mDDBlocksMaxNumberOfClasses[vLayer] = -1;
    }
  }
}

int QgsVectorLayerAndAttributeModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 4;
}

Qt::ItemFlags QgsVectorLayerAndAttributeModel::flags( const QModelIndex &index ) const
{
  QgsVectorLayer *vl = vectorLayer( index );
  if ( index.column() == LAYER_COL )
  {
    return vl ? Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable : Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
  }
  else if ( index.column() == OUTPUT_LAYER_ATTRIBUTE_COL )
  {
    return vl ? Qt::ItemIsEnabled | Qt::ItemIsEditable : Qt::ItemIsEnabled;
  }
  else if ( index.column() == ALLOW_DD_SYMBOL_BLOCKS_COL )
  {
    return ( vl && vl->geometryType() == Qgis::GeometryType::Point ) ? Qt::ItemIsEnabled | Qt::ItemIsUserCheckable : Qt::ItemIsEnabled;
  }
  else if ( index.column() == MAXIMUM_DD_SYMBOL_BLOCKS_COL )
  {
    return vl && vl->geometryType() == Qgis::GeometryType::Point ? Qt::ItemIsEnabled | Qt::ItemIsEditable : Qt::ItemIsEnabled;
  }
  return Qt::ItemIsEnabled;
}

QgsVectorLayer *QgsVectorLayerAndAttributeModel::vectorLayer( const QModelIndex &idx ) const
{
  QgsLayerTreeNode *n = index2node( idx );
  if ( !n || !QgsLayerTree::isLayer( n ) )
    return nullptr;

  return qobject_cast<QgsVectorLayer *>( QgsLayerTree::toLayer( n )->layer() );
}

int QgsVectorLayerAndAttributeModel::attributeIndex( const QgsVectorLayer *vl ) const
{
  return mAttributeIdx.value( vl, -1 );
}

QVariant QgsVectorLayerAndAttributeModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Horizontal )
  {
    if ( role == Qt::DisplayRole )
    {
      if ( section == LAYER_COL )
        return tr( "Layer" );
      else if ( section == OUTPUT_LAYER_ATTRIBUTE_COL )
        return tr( "Output Layer Attribute" );
      else if ( section == ALLOW_DD_SYMBOL_BLOCKS_COL )
      {
        return tr( "Allow data defined symbol blocks" );
      }
      else if ( section == MAXIMUM_DD_SYMBOL_BLOCKS_COL )
      {
        return tr( "Maximum number of symbol blocks" );
      }
    }
    else if ( role == Qt::ToolTipRole )
    {
      if ( section == OUTPUT_LAYER_ATTRIBUTE_COL )
        return tr( "Attribute containing the name of the destination layer in the DXF output." );
    }
  }
  return QVariant();
}

QVariant QgsVectorLayerAndAttributeModel::data( const QModelIndex &idx, int role ) const
{
  QgsVectorLayer *vl = vectorLayer( idx );

  if ( idx.column() == LAYER_COL )
  {
    if ( role == Qt::CheckStateRole )
    {
      if ( !idx.isValid() )
        return QVariant();

      if ( mCheckedLeafs.contains( idx ) )
        return Qt::Checked;

      bool hasChecked = false, hasUnchecked = false;
      int n;
      for ( n = 0; !hasChecked || !hasUnchecked; n++ )
      {
        QVariant v = data( index( n, 0, idx ), role );
        if ( !v.isValid() )
          break;

        switch ( v.toInt() )
        {
          case Qt::PartiallyChecked:
            // parent of partially checked child shared state
            return Qt::PartiallyChecked;

          case Qt::Checked:
            hasChecked = true;
            break;

          case Qt::Unchecked:
            hasUnchecked = true;
            break;
        }
      }

      // unchecked leaf
      if ( n == 0 )
        return Qt::Unchecked;

      // both
      if ( hasChecked && hasUnchecked )
        return Qt::PartiallyChecked;

      if ( hasChecked )
        return Qt::Checked;

      Q_ASSERT( hasUnchecked );
      return Qt::Unchecked;
    }
    else if ( role == Qt::DisplayRole && vl && mOverriddenName.contains( vl ) )
    {
      return mOverriddenName[vl];
    }
    else
    {
      return QgsLayerTreeModel::data( idx, role );
    }
  }
  else if ( idx.column() == OUTPUT_LAYER_ATTRIBUTE_COL && vl )
  {
    int idx = mAttributeIdx.value( vl, -1 );
    if ( role == Qt::EditRole )
      return idx;

    if ( role == Qt::DisplayRole )
    {
      if ( vl->fields().exists( idx ) )
        return vl->fields().at( idx ).name();
      else
        return mOverriddenName.contains( vl ) ? mOverriddenName[vl] : vl->name();
    }

    if ( role == Qt::ToolTipRole )
    {
      return tr( "Attribute containing the name of the destination layer in the DXF output." );
    }
  }
  else if ( idx.column() == ALLOW_DD_SYMBOL_BLOCKS_COL )
  {
    if ( !vl || vl->geometryType() != Qgis::GeometryType::Point )
    {
      return QVariant();
    }

    bool checked = mCreateDDBlockInfo.contains( vl ) ? mCreateDDBlockInfo[vl] : DEFAULT_DXF_DATA_DEFINED_BLOCKS;
    if ( role == Qt::CheckStateRole )
    {
      return checked ? Qt::Checked : Qt::Unchecked;
    }
    else
    {
      return QgsLayerTreeModel::data( idx, role );
    }
  }
  else if ( idx.column() == MAXIMUM_DD_SYMBOL_BLOCKS_COL )
  {
    if ( !vl || vl->geometryType() != Qgis::GeometryType::Point )
    {
      return QVariant();
    }

    if ( role == Qt::DisplayRole )
    {
      if ( !mDDBlocksMaxNumberOfClasses.contains( vl ) )
      {
        return QVariant( -1 );
      }
      else
      {
        return QVariant( mDDBlocksMaxNumberOfClasses[vl] );
      }
    }
  }

  return QVariant();
}

bool QgsVectorLayerAndAttributeModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  QgsVectorLayer *vl = vectorLayer( index );

  if ( index.column() == LAYER_COL )
  {
    if ( role == Qt::CheckStateRole )
    {
      int i = 0;
      for ( i = 0;; i++ )
      {
        QModelIndex child = QgsVectorLayerAndAttributeModel::index( i, 0, index );
        if ( !child.isValid() )
          break;

        setData( child, value, role );
      }

      if ( i == 0 )
      {
        if ( value.toInt() == Qt::Checked )
          mCheckedLeafs.insert( index );
        else if ( value.toInt() == Qt::Unchecked )
          mCheckedLeafs.remove( index );
        else
          Q_ASSERT( "expected checked or unchecked" );

        emit dataChanged( QModelIndex(), index );
      }

      return true;
    }
    else if ( role == Qt::EditRole )
    {
      if ( !value.toString().trimmed().isEmpty() && value.toString() != vl->name() )
      {
        mOverriddenName[vl] = value.toString();
      }
      else
      {
        mOverriddenName.remove( vl );
      }
      return true;
    }
  }
  else if ( index.column() == OUTPUT_LAYER_ATTRIBUTE_COL )
  {
    if ( role != Qt::EditRole )
      return false;

    if ( vl )
    {
      mAttributeIdx[vl] = value.toInt();
      return true;
    }
  }
  else if ( index.column() == ALLOW_DD_SYMBOL_BLOCKS_COL && role == Qt::CheckStateRole )
  {
    if ( vl )
    {
      mCreateDDBlockInfo[vl] = value.toBool();
      return true;
    }
  }
  else if ( index.column() == MAXIMUM_DD_SYMBOL_BLOCKS_COL && role == Qt::EditRole )
  {
    if ( vl )
    {
      mDDBlocksMaxNumberOfClasses[vl] = value.toInt();
      return true;
    }
  }

  return QgsLayerTreeModel::setData( index, value, role );
}


QList<QgsDxfExport::DxfLayer> QgsVectorLayerAndAttributeModel::layers() const
{
  QList<QgsDxfExport::DxfLayer> layers;
  QHash<QString, int> layerIdx;

  for ( const QModelIndex &idx : std::as_const( mCheckedLeafs ) )
  {
    QgsLayerTreeNode *node = index2node( idx );
    if ( QgsLayerTree::isGroup( node ) )
    {
      const auto childLayers = QgsLayerTree::toGroup( node )->findLayers();
      for ( QgsLayerTreeLayer *treeLayer : childLayers )
      {
        QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( treeLayer->layer() );
        Q_ASSERT( vl );
        if ( !layerIdx.contains( vl->id() ) )
        {
          layerIdx.insert( vl->id(), layers.size() );
          layers << QgsDxfExport::DxfLayer( vl, mAttributeIdx.value( vl, -1 ), mCreateDDBlockInfo.value( vl, DEFAULT_DXF_DATA_DEFINED_BLOCKS ), mDDBlocksMaxNumberOfClasses.value( vl, -1 ), mOverriddenName.value( vl, QString() ) );
        }
      }
    }
    else if ( QgsLayerTree::isLayer( node ) )
    {
      QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( QgsLayerTree::toLayer( node )->layer() );
      Q_ASSERT( vl );
      if ( !layerIdx.contains( vl->id() ) )
      {
        layerIdx.insert( vl->id(), layers.size() );
        layers << QgsDxfExport::DxfLayer( vl, mAttributeIdx.value( vl, -1 ), mCreateDDBlockInfo.value( vl, DEFAULT_DXF_DATA_DEFINED_BLOCKS ), mDDBlocksMaxNumberOfClasses.value( vl, -1 ), mOverriddenName.value( vl, QString() ) );
      }
    }
  }

  QList<QgsDxfExport::DxfLayer> layersInROrder;

  QList<QgsMapLayer *> layerOrder = mRootNode->layerOrder();

  QList<QgsMapLayer *>::ConstIterator layerIterator = layerOrder.constEnd();

  while ( layerIterator != layerOrder.constBegin() )
  {
    --layerIterator;

    QgsMapLayer *l = *layerIterator;
    int idx = layerIdx.value( l->id(), -1 );
    if ( idx < 0 )
      continue;

    layersInROrder << layers[idx];
  }

  Q_ASSERT( layersInROrder.size() == layers.size() );

  return layersInROrder;
}

void QgsVectorLayerAndAttributeModel::applyVisibilityPreset( const QString &name )
{
  QSet<QString> visibleLayers;

  if ( name.isEmpty() )
  {
    const auto constLayers = QgisApp::instance()->mapCanvas()->layers( true );
    for ( const QgsMapLayer *ml : constLayers )
    {
      const QgsVectorLayer *vl = qobject_cast<const QgsVectorLayer *>( ml );
      if ( !vl )
        continue;
      visibleLayers.insert( vl->id() );
    }
  }
  else
  {
    visibleLayers = qgis::listToSet( QgsProject::instance()->mapThemeCollection()->mapThemeVisibleLayerIds( name ) );
  }

  if ( visibleLayers.isEmpty() )
    return;

  mCheckedLeafs.clear();
  applyVisibility( visibleLayers, rootGroup() );
}

void QgsVectorLayerAndAttributeModel::applyVisibility( QSet<QString> &visibleLayers, QgsLayerTreeNode *node )
{
  QgsLayerTreeGroup *group = QgsLayerTree::isGroup( node ) ? QgsLayerTree::toGroup( node ) : nullptr;
  if ( !group )
    return;

  const auto constChildren = node->children();
  for ( QgsLayerTreeNode *child : constChildren )
  {
    if ( QgsLayerTree::isLayer( child ) )
    {
      QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( QgsLayerTree::toLayer( child )->layer() );
      if ( vl )
      {
        QModelIndex idx = node2index( child );
        if ( visibleLayers.contains( vl->id() ) )
        {
          visibleLayers.remove( vl->id() );
          mCheckedLeafs.insert( idx );
        }
        emit dataChanged( idx, idx, QVector<int>() << Qt::CheckStateRole );
      }
      continue;
    }

    applyVisibility( visibleLayers, child );
  }
}

void QgsVectorLayerAndAttributeModel::loadLayersOutputAttribute( QgsLayerTreeNode *node )
{
  const auto constChildren = node->children();
  for ( QgsLayerTreeNode *child : constChildren )
  {
    if ( QgsLayerTree::isLayer( child ) )
    {
      QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( QgsLayerTree::toLayer( child )->layer() );
      if ( vl )
      {
        QModelIndex idx = node2index( child );

        const int attributeIndex = vl->fields().lookupField( vl->customProperty( QStringLiteral( "lastDxfOutputAttribute" ), -1 ).toString() );
        if ( attributeIndex > -1 )
        {
          mAttributeIdx[vl] = attributeIndex;
          idx = index( idx.row(), OUTPUT_LAYER_ATTRIBUTE_COL, idx.parent() );
          emit dataChanged( idx, idx, QVector<int>() << Qt::EditRole );
        }

        if ( vl->geometryType() == Qgis::GeometryType::Point )
        {
          const bool allowDataDefinedBlocks = vl->customProperty( QStringLiteral( "lastAllowDataDefinedBlocks" ), DEFAULT_DXF_DATA_DEFINED_BLOCKS ).toBool();

          if ( allowDataDefinedBlocks != DEFAULT_DXF_DATA_DEFINED_BLOCKS )
          {
            mCreateDDBlockInfo[vl] = allowDataDefinedBlocks;
            idx = index( idx.row(), ALLOW_DD_SYMBOL_BLOCKS_COL, idx.parent() );
            emit dataChanged( idx, idx, QVector<int>() << Qt::CheckStateRole );
          }

          const int maximumNumberOfBlocks = vl->customProperty( QStringLiteral( "lastMaximumNumberOfBlocks" ), -1 ).toInt();
          if ( maximumNumberOfBlocks > -1 )
          {
            mDDBlocksMaxNumberOfClasses[vl] = maximumNumberOfBlocks;
            idx = index( idx.row(), MAXIMUM_DD_SYMBOL_BLOCKS_COL, idx.parent() );
            emit dataChanged( idx, idx, QVector<int>() << Qt::EditRole );
          }
        }
      }
    }
    else if ( QgsLayerTree::isGroup( child ) )
    {
      loadLayersOutputAttribute( child );
    }
  }
}

void QgsVectorLayerAndAttributeModel::saveLayersOutputAttribute( QgsLayerTreeNode *node )
{
  const auto constChildren = node->children();
  for ( QgsLayerTreeNode *child : constChildren )
  {
    if ( QgsLayerTree::isLayer( child ) )
    {
      QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( QgsLayerTree::toLayer( child )->layer() );
      if ( vl )
      {
        QModelIndex idx = node2index( child );
        const int attributeIndex = data( index( idx.row(), OUTPUT_LAYER_ATTRIBUTE_COL, idx.parent() ), Qt::EditRole ).toInt();
        const QgsFields fields = vl->fields();
        if ( attributeIndex > -1 && attributeIndex < fields.count() )
        {
          vl->setCustomProperty( QStringLiteral( "lastDxfOutputAttribute" ), fields.at( attributeIndex ).name() );
        }
        else
        {
          vl->removeCustomProperty( QStringLiteral( "lastDxfOutputAttribute" ) );
        }

        if ( vl->geometryType() == Qgis::GeometryType::Point )
        {
          const bool allowDataDefinedBlocks = data( index( idx.row(), ALLOW_DD_SYMBOL_BLOCKS_COL, idx.parent() ), Qt::CheckStateRole ).toBool();
          vl->setCustomProperty( QStringLiteral( "lastAllowDataDefinedBlocks" ), allowDataDefinedBlocks );

          const int maximumNumberOfBlocks = data( index( idx.row(), MAXIMUM_DD_SYMBOL_BLOCKS_COL, idx.parent() ), Qt::DisplayRole ).toInt();
          if ( maximumNumberOfBlocks > -1 )
          {
            vl->setCustomProperty( QStringLiteral( "lastMaximumNumberOfBlocks" ), maximumNumberOfBlocks );
          }
          else
          {
            vl->removeCustomProperty( QStringLiteral( "lastMaximumNumberOfBlocks" ) );
          }
        }
      }
    }
    else if ( QgsLayerTree::isGroup( child ) )
    {
      saveLayersOutputAttribute( child );
    }
  }
}

void QgsVectorLayerAndAttributeModel::retrieveAllLayers( QgsLayerTreeNode *node, QSet<QString> &set )
{
  if ( QgsLayerTree::isLayer( node ) )
  {
    set << QgsLayerTree::toLayer( node )->layer()->id();
  }
  else if ( QgsLayerTree::isGroup( node ) )
  {
    const auto constChildren = QgsLayerTree::toGroup( node )->children();
    for ( QgsLayerTreeNode *child : constChildren )
    {
      retrieveAllLayers( child, set );
    }
  }
}

void QgsVectorLayerAndAttributeModel::selectAll()
{
  mCheckedLeafs.clear();

  QSet<QString> allLayers;
  retrieveAllLayers( rootGroup(), allLayers );
  applyVisibility( allLayers, rootGroup() );

  emit dataChanged( index( 0, 0 ), index( rowCount() - 1, 0 ) );
}

void QgsVectorLayerAndAttributeModel::deSelectAll()
{
  mCheckedLeafs.clear();

  QSet<QString> noLayers;
  applyVisibility( noLayers, rootGroup() );

  emit dataChanged( index( 0, 0 ), index( rowCount() - 1, 0 ) );
}

void QgsVectorLayerAndAttributeModel::selectDataDefinedBlocks()
{
  enableDataDefinedBlocks( true );
}

void QgsVectorLayerAndAttributeModel::deselectDataDefinedBlocks()
{
  enableDataDefinedBlocks( false );
}

void QgsVectorLayerAndAttributeModel::enableDataDefinedBlocks( bool enabled )
{
  QHash<const QgsVectorLayer *, bool>::const_iterator it = mCreateDDBlockInfo.constBegin();
  for ( ; it != mCreateDDBlockInfo.constEnd(); ++it )
  {
    mCreateDDBlockInfo[it.key()] = enabled;
  }
  emit dataChanged( index( 0, 0 ), index( rowCount() - 1, 0 ) );
}

QgsDxfExportLayerTreeView::QgsDxfExportLayerTreeView( QWidget *parent )
  : QgsLayerTreeView( parent )
{
}

void QgsDxfExportLayerTreeView::resizeEvent( QResizeEvent *event )
{
  header()->setMinimumSectionSize( viewport()->width() / 2 );
  header()->setMaximumSectionSize( viewport()->width() / 2 );
  QTreeView::resizeEvent( event ); // NOLINT(bugprone-parent-virtual-call) clazy:exclude=skipped-base-method
}

QgsDxfExportDialog::QgsDxfExportDialog( QWidget *parent, Qt::WindowFlags f )
  : QDialog( parent, f )
{
  setupUi( this );

  mTreeView = new QgsDxfExportLayerTreeView( this );
  mTreeViewContainer->layout()->addWidget( mTreeView );

  QgsGui::enableAutoGeometryRestore( this );

  connect( mVisibilityPresets, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsDxfExportDialog::mVisibilityPresets_currentIndexChanged );
  connect( mCrsSelector, &QgsProjectionSelectionWidget::crsChanged, this, &QgsDxfExportDialog::mCrsSelector_crsChanged );

  QgsSettings settings;

  mLayerTreeGroup = QgsProject::instance()->layerTreeRoot()->clone();
  cleanGroup( mLayerTreeGroup );

  mFieldSelectorDelegate = new FieldSelectorDelegate( this );
  mTreeView->setEditTriggers( QAbstractItemView::AllEditTriggers );
  mTreeView->setItemDelegate( mFieldSelectorDelegate );

  mModel = new QgsVectorLayerAndAttributeModel( mLayerTreeGroup, this );
  mModel->setFlags( QgsLayerTreeModel::Flags() );

  mTreeView->setModel( mModel );
  mTreeView->header()->show();

  mFileName->setStorageMode( QgsFileWidget::SaveFile );
  mFileName->setFilter( tr( "DXF files" ) + " (*.dxf *.DXF)" );
  mFileName->setDialogTitle( tr( "Export as DXF" ) );
  mFileName->setDefaultRoot( settings.value( QStringLiteral( "qgis/lastDxfDir" ), QDir::homePath() ).toString() );

  connect( this, &QDialog::accepted, this, &QgsDxfExportDialog::saveSettings );
  connect( mSelectAllButton, &QAbstractButton::clicked, this, &QgsDxfExportDialog::selectAll );
  connect( mDeselectAllButton, &QAbstractButton::clicked, this, &QgsDxfExportDialog::deSelectAll );
  connect( mSelectDataDefinedBlocks, &QAbstractButton::clicked, this, &QgsDxfExportDialog::selectDataDefinedBlocks );
  connect( mDeselectDataDefinedBlocks, &QAbstractButton::clicked, this, &QgsDxfExportDialog::deselectDataDefinedBlocks );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsDxfExportDialog::showHelp );

  connect( mFileName, &QgsFileWidget::fileChanged, this, [=]( const QString &filePath ) {
    QgsSettings settings;
    QFileInfo tmplFileInfo( filePath );
    settings.setValue( QStringLiteral( "qgis/lastDxfDir" ), tmplFileInfo.absolutePath() );

    setOkEnabled();
  } );

  mSymbologyModeComboBox->addItem( tr( "No Symbology" ), QVariant::fromValue( Qgis::FeatureSymbologyExport::NoSymbology ) );
  mSymbologyModeComboBox->addItem( tr( "Feature Symbology" ), QVariant::fromValue( Qgis::FeatureSymbologyExport::PerFeature ) );
  mSymbologyModeComboBox->addItem( tr( "Symbol Layer Symbology" ), QVariant::fromValue( Qgis::FeatureSymbologyExport::PerSymbolLayer ) );

  //last dxf symbology mode
  mSymbologyModeComboBox->setCurrentIndex( QgsProject::instance()->readEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastDxfSymbologyMode" ), settings.value( QStringLiteral( "qgis/lastDxfSymbologyMode" ), "2" ).toString() ).toInt() );

  //last symbol scale
  mScaleWidget->setMapCanvas( QgisApp::instance()->mapCanvas() );
  double oldScale = QgsProject::instance()->readEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastSymbologyExportScale" ), settings.value( QStringLiteral( "qgis/lastSymbologyExportScale" ), "1/50000" ).toString() ).toDouble();
  if ( oldScale != 0.0 )
    mScaleWidget->setScale( 1.0 / oldScale );
  mLayerTitleAsName->setChecked( QgsProject::instance()->readEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastDxfLayerTitleAsName" ), settings.value( QStringLiteral( "qgis/lastDxfLayerTitleAsName" ), "false" ).toString() ) != QLatin1String( "false" ) );
  mMapExtentCheckBox->setChecked( QgsProject::instance()->readEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastDxfMapRectangle" ), settings.value( QStringLiteral( "qgis/lastDxfMapRectangle" ), "false" ).toString() ) != QLatin1String( "false" ) );
  mSelectedFeaturesOnly->setChecked( QgsProject::instance()->readEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastDxfSelectedFeaturesOnly" ), settings.value( QStringLiteral( "qgis/lastDxfSelectedFeaturesOnly" ), "false" ).toString() ) != QLatin1String( "false" ) );
  mMTextCheckBox->setChecked( QgsProject::instance()->readEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastDxfUseMText" ), settings.value( QStringLiteral( "qgis/lastDxfUseMText" ), "true" ).toString() ) != QLatin1String( "false" ) );
  mForce2d->setChecked( QgsProject::instance()->readEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastDxfForce2d" ), settings.value( QStringLiteral( "qgis/lastDxfForce2d" ), "false" ).toString() ) != QLatin1String( "false" ) );
  mHairlineWidthExportCheckBox->setChecked( QgsProject::instance()->readEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastDxfHairlineWidthExport" ), settings.value( QStringLiteral( "qgis/lastDxfHairlineWidthExport" ), "false" ).toString() ) != QLatin1String( "false" ) );

  QStringList ids = QgsProject::instance()->mapThemeCollection()->mapThemes();
  ids.prepend( QString() );
  mVisibilityPresets->addItems( ids );
  mVisibilityPresets->setCurrentIndex( mVisibilityPresets->findText( QgsProject::instance()->readEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastVisibilityPreset" ), QString() ) ) );

  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );

  long crsid = QgsProject::instance()->readEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastDxfCrs" ), settings.value( QStringLiteral( "qgis/lastDxfCrs" ), QString::number( QgsProject::instance()->crs().srsid() ) ).toString() ).toLong();
  mCRS = QgsCoordinateReferenceSystem::fromSrsId( crsid );
  mCrsSelector->setCrs( mCRS );
  mCrsSelector->setLayerCrs( mCRS );
  mCrsSelector->setShowAccuracyWarnings( true );
  mCrsSelector->setMessage( tr( "Select the coordinate reference system for the dxf file. "
                                "The data points will be transformed from the layer coordinate reference system." ) );

  mEncoding->addItems( QgsDxfExport::encodings() );
  mEncoding->setCurrentIndex( mEncoding->findText( QgsProject::instance()->readEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastDxfEncoding" ), settings.value( QStringLiteral( "qgis/lastDxfEncoding" ), "CP1252" ).toString() ) ) );

  QPushButton *btnLoadSaveSettings = new QPushButton( tr( "Settings" ), this );
  QMenu *menuSettings = new QMenu( this );
  menuSettings->addAction( tr( "Load Settings from File…" ), this, &QgsDxfExportDialog::loadSettingsFromFile );
  menuSettings->addAction( tr( "Save Settings to File…" ), this, &QgsDxfExportDialog::saveSettingsToFile );
  btnLoadSaveSettings->setMenu( menuSettings );
  buttonBox->addButton( btnLoadSaveSettings, QDialogButtonBox::ResetRole );

  mMessageBar = new QgsMessageBar();
  mMessageBar->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
  mainLayout->insertWidget( 0, mMessageBar );

  mModel->loadLayersOutputAttribute( mModel->rootGroup() );
}


QgsDxfExportDialog::~QgsDxfExportDialog()
{
  delete mLayerTreeGroup;
}

void QgsDxfExportDialog::mVisibilityPresets_currentIndexChanged( int index )
{
  Q_UNUSED( index )
  mModel->applyVisibilityPreset( mVisibilityPresets->currentText() );
}

void QgsDxfExportDialog::cleanGroup( QgsLayerTreeNode *node )
{
  QgsLayerTreeGroup *group = QgsLayerTree::isGroup( node ) ? QgsLayerTree::toGroup( node ) : nullptr;
  if ( !group )
    return;

  QList<QgsLayerTreeNode *> toRemove;
  const auto constChildren = node->children();
  for ( QgsLayerTreeNode *child : constChildren )
  {
    if ( QgsLayerTree::isLayer( child ) && ( QgsLayerTree::toLayer( child )->layer()->type() != Qgis::LayerType::Vector || !QgsLayerTree::toLayer( child )->layer()->isSpatial() || !QgsLayerTree::toLayer( child )->layer()->isValid() ) )
    {
      toRemove << child;
      continue;
    }

    cleanGroup( child );

    if ( QgsLayerTree::isGroup( child ) && child->children().isEmpty() )
      toRemove << child;
  }

  const auto constToRemove = toRemove;
  for ( QgsLayerTreeNode *child : constToRemove )
    group->removeChildNode( child );
}


void QgsDxfExportDialog::selectAll()
{
  mModel->selectAll();
}

void QgsDxfExportDialog::deSelectAll()
{
  mModel->deSelectAll();
}

void QgsDxfExportDialog::selectDataDefinedBlocks()
{
  mModel->selectDataDefinedBlocks();
}

void QgsDxfExportDialog::deselectDataDefinedBlocks()
{
  mModel->deselectDataDefinedBlocks();
}


void QgsDxfExportDialog::loadSettingsFromFile()
{
  const QString fileName = QFileDialog::getOpenFileName( this, tr( "Load DXF Export Settings" ), QgsDxfExportDialog::settingsDxfLastSettingsDir->value(), tr( "XML file" ) + " (*.xml)" );
  if ( fileName.isNull() )
  {
    return;
  }

  bool resultFlag = false;

  QDomDocument domDocument( QStringLiteral( "qgis" ) );

  // location of problem associated with errorMsg
  int line, column;
  QString errorMessage;

  QFile file( fileName );
  if ( file.open( QFile::ReadOnly ) )
  {
    QgsDebugMsgLevel( QStringLiteral( "file found %1" ).arg( fileName ), 2 );
    // read file
    resultFlag = domDocument.setContent( &file, &errorMessage, &line, &column );
    if ( !resultFlag )
      errorMessage = tr( "%1 at line %2 column %3" ).arg( errorMessage ).arg( line ).arg( column );
    file.close();
  }

  if ( QMessageBox::question( this, tr( "DXF Export - Load from XML File" ), tr( "Are you sure you want to load settings from XML? This will change some values in the DXF Export dialog." ) ) == QMessageBox::Yes )
  {
    resultFlag = loadSettingsFromXML( domDocument, errorMessage );
    if ( !resultFlag )
    {
      mMessageBar->pushWarning( tr( "Load DXF Export Settings" ), tr( "Failed to load DXF Export settings file as %1. Details: %2" ).arg( fileName, errorMessage ) );
    }
    else
    {
      QgsDxfExportDialog::settingsDxfLastSettingsDir->setValue( QFileInfo( fileName ).path() );
      mMessageBar->pushMessage( QString(), tr( "DXF Export settings loaded!" ), Qgis::MessageLevel::Success, 0 );
    }
  }
}


bool QgsDxfExportDialog::loadSettingsFromXML( QDomDocument &doc, QString &errorMessage ) const
{
  const QDomElement rootElement = doc.firstChildElement( QStringLiteral( "qgis" ) );
  if ( rootElement.isNull() )
  {
    errorMessage = tr( "Root &lt;qgis&gt; element could not be found." );
    return false;
  }

  const QDomElement dxfElement = rootElement.firstChildElement( QStringLiteral( "dxf_settings" ) );
  if ( dxfElement.isNull() )
  {
    errorMessage = tr( "The XML file does not correspond to DXF Export settings. It must have a &lt;dxf-settings&gt; element." );
    return false;
  }

  QDomElement element;
  QVariant value;

  element = dxfElement.namedItem( QStringLiteral( "symbology_mode" ) ).toElement();
  value = QgsXmlUtils::readVariant( element.firstChildElement() );
  if ( !value.isNull() )
    mSymbologyModeComboBox->setCurrentIndex( value.toInt() );

  element = dxfElement.namedItem( QStringLiteral( "symbology_scale" ) ).toElement();
  value = QgsXmlUtils::readVariant( element.firstChildElement() );
  if ( !value.isNull() )
    mScaleWidget->setScale( value.toDouble() );

  element = dxfElement.namedItem( QStringLiteral( "encoding" ) ).toElement();
  value = QgsXmlUtils::readVariant( element.firstChildElement() );
  if ( !value.isNull() )
    mEncoding->setCurrentText( value.toString() );

  element = dxfElement.namedItem( QStringLiteral( "crs" ) ).toElement();
  value = QgsXmlUtils::readVariant( element.firstChildElement() );
  if ( !value.isNull() )
    mCrsSelector->setCrs( value.value<QgsCoordinateReferenceSystem>() );

  element = dxfElement.namedItem( QStringLiteral( "map_theme" ) ).toElement();
  value = QgsXmlUtils::readVariant( element.firstChildElement() );
  if ( !value.isNull() )
    mVisibilityPresets->setCurrentText( value.toString() );

  // layer settings
  element = dxfElement.namedItem( QStringLiteral( "layers" ) ).toElement();
  QDomNodeList layerNodeList = element.elementsByTagName( QStringLiteral( "layer" ) );
  const QgsReadWriteContext rwContext = QgsReadWriteContext();

  QgsVectorLayer *vl;
  QgsVectorLayerRef vlRef;

  for ( int i = 0; i < layerNodeList.length(); i++ )
  {
    element = layerNodeList.at( i ).toElement();
    if ( vlRef.readXml( element, rwContext ) )
    {
      vl = vlRef.resolveWeakly( QgsProject::instance() );
      if ( vl )
      {
        QgsLayerTreeLayer *treeNode = mLayerTreeGroup->findLayer( vl );
        QModelIndex idx = mModel->node2index( treeNode );

        idx = mModel->index( idx.row(), OUTPUT_LAYER_ATTRIBUTE_COL, idx.parent() );
        mModel->setData( idx, element.attribute( QStringLiteral( "attribute-index" ), QStringLiteral( "-1" ) ) );

        idx = mModel->index( idx.row(), ALLOW_DD_SYMBOL_BLOCKS_COL, idx.parent() );
        mModel->setData( idx, element.attribute( QStringLiteral( "use_symbol_blocks" ), QStringLiteral( "0" ) ), Qt::CheckStateRole );

        idx = mModel->index( idx.row(), MAXIMUM_DD_SYMBOL_BLOCKS_COL, idx.parent() );
        mModel->setData( idx, element.attribute( QStringLiteral( "max_number_of_classes" ), QStringLiteral( "-1" ) ) );
      }
      else
      {
        QgsDebugMsgLevel( QStringLiteral( " Layer '%1' found in the DXF Export settings XML file, but not present in the project." ).arg( element.attribute( QStringLiteral( "name" ) ) ), 1 );
      }
    }
  }

  element = dxfElement.namedItem( QStringLiteral( "use_layer_title" ) ).toElement();
  value = QgsXmlUtils::readVariant( element.firstChildElement() );
  if ( !value.isNull() )
    mLayerTitleAsName->setChecked( value == true );

  element = dxfElement.namedItem( QStringLiteral( "use_map_extent" ) ).toElement();
  value = QgsXmlUtils::readVariant( element.firstChildElement() );
  if ( !value.isNull() )
    mMapExtentCheckBox->setChecked( value == true );

  element = dxfElement.namedItem( QStringLiteral( "force_2d" ) ).toElement();
  value = QgsXmlUtils::readVariant( element.firstChildElement() );
  if ( !value.isNull() )
    mForce2d->setChecked( value == true );

  element = dxfElement.namedItem( QStringLiteral( "mtext" ) ).toElement();
  value = QgsXmlUtils::readVariant( element.firstChildElement() );
  if ( !value.isNull() )
    mMTextCheckBox->setChecked( value == true );

  element = dxfElement.namedItem( QStringLiteral( "selected_features_only" ) ).toElement();
  value = QgsXmlUtils::readVariant( element.firstChildElement() );
  if ( !value.isNull() )
    mSelectedFeaturesOnly->setChecked( value == true );

  element = dxfElement.namedItem( QStringLiteral( "hairline_width_export" ) ).toElement();
  value = QgsXmlUtils::readVariant( element.firstChildElement() );
  if ( !value.isNull() )
    mHairlineWidthExportCheckBox->setChecked( value == true );

  return true;
}


void QgsDxfExportDialog::saveSettingsToFile()
{
  QString outputFileName = QFileDialog::getSaveFileName( this, tr( "Save DXF Export Settings as XML" ), QgsDxfExportDialog::settingsDxfLastSettingsDir->value(), tr( "XML file" ) + " (*.xml)" );
  // return dialog focus on Mac
  activateWindow();
  raise();
  if ( outputFileName.isEmpty() )
  {
    return;
  }

  //ensure the user never omitted the extension from the file name
  if ( !outputFileName.endsWith( QStringLiteral( ".xml" ), Qt::CaseInsensitive ) )
  {
    outputFileName += QLatin1String( ".xml" );
  }

  QDomDocument domDocument;

  saveSettingsToXML( domDocument );

  const QFileInfo fileInfo( outputFileName );
  const QFileInfo dirInfo( fileInfo.path() ); //excludes file name
  if ( !dirInfo.isWritable() )
  {
    mMessageBar->pushInfo( tr( "Save DXF Export Settings" ), tr( "The directory containing your dataset needs to be writable!" ) );
    return;
  }

  QFile file( outputFileName );
  if ( file.open( QFile::WriteOnly | QFile::Truncate ) )
  {
    QTextStream fileStream( &file );
    // save as utf-8 with 2 spaces for indents
    domDocument.save( fileStream, 2 );
    file.close();
    mMessageBar->pushSuccess( tr( "Save DXF Export Settings" ), tr( "Created DXF Export settings file as %1" ).arg( outputFileName ) );
    QgsDxfExportDialog::settingsDxfLastSettingsDir->setValue( QFileInfo( outputFileName ).absolutePath() );
    return;
  }
  else
  {
    mMessageBar->pushWarning( tr( "Save DXF Export Settings" ), tr( "Failed to created DXF Export settings file as %1. Check file permissions and retry." ).arg( outputFileName ) );
    return;
  }
}


void QgsDxfExportDialog::saveSettingsToXML( QDomDocument &doc ) const
{
  QDomImplementation DomImplementation;
  const QDomDocumentType documentType = DomImplementation.createDocumentType( QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  QDomDocument domDocument( documentType );

  QDomElement rootElement = domDocument.createElement( QStringLiteral( "qgis" ) );
  rootElement.setAttribute( QStringLiteral( "version" ), Qgis::version() );
  domDocument.appendChild( rootElement );

  QDomElement dxfElement = domDocument.createElement( QStringLiteral( "dxf_settings" ) );
  rootElement.appendChild( dxfElement );

  QDomElement symbologyModeElement = domDocument.createElement( QStringLiteral( "symbology_mode" ) );
  symbologyModeElement.appendChild( QgsXmlUtils::writeVariant( static_cast<int>( symbologyMode() ), doc ) );
  dxfElement.appendChild( symbologyModeElement );

  QDomElement symbologyScaleElement = domDocument.createElement( QStringLiteral( "symbology_scale" ) );
  symbologyScaleElement.appendChild( QgsXmlUtils::writeVariant( symbologyScale(), doc ) );
  dxfElement.appendChild( symbologyScaleElement );

  QDomElement encodingElement = domDocument.createElement( QStringLiteral( "encoding" ) );
  encodingElement.appendChild( QgsXmlUtils::writeVariant( encoding(), doc ) );
  dxfElement.appendChild( encodingElement );

  QDomElement crsElement = domDocument.createElement( QStringLiteral( "crs" ) );
  crsElement.appendChild( QgsXmlUtils::writeVariant( crs(), doc ) );
  dxfElement.appendChild( crsElement );

  QDomElement mapThemeElement = domDocument.createElement( QStringLiteral( "map_theme" ) );
  mapThemeElement.appendChild( QgsXmlUtils::writeVariant( mapTheme(), doc ) );
  dxfElement.appendChild( mapThemeElement );

  QDomElement layersElement = domDocument.createElement( QStringLiteral( "layers" ) );
  QgsVectorLayerRef vlRef;
  const QgsReadWriteContext rwContext = QgsReadWriteContext();

  for ( const auto &dxfLayer : layers() )
  {
    QDomElement layerElement = domDocument.createElement( QStringLiteral( "layer" ) );
    vlRef.setLayer( dxfLayer.layer() );
    vlRef.writeXml( layerElement, rwContext );
    layerElement.setAttribute( QStringLiteral( "attribute-index" ), dxfLayer.layerOutputAttributeIndex() );
    layerElement.setAttribute( QStringLiteral( "use_symbol_blocks" ), dxfLayer.buildDataDefinedBlocks() );
    layerElement.setAttribute( QStringLiteral( "max_number_of_classes" ), dxfLayer.dataDefinedBlocksMaximumNumberOfClasses() );
    layersElement.appendChild( layerElement );
  }
  dxfElement.appendChild( layersElement );

  QDomElement titleAsNameElement = domDocument.createElement( QStringLiteral( "use_layer_title" ) );
  titleAsNameElement.appendChild( QgsXmlUtils::writeVariant( layerTitleAsName(), doc ) );
  dxfElement.appendChild( titleAsNameElement );

  QDomElement useMapExtentElement = domDocument.createElement( QStringLiteral( "use_map_extent" ) );
  useMapExtentElement.appendChild( QgsXmlUtils::writeVariant( exportMapExtent(), doc ) );
  dxfElement.appendChild( useMapExtentElement );

  QDomElement force2dElement = domDocument.createElement( QStringLiteral( "force_2d" ) );
  force2dElement.appendChild( QgsXmlUtils::writeVariant( force2d(), doc ) );
  dxfElement.appendChild( force2dElement );

  QDomElement useMTextElement = domDocument.createElement( QStringLiteral( "mtext" ) );
  useMTextElement.appendChild( QgsXmlUtils::writeVariant( useMText(), doc ) );
  dxfElement.appendChild( useMTextElement );

  QDomElement selectedFeatures = domDocument.createElement( QStringLiteral( "selected_features_only" ) );
  selectedFeatures.appendChild( QgsXmlUtils::writeVariant( selectedFeaturesOnly(), doc ) );
  dxfElement.appendChild( selectedFeatures );

  QDomElement hairlineWidthExportElem = domDocument.createElement( QStringLiteral( "hairline_width_export" ) );
  hairlineWidthExportElem.appendChild( QgsXmlUtils::writeVariant( hairlineWidthExport(), doc ) );
  dxfElement.appendChild( hairlineWidthExportElem );


  doc = domDocument;
}


QList<QgsDxfExport::DxfLayer> QgsDxfExportDialog::layers() const
{
  return mModel->layers();
}


double QgsDxfExportDialog::symbologyScale() const
{
  double scale = mScaleWidget->scale();
  if ( qgsDoubleNear( scale, 0.0 ) )
    return 1.0;

  return scale;
}


QString QgsDxfExportDialog::saveFile() const
{
  return mFileName->filePath();
}


Qgis::FeatureSymbologyExport QgsDxfExportDialog::symbologyMode() const
{
  return mSymbologyModeComboBox->currentData().value<Qgis::FeatureSymbologyExport>();
}

void QgsDxfExportDialog::setOkEnabled()
{
  QPushButton *btn = buttonBox->button( QDialogButtonBox::Ok );

  QString filePath = mFileName->filePath();
  if ( filePath.isEmpty() )
  {
    btn->setEnabled( false );
    return;
  }

  QFileInfo fi( filePath );

  bool ok = ( fi.absoluteDir().exists() && !fi.baseName().isEmpty() );
  btn->setEnabled( ok );
}


bool QgsDxfExportDialog::exportMapExtent() const
{
  return mMapExtentCheckBox->isChecked();
}

bool QgsDxfExportDialog::selectedFeaturesOnly() const
{
  return mSelectedFeaturesOnly->isChecked();
}

bool QgsDxfExportDialog::layerTitleAsName() const
{
  return mLayerTitleAsName->isChecked();
}

bool QgsDxfExportDialog::force2d() const
{
  return mForce2d->isChecked();
}

bool QgsDxfExportDialog::useMText() const
{
  return mMTextCheckBox->isChecked();
}

bool QgsDxfExportDialog::hairlineWidthExport() const
{
  return mHairlineWidthExportCheckBox->isChecked();
}

void QgsDxfExportDialog::saveSettings()
{
  QgsSettings settings;
  QFileInfo dxfFileInfo( mFileName->filePath() );
  settings.setValue( QStringLiteral( "qgis/lastDxfDir" ), dxfFileInfo.absolutePath() );
  settings.setValue( QStringLiteral( "qgis/lastDxfSymbologyMode" ), mSymbologyModeComboBox->currentIndex() );
  settings.setValue( QStringLiteral( "qgis/lastSymbologyExportScale" ), mScaleWidget->scale() != 0 ? 1.0 / mScaleWidget->scale() : 0 );
  settings.setValue( QStringLiteral( "qgis/lastDxfMapRectangle" ), mMapExtentCheckBox->isChecked() );
  settings.setValue( QStringLiteral( "qgis/lastDxfSelectedFeaturesOnly" ), mSelectedFeaturesOnly->isChecked() );
  settings.setValue( QStringLiteral( "qgis/lastDxfLayerTitleAsName" ), mLayerTitleAsName->isChecked() );
  settings.setValue( QStringLiteral( "qgis/lastDxfEncoding" ), mEncoding->currentText() );
  settings.setValue( QStringLiteral( "qgis/lastDxfCrs" ), QString::number( mCRS.srsid() ) );
  settings.setValue( QStringLiteral( "qgis/lastDxfUseMText" ), mMTextCheckBox->isChecked() );
  settings.setValue( QStringLiteral( "qgis/lastDxfForce2d" ), mForce2d->isChecked() );
  settings.setValue( QStringLiteral( "qgis/lastDxfHairlineWidthExport" ), mHairlineWidthExportCheckBox->isChecked() );

  QgsProject::instance()->writeEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastDxfSymbologyMode" ), mSymbologyModeComboBox->currentIndex() );
  QgsProject::instance()->writeEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastSymbologyExportScale" ), mScaleWidget->scale() != 0 ? 1.0 / mScaleWidget->scale() : 0 );
  QgsProject::instance()->writeEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastDxfLayerTitleAsName" ), mLayerTitleAsName->isChecked() );
  QgsProject::instance()->writeEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastDxfMapRectangle" ), mMapExtentCheckBox->isChecked() );
  QgsProject::instance()->writeEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastDxfSelectedFeaturesOnly" ), mSelectedFeaturesOnly->isChecked() );
  QgsProject::instance()->writeEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastDxfEncoding" ), mEncoding->currentText() );
  QgsProject::instance()->writeEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastVisibilityPreset" ), mVisibilityPresets->currentText() );
  QgsProject::instance()->writeEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastDxfCrs" ), QString::number( mCRS.srsid() ) );
  QgsProject::instance()->writeEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastDxfUseMText" ), mMTextCheckBox->isChecked() );
  QgsProject::instance()->writeEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastDxfForce2d" ), mForce2d->isChecked() );
  QgsProject::instance()->writeEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastDxfHairlineWidthExport" ), mHairlineWidthExportCheckBox->isChecked() );

  mModel->saveLayersOutputAttribute( mModel->rootGroup() );
}


QString QgsDxfExportDialog::encoding() const
{
  return mEncoding->currentText();
}

void QgsDxfExportDialog::mCrsSelector_crsChanged( const QgsCoordinateReferenceSystem &crs )
{
  mCRS = crs;
}

QgsCoordinateReferenceSystem QgsDxfExportDialog::crs() const
{
  return mCRS;
}

QString QgsDxfExportDialog::mapTheme() const
{
  return mVisibilityPresets->currentText();
}
void QgsDxfExportDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/create_layers.html#create-dxf-files" ) );
}
