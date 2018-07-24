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
#include "qgswfsconstants.h"
#include "qgswfscapabilities.h"

class QgsWFSNewConnection : public QgsNewHttpConnection
{
    Q_OBJECT

  public:
    //! Constructor
    QgsWFSNewConnection( QWidget *parent = nullptr, const QString &connName = QString::null );
    ~QgsWFSNewConnection() override;

  private slots:
    void versionDetectButton();
    void capabilitiesReplyFinished();

  private:
    QgsWfsCapabilities *mCapabilities = nullptr;

};

#endif //QGSWFSNEWCONNECTION_H
