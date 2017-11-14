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
#include "qgssettings.h"

#include <QDomDocument>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>

QgsProjectLayerGroupDialog::QgsProjectLayerGroupDialog( QWidget *parent, const QString &projectFile, Qt::WindowFlags f )
  : QDialog( parent, f )
  , mRootGroup( new QgsLayerTree )
{
  setupUi( this );
  connect( mBrowseFileToolButton, &QToolButton::clicked, this, &QgsProjectLayerGroupDialog::mBrowseFileToolButton_clicked );
  connect( mProjectFileLineEdit, &QLineEdit::editingFinished, this, &QgsProjectLayerGroupDialog::mProjectFileLineEdit_editingFinished );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsProjectLayerGroupDialog::mButtonBox_accepted );

  QgsSettings settings;
  restoreGeometry( settings.value( QStringLiteral( "Windows/EmbedLayer/geometry" ) ).toByteArray() );

  if ( !projectFile.isEmpty() )
  {
    mProjectFileLineEdit->setText( projectFile );
    mProjectFileLabel->hide();
    mProjectFileLineEdit->hide();
    mBrowseFileToolButton->hide();
    mShowEmbeddedContent = true;
    changeProjectFile();
  }

  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, &QgsProjectLayerGroupDialog::showHelp );
}

QgsProjectLayerGroupDialog::~QgsProjectLayerGroupDialog()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/EmbedLayer/geometry" ), saveGeometry() );

  delete mRootGroup;
}

QStringList QgsProjectLayerGroupDialog::selectedGroups() const
{
  QStringList groups;
  QgsLayerTreeModel *model = mTreeView->layerTreeModel();
  Q_FOREACH ( const QModelIndex &index, mTreeView->selectionModel()->selectedIndexes() )
  {
    QgsLayerTreeNode *node = model->index2node( index );
    if ( QgsLayerTree::isGroup( node ) )
      groups << QgsLayerTree::toGroup( node )->name();
  }
  return groups;
}

QStringList QgsProjectLayerGroupDialog::selectedLayerIds() const
{
  QStringList layerIds;
  QgsLayerTreeModel *model = mTreeView->layerTreeModel();
  Q_FOREACH ( const QModelIndex &index, mTreeView->selectionModel()->selectedIndexes() )
  {
    QgsLayerTreeNode *node = model->index2node( index );
    if ( QgsLayerTree::isLayer( node ) )
      layerIds << QgsLayerTree::toLayer( node )->layerId();
  }
  return layerIds;
}

QStringList QgsProjectLayerGroupDialog::selectedLayerNames() const
{
  QStringList layerNames;
  QgsLayerTreeModel *model = mTreeView->layerTreeModel();
  Q_FOREACH ( const QModelIndex &index, mTreeView->selectionModel()->selectedIndexes() )
  {
    QgsLayerTreeNode *node = model->index2node( index );
    if ( QgsLayerTree::isLayer( node ) )
      layerNames << QgsLayerTree::toLayer( node )->name();
  }
  return layerNames;
}

QString QgsProjectLayerGroupDialog::selectedProjectFile() const
{
  return mProjectFileLineEdit->text();
}

bool QgsProjectLayerGroupDialog::isValid() const
{
  return nullptr != mTreeView->layerTreeModel();
}

void QgsProjectLayerGroupDialog::mBrowseFileToolButton_clicked()
{
  //line edit might emit editingFinished signal when losing focus
  mProjectFileLineEdit->blockSignals( true );

  QgsSettings s;
  QString projectFile = QFileDialog::getOpenFileName( this,
                        tr( "Select project file" ),
                        s.value( QStringLiteral( "/qgis/last_embedded_project_path" ), QDir::homePath() ).toString(),
                        tr( "QGIS files" ) + " (*.qgs *.QGS)" );
  if ( !projectFile.isEmpty() )
  {
    mProjectFileLineEdit->setText( projectFile );
  }
  changeProjectFile();
  mProjectFileLineEdit->blockSignals( false );
}

void QgsProjectLayerGroupDialog::mProjectFileLineEdit_editingFinished()
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
    QMessageBox::critical( nullptr, tr( "Recursive embedding not possible" ), tr( "It is not possible to embed layers / groups from the current project." ) );
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

  QDomElement layerTreeElem = projectDom.documentElement().firstChildElement( QStringLiteral( "layer-tree-group" ) );
  if ( !layerTreeElem.isNull() )
  {
    mRootGroup->readChildrenFromXml( layerTreeElem, QgsReadWriteContext() );
  }
  else
  {
    QgsLayerTreeUtils::readOldLegend( mRootGroup, projectDom.documentElement().firstChildElement( QStringLiteral( "legend" ) ) );
  }

  if ( !mShowEmbeddedContent )
    removeEmbeddedNodes( mRootGroup );

  QgsLayerTreeModel *model = new QgsLayerTreeModel( mRootGroup, this );
  mTreeView->setModel( model );

  connect( mTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsProjectLayerGroupDialog::onTreeViewSelectionChanged );

  mProjectPath = mProjectFileLineEdit->text();
}


void QgsProjectLayerGroupDialog::removeEmbeddedNodes( QgsLayerTreeGroup *node )
{
  QList<QgsLayerTreeNode *> childrenToRemove;
  Q_FOREACH ( QgsLayerTreeNode *child, node->children() )
  {
    if ( child->customProperty( QStringLiteral( "embedded" ) ).toInt() )
      childrenToRemove << child;
    else if ( QgsLayerTree::isGroup( child ) )
      removeEmbeddedNodes( QgsLayerTree::toGroup( child ) );
  }
  Q_FOREACH ( QgsLayerTreeNode *childToRemove, childrenToRemove )
    node->removeChildNode( childToRemove );
}


void QgsProjectLayerGroupDialog::onTreeViewSelectionChanged()
{
  Q_FOREACH ( const QModelIndex &index, mTreeView->selectionModel()->selectedIndexes() )
  {
    deselectChildren( index );
  }
}


void QgsProjectLayerGroupDialog::deselectChildren( const QModelIndex &index )
{
  int childCount = mTreeView->model()->rowCount( index );
  for ( int i = 0; i < childCount; ++i )
  {
    QModelIndex childIndex = mTreeView->model()->index( i, 0, index );
    if ( mTreeView->selectionModel()->isSelected( childIndex ) )
      mTreeView->selectionModel()->select( childIndex, QItemSelectionModel::Deselect );

    deselectChildren( childIndex );
  }
}

void QgsProjectLayerGroupDialog::mButtonBox_accepted()
{
  QgsSettings s;
  QFileInfo fi( mProjectPath );
  if ( fi.exists() )
  {
    s.setValue( QStringLiteral( "/qgis/last_embedded_project_path" ), fi.absolutePath() );
  }
  accept();
}

void QgsProjectLayerGroupDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "introduction/general_tools.html#nesting-projects" ) );

}
