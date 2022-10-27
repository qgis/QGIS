/***************************************************************************
               qgsgpsinformationwidget.h  -  description
                             -------------------
    begin                : Sat Jan 01 2010
    copyright            : (C) 2010 by Tim Sutton and Marco Hugentobler
    email                : tim@linfiniti.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGPSINFORMATIONWIDGET_H
#define QGSGPSINFORMATIONWIDGET_H

#include "ui_qgsgpsinformationwidgetbase.h"
#include "qgis_app.h"
#include "info.h"
#include "nmeatime.h"
#include "qgsgpsmarker.h"
#include "qgsmaptoolcapture.h"
#include "qgspanelwidget.h"
#include "qgsmapcanvasinteractionblocker.h"
#include "qgsdistancearea.h"
#include "qgssettingsentryimpl.h"

#include <qwt_plot_curve.h>
#ifdef WITH_QWTPOLAR
#include <qwt_polar_plot.h>
#include <qwt_polar_grid.h>
#include <qwt_polar_marker.h>
#endif
#include <QTextStream>

class QextSerialPort;
class QgsAppGpsConnection;
class QgsGpsConnection;
class QgsGpsTrackerThread;
class QgsGpsInformation;
class QgsMapCanvas;
class QgsFeature;
class QgsGpsBearingItem;
class QgsBearingNumericFormat;

class QFile;
class QColor;

/**
 * A dock widget that displays information from a GPS device and
 * allows the user to capture features using gps readings to
 * specify the geometry.
*/
class APP_EXPORT QgsGpsInformationWidget: public QgsPanelWidget, public QgsMapCanvasInteractionBlocker, private Ui::QgsGpsInformationWidgetBase
{
    Q_OBJECT
  public:

    static const inline QgsSettingsEntryString settingLastLogFolder = QgsSettingsEntryString( QStringLiteral( "last-log-folder" ), QgsSettings::Prefix::GPS, QString(), QStringLiteral( "Last used folder for GPS log files" ) );
    static const inline QgsSettingsEntryString settingTrackLineSymbol = QgsSettingsEntryString( QStringLiteral( "track-line-symbol" ), QgsSettings::Prefix::GPS, QStringLiteral( "<symbol alpha=\"1\" name=\"gps-track-symbol\" force_rhr=\"0\" clip_to_extent=\"1\" type=\"line\"><layer enabled=\"1\" pass=\"0\" locked=\"0\" class=\"SimpleLine\"><Option type=\"Map\"><Option name=\"line_color\" type=\"QString\" value=\"219,30,42,255\"/><Option name=\"line_style\" type=\"QString\" value=\"solid\"/><Option name=\"line_width\" type=\"QString\" value=\"0.4\"/></Option></layer></symbol>" ), QStringLiteral( "Line symbol to use for GPS track line" ), Qgis::SettingsOptions(), 0 );
    static const inline QgsSettingsEntryString settingBearingLineSymbol = QgsSettingsEntryString( QStringLiteral( "bearing-line-symbol" ), QgsSettings::Prefix::GPS, QString(), QStringLiteral( "Line symbol to use for GPS bearing line" ), Qgis::SettingsOptions(), 0 );
    static const inline QgsSettingsEntryInteger settingMapExtentRecenteringThreshold = QgsSettingsEntryInteger( QStringLiteral( "map-recentering-threshold" ), QgsSettings::Prefix::GPS, 50, QStringLiteral( "Threshold for GPS automatic map centering" ) );
    static const inline QgsSettingsEntryInteger settingMapRotateInterval = QgsSettingsEntryInteger( QStringLiteral( "map-rotate-interval" ), QgsSettings::Prefix::GPS, 0, QStringLiteral( "Interval for GPS automatic map rotation" ) );

    QgsGpsInformationWidget( QgsAppGpsConnection *connection, QgsMapCanvas *mapCanvas, QWidget *parent = nullptr );
    ~QgsGpsInformationWidget() override;

    bool blockCanvasInteraction( Interaction interaction ) const override;

  public slots:
    void tapAndHold( const QgsPointXY &mapPoint, QTapAndHoldGesture *gesture );


