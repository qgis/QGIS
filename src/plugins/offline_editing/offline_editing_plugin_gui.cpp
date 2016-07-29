/***************************************************************************
    offline_editing_plugin_gui.cpp

    Offline Editing Plugin
    a QGIS plugin
     --------------------------------------
    Date                 : 08-Jul-2010
    Copyright            : (C) 2010 by Sourcepole
    Email                : info at sourcepole.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "offline_editing_plugin_gui.h"

#include "qgscontexthelp.h"
#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsproject.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>


QgsSelectLayerTreeModel::QgsSelectLayerTreeModel( QgsLayerTreeGroup* rootNode, QObject* parent )
    : QgsLayerTreeModel( rootNode, parent )
{
  setFlag( QgsLayerTreeModel::ShowLegend, false );
  setFlag( QgsLayerTreeModel::AllowNodeChangeVisibility, true );
}

QgsSelectLayerTreeModel::~QgsSelectLayerTreeModel()
{
}

QVariant QgsSelectLayerTreeModel::data( const QModelIndex& index, int role ) const
{
  if ( role == Qt::CheckStateRole )
  {
    QgsLayerTreeNode* node = index2node( index );
    if ( QgsLayerTree::isLayer( node ) )
    {
      QgsLayerTreeLayer* nodeLayer = QgsLayerTree::toLayer( node );
      return nodeLayer->isVisible();
    }
    else if ( QgsLayerTree::isGroup( node ) )
    {
      QgsLayerTreeGroup* nodeGroup = QgsLayerTree::toGroup( node );
      return nodeGroup->isVisible();
    }
    else
    {
      return QVariant();
    }
  }
  return QgsLayerTreeModel::data( index, role );
}


QgsOfflineEditingPluginGui::QgsOfflineEditingPluginGui( QWidget* parent, Qt::WindowFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );

  restoreState();

  mOfflineDbFile = "offline.sqlite";
  mOfflineDataPathLineEdit->setText( QDir( mOfflineDataPath ).absoluteFilePath( mOfflineDbFile ) );

  QgsLayerTreeGroup* rootNode = QgsLayerTree::toGroup( QgsProject::instance()->layerTreeRoot()->clone() );
  QgsLayerTreeModel* treeModel = new QgsSelectLayerTreeModel( rootNode, this );
  mLayerTree->setModel( treeModel );

  connect( mSelectAllButton, SIGNAL( clicked() ), this, SLOT( selectAll() ) );
  connect( mUnselectAllButton, SIGNAL( clicked() ), this, SLOT( unSelectAll() ) );
}

QgsOfflineEditingPluginGui::~QgsOfflineEditingPluginGui()
{
  QSettings settings;
  settings.setValue( "Plugin-OfflineEditing/geometry", saveGeometry() );
  settings.setValue( "Plugin-OfflineEditing/offline_data_path", mOfflineDataPath );
}

QString QgsOfflineEditingPluginGui::offlineDataPath()
{
  return mOfflineDataPath;
}

QString QgsOfflineEditingPluginGui::offlineDbFile()
{
  return mOfflineDbFile;
}

QStringList& QgsOfflineEditingPluginGui::selectedLayerIds()
{
  return mSelectedLayerIds;
}

void QgsOfflineEditingPluginGui::on_mBrowseButton_clicked()
{
  QString fileName = QFileDialog::getSaveFileName( this,
                     tr( "Select target database for offline data" ),
                     QDir( mOfflineDataPath ).absoluteFilePath( mOfflineDbFile ),
                     tr( "SpatiaLite DB" ) + " (*.sqlite);;"
                     + tr( "All files" ) + " (*.*)" );

  if ( !fileName.isEmpty() )
  {
    if ( !fileName.endsWith( ".sqlite", Qt::CaseInsensitive ) )
    {
      fileName += ".sqlite";
    }
    mOfflineDbFile = QFileInfo( fileName ).fileName();
    mOfflineDataPath = QFileInfo( fileName ).absolutePath();
    mOfflineDataPathLineEdit->setText( fileName );
  }
}

void QgsOfflineEditingPluginGui::on_buttonBox_accepted()
{
  if ( QFile( QDir( mOfflineDataPath ).absoluteFilePath( mOfflineDbFile ) ).exists() )
  {
    QMessageBox msgBox;
    msgBox.setWindowTitle( tr( "Offline Editing Plugin" ) );
    msgBox.setText( tr( "Converting to offline project." ) );
    msgBox.setInformativeText( tr( "Offline database file '%1' exists. Overwrite?" ).arg( mOfflineDbFile ) );
    msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::Cancel );
    msgBox.setDefaultButton( QMessageBox::Cancel );
    if ( msgBox.exec() != QMessageBox::Yes )
    {
      return;
    }
  }

  mSelectedLayerIds.clear();
  Q_FOREACH ( QgsLayerTreeLayer* nodeLayer, mLayerTree->layerTreeModel()->rootGroup()->findLayers() )
  {
    if ( nodeLayer->isVisible() )
    {
      QgsDebugMsg( nodeLayer->layerId() );
      mSelectedLayerIds.append( nodeLayer->layerId() );
    }
  }

  accept();
}

void QgsOfflineEditingPluginGui::on_buttonBox_rejected()
{
  reject();
}

// TODO: help
void QgsOfflineEditingPluginGui::on_buttonBox_helpRequested()
{
  QgsContextHelp::run( metaObject()->className() );
}

void QgsOfflineEditingPluginGui::restoreState()
{
  QSettings settings;
  mOfflineDataPath = settings.value( "Plugin-OfflineEditing/offline_data_path", QDir::homePath() ).toString();
  restoreGeometry( settings.value( "Plugin-OfflineEditing/geometry" ).toByteArray() );
}

void QgsOfflineEditingPluginGui::selectAll()
{
  Q_FOREACH ( QgsLayerTreeLayer* nodeLayer, mLayerTree->layerTreeModel()->rootGroup()->findLayers() )
    nodeLayer->setVisible( Qt::Checked );
}


void QgsOfflineEditingPluginGui::unSelectAll()
{
  Q_FOREACH ( QgsLayerTreeLayer* nodeLayer, mLayerTree->layerTreeModel()->rootGroup()->findLayers() )
    nodeLayer->setVisible( Qt::Unchecked );
}
