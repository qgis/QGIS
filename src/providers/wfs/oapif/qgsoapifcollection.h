/***************************************************************************
    qgsoapifcollection.h
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

#ifndef QGSOAPIFCOLLECTION_H
#define QGSOAPIFCOLLECTION_H

#include <QObject>

#include "qgsdatasourceuri.h"
#include "qgsbasenetworkrequest.h"
#include "qgsrectangle.h"
#include "qgslayermetadata.h"

#include <nlohmann/json.hpp>
using namespace nlohmann;
#include <vector>

//! Describes a collection
struct QgsOapifCollection
{
  //! Identifier
  QString mId;

  //! Title
  QString mTitle;

  //! Description
  QString mDescription;

  //! Bounding box (in CRS84)
  QgsRectangle mBbox;

  //! Layer metadata
  QgsLayerMetadata mLayerMetadata;

  //! Fills a collection from its JSON serialization
  bool deserialize( const json &j );
};

//! Manages the /collections request
class QgsOapifCollectionsRequest : public QgsBaseNetworkRequest
{
    Q_OBJECT
  public:
    explicit QgsOapifCollectionsRequest( const QgsDataSourceUri &baseUri, const QString &url );

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

    //! Returns collections description.
    const std::vector<QgsOapifCollection> &collections() const { return mCollections; }

    //! Return the url of the next page (extension to the spec)
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

    std::vector<QgsOapifCollection> mCollections;

    QString mNextUrl;

    ApplicationLevelError mAppLevelError = ApplicationLevelError::NoError;

};

//! Manages the /collection/{collectionId} request
class QgsOapifCollectionRequest : public QgsBaseNetworkRequest
{
    Q_OBJECT
  public:
    explicit QgsOapifCollectionRequest( const QgsDataSourceUri &baseUri, const QString &url );

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

    //! Returns collection description.
    const QgsOapifCollection &collection() const { return mCollection; }

  signals:
    //! emitted when the capabilities have been fully parsed, or an error occurred */
    void gotResponse();

  private slots:
    void processReply();

  protected:
    QString errorMessageWithReason( const QString &reason ) override;

  private:
    QString mUrl;

    QgsOapifCollection mCollection;

    ApplicationLevelError mAppLevelError = ApplicationLevelError::NoError;

};

#endif // QGSOAPIFCOLLECTION_H
