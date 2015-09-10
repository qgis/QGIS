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
#include "qgis.h"
#include "qgsfieldcombobox.h"
#include "qgisapp.h"
#include "qgslayertreemapcanvasbridge.h"
#include "qgsvisibilitypresetcollection.h"
#include "qgsmapcanvas.h"

#include <QFileDialog>
#include <QPushButton>
#include <QSettings>

FieldSelectorDelegate::FieldSelectorDelegate( QObject *parent )
    : QItemDelegate( parent )
{
}

QWidget *FieldSelectorDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( option );

  const QgsVectorLayerAndAttributeModel *m = qobject_cast< const QgsVectorLayerAndAttributeModel *>( index.model() );
  if ( !m )
    return 0;

  QgsVectorLayer *vl = m->vectorLayer( index );
  if ( !vl )
    return 0;


  QgsFieldComboBox *w = new QgsFieldComboBox( parent );
  w->setLayer( vl );
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
    fcb->setField( vl->fields()[ idx ].name() );
}

void FieldSelectorDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QgsVectorLayerAndAttributeModel *m = dynamic_cast< QgsVectorLayerAndAttributeModel *>( model );
  if ( !m )
    return;

  QgsVectorLayer *vl = m->vectorLayer( index );
  if ( !vl )
    return;

  QgsFieldComboBox *fcb = qobject_cast<QgsFieldComboBox *>( editor );
  if ( !fcb )
    return;

  model->setData( index, vl->fieldNameIndex( fcb->currentField() ) );
}

QgsVectorLayerAndAttributeModel::QgsVectorLayerAndAttributeModel( QgsLayerTreeGroup* rootNode, QObject *parent )
    : QgsLayerTreeModel( rootNode, parent )
{
}

QgsVectorLayerAndAttributeModel::~QgsVectorLayerAndAttributeModel()
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
    return 0;

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

QVariant QgsVectorLayerAndAttributeModel::data( const QModelIndex& idx, int role ) const
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
        return vl->fields()[ idx ].name();
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


QList< QPair<QgsVectorLayer *, int> > QgsVectorLayerAndAttributeModel::layers() const
{
  QList< QPair<QgsVectorLayer *, int> > layers;
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
          layers << qMakePair<QgsVectorLayer *, int>( vl, mAttributeIdx.value( vl, -1 ) );
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
        layers << qMakePair<QgsVectorLayer *, int>( vl, mAttributeIdx.value( vl, -1 ) );
      }
    }
  }

  QgsLayerTreeMapCanvasBridge* bridge = QgisApp::instance()->layerTreeCanvasBridge();
  QStringList inDrawingOrder = bridge->hasCustomLayerOrder() ? bridge->customLayerOrder() : bridge->defaultLayerOrder();
  QList< QPair<QgsVectorLayer *, int> > layersInROrder;

  for ( int i = inDrawingOrder.size() - 1; i >= 0; i-- )
  {
    int idx = layerIdx.value( inDrawingOrder[i], -1 );
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
    visibleLayers = QgsProject::instance()->visibilityPresetCollection()->presetVisibleLayers( name ).toSet();
  }

  if ( visibleLayers.isEmpty() )
    return;

  mCheckedLeafs.clear();
  applyVisibility( visibleLayers, rootGroup() );

  emit dataChanged( QModelIndex(), QModelIndex() );
}

void QgsVectorLayerAndAttributeModel::applyVisibility( QSet<QString> &visibleLayers, QgsLayerTreeNode *node )
{
  QgsLayerTreeGroup *group = QgsLayerTree::isGroup( node ) ? QgsLayerTree::toGroup( node ) : 0;
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

void QgsVectorLayerAndAttributeModel::unSelectAll()
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

  mLayerTreeGroup = QgsLayerTree::toGroup( QgsProject::instance()->layerTreeRoot()->clone() );
  cleanGroup( mLayerTreeGroup );

  mFieldSelectorDelegate = new FieldSelectorDelegate( this );
  mTreeView->setEditTriggers( QAbstractItemView::AllEditTriggers );
  mTreeView->setItemDelegate( mFieldSelectorDelegate );

  QgsLayerTreeModel *model = new QgsVectorLayerAndAttributeModel( mLayerTreeGroup, this );
  model->setFlags( 0 );
  mTreeView->setModel( model );
  mTreeView->resizeColumnToContents( 0 );
  mTreeView->header()->show();

  connect( mFileLineEdit, SIGNAL( textChanged( const QString& ) ), this, SLOT( setOkEnabled() ) );
  connect( this, SIGNAL( accepted() ), this, SLOT( saveSettings() ) );
  connect( mSelectAllButton, SIGNAL( clicked() ), this, SLOT( selectAll() ) );
  connect( mUnSelectAllButton, SIGNAL( clicked() ), this, SLOT( unSelectAll() ) );

  //last dxf symbology mode
  QSettings s;
  mSymbologyModeComboBox->setCurrentIndex( QgsProject::instance()->readEntry( "dxf", "/lastDxfSymbologyMode", s.value( "qgis/lastDxfSymbologyMode", "2" ).toString() ).toInt() );

  //last symbol scale
  mScaleWidget->setMapCanvas( QgisApp::instance()->mapCanvas() );
  mScaleWidget->setScale( QgsProject::instance()->readEntry( "dxf", "/lastSymbologyExportScale", s.value( "qgis/lastSymbologyExportScale", "1/50000" ).toString() ).toDouble() );
  mMapExtentCheckBox->setChecked( QgsProject::instance()->readEntry( "dxf", "/lastDxfMapRectangle", s.value( "qgis/lastDxfMapRectangle", "false" ).toString() ) != "false" );

  QStringList ids = QgsProject::instance()->visibilityPresetCollection()->presets();
  ids.prepend( "" );
  mVisibilityPresets->addItems( ids );
  mVisibilityPresets->setCurrentIndex( mVisibilityPresets->findText( QgsProject::instance()->readEntry( "dxf", "/lastVisibilityPreset", "" ) ) );

  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
  restoreGeometry( s.value( "/Windows/DxfExport/geometry" ).toByteArray() );
  mEncoding->addItems( QgsDxfExport::encodings() );
  mEncoding->setCurrentIndex( mEncoding->findText( QgsProject::instance()->readEntry( "dxf", "/lastDxfEncoding", s.value( "qgis/lastDxfEncoding", "CP1252" ).toString() ) ) );
}


