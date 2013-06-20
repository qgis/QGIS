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
#include <qgsvectordataprovider.h>
#include <qgsvectorlayer.h>

#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

QgsOfflineEditingPluginGui::QgsOfflineEditingPluginGui( QWidget* parent /*= 0*/, Qt::WFlags fl /*= 0*/ )
    : QDialog( parent, fl )
{
  setupUi( this );

  restoreState();

  mOfflineDbFile = "offline.sqlite";
  ui_offlineDataPath->setText( QDir( mOfflineDataPath ).absoluteFilePath( mOfflineDbFile ) );

  updateLayerList( checkboxShowEditableLayers->isChecked() );
}

QgsOfflineEditingPluginGui::~QgsOfflineEditingPluginGui()
{
  QSettings settings;
  settings.setValue( "Plugin-OfflineEditing/geometry", saveGeometry() );
  settings.setValue( "Plugin-OfflineEditing/offline_data_path", mOfflineDataPath );
  settings.setValue( "Plugin-OfflineEditing/onlyEditableLayers", checkboxShowEditableLayers->isChecked() );
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

void QgsOfflineEditingPluginGui::updateLayerList( bool filterEditableLayers )
{
  ui_layerList->clear();

  QMap<QString, QgsMapLayer*> mapLayers = QgsMapLayerRegistry::instance()->mapLayers();
  for ( QMap<QString, QgsMapLayer*>::iterator layer_it = mapLayers.begin() ; layer_it != mapLayers.end(); ++layer_it )
  {
    if ( layer_it.value()->type() == QgsMapLayer::VectorLayer )
    {
      QgsVectorLayer* layer = qobject_cast<QgsVectorLayer*>( layer_it.value() );

      bool showLayer = true;
      if ( filterEditableLayers )
      {
        int cap = layer->dataProvider()->capabilities();
        showLayer = ( cap & QgsVectorDataProvider::AddFeatures ) &&
                    ( cap & QgsVectorDataProvider::DeleteFeatures ) &&
                    ( cap & QgsVectorDataProvider::ChangeAttributeValues ) &&
                    ( cap & QgsVectorDataProvider::AddAttributes ) &&
                    ( cap & QgsVectorDataProvider::ChangeGeometries );
      }
      if ( showLayer )
      {
        QListWidgetItem* item = new QListWidgetItem( layer->name(), ui_layerList );
        item->setData( Qt::UserRole, QVariant( layer_it.key() ) );
      }
    }
  }
}

void QgsOfflineEditingPluginGui::on_butBrowse_clicked()
{
  QString fileName = QFileDialog::getSaveFileName( this,
                     tr( "Select target database for offline data" ),
                     QDir( mOfflineDataPath ).absoluteFilePath( mOfflineDbFile ),
                     tr( "SpatiaLite DB" ) + " (*.sqlite);;"
                     + tr( "All files" ) + " (*.*)" );

  if ( !fileName.isEmpty() )
  {
    if ( !fileName.toLower().endsWith( ".sqlite" ) )
    {
      fileName += ".sqlite";
    }
    mOfflineDbFile = QFileInfo( fileName ).fileName();
    mOfflineDataPath = QFileInfo( fileName ).absolutePath();
    ui_offlineDataPath->setText( fileName );
  }
}

void QgsOfflineEditingPluginGui::on_checkboxShowEditableLayers_stateChanged( int state )
{
  Q_UNUSED( state );
  updateLayerList( checkboxShowEditableLayers->isChecked() );
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
  mOfflineDataPath = settings.value( "Plugin-OfflineEditing/offline_data_path", QDir().absolutePath() ).toString();
  restoreGeometry( settings.value( "Plugin-OfflineEditing/geometry" ).toByteArray() );
  checkboxShowEditableLayers->setChecked( settings.value( "Plugin-OfflineEditing/onlyEditableLayers", true ).toBool() );
}
