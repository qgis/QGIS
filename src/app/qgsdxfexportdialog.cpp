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

  if ( index.column() == ALLOW_DD_SYMBOL_BLOCKS_COL )
  {
    return nullptr;
  }
  else if ( index.column() == MAXIMUM_DD_SYMBOL_BLOCKS_COL )
  {
    QLineEdit *le = new QLineEdit( parent );
    le->setValidator( new QIntValidator( le ) );
    return le;
  }

  QgsVectorLayer *vl = indexToLayer( index.model(), index );
  if ( !vl )
    return nullptr;

  QgsFieldComboBox *w = new QgsFieldComboBox( parent );
  w->setLayer( vl );
  w->setAllowEmptyFieldName( true );
  return w;
}

void FieldSelectorDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  if ( index.column() == MAXIMUM_DD_SYMBOL_BLOCKS_COL )
  {
    QLineEdit *le = qobject_cast<QLineEdit *>( editor );
    if ( le )
    {
      le->setText( index.data().toString() );
    }
    return;
  }

  QgsVectorLayer *vl = indexToLayer( index.model(), index );
  if ( !vl )
    return;

  QgsFieldComboBox *fcb = qobject_cast<QgsFieldComboBox *>( editor );
  if ( !fcb )
    return;

  int idx = attributeIndex( index.model(), vl );
  if ( vl->fields().exists( idx ) )
    fcb->setField( vl->fields().at( idx ).name() );
}

void FieldSelectorDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  if ( index.column() == MAXIMUM_DD_SYMBOL_BLOCKS_COL )
  {
    QLineEdit *le = qobject_cast<QLineEdit *>( editor );
    if ( le )
    {
      model->setData( index, le->text().toInt() );
    }
  }

  QgsVectorLayer *vl = indexToLayer( index.model(), index );
  if ( !vl )
    return;

  QgsFieldComboBox *fcb = qobject_cast<QgsFieldComboBox *>( editor );
  if ( !fcb )
    return;

  model->setData( index, vl->fields().lookupField( fcb->currentField() ) );
}

QgsVectorLayer *FieldSelectorDelegate::indexToLayer( const QAbstractItemModel *model, const QModelIndex &index ) const
{
  const QgsLayerTreeProxyModel *proxy = qobject_cast< const QgsLayerTreeProxyModel *>( model );
  Q_ASSERT( proxy );

  const QgsVectorLayerAndAttributeModel *m = qobject_cast< const QgsVectorLayerAndAttributeModel *>( proxy->sourceModel() );
  Q_ASSERT( m );

  return m->vectorLayer( proxy->mapToSource( index ) );
}

