/***************************************************************************
  qgs3dmapcanvasdockwidget.h
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DMAPCANVASDOCKWIDGET_H
#define QGS3DMAPCANVASDOCKWIDGET_H

#include "qmenu.h"
#include "qgsdockwidget.h"
#include "qgis_app.h"
#include "qtoolbutton.h"

#define SIP_NO_FILE

class QLabel;
class QProgressBar;

class Qgs3DAnimationWidget;
class Qgs3DMapCanvas;
class Qgs3DMapSettings;
class Qgs3DMapToolIdentify;
class Qgs3DMapToolMeasureLine;
class QgsMapCanvas;



class APP_EXPORT Qgs3DMapCanvasDockWidget : public QgsDockWidget
{
    Q_OBJECT
  public:
    Qgs3DMapCanvasDockWidget( QWidget *parent = nullptr );

    //! takes ownership
    void setMapSettings( Qgs3DMapSettings *map );

    void setMainCanvas( QgsMapCanvas *canvas );

    Qgs3DMapCanvas *mapCanvas3D() { return mCanvas; }
    Qgs3DAnimationWidget *animationWidget() { return mAnimationWidget; }

    Qgs3DMapToolMeasureLine *measurementLineTool() { return  mMapToolMeasureLine; }

  private slots:
    void resetView();
    void configure();
    void saveAsImage();
    void toggleAnimations();
    void cameraControl();
    void identify();
    void measureLine();
    void exportScene();
    void toggleNavigationWidget( bool visibility );

    void onMainCanvasLayersChanged();
    void onMainCanvasColorChanged();
    void onTotalPendingJobsCountChanged();
    void mapThemeMenuAboutToShow();
    //! Renames the active map theme called \a theme to \a newTheme
    void currentMapThemeRenamed( const QString &theme, const QString &newTheme );

  private:
    Qgs3DMapCanvas *mCanvas = nullptr;
    Qgs3DAnimationWidget *mAnimationWidget = nullptr;
    QgsMapCanvas *mMainCanvas = nullptr;
    QProgressBar *mProgressPendingJobs = nullptr;
    QLabel *mLabelPendingJobs = nullptr;
    Qgs3DMapToolIdentify *mMapToolIdentify = nullptr;
    Qgs3DMapToolMeasureLine *mMapToolMeasureLine = nullptr;
    QMenu *mMapThemeMenu = nullptr;
    QList<QAction *> mMapThemeMenuPresetActions;
    QToolButton *mBtnMapThemes = nullptr;
};

#endif // QGS3DMAPCANVASDOCKWIDGET_H
