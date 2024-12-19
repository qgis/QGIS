/***************************************************************************
    qgswfsgetfeature.h
    ---------------------
    begin                : November 2022
    copyright            : (C) 2022 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSWFSGETFEATURE_H
#define QGSWFSGETFEATURE_H

#include "qgswfsrequest.h"
#include "qgswfscapabilities.h"

//! Manages the QgsWFSGetFeature request
class QgsWFSGetFeature : public QgsWfsRequest
{
    Q_OBJECT
  public:
    explicit QgsWFSGetFeature( QgsWFSDataSourceURI &uri );

    //! Issue the request
    bool request( bool synchronous,
                  const QString &WFSVersion,
                  const QString &typeName,
                  const QString &filter,
                  bool hitsOnly,
                  const QgsWfsCapabilities::Capabilities &caps );

  protected:
    QString errorMessageWithReason( const QString &reason ) override;
};

#endif // QGSWFSGETFEATURE_H
