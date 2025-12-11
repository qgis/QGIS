/***************************************************************************
    qgsoapifitemsrequest.h
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

#ifndef QGSOAPIFITEMSREQUEST_H
#define QGSOAPIFITEMSREQUEST_H

#include <vector>

#include "qgsbackgroundcachedfeatureiterator.h"
#include "qgsbasenetworkrequest.h"
#include "qgsdatasourceuri.h"
#include "qgsfeature.h"
#include "qgsrectangle.h"

#include <QObject>

//! Manages the /items request
class QgsOapifItemsRequest : public QgsBaseNetworkRequest
{
    Q_OBJECT
  public:
    explicit QgsOapifItemsRequest( const QgsDataSourceUri &uri, const QString &url, const QString &featureFormat );

    //! Ask to compute the bbox of the returned items.
    void setComputeBbox() { mComputeBbox = true; }

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
    [[nodiscard]] ApplicationLevelError applicationLevelError() const { return mAppLevelError; }

    //! Return fields.
    [[nodiscard]] const QgsFields &fields() const { return mFields; }

    //! Return geometry type.
    [[nodiscard]] Qgis::WkbType wkbType() const { return mWKBType; }

    //! Return features.
    [[nodiscard]] const std::vector<QgsFeatureUniqueIdPair> &features() const { return mFeatures; }

    //! Return features bounding box
    [[nodiscard]] const QgsRectangle &bbox() const { return mBbox; }

    //! Return number of matched features, or -1 if unknown.
    [[nodiscard]] int numberMatched() const { return mNumberMatched; }

    //! Return the url of the next page
    [[nodiscard]] const QString &nextUrl() const { return mNextUrl; }

    //! Return if an "id" is present at top level of features
    [[nodiscard]] bool foundIdTopLevel() const { return mFoundIdTopLevel; }

    //! Return if an "id" is present in the "properties" object of features
    [[nodiscard]] bool foundIdInProperties() const { return mFoundIdInProperties; }

  signals:
    //! emitted when the capabilities have been fully parsed, or an error occurred
    void gotResponse();

  private slots:
    void processReply();

  protected:
    QString errorMessageWithReason( const QString &reason ) override;

  private:
    const QString mUrl;

    const QString mFeatureFormat;

    bool mComputeBbox = false;

    QgsFields mFields;

    Qgis::WkbType mWKBType = Qgis::WkbType::Unknown;

    std::vector<QgsFeatureUniqueIdPair> mFeatures;

    QgsRectangle mBbox;

    int mNumberMatched = -1;

    QString mNextUrl;

    ApplicationLevelError mAppLevelError = ApplicationLevelError::NoError;

    bool mFoundIdTopLevel = false;

    bool mFoundIdInProperties = false;
};

#endif // QGSOAPIFITEMSREQUEST_H
