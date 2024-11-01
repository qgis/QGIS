/***************************************************************************
    qgsoapifqueryablesrequest.h
    ---------------------------
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

#ifndef QGSOAPIFQUERYABLESREQUEST_H
#define QGSOAPIFQUERYABLESREQUEST_H

#include <QObject>
#include <QMap>

#include "qgsdatasourceuri.h"
#include "qgsbasenetworkrequest.h"

//! Manages the conformance request
class QgsOapifQueryablesRequest : public QgsBaseNetworkRequest
{
    Q_OBJECT
  public:
    explicit QgsOapifQueryablesRequest( const QgsDataSourceUri &uri );

    //! Describes a queryable parameter.
    struct Queryable
    {
        //! whether the parameter is a geometry
        bool mIsGeometry = false;

        //! type as in a JSON schema: "string", "integer", "number", etc.
        QString mType;

        //! format as in JSON schema. e.g "date-time" if mType="string"
        QString mFormat;
    };

    //! Issue the request synchronously and return queryables
    const QMap<QString, Queryable> &queryables( const QUrl &queryablesUrl );

  private slots:
    void processReply();

  private:
    QMap<QString, Queryable> mQueryables;

  protected:
    QString errorMessageWithReason( const QString &reason ) override;
};

#endif // QGSOAPIFQUERYABLESREQUEST_H
