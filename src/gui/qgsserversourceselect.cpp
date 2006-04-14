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
#include "../providers/wms/qgswmsprovider.h"
#include "qgscontexthelp.h"

#include "qgsproject.h"

#include "qgsmessageviewer.h"

#include <QMessageBox>
#include <QPicture>
#include <QSettings>
#include <QButtonGroup>

#include <iostream>

static long DEFAULT_WMS_EPSG = 4326;  // WGS 84


QgsServerSourceSelect::QgsServerSourceSelect(QgisApp * app, QWidget * parent, Qt::WFlags fl)
  : QDialog(parent, fl),
    qgisApp(app),
    mWmsProvider(0),
    m_Epsg(DEFAULT_WMS_EPSG)
{
  setupUi(this);
  connect(btnCancel, SIGNAL(clicked()), this, SLOT(reject()));

  // Qt Designer 4.1 doesn't let us use a QButtonGroup, so it has to
  // be done manually... Unless I'm missing something, it's a whole
  // lot harder to do groups of radio buttons in Qt4 than Qt3.
  m_imageFormatBtns = new QButtonGroup;
  // Populate it with a couple of buttons
  QRadioButton* btn1 = new QRadioButton(tr("PNG"));
  QRadioButton* btn2 = new QRadioButton(tr("JPEG"));
  m_imageFormatBtns->addButton(btn1, 1);
  m_imageFormatBtns->addButton(btn2, 2);
  btn1->setChecked(true);
  // And lay then out horizontally
  QHBoxLayout *hbox = new QHBoxLayout;
  hbox->addWidget(btn1);
  hbox->addWidget(btn2);
  hbox->addStretch();
  btnGrpImageEncoding->setLayout(hbox);

  // set up the WMS connections we already know about
  populateConnectionList();

  // set up the default WMS Coordinate Reference System
  labelCoordRefSys->setText( descriptionForEpsg(m_Epsg) );
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
  setConnectionListPosition();

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

  QgsNewHttpConnection *nc = new QgsNewHttpConnection(this, cmbConnections->currentText());

  if (nc->exec())
  {
    nc->saveConnection();
  }
}

