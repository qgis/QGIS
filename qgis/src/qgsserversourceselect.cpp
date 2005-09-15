/***************************************************************************
    qgserversourceselect.cpp  -  selector for WMS servers, etc.
                             -------------------
    begin                : 3 April 2005
    copyright            : (C) 2005 by Brendan Morley
    email                : morb at ozemail dot com dot au
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include <iostream>
#include <cassert>
#include <qsettings.h>
#include <qpixmap.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qstringlist.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qinputdialog.h>
#include <qgroupbox.h>

#include "xpm/point_layer.xpm"
#include "xpm/line_layer.xpm"
#include "xpm/polygon_layer.xpm"

#include "qgsserversourceselect.h"
#include "qgsnewhttpconnection.h"

#include "qgsproviderregistry.h"

#include "../providers/wms/qgswmsprovider.h"

#include "qgisapp.h"

QgsServerSourceSelect::QgsServerSourceSelect(QgisApp * app, QWidget * parent, const char *name)
  : QgsServerSourceSelectBase(parent, name),
    qgisApp(app)
{
  btnAdd->setEnabled(false);
  populateConnectionList();
  // connect the double-click signal to the addSingleLayer slot in the parent

  //disable the 'where clause' box for 0.4 release
  //  groupBox3->hide();

}

QgsServerSourceSelect::~QgsServerSourceSelect()
{
    
}
void QgsServerSourceSelect::populateConnectionList()
{
  QSettings settings;
  QStringList keys = settings.subkeyList("/Qgis/connections-wms");
  QStringList::Iterator it = keys.begin();
  cmbConnections->clear();
  while (it != keys.end())
  {
    cmbConnections->insertItem(*it);
    ++it;
  }
}
void QgsServerSourceSelect::addNewConnection()
{

  QgsNewHttpConnection *nc = new QgsNewHttpConnection();

  if (nc->exec())
  {
    populateConnectionList();
  }
}

void QgsServerSourceSelect::editConnection()
{

  QgsNewHttpConnection *nc = new QgsNewHttpConnection(cmbConnections->currentText());

  if (nc->exec())
  {
    nc->saveConnection();
  }
}

void QgsServerSourceSelect::deleteConnection()
{
  QSettings settings;
  QString key = "/Qgis/connections-wms/" + cmbConnections->currentText();
  QString msg =
    tr("Are you sure you want to remove the ") + cmbConnections->currentText() + tr(" connection and all associated settings?");
  int result = QMessageBox::information(this, tr("Confirm Delete"), msg, tr("Yes"), tr("No"));
  if (result == 0)
  {
    settings.removeEntry(key + "/url");
    cmbConnections->removeItem(cmbConnections->currentItem());  // populateConnectionList();
  }
}


void QgsServerSourceSelect::serverConnect()
{
  // populate the table list
  QSettings settings;

  QString key = "/Qgis/connections-wms/" + cmbConnections->currentText();
  
  QStringList connStringParts;
  QString part;
  
  connStringParts += settings.readEntry(key + "/url");
  
  // Add the proxy host and port if any are defined.
  if ( ! ( (part = settings.readEntry(key + "/proxyhost")).isEmpty() ) )
  {
#ifdef QGISDEBUG
  std::cout << "QgsServerSourceSelect::serverConnect: Got a proxyhost - '" << part.local8Bit() << "'." << std::endl;
#endif
    connStringParts += part;
  
    if ( ! ( (part = settings.readEntry(key + "/proxyport")).isEmpty() ) )
    {
#ifdef QGISDEBUG
  std::cout << "QgsServerSourceSelect::serverConnect: Got a proxyport - '" << part.local8Bit() << "'." << std::endl;
#endif
      connStringParts += part;
    }
  }  

  m_connName = cmbConnections->currentText();
  m_connInfo = connStringParts.join(" ");  // url ( + " " + proxyhost + " " + proxyport)

#ifdef QGISDEBUG
  std::cout << "QgsServerSourceSelect::serverConnect: Connection info: '" << m_connInfo.local8Bit() << "'." << std::endl;
#endif
    
    
  // TODO: Create and bind to data provider
  
  // load the server data provider plugin
  QgsProviderRegistry * pReg = QgsProviderRegistry::instance();
  
  QgsWmsProvider* wmsProvider = 
    (QgsWmsProvider*) pReg->getProvider( "wms", m_connInfo );
  
  std::vector<QgsWmsLayerProperty> layers;
   
  layers = wmsProvider->supportedLayers();
    
  lstLayers->clear();
  
  for (std::vector<QgsWmsLayerProperty>::iterator layer  = layers.begin();
                                                  layer != layers.end();
                                                  layer++)
       
  {

//    QgsWmsLayerProperty layer = *it;

#ifdef QGISDEBUG
  std::cout << "QgsServerSourceSelect::serverConnect: got layer name " << layer->name.local8Bit() << " and title '" << layer->title.local8Bit() << "'." << std::endl;
#endif


    QListViewItem *lItem = new QListViewItem(lstLayers);
    lItem->setText(1,layer->name);
    lItem->setText(2,layer->title);
//    lItem->setPixmap(0,*p);
    lstLayers->insertItem(lItem);

  }

  
  if (lstLayers->childCount() > 0)
  {
    btnAdd->setEnabled(true);
  }  

    
}

void QgsServerSourceSelect::addLayers()
{
  //store the layer info
  QListViewItemIterator it( lstLayers );
  while ( it.current() ) 
  {
    QListViewItem *item = it.current();
    ++it;

    if ( item->isSelected() )
    {
      m_selectedLayers += item->text(1);

#ifdef QGISDEBUG
  std::cout << "QgsServerSourceSelect::addLayers: Added " << item->text(1).local8Bit() << std::endl;
#endif
    
    }
  }

  if (m_selectedLayers.empty() == true)
  {
    QMessageBox::information(this, tr("Select Layer"), tr("You must select at least one layer first."));
  }  
  else
  {  
//    qgisApp->addRasterLayers("http://ims.cr.usgs.gov:80/servlet/com.esri.wms.Esrimap/USGS_EDC_Trans_BTS_Roads?", 
//                             "Test WMS Layer", "wms");

    accept();
  }  
}


QString QgsServerSourceSelect::connName()
{
  return m_connName;
}

QString QgsServerSourceSelect::connInfo()
{
  return m_connInfo;
}

QStringList QgsServerSourceSelect::selectedLayers()
{
  return m_selectedLayers;
}

