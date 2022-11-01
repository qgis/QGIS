/***************************************************************************
    qgsappgpsdigitizing.h
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

#ifndef QGSAPPGPSDIGITIZING_H
#define QGSAPPGPSDIGITIZING_H

#include <QObject>

#include "info.h"
#include "nmeatime.h"
#include "qgspointxy.h"
#include "qgscoordinatetransform.h"
#include "qgsdistancearea.h"
#include "qgis_app.h"

class QgsAppGpsConnection;
class QgsMapCanvas;
class QgsRubberBand;
class QgsPoint;
class QgsGpsInformation;
class QgsVectorLayer;
class QTimer;

class APP_EXPORT QgsAppGpsDigitizing: public QObject
{
    Q_OBJECT

  public:

    QgsAppGpsDigitizing( QgsAppGpsConnection *connection, QgsMapCanvas *canvas, QObject *parent = nullptr );
    ~QgsAppGpsDigitizing() override;

  public slots:
    void addVertex();
    void resetFeature();
    void addFeature();

    void setAutoAddVertices( bool enabled );
    void setAutoSaveFeature( bool enabled );

    void setTimeStampDestination( const QString &fieldName );

  signals:

    void timeStampDestinationChanged( const QString &fieldName );

  private slots:
    void gpsSettingsChanged();
    void updateTrackAppearance();
    void switchAcquisition();

    void gpsStateChanged( const QgsGpsInformation &info );

    /**
     * Updates compatible fields for timestamp recording
     */
    void updateTimestampDestinationFields( QgsMapLayer *mapLayer );

  private:
    void createRubberBand();
    QVariant timestamp( QgsVectorLayer *vlayer, int idx );

    QgsAppGpsConnection *mConnection = nullptr;
    QgsMapCanvas *mCanvas = nullptr;

    bool mAutoAddVertices = false;
    bool mAutoSave = false;

    QgsPointXY mLastGpsPosition;

    QgsRubberBand *mRubberBand = nullptr;

    QVector<QgsPoint> mCaptureList;
    double mLastElevation = 0.0;

    nmeaPOS mLastNmeaPosition;
    nmeaTIME mLastNmeaTime;

    QgsCoordinateReferenceSystem mWgs84CRS;
    QgsDistanceArea mDistanceCalculator;
    QgsCoordinateTransform mCanvasToWgs84Transform;

    int mBlockGpsStateChanged = 0;

    std::unique_ptr<QTimer> mAcquisitionTimer;
    bool mAcquisitionEnabled = true;
    int mAcquisitionInterval = 0;
    double mDistanceThreshold = 0;

    bool mApplyLeapSettings = false;
    int mLeapSeconds = 0;
    Qt::TimeSpec mTimeStampSpec = Qt::TimeSpec::LocalTime;
    QString mTimeZone;

    //! Temporary storage of preferred fields
    QMap<QString, QString> mPreferredTimestampFields;
    QString mTimestampField;

    friend class TestQgsGpsIntegration;
};

#endif // QGSAPPGPSDIGITIZING
