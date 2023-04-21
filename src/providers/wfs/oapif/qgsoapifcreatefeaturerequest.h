/***************************************************************************
    qgsoapifcreatefeaturerequest.h
    ------------------------------
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

#ifndef QGSOAPIFCREATEFEATUREREQUEST_H
#define QGSOAPIFCREATEFEATUREREQUEST_H

#include <QObject>

#include "qgsdatasourceuri.h"
#include "qgsfeature.h"
#include "qgsbasenetworkrequest.h"

class QgsOapifSharedData;

//! Manages the Create Feature request
class QgsOapifCreateFeatureRequest : public QgsBaseNetworkRequest
{
    Q_OBJECT
  public:
    explicit QgsOapifCreateFeatureRequest( const QgsDataSourceUri &uri );

    //! Issue a POST request to create the feature and return its id
    QString createFeature( const QgsOapifSharedData *sharedData, const QgsFeature &f, const QString &contentCrs, bool hasAxisInverted );

  protected:
    QString errorMessageWithReason( const QString &reason ) override;
};

#endif // QGSOAPIFCREATEFEATUREREQUEST_H
