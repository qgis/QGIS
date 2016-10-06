/***************************************************************************
 *   Copyright (C) 2004 by Lars Luthman
 *   larsl@users.sourceforge.net
 *                                                                         *
 *   This is a plugin generated from the QGIS plugin template              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "qgsgpsdevicedialog.h"
#include "qgisgui.h"

#include <QMessageBox>
#include <QSettings>


QgsGPSDeviceDialog::QgsGPSDeviceDialog( std::map < QString,
                                        QgsGPSDevice* > & devices )
    : QDialog( nullptr, QgisGui::ModalDialogFlags )
    , mDevices( devices )
{
  setupUi( this );
  setAttribute( Qt::WA_DeleteOnClose );
  // Manually set the relative size of the two main parts of the
  // device dialog box.

  QObject::connect( lbDeviceList, SIGNAL( currentItemChanged( QListWidgetItem*, QListWidgetItem* ) ),
                    this, SLOT( slotSelectionChanged( QListWidgetItem* ) ) );
  slotUpdateDeviceList();
}


void QgsGPSDeviceDialog::on_pbnNewDevice_clicked()
{
  std::map<QString, QgsGPSDevice*>::const_iterator iter = mDevices.begin();
  QString deviceName = tr( "New device %1" );
  int i = 1;
  for ( ; iter != mDevices.end(); ++i )
    iter = mDevices.find( deviceName.arg( i ) );
  deviceName = deviceName.arg( i - 1 );
  mDevices[deviceName] = new QgsGPSDevice;
  writeDeviceSettings();
  slotUpdateDeviceList( deviceName );
  emit devicesChanged();
}


void QgsGPSDeviceDialog::on_pbnDeleteDevice_clicked()
{
  if ( QMessageBox::warning( this, tr( "Are you sure?" ),
                             tr( "Are you sure that you want to delete this device?" ),
                             QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Ok )
  {

    std::map<QString, QgsGPSDevice*>::iterator iter =
      mDevices.find( lbDeviceList->currentItem()->text() );
    if ( iter != mDevices.end() )
    {
      delete iter->second;
      mDevices.erase( iter );
      writeDeviceSettings();
      slotUpdateDeviceList();
      emit devicesChanged();
    }
  }
}


void QgsGPSDeviceDialog::on_pbnUpdateDevice_clicked()
{
  if ( lbDeviceList->count() > 0 )
  {
    std::map<QString, QgsGPSDevice*>::iterator iter =
      mDevices.find( lbDeviceList->currentItem()->text() );
    if ( iter != mDevices.end() )
    {
      delete iter->second;
      mDevices.erase( iter );
      mDevices[leDeviceName->text()] =
        new QgsGPSDevice( leWptDown->text(), leWptUp->text(),
                          leRteDown->text(), leRteUp->text(),
                          leTrkDown->text(), leTrkUp->text() );
      writeDeviceSettings();
      slotUpdateDeviceList( leDeviceName->text() );
      emit devicesChanged();
    }
  }
}

void QgsGPSDeviceDialog::slotUpdateDeviceList( const QString& selection )
{
  QString selected;
  if ( selection == "" )
  {
    QListWidgetItem* item = lbDeviceList->currentItem();
    selected = ( item ? item->text() : "" );
  }
  else
  {
    selected = selection;
  }

  // We're going to be changing the selected item, so disable our
  // notificaton of that.
  QObject::disconnect( lbDeviceList, SIGNAL( currentItemChanged( QListWidgetItem*, QListWidgetItem* ) ),
                       this, SLOT( slotSelectionChanged( QListWidgetItem* ) ) );

  lbDeviceList->clear();
  std::map<QString, QgsGPSDevice*>::const_iterator iter;
  for ( iter = mDevices.begin(); iter != mDevices.end(); ++iter )
  {
    QListWidgetItem* item = new QListWidgetItem( iter->first, lbDeviceList );
    if ( iter->first == selected )
    {
      lbDeviceList->setCurrentItem( item );
    }
  }

  if ( !lbDeviceList->currentItem() && lbDeviceList->count() > 0 )
    lbDeviceList->setCurrentRow( 0 );

  // Update the display and reconnect the selection changed signal
  slotSelectionChanged( lbDeviceList->currentItem() );
  QObject::connect( lbDeviceList, SIGNAL( currentItemChanged( QListWidgetItem*, QListWidgetItem* ) ),
                    this, SLOT( slotSelectionChanged( QListWidgetItem* ) ) );
}


void QgsGPSDeviceDialog::slotSelectionChanged( QListWidgetItem *current )
{
  if ( lbDeviceList->count() > 0 )
  {
    QString devName = current->text();
    leDeviceName->setText( devName );
    QgsGPSDevice* device = mDevices[devName];
    leWptDown->setText( device->
                        importCommand( "%babel", "-w", "%in", "%out" ).join( " " ) );
    leWptUp->setText( device->
                      exportCommand( "%babel", "-w", "%in", "%out" ).join( " " ) );
    leRteDown->setText( device->
                        importCommand( "%babel", "-r", "%in", "%out" ).join( " " ) );
    leRteUp->setText( device->
                      exportCommand( "%babel", "-r", "%in", "%out" ).join( " " ) );
    leTrkDown->setText( device->
                        importCommand( "%babel", "-t", "%in", "%out" ).join( " " ) );
    leTrkUp->setText( device->
                      exportCommand( "%babel", "-t", "%in", "%out" ).join( " " ) );
  }
}


void QgsGPSDeviceDialog::writeDeviceSettings()
{
  QStringList deviceNames;
  QSettings settings;
  QString devPath = "/Plugin-GPS/devices/%1";
  settings.remove( "/Plugin-GPS/devices" );

  std::map<QString, QgsGPSDevice*>::const_iterator iter;
  for ( iter = mDevices.begin(); iter != mDevices.end(); ++iter )
  {
    deviceNames.append( iter->first );
    QString wptDownload =
      iter->second->importCommand( "%babel", "-w", "%in", "%out" ).join( " " );
    QString wptUpload =
      iter->second->exportCommand( "%babel", "-w", "%in", "%out" ).join( " " );
    QString rteDownload =
      iter->second->importCommand( "%babel", "-r", "%in", "%out" ).join( " " );
    QString rteUpload =
      iter->second->exportCommand( "%babel", "-r", "%in", "%out" ).join( " " );
    QString trkDownload =
      iter->second->importCommand( "%babel", "-t", "%in", "%out" ).join( " " );
    QString trkUpload =
      iter->second->exportCommand( "%babel", "-t", "%in", "%out" ).join( " " );
    settings.setValue( devPath.arg( iter->first ) + "/wptdownload",
                       wptDownload );
    settings.setValue( devPath.arg( iter->first ) + "/wptupload", wptUpload );
    settings.setValue( devPath.arg( iter->first ) + "/rtedownload",
                       rteDownload );
    settings.setValue( devPath.arg( iter->first ) + "/rteupload", rteUpload );
    settings.setValue( devPath.arg( iter->first ) + "/trkdownload",
                       trkDownload );
    settings.setValue( devPath.arg( iter->first ) + "/trkupload", trkUpload );
  }
  settings.setValue( "/Plugin-GPS/devicelist", deviceNames );
}

void QgsGPSDeviceDialog::on_pbnClose_clicked()
{
  close();
}
