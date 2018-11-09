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
#include "qgsziputils.h"

#include <QDomDocument>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QPushButton>

QgsEmbeddedLayerTreeModel::QgsEmbeddedLayerTreeModel( QgsLayerTree *rootNode, QObject *parent )
  : QgsLayerTreeModel( rootNode, parent )
{
}

QVariant QgsEmbeddedLayerTreeModel::data( const QModelIndex &index, int role ) const
{
  if ( role == Qt::ForegroundRole || role == Qt::FontRole )
    return QVariant();

  return QgsLayerTreeModel::data( index, role );
}



QgsProjectLayerGroupDialog::QgsProjectLayerGroupDialog( QWidget *parent, const QString &projectFile, Qt::WindowFlags f )
  : QDialog( parent, f )
  , mRootGroup( new QgsLayerTree )
{
  setupUi( this );

  QgsEmbeddedLayerTreeModel *model = new QgsEmbeddedLayerTreeModel( mRootGroup, this );
  mTreeView->setModel( model );

  QgsSettings settings;

  mProjectFileWidget->setStorageMode( QgsFileWidget::GetFile );
  mProjectFileWidget->setFilter( tr( "QGIS files" ) + QStringLiteral( " (*.qgs *.QGS *.qgz *.QGZ)" ) );
  mProjectFileWidget->setDialogTitle( tr( "Select Project File" ) );
  mProjectFileWidget->setDefaultRoot( settings.value( QStringLiteral( "/qgis/last_embedded_project_path" ), QDir::homePath() ).toString() );
  if ( !projectFile.isEmpty() )
  {
    mProjectFileWidget->setFilePath( projectFile );
    mProjectFileLabel->hide();
    mProjectFileWidget->hide();
    mShowEmbeddedContent = true;
    mPresetProjectMode = true;
    changeProjectFile();
    mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( true );
  }
  else
  {
    mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
  }

  connect( mProjectFileWidget, &QgsFileWidget::fileChanged, this, &QgsProjectLayerGroupDialog::changeProjectFile );

  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsProjectLayerGroupDialog::mButtonBox_accepted );

  restoreGeometry( settings.value( QStringLiteral( "Windows/EmbedLayer/geometry" ) ).toByteArray() );


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
  return mProjectFileWidget->filePath();
}

bool QgsProjectLayerGroupDialog::isValid() const
{
  return nullptr != mTreeView->layerTreeModel();
}

void QgsProjectLayerGroupDialog::changeProjectFile()
{
  QFile projectFile( mProjectFileWidget->filePath() );
  if ( !projectFile.exists() )
  {
    return;
  }

  if ( mProjectPath == mProjectFileWidget->filePath() )
  {
    //already up to date
    return;
  }

  //check we are not embedding from/to the same project
  if ( mProjectFileWidget->isVisible() && mProjectFileWidget->filePath() == QgsProject::instance()->fileName() )
  {
    QMessageBox::critical( nullptr, tr( "Embed Layers and Groups" ), tr( "Recursive embedding is not supported. It is not possible to embed layers / groups from the current project." ) );
    return;
  }

  //parse project file and fill tree
  if ( !projectFile.open( QIODevice::ReadOnly ) )
  {
    return;
  }

  std::unique_ptr<QgsProjectArchive> archive;

  QDomDocument projectDom;
  if ( QgsZipUtils::isZipFile( mProjectFileWidget->filePath() ) )
  {

    archive = qgis::make_unique<QgsProjectArchive>();

    // unzip the archive
    if ( !archive->unzip( mProjectFileWidget->filePath() ) )
    {
      return;
    }

    // test if zip provides a .qgs file
    if ( archive->projectFile().isEmpty() )
    {
      return;
    }

    projectFile.setFileName( archive->projectFile() );
    if ( !projectFile.exists() )
    {
      return;
    }
  }
  QString errorMessage;
  int errorLine;
  if ( !projectDom.setContent( &projectFile, &errorMessage, &errorLine ) )
  {
    QgsDebugMsg( QStringLiteral( "Error reading the project file %1 at line %2: %3" )
                 .arg( projectFile.fileName() )
                 .arg( errorLine )
                 .arg( errorMessage ) );
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

  connect( mTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsProjectLayerGroupDialog::onTreeViewSelectionChanged );

  mProjectPath = mProjectFileWidget->filePath();
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

  if ( !mPresetProjectMode )
    mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( !mTreeView->selectionModel()->selectedIndexes().empty() );
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
