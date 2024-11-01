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
#include "qgis_app.h"

class QgsAppGpsConnection;
class QgsMapCanvas;
class QgsGpsMarker;
class QgsGpsBearingItem;
class QgsGpsInformation;
class QgsBearingNumericFormat;

class QgsSettingsEntryBool;
class QgsSettingsEntryInteger;
class QgsSettingsEntryString;
template<class T> class QgsSettingsEntryEnumFlag;

class QTapAndHoldGesture;

class APP_EXPORT QgsGpsCanvasBridge : public QObject, public QgsMapCanvasInteractionBlocker
{
    Q_OBJECT

  public:
    static const QgsSettingsEntryBool *settingShowBearingLine;
    static const QgsSettingsEntryString *settingBearingLineSymbol;
    static const QgsSettingsEntryInteger *settingMapExtentRecenteringThreshold;
    static const QgsSettingsEntryEnumFlag<Qgis::MapRecenteringMode> *settingMapCenteringMode;
    static const QgsSettingsEntryBool *settingRotateMap;
    static const QgsSettingsEntryInteger *settingMapRotateInterval;

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

    std::unique_ptr<QgsBearingNumericFormat> mBearingNumericFormat;
};

#endif // QGSGPSCANVASBRIDGE_H
