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
class QgsAbstractVectorLayerLabeling;
class QgsCoordinateReferenceSystem;
class QgsFeedback;
class QgsSymbol;
class QgsLineSymbol;
class QgsFillSymbol;
class QgsMarkerSymbol;
class QgsFeatureRenderer;

class QgsArcGisRestUtils
{
  public:
    static QVariant::Type mapEsriFieldType( const QString &esriFieldType );
    static QgsWkbTypes::Type mapEsriGeometryType( const QString &esriGeometryType );
    static std::unique_ptr< QgsAbstractGeometry > parseEsriGeoJSON( const QVariantMap &geometryData, const QString &esriGeometryType, bool readM, bool readZ, QgsCoordinateReferenceSystem *crs = nullptr );
    static QgsCoordinateReferenceSystem parseSpatialReference( const QVariantMap &spatialReferenceMap );

    static QVariantMap getServiceInfo( const QString &baseurl, const QString &authcfg, QString &errorTitle, QString &errorText, const QgsStringMap &requestHeaders = QgsStringMap() );
    static QVariantMap getLayerInfo( const QString &layerurl, const QString &authcfg, QString &errorTitle, QString &errorText, const QgsStringMap &requestHeaders = QgsStringMap() );
    static QVariantMap getObjectIds( const QString &layerurl, const QString &authcfg, const QString &objectIdFieldName, QString &errorTitle, QString &errorText, const QgsStringMap &requestHeaders = QgsStringMap(),
                                     const QgsRectangle &bbox = QgsRectangle() );
    static QVariantMap getObjects( const QString &layerurl, const QString &authcfg, const QList<quint32> &objectIds, const QString &crs,
                                   bool fetchGeometry, const QStringList &fetchAttributes, bool fetchM, bool fetchZ,
                                   const QgsRectangle &filterRect, QString &errorTitle, QString &errorText, const QgsStringMap &requestHeaders = QgsStringMap(), QgsFeedback *feedback = nullptr );
    static QList<quint32> getObjectIdsByExtent( const QString &layerurl, const QString &objectIdField, const QgsRectangle &filterRect, QString &errorTitle, QString &errorText, const QString &authcfg, const QgsStringMap &requestHeaders = QgsStringMap(), QgsFeedback *feedback = nullptr );
    static QByteArray queryService( const QUrl &url, const QString &authcfg, QString &errorTitle, QString &errorText, const QgsStringMap &requestHeaders = QgsStringMap(), QgsFeedback *feedback = nullptr );
    static QVariantMap queryServiceJSON( const QUrl &url, const QString &authcfg, QString &errorTitle, QString &errorText, const QgsStringMap &requestHeaders = QgsStringMap(), QgsFeedback *feedback = nullptr );

    static std::unique_ptr< QgsSymbol > parseEsriSymbolJson( const QVariantMap &symbolData );
    static std::unique_ptr< QgsLineSymbol > parseEsriLineSymbolJson( const QVariantMap &symbolData );
    static std::unique_ptr< QgsFillSymbol > parseEsriFillSymbolJson( const QVariantMap &symbolData );
    static std::unique_ptr< QgsFillSymbol > parseEsriPictureFillSymbolJson( const QVariantMap &symbolData );
    static std::unique_ptr< QgsMarkerSymbol > parseEsriMarkerSymbolJson( const QVariantMap &symbolData );
    static std::unique_ptr< QgsMarkerSymbol > parseEsriPictureMarkerSymbolJson( const QVariantMap &symbolData );
    static QgsFeatureRenderer *parseEsriRenderer( const QVariantMap &rendererData );
    static QgsAbstractVectorLayerLabeling *parseEsriLabeling( const QVariantList &labelingData );

    static QString parseEsriLabelingExpression( const QString &string );
    static QColor parseEsriColorJson( const QVariant &colorData );
    static Qt::PenStyle parseEsriLineStyle( const QString &style );
    static Qt::BrushStyle parseEsriFillStyle( const QString &style );

    static QDateTime parseDateTime( const QVariant &value );

    static QUrl parseUrl( const QUrl &url );
    static void adjustBaseUrl( QString &baseUrl, const QString name );
    static void visitFolderItems( const std::function<void ( const QString &folderName, const QString &url )> &visitor, const QVariantMap &serviceData, const QString &baseUrl );
    static void visitServiceItems( const std::function<void ( const QString &serviceName, const QString &url )> &visitor, const QVariantMap &serviceData, const QString &baseUrl );
    static void addLayerItems( const std::function<void ( const QString &parentLayerId, const QString &layerId, const QString &name, const QString &description, const QString &url, bool isParentLayer, const QString &authid )> &visitor, const QVariantMap &serviceData, const QString &parentUrl );
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
