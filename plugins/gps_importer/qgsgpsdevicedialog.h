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
#ifndef QGSGPSDEVICEDIALOG_H
#define QGSGPSDEVICEDIALOG_H

#include "qgsgpsdevicedialogbase.h"
#include "qgsgpsdevice.h"

#include <qstring.h>


class QgsGPSDeviceDialog : public QgsGPSDeviceDialogBase {
  Q_OBJECT
public:
  QgsGPSDeviceDialog(std::map<QString, QgsGPSDevice*>& devices);

public slots:
  void slotNewDevice();
  void slotDeleteDevice();
  void slotUpdateDevice();
  void slotUpdateDeviceList(const QString& selection = "");
  void slotSelectionChanged();

signals:
  void devicesChanged();

private:
  void writeDeviceSettings();
  
  std::map<QString, QgsGPSDevice*>& mDevices;
};

#endif
