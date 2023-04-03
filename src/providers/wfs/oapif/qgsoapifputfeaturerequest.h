/***************************************************************************
    qgsoapifputfeaturerequest.h
    ---------------------------
    begin                : March 2023
    copyright            : (C) 2023 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOAPIFPUTFEATUREREQUEST_H
#define QGSOAPIFPUTFEATUREREQUEST_H

#include <QObject>

#include "qgsdatasourceuri.h"
#include "qgsfeature.h"
#include "qgsbasenetworkrequest.h"

class QgsOapifSharedData;

//! Manages the Put Feature request
class QgsOapifPutFeatureRequest : public QgsBaseNetworkRequest
{
    Q_OBJECT
  public:
    explicit QgsOapifPutFeatureRequest( const QgsDataSourceUri &uri );

    //! Issue a PUT request to overwrite the feature
    bool putFeature( const QgsOapifSharedData *sharedData, const QString &jsonId, const QgsFeature &f, const QString &contentCrs, bool hasAxisInverted );

  protected:
    QString errorMessageWithReason( const QString &reason ) override;
};

#endif // QGSOAPIFPUTFEATUREREQUEST_H
