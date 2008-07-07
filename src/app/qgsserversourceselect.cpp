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

#include "qgsserversourceselect.h"

#include "qgslayerprojectionselector.h"

#include "qgsnewhttpconnection.h"
#include "qgsnumericsortlistviewitem.h"
#include "qgsproviderregistry.h"
#include "qgsspatialrefsys.h"
#include "../providers/wms/qgswmsprovider.h"
#include "qgscontexthelp.h"

#include "qgsproject.h"

#include "qgsmessageviewer.h"

#include <QMessageBox>
#include <QPicture>
#include <QSettings>
#include <QButtonGroup>
#include <QMap>
#include <QImageReader>

#include <iostream>

static long DEFAULT_WMS_EPSG = 4326;  // WGS 84


QgsServerSourceSelect::QgsServerSourceSelect(QgisApp * app, QWidget * parent, Qt::WFlags fl)
  : QDialog(parent, fl),
    m_Epsg(DEFAULT_WMS_EPSG),
    qgisApp(app),
    mWmsProvider(0)
{
  setupUi(this);
  connect(btnCancel, SIGNAL(clicked()), this, SLOT(reject()));

  // Qt Designer 4.1 doesn't let us use a QButtonGroup, so it has to
  // be done manually... Unless I'm missing something, it's a whole
  // lot harder to do groups of radio buttons in Qt4 than Qt3.
  m_imageFormatGroup = new QButtonGroup;
  m_imageFormatLayout = new QHBoxLayout;
  // Populate it with buttons for all of the formats that we can support. The
  // value is the user visible name, and a unique integer, and the key is the
  // mime type.
  QMap<QString, QPair<QString, int> > formats;
  // Note - the "PNG", etc text should match the Qt format strings as given by
  // a called QImageReader::supportedImageFormats(), but case doesn't matter
  m_PotentialFormats.insert("image/png",             qMakePair(QString("PNG"),  1));
  m_PotentialFormats.insert("image/jpeg",            qMakePair(QString("JPEG"), 2));
  m_PotentialFormats.insert("image/gif",             qMakePair(QString("GIF"),  4));
  // Desipte the Qt docs saying that it supports bmp, it practise it doesn't work...
  //m_PotentialFormats.insert("image/wbmp",            qMakePair(QString("BMP"),  5));
  // Some wms servers will support tiff, but by default Qt doesn't, but leave
  // this in for those versions of Qt that might support tiff
  m_PotentialFormats.insert("image/tiff",            qMakePair(QString("TIFF"), 6));

  QMap<QString, QPair<QString, int> >::const_iterator iter = m_PotentialFormats.constBegin();
  int i = 1;
  while (iter != m_PotentialFormats.end())
  {
    QRadioButton* btn = new QRadioButton(iter.value().first);
    m_imageFormatGroup->addButton(btn, iter.value().second);
    m_imageFormatLayout->addWidget(btn);
    if (i ==1)
    {
      btn->setChecked(true);
    }
    iter++;
    i++;
  }

  m_imageFormatLayout->addStretch();
  btnGrpImageEncoding->setLayout(m_imageFormatLayout);

  // set up the WMS connections we already know about
  populateConnectionList();

  //set the current project CRS if available
  long currentSRS = QgsProject::instance()->readNumEntry("SpatialRefSys", "/ProjectSRSID", -1);
  if(currentSRS != -1)
    {
      //convert SRS id to epsg
      QgsSpatialRefSys currentRefSys(currentSRS, QgsSpatialRefSys::QGIS_SRSID);
      if(currentRefSys.isValid())
	{
	  m_Epsg = currentRefSys.epsg();
	}
     }

  // set up the default WMS Coordinate Reference System
  labelCoordRefSys->setText( descriptionForEpsg(m_Epsg) );
}

QgsServerSourceSelect::~QgsServerSourceSelect()
{
    
}
void QgsServerSourceSelect::populateConnectionList()
{
  QSettings settings;
  settings.beginGroup("/Qgis/connections-wms");
  QStringList keys = settings.childGroups();
  QStringList::Iterator it = keys.begin();
  cmbConnections->clear();
  while (it != keys.end())
  {
    cmbConnections->addItem(*it);
    ++it;
  }
  settings.endGroup();
  setConnectionListPosition();

  if (keys.begin() == keys.end())
    {
      // No connections - disable various buttons
      btnConnect->setEnabled(FALSE);
      btnEdit->setEnabled(FALSE);
      btnDelete->setEnabled(FALSE);
    }
  else
    {
      // Connections - enable various buttons
      btnConnect->setEnabled(TRUE);
      btnEdit->setEnabled(TRUE);
      btnDelete->setEnabled(TRUE);
    }
}
void QgsServerSourceSelect::on_btnNew_clicked()
{

  QgsNewHttpConnection *nc = new QgsNewHttpConnection(this);

  if (nc->exec())
  {
    populateConnectionList();
  }
}

