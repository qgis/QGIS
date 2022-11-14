/***************************************************************************
    qgsgpscanvasbridge.h
    -------------------
    begin                : October 2022
    copyright            : (C) 2022 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGPSCANVASBRIDGE_H
#define QGSGPSCANVASBRIDGE_H

#include <QObject>
#include <QElapsedTimer>
#include "qgspointxy.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsdistancearea.h"
#include "qgsmapcanvasinteractionblocker.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingsentryenumflag.h"
#include "qgis_app.h"

class QgsAppGpsConnection;
class QgsMapCanvas;
class QgsGpsMarker;
class QgsGpsBearingItem;
class QgsGpsInformation;
class QgsBearingNumericFormat;

class QTapAndHoldGesture;

class APP_EXPORT QgsGpsCanvasBridge : public QObject, public QgsMapCanvasInteractionBlocker
{
    Q_OBJECT

  public:

    static const inline QgsSettingsEntryBool settingShowBearingLine = QgsSettingsEntryBool( QStringLiteral( "show-bearing-line" ), QgsSettings::Prefix::GPS, false, QStringLiteral( "Whether the GPS bearing line symbol should be shown" ) );
    static const inline QgsSettingsEntryString settingBearingLineSymbol = QgsSettingsEntryString( QStringLiteral( "bearing-line-symbol" ), QgsSettings::Prefix::GPS, QString(), QStringLiteral( "Line symbol to use for GPS bearing line" ), Qgis::SettingsOptions(), 0 );
    static const inline QgsSettingsEntryInteger settingMapExtentRecenteringThreshold = QgsSettingsEntryInteger( QStringLiteral( "map-recentering-threshold" ), QgsSettings::Prefix::GPS, 50, QStringLiteral( "Threshold for GPS automatic map centering" ) );
    static const inline QgsSettingsEntryEnumFlag<Qgis::MapRecenteringMode> settingMapCenteringMode = QgsSettingsEntryEnumFlag<Qgis::MapRecenteringMode>( QStringLiteral( "map-recentering" ), QgsSettings::Prefix::GPS, Qgis::MapRecenteringMode::WhenOutsideVisibleExtent, QStringLiteral( "Automatic GPS based map recentering mode" ) );
    static const inline QgsSettingsEntryBool settingRotateMap = QgsSettingsEntryBool( QStringLiteral( "auto-map-rotate" ), QgsSettings::Prefix::GPS, false, QStringLiteral( "Whether to automatically rotate the map to match GPS bearing" ) );
    static const inline QgsSettingsEntryInteger settingMapRotateInterval = QgsSettingsEntryInteger( QStringLiteral( "map-rotate-interval" ), QgsSettings::Prefix::GPS, 0, QStringLiteral( "Interval for GPS automatic map rotation" ) );

    QgsGpsCanvasBridge( QgsAppGpsConnection *connection, QgsMapCanvas *canvas, QObject *parent = nullptr );
    ~QgsGpsCanvasBridge() override;

    bool blockCanvasInteraction( Interaction interaction ) const override;

  public slots:

    void setLocationMarkerVisible( bool show );
    void setBearingLineVisible( bool show );
    void setRotateMap( bool enabled );
    void setMapCenteringMode( Qgis::MapRecenteringMode mode );
    void tapAndHold( const QgsPointXY &mapPoint, QTapAndHoldGesture *gesture );

  private slots:
    void updateBearingAppearance();
    void gpsSettingsChanged();
    void gpsDisconnected();

    void gpsStateChanged( const QgsGpsInformation &info );

    void cursorCoordinateChanged( const QgsPointXY &point );
    void updateGpsDistanceStatusMessage( bool forceDisplay );

  private:

    QgsAppGpsConnection *mConnection = nullptr;
    QgsMapCanvas *mCanvas = nullptr;

    bool mShowMarker = false;
    QgsGpsMarker *mMapMarker = nullptr;

    bool mShowBearingLine = false;
    QgsGpsBearingItem *mMapBearingItem = nullptr;

    QgsPointXY mLastGpsPosition;
    QgsPointXY mSecondLastGpsPosition;

    bool mRotateMap = false;

    Qgis::MapRecenteringMode mCenteringMode = Qgis::MapRecenteringMode::Always;

    QgsDistanceArea mDistanceCalculator;
    QgsCoordinateTransform mCanvasToWgs84Transform;
    QElapsedTimer mLastRotateTimer;

    bool mBearingFromTravelDirection = false;
    int mMapExtentMultiplier = 50;
    int mMapRotateInterval = 0;

    QgsCoordinateReferenceSystem mWgs84CRS;
    QgsPointXY mLastCursorPosWgs84;

    QElapsedTimer mLastForcedStatusUpdate;

    std::unique_ptr< QgsBearingNumericFormat > mBearingNumericFormat;
};

#endif // QGSGPSCANVASBRIDGE_H
