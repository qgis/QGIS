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


QgsGPSDeviceDialog::QgsGPSDeviceDialog(std::map<QString, QgsGPSDevice*>& 
				       devices) : 
     QgsGPSDeviceDialogBase(0, 0, true ), //ensure dialog is openened modal
     mDevices(devices)
  
{
  slotUpdateDeviceList();
}


void QgsGPSDeviceDialog::slotNewDevice() {
  std::map<QString, QgsGPSDevice*>::const_iterator iter = mDevices.begin();
  QString deviceName = "New device %1";
  int i;
  for (i = 1; iter != mDevices.end(); ++i)
    iter = mDevices.find(deviceName.arg(i));
  deviceName = deviceName.arg(i - 1);
  mDevices[deviceName] = new QgsGPSDevice;
  writeDeviceSettings();
  slotUpdateDeviceList(deviceName);
  emit devicesChanged();
}


void QgsGPSDeviceDialog::slotDeleteDevice() {
  if (QMessageBox::warning(this, "Are you sure?", 
			   "Are you sure that you want to delete this device?",
			   QMessageBox::Ok, QMessageBox::Cancel) == 
      QMessageBox::Ok) {
    std::map<QString, QgsGPSDevice*>::iterator iter = 
      mDevices.find(lbDeviceList->selectedItem()->text());
    delete iter->second;
    mDevices.erase(iter);
    writeDeviceSettings();
    slotUpdateDeviceList();
    emit devicesChanged();
  }
}


void QgsGPSDeviceDialog::slotUpdateDevice() {
  std::map<QString, QgsGPSDevice*>::iterator iter = 
    mDevices.find(lbDeviceList->selectedItem()->text());
  delete iter->second;
  mDevices.erase(iter);
  mDevices[leDeviceName->text()] =
    new QgsGPSDevice(leWptDown->text(), leWptUp->text(),
		     leRteDown->text(), leRteUp->text(),
		     leTrkDown->text(), leTrkUp->text());
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
  std::map<QString, QgsGPSDevice*>::const_iterator iter;
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
  QgsGPSDevice* device = mDevices[devName];
  QStringList tmpList;
  leWptDown->setText(device->
		     importCommand("%babel", "-w", "%in", "%out").join(" "));
  leWptUp->setText(device->
		   exportCommand("%babel", "-w", "%in", "%out").join(" "));
  leRteDown->setText(device->
		     importCommand("%babel", "-r", "%in", "%out").join(" "));
  leRteUp->setText(device->
		   exportCommand("%babel", "-r", "%in", "%out").join(" "));
  leTrkDown->setText(device->
		     importCommand("%babel", "-t", "%in", "%out").join(" "));
  leTrkUp->setText(device->
		   exportCommand("%babel", "-t", "%in", "%out").join(" "));
}


void QgsGPSDeviceDialog::writeDeviceSettings() {
  QStringList deviceNames;
  QSettings settings;
  QString devPath = "/qgis/gps/devices/%1";
  std::map<QString, QgsGPSDevice*>::const_iterator iter;
  for (iter = mDevices.begin(); iter != mDevices.end(); ++iter) {
    deviceNames.append(iter->first);
    QString wptDownload = 
      iter->second->importCommand("%babel","-w","%in","%out").join(" ");
    QString wptUpload = 
      iter->second->exportCommand("%babel","-w","%in","%out").join(" ");
    QString rteDownload = 
      iter->second->importCommand("%babel","-r","%in","%out").join(" ");
    QString rteUpload = 
      iter->second->exportCommand("%babel","-r","%in","%out").join(" ");
    QString trkDownload = 
      iter->second->importCommand("%babel","-t","%in","%out").join(" ");
    QString trkUpload = 
      iter->second->exportCommand("%babel","-t","%in","%out").join(" ");
    settings.writeEntry(devPath.arg(iter->first) + "/wptdownload", 
			wptDownload);
    settings.writeEntry(devPath.arg(iter->first) + "/wptupload", wptUpload);
    settings.writeEntry(devPath.arg(iter->first) + "/rtedownload", 
			rteDownload);
    settings.writeEntry(devPath.arg(iter->first) + "/rteupload", rteUpload);
    settings.writeEntry(devPath.arg(iter->first) + "/trkdownload", 
			trkDownload);
    settings.writeEntry(devPath.arg(iter->first) + "/trkupload", trkUpload);
  }
  settings.writeEntry("/qgis/gps/devicelist", deviceNames);
}