QgsDxfExportDialog::~QgsDxfExportDialog()
{
  delete mLayerTreeGroup;

  QSettings().setValue( "/Windows/DxfExport/geometry", saveGeometry() );
}

void QgsDxfExportDialog::on_mVisibilityPresets_currentIndexChanged( int index )
{
  Q_UNUSED( index );
  QgsVectorLayerAndAttributeModel *model = dynamic_cast< QgsVectorLayerAndAttributeModel * >( mTreeView->model() );
  Q_ASSERT( model );
  model->applyVisibilityPreset( mVisibilityPresets->currentText() );
}

void QgsDxfExportDialog::cleanGroup( QgsLayerTreeNode *node )
{
  QgsLayerTreeGroup *group = QgsLayerTree::isGroup( node ) ? QgsLayerTree::toGroup( node ) : 0;
  if ( !group )
    return;

  QList<QgsLayerTreeNode *> toRemove;
  Q_FOREACH ( QgsLayerTreeNode *child, node->children() )
  {
    if ( QgsLayerTree::isLayer( child ) && QgsLayerTree::toLayer( child )->layer()->type() != QgsMapLayer::VectorLayer )
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
  QgsVectorLayerAndAttributeModel *model = dynamic_cast< QgsVectorLayerAndAttributeModel *>( mTreeView->model() );
  Q_ASSERT( model );
  model->selectAll();
}

void QgsDxfExportDialog::unSelectAll()
{
  QgsVectorLayerAndAttributeModel *model = dynamic_cast< QgsVectorLayerAndAttributeModel *>( mTreeView->model() );
  Q_ASSERT( model );
  model->unSelectAll();
}


QList< QPair<QgsVectorLayer *, int> > QgsDxfExportDialog::layers() const
{
  const QgsVectorLayerAndAttributeModel *model = dynamic_cast< const QgsVectorLayerAndAttributeModel *>( mTreeView->model() );
  Q_ASSERT( model );
  return model->layers();
}


double QgsDxfExportDialog::symbologyScale() const
{
  double scale = 1 / mScaleWidget->scale();
  if ( qgsDoubleNear( scale, 0.0 ) )
  {
    return 1.0;
  }
  return scale;
}


QString QgsDxfExportDialog::saveFile() const
{
  return mFileLineEdit->text();
}


QgsDxfExport::SymbologyExport QgsDxfExportDialog::symbologyMode() const
{
  return ( QgsDxfExport::SymbologyExport )mSymbologyModeComboBox->currentIndex();
}


void QgsDxfExportDialog::on_mFileSelectionButton_clicked()
{
  //get last dxf save directory
  QSettings s;
  QString lastSavePath = s.value( "qgis/lastDxfDir" ).toString();

  QString filePath = QFileDialog::getSaveFileName( 0, tr( "Export as DXF" ), lastSavePath, tr( "DXF files *.dxf *.DXF" ) );
  if ( !filePath.isEmpty() )
  {
    mFileLineEdit->setText( filePath );
  }
}


void QgsDxfExportDialog::setOkEnabled()
{
  QPushButton* btn = buttonBox->button( QDialogButtonBox::Ok );

  QString filePath = mFileLineEdit->text();
  if ( filePath.isEmpty() )
  {
    btn->setEnabled( false );
  }

  QFileInfo fi( filePath );
  btn->setEnabled( fi.absoluteDir().exists() );
}


bool QgsDxfExportDialog::exportMapExtent() const
{
  return mMapExtentCheckBox->isChecked();
}


void QgsDxfExportDialog::saveSettings()
{
  QSettings s;
  QFileInfo dxfFileInfo( mFileLineEdit->text() );
  s.setValue( "qgis/lastDxfDir", dxfFileInfo.absolutePath() );
  s.setValue( "qgis/lastDxfSymbologyMode", mSymbologyModeComboBox->currentIndex() );
  s.setValue( "qgis/lastSymbologyExportScale", mScaleWidget->scale() );
  s.setValue( "qgis/lastDxfMapRectangle", mMapExtentCheckBox->isChecked() );
  s.setValue( "qgis/lastDxfEncoding", mEncoding->currentText() );

  QgsProject::instance()->writeEntry( "dxf", "/lastDxfSymbologyMode", mSymbologyModeComboBox->currentIndex() );
  QgsProject::instance()->writeEntry( "dxf", "/lastSymbologyExportScale", mScaleWidget->scale() );
  QgsProject::instance()->writeEntry( "dxf", "/lastDxfMapRectangle", mMapExtentCheckBox->isChecked() );
  QgsProject::instance()->writeEntry( "dxf", "/lastDxfEncoding", mEncoding->currentText() );
  QgsProject::instance()->writeEntry( "dxf", "/lastVisibilityPreset", mVisibilityPresets->currentText() );
}


QString QgsDxfExportDialog::encoding() const
{
  return mEncoding->currentText();
}