int FieldSelectorDelegate::attributeIndex( const QAbstractItemModel *model, const QgsVectorLayer *vl ) const
{
  const QgsLayerTreeProxyModel *proxy = qobject_cast< const QgsLayerTreeProxyModel *>( model );
  Q_ASSERT( proxy );

  const QgsVectorLayerAndAttributeModel *m = qobject_cast< const QgsVectorLayerAndAttributeModel *>( proxy->sourceModel() );
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
    const QgsVectorLayer *vLayer = qobject_cast< const QgsVectorLayer *>( QgsProject::instance()->mapLayer( id ) );
    if ( vLayer )
    {
      mCreateDDBlockInfo[vLayer] = QgsDxfExportDialog::settingsDxfEnableDDBlocks->value();
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
    return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
  }
  else if ( index.column() == OUTPUT_LAYER_ATTRIBUTE_COL )
  {
    return vl ? Qt::ItemIsEnabled | Qt::ItemIsEditable : Qt::ItemIsEnabled;
  }
  else if ( index.column() == ALLOW_DD_SYMBOL_BLOCKS_COL )
  {
    return ( vl && vl->geometryType() == Qgis::GeometryType::Point ) ? Qt::ItemIsEnabled | Qt::ItemIsUserCheckable : Qt::ItemIsEnabled ;
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
    else
      return QgsLayerTreeModel::data( idx, role );
  }
  else if ( idx.column() == ALLOW_DD_SYMBOL_BLOCKS_COL )
  {
    if ( !vl || vl->geometryType() != Qgis::GeometryType::Point )
    {
      return QVariant();
    }

    bool checked = mCreateDDBlockInfo.contains( vl ) ? mCreateDDBlockInfo[vl] : false;
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


  if ( idx.column() == OUTPUT_LAYER_ATTRIBUTE_COL && vl )
  {
    int idx = mAttributeIdx.value( vl, -1 );
    if ( role == Qt::EditRole )
      return idx;

    if ( role == Qt::DisplayRole )
    {
      if ( vl->fields().exists( idx ) )
        return vl->fields().at( idx ).name();
      else
        return vl->name();
    }

    if ( role == Qt::ToolTipRole )
    {
      return tr( "Attribute containing the name of the destination layer in the DXF output." );
    }
  }

  return QVariant();
}

bool QgsVectorLayerAndAttributeModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( index.column() == LAYER_COL && role == Qt::CheckStateRole )
  {
    int i = 0;
    for ( i = 0; ; i++ )
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

  QgsVectorLayer *vl = vectorLayer( index );
  if ( index.column() == OUTPUT_LAYER_ATTRIBUTE_COL )
  {
    if ( role != Qt::EditRole )
      return false;


    if ( vl )
    {
      mAttributeIdx[ vl ] = value.toInt();
      return true;
    }
  }

  if ( index.column() == ALLOW_DD_SYMBOL_BLOCKS_COL && role == Qt::CheckStateRole )
  {
    if ( vl )
    {
      mCreateDDBlockInfo[ vl ] = value.toBool();
      return true;
    }
  }
  else if ( index.column() == MAXIMUM_DD_SYMBOL_BLOCKS_COL && role == Qt::EditRole )
  {
    if ( vl )
    {
      mDDBlocksMaxNumberOfClasses[ vl ] = value.toInt();
      return true;
    }
  }

  return QgsLayerTreeModel::setData( index, value, role );
}


QList< QgsDxfExport::DxfLayer > QgsVectorLayerAndAttributeModel::layers() const
{
  QList< QgsDxfExport::DxfLayer > layers;
  QHash< QString, int > layerIdx;

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
          layers << QgsDxfExport::DxfLayer( vl, mAttributeIdx.value( vl, -1 ), mCreateDDBlockInfo.value( vl, false ), mDDBlocksMaxNumberOfClasses.value( vl,  -1 ) );
        }
      }
    }
    else if ( QgsLayerTree::isLayer( node ) )
    {
      QgsVectorLayer *vl = qobject_cast< QgsVectorLayer *>( QgsLayerTree::toLayer( node )->layer() );
      Q_ASSERT( vl );
      if ( !layerIdx.contains( vl->id() ) )
      {
        layerIdx.insert( vl->id(), layers.size() );
        layers << QgsDxfExport::DxfLayer( vl, mAttributeIdx.value( vl, -1 ), mCreateDDBlockInfo.value( vl, false ), mDDBlocksMaxNumberOfClasses.value( vl,  -1 ) );
      }
    }
  }

  QList< QgsDxfExport::DxfLayer > layersInROrder;

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
      QgsVectorLayer *vl = qobject_cast< QgsVectorLayer * >( QgsLayerTree::toLayer( child )->layer() );
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
      QgsVectorLayer *vl = qobject_cast< QgsVectorLayer * >( QgsLayerTree::toLayer( child )->layer() );
      if ( vl )
      {
        const int attributeIndex = vl->fields().lookupField( vl->customProperty( QStringLiteral( "lastDxfOutputAttribute" ), -1 ).toString() );
        if ( attributeIndex > -1 )
        {
          mAttributeIdx[vl] = attributeIndex;

          QModelIndex idx = node2index( child );
          idx = index( idx.row(), 1, idx.parent() );
          emit dataChanged( idx, idx, QVector<int>() << Qt::EditRole );
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
      QgsVectorLayer *vl = qobject_cast< QgsVectorLayer * >( QgsLayerTree::toLayer( child )->layer() );
      if ( vl )
      {
        QModelIndex idx = node2index( child );
        const int attributeIndex = data( index( idx.row(), 1, idx.parent() ), Qt::EditRole ).toInt();
        const QgsFields fields = vl->fields();
        if ( attributeIndex > -1 && attributeIndex < fields.count() )
        {
          vl->setCustomProperty( QStringLiteral( "lastDxfOutputAttribute" ), fields.at( attributeIndex ).name() );
        }
        else
        {
          vl->removeCustomProperty( QStringLiteral( "lastDxfOutputAttribute" ) );
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

  connect( mFileName, &QgsFileWidget::fileChanged, this, [ = ]( const QString & filePath )
  {
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

  QStringList ids = QgsProject::instance()->mapThemeCollection()->mapThemes();
  ids.prepend( QString() );
  mVisibilityPresets->addItems( ids );
  mVisibilityPresets->setCurrentIndex( mVisibilityPresets->findText( QgsProject::instance()->readEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastVisibilityPreset" ), QString() ) ) );

  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );

  long crsid = QgsProject::instance()->readEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastDxfCrs" ),
               settings.value( QStringLiteral( "qgis/lastDxfCrs" ), QString::number( QgsProject::instance()->crs().srsid() ) ).toString()
                                                ).toLong();
  mCRS = QgsCoordinateReferenceSystem::fromSrsId( crsid );
  mCrsSelector->setCrs( mCRS );
  mCrsSelector->setLayerCrs( mCRS );
  mCrsSelector->setShowAccuracyWarnings( true );
  mCrsSelector->setMessage( tr( "Select the coordinate reference system for the dxf file. "
                                "The data points will be transformed from the layer coordinate reference system." ) );

  mEncoding->addItems( QgsDxfExport::encodings() );
  mEncoding->setCurrentIndex( mEncoding->findText( QgsProject::instance()->readEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastDxfEncoding" ), settings.value( QStringLiteral( "qgis/lastDxfEncoding" ), "CP1252" ).toString() ) ) );

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
    if ( QgsLayerTree::isLayer( child ) &&
         ( QgsLayerTree::toLayer( child )->layer()->type() != Qgis::LayerType::Vector ||
           ! QgsLayerTree::toLayer( child )->layer()->isSpatial() ) )
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


QList< QgsDxfExport::DxfLayer > QgsDxfExportDialog::layers() const
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
  return mSymbologyModeComboBox->currentData().value< Qgis::FeatureSymbologyExport >();
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
