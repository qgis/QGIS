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
#include "qgselevationprofilelayertreeview.h"
#include "ui_qgselevationprofileaddlayersdialogbase.h"

#include <QWidgetAction>
#include <QElapsedTimer>
#include <QTimer>
#include <QPointer>

class QgsElevationProfile;
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
class QgsElevationProfileToleranceWidgetSettingsAction;
class QgsElevationProfileScaleRatioWidgetSettingsAction;
class QgsLayerTree;
class QgsLayerTreeRegistryBridge;
class QgsElevationProfileToolIdentify;
class QgsElevationProfileToolMeasure;
class QLabel;
class QgsProfilePoint;
class QgsSettingsEntryDouble;
class QgsSettingsEntryBool;
class QgsSettingsEntryString;
class QgsSettingsEntryColor;
class QgsMapLayerProxyModel;
class QgsLineSymbol;
class QgsScaleComboBox;

class QgsAppElevationProfileLayerTreeView : public QgsElevationProfileLayerTreeView
{
    Q_OBJECT
  public:
    explicit QgsAppElevationProfileLayerTreeView( QgsLayerTree *rootNode, QWidget *parent = nullptr );

  protected:
    void contextMenuEvent( QContextMenuEvent *event ) override;
};

class QgsElevationProfileLayersDialog : public QDialog, private Ui::QgsElevationProfileAddLayersDialogBase
{
    Q_OBJECT

  public:
    QgsElevationProfileLayersDialog( QWidget *parent = nullptr );
    void setVisibleLayers( const QList<QgsMapLayer *> &layers );
    void setHiddenLayers( const QList<QgsMapLayer *> &layers );
    QList<QgsMapLayer *> selectedLayers() const;

  private slots:

    void filterVisible( bool enabled );

  private:
    QgsMapLayerProxyModel *mModel = nullptr;
    QList<QgsMapLayer *> mVisibleLayers;
};

class QgsElevationProfileWidget : public QWidget
{
    Q_OBJECT
  public:
    static const QgsSettingsEntryDouble *settingTolerance;
    static const QgsSettingsEntryBool *settingShowLayerTree;
    static const QgsSettingsEntryBool *settingLockAxis;
    static const QgsSettingsEntryString *settingLastExportDir;
    static const QgsSettingsEntryColor *settingBackgroundColor;
    static const QgsSettingsEntryBool *settingShowSubsections;
    static const QgsSettingsEntryBool *settingShowScaleRatioInToolbar;

    QgsElevationProfileWidget( QgsElevationProfile *profile, QgsMapCanvas *canvas );
    ~QgsElevationProfileWidget();

    /**
     * Modifies an elevation \a profile to apply default QGIS app settings to it.
     */
    static void applyDefaultSettingsToProfile( QgsElevationProfile *profile );

    QgsElevationProfile *profile();

    QgsDockableWidgetHelper *dockableWidgetHelper() { return mDockableWidgetHelper; }

    QgsElevationProfileCanvas *profileCanvas() { return mCanvas; }

    /**
     * Cancel any rendering job, in a blocking way. Used for application closing.
     */
    void cancelJobs();

  signals:
    void toggleDockModeRequested( bool docked );

  private slots:
    void addLayers();
    void addLayersInternal( const QList<QgsMapLayer *> &layers );
    void updateCanvasSources();
    void onTotalPendingJobsCountChanged( int count );
    void setProfileCurve( const QgsGeometry &curve, bool resetView, bool storeCurve = true );
    void onCanvasPointHovered( const QgsPointXY &point, const QgsProfilePoint &profilePoint );
    void updatePlot();
    void scheduleUpdate();
    void clear();
    void exportAsPdf();
    void exportAsImage();
    void exportResults( Qgis::ProfileExportType type );
    void nudgeLeft();
    void nudgeRight();
    void nudgeCurve( Qgis::BufferSide side );
    void axisScaleLockToggled( bool active );
    void renameProfileTriggered();
    void onProjectElevationPropertiesChanged();
    void showSubsectionsTriggered();
    void editSubsectionsSymbology();

  private:
    void setMainCanvas( QgsMapCanvas *canvas );

    QgsElevationProfileCanvas *mCanvas = nullptr;
    QPointer< QgsElevationProfile > mProfile;

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
    QAction *mRenameProfileAction = nullptr;
    QAction *mLockRatioAction = nullptr;
    QAction *mShowSubsectionsAction = nullptr;
    QAction *mSubsectionsSymbologyAction = nullptr;
    QMenu *mDistanceUnitMenu = nullptr;

    QgsDockableWidgetHelper *mDockableWidgetHelper = nullptr;
    std::unique_ptr<QgsMapToolProfileCurve> mCaptureCurveMapTool;
    std::unique_ptr<QgsMapToolProfileCurveFromFeature> mCaptureCurveFromFeatureMapTool;
    std::unique_ptr<QgsElevationProfileToolMeasure> mMeasureTool;
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
    QgsElevationProfileToolIdentify *mIdentifyTool = nullptr;

    QgsElevationProfileToleranceWidgetSettingsAction *mToleranceSettingsAction = nullptr;
    int mBlockScaleRatioChanges = 0;
    QgsElevationProfileScaleRatioWidgetSettingsAction *mScaleRatioSettingsAction = nullptr;

    QgsLayerTreeRegistryBridge *mLayerTreeBridge = nullptr;
    QgsElevationProfileLayerTreeView *mLayerTreeView = nullptr;

    std::unique_ptr<QgsLineSymbol> mSubsectionsSymbol;
};


class QgsElevationProfileToleranceWidgetSettingsAction : public QWidgetAction
{
    Q_OBJECT

  public:
    QgsElevationProfileToleranceWidgetSettingsAction( QWidget *parent = nullptr );

    QgsDoubleSpinBox *toleranceSpinBox() { return mToleranceWidget; }

  private:
    QgsDoubleSpinBox *mToleranceWidget = nullptr;
};

class QgsElevationProfileScaleRatioWidgetSettingsAction : public QWidgetAction
{
    Q_OBJECT

  public:
    QgsElevationProfileScaleRatioWidgetSettingsAction( QWidget *parent = nullptr );
    QWidget *newWidget();

    QgsScaleComboBox *scaleRatioWidget() { return mScaleRatioWidget; }

  private:
    QgsScaleComboBox *mScaleRatioWidget = nullptr;
};


#endif // QGSELEVATIONPROFILEWIDGET_H