void QgsServerSourceSelect::on_btnEdit_clicked()
{

  QgsNewHttpConnection *nc = new QgsNewHttpConnection(this, "/Qgis/connections-wms/", cmbConnections->currentText());

  if (nc->exec())
  {
    populateConnectionList();
  }
}

void QgsServerSourceSelect::on_btnDelete_clicked()
{
  QSettings settings;
  QString key = "/Qgis/connections-wms/" + cmbConnections->currentText();
  QString msg =
    tr("Are you sure you want to remove the ") + cmbConnections->currentText() + tr(" connection and all associated settings?");
  QMessageBox::StandardButton result = QMessageBox::information(this, tr("Confirm Delete"), msg, QMessageBox::Ok | QMessageBox::Cancel);
  if (result == QMessageBox::Ok)
  {
    settings.remove(key);
    cmbConnections->removeItem(cmbConnections->currentIndex());  // populateConnectionList();
    setConnectionListPosition();
  }
}

void QgsServerSourceSelect::on_btnHelp_clicked()
{
  
  QgsContextHelp::run(context_id);

}

bool QgsServerSourceSelect::populateLayerList(QgsWmsProvider* wmsProvider)
{
  std::vector<QgsWmsLayerProperty> layers;

  if (!wmsProvider->supportedLayers(layers))
  {
    return FALSE;
  }

  lstLayers->clear();

  int layerAndStyleCount = 0;

  for (std::vector<QgsWmsLayerProperty>::iterator layer  = layers.begin();
                                                  layer != layers.end();
                                                  layer++)
  {

#ifdef QGISDEBUG
//  std::cout << "QgsServerSourceSelect::populateLayerList: got layer name " << layer->name.toLocal8Bit().data() << " and title '" << layer->title.toLocal8Bit().data() << "'." << std::endl;
#endif

    layerAndStyleCount++;

    QgsNumericSortTreeWidgetItem *lItem = new QgsNumericSortTreeWidgetItem(lstLayers);
    lItem->setText(0, QString::number(layerAndStyleCount));
    lItem->setText(1, layer->name.simplified());
    lItem->setText(2, layer->title.simplified());
    lItem->setText(3, layer->abstract.simplified());

    // Also insert the styles
    // Layer Styles
    for (uint j = 0; j < layer->style.size(); j++)
    {
#ifdef QGISDEBUG
  std::cout << "QgsServerSourceSelect::populateLayerList: got style name " << layer->style[j].name.toLocal8Bit().data() << " and title '" << layer->style[j].title.toLocal8Bit().data() << "'." << std::endl;
#endif

      layerAndStyleCount++;

      QgsNumericSortTreeWidgetItem *lItem2 = new QgsNumericSortTreeWidgetItem(lItem);
      lItem2->setText(0, QString::number(layerAndStyleCount));
      lItem2->setText(1, layer->style[j].name.simplified());
      lItem2->setText(2, layer->style[j].title.simplified());
      lItem2->setText(3, layer->style[j].abstract.simplified());

    }

  }

  // If we got some layers, let the user add them to the map
  if (lstLayers->topLevelItemCount() > 0)
  {
    btnAdd->setEnabled(TRUE);
  }
  else
  {
    btnAdd->setEnabled(FALSE);
  }

  return TRUE;
}


