/***************************************************************************
                          qgselevationprofilewidget.h
                          ---------------
    begin                : March 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSELEVATIONPROFILEWIDGET_H
#define QGSELEVATIONPROFILEWIDGET_H

#include "qmenu.h"
#include "qgsdockwidget.h"
#include "qgis_app.h"

class QgsDockableWidgetHelper;
class QgsMapCanvas;
class QProgressBar;
class QToolButton;
class QgsElevationProfileCanvas;
class QgsMapToolProfileCurve;
class QgsGeometry;

class QgsElevationProfileWidget : public QWidget
{
    Q_OBJECT
  public:
    QgsElevationProfileWidget( const QString &name, bool isDocked );
    ~QgsElevationProfileWidget();

    QgsDockableWidgetHelper *dockableWidgetHelper() { return mDockableWidgetHelper; }

    void setCanvasName( const QString &name );
    QString canvasName() const { return mCanvasName; }

    void setMainCanvas( QgsMapCanvas *canvas );
  signals:
    void toggleDockModeRequested( bool docked );

  private slots:
    void onMainCanvasLayersChanged();
    void onTotalPendingJobsCountChanged();
    void setProfileCurve( const QgsGeometry &curve );
  private:
    QgsElevationProfileCanvas *mCanvas = nullptr;

    QString mCanvasName;
    QgsMapCanvas *mMainCanvas = nullptr;
    QProgressBar *mProgressPendingJobs = nullptr;
    QMenu *mOptionsMenu = nullptr;
    QToolButton *mBtnOptions = nullptr;
    QAction *mCaptureCurveAction = nullptr;

    QgsDockableWidgetHelper *mDockableWidgetHelper = nullptr;
    std::unique_ptr< QgsMapToolProfileCurve > mCaptureCurveMapTool;
};

#endif // QGSELEVATIONPROFILEWIDGET_H
