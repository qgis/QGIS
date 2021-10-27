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
#include "gmath.h"
#include "info.h"
#include "nmeatime.h"
#include "qgsgpsmarker.h"
#include "qgsmaptoolcapture.h"
#include "qgspanelwidget.h"
#include "qgsmapcanvasinteractionblocker.h"
#include "qgsdistancearea.h"

#include <qwt_plot_curve.h>
#ifdef WITH_QWTPOLAR
#include <qwt_polar_plot.h>
#include <qwt_polar_grid.h>
#include <qwt_polar_marker.h>
#endif
#include <QTextStream>

class QextSerialPort;
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
    QgsGpsInformationWidget( QgsMapCanvas *mapCanvas, QWidget *parent = nullptr );
    ~QgsGpsInformationWidget() override;

    bool blockCanvasInteraction( Interaction interaction ) const override;

    /**
     * Sets a GPS \a connection to use within the GPS Panel widget.
     *
     * Any existing GPS connection used by the widget will be disconnect and replaced with this connection. The connection
     * is automatically registered within the QgsApplication::gpsConnectionRegistry().
     */
    void setConnection( QgsGpsConnection *connection );

  public slots:
    void tapAndHold( const QgsPointXY &mapPoint, QTapAndHoldGesture *gesture );


  private slots:
    void mConnectButton_toggled( bool flag );
    void recenter();
    void displayGPSInformation( const QgsGpsInformation &info );
    void logNmeaSentence( const QString &nmeaString ); // added to handle 'raw' data
    void updateCloseFeatureButton( QgsMapLayer *lyr );
    void layerEditStateChanged();
//   void setTrackColor(); // no longer used
    void trackColorChanged( const QColor &color );
    void mSpinTrackWidth_valueChanged( int value );
    void mBtnPosition_clicked();
    void mBtnSignal_clicked();
    void mBtnSatellites_clicked();
    void mBtnOptions_clicked();
    void mBtnDebug_clicked();
    void mBtnRefreshDevices_clicked();
    void mBtnAddVertex_clicked();
    void mBtnCloseFeature_clicked();
    void mBtnResetFeature_clicked();
// not needed    void on_mCbxAutoAddVertices_toggled( bool flag );
    void mBtnLogFile_clicked();

    void connected( QgsGpsConnection * );
    void timedout();
    void switchAcquisition();
    void cboAcquisitionIntervalEdited();
    void cboDistanceThresholdEdited();
    void timestampFormatChanged( int index );
    void cursorCoordinateChanged( const QgsPointXY &point );

    /**
     * Updates compatible fields for timestamp recording
     */
    void updateTimestampDestinationFields( QgsMapLayer *mapLayer );

  private:
    enum FixStatus  //GPS status
    {
      NoData, NoFix, Fix2D, Fix3D
    };
    void addVertex();
    void connectGps();
    void connectGpsSlot();
    void disconnectGps();
    void populateDevices();
    void setStatusIndicator( FixStatus statusValue );
    void showStatusBarMessage( const QString &msg );
    void setAcquisitionInterval( uint );
    void setDistanceThreshold( uint );
    void updateTimeZones();
    QVariant timestamp( QgsVectorLayer *vlayer, int idx );
    QgsGpsConnection *mNmea = nullptr;
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
    FixStatus mLastFixStatus;
    QString mDateTimeFormat; // user specified format string in registry (no UI presented)
    QPointer< QgsVectorLayer > mLastLayer;
    QFile *mLogFile = nullptr;
    QTextStream mLogFileTextStream;
    QIntValidator *mAcquisitionIntValidator = nullptr;
    QIntValidator *mDistanceThresholdValidator = nullptr;
    nmeaPOS mLastNmeaPosition;
    nmeaTIME mLastNmeaTime;
    std::unique_ptr<QTimer> mAcquisitionTimer;
    bool mAcquisitionEnabled = true;
    int mAcquisitionInterval = 0;
    unsigned int mDistanceThreshold = 0;
    //! Temporary storage of preferred fields
    QMap<QString, QString> mPreferredTimestampFields;
    //! Flag when updating fields
    bool mPopulatingFields = false;

    QgsPointXY mLastCursorPosWgs84;
    std::unique_ptr< QgsBearingNumericFormat > mBearingNumericFormat;

    QElapsedTimer mLastRotateTimer;
    QElapsedTimer mLastForcedStatusUpdate;

    friend class TestQgsGpsInformationWidget;
};

#endif // QGSGPSINFORMATIONWIDGET_H
