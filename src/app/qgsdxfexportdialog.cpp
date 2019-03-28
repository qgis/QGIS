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
#include "qgslayertreemapcanvasbridge.h"
#include "qgsmapthemecollection.h"
#include "qgsmapcanvas.h"
#include "qgsprojectionselectiondialog.h"
#include "qgssettings.h"
#include "qgsgui.h"

#include <QFileDialog>
#include <QPushButton>

FieldSelectorDelegate::FieldSelectorDelegate( QObject *parent )
  : QItemDelegate( parent )
{
}

QWidget *FieldSelectorDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( option );

  const QgsVectorLayerAndAttributeModel *m = qobject_cast< const QgsVectorLayerAndAttributeModel *>( index.model() );
  if ( !m )
    return nullptr;

  QgsVectorLayer *vl = m->vectorLayer( index );
  if ( !vl )
    return nullptr;

  QgsFieldComboBox *w = new QgsFieldComboBox( parent );
  w->setLayer( vl );
  w->setAllowEmptyFieldName( true );
  return w;
}

void FieldSelectorDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  const QgsVectorLayerAndAttributeModel *m = dynamic_cast< const QgsVectorLayerAndAttributeModel *>( index.model() );
  if ( !m )
    return;

  QgsVectorLayer *vl = m->vectorLayer( index );
  if ( !vl )
    return;

  QgsFieldComboBox *fcb = qobject_cast<QgsFieldComboBox *>( editor );
  if ( !fcb )
    return;

  int idx = m->attributeIndex( vl );
  if ( vl->fields().exists( idx ) )
    fcb->setField( vl->fields().at( idx ).name() );
}

void FieldSelectorDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QgsVectorLayerAndAttributeModel *m = qobject_cast< QgsVectorLayerAndAttributeModel *>( model );
  if ( !m )
    return;

  QgsVectorLayer *vl = m->vectorLayer( index );
  if ( !vl )
    return;

  QgsFieldComboBox *fcb = qobject_cast<QgsFieldComboBox *>( editor );
  if ( !fcb )
    return;

  model->setData( index, vl->fields().lookupField( fcb->currentField() ) );
}

QgsVectorLayerAndAttributeModel::QgsVectorLayerAndAttributeModel( QgsLayerTree *rootNode, QObject *parent )
  : QgsLayerTreeModel( rootNode, parent )
{
}

int QgsVectorLayerAndAttributeModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  return 2;
}

Qt::ItemFlags QgsVectorLayerAndAttributeModel::flags( const QModelIndex &index ) const
{
  if ( index.column() == 0 )
    return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;

  QgsVectorLayer *vl = vectorLayer( index );
  if ( !vl )
    return Qt::ItemIsEnabled;
  else
    return Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

QgsVectorLayer *QgsVectorLayerAndAttributeModel::vectorLayer( const QModelIndex &idx ) const
{
  QgsLayerTreeNode *n = index2node( idx );
  if ( !n || !QgsLayerTree::isLayer( n ) )
    return nullptr;

  return dynamic_cast<QgsVectorLayer *>( QgsLayerTree::toLayer( n )->layer() );
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
      if ( section == 0 )
        return tr( "Layer" );
      else if ( section == 1 )
        return tr( "Output layer attribute" );
    }
    else if ( role == Qt::ToolTipRole )
    {
      if ( section == 1 )
        return tr( "Attribute containing the name of the destination layer in the DXF output." );
    }
  }
  return QVariant();
}

