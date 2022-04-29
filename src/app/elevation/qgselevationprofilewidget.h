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
#include "qgsgeometry.h"
#include "qobjectuniqueptr.h"
#include "qgssettingsentryimpl.h"

#include <QWidgetAction>
#include <QElapsedTimer>
#include <QTimer>

class QgsDockableWidgetHelper;
class QgsMapCanvas;
class QProgressBar;
class QToolButton;
class QgsElevationProfileCanvas;
class QgsMapToolProfileCurve;
class QgsMapToolProfileCurveFromFeature;
class QgsGeometry;
class QgsRubberBand;
class QgsPlotToolPan;
class QgsPlotToolZoom;
class QgsPlotToolXAxisZoom;
class QgsDoubleSpinBox;
class QgsElevationProfileWidgetSettingsAction;
class QgsElevationProfileLayerTreeView;
class QgsLayerTree;
class QgsLayerTreeRegistryBridge;

class QgsElevationProfileWidget : public QWidget
{
    Q_OBJECT
  public:

    static const inline QgsSettingsEntryDouble settingTolerance = QgsSettingsEntryDouble( QStringLiteral( "tolerance" ), QgsSettings::Prefix::ELEVATION_PROFILE, 0.1, QStringLiteral( "Tolerance distance for elevation profile plots" ), Qgis::SettingsOptions(), 0 );
    static const inline QgsSettingsEntryBool settingShowLayerTree = QgsSettingsEntryBool( QStringLiteral( "show-layer-tree" ), QgsSettings::Prefix::ELEVATION_PROFILE, true, QStringLiteral( "Whether the layer tree should be shown for elevation profile plots" ) );

    QgsElevationProfileWidget( const QString &name );
    ~QgsElevationProfileWidget();

    QgsDockableWidgetHelper *dockableWidgetHelper() { return mDockableWidgetHelper; }

    void setCanvasName( const QString &name );
    QString canvasName() const { return mCanvasName; }

    void setMainCanvas( QgsMapCanvas *canvas );

    /**
     * Cancel any rendering job, in a blocking way. Used for application closing.
     */
    void cancelJobs();

  signals:
    void toggleDockModeRequested( bool docked );

  private slots:
    void populateInitialLayers();
    void updateCanvasLayers();
    void onTotalPendingJobsCountChanged( int count );
    void setProfileCurve( const QgsGeometry &curve );
    void onCanvasPointHovered( const QgsPointXY &point );
    void updatePlot();
    void scheduleUpdate();
    void clear();
    void exportAsPdf();
    void exportAsImage();
    void nudgeLeft();
    void nudgeRight();
    void nudgeCurve( Qgis::BufferSide side );

  private:
    QgsElevationProfileCanvas *mCanvas = nullptr;

    QString mCanvasName;
    QgsMapCanvas *mMainCanvas = nullptr;

    QProgressBar *mProgressPendingJobs = nullptr;
    QElapsedTimer mLastJobTime;
    double mLastJobTimeSeconds = 0;
    QTimer mJobProgressBarTimer;
    QMetaObject::Connection mJobProgressBarTimerConnection;

    QMenu *mOptionsMenu = nullptr;
    QToolButton *mBtnOptions = nullptr;
    QAction *mCaptureCurveAction = nullptr;
    QAction *mCaptureCurveFromFeatureAction = nullptr;
    QAction *mNudgeLeftAction = nullptr;
    QAction *mNudgeRightAction = nullptr;

    QgsDockableWidgetHelper *mDockableWidgetHelper = nullptr;
    std::unique_ptr< QgsMapToolProfileCurve > mCaptureCurveMapTool;
    std::unique_ptr< QgsMapToolProfileCurveFromFeature > mCaptureCurveFromFeatureMapTool;
    QgsGeometry mProfileCurve;

    QObjectUniquePtr<QgsRubberBand> mMapPointRubberBand;
    QObjectUniquePtr<QgsRubberBand> mRubberBand;
    QObjectUniquePtr<QgsRubberBand> mToleranceRubberBand;

    QTimer *mSetCurveTimer = nullptr;
    bool mUpdateScheduled = false;
    void createOrUpdateRubberBands();

    QgsPlotToolPan *mPanTool = nullptr;
    QgsPlotToolXAxisZoom *mXAxisZoomTool = nullptr;
    QgsPlotToolZoom *mZoomTool = nullptr;

    QgsElevationProfileWidgetSettingsAction *mSettingsAction = nullptr;

    std::unique_ptr< QgsLayerTree > mLayerTree;
    QgsLayerTreeRegistryBridge *mLayerTreeBridge = nullptr;
    QgsElevationProfileLayerTreeView *mLayerTreeView = nullptr;
};


class QgsElevationProfileWidgetSettingsAction: public QWidgetAction
{
    Q_OBJECT

  public:

    QgsElevationProfileWidgetSettingsAction( QWidget *parent = nullptr );

    QgsDoubleSpinBox *toleranceSpinBox() { return mToleranceWidget; }

  private:
    QgsDoubleSpinBox *mToleranceWidget = nullptr;
};

#endif // QGSELEVATIONPROFILEWIDGET_H
