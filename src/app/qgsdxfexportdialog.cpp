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

#include <QFileDialog>
#include <QPushButton>
#include <QSettings>

#if 0
FieldSelectorDelegate::FieldSelectorDelegate( QObject *parent ) : QItemDelegate( parent )
{
}

QWidget *FieldSelectorDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  QgsDebugCall;
  Q_UNUSED( option );

  QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( reinterpret_cast<QObject*>( index.internalPointer() ) );
  if ( !vl )
    return 0;

  QgsFieldComboBox *w = new QgsFieldComboBox( parent );
  w->setLayer( vl );
  return w;
}

void FieldSelectorDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  QgsDebugCall;
  QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( reinterpret_cast<QObject*>( index.internalPointer() ) );
  if ( !vl )
    return;

  const QgsVectorLayerAndAttributeModel *model = dynamic_cast< const QgsVectorLayerAndAttributeModel *>( index.model() );
  if ( !model )
    return;

  QgsFieldComboBox *fcb = qobject_cast<QgsFieldComboBox *>( editor );
  if ( !fcb )
    return;

  int idx = model->mAttributeIdx.value( vl, -1 );
  if ( vl->pendingFields().exists( idx ) )
    fcb->setField( vl->pendingFields()[ idx ].name() );
}

void FieldSelectorDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QgsDebugCall;
  QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( reinterpret_cast<QObject*>( index.internalPointer() ) );
  if ( !vl )
    return;

  QgsFieldComboBox *fcb = qobject_cast<QgsFieldComboBox *>( editor );
  if ( !fcb )
    return;

  model->setData( index, vl->fieldNameIndex( fcb->currentField() ) );
}
#endif

QgsVectorLayerAndAttributeModel::QgsVectorLayerAndAttributeModel( QgsLayerTreeGroup* rootNode, QObject *parent )
    : QgsLayerTreeModel( rootNode, parent )
{
}

QgsVectorLayerAndAttributeModel::~QgsVectorLayerAndAttributeModel()
{
}

QModelIndex QgsVectorLayerAndAttributeModel::index( int row, int column, const QModelIndex &parent ) const
{
  QgsLayerTreeNode *n = index2node( parent );
  if ( !QgsLayerTree::isLayer( n ) )
    return QgsLayerTreeModel::index( row, column, parent );

  if ( row != 0 || column != 0 )
    return QModelIndex();

  return createIndex( 0, 0, QgsLayerTree::toLayer( n )->layer() );
}

QModelIndex QgsVectorLayerAndAttributeModel::parent( const QModelIndex &child ) const
{
  QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( reinterpret_cast<QObject*>( child.internalPointer() ) );
  if ( !vl )
    return QgsLayerTreeModel::parent( child );

  QgsLayerTreeLayer *layer = rootGroup()->findLayer( vl->id() );
  Q_ASSERT( layer );
  QgsLayerTreeNode *parent = layer->parent();
  if ( !parent )
    return QModelIndex();

  int row = parent->children().indexOf( layer );
  Q_ASSERT( row >= 0 );
  return createIndex( row, 0, layer );
}

int QgsVectorLayerAndAttributeModel::rowCount( const QModelIndex &index ) const
{
  QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( reinterpret_cast<QObject*>( index.internalPointer() ) );
  if ( vl )
    return 0;

  QgsLayerTreeNode *n = index2node( index );
  if ( QgsLayerTree::isLayer( n ) )
    return 1;

  return QgsLayerTreeModel::rowCount( index );
}

Qt::ItemFlags QgsVectorLayerAndAttributeModel::flags( const QModelIndex &index ) const
{
  QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( reinterpret_cast<QObject*>( index.internalPointer() ) );
  if ( !vl )
    return QgsLayerTreeModel::flags( index );

  return Qt::ItemIsEnabled /* | Qt::ItemIsEditable */;
}


QVariant QgsVectorLayerAndAttributeModel::data( const QModelIndex& index, int role ) const
{
  QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( reinterpret_cast<QObject*>( index.internalPointer() ) );
  if ( !vl )
    return QgsLayerTreeModel::data( index, role );

  int idx = mAttributeIdx.value( vl, -1 );
  if ( role == Qt::EditRole )
    return idx;

  if ( role != Qt::DisplayRole )
    return QVariant();

  if ( vl->pendingFields().exists( idx ) )
    return vl->pendingFields()[ idx ].name();

  return vl->name();
}


bool QgsVectorLayerAndAttributeModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( role != Qt::EditRole )
    return false;

  QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( reinterpret_cast<QObject*>( index.internalPointer() ) );
  if ( !vl )
    return QgsLayerTreeModel::setData( index, value, role );

  mAttributeIdx[ vl ] = value.toInt();

  return true;
}

QList< QPair<QgsVectorLayer *, int> > QgsVectorLayerAndAttributeModel::layers( const QModelIndexList &selectedIndexes ) const
{
  QList< QPair<QgsVectorLayer *, int> > layers;
  QHash< QgsMapLayer *, int > layerIdx;

  foreach ( const QModelIndex &idx, selectedIndexes )
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
  QList< QPair<QgsVectorLayer *, int> > layersInOrder;

  for ( int i = 0; i < inDrawingOrder.size(); i++ )
  {
    int idx = layerIdx.value( inDrawingOrder[i], -1 );
    if ( idx < 0 )
      continue;

    layersInOrder << layers[idx];
  }

  Q_ASSERT( layersInOrder.size() == layers.size() );

  return layersInOrder;
}


QgsDxfExportDialog::QgsDxfExportDialog( QWidget* parent, Qt::WindowFlags f ): QDialog( parent, f )
{
  setupUi( this );

  mLayerTreeGroup = QgsLayerTree::toGroup( QgsProject::instance()->layerTreeRoot()->clone() );
  cleanGroup( mLayerTreeGroup );

  QgsLayerTreeModel *model = new QgsVectorLayerAndAttributeModel( mLayerTreeGroup, this );
  model->setFlags( 0 );
  mTreeView->setModel( model );

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

#if 0
  mFieldSelectorDelegate = new FieldSelectorDelegate( this );
  mTreeView->setItemDelegateForColumn( 0, mFieldSelectorDelegate );
#endif

  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );

  restoreGeometry( s.value( "/Windows/DxfExport/geometry" ).toByteArray() );
}

QgsDxfExportDialog::~QgsDxfExportDialog()
{
  delete mLayerTreeGroup;
#if 0
  delete mFieldSelectorDelegate;
#endif

  QSettings().setValue( "/Windows/DxfExport/geometry", saveGeometry() );
}

void QgsDxfExportDialog::on_mTreeView_clicked( const QModelIndex & current )
{
  QgsDebugCall;
  QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( reinterpret_cast<QObject*>( current.internalPointer() ) );
  if ( !vl )
  {
    mLayerAttributeComboBox->setDisabled( true );
    return;
  }

  mLayerAttributeComboBox->setEnabled( true );

  int idx = mTreeView->model()->data( current, Qt::EditRole ).toInt();

  mLayerAttributeComboBox->setLayer( vl );
  if ( vl->pendingFields().exists( idx ) )
    mLayerAttributeComboBox->setField( vl->pendingFields()[ idx ].name() );
}

void QgsDxfExportDialog::on_mLayerAttributeComboBox_fieldChanged( QString fieldName )
{
  QgsDebugCall;
  mTreeView->model()->setData( mTreeView->currentIndex(), mLayerAttributeComboBox->layer()->fieldNameIndex( fieldName ) );
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
  return model->layers( mTreeView->selectionModel()->selectedIndexes() );
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