QVariant QgsVectorLayerAndAttributeModel::data( const QModelIndex &idx, int role ) const
{
  if ( idx.column() == 0 )
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
        QVariant v = data( idx.child( n, 0 ), role );
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

  QgsVectorLayer *vl = vectorLayer( idx );
  if ( vl )
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
  if ( index.column() == 0 && role == Qt::CheckStateRole )
  {
    int i = 0;
    for ( i = 0; ; i++ )
    {
      QModelIndex child = index.child( i, 0 );
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

  if ( index.column() == 1 )
  {
    if ( role != Qt::EditRole )
      return false;

    QgsVectorLayer *vl = vectorLayer( index );
    if ( vl )
    {
      mAttributeIdx[ vl ] = value.toInt();
      return true;
    }
  }

  return QgsLayerTreeModel::setData( index, value, role );
}


QList< QgsDxfExport::DxfLayer > QgsVectorLayerAndAttributeModel::layers() const
{
  QList< QgsDxfExport::DxfLayer > layers;
  QHash< QString, int > layerIdx;

  Q_FOREACH ( const QModelIndex &idx, mCheckedLeafs )
  {
    QgsLayerTreeNode *node = index2node( idx );
    if ( QgsLayerTree::isGroup( node ) )
    {
      Q_FOREACH ( QgsLayerTreeLayer *treeLayer, QgsLayerTree::toGroup( node )->findLayers() )
      {
        QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( treeLayer->layer() );
        Q_ASSERT( vl );
        if ( !layerIdx.contains( vl->id() ) )
        {
          layerIdx.insert( vl->id(), layers.size() );
          layers << QgsDxfExport::DxfLayer( vl, mAttributeIdx.value( vl, -1 ) );
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
        layers << QgsDxfExport::DxfLayer( vl, mAttributeIdx.value( vl, -1 ) );
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
    Q_FOREACH ( const QgsMapLayer *ml, QgisApp::instance()->mapCanvas()->layers() )
    {
      const QgsVectorLayer *vl = qobject_cast<const QgsVectorLayer *>( ml );
      if ( !vl )
        continue;
      visibleLayers.insert( vl->id() );
    }
  }
  else
  {
    visibleLayers = QgsProject::instance()->mapThemeCollection()->mapThemeVisibleLayerIds( name ).toSet();
  }

  if ( visibleLayers.isEmpty() )
    return;

  mCheckedLeafs.clear();
  applyVisibility( visibleLayers, rootGroup() );

  emit dataChanged( QModelIndex(), QModelIndex() );
}

void QgsVectorLayerAndAttributeModel::applyVisibility( QSet<QString> &visibleLayers, QgsLayerTreeNode *node )
{
  QgsLayerTreeGroup *group = QgsLayerTree::isGroup( node ) ? QgsLayerTree::toGroup( node ) : nullptr;
  if ( !group )
    return;

  Q_FOREACH ( QgsLayerTreeNode *child, node->children() )
  {
    if ( QgsLayerTree::isLayer( child ) )
    {
      QgsVectorLayer *vl = qobject_cast< QgsVectorLayer * >( QgsLayerTree::toLayer( child )->layer() );
      if ( vl && visibleLayers.contains( vl->id() ) )
      {
        visibleLayers.remove( vl->id() );
        mCheckedLeafs.insert( node2index( child ) );
      }
      continue;
    }

    applyVisibility( visibleLayers, child );
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
    Q_FOREACH ( QgsLayerTreeNode *child, QgsLayerTree::toGroup( node )->children() )
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

  emit dataChanged( QModelIndex(), QModelIndex() );
}

void QgsVectorLayerAndAttributeModel::deSelectAll()
{
  mCheckedLeafs.clear();

  QSet<QString> noLayers;
  applyVisibility( noLayers, rootGroup() );

  emit dataChanged( QModelIndex(), QModelIndex() );
}

QgsDxfExportDialog::QgsDxfExportDialog( QWidget *parent, Qt::WindowFlags f )
  : QDialog( parent, f )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  connect( mVisibilityPresets, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsDxfExportDialog::mVisibilityPresets_currentIndexChanged );
  connect( mCrsSelector, &QgsProjectionSelectionWidget::crsChanged, this, &QgsDxfExportDialog::mCrsSelector_crsChanged );

  QgsSettings settings;

  mLayerTreeGroup = QgsProject::instance()->layerTreeRoot()->clone();
  cleanGroup( mLayerTreeGroup );

  mFieldSelectorDelegate = new FieldSelectorDelegate( this );
  mTreeView->setEditTriggers( QAbstractItemView::AllEditTriggers );
  mTreeView->setItemDelegate( mFieldSelectorDelegate );

  QgsLayerTreeModel *model = new QgsVectorLayerAndAttributeModel( mLayerTreeGroup, this );
  model->setFlags( nullptr );
  mTreeView->setModel( model );
  mTreeView->resizeColumnToContents( 0 );
  mTreeView->header()->show();

  mFileName->setStorageMode( QgsFileWidget::SaveFile );
  mFileName->setFilter( tr( "DXF files" ) + " (*.dxf *.DXF)" );
  mFileName->setDialogTitle( tr( "Export as DXF" ) );
  mFileName->setDefaultRoot( settings.value( QStringLiteral( "qgis/lastDxfDir" ), QDir::homePath() ).toString() );

  connect( this, &QDialog::accepted, this, &QgsDxfExportDialog::saveSettings );
  connect( mSelectAllButton, &QAbstractButton::clicked, this, &QgsDxfExportDialog::selectAll );
  connect( mDeselectAllButton, &QAbstractButton::clicked, this, &QgsDxfExportDialog::deSelectAll );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsDxfExportDialog::showHelp );

  connect( mFileName, &QgsFileWidget::fileChanged, this, [ = ]( const QString & filePath )
  {
    QgsSettings settings;
    QFileInfo tmplFileInfo( filePath );
    settings.setValue( QStringLiteral( "qgis/lastDxfDir" ), tmplFileInfo.absolutePath() );

    setOkEnabled();
  } );

  //last dxf symbology mode
  mSymbologyModeComboBox->setCurrentIndex( QgsProject::instance()->readEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastDxfSymbologyMode" ), settings.value( QStringLiteral( "qgis/lastDxfSymbologyMode" ), "2" ).toString() ).toInt() );

  //last symbol scale
  mScaleWidget->setMapCanvas( QgisApp::instance()->mapCanvas() );
  double oldScale = QgsProject::instance()->readEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastSymbologyExportScale" ), settings.value( QStringLiteral( "qgis/lastSymbologyExportScale" ), "1/50000" ).toString() ).toDouble();
  if ( oldScale != 0.0 )
    mScaleWidget->setScale( 1.0 / oldScale );
  mLayerTitleAsName->setChecked( QgsProject::instance()->readEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastDxfLayerTitleAsName" ), settings.value( QStringLiteral( "qgis/lastDxfLayerTitleAsName" ), "false" ).toString() ) != QLatin1String( "false" ) );
  mMapExtentCheckBox->setChecked( QgsProject::instance()->readEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastDxfMapRectangle" ), settings.value( QStringLiteral( "qgis/lastDxfMapRectangle" ), "false" ).toString() ) != QLatin1String( "false" ) );
  mMTextCheckBox->setChecked( QgsProject::instance()->readEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastDxfUseMText" ), settings.value( QStringLiteral( "qgis/lastDxfUseMText" ), "true" ).toString() ) != QLatin1String( "false" ) );

  QStringList ids = QgsProject::instance()->mapThemeCollection()->mapThemes();
  ids.prepend( QString() );
  mVisibilityPresets->addItems( ids );
  mVisibilityPresets->setCurrentIndex( mVisibilityPresets->findText( QgsProject::instance()->readEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastVisibliltyPreset" ), QString() ) ) );

  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );

  long crsid = QgsProject::instance()->readEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastDxfCrs" ),
               settings.value( QStringLiteral( "qgis/lastDxfCrs" ), QString::number( QgsProject::instance()->crs().srsid() ) ).toString()
                                                ).toLong();
  mCRS = QgsCoordinateReferenceSystem::fromSrsId( crsid );
  mCrsSelector->setCrs( mCRS );
  mCrsSelector->setLayerCrs( mCRS );
  mCrsSelector->setMessage( tr( "Select the coordinate reference system for the dxf file. "
                                "The data points will be transformed from the layer coordinate reference system." ) );

  mEncoding->addItems( QgsDxfExport::encodings() );
  mEncoding->setCurrentIndex( mEncoding->findText( QgsProject::instance()->readEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastDxfEncoding" ), settings.value( QStringLiteral( "qgis/lastDxfEncoding" ), "CP1252" ).toString() ) ) );
}


QgsDxfExportDialog::~QgsDxfExportDialog()
{
  delete mLayerTreeGroup;
}

void QgsDxfExportDialog::mVisibilityPresets_currentIndexChanged( int index )
{
  Q_UNUSED( index );
  QgsVectorLayerAndAttributeModel *model = qobject_cast< QgsVectorLayerAndAttributeModel * >( mTreeView->model() );
  Q_ASSERT( model );
  model->applyVisibilityPreset( mVisibilityPresets->currentText() );
}

void QgsDxfExportDialog::cleanGroup( QgsLayerTreeNode *node )
{
  QgsLayerTreeGroup *group = QgsLayerTree::isGroup( node ) ? QgsLayerTree::toGroup( node ) : nullptr;
  if ( !group )
    return;

  QList<QgsLayerTreeNode *> toRemove;
  Q_FOREACH ( QgsLayerTreeNode *child, node->children() )
  {
    if ( QgsLayerTree::isLayer( child ) && QgsLayerTree::toLayer( child )->layer()->type() != QgsMapLayerType::VectorLayer )
    {
      toRemove << child;
      continue;
    }

    cleanGroup( child );

    if ( QgsLayerTree::isGroup( child ) && child->children().isEmpty() )
      toRemove << child;
  }

  Q_FOREACH ( QgsLayerTreeNode *child, toRemove )
    group->removeChildNode( child );
}


void QgsDxfExportDialog::selectAll()
{
  QgsVectorLayerAndAttributeModel *model = qobject_cast< QgsVectorLayerAndAttributeModel *>( mTreeView->model() );
  Q_ASSERT( model );
  model->selectAll();
}

void QgsDxfExportDialog::deSelectAll()
{
  QgsVectorLayerAndAttributeModel *model = qobject_cast< QgsVectorLayerAndAttributeModel *>( mTreeView->model() );
  Q_ASSERT( model );
  model->deSelectAll();
}


QList< QgsDxfExport::DxfLayer > QgsDxfExportDialog::layers() const
{
  const QgsVectorLayerAndAttributeModel *model = dynamic_cast< const QgsVectorLayerAndAttributeModel *>( mTreeView->model() );
  Q_ASSERT( model );
  return model->layers();
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


QgsDxfExport::SymbologyExport QgsDxfExportDialog::symbologyMode() const
{
  return ( QgsDxfExport::SymbologyExport )mSymbologyModeComboBox->currentIndex();
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
  settings.setValue( QStringLiteral( "qgis/lastDxfLayerTitleAsName" ), mLayerTitleAsName->isChecked() );
  settings.setValue( QStringLiteral( "qgis/lastDxfEncoding" ), mEncoding->currentText() );
  settings.setValue( QStringLiteral( "qgis/lastDxfCrs" ), QString::number( mCRS.srsid() ) );
  settings.setValue( QStringLiteral( "qgis/lastDxfUseMText" ), mMTextCheckBox->isChecked() );

  QgsProject::instance()->writeEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastDxfSymbologyMode" ), mSymbologyModeComboBox->currentIndex() );
  QgsProject::instance()->writeEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastSymbologyExportScale" ), mScaleWidget->scale() != 0 ? 1.0 / mScaleWidget->scale() : 0 );
  QgsProject::instance()->writeEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastDxfLayerTitleAsName" ), mLayerTitleAsName->isChecked() );
  QgsProject::instance()->writeEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastDxfMapRectangle" ), mMapExtentCheckBox->isChecked() );
  QgsProject::instance()->writeEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastDxfEncoding" ), mEncoding->currentText() );
  QgsProject::instance()->writeEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastVisibilityPreset" ), mVisibilityPresets->currentText() );
  QgsProject::instance()->writeEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastDxfCrs" ), QString::number( mCRS.srsid() ) );
  QgsProject::instance()->writeEntry( QStringLiteral( "dxf" ), QStringLiteral( "/lastDxfUseMText" ), mMTextCheckBox->isChecked() );
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
