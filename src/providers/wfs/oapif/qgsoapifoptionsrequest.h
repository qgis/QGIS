/***************************************************************************
    qgsoapifoptionsrequest.h
    ------------------------
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

#ifndef QGSOAPIFOPTIONSREQUEST_H
#define QGSOAPIFOPTIONSREQUEST_H

#include <QObject>

#include "qgsdatasourceuri.h"
#include "qgsbasenetworkrequest.h"

//! Manages the Options request
class QgsOapifOptionsRequest : public QgsBaseNetworkRequest
{
    Q_OBJECT
  public:
    explicit QgsOapifOptionsRequest( const QgsDataSourceUri &uri );

  protected:
    QString errorMessageWithReason( const QString &reason ) override;
};

#endif // QGSOAPIFOPTIONSREQUEST_H
