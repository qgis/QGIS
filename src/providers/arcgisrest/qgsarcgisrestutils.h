/***************************************************************************
    qgsarcgisrestutils.h
    --------------------
    begin                : Nov 25, 2015
    copyright            : (C) 2015 by Sandro Mani
    email                : manisandro@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSARCGISRESTUTILS_H
#define QGSARCGISRESTUTILS_H

#include <QStringList>
#include <QVariant>
#include "geometry/qgswkbtypes.h"

class QNetworkReply;
class QgsNetworkAccessManager;
class QgsFields;
class QgsRectangle;
class QgsAbstractGeometryV2;
class QgsCoordinateReferenceSystem;

class QgsArcGisRestUtils
{
  public:
    static QVariant::Type mapEsriFieldType( const QString& esriFieldType );
    static QgsWKBTypes::Type mapEsriGeometryType( const QString& esriGeometryType );
    static QgsAbstractGeometryV2* parseEsriGeoJSON( const QVariantMap& geometryData, const QString& esriGeometryType, bool readM, bool readZ, QgsCoordinateReferenceSystem *crs = 0 );
    static QgsCoordinateReferenceSystem parseSpatialReference( const QVariantMap& spatialReferenceMap );

    static QVariantMap getServiceInfo( const QString& baseurl, QString &errorTitle, QString &errorText );
    static QVariantMap getLayerInfo( const QString& layerurl, QString &errorTitle, QString &errorText );
    static QVariantMap getObjectIds( const QString& layerurl, QString &errorTitle, QString &errorText );
    static QVariantMap getObjects( const QString& layerurl, const QList<quint32> &objectIds, const QString& crs,
                                   bool fetchGeometry, const QStringList &fetchAttributes, bool fetchM, bool fetchZ,
                                   const QgsRectangle& filterRect , QString &errorTitle, QString &errorText );
    static QByteArray queryService( const QUrl& url, QString &errorTitle, QString &errorText );
    static QVariantMap queryServiceJSON( const QUrl& url, QString &errorTitle, QString &errorText );
};

class QgsArcGisAsyncQuery : public QObject
{
    Q_OBJECT
  public:
    QgsArcGisAsyncQuery( QObject* parent = nullptr );
    ~QgsArcGisAsyncQuery();

    void start( const QUrl& url, QByteArray* result, bool allowCache = false );
  signals:
    void finished();
    void failed( QString errorTitle, QString errorName );
  private slots:
    void handleReply();

  private:
    QNetworkReply* mReply;
    QByteArray* mResult;
};

class QgsArcGisAsyncParallelQuery : public QObject
{
    Q_OBJECT
  public:
    QgsArcGisAsyncParallelQuery( QObject* parent = nullptr );
    void start( const QVector<QUrl>& urls, QVector<QByteArray>* results, bool allowCache = false );
  signals:
    void finished( QStringList errors );
  private slots:
    void handleReply();

  private:
    QVector<QByteArray>* mResults;
    int mPendingRequests;
    QStringList mErrors;
};

#endif // QGSARCGISRESTUTILS_H
