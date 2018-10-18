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
#include "qgswkbtypes.h"
#include "qgsrectangle.h"

class QNetworkReply;
class QgsNetworkAccessManager;
class QgsFields;
class QgsAbstractGeometry;
class QgsCoordinateReferenceSystem;
class QgsFeedback;
class QgsSymbol;
class QgsLineSymbol;
class QgsFillSymbol;
class QgsMarkerSymbol;
class QgsFeatureRenderer;

/**
 * Manages token storage and retrieval and ArcGIS rest servers.
 */
class QgsArcGisRestTokenManager
{

  public:

    /**
     * Sets the \a token required to connect to servers with a matching prefix. More explicit server prefixes
     * take precedence over less explicit prefixes, so a token for "http://myserver.domain.com/portal/" would take precendence
     * over a token set for "http://myserver.domain.com".
     *
     * Any existing tokens stored for the same prefix will be removed.
     *
     * \see setTokens()
     * \see tokens()
     */
    static void setToken( const QString &serverPrefix, const QString &token );

    /**
     * Returns the current map of server prefix to tokens.
     * \see setTokens()
     * \see setToken()
     */
    static QMap< QString, QString > tokens();

    /**
     * Sets the map of server prefix to token, replacing all existing tokens stored in the manager.
     *
     * \see tokens()
     */
    static void setTokens( const QMap< QString, QString> &tokens );

    /**
     * Retrieves the token required to connect to the given \a url, or an
     * empty string if no token is required.
     */
    static QString token( const QString &url );

  private:

    static QString cleanServerName( const QString &server );

};

class QgsArcGisRestUtils
{
  public:
    static QVariant::Type mapEsriFieldType( const QString &esriFieldType );
    static QgsWkbTypes::Type mapEsriGeometryType( const QString &esriGeometryType );
    static std::unique_ptr< QgsAbstractGeometry > parseEsriGeoJSON( const QVariantMap &geometryData, const QString &esriGeometryType, bool readM, bool readZ, QgsCoordinateReferenceSystem *crs = nullptr );
    static QgsCoordinateReferenceSystem parseSpatialReference( const QVariantMap &spatialReferenceMap );

    static QVariantMap getServiceInfo( const QString &baseurl, const QString &token, QString &errorTitle, QString &errorText );
    static QVariantMap getLayerInfo( const QString &layerurl, const QString &token, QString &errorTitle, QString &errorText );
    static QVariantMap getObjectIds( const QString &layerurl, const QString &token, const QString &objectIdFieldName, QString &errorTitle, QString &errorText,
                                     const QgsRectangle &bbox = QgsRectangle() );
    static QVariantMap getObjects( const QString &layerurl, const QString &token, const QList<quint32> &objectIds, const QString &crs,
                                   bool fetchGeometry, const QStringList &fetchAttributes, bool fetchM, bool fetchZ,
                                   const QgsRectangle &filterRect, QString &errorTitle, QString &errorText, QgsFeedback *feedback = nullptr );
    static QList<quint32> getObjectIdsByExtent( const QString &layerurl, const QString &objectIdField, const QgsRectangle &filterRect, QString &errorTitle, QString &errorText, const QString &token, QgsFeedback *feedback = nullptr );
    static QByteArray queryService( const QUrl &url, const QString &token, QString &errorTitle, QString &errorText, QgsFeedback *feedback = nullptr );
    static QVariantMap queryServiceJSON( const QUrl &url, const QString &token, QString &errorTitle, QString &errorText, QgsFeedback *feedback = nullptr );

    static std::unique_ptr< QgsSymbol > parseEsriSymbolJson( const QVariantMap &symbolData );
    static std::unique_ptr< QgsLineSymbol > parseEsriLineSymbolJson( const QVariantMap &symbolData );
    static std::unique_ptr< QgsFillSymbol > parseEsriFillSymbolJson( const QVariantMap &symbolData );
    static std::unique_ptr< QgsMarkerSymbol > parseEsriMarkerSymbolJson( const QVariantMap &symbolData );
    static QgsFeatureRenderer *parseEsriRenderer( const QVariantMap &rendererData );

    static QColor parseEsriColorJson( const QVariant &colorData );
    static Qt::PenStyle parseEsriLineStyle( const QString &style );
    static Qt::BrushStyle parseEsriFillStyle( const QString &style );

    static QDateTime parseDateTime( const QVariant &value );

    static QUrl parseUrl( const QUrl &url );
};

class QgsArcGisAsyncQuery : public QObject
{
    Q_OBJECT
  public:
    QgsArcGisAsyncQuery( QObject *parent = nullptr );
    ~QgsArcGisAsyncQuery() override;

    void start( const QUrl &url, QByteArray *result, bool allowCache = false );
  signals:
    void finished();
    void failed( QString errorTitle, QString errorName );
  private slots:
    void handleReply();

  private:
    QNetworkReply *mReply = nullptr;
    QByteArray *mResult = nullptr;
};

class QgsArcGisAsyncParallelQuery : public QObject
{
    Q_OBJECT
  public:
    QgsArcGisAsyncParallelQuery( QObject *parent = nullptr );
    void start( const QVector<QUrl> &urls, QVector<QByteArray> *results, bool allowCache = false );
  signals:
    void finished( QStringList errors );
  private slots:
    void handleReply();

  private:
    QVector<QByteArray> *mResults = nullptr;
    int mPendingRequests = 0;
    QStringList mErrors;
};

#endif // QGSARCGISRESTUTILS_H
