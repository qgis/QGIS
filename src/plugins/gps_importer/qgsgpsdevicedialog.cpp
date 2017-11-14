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
#include "qgsguiutils.h"
#include "qgssettings.h"

#include <QMessageBox>


QgsGPSDeviceDialog::QgsGPSDeviceDialog( std::map < QString,
                                        QgsGPSDevice * > &devices )
  : QDialog( nullptr, QgsGuiUtils::ModalDialogFlags )
  , mDevices( devices )
{
  setupUi( this );
  connect( pbnNewDevice, &QPushButton::clicked, this, &QgsGPSDeviceDialog::pbnNewDevice_clicked );
  connect( pbnDeleteDevice, &QPushButton::clicked, this, &QgsGPSDeviceDialog::pbnDeleteDevice_clicked );
  connect( pbnUpdateDevice, &QPushButton::clicked, this, &QgsGPSDeviceDialog::pbnUpdateDevice_clicked );
  setAttribute( Qt::WA_DeleteOnClose );
  // Manually set the relative size of the two main parts of the
  // device dialog box.

  QObject::connect( lbDeviceList, &QListWidget::currentItemChanged,
                    this, &QgsGPSDeviceDialog::slotSelectionChanged );
  slotUpdateDeviceList();
}


void QgsGPSDeviceDialog::pbnNewDevice_clicked()
{
  std::map<QString, QgsGPSDevice *>::const_iterator iter = mDevices.begin();
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


void QgsGPSDeviceDialog::pbnDeleteDevice_clicked()
{
  if ( QMessageBox::warning( this, tr( "Delete Device" ),
                             tr( "Are you sure that you want to delete this device?" ),
                             QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Ok )
  {

    std::map<QString, QgsGPSDevice *>::iterator iter =
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


void QgsGPSDeviceDialog::pbnUpdateDevice_clicked()
{
  if ( lbDeviceList->count() > 0 )
  {
    std::map<QString, QgsGPSDevice *>::iterator iter =
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

void QgsGPSDeviceDialog::slotUpdateDeviceList( const QString &selection )
{
  QString selected;
  if ( selection.isEmpty() )
  {
    QListWidgetItem *item = lbDeviceList->currentItem();
    selected = ( item ? item->text() : QString() );
  }
  else
  {
    selected = selection;
  }

  // We're going to be changing the selected item, so disable our
  // notificaton of that.
  QObject::disconnect( lbDeviceList, &QListWidget::currentItemChanged,
                       this, &QgsGPSDeviceDialog::slotSelectionChanged );

  lbDeviceList->clear();
  std::map<QString, QgsGPSDevice *>::const_iterator iter;
  for ( iter = mDevices.begin(); iter != mDevices.end(); ++iter )
  {
    QListWidgetItem *item = new QListWidgetItem( iter->first, lbDeviceList );
    if ( iter->first == selected )
    {
      lbDeviceList->setCurrentItem( item );
    }
  }

  if ( !lbDeviceList->currentItem() && lbDeviceList->count() > 0 )
    lbDeviceList->setCurrentRow( 0 );

  // Update the display and reconnect the selection changed signal
  slotSelectionChanged( lbDeviceList->currentItem() );
  QObject::connect( lbDeviceList, &QListWidget::currentItemChanged,
                    this, &QgsGPSDeviceDialog::slotSelectionChanged );
}


void QgsGPSDeviceDialog::slotSelectionChanged( QListWidgetItem *current )
{
  if ( lbDeviceList->count() > 0 )
  {
    QString devName = current->text();
    leDeviceName->setText( devName );
    QgsGPSDevice *device = mDevices[devName];
    leWptDown->setText( device->
                        importCommand( QStringLiteral( "%babel" ), QStringLiteral( "-w" ), QStringLiteral( "%in" ), QStringLiteral( "%out" ) ).join( QStringLiteral( " " ) ) );
    leWptUp->setText( device->
                      exportCommand( QStringLiteral( "%babel" ), QStringLiteral( "-w" ), QStringLiteral( "%in" ), QStringLiteral( "%out" ) ).join( QStringLiteral( " " ) ) );
    leRteDown->setText( device->
                        importCommand( QStringLiteral( "%babel" ), QStringLiteral( "-r" ), QStringLiteral( "%in" ), QStringLiteral( "%out" ) ).join( QStringLiteral( " " ) ) );
    leRteUp->setText( device->
                      exportCommand( QStringLiteral( "%babel" ), QStringLiteral( "-r" ), QStringLiteral( "%in" ), QStringLiteral( "%out" ) ).join( QStringLiteral( " " ) ) );
    leTrkDown->setText( device->
                        importCommand( QStringLiteral( "%babel" ), QStringLiteral( "-t" ), QStringLiteral( "%in" ), QStringLiteral( "%out" ) ).join( QStringLiteral( " " ) ) );
    leTrkUp->setText( device->
                      exportCommand( QStringLiteral( "%babel" ), QStringLiteral( "-t" ), QStringLiteral( "%in" ), QStringLiteral( "%out" ) ).join( QStringLiteral( " " ) ) );
  }
}


void QgsGPSDeviceDialog::writeDeviceSettings()
{
  QStringList deviceNames;
  QgsSettings settings;
  QString devPath = QStringLiteral( "/Plugin-GPS/devices/%1" );
  settings.remove( QStringLiteral( "/Plugin-GPS/devices" ) );

  std::map<QString, QgsGPSDevice *>::const_iterator iter;
  for ( iter = mDevices.begin(); iter != mDevices.end(); ++iter )
  {
    deviceNames.append( iter->first );
    QString wptDownload =
      iter->second->importCommand( QStringLiteral( "%babel" ), QStringLiteral( "-w" ), QStringLiteral( "%in" ), QStringLiteral( "%out" ) ).join( QStringLiteral( " " ) );
    QString wptUpload =
      iter->second->exportCommand( QStringLiteral( "%babel" ), QStringLiteral( "-w" ), QStringLiteral( "%in" ), QStringLiteral( "%out" ) ).join( QStringLiteral( " " ) );
    QString rteDownload =
      iter->second->importCommand( QStringLiteral( "%babel" ), QStringLiteral( "-r" ), QStringLiteral( "%in" ), QStringLiteral( "%out" ) ).join( QStringLiteral( " " ) );
    QString rteUpload =
      iter->second->exportCommand( QStringLiteral( "%babel" ), QStringLiteral( "-r" ), QStringLiteral( "%in" ), QStringLiteral( "%out" ) ).join( QStringLiteral( " " ) );
    QString trkDownload =
      iter->second->importCommand( QStringLiteral( "%babel" ), QStringLiteral( "-t" ), QStringLiteral( "%in" ), QStringLiteral( "%out" ) ).join( QStringLiteral( " " ) );
    QString trkUpload =
      iter->second->exportCommand( QStringLiteral( "%babel" ), QStringLiteral( "-t" ), QStringLiteral( "%in" ), QStringLiteral( "%out" ) ).join( QStringLiteral( " " ) );
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
  settings.setValue( QStringLiteral( "/Plugin-GPS/devicelist" ), deviceNames );
}

void QgsGPSDeviceDialog::on_pbnClose_clicked()
{
  close();
}
