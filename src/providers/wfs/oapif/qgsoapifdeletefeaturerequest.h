/***************************************************************************
    qgsoapifdeletefeaturerequest.h
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

#ifndef QGSOAPIFDELETEFEATUREREQUEST_H
#define QGSOAPIFDELETEFEATUREREQUEST_H

#include <QObject>

#include "qgsdatasourceuri.h"
#include "qgsbasenetworkrequest.h"

//! Manages the Delete Feature request
class QgsOapifDeleteFeatureRequest : public QgsBaseNetworkRequest
{
    Q_OBJECT
  public:
    explicit QgsOapifDeleteFeatureRequest( const QgsDataSourceUri &uri );

  protected:
    QString errorMessageWithReason( const QString &reason ) override;
};

#endif // QGSOAPIFDELETEFEATUREREQUEST_H
