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
#include "qgscontexthelp.h"

class QgisInterface;
class QgsGenericProjectionSelector;
class QNetworkReply;

class QgsWFSSourceSelect: public QDialog, private Ui::QgsWFSSourceSelectBase
{
    Q_OBJECT

  public:

    QgsWFSSourceSelect( QWidget* parent, QgisInterface* iface );
    ~QgsWFSSourceSelect();

  private:
    QgsWFSSourceSelect(); //default constructor is forbidden
    QgisInterface* mIface; //pointer to the QGIS interface object (needed to add WFS layers)
    QString mUri; //uri of the currently connected server
    QgsGenericProjectionSelector* mProjectionSelector;
    /**Stores the available CRS for a server connections.
     The first string is the typename, the corresponding list
    stores the CRS for the typename in the form 'EPSG:XXXX'*/
    std::map<QString, std::list<QString> > mAvailableCRS;
    QAbstractButton* btnAdd;
    QNetworkReply *mCapabilitiesReply;

    void populateConnectionList();

    /**Returns the best suited CRS from a set of authority ids
       1. project CRS if contained in the set
       2. WGS84 if contained in the set
       3. the first entry in the set else
    @return the authority id of the crs or an empty string in case of error*/
    QString getPreferredCrs( const QSet<QString>& crsSet ) const;

  private slots:
    void addEntryToServerList();
    void modifyEntryOfServerList();
    void deleteEntryOfServerList();
    void connectToServer();
    void addLayer();
    void changeCRS();
    void changeCRSFilter();
    void on_cmbConnections_activated( int index );
    void capabilitiesReplyFinished();
    void capabilitiesReplyProgress( qint64, qint64 );

    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }
};

#endif
