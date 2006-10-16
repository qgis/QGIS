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
#include "qgsnewhttpconnection.h"
#include "qgslayerprojectionselector.h"
#include "qgshttptransaction.h"
#include <QDomDocument>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QSettings>

static const QString WFS_NAMESPACE = "http://www.opengis.net/wfs";

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
  connect(treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(changeCRSFilter()));
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

int QgsWFSSourceSelect::getCapabilities(const QString& uri, QgsWFSSourceSelect::REQUEST_ENCODING e, std::list<QString>& typenames, std::list< std::list<QString> >& crs, std::list<QString>& titles, std::list<QString>& abstracts)
{
  switch(e)
    {
    case QgsWFSSourceSelect::GET:
      return getCapabilitiesGET(uri, typenames, crs, titles, abstracts);
    case QgsWFSSourceSelect::POST:
      return getCapabilitiesPOST(uri, typenames, crs, titles, abstracts);
    case QgsWFSSourceSelect::SOAP:
      return getCapabilitiesSOAP(uri, typenames, crs, titles, abstracts);
    }
  return 1;
}

int QgsWFSSourceSelect::getCapabilitiesGET(const QString& uri, std::list<QString>& typenames, std::list< std::list<QString> >& crs, std::list<QString>& titles, std::list<QString>& abstracts)
{
  QString request = uri + "SERVICE=WFS&REQUEST=GetCapabilities&VERSION=1.1.1";
  QByteArray result;
  QgsHttpTransaction http(request);
  http.getSynchronously(result);
  
  QDomDocument capabilitiesDocument;
  if(!capabilitiesDocument.setContent(result, true))
    {
      return 1; //error
    }
  
  

  //get the <FeatureType> elements
  QDomNodeList featureTypeList = capabilitiesDocument.elementsByTagNameNS(WFS_NAMESPACE, "FeatureType");
  for(unsigned int i = 0; i < featureTypeList.length(); ++i)
    {
      QString tname, title, abstract;
      QDomElement featureTypeElem = featureTypeList.at(i).toElement();
      std::list<QString> featureSRSList; //SRS list for this feature

      //Name
      QDomNodeList nameList = featureTypeElem.elementsByTagNameNS(WFS_NAMESPACE, "Name");
      if(nameList.length() > 0)
	{
	  tname = nameList.at(0).toElement().text();
	}
      //Title
      QDomNodeList titleList = featureTypeElem.elementsByTagNameNS(WFS_NAMESPACE, "Title");
      if(titleList.length() > 0)
	{
	  title = titleList.at(0).toElement().text();
	}
      //Abstract
      QDomNodeList abstractList = featureTypeElem.elementsByTagNameNS(WFS_NAMESPACE, "Abstract");
      if(abstractList.length() > 0)
	{
	  abstract = abstractList.at(0).toElement().text();
	}

      //DefaultSRS is always the first entry in the feature crs list
      QDomNodeList defaultSRSList = featureTypeElem.elementsByTagNameNS(WFS_NAMESPACE, "DefaultSRS");
      if(defaultSRSList.length() > 0)
	{
	  featureSRSList.push_back(defaultSRSList.at(0).toElement().text());
	}

      //OtherSRS
      QDomNodeList otherSRSList = featureTypeElem.elementsByTagNameNS(WFS_NAMESPACE, "OtherSRS");
      for(unsigned int i = 0; i < otherSRSList.length(); ++i)
	{
	  featureSRSList.push_back(otherSRSList.at(i).toElement().text());
	}

      //Support <SRS> for compatibility with older versions
      QDomNodeList srsList = featureTypeElem.elementsByTagNameNS(WFS_NAMESPACE, "SRS");
      for(unsigned int i = 0; i < srsList.length(); ++i)
	{
	  featureSRSList.push_back(srsList.at(i).toElement().text());
	}

      crs.push_back(featureSRSList);
      typenames.push_back(tname);
      titles.push_back(title);
      abstracts.push_back(abstract);
    }


  //print out result for a test
  QString resultString(result);
  qWarning(resultString);

  return 0;
}

int QgsWFSSourceSelect::getCapabilitiesPOST(const QString& uri, std::list<QString>& typenames, std::list< std::list<QString> >& crs, std::list<QString>& titles, std::list<QString>& abstracts)
{
  return 1; //soon...
}

int QgsWFSSourceSelect::getCapabilitiesSOAP(const QString& uri, std::list<QString>& typenames, std::list< std::list<QString> >& crs, std::list<QString>& titles, std::list<QString>& abstracts)
{
  return 1; //soon...
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
  std::list<QString> titles;
  std::list<QString> abstracts;

  if(getCapabilities(mUri, QgsWFSSourceSelect::GET, typenames, crsList, titles, abstracts) != 0)
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

  //insert the typenames, titles and abstracts into the tree view
  treeWidget->clear();
  std::list<QString>::const_iterator t_it = titles.begin();
  std::list<QString>::const_iterator n_it = typenames.begin();
  std::list<QString>::const_iterator a_it = abstracts.begin();
  for(; t_it != titles.end(); ++t_it, ++n_it, ++a_it)
    {
      QTreeWidgetItem* newItem = new QTreeWidgetItem();
      newItem->setText(0, *t_it);
      newItem->setText(1, *n_it);
      newItem->setText(2, *a_it);
      treeWidget->addTopLevelItem(newItem);
    }
  
  if(typenames.size() > 0)
    {
      btnAdd->setEnabled(true);
      treeWidget->setCurrentItem(treeWidget->topLevelItem(0));
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
  QTreeWidgetItem* tItem = treeWidget->currentItem();
  if(!tItem)
    {
      return;
    }
  QString typeName = tItem->text(1);
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
  QTreeWidgetItem* currentTreeItem = treeWidget->currentItem();
  if(currentTreeItem)
    {
      QString currentTypename = currentTreeItem->text(1);
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
