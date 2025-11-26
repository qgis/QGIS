/***************************************************************************
    qgsoapifshareddata.h
    --------------------
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

#ifndef QGSOAPIFSHAREDDATA_H
#define QGSOAPIFSHAREDDATA_H

#include "qgsbackgroundcachedshareddata.h"
#include "qgsexpression.h"
#include "qgsoapifapirequest.h"
#include "qgsoapiffiltertranslationstate.h"
#include "qgsoapifqueryablesrequest.h"
#include "qgsrectangle.h"
#include "qgswfsdatasourceuri.h"

#include <QMap>
#include <QString>

class QgsFeatureDownloader;
class QgsFeatureDownloaderImpl;

//! Class shared between provider and feature source
class QgsOapifSharedData final : public QObject, public QgsBackgroundCachedSharedData
{
    Q_OBJECT
  public:
    explicit QgsOapifSharedData( const QString &uri );
    ~QgsOapifSharedData() override;

    //! Compute OAPIF filter from the filter in the URI
    bool computeServerFilter( QString &errorMsg );

    QString computedExpression( const QgsExpression &expression ) const override;

    bool hasGeometry() const override { return mWKBType != Qgis::WkbType::Unknown; }

    std::unique_ptr<QgsFeatureDownloaderImpl> newFeatureDownloaderImpl( QgsFeatureDownloader *, bool requestFromMainThread ) override;

    bool isRestrictedToRequestBBOX() const override;

    //! Creates a deep copy of this shared data
    QgsOapifSharedData *clone() const;

  signals:

    //! Raise error
    void raiseError( const QString &errorMsg ) const;

    //! Extent has been updated
    void extentUpdated();

  protected:
    friend class QgsOapifProvider;
    friend class QgsOapifFeatureDownloaderImpl;
    friend class QgsOapifCreateFeatureRequest;
    friend class QgsOapifPutFeatureRequest;
    friend class QgsOapifPatchFeatureRequest;

    //! Datasource URI
    QgsWFSDataSourceURI mURI;

    //! Geometry type of the features in this layer
    Qgis::WkbType mWKBType = Qgis::WkbType::Unknown;

    //! Page size. 0 = disabled
    long long mPageSize = 0;

    //! Extra query parameters from the URI, to append to other requests
    QString mExtraQueryParameters;

    //! Url to /collections/{collectionId}
    QString mCollectionUrl;

    //! Url to /collections/{collectionId}/items
    QString mItemsUrl;

    //! Media type of feature format requests to /items. May be empty for default
    QString mFeatureFormat;

    //! Server filter
    QString mServerFilter;

    //! Geometry column name
    QString mGeometryColumnName;

    //! Translation state of filter to server-side filter.
    QgsOapifFilterTranslationState mFilterTranslationState = QgsOapifFilterTranslationState::FULLY_CLIENT;

    //! Set if an "id" is present at top level of features
    bool mFoundIdTopLevel = false;

    //! Set if an "id" is present in the "properties" object of features
    bool mFoundIdInProperties = false;

    // Map of simple queryables items (that is as query parameters). The key of the map is a queryable name.
    QMap<QString, QgsOapifApiRequest::SimpleQueryable> mSimpleQueryables;

    //! Whether server supports OGC API Features Part3 with CQL2-Text
    bool mServerSupportsFilterCql2Text = false;

    //! Whether server supports CQL2 advanced-comparison-operators conformance class (LIKE, BETWEEN, IN)
    bool mServerSupportsLikeBetweenIn = false;

    //! Whether server supports CQL2 case-insensitive-comparison conformance class (CASEI function)
    bool mServerSupportsCaseI = false;

    //! Whether server supports CQL2 basic-spatial-functions conformance class (S_INTERSECTS(,BBOX() or POINT()))
    bool mServerSupportsBasicSpatialFunctions = false;

    // Map of queryables items for CQL2 request. The key of the map is a queryable name.
    QMap<QString, QgsOapifQueryablesRequest::Queryable> mQueryables;

    //! Append extra query parameters if needed
    QString appendExtraQueryParameters( const QString &url ) const;

  private:
    // Translate part of an expression to a server-side filter using Part1 features only
    QString compileExpressionNodeUsingPart1( const QgsExpressionNode *node, QgsOapifFilterTranslationState &translationState, QString &untranslatedPart ) const;

    // Translate part of an expression to a server-side filter using Part1 or Part3
    bool computeFilter( const QgsExpression &expr, QgsOapifFilterTranslationState &translationState, QString &serverSideParameters, QString &clientSideFilterExpression ) const;

    //! Log error to QgsMessageLog and raise it to the provider
    void pushError( const QString &errorMsg ) const override;

    void emitExtentUpdated() override { emit extentUpdated(); }

    void invalidateCacheBaseUnderLock() override;

    bool supportsLimitedFeatureCountDownloads() const override { return true; }

    QString layerName() const override { return mURI.typeName(); }

    bool hasServerSideFilter() const override { return false; }

    bool supportsFastFeatureCount() const override { return false; }

    QgsRectangle getExtentFromSingleFeatureRequest() const override { return QgsRectangle(); }

    long long getFeatureCountFromServer() const override { return -1; }
};

#endif // QGSOAPIFPROVIDERSHAREDDATA_H
