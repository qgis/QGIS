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

#include "qgspanelwidget.h"
#include "qgspointxy.h"

#include <qwt_plot_curve.h>
#ifdef WITH_QWTPOLAR
#include <qwt_polar_plot.h>
#include <qwt_polar_grid.h>
#include <qwt_polar_marker.h>
#endif
#include <QTextStream>
#include <QPointer>

class QextSerialPort;
class QgsAppGpsConnection;
class QgsGpsInformation;
class QgsMapCanvas;
class QgsAppGpsDigitizing;

/**
 * A dock widget that displays information from a GPS device and
 * allows the user to capture features using gps readings to
 * specify the geometry.
*/
class APP_EXPORT QgsGpsInformationWidget: public QgsPanelWidget, private Ui::QgsGpsInformationWidgetBase
{
    Q_OBJECT
  public:

    QgsGpsInformationWidget( QgsAppGpsConnection *connection, QgsMapCanvas *mapCanvas, QgsAppGpsDigitizing *digitizing = nullptr, QWidget *parent = nullptr );
    ~QgsGpsInformationWidget() override;

  private slots:
    void mConnectButton_toggled( bool flag );
    void displayGPSInformation( const QgsGpsInformation &info );

    void mBtnPosition_clicked();
    void mBtnSignal_clicked();
    void mBtnSatellites_clicked();
    void mBtnDebug_clicked();

    void timedout();

    void gpsConnecting();
    void gpsDisconnected();
    void gpsConnected();
    void updateTrackInformation();

  private:

    void setStatusIndicator( Qgis::GpsFixStatus statusValue );
    void showStatusBarMessage( const QString &msg );

    QgsAppGpsConnection *mConnection = nullptr;
    QPointer< QgsMapCanvas > mMapCanvas;
    QgsAppGpsDigitizing *mDigitizing = nullptr;

    QwtPlot *mPlot = nullptr;
    QwtPlotCurve *mCurve = nullptr;
#ifdef WITH_QWTPOLAR
    QwtPolarPlot *mpSatellitesWidget = nullptr;
    QwtPolarGrid *mpSatellitesGrid = nullptr;
    QList< QwtPolarMarker * > mMarkerList;
#endif

    QgsPointXY mLastGpsPosition;

    QString mDateTimeFormat; // user specified format string in registry (no UI presented)

    friend class TestQgsGpsInformationWidget;
};

#endif // QGSGPSINFORMATIONWIDGET_H
