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

#include <iostream>

#include <qlistbox.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qsettings.h>


QgsGPSDeviceDialog::QgsGPSDeviceDialog(BabelMap& devices) : mDevices(devices) {
  slotUpdateDeviceList();
}


void QgsGPSDeviceDialog::slotNewDevice() {
  BabelMap::const_iterator iter = mDevices.begin();
  QString deviceName = "New device %1";
  int i;
  for (i = 1; iter != mDevices.end(); ++i)
    iter = mDevices.find(deviceName.arg(i));
  deviceName = deviceName.arg(i - 1);
  mDevices[deviceName] = new QgsBabelCommand("download command",
					     "upload command");
  writeDeviceSettings();
  slotUpdateDeviceList(deviceName);
  emit devicesChanged();
}


void QgsGPSDeviceDialog::slotDeleteDevice() {
  if (QMessageBox::warning(this, "Are you sure?", 
			   "Are you sure that you want to delete this device?",
			   QMessageBox::Ok, QMessageBox::Cancel) == 
      QMessageBox::Ok) {
    BabelMap::iterator iter = mDevices.find(lbDeviceList->
					    selectedItem()->text());
    delete iter->second;
    mDevices.erase(iter);
    writeDeviceSettings();
    slotUpdateDeviceList();
    emit devicesChanged();
  }
}


void QgsGPSDeviceDialog::slotUpdateDevice() {
  BabelMap::iterator iter = mDevices.find(lbDeviceList->
					  selectedItem()->text());
  delete iter->second;
  mDevices.erase(iter);
  mDevices[leDeviceName->text()] =
    new QgsBabelCommand(leDownloadCmd->text(), leUploadCmd->text());
  writeDeviceSettings();
  slotUpdateDeviceList(leDeviceName->text());
  emit devicesChanged();
}


void QgsGPSDeviceDialog::slotUpdateDeviceList(const QString& selection) {
  QString selected;
  if (selection == "") {
    QListBoxItem* item = lbDeviceList->selectedItem();
    selected = (item ? item->text() : "");
  }
  else {
    selected = selection;
  }
  lbDeviceList->clear();
  BabelMap::const_iterator iter;
  for (iter = mDevices.begin(); iter != mDevices.end(); ++iter) {
    QListBoxText* item = new QListBoxText(iter->first);
    lbDeviceList->insertItem(item);
    if (iter->first == selected)
      lbDeviceList->setSelected(item, true);
  }
  if (lbDeviceList->selectedItem() == NULL)
    lbDeviceList->setSelected(0, true);
}


void QgsGPSDeviceDialog::slotSelectionChanged() {
  QString devName = lbDeviceList->selectedItem()->text();
  leDeviceName->setText(devName);
  QgsBabelFormat* device = dynamic_cast<QgsBabelCommand*>(mDevices[devName]);
  leDownloadCmd->setText(device->importCommand("%babel", "%type", 
					       "%in", "%out").join(" "));
  leUploadCmd->setText(device->exportCommand("%babel", "%type", 
					     "%in", "%out").join(" "));
}


void QgsGPSDeviceDialog::writeDeviceSettings() {
  QStringList deviceNames;
  QSettings settings;
  QString devPath = "/qgis/gps/devices/%1";
  BabelMap::const_iterator iter;
  for (iter = mDevices.begin(); iter != mDevices.end(); ++iter) {
    deviceNames.append(iter->first);
    QString download = 
      iter->second->importCommand("%babel","%type","%in","%out").join(" ");
    QString upload = 
      iter->second->exportCommand("%babel","%type","%in","%out").join(" ");
    settings.writeEntry(devPath.arg(iter->first) + "/download", download);
    settings.writeEntry(devPath.arg(iter->first) + "/upload", upload);
  }
  settings.writeEntry("/qgis/gps/devicelist", deviceNames);
}
