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

  enum REQUEST_ENCODING
    {
      GET,
      POST,
      SOAP /*Note that this goes also through HTTP POST but additionally uses soap envelope and friends*/
    };

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

  /**Makes a GetCapabilities and returns the typenamse and crs supported by the server.
     @param typenames a list of layers provided by the server
     @param crs a list of crs supported by the server. The place in the list corresponds to the \
     typenames list (means that the crs list at position 0 is a crs for typename at position 0 etc.)
     @param title title list 
     @param abstract textual descriptions for the types
     @return 0 in case of success*/
  int getCapabilities(const QString& uri, QgsWFSSourceSelect::REQUEST_ENCODING e, std::list<QString>& typenames, std::list< std::list<QString> >& crs, std::list<QString>& titles, std::list<QString>& abstracts);
  //encoding specific methods of getCapabilities
  int getCapabilitiesGET(const QString& uri, std::list<QString>& typenames, std::list< std::list<QString> >& crs, std::list<QString>& titles, std::list<QString>& abstracts);
  int getCapabilitiesPOST(const QString& uri, std::list<QString>& typenames, std::list< std::list<QString> >& crs, std::list<QString>& titles, std::list<QString>& abstracts);
  int getCapabilitiesSOAP(const QString& uri, std::list<QString>& typenames, std::list< std::list<QString> >& crs, std::list<QString>& titles, std::list<QString>& abstracts);

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
