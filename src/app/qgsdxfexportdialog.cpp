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
#include "qgsmapcanvas.h"
#include "qgsvisibilitypresets.h"

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

  int idx = m->mAttributeIdx.value( vl, -1 );
  if ( vl->pendingFields().exists( idx ) )
    fcb->setField( vl->pendingFields()[ idx ].name() );
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

QVariant QgsVectorLayerAndAttributeModel::data( const QModelIndex& idx, int role ) const
{
  if ( idx.column() == 0 )
  {
    if ( role == Qt::CheckStateRole )
      return mCheckedIndexes.contains( idx ) ? Qt::Checked : Qt::Unchecked;
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
      if ( vl->pendingFields().exists( idx ) )
        return vl->pendingFields()[ idx ].name();
      else
        return vl->name();
    }
  }

  return QVariant();
}

bool QgsVectorLayerAndAttributeModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( index.column() == 0 && role == Qt::CheckStateRole )
  {
    if ( value.toInt() == Qt::Checked )
      mCheckedIndexes.append( index );
    else
      mCheckedIndexes.removeAll( index );
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
  QHash< QgsMapLayer *, int > layerIdx;

  foreach ( const QModelIndex &idx, mCheckedIndexes )
  {
    QgsLayerTreeNode *node = index2node( idx );
    if ( QgsLayerTree::isGroup( node ) )
    {
      foreach ( QgsLayerTreeLayer *treeLayer, QgsLayerTree::toGroup( node )->findLayers() )
      {
        QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( treeLayer->layer() );
        Q_ASSERT( vl );
        if ( !layerIdx.contains( vl ) )
        {
          layerIdx.insert( vl, layers.size() );
          layers << qMakePair<QgsVectorLayer *, int>( vl, mAttributeIdx.value( vl, -1 ) );
        }
      }
    }
    else if ( QgsLayerTree::isLayer( node ) )
    {
      QgsVectorLayer *vl = qobject_cast< QgsVectorLayer *>( QgsLayerTree::toLayer( node )->layer() );
      Q_ASSERT( vl );
      if ( !layerIdx.contains( vl ) )
      {
        layerIdx.insert( vl, layers.size() );
        layers << qMakePair<QgsVectorLayer *, int>( vl, mAttributeIdx.value( vl, -1 ) );
      }
    }
  }

  QList<QgsMapLayer*> inDrawingOrder = QgisApp::instance()->mapCanvas()->layers();
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
  QSet<QString> visibleLayers = QgsVisibilityPresets::instance()->presetVisibleLayers( name ).toSet();
  if ( visibleLayers.isEmpty() )
    return;

  mCheckedIndexes.clear();
  applyVisibility( visibleLayers, rootGroup() );

  emit dataChanged( QModelIndex(), QModelIndex() );
}

void QgsVectorLayerAndAttributeModel::applyVisibility( QSet<QString> &visibleLayers, QgsLayerTreeNode *node )
{
  QgsLayerTreeGroup *group = QgsLayerTree::toGroup( node );
  if ( !group )
    return;

  foreach ( QgsLayerTreeNode *child, node->children() )
  {
    if ( QgsLayerTree::isLayer( child ) )
    {
      QgsVectorLayer *vl = qobject_cast< QgsVectorLayer * >( QgsLayerTree::toLayer( child )->layer() );
      if ( vl && visibleLayers.contains( vl->id() ) )
      {
        visibleLayers.remove( vl->id() );
        mCheckedIndexes.append( node2index( child ) );
      }
      continue;
    }

    applyVisibility( visibleLayers, child );
  }
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

  connect( mFileLineEdit, SIGNAL( textChanged( const QString& ) ), this, SLOT( setOkEnabled() ) );
  connect( this, SIGNAL( accepted() ), this, SLOT( saveSettings() ) );
  connect( mSelectAllButton, SIGNAL( clicked() ), this, SLOT( selectAll() ) );
  connect( mUnSelectAllButton, SIGNAL( clicked() ), this, SLOT( unSelectAll() ) );

  //last dxf symbology mode
  QSettings s;
  mSymbologyModeComboBox->setCurrentIndex( s.value( "qgis/lastDxfSymbologyMode", "2" ).toInt() );
  //last symbol scale
  mSymbologyScaleLineEdit->setText( s.value( "qgis/lastSymbologyExportScale", "50000" ).toString() );
  mMapExtentCheckBox->setChecked( s.value( "qgis/lastDxfMapRectangle", "false" ).toBool() );

  QStringList ids = QgsVisibilityPresets::instance()->presets();
  ids.prepend( "" );
  mVisibilityPresets->addItems( ids );

  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
  restoreGeometry( s.value( "/Windows/DxfExport/geometry" ).toByteArray() );
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
  QgsLayerTreeGroup *group = QgsLayerTree::toGroup( node );
  if ( !group )
    return;

  QList<QgsLayerTreeNode *> toRemove;
  foreach ( QgsLayerTreeNode *child, node->children() )
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

  foreach ( QgsLayerTreeNode *child, toRemove )
    group->removeChildNode( child );
}


void QgsDxfExportDialog::selectAll()
{
  mTreeView->selectAll();
}


void QgsDxfExportDialog::unSelectAll()
{
  mTreeView->clearSelection();
}


QList< QPair<QgsVectorLayer *, int> > QgsDxfExportDialog::layers() const
{
  const QgsVectorLayerAndAttributeModel *model = dynamic_cast< const QgsVectorLayerAndAttributeModel *>( mTreeView->model() );
  Q_ASSERT( model );
  return model->layers();
}


double QgsDxfExportDialog::symbologyScale() const
{
  double scale = mSymbologyScaleLineEdit->text().toDouble();
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
  s.setValue( "qgis/lastSymbologyExportScale", mSymbologyScaleLineEdit->text() );
  s.setValue( "qgis/lastDxfMapRectangle", mMapExtentCheckBox->isChecked() );
}
