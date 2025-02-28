/***************************************************************************
  qgs3dmapcanvaswidget.h
  --------------------------------------
  Date                 : January 2022
  Copyright            : (C) 2022 by Belgacem Nedjima
  Email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DMAPCANVASWIDGET_H
#define QGS3DMAPCANVASWIDGET_H

#include "qgis_app.h"
#include "qobjectuniqueptr.h"
#include "qgsrectangle.h"

#include <QComboBox>
#include <QMenu>
#include <QPointer>
#include <QToolBar>

#define SIP_NO_FILE


class Qgs3DMapToolPaintBrush;
class QLabel;
class QProgressBar;

class Qgs3DAnimationWidget;
class Qgs3DMapCanvas;
class Qgs3DMapSettings;
class Qgs3DMapToolIdentify;
class Qgs3DMapToolMeasureLine;
class Qgs3DMapToolPointCloudChangeAttribute;
class Qgs3DNavigationWidget;
class Qgs3DDebugWidget;
class QgsMapLayer;
class QgsMapTool;
class QgsMapToolExtent;
class QgsMapCanvas;
class QgsDockableWidgetHelper;
class QgsMessageBar;
class QgsRubberBand;
class QgsDoubleSpinBox;

//! Helper validator for classification classes
class ClassValidator : public QValidator
{
  public:
    ClassValidator( QWidget *parent );
    QValidator::State validate( QString &input, int &pos ) const override;
    void fixup( QString &input ) const override;
    void setClasses( const QMap<int, QString> &classes ) { mClasses = classes; }

  private:
    QMap<int, QString> mClasses;
    QRegularExpression mRx;
};

class APP_EXPORT Qgs3DMapCanvasWidget : public QWidget
{
    Q_OBJECT

  public:
    Qgs3DMapCanvasWidget( const QString &name, bool isDocked );
    ~Qgs3DMapCanvasWidget() override;

    //! takes ownership
    void setMapSettings( Qgs3DMapSettings *map );

    void setMainCanvas( QgsMapCanvas *canvas );

    Qgs3DMapCanvas *mapCanvas3D() const { return mCanvas; }

    Qgs3DAnimationWidget *animationWidget() const { return mAnimationWidget; }

    Qgs3DMapToolMeasureLine *measurementLineTool() const { return mMapToolMeasureLine; }

    QgsDockableWidgetHelper *dockableWidgetHelper() const { return mDockableWidgetHelper; }

    void setCanvasName( const QString &name );
    QString canvasName() const { return mCanvasName; }

    void showAnimationWidget() { mActionAnim->trigger(); }

    void updateLayerRelatedActions( QgsMapLayer *layer );

    bool eventFilter( QObject *watched, QEvent *event ) override;


  private slots:
    void resetView();
    void configure();
    void saveAsImage();
    void toggleAnimations();
    void cameraControl();
    void identify();
    void measureLine();
    void paintBrush();
    void polygonTool();
    void exportScene();
    void toggleNavigationWidget( bool visibility );
    void toggleFpsCounter( bool visibility );
    void toggleDebugWidget( bool visibility ) const;
    void toggleDebugWidget() const;
    void setSceneExtentOn2DCanvas();
    void setSceneExtent( const QgsRectangle &extent );

    void onMainCanvasLayersChanged();
    void onMainCanvasColorChanged();
    void onTotalPendingJobsCountChanged();
    void updateFpsCount( float fpsCount );
    void cameraNavigationSpeedChanged( double speed );
    void mapThemeMenuAboutToShow();
    //! Renames the active map theme called \a theme to \a newTheme
    void currentMapThemeRenamed( const QString &theme, const QString &newTheme );

    void onMainMapCanvasExtentChanged();
    void onViewed2DExtentFrom3DChanged( QVector<QgsPointXY> extent );
    void onViewFrustumVisualizationEnabledChanged();
    void onExtentChanged();
    void onGpuMemoryLimitReached();

    void onPointCloudChangeAttributeSettingsChanged();

  private:
    QString mCanvasName;
    Qgs3DMapCanvas *mCanvas = nullptr;
    Qgs3DAnimationWidget *mAnimationWidget = nullptr;
    QgsMapCanvas *mMainCanvas = nullptr;
    QProgressBar *mProgressPendingJobs = nullptr;
    QLabel *mLabelPendingJobs = nullptr;
    QLabel *mLabelFpsCounter = nullptr;
    QLabel *mLabelNavigationSpeed = nullptr;
    QTimer *mLabelNavSpeedHideTimeout = nullptr;
    Qgs3DMapToolIdentify *mMapToolIdentify = nullptr;
    Qgs3DMapToolMeasureLine *mMapToolMeasureLine = nullptr;
    std::unique_ptr<Qgs3DMapToolPointCloudChangeAttribute> mMapToolChangeAttribute;
    std::unique_ptr<QgsMapToolExtent> mMapToolExtent;
    QgsMapTool *mMapToolPrevious = nullptr;
    QMenu *mExportMenu = nullptr;
    QMenu *mMapThemeMenu = nullptr;
    QMenu *mCameraMenu = nullptr;
    QMenu *mEffectsMenu = nullptr;
    QMenu *mEditingToolsMenu = nullptr;
    QList<QAction *> mMapThemeMenuPresetActions;
    QAction *mActionEnableShadows = nullptr;
    QAction *mActionEnableEyeDome = nullptr;
    QAction *mActionEnableAmbientOcclusion = nullptr;
    QAction *mActionSync2DNavTo3D = nullptr;
    QAction *mActionSync3DNavTo2D = nullptr;
    QAction *mShowFrustumPolygon = nullptr;
    QAction *mActionAnim = nullptr;
    QAction *mActionExport = nullptr;
    QAction *mActionMapThemes = nullptr;
    QAction *mActionCamera = nullptr;
    QAction *mActionEffects = nullptr;
    QAction *mActionSetSceneExtent = nullptr;
    QAction *mActionToggleEditing = nullptr;
    QAction *mActionUndo = nullptr;
    QAction *mActionRedo = nullptr;
    QAction *mEditingToolsAction = nullptr;
    QToolBar *mPointCloudEditingToolbar = nullptr;
    QgsDockableWidgetHelper *mDockableWidgetHelper = nullptr;
    QObjectUniquePtr<QgsRubberBand> mViewFrustumHighlight;
    QObjectUniquePtr<QgsRubberBand> mViewExtentHighlight;
    QPointer<QDialog> mConfigureDialog;
    QgsMessageBar *mMessageBar = nullptr;
    bool mGpuMemoryLimitReachedReported = false;

    //! Container QWidget that encapsulates 3D QWindow
    QWidget *mContainer = nullptr;
    //! On-Screen Navigation widget.
    Qgs3DNavigationWidget *mNavigationWidget = nullptr;
    //! On-screen Debug widget
    Qgs3DDebugWidget *mDebugWidget = nullptr;

    QToolBar *mEditingToolBar = nullptr;
    QComboBox *mCboChangeAttribute = nullptr;
    QComboBox *mCboChangeAttributeValue = nullptr;
    ClassValidator *mClassValidator = nullptr;
    QgsDoubleSpinBox *mSpinChangeAttributeValue = nullptr;
    QAction *mCboChangeAttributeValueAction = nullptr;
    QAction *mSpinChangeAttributeValueAction = nullptr;

    QMenu *mToolbarMenu = nullptr;
};

#endif // QGS3DMAPCANVASWIDGET_H
