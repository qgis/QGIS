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
/*  $Id: qgisapp.h 12390 2009-12-09 21:35:43Z jef $ */
#ifndef QGSGPSINFORMATIONWIDGET_H
#define QGSGPSINFORMATIONWIDGET_H

#include "ui_qgsgpsinformationwidgetbase.h"

#include <qgsmapcanvas.h>
#include <qgsgpsmarker.h>
#include <qgsmaptoolcapture.h>
#include <qwt_plot_curve.h>
#include <qwt_polar_plot.h>
#include <qwt_polar_marker.h>
class QextSerialPort;
class QgsGPSConnection;
class QgsGPSTrackerThread;
struct QgsGPSInformation;
class QPointF;

/**A dock widget that displays information from a GPS device and
 * allows the user to capture features using gps readings to
 * specify the geometry.*/
class QgsGPSInformationWidget: public QWidget, private Ui::QgsGPSInformationWidgetBase
{
    Q_OBJECT
  public:
    QgsGPSInformationWidget( QgsMapCanvas * thepCanvas, QWidget * parent = 0, Qt::WindowFlags f = 0 );
    ~QgsGPSInformationWidget();

  private slots:
    void on_mConnectButton_toggled( bool theFlag );
    void displayGPSInformation( const QgsGPSInformation& info );
    void setTrackColour( );
    void on_mBtnTrackColour_clicked( );
    void on_mSpinTrackWidth_valueChanged( int theValue );
    void on_mBtnPosition_clicked( );
    void on_mBtnSignal_clicked( );
    void on_mBtnSatellites_clicked( );
    void on_mBtnOptions_clicked( );
    void on_mBtnDebug_clicked( );
    void on_mBtnRefreshDevices_clicked( );
    void on_mBtnAddVertex_clicked( );
    void on_mBtnCloseFeature_clicked( );
    void on_mBtnResetFeature_clicked( );
    void on_mCbxAutoAddVertices_toggled( bool theFlag );

    void connected( QgsGPSConnection * );
    void timedout();

  private:
    void addVertex( );
    void connectGps();
    void connectGpsSlot( );
    void disconnectGps();
    void populateDevices();
    QgsGPSConnection* mNmea;
    QgsMapCanvas * mpCanvas;
    QgsGpsMarker * mpMapMarker;
    QwtPlot * mpPlot;
    QwtPlotCurve * mpCurve;
    QwtPolarPlot * mpSatellitesWidget;
    QList< QwtPolarMarker * > mMarkerList;
    void createRubberBand( );
    QgsCoordinateReferenceSystem mWgs84CRS;
    QPointF gpsToPixelPosition( const QgsPoint& point );
    QgsRubberBand * mpRubberBand;
    QgsPoint mLastGpsPosition;
    QList<QgsPoint> mCaptureList;
};

#endif // QGSGPSINFORMATIONWIDGET_H
