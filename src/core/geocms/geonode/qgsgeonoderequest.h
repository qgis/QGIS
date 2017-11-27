/***************************************************************************
    qgsgeonoderequest.h
    ---------------------
    begin                : Jul 2017
    copyright            : (C) 2017 by Muhammad Yarjuna Rohmat, Ismail Sunni
    email                : rohmat at kartoza dot com, ismail at kartoza dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGEONODEREQUEST_H
#define QGSGEONODEREQUEST_H

#include "qgis.h"
#include "qgis_core.h"
#include <QNetworkReply>
#include <QDomDocument>
#include <QObject>
#include <QUuid>

/**
 * \ingroup core
 * \class QgsGeoNodeStyle
 * \brief Encapsulates information about a GeoNode layer style.
 * \since QGIS 3.0
 */
struct CORE_EXPORT QgsGeoNodeStyle
{
#ifdef SIP_RUN
  % TypeHeaderCode
#include <qgsgeonoderequest.h>
  % End
#endif

  //! Unique style ID
  QString id;

  //! Style name
  QString name;

  //! Style title
  QString title;

  //! DOM documenting containing style
  QDomDocument body;

  //! Associated URL
  QString styleUrl;
};

/**
 * \ingroup core
 * \class QgsGeoNodeRequest
 * \brief Request handler for GeoNode servers.
 *
 * QgsGeoNodeRequest handles requesting and parsing service details from a GeoNode
 * server instance, for instance requesting all available layers or layer styles.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsGeoNodeRequest : public QObject
{
    Q_OBJECT
  public:

    /**
     * Service layer details for an individual layer from a GeoNode connection.
     */
    struct ServiceLayerDetail
    {
      //! Unique identifier (generate on the client side, not at the GeoNode server)
      QUuid uuid;
      //! Layer name
      QString name;
      //! Layer type name
      QString typeName;
      //! Layer title
      QString title;
      //! WMS URL for layer
      QString wmsURL;
      //! WFS URL for layer
      QString wfsURL;
      //! XYZ tileserver URL for layer
      QString xyzURL;
    };

    /**
     * Constructor for QgsGeoNodeRequest.
     *
     * If \a forceRefresh is false, then cached copies of the request may be reused.
     */
    QgsGeoNodeRequest( const QString &baseUrl, bool forceRefresh, QObject *parent = nullptr );

    virtual ~QgsGeoNodeRequest();

    /**
     * Triggers a new request to the GeoNode server, with the requested \a endPoint.
     * Any existing request will be aborted.
     *
     * Calling this method does not block while waiting for a result.
     *
     * \warning When using the non-blocking methods in this class, sending
     * overlapping requests results in undefined behavior. Use separate instances
     * of QgsGeoNodeRequest instead to avoid this.
     *
     * \see requestBlocking()
     */
    void request( const QString &endPoint );

    /**
     * Triggers a new request to the GeoNode server, with the requested \a endPoint.
     * Any existing request will be aborted.
     *
     * Calling this method will block while waiting for a result. It should not be
     * used from any code which potentially blocks operation in the main GUI thread.
     *
     * \see request()
     */
    bool requestBlocking( const QString &endPoint );

    /**
     * Triggers a new request to fetch the list of available layers from the
     * server. When complete, the layersFetched() signal will be emitted
     * with the result.
     *
     * This method is non-blocking and returns immediately.
     *
     * \warning When using the non-blocking methods in this class, sending
     * overlapping requests results in undefined behavior. Use separate instances
     * of QgsGeoNodeRequest instead to avoid this.
     *
     * \see layersFetched()
     * \see fetchLayersBlocking()
     */
    void fetchLayers();

    /**
     * Requests the list of available layers from the server.
     *
     * This method is blocking and will wait for results from the server before returning.
     * Accordingly it should not be used from any code which potentially blocks operation in the main GUI thread.
     *
     * \see fetchLayers()
     */
    QList<QgsGeoNodeRequest::ServiceLayerDetail> fetchLayersBlocking();

    /**
     * Requests the list of available styles for the layer
     * with matching \a layerName from the server.
     *
     * This method is blocking and will wait for results from the server before returning.
     * Accordingly it should not be used from any code which potentially blocks operation in the main GUI thread.
     *
     */
    QList<QgsGeoNodeStyle> fetchStylesBlocking( const QString &layerName );

    /**
     * Requests the default style for the layer with matching \a layerName from the server.
     *
     * This method is blocking and will wait for results from the server before returning.
     * Accordingly it should not be used from any code which potentially blocks operation in the main GUI thread.
     */
    QgsGeoNodeStyle fetchDefaultStyleBlocking( const QString &layerName );

    /**
     * Requests the details for the style with matching \a styleId from the server.
     *
     * This method is blocking and will wait for results from the server before returning.
     * Accordingly it should not be used from any code which potentially blocks operation in the main GUI thread.
     */
    QgsGeoNodeStyle fetchStyleBlocking( const QString &styleId );

    /**
     * Requests the list of unique URLs for available services with matching \a serviceType from the server.
     *
     * This method is blocking and will wait for results from the server before returning.
     * Accordingly it should not be used from any code which potentially blocks operation in the main GUI thread.
     */
    QStringList fetchServiceUrlsBlocking( const QString &serviceType );

    /**
     * Obtains a map of layer name to URL for available services with matching \a serviceType from the server.
     *
     * This method is blocking and will wait for results from the server before returning.
     * Accordingly it should not be used from any code which potentially blocks operation in the main GUI thread.
     */
    QgsStringMap fetchServiceUrlDataBlocking( const QString &serviceType );

    /**
     * Returns the most recent error string for any encountered errors, or an empty string if
     * no errors have been encountered.
     */
    QString lastError() const { return mError; }

    /**
     * Returns the most recent response obtained from the server.
     */
    QByteArray lastResponse() const { return mHttpGeoNodeResponse; }

    /**
     * Returns the network protocol (e.g. 'http') used for connecting with the server.
     * \see setProtocol()
     */
    QString protocol() const;

    /**
     * Sets the network \a protocol (e.g. 'http') used for connecting with the server.
     * \see protocol()
     */
    void setProtocol( const QString &protocol );

  public slots:

    /**
     * Aborts any active network request immediately.
     */
    void abort();

  signals:

    /**
     * Emitted when the status of an ongoing request is changed.
     */
    void statusChanged( const QString &statusQString );

    /**
     * Emitted when the existing request has been completed.
     */
    void requestFinished();

    /**
     * Emitted when the result of a fetchLayers call has been received and processed.
     */
    void layersFetched( const QList<QgsGeoNodeRequest::ServiceLayerDetail> &layers );

  private slots:
    void replyFinished();
    void replyProgress( qint64, qint64 );

  private:

    //! URL part of URI (httpuri)
    QString mProtocol;

    //! URL part of URI (httpuri)
    QString mBaseUrl;

    //! The reply to the geonode request
    QNetworkReply *mGeoNodeReply = nullptr;

    //! The error message associated with the last error.
    QString mError;

    //! The mime type of the message
    QString mErrorFormat;

    //! Response
    QByteArray mHttpGeoNodeResponse;

    bool mIsAborted = false;
    bool mForceRefresh = false;

    QList<QgsGeoNodeRequest::ServiceLayerDetail> parseLayers( const QByteArray &layerResponse );
    QgsGeoNodeStyle retrieveStyle( const QString &styleUrl );

    QNetworkReply *requestUrl( const QString &url );

};

#endif // QGSGEONODEREQUEST_H
