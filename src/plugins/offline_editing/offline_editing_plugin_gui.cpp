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

#include "qgshelp.h"
#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgsmaplayer.h"
#include "qgsproject.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgssettings.h"
#include "qgsapplication.h"
#include "qgsgui.h"

#include <QFileDialog>
#include <QMessageBox>

QgsSelectLayerTreeModel::QgsSelectLayerTreeModel( QgsLayerTree *rootNode, QObject *parent )
  : QgsLayerTreeModel( rootNode, parent )
{
  setFlag( QgsLayerTreeModel::ShowLegend, false );
  setFlag( QgsLayerTreeModel::AllowNodeChangeVisibility, true );
}

int QgsSelectLayerTreeModel::columnCount( const QModelIndex &parent ) const
{
  return QgsLayerTreeModel::columnCount( parent ) + 1;
}


QVariant QgsSelectLayerTreeModel::data( const QModelIndex &index, int role ) const
{
  QgsLayerTreeNode *node = index2node( index );
  if ( index.column() == 0 )
  {
    if ( role == Qt::CheckStateRole )
    {
      if ( QgsLayerTree::isLayer( node ) )
      {
        QgsLayerTreeLayer *nodeLayer = QgsLayerTree::toLayer( node );
        return nodeLayer->isVisible() ? Qt::Checked : Qt::Unchecked;
      }
      else if ( QgsLayerTree::isGroup( node ) )
      {
        QgsLayerTreeGroup *nodeGroup = QgsLayerTree::toGroup( node );
        return nodeGroup->isVisible() ? Qt::Checked : Qt::Unchecked;
      }
      else
      {
        return QVariant();
      }
    }
  }
  else
  {
    if ( QgsLayerTree::isLayer( node ) && index.column() > 0 )
    {
      QgsLayerTreeLayer *nodeLayer = QgsLayerTree::toLayer( node );
      if ( nodeLayer->layer()->providerType() == QLatin1String( "WFS" ) )
      {
        switch ( role )
        {
          case Qt::ToolTipRole:
            return tr( "The source of this layer is a <b>WFS</b> server.<br>"
                       "Some WFS layers are not suitable for offline<br>"
                       "editing due to unstable primary keys<br>"
                       "please check with your system administrator<br>"
                       "if this WFS layer can be used for offline<br>"
                       "editing." );

          case Qt::DecorationRole:
            return QgsApplication::getThemeIcon( QStringLiteral( "/mIconWarning.svg" ) );
        }
      }
    }
    return QVariant();
  }
  return QgsLayerTreeModel::data( index, role );
}


QgsOfflineEditingPluginGui::QgsOfflineEditingPluginGui( QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  connect( mBrowseButton, &QPushButton::clicked, this, &QgsOfflineEditingPluginGui::mBrowseButton_clicked );
  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsOfflineEditingPluginGui::buttonBox_accepted );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QgsOfflineEditingPluginGui::buttonBox_rejected );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsOfflineEditingPluginGui::showHelp );

  restoreState();

  mOfflineDbFile = QStringLiteral( "offline.gpkg" );
  mOfflineDataPathLineEdit->setText( QDir( mOfflineDataPath ).absoluteFilePath( mOfflineDbFile ) );

  QgsLayerTree *rootNode = QgsProject::instance()->layerTreeRoot()->clone();
  QgsLayerTreeModel *treeModel = new QgsSelectLayerTreeModel( rootNode, this );
  mLayerTree->setModel( treeModel );
  mLayerTree->header()->setSectionResizeMode( QHeaderView::ResizeToContents );

  connect( mSelectAllButton, &QAbstractButton::clicked, this, &QgsOfflineEditingPluginGui::selectAll );
  connect( mDeselectAllButton, &QAbstractButton::clicked, this, &QgsOfflineEditingPluginGui::deSelectAll );
  connect( mSelectDatatypeCombo, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsOfflineEditingPluginGui::datatypeChanged );
}

QgsOfflineEditingPluginGui::~QgsOfflineEditingPluginGui()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "OfflineEditing/offline_data_path" ), mOfflineDataPath, QgsSettings::Section::Plugins );
}

QString QgsOfflineEditingPluginGui::offlineDataPath()
{
  return mOfflineDataPath;
}

QString QgsOfflineEditingPluginGui::offlineDbFile()
{
  return mOfflineDbFile;
}

QStringList QgsOfflineEditingPluginGui::selectedLayerIds()
{
  return mSelectedLayerIds;
}

