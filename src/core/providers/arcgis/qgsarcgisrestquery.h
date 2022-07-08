/***************************************************************************
    qgsarcgisrestquery.h
    --------------------
    begin                : December 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSARCGISRESTQUERY_H
#define QGSARCGISRESTQUERY_H

#define SIP_NO_FILE

#include "qgis_core.h"
#include "qgsrectangle.h"
#include "qgswkbtypes.h"
#include "qgshttpheaders.h"

#include <QString>
#include <QVariantMap>

class QgsFeedback;
class QNetworkReply;

/**
 * \ingroup core
 * \brief Utility functions for querying ArcGIS REST services.
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsArcGisRestQueryUtils
{
  public:

    /**
     * Service types
     */
    enum ServiceTypeFilter
    {
      AllTypes, //!< All types
      Vector,   //!< Vector type
      Raster   //!< Raster type
    };

    /**
     * Retrieves JSON service info for the specified base URL.
     */
    static QVariantMap getServiceInfo( const QString &baseurl, const QString &authcfg, QString &errorTitle, QString &errorText, const QgsHttpHeaders &requestHeaders = QgsHttpHeaders() );

    /**
     * Retrieves JSON layer info for the specified layer URL.
     */
    static QVariantMap getLayerInfo( const QString &layerurl, const QString &authcfg, QString &errorTitle, QString &errorText, const QgsHttpHeaders &requestHeaders = QgsHttpHeaders() );

    /**
     * Retrieves all object IDs for the specified layer URL.
     */
    static QVariantMap getObjectIds( const QString &layerurl, const QString &authcfg, QString &errorTitle, QString &errorText, const QgsHttpHeaders &requestHeaders = QgsHttpHeaders(),
                                     const QgsRectangle &bbox = QgsRectangle() );

    /**
     * Retrieves all matching objects from the specified layer URL.
     */
    static QVariantMap getObjects( const QString &layerurl, const QString &authcfg, const QList<quint32> &objectIds, const QString &crs,
                                   bool fetchGeometry, const QStringList &fetchAttributes, bool fetchM, bool fetchZ,
                                   const QgsRectangle &filterRect, QString &errorTitle, QString &errorText, const QgsHttpHeaders &requestHeaders = QgsHttpHeaders(), QgsFeedback *feedback = nullptr );

    /**
     * Gets a list of object IDs which fall within the specified extent.
     */
    static QList<quint32> getObjectIdsByExtent( const QString &layerurl, const QgsRectangle &filterRect, QString &errorTitle, QString &errorText, const QString &authcfg, const QgsHttpHeaders &requestHeaders = QgsHttpHeaders(), QgsFeedback *feedback = nullptr );

    /**
     * Performs a blocking request to a URL and returns the retrieved data.
     */
    static QByteArray queryService( const QUrl &url, const QString &authcfg, QString &errorTitle, QString &errorText, const QgsHttpHeaders &requestHeaders = QgsHttpHeaders(), QgsFeedback *feedback = nullptr, QString *contentType = nullptr );

    /**
     * Performs a blocking request to a URL and returns the retrieved JSON content.
     */
    static QVariantMap queryServiceJSON( const QUrl &url, const QString &authcfg, QString &errorTitle, QString &errorText, const QgsHttpHeaders &requestHeaders = QgsHttpHeaders(), QgsFeedback *feedback = nullptr );

    /**
     * Calls the specified \a visitor function on all folder items found within the given service data.
     */
    static void visitFolderItems( const std::function<void ( const QString &folderName, const QString &url )> &visitor, const QVariantMap &serviceData, const QString &baseUrl );

    /**
     * Calls the specified \a visitor function on all service items found within the given service data.
     */
    static void visitServiceItems( const std::function<void ( const QString &serviceName, const QString &url, const QString &service, ServiceTypeFilter serviceType )> &visitor, const QVariantMap &serviceData, const QString &baseUrl );

    /**
     * Calls the specified \a visitor function on all layer items found within the given service data.
     */
    static void addLayerItems( const std::function<void ( const QString &parentLayerId, ServiceTypeFilter serviceType, QgsWkbTypes::GeometryType geometryType, const QString &layerId, const QString &name, const QString &description, const QString &url, bool isParentLayer, const QString &authid, const QString &format )> &visitor, const QVariantMap &serviceData, const QString &parentUrl, const QString &parentSupportedFormats, const ServiceTypeFilter filter = AllTypes );

  private:

    static QUrl parseUrl( const QUrl &url );
    static void adjustBaseUrl( QString &baseUrl, const QString &name );

    friend class TestQgsArcGisRestUtils;
};

///@cond PRIVATE
class CORE_EXPORT QgsArcGisAsyncQuery : public QObject
{
    Q_OBJECT
  public:
    QgsArcGisAsyncQuery( QObject *parent = nullptr );
    ~QgsArcGisAsyncQuery() override;

    void start( const QUrl &url, const QString &authCfg, QByteArray *result, bool allowCache = false, const QgsHttpHeaders &headers = QgsHttpHeaders() );
  signals:
    void finished();
    void failed( QString errorTitle, QString errorName );
  private slots:
    void handleReply();

  private:
    QNetworkReply *mReply = nullptr;
    QByteArray *mResult = nullptr;
};

class CORE_EXPORT QgsArcGisAsyncParallelQuery : public QObject
{
    Q_OBJECT
  public:
    QgsArcGisAsyncParallelQuery( const QString &authcfg, const QgsHttpHeaders &requestHeaders, QObject *parent = nullptr );
    void start( const QVector<QUrl> &urls, QVector<QByteArray> *results, bool allowCache = false );

  signals:
    void finished( QStringList errors );
  private slots:
    void handleReply();

  private:
    QVector<QByteArray> *mResults = nullptr;
    int mPendingRequests = 0;
    QStringList mErrors;
    QString mAuthCfg;
    QgsHttpHeaders mRequestHeaders;
};

///@endcond

#endif // QGSARCGISRESTQUERY_H