void QgsServerSourceSelect::populateImageEncodingGroup(QgsWmsProvider* wmsProvider)
{
  QStringList formats;

  formats = wmsProvider->supportedImageEncodings();

  //
  // Remove old group of buttons
  //
  QList<QAbstractButton*> btns = m_imageFormatGroup->buttons();
  for (int i = 0; i < btns.count(); ++i)
  {
    btns.at(i)->hide();
  }

  //
  // Collect capabilities reported by Qt itself
  //
  QList<QByteArray> qtImageFormats = QImageReader::supportedImageFormats();

  QList<QByteArray>::const_iterator it = qtImageFormats.begin();
  while( it != qtImageFormats.end() )
  {
#ifdef QGISDEBUG
    std::cout << "QgsServerSourceSelect::populateImageEncodingGroup: can support input of '" << (*it).data() << "'." << std::endl;
#endif
    ++it;
  }


  //
  // Add new group of buttons
  //

  bool first = true;
  for (QStringList::Iterator format  = formats.begin();
                             format != formats.end();
                           ++format)
  {
#ifdef QGISDEBUG
    std::cout << "QgsServerSourceSelect::populateImageEncodingGroup: got image format " << (*format).toLocal8Bit().data() << "." << std::endl;
#endif

    QMap<QString, QPair<QString, int> >::const_iterator iter = 
	m_PotentialFormats.find(*format);

    // The formats in qtImageFormats are lowercase, so force ours to lowercase too.
    if (iter != m_PotentialFormats.end() && 
	qtImageFormats.contains(iter.value().first.toLower().toLocal8Bit()))
    {
      m_imageFormatGroup->button(iter.value().second)->show();
      if (first)
      {
	first = false;
	m_imageFormatGroup->button(iter.value().second)->setChecked(true);
      }
    }
    else
    {
#ifdef QGISDEBUG
      std::cerr<<"Unsupported type of " << format->toLocal8Bit().data() << '\n';
#endif
    }
  }
}


std::set<QString> QgsServerSourceSelect::crsForSelection()
{
  std::set<QString> crsCandidates;

  // XXX - mloskot - temporary solution, function must return a value
  return crsCandidates;

  QStringList::const_iterator i;
  for (i = m_selectedLayers.constBegin(); i != m_selectedLayers.constEnd(); ++i)
  {
/*    

    for ( int j = 0; j < ilayerProperty.boundingBox.size(); i++ ) 
    {
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseLayer: testing bounding box CRS which is " 
            << layerProperty.boundingBox[i].crs.toLocal8Bit().data() << "." << std::endl;
#endif
      if ( layerProperty.boundingBox[i].crs == DEFAULT_LATLON_CRS )
      {
        extentForLayer[ layerProperty.name ] = 
                layerProperty.boundingBox[i].box;
      }
    }


    if
      cout << (*i).ascii() << endl;*/
  }

}


void QgsServerSourceSelect::on_btnConnect_clicked()
{
  // populate the table list
  QSettings settings;

  QString key = "/Qgis/connections-wms/" + cmbConnections->currentText();
  
  QStringList connStringParts;
  QString part;
  
  connStringParts += settings.value(key + "/url").toString();

  m_connName = cmbConnections->currentText();
  m_connInfo = connStringParts.join(" ");

#ifdef QGISDEBUG
  std::cout << "QgsServerSourceSelect::serverConnect: Connection info: '" << m_connInfo.toLocal8Bit().data() << "'." << std::endl;
#endif


  // TODO: Create and bind to data provider

  // load the server data provider plugin
  QgsProviderRegistry * pReg = QgsProviderRegistry::instance();

  mWmsProvider = 
    (QgsWmsProvider*) pReg->getProvider( "wms", m_connInfo );

  if (mWmsProvider)
  {
    connect(mWmsProvider, SIGNAL(setStatus(QString)), this, SLOT(showStatusMessage(QString)));

    // WMS Provider all set up; let's get some layers

    if (!populateLayerList(mWmsProvider))
    {
      showError(mWmsProvider);
    }
    populateImageEncodingGroup(mWmsProvider);
  }
  else
  {
    // Let user know we couldn't initialise the WMS provider
    QMessageBox::warning(
      this,
      tr("WMS Provider"),
      tr("Could not open the WMS Provider")
    );
  }

}

void QgsServerSourceSelect::on_btnAdd_clicked()
{
  if (m_selectedLayers.empty() == TRUE)
  {
    QMessageBox::information(this, tr("Select Layer"), tr("You must select at least one layer first."));
  }
  else if (mWmsProvider->supportedCrsForLayers(m_selectedLayers).size() == 0)
  {
    QMessageBox::information(this, tr("Coordinate Reference System"), tr("There are no available coordinate reference system for the set of layers you've selected."));
  }
  else
  {
    accept();
  }
}


void QgsServerSourceSelect::on_btnChangeSpatialRefSys_clicked()
{
  if (!mWmsProvider)
  {
    return;
  }

  QSet<QString> crsFilter = mWmsProvider->supportedCrsForLayers(m_selectedLayers);

  QgsLayerProjectionSelector * mySelector =
    new QgsLayerProjectionSelector(this);

  mySelector->setOgcWmsCrsFilter(crsFilter);

  QString myDefaultProjString = QgsProject::instance()->readEntry("SpatialRefSys", "/ProjectSRSProj4String", GEOPROJ4);
  QgsSpatialRefSys defaultSRS;
  if(defaultSRS.createFromProj4(myDefaultProjString))
    {
      mySelector->setSelectedSRSID(defaultSRS.srsid());
    }

  if (mySelector->exec())
  {
    m_Epsg = mySelector->getCurrentEpsg();
  }
  else
  {
    // NOOP
  }

  labelCoordRefSys->setText( descriptionForEpsg(m_Epsg) );
//  labelCoordRefSys->setText( mySelector->getCurrentProj4String() );

  delete mySelector;

  // update the display of this widget
  this->update();
}


