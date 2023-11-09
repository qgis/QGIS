/***************************************************************************
    qgsoapifpatchfeaturerequest.h
    -----------------------------
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

#ifndef QGSOAPIFPATCHFEATUREREQUEST_H
#define QGSOAPIFPATCHFEATUREREQUEST_H

#include <QObject>

#include "qgsdatasourceuri.h"
#include "qgsfeature.h"
#include "qgsbasenetworkrequest.h"

class QgsOapifSharedData;

//! Manages the Patch Feature request
class QgsOapifPatchFeatureRequest : public QgsBaseNetworkRequest
{
    Q_OBJECT
  public:
    explicit QgsOapifPatchFeatureRequest( const QgsDataSourceUri &uri );

    //! Issue a PATCH request to overwrite the feature by changing its geometry
    bool patchFeature( const QgsOapifSharedData *sharedData, const QString &jsonId, const QgsGeometry &geom, const QString &contentCrs, bool hasAxisInverted );

    //! Issue a PATCH request to overwrite the feature by changing some attributes
    bool patchFeature( const QgsOapifSharedData *sharedData, const QString &jsonId, const QgsAttributeMap &attrMap );

  protected:
    QString errorMessageWithReason( const QString &reason ) override;
};

#endif // QGSOAPIFPATCHFEATUREREQUEST_H