void QgsServerSourceSelect::on_btnDelete_clicked()
{
  QSettings settings;
  QString key = "/Qgis/connections-wms/" + cmbConnections->currentText();
  QString msg =
    tr("Are you sure you want to remove the ") + cmbConnections->currentText() + tr(" connection and all associated settings?");
  int result = QMessageBox::information(this, tr("Confirm Delete"), msg, tr("Yes"), tr("No"));
  if (result == 0)
  {
    settings.remove(key);
    cmbConnections->removeItem(cmbConnections->currentItem());  // populateConnectionList();
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

    QgsNumericSortListViewItem *lItem = new QgsNumericSortListViewItem(lstLayers);
    lItem->setText(0, QString::number(layerAndStyleCount));
    lItem->setText(1, layer->name.simplifyWhiteSpace());
    lItem->setText(2, layer->title.simplifyWhiteSpace());
    lItem->setText(3, layer->abstract.simplifyWhiteSpace());
    lstLayers->insertItem(lItem);

    // Also insert the styles
    // Layer Styles
    for (int j = 0; j < layer->style.size(); j++)
    {
#ifdef QGISDEBUG
  std::cout << "QgsServerSourceSelect::populateLayerList: got style name " << layer->style[j].name.toLocal8Bit().data() << " and title '" << layer->style[j].title.toLocal8Bit().data() << "'." << std::endl;
#endif

      layerAndStyleCount++;

      QgsNumericSortListViewItem *lItem2 = new QgsNumericSortListViewItem(lItem);
      lItem2->setText(0, QString::number(layerAndStyleCount));
      lItem2->setText(1, layer->style[j].name.simplifyWhiteSpace());
      lItem2->setText(2, layer->style[j].title.simplifyWhiteSpace());
      lItem2->setText(3, layer->style[j].abstract.simplifyWhiteSpace());

      lItem->insertItem(lItem2);

    }

  }

  // If we got some layers, let the user add them to the map
  if (lstLayers->childCount() > 0)
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
  QList<QAbstractButton*> btns = m_imageFormatBtns->buttons();
  QList<QAbstractButton*>::const_iterator iter = btns.begin();
  for (; iter != btns.end(); ++iter)
  {
    m_imageFormatBtns->removeButton(*iter);
    // Should the buttons be deleted too?
  }

  m_MimeTypeForButtonId.clear();

  //
  // Collect capabilities reported by Qt itself
  //
  QStringList qtImageFormats = QPicture::inputFormatList();

  QStringList::Iterator it = qtImageFormats.begin();
  while( it != qtImageFormats.end() )
  {
    std::cout << "QgsServerSourceSelect::populateImageEncodingGroup: can support input of '" << (*it).toLocal8Bit().data() << "'." << std::endl;
    ++it;
  }

  //
  // Add new group of buttons
  //

  int i = 1;
  for (QStringList::Iterator format  = formats.begin();
                             format != formats.end();
                           ++format)
  {

#ifdef QGISDEBUG
  std::cout << "QgsServerSourceSelect::populateImageEncodingGroup: got image format " << (*format).toLocal8Bit().data() << "." << std::endl;
#endif

    QRadioButton* radioButton = new QRadioButton;
    m_imageFormatBtns->addButton(radioButton, i);

    if      ((*format) == "image/png")
    {
      radioButton->setText(tr("PNG"));
    }
    else if ((*format) == "image/jpeg")
    {
      radioButton->setText(tr("JPEG"));
    }

    m_imageFormatBtns->addButton(radioButton);

    m_MimeTypeForButtonId[i] = (*format);

    i++;
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
  
  connStringParts += settings.readEntry(key + "/url");
  
  // Add the proxy host and port if any are defined.
  if ( ! ( (part = settings.readEntry(key + "/proxyhost")).isEmpty() ) )
  {
#ifdef QGISDEBUG
  std::cout << "QgsServerSourceSelect::serverConnect: Got a proxyhost - '" << part.toLocal8Bit().data() << "'." << std::endl;
#endif
    connStringParts += part;
  
    if ( ! ( (part = settings.readEntry(key + "/proxyport")).isEmpty() ) )
    {
#ifdef QGISDEBUG
  std::cout << "QgsServerSourceSelect::serverConnect: Got a proxyport - '" << part.toLocal8Bit().data() << "'." << std::endl;
#endif
      connStringParts += part;
    }
    else
    {
      connStringParts += "80";   // well-known http port
    }

    if ( ! ( (part = settings.readEntry(key + "/proxyuser")).isEmpty() ) )
    {
#ifdef QGISDEBUG
  std::cout << "QgsServerSourceSelect::serverConnect: Got a proxyuser - '" << part.toLocal8Bit().data() << "'." << std::endl;
#endif
      connStringParts += part;

      if ( ! ( (part = settings.readEntry(key + "/proxypass")).isEmpty() ) )
      {
#ifdef QGISDEBUG
  std::cout << "QgsServerSourceSelect::serverConnect: Got a proxypass - '" << part.toLocal8Bit().data() << "'." << std::endl;
#endif
        connStringParts += part;
      }
    }
  }  

  m_connName = cmbConnections->currentText();
  // setup 'url ( + " " + proxyhost + " " + proxyport + " " + proxyuser + " " + proxypass)'
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

    if (!populateLayerList(mWmsProvider))
    {
      showError(mWmsProvider);
    }
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

  long myDefaultSRS = QgsProject::instance()->readNumEntry("SpatialRefSys", "/ProjectSRSID", GEOSRS_ID);

  mySelector->setSelectedSRSID(myDefaultSRS);

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
void QgsServerSourceSelect::on_lstLayers_selectionChanged()
{
  QString layerName = "";

  QStringList newSelectedLayers;
  QStringList newSelectedStylesForSelectedLayers;

  std::map<QString, QString> newSelectedStyleIdForLayer;

  // Iterate through the layers
  Q3ListViewItemIterator it( lstLayers );
  while ( it.current() )
  {
    Q3ListViewItem *item = it.current();

    // save the name of the layer (in case only one of its styles was
    // selected)
    if (item->parent() == 0)
    {
      layerName = item->text(1);
    }

    if ( item->isSelected() )
    {
      newSelectedLayers += layerName;

      // save the name of the style selected for the layer, if appropriate

      if (item->parent() != 0)
      {
        newSelectedStylesForSelectedLayers += item->text(1);
      }
      else
      {
        newSelectedStylesForSelectedLayers += "";
      }

      newSelectedStyleIdForLayer[layerName] = item->text(0);

      // Check if multiple styles have now been selected
      if (
          (!(m_selectedStyleIdForLayer[layerName].isNull())) &&  // not just isEmpty()
          (newSelectedStyleIdForLayer[layerName] != m_selectedStyleIdForLayer[layerName])
          )
      {
        // Remove old style selection
        lstLayers->findItem(m_selectedStyleIdForLayer[layerName], 0)->setSelected(FALSE);
      }

#ifdef QGISDEBUG
  std::cout << "QgsServerSourceSelect::addLayers: Added " << item->text(0).toLocal8Bit().data() << std::endl;
#endif
    
    }

    ++it;
  }

  // If we got some selected items, let the user play with projections
  if (newSelectedLayers.count() > 0)
  {
    // Determine how many CRSs there are to play with
    if (mWmsProvider)
    {
      QSet<QString> crsFilter = mWmsProvider->supportedCrsForLayers(newSelectedLayers);

      gbCRS->setTitle(
                       QString( tr("Coordinate Reference System (%1 available)") )
                          .arg( crsFilter.count() )
                     );

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

  QAbstractButton* checked = m_imageFormatBtns->checkedButton();

  if (checked->text() == tr("PNG"))
    return "image/png";
  else if (checked->text() == tr("JPEG"))
    return "image/jpeg";
  else // Worst-case scenario - fall back to PNG
    return "image/png";

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
  settings.writeEntry("/Qgis/connections-wms/selected", 
		      cmbConnections->currentText());
}

void QgsServerSourceSelect::setConnectionListPosition()
{
  QSettings settings;
  QString toSelect = settings.readEntry("/Qgis/connections-wms/selected");
  // Does toSelect exist in cmbConnections?
  bool set = false;
  for (int i = 0; i < cmbConnections->count(); ++i)
    if (cmbConnections->text(i) == toSelect)
    {
      cmbConnections->setCurrentItem(i);
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
      cmbConnections->setCurrentItem(0);
    else
      cmbConnections->setCurrentItem(cmbConnections->count()-1);
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
  mv->setCaption( wms->errorCaptionString() );
  mv->setMessageAsPlainText(
    tr("Could not understand the response.  The") + " " + wms->name() + " " +
    tr("provider said") + ":\n" +
    wms->errorString()
  );
  mv->exec();
  delete mv;
}

void QgsServerSourceSelect::on_cmbConnections_activated(int)
{
  serverChanged();
}

QString QgsServerSourceSelect::descriptionForEpsg(long epsg)
{
  // We'll assume this function isn't called very often,
  // so please forgive the lack of caching of results

  QgsSpatialRefSys qgisSrs = QgsSpatialRefSys(epsg, QgsSpatialRefSys::EPSG);

  return qgisSrs.description();
}


// ENDS
