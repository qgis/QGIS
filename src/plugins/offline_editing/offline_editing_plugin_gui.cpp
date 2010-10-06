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

#include <qgscontexthelp.h>
#include <qgsmaplayer.h>
#include <qgsmaplayerregistry.h>

#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

#define SETTINGS_OFFLINE_DATA_PATH "Plugin-OfflineEditing/offline_data_path"

QgsOfflineEditingPluginGui::QgsOfflineEditingPluginGui( QWidget* parent /*= 0*/, Qt::WFlags fl /*= 0*/ )
    : QDialog( parent, fl )
{
  setupUi( this );

  QDir dir;
  QSettings settings;
  mOfflineDataPath = settings.value( SETTINGS_OFFLINE_DATA_PATH, dir.absolutePath() ).toString();
  mOfflineDbFile = "offline.sqlite";
  ui_offlineDataPath->setText( QDir( mOfflineDataPath ).absoluteFilePath( mOfflineDbFile ) );

  QMap<QString, QgsMapLayer*> mapLayers = QgsMapLayerRegistry::instance()->mapLayers();
  for ( QMap<QString, QgsMapLayer*>::iterator layer_it = mapLayers.begin() ; layer_it != mapLayers.end(); ++layer_it )
  {
    if ( layer_it.value()->type() == QgsMapLayer::VectorLayer )
    {
      QListWidgetItem* item = new QListWidgetItem( layer_it.value()->name(), ui_layerList );
      item->setData( Qt::UserRole, QVariant( layer_it.key() ) );
    }
  }
}

QgsOfflineEditingPluginGui::~QgsOfflineEditingPluginGui()
{
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

void QgsOfflineEditingPluginGui::on_butBrowse_clicked()
{
  QString fileName = QFileDialog::getSaveFileName( this,
                     tr( "Select target database for offline data" ),
                     QDir( mOfflineDataPath ).absoluteFilePath( mOfflineDbFile ),
                     tr( "SpatiaLite DB(*.sqlite);;All files(*.*)" )
                                                 );

  if ( !fileName.isEmpty() )
  {
    mOfflineDbFile = QFileInfo( fileName ).fileName();
    mOfflineDataPath = QFileInfo( fileName ).absolutePath();
    ui_offlineDataPath->setText( fileName );
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
  QList<QListWidgetItem*> layers = ui_layerList->selectedItems();
  for ( QList<QListWidgetItem*>::const_iterator it = layers.begin(); it != layers.end(); ++it )
  {
    mSelectedLayerIds.append(( *it )->data( Qt::UserRole ).toString() );
  }

  QSettings settings;
  settings.setValue( SETTINGS_OFFLINE_DATA_PATH, mOfflineDataPath );

  accept();
}

void QgsOfflineEditingPluginGui::on_buttonBox_rejected()
{
  reject();
}

// TODO: help
void QgsOfflineEditingPluginGui::on_buttonBox_helpRequested()
{
  QgsContextHelp::run( context_id );
}
