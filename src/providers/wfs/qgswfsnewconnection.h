/***************************************************************************
    qgswfsnewconnection.h
    ---------------------
    begin                : June 2018
    copyright            : (C) 2018 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWFSNEWCONNECTION_H
#define QGSWFSNEWCONNECTION_H

#include "qgsnewhttpconnection.h"
#include "qgswfsgetcapabilities.h"
#include "qgsoapiflandingpagerequest.h"
#include "qgsoapifapirequest.h"
#include "qgsoapifcollection.h"

class QgsWFSNewConnection : public QgsNewHttpConnection
{
    Q_OBJECT

  public:
    //! Constructor
    QgsWFSNewConnection( QWidget *parent = nullptr, const QString &connName = QString() );
    ~QgsWFSNewConnection() override;

  private slots:
    void versionDetectButton();
    void detectFormat();
    void capabilitiesReplyFinished();
    void oapifLandingPageReplyFinished();
    void oapifApiReplyFinished();
    void oapifCollectionsReplyFinished();

  private:
    QgsDataSourceUri createUri();
    void startCapabilitiesRequest();
    void startOapifLandingPageRequest();
    void startOapifApiRequest();
    void startOapifCollectionsRequest();

    std::unique_ptr<QgsWfsGetCapabilitiesRequest> mCapabilities;
    std::unique_ptr<QgsOapifLandingPageRequest> mOAPIFLandingPage;
    std::unique_ptr<QgsOapifApiRequest> mOAPIFApi;
    std::unique_ptr<QgsOapifCollectionsRequest> mOAPIFCollectionsRequest;
    QString mOAPIFCollectionsUrl;
    QString mOAPIFApiUrl;
    bool mDetectFormatInProgress = false;
};

#endif //QGSWFSNEWCONNECTION_H