/**
 * This function is used to:
 * 1. Store the list of selected layers and visual styles as appropriate.
 * 2. Ensure that only one style is selected per layer.
 *    If more than one is found, the most recently selected style wins.
 */
void QgsServerSourceSelect::on_lstLayers_itemSelectionChanged()
{
  QStringList newSelectedLayers;
  QStringList newSelectedStylesForSelectedLayers;

  std::map<QString, QString> newSelectedStyleIdForLayer;

  // Iterate through the layers
  QList<QTreeWidgetItem *> selected( lstLayers->selectedItems() );
  QList<QTreeWidgetItem *>::iterator it;
  for (it = selected.begin(); it != selected.end(); ++it)
  {
    QTreeWidgetItem *item = *it;
    QString layerName;

    if (item->parent() != 0)
    {
      layerName = item->parent()->text(1);
      newSelectedStylesForSelectedLayers += item->text(1);
    }
    else
    {
      layerName = item->text(1);
      newSelectedStylesForSelectedLayers += "";
    }

    newSelectedLayers += layerName;
    newSelectedStyleIdForLayer[layerName] = item->text(0);

    // Check if multiple styles have now been selected
    if (
        (!(m_selectedStyleIdForLayer[layerName].isNull())) &&  // not just isEmpty()
        (newSelectedStyleIdForLayer[layerName] != m_selectedStyleIdForLayer[layerName])
        )
    {
      // Remove old style selection
      lstLayers->findItems(m_selectedStyleIdForLayer[layerName], Qt::MatchRecursive).first()->setSelected(false);
    }

#ifdef QGISDEBUG
  std::cout << "QgsServerSourceSelect::addLayers: Added " << item->text(0).toLocal8Bit().data() << std::endl;
#endif

  }

  // If we got some selected items, let the user play with projections
  if (newSelectedLayers.count() > 0)
  {
    // Determine how many CRSs there are to play with
    if (mWmsProvider)
    {
      QSet<QString> crsFilter = mWmsProvider->supportedCrsForLayers(newSelectedLayers);

      gbCRS->setTitle(
                       QString( tr("Coordinate Reference System (%1 available)", "", crsFilter.count() ) )
                          .arg( crsFilter.count() )
                     );

      // check whether current CRS is supported
      // if not, use one of the available CRS
      bool isThere = false;
      long defaultEpsg = 0;
      for ( QSet<QString>::const_iterator i = crsFilter.begin(); i != crsFilter.end(); ++i)
      {
        QStringList parts = i->split(":");
        
        if (parts.at(0) == "EPSG")
        {
          long epsg = atol(parts.at(1).toUtf8());
          if (epsg == m_Epsg)
          {
            isThere = true;
            break;
          }
          
          // save first CRS in case we current m_Epsg is not available
          if (i == crsFilter.begin())
            defaultEpsg = epsg;
          // prefer value of DEFAULT_WMS_EPSG if available
          if (epsg == DEFAULT_WMS_EPSG)
            defaultEpsg = epsg;
        }
      }
      
      // we have to change selected CRS to default one
      if (!isThere && crsFilter.size() > 0)
      {
        m_Epsg = defaultEpsg;
        labelCoordRefSys->setText( descriptionForEpsg(m_Epsg) );
      }
    }

    btnChangeSpatialRefSys->setEnabled(TRUE);
  }
  else
  {
    btnChangeSpatialRefSys->setEnabled(FALSE);
  }

  m_selectedLayers                  = newSelectedLayers;
  m_selectedStylesForSelectedLayers = newSelectedStylesForSelectedLayers;
  m_selectedStyleIdForLayer         = newSelectedStyleIdForLayer;
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

QStringList QgsServerSourceSelect::selectedStylesForSelectedLayers()
{
  return m_selectedStylesForSelectedLayers;
}


QString QgsServerSourceSelect::selectedImageEncoding()
{
  // TODO: Match this hard coded list to the list of formats Qt reports it can actually handle.

  int id = m_imageFormatGroup->checkedId();
  QString label = m_imageFormatGroup->button(id)->text();

  // The way that we construct the buttons, this call to key() will never have
  // a value that isn't in the map, hence we don't need to check for the value
  // not being found.

  return m_PotentialFormats.key(qMakePair(label, id));
}


QString QgsServerSourceSelect::selectedCrs()
{
  if (m_Epsg)
  {
    return QString("EPSG:%1")
              .arg(m_Epsg);
  }
  else
  {
    return QString();
  }
}

void QgsServerSourceSelect::serverChanged()
{
  // Remember which server was selected.
  QSettings settings;
  settings.setValue("/Qgis/connections-wms/selected", 
		      cmbConnections->currentText());
}

void QgsServerSourceSelect::setConnectionListPosition()
{
  QSettings settings;
  QString toSelect = settings.value("/Qgis/connections-wms/selected").toString();
  // Does toSelect exist in cmbConnections?
  bool set = false;
  for (int i = 0; i < cmbConnections->count(); ++i)
    if (cmbConnections->itemText(i) == toSelect)
    {
      cmbConnections->setCurrentIndex(i);
      set = true;
      break;
    }
  // If we couldn't find the stored item, but there are some, 
  // default to the last item (this makes some sense when deleting
  // items as it allows the user to repeatidly click on delete to
  // remove a whole lot of items).
  if (!set && cmbConnections->count() > 0)
  {
    // If toSelect is null, then the selected connection wasn't found
    // by QSettings, which probably means that this is the first time
    // the user has used qgis with database connections, so default to
    // the first in the list of connetions. Otherwise default to the last.
    if (toSelect.isNull())
      cmbConnections->setCurrentIndex(0);
    else
      cmbConnections->setCurrentIndex(cmbConnections->count()-1);
  }
}
void QgsServerSourceSelect::showStatusMessage(QString const & theMessage)
{
  labelStatus->setText(theMessage);

  // update the display of this widget
  this->update();
}


void QgsServerSourceSelect::showError(QgsWmsProvider * wms)
{
//   QMessageBox::warning(
//     this,
//     wms->errorCaptionString(),
//     tr("Could not understand the response.  The") + " " + wms->name() + " " +
//       tr("provider said") + ":\n" +
//       wms->errorString()
//   );

  QgsMessageViewer * mv = new QgsMessageViewer(this);
  mv->setWindowTitle( wms->errorCaptionString() );
  mv->setMessageAsPlainText(
    tr("Could not understand the response.  The") + " " + wms->name() + " " +
    tr("provider said") + ":\n" +
    wms->errorString()
  );
  mv->showMessage(true); // Is deleted when closed
}

void QgsServerSourceSelect::on_cmbConnections_activated(int)
{
  serverChanged();
}

void QgsServerSourceSelect::on_btnAddDefault_clicked()
{
  addDefaultServers();
}

QString QgsServerSourceSelect::descriptionForEpsg(long epsg)
{
  // We'll assume this function isn't called very often,
  // so please forgive the lack of caching of results

  QgsSpatialRefSys qgisSrs = QgsSpatialRefSys(epsg, QgsSpatialRefSys::EPSG);

  return qgisSrs.description();
}

void QgsServerSourceSelect::addDefaultServers()
{
  QMap<QString, QString> exampleServers;
  exampleServers["NASA (JPL)"] = "http://wms.jpl.nasa.gov/wms.cgi";
  exampleServers["DM Solutions GMap"] = "http://www2.dmsolutions.ca/cgi-bin/mswms_gmap";
  exampleServers["Lizardtech server"] =  "http://wms.lizardtech.com/lizardtech/iserv/ows";
  // Nice to have the qgis users map, but I'm not sure of the URL at the moment.
  //  exampleServers["Qgis users map"] = "http://qgis.org/wms.cgi";

  QSettings settings;
  settings.beginGroup("/Qgis/connections-wms");
  QMap<QString, QString>::const_iterator i = exampleServers.constBegin();
  for (; i != exampleServers.constEnd(); ++i)
  {
    // Only do a server if it's name doesn't already exist.
    QStringList keys = settings.childGroups();
    if (!keys.contains(i.key()))
    {
      QString path = i.key();
      settings.setValue(path + "/url", i.value());
    }
  }
  settings.endGroup();
  populateConnectionList();

  QMessageBox::information(this, tr("WMS proxies"), "<p>" + tr("Several WMS servers have "
        "been added to the server list. Note that if "
        "you access the internet via a web proxy, you will "
        "need to set the proxy settings in the QGIS options dialog.") + "</p>");
}

// ENDS
