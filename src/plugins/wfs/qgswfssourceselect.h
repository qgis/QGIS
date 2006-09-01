/***************************************************************************
                              qgswfssourceselect.h    
                              -------------------
  begin                : August 25, 2006
  copyright            : (C) 2006 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWFSSOURCESELECT_H
#define QGSWFSSOURCESELECT_H

#include "ui_qgswfssourceselectbase.h"

class QgisIface;
class QgsLayerProjectionSelector;

class QgsWFSSourceSelect: public QDialog, private Ui::QgsWFSSourceSelectBase
{
  Q_OBJECT
 public:
  QgsWFSSourceSelect(QWidget* parent, QgisIface* iface);
  ~QgsWFSSourceSelect();
  
 private:
  QgsWFSSourceSelect(); //default constructor is forbidden
  QgisIface* mIface; //pointer to the QGIS interface object (needed to add WFS layers)
  QString mUri; //uri of the currently connected server
  QgsLayerProjectionSelector* mProjectionSelector;
  /**Stores the available CRS for a server connections.
   The first string is the typename, the corresponding list
  stores the CRS for the typename in the form 'EPSG:XXXX'*/
  std::map<QString, std::list<QString> > mAvailableCRS;
  void populateConnectionList();

  private slots:
  void addEntryToServerList();
  void modifyEntryOfServerList();
  void deleteEntryOfServerList();
  void connectToServer();
  void addLayer();
  void changeCRS();
  void changeCRSFilter();
};

#endif
