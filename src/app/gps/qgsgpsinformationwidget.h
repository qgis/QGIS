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

#include <qgsmapcanvas.h>
#include <qgsgpsmarker.h>
#include <qgsmaptoolcapture.h>
#include <qwt_plot_curve.h>
#ifdef WITH_QWTPOLAR
#include <qwt_polar_plot.h>
#include <qwt_polar_grid.h>
#include <qwt_polar_marker.h>
#endif

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
    QgsGPSInformationWidget( QgsMapCanvas * thepCanvas, QWidget * parent = nullptr, Qt::WindowFlags f = nullptr );
    ~QgsGPSInformationWidget();

  private slots:
    void on_mConnectButton_toggled( bool theFlag );
    void displayGPSInformation( const QgsGPSInformation& info );
    void logNmeaSentence( const QString& nmeaString ); // added to handle 'raw' data
    void updateCloseFeatureButton( QgsMapLayer * lyr );
    void layerEditStateChanged();
//   void setTrackColor(); // no longer used
    void on_mBtnTrackColor_clicked();
    void on_mSpinTrackWidth_valueChanged( int theValue );
    void on_mBtnPosition_clicked();
    void on_mBtnSignal_clicked();
    void on_mBtnSatellites_clicked();
    void on_mBtnOptions_clicked();
    void on_mBtnDebug_clicked();
    void on_mBtnRefreshDevices_clicked();
    void on_mBtnAddVertex_clicked();
    void on_mBtnCloseFeature_clicked();
    void on_mBtnResetFeature_clicked();
// not needed    void on_mCbxAutoAddVertices_toggled( bool theFlag );
    void on_mBtnLogFile_clicked();

    void connected( QgsGPSConnection * );
    void timedout();

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
    void showStatusBarMessage( const QString& msg );
    QgsGPSConnection* mNmea;
    QgsMapCanvas * mpCanvas;
    QgsGpsMarker * mpMapMarker;
    QwtPlot * mpPlot;
    QwtPlotCurve * mpCurve;
#ifdef WITH_QWTPOLAR
    QwtPolarPlot * mpSatellitesWidget;
    QwtPolarGrid *mpSatellitesGrid;
    QList< QwtPolarMarker * > mMarkerList;
#endif
    void createRubberBand();
    QgsCoordinateReferenceSystem mWgs84CRS;
// not used    QPointF gpsToPixelPosition( const QgsPoint& point );
    QgsRubberBand * mpRubberBand;
    QgsPoint mLastGpsPosition;
    QList<QgsPoint> mCaptureList;
    FixStatus mLastFixStatus;
    QString mDateTimeFormat; // user specified format string in registry (no UI presented)
    QgsVectorLayer * mpLastLayer;
    QFile * mLogFile;
    QTextStream mLogFileTextStream;
    QColor mTrackColor;
};

#endif // QGSGPSINFORMATIONWIDGET_H