bool QgsOfflineEditingPluginGui::onlySelected() const
{
  return mOnlySelectedCheckBox->checkState() == Qt::Checked;
}

QgsOfflineEditing::ContainerType QgsOfflineEditingPluginGui::dbContainerType() const
{
  if ( mSelectDatatypeCombo->currentIndex() == 0 )
    return QgsOfflineEditing::GPKG;
  else
    return QgsOfflineEditing::SpatiaLite;
}

void QgsOfflineEditingPluginGui::mBrowseButton_clicked()
{
  switch ( dbContainerType() )
  {
    case QgsOfflineEditing::GPKG:
    {
      //GeoPackage
      QString fileName = QFileDialog::getSaveFileName( this,
                         tr( "Select target database for offline data" ),
                         QDir( mOfflineDataPath ).absoluteFilePath( mOfflineDbFile ),
                         tr( "GeoPackage" ) + " (*.gpkg);;"
                         + tr( "All files" ) + " (*.*)" );

      if ( !fileName.isEmpty() )
      {
        if ( !fileName.endsWith( QLatin1String( ".gpkg" ), Qt::CaseInsensitive ) )
        {
          fileName += QLatin1String( ".gpkg" );
        }
        mOfflineDbFile = QFileInfo( fileName ).fileName();
        mOfflineDataPath = QFileInfo( fileName ).absolutePath();
        mOfflineDataPathLineEdit->setText( fileName );
      }
      break;
    }

    case QgsOfflineEditing::SpatiaLite:
    {
      //SpaciaLite
      QString fileName = QFileDialog::getSaveFileName( this,
                         tr( "Select target database for offline data" ),
                         QDir( mOfflineDataPath ).absoluteFilePath( mOfflineDbFile ),
                         tr( "SpatiaLite DB" ) + " (*.sqlite);;"
                         + tr( "All files" ) + " (*.*)" );

      if ( !fileName.isEmpty() )
      {
        if ( !fileName.endsWith( QLatin1String( ".sqlite" ), Qt::CaseInsensitive ) )
        {
          fileName += QLatin1String( ".sqlite" );
        }
        mOfflineDbFile = QFileInfo( fileName ).fileName();
        mOfflineDataPath = QFileInfo( fileName ).absolutePath();
        mOfflineDataPathLineEdit->setText( fileName );
      }
      break;
    }
  }
}

void QgsOfflineEditingPluginGui::buttonBox_accepted()
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

  const QList<QgsLayerTreeLayer *> layers = mLayerTree->layerTreeModel()->rootGroup()->findLayers();
  for ( QgsLayerTreeLayer *nodeLayer : layers )
  {
    if ( nodeLayer->isVisible() )
    {
      mSelectedLayerIds.append( nodeLayer->layerId() );
    }
  }

  accept();
}

void QgsOfflineEditingPluginGui::buttonBox_rejected()
{
  reject();
}

void QgsOfflineEditingPluginGui::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "plugins/core_plugins/plugins_offline_editing.html" ) );
}

void QgsOfflineEditingPluginGui::restoreState()
{
  const QgsSettings settings;
  mOfflineDataPath = settings.value( QStringLiteral( "OfflineEditing/offline_data_path" ), QDir::homePath(), QgsSettings::Section::Plugins ).toString();
}

void QgsOfflineEditingPluginGui::selectAll()
{
  const QList<QgsLayerTreeLayer *> layers = mLayerTree->layerTreeModel()->rootGroup()->findLayers();
  for ( QgsLayerTreeLayer *nodeLayer : layers )
    nodeLayer->setItemVisibilityCheckedParentRecursive( true );
}

void QgsOfflineEditingPluginGui::deSelectAll()
{
  const QList<QgsLayerTreeLayer *> layers = mLayerTree->layerTreeModel()->rootGroup()->findLayers();
  for ( QgsLayerTreeLayer *nodeLayer : layers )
    nodeLayer->setItemVisibilityCheckedParentRecursive( false );
}

void QgsOfflineEditingPluginGui::datatypeChanged( int index )
{
  if ( index == 0 )
  {
    //GeoPackage
    mOfflineDbFile = QStringLiteral( "offline.gpkg" );
  }
  else
  {
    //SpatiaLite
    mOfflineDbFile = QStringLiteral( "offline.sqlite" );
  }
  mOfflineDataPathLineEdit->setText( QDir( mOfflineDataPath ).absoluteFilePath( mOfflineDbFile ) );
}

