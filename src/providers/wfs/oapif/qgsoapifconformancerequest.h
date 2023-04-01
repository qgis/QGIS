/***************************************************************************
    qgsoapifconformancerequest.h
    -----------------------------
    begin                : April 2023
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

#ifndef QGSOAPIFCONFORMANCEREQUEST_H
#define QGSOAPIFCONFORMANCEREQUEST_H

#include <QObject>

#include "qgsdatasourceuri.h"
#include "qgsbasenetworkrequest.h"

//! Manages the conformance request
class QgsOapifConformanceRequest : public QgsBaseNetworkRequest
{
    Q_OBJECT
  public:
    explicit QgsOapifConformanceRequest( const QgsDataSourceUri &uri );

    //! Issue the request synchronously and return conformance classes
    QStringList conformanceClasses( const QUrl &conformanceUrl );

  private slots:
    void processReply();

  private:
    QStringList mConformanceClasses;

  protected:
    QString errorMessageWithReason( const QString &reason ) override;
};

#endif // QGSOAPIFCONFORMANCEREQUEST_H
