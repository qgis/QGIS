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
#include "qgssettingsentryimpl.h"

#include <QTextStream>

class QgsAppGpsConnection;
class QgsMapCanvas;
class QgsRubberBand;
class QgsPoint;
class QgsGpsInformation;
class QgsVectorLayer;
class QTimer;
class QFile;

class APP_EXPORT QgsAppGpsDigitizing: public QObject
{
    Q_OBJECT

  public:

    static const inline QgsSettingsEntryString settingTrackLineSymbol = QgsSettingsEntryString( QStringLiteral( "track-line-symbol" ), QgsSettings::Prefix::GPS, QStringLiteral( "<symbol alpha=\"1\" name=\"gps-track-symbol\" force_rhr=\"0\" clip_to_extent=\"1\" type=\"line\"><layer enabled=\"1\" pass=\"0\" locked=\"0\" class=\"SimpleLine\"><Option type=\"Map\"><Option name=\"line_color\" type=\"QString\" value=\"219,30,42,255\"/><Option name=\"line_style\" type=\"QString\" value=\"solid\"/><Option name=\"line_width\" type=\"QString\" value=\"0.4\"/></Option></layer></symbol>" ), QStringLiteral( "Line symbol to use for GPS track line" ), Qgis::SettingsOptions(), 0 );

    static const inline QgsSettingsEntryString settingLastLogFolder = QgsSettingsEntryString( QStringLiteral( "last-log-folder" ), QgsSettings::Prefix::GPS, QString(), QStringLiteral( "Last used folder for GPS log files" ) );

    QgsAppGpsDigitizing( QgsAppGpsConnection *connection, QgsMapCanvas *canvas, QObject *parent = nullptr );
    ~QgsAppGpsDigitizing() override;

  public slots:
    void addVertex();
    void resetFeature();
    void addFeature();

    void setTimeStampDestination( const QString &fieldName );

    void setNmeaLogFile( const QString &filename );
    void setNmeaLoggingEnabled( bool enabled );

  signals:

    /**
     * Emitted whenever the current track changes from being empty to non-empty or vice versa.
     */
    void trackIsEmptyChanged( bool isEmpty );

    void timeStampDestinationChanged( const QString &fieldName );

  private slots:
    void gpsSettingsChanged();
    void updateTrackAppearance();
    void switchAcquisition();

    void gpsConnected();
    void gpsDisconnected();

    void gpsStateChanged( const QgsGpsInformation &info );

    /**
     * Updates compatible fields for timestamp recording
     */
    void updateTimestampDestinationFields( QgsVectorLayer *mapLayer );
    void logNmeaSentence( const QString &nmeaString ); // added to handle 'raw' data

    void startLogging();
    void stopLogging();

  private:
    void createRubberBand();
    QVariant timestamp( QgsVectorLayer *vlayer, int idx );

    QgsAppGpsConnection *mConnection = nullptr;
    QgsMapCanvas *mCanvas = nullptr;

    QgsPointXY mLastGpsPositionWgs84;

    QgsRubberBand *mRubberBand = nullptr;

    QVector<QgsPoint> mCaptureListWgs84;
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

    QString mNmeaLogFile;
    bool mEnableNmeaLogging = false;

    std::unique_ptr< QFile > mLogFile;
    QTextStream mLogFileTextStream;

    friend class TestQgsGpsIntegration;
};

#endif // QGSAPPGPSDIGITIZING
