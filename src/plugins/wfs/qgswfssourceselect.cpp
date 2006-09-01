/***************************************************************************
                              qgswfssourceselect.cpp   
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

#include "qgisiface.h"
#include "qgswfssourceselect.h"
#include "../../providers/wfs/qgswfsprovider.h"
#include "qgsnewhttpconnection.h"
#include "qgslayerprojectionselector.h"
#include <QListWidgetItem>
#include <QMessageBox>
#include <QSettings>

QgsWFSSourceSelect::QgsWFSSourceSelect(QWidget* parent, QgisIface* iface): QDialog(parent), mIface(iface) 
{
  setupUi(this);
  
  connect(btnCancel, SIGNAL(clicked()), this, SLOT(reject()));
  connect(btnAdd, SIGNAL(clicked()), this, SLOT(addLayer()));
  connect(btnNew, SIGNAL(clicked()), this, SLOT(addEntryToServerList()));
  connect(btnEdit, SIGNAL(clicked()), this, SLOT(modifyEntryOfServerList()));
  connect(btnDelete, SIGNAL(clicked()), this, SLOT(deleteEntryOfServerList()));
  connect(btnConnect,SIGNAL(clicked()), this, SLOT(connectToServer()));
  connect(btnChangeSpatialRefSys, SIGNAL(clicked()), this, SLOT(changeCRS()));
  connect(lstWidget, SIGNAL(currentRowChanged(int)), this, SLOT(changeCRSFilter()));
  populateConnectionList();

  mProjectionSelector = new QgsLayerProjectionSelector(this);
}

QgsWFSSourceSelect::~QgsWFSSourceSelect()
{
  delete mProjectionSelector;
}

void QgsWFSSourceSelect::populateConnectionList()
{
  QSettings settings;
  QStringList keys = settings.subkeyList("/Qgis/connections-wfs");
  QStringList::Iterator it = keys.begin();
  cmbConnections->clear();
  while (it != keys.end())
  {
    cmbConnections->insertItem(*it);
    ++it;
  }

  if (keys.begin() != keys.end())
  {
    // Connections available - enable various buttons
    btnConnect->setEnabled(TRUE);
    btnEdit->setEnabled(TRUE);
    btnDelete->setEnabled(TRUE);
  }

  else
  {
    // No connections available - disable various buttons
    btnConnect->setEnabled(FALSE);
    btnEdit->setEnabled(FALSE);
    btnDelete->setEnabled(FALSE);
  }
}

void QgsWFSSourceSelect::addEntryToServerList()
{
  QgsNewHttpConnection *nc = new QgsNewHttpConnection(this, "/Qgis/connections-wfs/");

  if (nc->exec())
  {
    populateConnectionList();
  }
}

void QgsWFSSourceSelect::modifyEntryOfServerList()
{
  QgsNewHttpConnection nc(0, "/Qgis/connections-wfs/", cmbConnections->currentText());

  if (nc.exec())
  {
    nc.saveConnection();
  }
  populateConnectionList();
}

void QgsWFSSourceSelect::deleteEntryOfServerList()
{
  QSettings settings;
  QString key = "/Qgis/connections-wfs/" + cmbConnections->currentText();
  QString msg =
    tr("Are you sure you want to remove the ") + cmbConnections->currentText() + tr(" connection and all associated settings?");
  int result = QMessageBox::information(this, tr("Confirm Delete"), msg, tr("Yes"), tr("No"));
  if (result == 0)
  {
    settings.remove(key);
    cmbConnections->removeItem(cmbConnections->currentItem());
  }
}

void QgsWFSSourceSelect::connectToServer()
{
  //find out the server URL
  QSettings settings;
  QString key = "/Qgis/connections-wfs/" + cmbConnections->currentText() + "/url";
  mUri = settings.value(key).toString();
  qWarning("url is: " + mUri);

  //make a GetCapabilities request
  std::list<QString> typenames;
  std::list< std::list<QString> > crsList;
  if(QgsWFSProvider::getCapabilities(mUri, QgsWFSProvider::GET, typenames, crsList) != 0)
    {
      qWarning("error during GetCapabilities request");
    }

  //insert the available CRS into mAvailableCRS
  mAvailableCRS.clear();
  std::list<QString>::const_iterator typeNameIter;
  std::list< std::list<QString> >::const_iterator crsIter;
  for(typeNameIter = typenames.begin(), crsIter = crsList.begin(); typeNameIter != typenames.end(); ++typeNameIter, ++crsIter)
    {
      std::list<QString> currentCRSList;
      for(std::list<QString>::const_iterator it = crsIter->begin(); it != crsIter->end(); ++it)
	{
	  currentCRSList.push_back(*it);
	}
      mAvailableCRS.insert(std::make_pair(*typeNameIter, currentCRSList));
    }

  //insert the typenames into the list view
  lstWidget->clear();
  for(std::list<QString>::const_iterator it = typenames.begin(); it != typenames.end(); ++it)
    {
      lstWidget->addItem(*it);
    }
  
  if(typenames.size() > 0)
    {
      btnAdd->setEnabled(true);
      lstWidget->setCurrentRow(0);
      btnChangeSpatialRefSys->setEnabled(true);
    }
  else
    {
      btnAdd->setEnabled(false);
    }

  
}

void QgsWFSSourceSelect::addLayer()
{
  //get selected entry in lstWidget
  QListWidgetItem* cItem = lstWidget->currentItem();
  if(!cItem)
    {
      return;
    }
  QString typeName = cItem->text();
  qWarning(mUri + "SERVICE=WFS&VERSION=1.0.0&REQUEST=GetFeature&TYPENAME=" + typeName);

  //get CRS
  QString crsString;
  if(mProjectionSelector)
    {
      long epsgNr = mProjectionSelector->getCurrentEpsg();
      if(epsgNr != 0)
	{
	  crsString = "&SRSNAME=EPSG:"+QString::number(epsgNr);
	}
    }
  //add a wfs layer to the map
  if(mIface)
    {
      qWarning(mUri + "SERVICE=WFS&VERSION=1.0.0&REQUEST=GetFeature&TYPENAME=" + typeName + crsString);
      mIface->addVectorLayer(mUri + "SERVICE=WFS&VERSION=1.0.0&REQUEST=GetFeature&TYPENAME=" + typeName + crsString, typeName, "WFS");
    }
  accept();
}

void QgsWFSSourceSelect::changeCRS()
{
  if(mProjectionSelector->exec())
    {
      
    }
}

void QgsWFSSourceSelect::changeCRSFilter()
{
  //evaluate currently selected typename and set the CRS filter in mProjectionSelector
  QListWidgetItem* currentListItem = lstWidget->currentItem();
  if(currentListItem)
    {
      QString currentTypename = currentListItem->text();
      qWarning("the current typename is: " + currentTypename);
    
      std::map<QString, std::list<QString> >::const_iterator crsIterator = mAvailableCRS.find(currentTypename);
      if(crsIterator != mAvailableCRS.end())
	{
	  std::list<QString> crsList = crsIterator->second;
	  QSet<QString> crsSet;
	  for(std::list<QString>::const_iterator it = crsList.begin(); it != crsList.end(); ++it)
	    {
	      qWarning("inserting " + *it);
	      crsSet.insert(*it);
	    }
	  if(mProjectionSelector)
	    {
	      mProjectionSelector->setOgcWmsCrsFilter(crsSet);
	    }
	}
    }
}
