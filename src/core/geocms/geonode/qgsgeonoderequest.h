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
#include <qnetworkreply.h>
#include <QDomDocument>


#include <QObject>
#include <QUuid>

struct CORE_EXPORT QgsServiceLayerDetail
{
#ifdef SIP_RUN
  % TypeHeaderCode
#include <qgsgeonoderequest.h>
  % End
#endif
  QUuid uuid;
  QString name;
  QString typeName;
  QString title;
  QString wmsURL;
  QString wfsURL;
  QString xyzURL;
};

struct CORE_EXPORT QgsGeoNodeStyle
{
#ifdef SIP_RUN
  % TypeHeaderCode
#include <qgsgeonoderequest.h>
  % End
#endif
  QString id;
  QString name;
  QString title;
  QDomDocument body;
  QString styleUrl;
};

class CORE_EXPORT QgsGeoNodeRequest : public QObject
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsGeoNodeRequest.
     *
     * If \a forceRefresh is false, then cached copies of the request may be reused.
     */
    explicit QgsGeoNodeRequest( bool forceRefresh, QObject *parent = nullptr );
    QgsGeoNodeRequest( const QString &baseUrl, bool forceRefresh, QObject *parent = nullptr );
    virtual ~QgsGeoNodeRequest();

    bool request( const QString &endPoint );

    QList<QgsServiceLayerDetail> getLayers();

    QList<QgsGeoNodeStyle> getStyles( const QString &layerName );

    QgsGeoNodeStyle getDefaultStyle( const QString &layerName );

    QgsGeoNodeStyle getStyle( const QString &styleID );

    //! Obtain list of unique URLs in the geonode
    QStringList serviceUrls( const QString &serviceType );

    //! Obtain map of layer name and url for a service type
    QgsStringMap serviceUrlData( const QString &serviceType );

    QString lastError() const { return mError; }

    QByteArray response() const { return mHttpGeoNodeResponse; }

    QNetworkReply *reply() const { return mGeoNodeReply; }

    //! Abort network request immediately
    void abort();

    QString getProtocol() const;
    void setProtocol( const QString &protocol );

  private:
    QList<QgsServiceLayerDetail> parseLayers( const QByteArray &layerResponse );
    QgsGeoNodeStyle retrieveStyle( const QString &styleUrl );

  signals:
    //! \brief emit a signal to be caught by qgisapp and display a statusQString on status bar
    void statusChanged( const QString &statusQString );

    //! \brief emit a signal once the request is finished
    void requestFinished();

  protected slots:
    void replyFinished();
    void replyProgress( qint64, qint64 );

  protected:

    //! URL part of URI (httpuri)
    QString mProtocol;

    //! URL part of URI (httpuri)
    QString mBaseUrl;

//  QgsWmsAuthorization mAuth;

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

};

#endif // QGSGEONODEREQUEST_H
