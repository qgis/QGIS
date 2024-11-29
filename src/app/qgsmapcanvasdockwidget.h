/***************************************************************************
    qgsmapcanvasdockwidget.h
    ------------------------
    begin                : February 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMAPCANVASDOCKWIDGET_H
#define QGSMAPCANVASDOCKWIDGET_H

#include "ui_qgsmapcanvasdockwidgetbase.h"

#include "qgspointxy.h"
#include "qgis_app.h"
#include <QWidgetAction>
#include <QTimer>
#include <memory>

class QgsMapCanvas;
class QgsScaleComboBox;
class QgsDoubleSpinBox;
class QgsStatusBarMagnifierWidget;
class QgsMapToolPan;
class QgsVertexMarker;
class QgsRubberBand;
class QCheckBox;
class QRadioButton;
class QgsDockableWidgetHelper;

/**
 * \class QgsMapCanvasDockWidget
 * A dock widget with an embedded map canvas, for additional map views.
 */
class APP_EXPORT QgsMapCanvasDockWidget : public QWidget, private Ui::QgsMapCanvasWidgetBase
{
    Q_OBJECT
  public:
    explicit QgsMapCanvasDockWidget( const QString &name, QWidget *parent = nullptr, bool isDocked = true );
    ~QgsMapCanvasDockWidget() override;

    QgsDockableWidgetHelper *dockableWidgetHelper();

    /**
     * Sets the main app map canvas.
     */
    void setMainCanvas( QgsMapCanvas *canvas );

    /**
     * Returns the map canvas contained in the dock widget.
     */
    QgsMapCanvas *mapCanvas();

    void setCanvasName( const QString &name );
    QString canvasName() const { return mCanvasName; }

    /**
     * Sets whether the view center should be synchronized with the main canvas center.
     * \see isViewCenterSynchronized()
     */
    void setViewCenterSynchronized( bool enabled );

    /**
     * Returns TRUE if the view extent is synchronized with the main canvas extent.
     * \see setViewCenterSynchronized()
     */
    bool isViewCenterSynchronized() const;

    /**
     * Returns TRUE if the view is synchronized with the selection on the main canvas.
     * \see setAutoZoomToSelected()
     */
    bool isAutoZoomToSelected() const;

    /**
     * Sets whether the view is synchronized with the selection on the main canvas.
     * \see isAutoZoomToSelected()
     */
    void setAutoZoomToSelected( bool autoZoom );

    /**
     * Sets whether the cursor position marker is visible.
     * \see isCursorMarkerVisible()
     */
    void setCursorMarkerVisible( bool visible );

    /**
     * Returns TRUE if the cursor position marker is visible.
     * \see setCursorMarkerVisible()
     */
    bool isCursorMarkerVisible() const;

    /**
     * Sets whether the main canvas extent is visible.
     * \see isMainCanvasExtentVisible()
     */
    void setMainCanvasExtentVisible( bool visible );

    /**
     * Returns TRUE if the main canvas extent is visible.
     * \see setMainCanvasExtentVisible()
     */
    bool isMainCanvasExtentVisible() const;

    /**
     * Returns the scaling factor for main canvas scale to view scale.
     * \see setScaleFactor()
     * \see isViewScaleSynchronized()
     */
    double scaleFactor() const;

    /**
     * Sets the scaling \a factor for main canvas scale to view scale.
     * \see scaleFactor()
     * \see setViewScaleSynchronized()
     */
    void setScaleFactor( double factor );

    /**
     * Sets whether the view scale should be synchronized with the main canvas center.
     * \see isViewScaleSynchronized()
     * \see setScaleFactor()
     */
    void setViewScaleSynchronized( bool enabled );

    /**
     * Returns TRUE if the view scale is synchronized with the main canvas extent.
     * \see setViewScaleSynchronized()
     * \see scaleFactor()
     */
    bool isViewScaleSynchronized() const;

    /**
     * Sets whether labels should be rendered in the view.
     * \see labelsVisible()
     */
    void setLabelsVisible( bool enabled );

    /**
     * Returns whether labels are rendered in the view.
     * \see setLabelsVisible()
     */
    bool labelsVisible() const;

  signals:

    void renameTriggered();

  protected:
    void resizeEvent( QResizeEvent *e ) override;

  private slots:

    void setMapCrs();
    void mapExtentChanged();
    void mapCrsChanged();
    void menuAboutToShow();
    //! Renames the active map theme called \a theme to \a newTheme
    void currentMapThemeRenamed( const QString &theme, const QString &newTheme );
    void settingsMenuAboutToShow();
    void syncMarker( const QgsPointXY &p );
    void mapScaleChanged();
    void updateExtentRect();
    void showLabels( bool show );
    void autoZoomToSelection( bool autoZoom );


  private:
    QString mCanvasName;
    QgsMapCanvas *mMapCanvas = nullptr;
    QgsMapCanvas *mMainCanvas = nullptr;
    QMenu *mMenu = nullptr;
    QList<QAction *> mMenuPresetActions;
    QCheckBox *mSyncExtentCheck = nullptr;
    QCheckBox *mSyncSelectionCheck = nullptr;
    QgsScaleComboBox *mScaleCombo = nullptr;
    QgsDoubleSpinBox *mRotationEdit = nullptr;
    QgsDoubleSpinBox *mMagnificationEdit = nullptr;
    QgsDoubleSpinBox *mScaleFactorWidget = nullptr;
    QCheckBox *mSyncScaleCheckBox = nullptr;
    bool mBlockScaleUpdate = false;
    bool mBlockRotationUpdate = false;
    bool mBlockMagnificationUpdate = false;
    bool mBlockExtentSync = false;
    QgsMapToolPan *mPanTool = nullptr;
    QTimer mResizeTimer;
    QgsVertexMarker *mXyMarker = nullptr;
    QgsRubberBand *mExtentRubberBand = nullptr;

    QgsDockableWidgetHelper *mDockableWidgetHelper = nullptr;

    void syncViewCenter( QgsMapCanvas *sourceCanvas );
    void syncSelection();

    friend class TestQgsMapCanvasDockWidget;
};

/**
 * \class QgsMapSettingsAction
 * Allows embedding a scale, rotation and other map settings into a menu.
 */

class QgsMapSettingsAction : public QWidgetAction
{
    Q_OBJECT

  public:
    QgsMapSettingsAction( QWidget *parent = nullptr );

    QCheckBox *syncExtentCheck() { return mSyncExtentCheck; }
    QCheckBox *syncSelectionCheck() { return mSyncSelectionCheck; }
    QgsScaleComboBox *scaleCombo() { return mScaleCombo; }
    QgsDoubleSpinBox *rotationSpinBox() { return mRotationWidget; }
    QgsDoubleSpinBox *magnifierSpinBox() { return mMagnifierWidget; }
    QgsDoubleSpinBox *scaleFactorSpinBox() { return mScaleFactorWidget; }
    QCheckBox *syncScaleCheckBox() { return mSyncScaleCheckBox; }

  private:
    QCheckBox *mSyncSelectionCheck = nullptr;
    QCheckBox *mSyncExtentCheck = nullptr;
    QgsScaleComboBox *mScaleCombo = nullptr;
    QgsDoubleSpinBox *mRotationWidget = nullptr;
    QgsDoubleSpinBox *mMagnifierWidget = nullptr;
    QCheckBox *mSyncScaleCheckBox = nullptr;
    QgsDoubleSpinBox *mScaleFactorWidget = nullptr;
};


#endif // QGSMAPCANVASDOCKWIDGET_H
