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

#include "gmath.h"
#include "info.h"
#include "qgsmapcanvas.h"
#include "qgsgpsmarker.h"
#include "qgsmaptoolcapture.h"
#include <qwt_plot_curve.h>

#ifdef WITH_QWTPOLAR
#include <qwt_polar_plot.h>
#include <qwt_polar_grid.h>
#include <qwt_polar_marker.h>
#endif
#define MAXACQUISITIONINTERVAL 300 // max gps information acquisition suspension interval (in seconds)
#define MAXDISTANCETHRESHOLD 10 // max gps distance threshold (in meters)

class QextSerialPort;
class QgsGPSConnection;
class QgsGPSTrackerThread;
struct QgsGPSInformation;

class QFile;
class QColor;

/** A dock widget that displays information from a GPS device and
 * allows the user to capture features using gps readings to
 * specify the geometry.*/
class QgsGPSInformationWidget: public QWidget, private Ui::QgsGPSInformationWidgetBase
{
    Q_OBJECT
  public:
    QgsGPSInformationWidget( QgsMapCanvas *thepCanvas, QWidget *parent = nullptr, Qt::WindowFlags f = 0 );
    ~QgsGPSInformationWidget();

  private slots:
    void on_mConnectButton_toggled( bool flag );
    void displayGPSInformation( const QgsGPSInformation &info );
    void logNmeaSentence( const QString &nmeaString ); // added to handle 'raw' data
    void updateCloseFeatureButton( QgsMapLayer *lyr );
    void layerEditStateChanged();
//   void setTrackColor(); // no longer used
    void on_mBtnTrackColor_clicked();
    void on_mSpinTrackWidth_valueChanged( int value );
    void on_mBtnPosition_clicked();
    void on_mBtnSignal_clicked();
    void on_mBtnSatellites_clicked();
    void on_mBtnOptions_clicked();
    void on_mBtnDebug_clicked();
    void on_mBtnRefreshDevices_clicked();
    void on_mBtnAddVertex_clicked();
    void on_mBtnCloseFeature_clicked();
    void on_mBtnResetFeature_clicked();
// not needed    void on_mCbxAutoAddVertices_toggled( bool flag );
    void on_mBtnLogFile_clicked();
    void connected( QgsGPSConnection * );
    void timedout();
    void switchAcquisition();
    void on_cboAcquisitionIntervalActivated( const QString & );
    void on_cboDistanceThresholdActivated( const QString & );
    void on_cboAcquisitionIntervalEdited();
    void on_cboDistanceThresholdEdited();
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
    void setStatusIndicator( const FixStatus statusValue );
    void showStatusBarMessage( const QString &msg );
    void setAcquisitionInterval( int );
    void setDistanceThreshold( int );
    QgsGPSConnection *mNmea = nullptr;
    QgsMapCanvas *mpCanvas = nullptr;
    QgsGpsMarker *mpMapMarker = nullptr;
    QwtPlot *mpPlot = nullptr;
    QwtPlotCurve *mpCurve = nullptr;
#ifdef WITH_QWTPOLAR
    QwtPolarPlot *mpSatellitesWidget = nullptr;
    QwtPolarGrid *mpSatellitesGrid = nullptr;
    QList< QwtPolarMarker * > mMarkerList;
#endif
    void createRubberBand();

    QgsCoordinateReferenceSystem mWgs84CRS;
// not used    QPointF gpsToPixelPosition( const QgsPoint& point );
    QgsRubberBand *mpRubberBand = nullptr;
    QgsPointXY mLastGpsPosition;
    QList<QgsPointXY> mCaptureList;
    FixStatus mLastFixStatus;
    QString mDateTimeFormat; // user specified format string in registry (no UI presented)
    QgsVectorLayer *mpLastLayer = nullptr;
    QFile *mLogFile = nullptr;
    QTextStream mLogFileTextStream;
    QColor mTrackColor;
    QIntValidator *acquisitionIntValidator;
    QIntValidator *distanceThresholdValidator;
    QLineEdit *acIntervalEdit, *distThresholdEdit;
    nmeaPOS lastNmeaPosition;
    QTimer *acquisitionTimer;
    bool acquisitionEnabled;
    unsigned int acquisitionInterval;
    unsigned int distanceThreshold;
};

#endif // QGSGPSINFORMATIONWIDGET_H
