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

#include "qgsdockwidget.h"
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

/**
 * \class QgsMapCanvasDockWidget
 * A dock widget with an embedded map canvas, for additional map views.
 * \since QGIS 3.0
 */
class APP_EXPORT QgsMapCanvasDockWidget : public QgsDockWidget, private Ui::QgsMapCanvasDockWidgetBase
{
    Q_OBJECT
  public:
    explicit QgsMapCanvasDockWidget( const QString &name, QWidget *parent = nullptr );

    /**
     * Sets the main app map canvas.
     */
    void setMainCanvas( QgsMapCanvas *canvas );

    /**
     * Returns the map canvas contained in the dock widget.
     */
    QgsMapCanvas *mapCanvas();

    /**
     * Sets whether the view center should be synchronized with the main canvas center.
     * \see isViewCenterSynchronized()
     */
    void setViewCenterSynchronized( bool enabled );

    /**
     * Returns true if the view extent is synchronized with the main canvas extent.
     * \see setViewCenterSynchronized()
     */
    bool isViewCenterSynchronized() const;

    /**
     * Sets whether the cursor position marker is visible.
     * \see isCursorMarkerVisible()
     */
    void setCursorMarkerVisible( bool visible );

    /**
     * Returns true if the cursor position marker is visible.
     * \see setCursorMarkerVisible()
     */
    bool isCursorMarkerVisible() const;

    /**
     * Sets whether the main canvas extent is visible.
     * \see isMainCanvasExtentVisible()
     */
    void setMainCanvasExtentVisible( bool visible );

    /**
     * Returns true if the main canvas extent is visible.
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
     * Returns true if the view scale is synchronized with the main canvas extent.
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
    void settingsMenuAboutToShow();
    void syncMarker( const QgsPointXY &p );
    void mapScaleChanged();
    void updateExtentRect();
    void showLabels( bool show );


  private:

    QgsMapCanvas *mMapCanvas = nullptr;
    QgsMapCanvas *mMainCanvas = nullptr;
    QMenu *mMenu = nullptr;
    QList<QAction *> mMenuPresetActions;
    QCheckBox *mSyncExtentCheckBox = nullptr;
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
    void syncViewCenter( QgsMapCanvas *sourceCanvas );
};

/**
 * \class QgsMapSettingsAction
 * Allows embedding a scale, rotation and other map settings into a menu.
 * \since QGIS 3.0
 */

class QgsMapSettingsAction: public QWidgetAction
{
    Q_OBJECT

  public:

    QgsMapSettingsAction( QWidget *parent = nullptr );

    QCheckBox *syncExtentCheckBox() { return mSyncExtentCheckBox; }
    QgsScaleComboBox *scaleCombo() { return mScaleCombo; }
    QgsDoubleSpinBox *rotationSpinBox() { return mRotationWidget; }
    QgsDoubleSpinBox *magnifierSpinBox() { return mMagnifierWidget; }
    QgsDoubleSpinBox *scaleFactorSpinBox() { return mScaleFactorWidget; }
    QCheckBox *syncScaleCheckBox() { return mSyncScaleCheckBox; }

  private:
    QCheckBox *mSyncExtentCheckBox = nullptr;
    QgsScaleComboBox *mScaleCombo = nullptr;
    QgsDoubleSpinBox *mRotationWidget = nullptr;
    QgsDoubleSpinBox *mMagnifierWidget = nullptr;
    QCheckBox *mSyncScaleCheckBox = nullptr;
    QgsDoubleSpinBox *mScaleFactorWidget = nullptr;
};


#endif // QGSMAPCANVASDOCKWIDGET_H
