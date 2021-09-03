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
#include "qgsgui.h"

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
  QgsGui::enableAutoGeometryRestore( this );

  QgsEmbeddedLayerTreeModel *model = new QgsEmbeddedLayerTreeModel( mRootGroup, this );
  mTreeView->setModel( model );

  const QgsSettings settings;

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
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, &QgsProjectLayerGroupDialog::showHelp );
}

QgsProjectLayerGroupDialog::QgsProjectLayerGroupDialog( const QgsProject *project, QWidget *parent, Qt::WindowFlags f )
  : QDialog( parent, f )
{

  // Preconditions
  Q_ASSERT( project );
  Q_ASSERT( project->layerTreeRoot() );

  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  mRootGroup = project->layerTreeRoot()->clone();
  QgsEmbeddedLayerTreeModel *model = new QgsEmbeddedLayerTreeModel( mRootGroup, this );
  mTreeView->setModel( model );

  mProjectFileWidget->hide();
  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( true );

  removeEmbeddedNodes( mRootGroup );

  connect( mTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsProjectLayerGroupDialog::onTreeViewSelectionChanged );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsProjectLayerGroupDialog::mButtonBox_accepted );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, &QgsProjectLayerGroupDialog::showHelp );

}

QgsProjectLayerGroupDialog::~QgsProjectLayerGroupDialog()
{
  delete mRootGroup;
}

QStringList QgsProjectLayerGroupDialog::selectedGroups() const
{
  QStringList groups;
  const auto constSelectedIndexes = mTreeView->selectionModel()->selectedIndexes();
  for ( const QModelIndex &index : constSelectedIndexes )
  {
    QgsLayerTreeNode *node = mTreeView->index2node( index );
    if ( QgsLayerTree::isGroup( node ) )
      groups << QgsLayerTree::toGroup( node )->name();
  }
  return groups;
}

QStringList QgsProjectLayerGroupDialog::selectedLayerIds() const
{
  QStringList layerIds;
  const auto constSelectedIndexes = mTreeView->selectionModel()->selectedIndexes();
  for ( const QModelIndex &index : constSelectedIndexes )
  {
    QgsLayerTreeNode *node = mTreeView->index2node( index );
    if ( QgsLayerTree::isLayer( node ) )
      layerIds << QgsLayerTree::toLayer( node )->layerId();
  }
  return layerIds;
}

QStringList QgsProjectLayerGroupDialog::selectedLayerNames() const
{
  QStringList layerNames;
  const auto constSelectedIndexes = mTreeView->selectionModel()->selectedIndexes();
  for ( const QModelIndex &index : constSelectedIndexes )
  {
    QgsLayerTreeNode *node = mTreeView->index2node( index );
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
  return static_cast< bool >( mTreeView->layerTreeModel() );
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

    archive = std::make_unique<QgsProjectArchive>();

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
    // Use a temporary tree to read the nodes to prevent signals being delivered to the models
    QgsLayerTree tempTree;
    tempTree.readChildrenFromXml( layerTreeElem,  QgsReadWriteContext() );
    mRootGroup->insertChildNodes( -1, tempTree.abandonChildren() );
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
  const auto constChildren = node->children();
  for ( QgsLayerTreeNode *child : constChildren )
  {
    if ( child->customProperty( QStringLiteral( "embedded" ) ).toInt() )
      childrenToRemove << child;
    else if ( QgsLayerTree::isGroup( child ) )
      removeEmbeddedNodes( QgsLayerTree::toGroup( child ) );
  }
  const auto constChildrenToRemove = childrenToRemove;
  for ( QgsLayerTreeNode *childToRemove : constChildrenToRemove )
    node->removeChildNode( childToRemove );
}


void QgsProjectLayerGroupDialog::onTreeViewSelectionChanged()
{
  const auto constSelectedIndexes = mTreeView->selectionModel()->selectedIndexes();
  for ( const QModelIndex &index : constSelectedIndexes )
  {
    deselectChildren( index );
  }

  if ( !mPresetProjectMode )
    mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( !mTreeView->selectionModel()->selectedIndexes().empty() );
}


void QgsProjectLayerGroupDialog::deselectChildren( const QModelIndex &index )
{
  const int childCount = mTreeView->model()->rowCount( index );
  for ( int i = 0; i < childCount; ++i )
  {
    const QModelIndex childIndex = mTreeView->model()->index( i, 0, index );
    if ( mTreeView->selectionModel()->isSelected( childIndex ) )
      mTreeView->selectionModel()->select( childIndex, QItemSelectionModel::Deselect );

    deselectChildren( childIndex );
  }
}

void QgsProjectLayerGroupDialog::mButtonBox_accepted()
{
  QgsSettings s;
  const QFileInfo fi( mProjectPath );
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
