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

#include "qgis.h"
#include "qgisapp.h"
#include "qgsfieldcombobox.h"
#include "qgsgui.h"
#include "qgshelp.h"
#include "qgslayertree.h"
#include "qgslayertreegroup.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsmapthemecollection.h"
#include "qgsproject.h"
#include "qgssettings.h"
#include "qgsvectorlayer.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>

#include "moc_qgsdxfexportdialog.cpp"

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

        const int attributeIndex = vl->fields().lookupField( vl->customProperty( u"lastDxfOutputAttribute"_s, -1 ).toString() );
        if ( attributeIndex > -1 )
        {
          mAttributeIdx[vl] = attributeIndex;
          idx = index( idx.row(), OUTPUT_LAYER_ATTRIBUTE_COL, idx.parent() );
          emit dataChanged( idx, idx, QVector<int>() << Qt::EditRole );
        }

        if ( vl->geometryType() == Qgis::GeometryType::Point )
        {
          const bool allowDataDefinedBlocks = vl->customProperty( u"lastAllowDataDefinedBlocks"_s, DEFAULT_DXF_DATA_DEFINED_BLOCKS ).toBool();

          if ( allowDataDefinedBlocks != DEFAULT_DXF_DATA_DEFINED_BLOCKS )
          {
            mCreateDDBlockInfo[vl] = allowDataDefinedBlocks;
            idx = index( idx.row(), ALLOW_DD_SYMBOL_BLOCKS_COL, idx.parent() );
            emit dataChanged( idx, idx, QVector<int>() << Qt::CheckStateRole );
          }

          const int maximumNumberOfBlocks = vl->customProperty( u"lastMaximumNumberOfBlocks"_s, -1 ).toInt();
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
          vl->setCustomProperty( u"lastDxfOutputAttribute"_s, fields.at( attributeIndex ).name() );
        }
        else
        {
          vl->removeCustomProperty( u"lastDxfOutputAttribute"_s );
        }

        if ( vl->geometryType() == Qgis::GeometryType::Point )
        {
          const bool allowDataDefinedBlocks = data( index( idx.row(), ALLOW_DD_SYMBOL_BLOCKS_COL, idx.parent() ), Qt::CheckStateRole ).toBool();
          vl->setCustomProperty( u"lastAllowDataDefinedBlocks"_s, allowDataDefinedBlocks );

          const int maximumNumberOfBlocks = data( index( idx.row(), MAXIMUM_DD_SYMBOL_BLOCKS_COL, idx.parent() ), Qt::DisplayRole ).toInt();
          if ( maximumNumberOfBlocks > -1 )
          {
            vl->setCustomProperty( u"lastMaximumNumberOfBlocks"_s, maximumNumberOfBlocks );
          }
          else
          {
            vl->removeCustomProperty( u"lastMaximumNumberOfBlocks"_s );
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
  mFileName->setDefaultRoot( settings.value( u"qgis/lastDxfDir"_s, QDir::homePath() ).toString() );

  connect( this, &QDialog::accepted, this, &QgsDxfExportDialog::saveSettings );
  connect( mSelectAllButton, &QAbstractButton::clicked, this, &QgsDxfExportDialog::selectAll );
  connect( mDeselectAllButton, &QAbstractButton::clicked, this, &QgsDxfExportDialog::deSelectAll );
  connect( mSelectDataDefinedBlocks, &QAbstractButton::clicked, this, &QgsDxfExportDialog::selectDataDefinedBlocks );
  connect( mDeselectDataDefinedBlocks, &QAbstractButton::clicked, this, &QgsDxfExportDialog::deselectDataDefinedBlocks );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsDxfExportDialog::showHelp );

  connect( mFileName, &QgsFileWidget::fileChanged, this, [this]( const QString &filePath ) {
    QgsSettings settings;
    QFileInfo tmplFileInfo( filePath );
    settings.setValue( u"qgis/lastDxfDir"_s, tmplFileInfo.absolutePath() );

    setOkEnabled();
  } );

  mSymbologyModeComboBox->addItem( tr( "No Symbology" ), QVariant::fromValue( Qgis::FeatureSymbologyExport::NoSymbology ) );
  mSymbologyModeComboBox->addItem( tr( "Feature Symbology" ), QVariant::fromValue( Qgis::FeatureSymbologyExport::PerFeature ) );
  mSymbologyModeComboBox->addItem( tr( "Symbol Layer Symbology" ), QVariant::fromValue( Qgis::FeatureSymbologyExport::PerSymbolLayer ) );

  //last dxf symbology mode
  mSymbologyModeComboBox->setCurrentIndex( QgsProject::instance()->readEntry( u"dxf"_s, u"/lastDxfSymbologyMode"_s, settings.value( u"qgis/lastDxfSymbologyMode"_s, "2" ).toString() ).toInt() );

  //last symbol scale
  mScaleWidget->setMapCanvas( QgisApp::instance()->mapCanvas() );
  double oldScale = QgsProject::instance()->readEntry( u"dxf"_s, u"/lastSymbologyExportScale"_s, settings.value( u"qgis/lastSymbologyExportScale"_s, "1/50000" ).toString() ).toDouble();
  if ( oldScale != 0.0 )
    mScaleWidget->setScale( 1.0 / oldScale );
  mLayerTitleAsName->setChecked( QgsProject::instance()->readEntry( u"dxf"_s, u"/lastDxfLayerTitleAsName"_s, settings.value( u"qgis/lastDxfLayerTitleAsName"_s, "false" ).toString() ) != "false"_L1 );
  mMapExtentCheckBox->setChecked( QgsProject::instance()->readEntry( u"dxf"_s, u"/lastDxfMapRectangle"_s, settings.value( u"qgis/lastDxfMapRectangle"_s, "false" ).toString() ) != "false"_L1 );
  mSelectedFeaturesOnly->setChecked( QgsProject::instance()->readEntry( u"dxf"_s, u"/lastDxfSelectedFeaturesOnly"_s, settings.value( u"qgis/lastDxfSelectedFeaturesOnly"_s, "false" ).toString() ) != "false"_L1 );
  mMTextCheckBox->setChecked( QgsProject::instance()->readEntry( u"dxf"_s, u"/lastDxfUseMText"_s, settings.value( u"qgis/lastDxfUseMText"_s, "true" ).toString() ) != "false"_L1 );
  mForce2d->setChecked( QgsProject::instance()->readEntry( u"dxf"_s, u"/lastDxfForce2d"_s, settings.value( u"qgis/lastDxfForce2d"_s, "false" ).toString() ) != "false"_L1 );
  mHairlineWidthExportCheckBox->setChecked( QgsProject::instance()->readEntry( u"dxf"_s, u"/lastDxfHairlineWidthExport"_s, settings.value( u"qgis/lastDxfHairlineWidthExport"_s, "false" ).toString() ) != "false"_L1 );

  QStringList ids = QgsProject::instance()->mapThemeCollection()->mapThemes();
  ids.prepend( QString() );
  mVisibilityPresets->addItems( ids );
  mVisibilityPresets->setCurrentIndex( mVisibilityPresets->findText( QgsProject::instance()->readEntry( u"dxf"_s, u"/lastVisibilityPreset"_s, QString() ) ) );

  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );

  long crsid = QgsProject::instance()->readEntry( u"dxf"_s, u"/lastDxfCrs"_s, settings.value( u"qgis/lastDxfCrs"_s, QString::number( QgsProject::instance()->crs().srsid() ) ).toString() ).toLong();
  mCRS = QgsCoordinateReferenceSystem::fromSrsId( crsid );
  mCrsSelector->setCrs( mCRS );
  mCrsSelector->setLayerCrs( mCRS );
  mCrsSelector->setShowAccuracyWarnings( true );
  mCrsSelector->setMessage( tr( "Select the coordinate reference system for the dxf file. "
                                "The data points will be transformed from the layer coordinate reference system." ) );

  mEncoding->addItems( QgsDxfExport::encodings() );
  mEncoding->setCurrentIndex( mEncoding->findText( QgsProject::instance()->readEntry( u"dxf"_s, u"/lastDxfEncoding"_s, settings.value( u"qgis/lastDxfEncoding"_s, "CP1252" ).toString() ) ) );

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

  QDomDocument domDocument( u"qgis"_s );

  // location of problem associated with errorMsg
  int line, column;
  QString errorMessage;

  QFile file( fileName );
  if ( file.open( QFile::ReadOnly ) )
  {
    QgsDebugMsgLevel( u"file found %1"_s.arg( fileName ), 2 );
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
  const QDomElement rootElement = doc.firstChildElement( u"qgis"_s );
  if ( rootElement.isNull() )
  {
    errorMessage = tr( "Root &lt;qgis&gt; element could not be found." );
    return false;
  }

  const QDomElement dxfElement = rootElement.firstChildElement( u"dxf_settings"_s );
  if ( dxfElement.isNull() )
  {
    errorMessage = tr( "The XML file does not correspond to DXF Export settings. It must have a &lt;dxf-settings&gt; element." );
    return false;
  }

  QDomElement element;
  QVariant value;

  element = dxfElement.namedItem( u"symbology_mode"_s ).toElement();
  value = QgsXmlUtils::readVariant( element.firstChildElement() );
  if ( !value.isNull() )
    mSymbologyModeComboBox->setCurrentIndex( value.toInt() );

  element = dxfElement.namedItem( u"symbology_scale"_s ).toElement();
  value = QgsXmlUtils::readVariant( element.firstChildElement() );
  if ( !value.isNull() )
    mScaleWidget->setScale( value.toDouble() );

  element = dxfElement.namedItem( u"encoding"_s ).toElement();
  value = QgsXmlUtils::readVariant( element.firstChildElement() );
  if ( !value.isNull() )
    mEncoding->setCurrentText( value.toString() );

  element = dxfElement.namedItem( u"crs"_s ).toElement();
  value = QgsXmlUtils::readVariant( element.firstChildElement() );
  if ( !value.isNull() )
    mCrsSelector->setCrs( value.value<QgsCoordinateReferenceSystem>() );

  element = dxfElement.namedItem( u"map_theme"_s ).toElement();
  value = QgsXmlUtils::readVariant( element.firstChildElement() );
  if ( !value.isNull() )
    mVisibilityPresets->setCurrentText( value.toString() );

  // layer settings
  element = dxfElement.namedItem( u"layers"_s ).toElement();
  QDomNodeList layerNodeList = element.elementsByTagName( u"layer"_s );
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
        mModel->setData( idx, element.attribute( u"attribute-index"_s, u"-1"_s ) );

        idx = mModel->index( idx.row(), ALLOW_DD_SYMBOL_BLOCKS_COL, idx.parent() );
        mModel->setData( idx, element.attribute( u"use_symbol_blocks"_s, u"0"_s ), Qt::CheckStateRole );

        idx = mModel->index( idx.row(), MAXIMUM_DD_SYMBOL_BLOCKS_COL, idx.parent() );
        mModel->setData( idx, element.attribute( u"max_number_of_classes"_s, u"-1"_s ) );
      }
      else
      {
        QgsDebugMsgLevel( u" Layer '%1' found in the DXF Export settings XML file, but not present in the project."_s.arg( element.attribute( u"name"_s ) ), 1 );
      }
    }
  }

  element = dxfElement.namedItem( u"use_layer_title"_s ).toElement();
  value = QgsXmlUtils::readVariant( element.firstChildElement() );
  if ( !value.isNull() )
    mLayerTitleAsName->setChecked( value == true );

  element = dxfElement.namedItem( u"use_map_extent"_s ).toElement();
  value = QgsXmlUtils::readVariant( element.firstChildElement() );
  if ( !value.isNull() )
    mMapExtentCheckBox->setChecked( value == true );

  element = dxfElement.namedItem( u"force_2d"_s ).toElement();
  value = QgsXmlUtils::readVariant( element.firstChildElement() );
  if ( !value.isNull() )
    mForce2d->setChecked( value == true );

  element = dxfElement.namedItem( u"mtext"_s ).toElement();
  value = QgsXmlUtils::readVariant( element.firstChildElement() );
  if ( !value.isNull() )
    mMTextCheckBox->setChecked( value == true );

  element = dxfElement.namedItem( u"selected_features_only"_s ).toElement();
  value = QgsXmlUtils::readVariant( element.firstChildElement() );
  if ( !value.isNull() )
    mSelectedFeaturesOnly->setChecked( value == true );

  element = dxfElement.namedItem( u"hairline_width_export"_s ).toElement();
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
  if ( !outputFileName.endsWith( u".xml"_s, Qt::CaseInsensitive ) )
  {
    outputFileName += ".xml"_L1;
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
  const QDomDocumentType documentType = DomImplementation.createDocumentType( u"qgis"_s, u"http://mrcc.com/qgis.dtd"_s, u"SYSTEM"_s );
  QDomDocument domDocument( documentType );

  QDomElement rootElement = domDocument.createElement( u"qgis"_s );
  rootElement.setAttribute( u"version"_s, Qgis::version() );
  domDocument.appendChild( rootElement );

  QDomElement dxfElement = domDocument.createElement( u"dxf_settings"_s );
  rootElement.appendChild( dxfElement );

  QDomElement symbologyModeElement = domDocument.createElement( u"symbology_mode"_s );
  symbologyModeElement.appendChild( QgsXmlUtils::writeVariant( static_cast<int>( symbologyMode() ), doc ) );
  dxfElement.appendChild( symbologyModeElement );

  QDomElement symbologyScaleElement = domDocument.createElement( u"symbology_scale"_s );
  symbologyScaleElement.appendChild( QgsXmlUtils::writeVariant( symbologyScale(), doc ) );
  dxfElement.appendChild( symbologyScaleElement );

  QDomElement encodingElement = domDocument.createElement( u"encoding"_s );
  encodingElement.appendChild( QgsXmlUtils::writeVariant( encoding(), doc ) );
  dxfElement.appendChild( encodingElement );

  QDomElement crsElement = domDocument.createElement( u"crs"_s );
  crsElement.appendChild( QgsXmlUtils::writeVariant( crs(), doc ) );
  dxfElement.appendChild( crsElement );

  QDomElement mapThemeElement = domDocument.createElement( u"map_theme"_s );
  mapThemeElement.appendChild( QgsXmlUtils::writeVariant( mapTheme(), doc ) );
  dxfElement.appendChild( mapThemeElement );

  QDomElement layersElement = domDocument.createElement( u"layers"_s );
  QgsVectorLayerRef vlRef;
  const QgsReadWriteContext rwContext = QgsReadWriteContext();

  for ( const auto &dxfLayer : layers() )
  {
    QDomElement layerElement = domDocument.createElement( u"layer"_s );
    vlRef.setLayer( dxfLayer.layer() );
    vlRef.writeXml( layerElement, rwContext );
    layerElement.setAttribute( u"attribute-index"_s, dxfLayer.layerOutputAttributeIndex() );
    layerElement.setAttribute( u"use_symbol_blocks"_s, dxfLayer.buildDataDefinedBlocks() );
    layerElement.setAttribute( u"max_number_of_classes"_s, dxfLayer.dataDefinedBlocksMaximumNumberOfClasses() );
    layersElement.appendChild( layerElement );
  }
  dxfElement.appendChild( layersElement );

  QDomElement titleAsNameElement = domDocument.createElement( u"use_layer_title"_s );
  titleAsNameElement.appendChild( QgsXmlUtils::writeVariant( layerTitleAsName(), doc ) );
  dxfElement.appendChild( titleAsNameElement );

  QDomElement useMapExtentElement = domDocument.createElement( u"use_map_extent"_s );
  useMapExtentElement.appendChild( QgsXmlUtils::writeVariant( exportMapExtent(), doc ) );
  dxfElement.appendChild( useMapExtentElement );

  QDomElement force2dElement = domDocument.createElement( u"force_2d"_s );
  force2dElement.appendChild( QgsXmlUtils::writeVariant( force2d(), doc ) );
  dxfElement.appendChild( force2dElement );

  QDomElement useMTextElement = domDocument.createElement( u"mtext"_s );
  useMTextElement.appendChild( QgsXmlUtils::writeVariant( useMText(), doc ) );
  dxfElement.appendChild( useMTextElement );

  QDomElement selectedFeatures = domDocument.createElement( u"selected_features_only"_s );
  selectedFeatures.appendChild( QgsXmlUtils::writeVariant( selectedFeaturesOnly(), doc ) );
  dxfElement.appendChild( selectedFeatures );

  QDomElement hairlineWidthExportElem = domDocument.createElement( u"hairline_width_export"_s );
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
  settings.setValue( u"qgis/lastDxfDir"_s, dxfFileInfo.absolutePath() );
  settings.setValue( u"qgis/lastDxfSymbologyMode"_s, mSymbologyModeComboBox->currentIndex() );
  settings.setValue( u"qgis/lastSymbologyExportScale"_s, mScaleWidget->scale() != 0 ? 1.0 / mScaleWidget->scale() : 0 );
  settings.setValue( u"qgis/lastDxfMapRectangle"_s, mMapExtentCheckBox->isChecked() );
  settings.setValue( u"qgis/lastDxfSelectedFeaturesOnly"_s, mSelectedFeaturesOnly->isChecked() );
  settings.setValue( u"qgis/lastDxfLayerTitleAsName"_s, mLayerTitleAsName->isChecked() );
  settings.setValue( u"qgis/lastDxfEncoding"_s, mEncoding->currentText() );
  settings.setValue( u"qgis/lastDxfCrs"_s, QString::number( mCRS.srsid() ) );
  settings.setValue( u"qgis/lastDxfUseMText"_s, mMTextCheckBox->isChecked() );
  settings.setValue( u"qgis/lastDxfForce2d"_s, mForce2d->isChecked() );
  settings.setValue( u"qgis/lastDxfHairlineWidthExport"_s, mHairlineWidthExportCheckBox->isChecked() );

  QgsProject::instance()->writeEntry( u"dxf"_s, u"/lastDxfSymbologyMode"_s, mSymbologyModeComboBox->currentIndex() );
  QgsProject::instance()->writeEntry( u"dxf"_s, u"/lastSymbologyExportScale"_s, mScaleWidget->scale() != 0 ? 1.0 / mScaleWidget->scale() : 0 );
  QgsProject::instance()->writeEntry( u"dxf"_s, u"/lastDxfLayerTitleAsName"_s, mLayerTitleAsName->isChecked() );
  QgsProject::instance()->writeEntry( u"dxf"_s, u"/lastDxfMapRectangle"_s, mMapExtentCheckBox->isChecked() );
  QgsProject::instance()->writeEntry( u"dxf"_s, u"/lastDxfSelectedFeaturesOnly"_s, mSelectedFeaturesOnly->isChecked() );
  QgsProject::instance()->writeEntry( u"dxf"_s, u"/lastDxfEncoding"_s, mEncoding->currentText() );
  QgsProject::instance()->writeEntry( u"dxf"_s, u"/lastVisibilityPreset"_s, mVisibilityPresets->currentText() );
  QgsProject::instance()->writeEntry( u"dxf"_s, u"/lastDxfCrs"_s, QString::number( mCRS.srsid() ) );
  QgsProject::instance()->writeEntry( u"dxf"_s, u"/lastDxfUseMText"_s, mMTextCheckBox->isChecked() );
  QgsProject::instance()->writeEntry( u"dxf"_s, u"/lastDxfForce2d"_s, mForce2d->isChecked() );
  QgsProject::instance()->writeEntry( u"dxf"_s, u"/lastDxfHairlineWidthExport"_s, mHairlineWidthExportCheckBox->isChecked() );

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
  QgsHelp::openHelp( u"managing_data_source/create_layers.html#create-dxf-files"_s );
}
