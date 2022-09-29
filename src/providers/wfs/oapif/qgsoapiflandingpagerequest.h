/***************************************************************************
    qgsoapiflandingpagerequest.h
    ---------------------
    begin                : October 2019
    copyright            : (C) 2019 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOAPIFLANDINGPAGEREQUEST_H
#define QGSOAPIFLANDINGPAGEREQUEST_H

#include <QObject>

#include "qgsdatasourceuri.h"
#include "qgsbasenetworkrequest.h"

//! Manages the GetLandingPage request
class QgsOapifLandingPageRequest : public QgsBaseNetworkRequest
{
    Q_OBJECT
  public:
    explicit QgsOapifLandingPageRequest( const QgsDataSourceUri &uri );

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

    //! Return URL of the api endpoint
    const QString &apiUrl() const { return mApiUrl; }

    //! Return URL of the api endpoint
    const QString &collectionsUrl() const { return mCollectionsUrl; }

  signals:
    //! emitted when the capabilities have been fully parsed, or an error occurred
    void gotResponse();

  private slots:
    void processReply();

  protected:
    QString errorMessageWithReason( const QString &reason ) override;

  private:
    QgsDataSourceUri mUri;

    //! URL of the api endpoint.
    QString mApiUrl;

    //! URL of the collections endpoint.
    QString mCollectionsUrl;

    ApplicationLevelError mAppLevelError = ApplicationLevelError::NoError;

};

#endif // QGSOAPIFLANDINGPAGEREQUEST_H
