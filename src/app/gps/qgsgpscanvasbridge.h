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

class QgsAppGpsConnection;
class QgsMapCanvas;
class QgsGpsMarker;
class QgsGpsBearingItem;
class QgsGpsInformation;

class QgsGpsCanvasBridge : public QObject
{
    Q_OBJECT

  public:

    QgsGpsCanvasBridge( QgsAppGpsConnection *connection, QgsMapCanvas *canvas, QObject *parent = nullptr );
    ~QgsGpsCanvasBridge() override;

public slots:

    void showBearingLine( bool show );

private slots:
        void updateBearingAppearance();
    void gpsSettingsChanged();
    void gpsDisconnected();

        void gpsStateChanged( const QgsGpsInformation &info );

  private:

    QgsAppGpsConnection *mConnection = nullptr;
    QgsMapCanvas *mCanvas = nullptr;

    QgsGpsMarker *mMapMarker = nullptr;
    QgsGpsBearingItem *mMapBearingItem = nullptr;

    QgsPointXY mLastGpsPosition;
    QgsPointXY mSecondLastGpsPosition;

        QElapsedTimer mLastRotateTimer;

    bool mBearingFromTravelDirection = false;
    int mMapExtentMultiplier = 50;
    int mMapRotateInterval = 0;

        QgsCoordinateReferenceSystem mWgs84CRS;
};

#endif // QGSGPSCANVASBRIDGE_H
