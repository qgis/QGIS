/***************************************************************************
    qgsoapifapirequest.h
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

#ifndef QGSOAPIFAPIREQUEST_H
#define QGSOAPIFAPIREQUEST_H

#include <QObject>

#include "qgsdatasourceuri.h"
#include "qgsbasenetworkrequest.h"
#include "qgslayermetadata.h"

//! Manages the /api request
class QgsOapifApiRequest : public QgsBaseNetworkRequest
{
    Q_OBJECT
  public:
    explicit QgsOapifApiRequest( const QgsDataSourceUri &baseUri, const QString &url );

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

    //! Return the maximum number of features that can be requested at once (-1 if unknown)
    int maxLimit() const { return mMaxLimit; }

    //! Return the default number of features that are requested at once (-1 if unknown)
    int defaultLimit() const { return mDefaultLimit; }

    //! Return metadata (mostly contact info)
    const QgsAbstractMetadataBase &metadata() const { return mMetadata; }

  signals:
    //! emitted when the capabilities have been fully parsed, or an error occurred */
    void gotResponse();

  private slots:
    void processReply();

  protected:
    QString errorMessageWithReason( const QString &reason ) override;

  private:
    QString mUrl;

    int mMaxLimit = -1;

    int mDefaultLimit = -1;

    QgsLayerMetadata mMetadata;

    ApplicationLevelError mAppLevelError = ApplicationLevelError::NoError;

};

#endif // QGSOAPIFAPIREQUEST_H
