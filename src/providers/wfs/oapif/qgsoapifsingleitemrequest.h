/***************************************************************************
    qgsoapifsingleitemrequest.h
    ---------------------------
    begin                : May 2024
    copyright            : (C) 2024 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOAPIFSINGLEITEMREQUEST_H
#define QGSOAPIFSINGLEITEMREQUEST_H

#include <QObject>

#include "qgsdatasourceuri.h"
#include "qgsbasenetworkrequest.h"
#include "qgsfeature.h"

//! Manages the /items/{id} request
class QgsOapifSingleItemRequest : public QgsBaseNetworkRequest
{
    Q_OBJECT
  public:
    explicit QgsOapifSingleItemRequest( const QgsDataSourceUri &uri, const QString &url );

    //! Issue the request
    bool request( bool synchronous, bool forceRefresh );

    //! Application level error
    enum class ApplicationLevelError
    {
      NoError,
      JsonError,
      IncompleteInformation
    };

    //! Returns application level error
    ApplicationLevelError applicationLevelError() const { return mAppLevelError; }

    //! Return fields.
    const QgsFields &fields() const { return mFields; }

    //! Return feature.
    const QgsFeature &feature() const { return mFeature; }

  signals:
    //! emitted when the capabilities have been fully parsed, or an error occurred
    void gotResponse();

  private slots:
    void processReply();

  protected:
    QString errorMessageWithReason( const QString &reason ) override;

  private:
    QString mUrl;

    QgsFields mFields;

    QgsFeature mFeature;

    ApplicationLevelError mAppLevelError = ApplicationLevelError::NoError;
};

#endif // QGSOAPIFSINGLEITEMREQUEST_H