  private slots:
    void mConnectButton_toggled( bool flag );
    void recenter();
    void displayGPSInformation( const QgsGpsInformation &info );
    void logNmeaSentence( const QString &nmeaString ); // added to handle 'raw' data
    void updateCloseFeatureButton( QgsMapLayer *lyr );
    void layerEditStateChanged();
    void gpsSettingsChanged();
    void updateTrackAppearance();
    void updateBearingAppearance();
    void mBtnPosition_clicked();
    void mBtnSignal_clicked();
    void mBtnSatellites_clicked();
    void mBtnOptions_clicked();
    void mBtnDebug_clicked();
    void mBtnAddVertex_clicked();
    void mBtnCloseFeature_clicked();
    void mBtnResetFeature_clicked();
// not needed    void on_mCbxAutoAddVertices_toggled( bool flag );

    void timedout();
    void switchAcquisition();
    void timestampFormatChanged( int index );
    void cursorCoordinateChanged( const QgsPointXY &point );

    /**
     * Updates compatible fields for timestamp recording
     */
    void updateTimestampDestinationFields( QgsMapLayer *mapLayer );

    void gpsConnecting();
    void gpsConnectionError( const QString &error );
    void gpsDisconnected();
    void gpsConnected();

  private:

    void addVertex();
    void setStatusIndicator( Qgis::GpsFixStatus statusValue );
    void showStatusBarMessage( const QString &msg );
    void updateTimeZones();
    QVariant timestamp( QgsVectorLayer *vlayer, int idx );

    QgsAppGpsConnection *mConnection = nullptr;
    QPointer< QgsMapCanvas > mMapCanvas;
    QgsGpsMarker *mMapMarker = nullptr;
    QgsGpsBearingItem *mMapBearingItem = nullptr;

    QwtPlot *mPlot = nullptr;
    QwtPlotCurve *mCurve = nullptr;
#ifdef WITH_QWTPOLAR
    QwtPolarPlot *mpSatellitesWidget = nullptr;
    QwtPolarGrid *mpSatellitesGrid = nullptr;
    QList< QwtPolarMarker * > mMarkerList;
#endif
    void createRubberBand();

    void updateGpsDistanceStatusMessage( bool forceDisplay );

    QgsCoordinateReferenceSystem mWgs84CRS;
    QgsCoordinateTransform mCanvasToWgs84Transform;
    QgsDistanceArea mDistanceCalculator;

// not used    QPointF gpsToPixelPosition( const QgsPoint& point );
    QgsRubberBand *mRubberBand = nullptr;
    QgsPointXY mLastGpsPosition;
    QgsPointXY mSecondLastGpsPosition;
    QVector<QgsPoint> mCaptureList;
    double mLastElevation = 0.0;
    Qgis::GpsFixStatus mLastFixStatus = Qgis::GpsFixStatus::NoData;
    QString mDateTimeFormat; // user specified format string in registry (no UI presented)
    QPointer< QgsVectorLayer > mLastLayer;
    QFile *mLogFile = nullptr;
    QTextStream mLogFileTextStream;
    nmeaPOS mLastNmeaPosition;
    nmeaTIME mLastNmeaTime;
    std::unique_ptr<QTimer> mAcquisitionTimer;
    bool mAcquisitionEnabled = true;
    int mAcquisitionInterval = 0;
    double mDistanceThreshold = 0;
    bool mBearingFromTravelDirection = false;
    int mMapExtentMultiplier = 50;
    int mMapRotateInterval = 0;
    //! Temporary storage of preferred fields
    QMap<QString, QString> mPreferredTimestampFields;
    //! Flag when updating fields
    bool mPopulatingFields = false;

    QgsPointXY mLastCursorPosWgs84;
    std::unique_ptr< QgsBearingNumericFormat > mBearingNumericFormat;

    QElapsedTimer mLastRotateTimer;
    QElapsedTimer mLastForcedStatusUpdate;

    int mBlockGpsStateChanged = 0;

    friend class TestQgsGpsInformationWidget;
};

#endif // QGSGPSINFORMATIONWIDGET_H
