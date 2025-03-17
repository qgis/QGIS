/***************************************************************************
    qgsoapifschemarequest.h
    -----------------------
    begin                : March 2025
    copyright            : (C) 2025 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOAPIFSCHEMAREQUEST_H
#define QGSOAPIFSCHEMAREQUEST_H

#include <QObject>
#include <QMap>

#include "qgsdatasourceuri.h"
#include "qgsfields.h"
#include "qgsbasenetworkrequest.h"
#include "qgswkbtypes.h"

//! Manages the schema request
class QgsOapifSchemaRequest : public QgsBaseNetworkRequest
{
    Q_OBJECT
  public:
    explicit QgsOapifSchemaRequest( const QgsDataSourceUri &uri );

    struct Schema
    {
        //! Fields
        QgsFields mFields;

        QString mGeometryColumnName;
        //! Geometry column name;

        //! Geometry type;
        Qgis::WkbType mWKBType = Qgis::WkbType::NoGeometry;
    };

    //! Issue the request synchronously and return fields
    const Schema &schema( const QUrl &schemaUrl );

  private slots:
    void processReply();

  private:
    QUrl mUrl;

    Schema mSchema;

  protected:
    QString errorMessageWithReason( const QString &reason ) override;
};

#endif // QGSOAPIFSCHEMAREQUEST_H
