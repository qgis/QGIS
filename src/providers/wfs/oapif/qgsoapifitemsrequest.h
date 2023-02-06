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

#include <QObject>

#include "qgsdatasourceuri.h"
#include "qgsbasenetworkrequest.h"
#include "qgsfeature.h"
#include "qgsbackgroundcachedfeatureiterator.h"
#include "qgsrectangle.h"

#include <vector>

//! Manages the /items request
class QgsOapifItemsRequest : public QgsBaseNetworkRequest
{
    Q_OBJECT
  public:
    explicit QgsOapifItemsRequest( const QgsDataSourceUri &uri, const QString &url );

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
    ApplicationLevelError applicationLevelError() const { return mAppLevelError; }

    //! Return fields.
    const QgsFields &fields() const { return mFields; }

    //! Return geometry type.
    QgsWkbTypes::Type wkbType() const { return mWKBType; }

    //! Return features.
    const std::vector<QgsFeatureUniqueIdPair> &features() const { return mFeatures; }

    //! Return features bounding box
    const QgsRectangle &bbox() const { return mBbox; }

    //! Return number of matched features, or -1 if unknown.
    int numberMatched() const { return mNumberMatched; }

    //! Return the url of the next page
    const QString &nextUrl() const { return mNextUrl; }

  signals:
    //! emitted when the capabilities have been fully parsed, or an error occurred
    void gotResponse();

  private slots:
    void processReply();

  protected:
    QString errorMessageWithReason( const QString &reason ) override;

  private:
    QString mUrl;

    bool mComputeBbox = false;

    QgsFields mFields;

    QgsWkbTypes::Type mWKBType = QgsWkbTypes::Unknown;

    std::vector<QgsFeatureUniqueIdPair> mFeatures;

    QgsRectangle mBbox;

    int mNumberMatched = -1;

    QString mNextUrl;

    ApplicationLevelError mAppLevelError = ApplicationLevelError::NoError;

};

#endif // QGSOAPIFITEMSREQUEST_H
