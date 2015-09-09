/***************************************************************************
    qgsembedlayerdialog.cpp
    ---------------------
    begin                : June 2011
    copyright            : (C) 2011 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsprojectlayergroupdialog.h"
#include "qgsproject.h"
#include "qgisapp.h"
#include "qgsapplication.h"

#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslayertreeutils.h"

#include <QDomDocument>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>

QgsProjectLayerGroupDialog::QgsProjectLayerGroupDialog( QWidget * parent, const QString& projectFile, Qt::WindowFlags f )
    : QDialog( parent, f )
    , mShowEmbeddedContent( false )
    , mRootGroup( new QgsLayerTreeGroup )
{
  setupUi( this );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/EmbedLayer/geometry" ).toByteArray() );

  if ( !projectFile.isEmpty() )
  {
    mProjectFileLineEdit->setText( projectFile );
    mProjectFileLabel->hide();
    mProjectFileLineEdit->hide();
    mBrowseFileToolButton->hide();
    mShowEmbeddedContent = true;
    changeProjectFile();
  }

  QObject::connect( mButtonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );
}

QgsProjectLayerGroupDialog::~QgsProjectLayerGroupDialog()
{
  QSettings settings;
  settings.setValue( "/Windows/EmbedLayer/geometry", saveGeometry() );

  delete mRootGroup;
}

QStringList QgsProjectLayerGroupDialog::selectedGroups() const
{
  QStringList groups;
  QgsLayerTreeModel* model = mTreeView->layerTreeModel();
  Q_FOREACH ( const QModelIndex& index, mTreeView->selectionModel()->selectedIndexes() )
  {
    QgsLayerTreeNode* node = model->index2node( index );
    if ( QgsLayerTree::isGroup( node ) )
      groups << QgsLayerTree::toGroup( node )->name();
  }
  return groups;
}

QStringList QgsProjectLayerGroupDialog::selectedLayerIds() const
{
  QStringList layerIds;
  QgsLayerTreeModel* model = mTreeView->layerTreeModel();
  Q_FOREACH ( const QModelIndex& index, mTreeView->selectionModel()->selectedIndexes() )
  {
    QgsLayerTreeNode* node = model->index2node( index );
    if ( QgsLayerTree::isLayer( node ) )
      layerIds << QgsLayerTree::toLayer( node )->layerId();
  }
  return layerIds;
}

QStringList QgsProjectLayerGroupDialog::selectedLayerNames() const
{
  QStringList layerNames;
  QgsLayerTreeModel* model = mTreeView->layerTreeModel();
  Q_FOREACH ( const QModelIndex& index, mTreeView->selectionModel()->selectedIndexes() )
  {
    QgsLayerTreeNode* node = model->index2node( index );
    if ( QgsLayerTree::isLayer( node ) )
      layerNames << QgsLayerTree::toLayer( node )->layerName();
  }
  return layerNames;
}

QString QgsProjectLayerGroupDialog::selectedProjectFile() const
{
  return mProjectFileLineEdit->text();
}

bool QgsProjectLayerGroupDialog::isValid() const
{
  return mTreeView->layerTreeModel() != 0;
}

void QgsProjectLayerGroupDialog::on_mBrowseFileToolButton_clicked()
{
  //line edit might emit editingFinished signal when loosing focus
  mProjectFileLineEdit->blockSignals( true );

  QSettings s;
  QString projectFile = QFileDialog::getOpenFileName( this,
                        tr( "Select project file" ),
                        s.value( "/qgis/last_embedded_project_path" ).toString(),
                        tr( "QGIS files" ) + " (*.qgs *.QGS)" );
  if ( !projectFile.isEmpty() )
  {
    mProjectFileLineEdit->setText( projectFile );
  }
  changeProjectFile();
  mProjectFileLineEdit->blockSignals( false );
}

void QgsProjectLayerGroupDialog::on_mProjectFileLineEdit_editingFinished()
{
  changeProjectFile();
}

void QgsProjectLayerGroupDialog::changeProjectFile()
{
  QFile projectFile( mProjectFileLineEdit->text() );
  if ( !projectFile.exists() )
  {
    return;
  }

  if ( mProjectPath == mProjectFileLineEdit->text() )
  {
    //already up to date
    return;
  }

  //check we are not embedding from/to the same project
  if ( mProjectFileLineEdit->isVisible() && mProjectFileLineEdit->text() == QgsProject::instance()->fileName() )
  {
    QMessageBox::critical( 0, tr( "Recursive embedding not possible" ), tr( "It is not possible to embed layers / groups from the current project." ) );
    return;
  }

  //parse project file and fill tree
  if ( !projectFile.open( QIODevice::ReadOnly ) )
  {
    return;
  }

  QDomDocument projectDom;
  if ( !projectDom.setContent( &projectFile ) )
  {
    return;
  }

  mRootGroup->removeAllChildren();

  QDomElement layerTreeElem = projectDom.documentElement().firstChildElement( "layer-tree-group" );
  if ( !layerTreeElem.isNull() )
  {
    mRootGroup->readChildrenFromXML( layerTreeElem );
  }
  else
  {
    QgsLayerTreeUtils::readOldLegend( mRootGroup, projectDom.documentElement().firstChildElement( "legend" ) );
  }

  if ( !mShowEmbeddedContent )
    removeEmbeddedNodes( mRootGroup );

  QgsLayerTreeModel* model = new QgsLayerTreeModel( mRootGroup, this );
  mTreeView->setModel( model );

  QObject::connect( mTreeView->selectionModel(), SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ), this, SLOT( onTreeViewSelectionChanged() ) );

  mProjectPath = mProjectFileLineEdit->text();
}


void QgsProjectLayerGroupDialog::removeEmbeddedNodes( QgsLayerTreeGroup* node )
{
  QList<QgsLayerTreeNode*> childrenToRemove;
  Q_FOREACH ( QgsLayerTreeNode* child, node->children() )
  {
    if ( child->customProperty( "embedded" ).toInt() )
      childrenToRemove << child;
    else if ( QgsLayerTree::isGroup( child ) )
      removeEmbeddedNodes( QgsLayerTree::toGroup( child ) );
  }
  Q_FOREACH ( QgsLayerTreeNode* childToRemove, childrenToRemove )
    node->removeChildNode( childToRemove );
}


void QgsProjectLayerGroupDialog::onTreeViewSelectionChanged()
{
  Q_FOREACH ( const QModelIndex& index, mTreeView->selectionModel()->selectedIndexes() )
  {
    unselectChildren( index );
  }
}


void QgsProjectLayerGroupDialog::unselectChildren( const QModelIndex& index )
{
  int childCount = mTreeView->model()->rowCount( index );
  for ( int i = 0; i < childCount; ++i )
  {
    QModelIndex childIndex = mTreeView->model()->index( i, 0, index );
    if ( mTreeView->selectionModel()->isSelected( childIndex ) )
      mTreeView->selectionModel()->select( childIndex, QItemSelectionModel::Deselect );

    unselectChildren( childIndex );
  }
}

void QgsProjectLayerGroupDialog::on_mButtonBox_accepted()
{
  QSettings s;
  QFileInfo fi( mProjectPath );
  if ( fi.exists() )
  {
    s.setValue( "/qgis/last_embedded_project_path", fi.absolutePath() );
  }
  accept();
}
