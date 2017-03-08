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

#include <ui_qgsmapcanvasdockwidgetbase.h>

#include "qgsdockwidget.h"
#include "qgis_app.h"
#include <QWidgetAction>

class QgsMapCanvas;
class QgsScaleComboBox;
class QgsDoubleSpinBox;
class QgsStatusBarMagnifierWidget;

/**
 * \class QgsMapCanvasDockWidget
 * A dock widget with an embedded map canvas, for additional map views.
 * \note added in QGIS 3.0
 */
class APP_EXPORT QgsMapCanvasDockWidget : public QgsDockWidget, private Ui::QgsMapCanvasDockWidgetBase
{
    Q_OBJECT
  public:
    explicit QgsMapCanvasDockWidget( const QString &name, QWidget *parent = nullptr );

    /**
     * Sets the main app map canvas.
     */
    void setMainCanvas( QgsMapCanvas *canvas ) { mMainCanvas = canvas; }

    /**
     * Returns the map canvas contained in the dock widget.
     */
    QgsMapCanvas *mapCanvas();

    /**
     * Closes the dock, bypassing the usual warning prompt.
     */
    void closeWithoutWarning();

  protected:

    virtual void closeEvent( QCloseEvent *event ) override;

  private slots:

    void setMapCrs();
    void syncView( bool enabled );
    void mapExtentChanged();

  private:

    QgsMapCanvas *mMapCanvas = nullptr;
    QgsMapCanvas *mMainCanvas = nullptr;
    bool mShowCloseWarning = true;
    QgsScaleComboBox *mScaleCombo = nullptr;
    QgsDoubleSpinBox *mRotationEdit = nullptr;
    QgsDoubleSpinBox *mMagnificationEdit = nullptr;
    bool mBlockScaleUpdate = false;
    bool mBlockRotationUpdate = false;
    bool mBlockMagnificationUpdate = false;
};

/**
 * \class QgsMapSettingsAction
 * Allows embedding a scale, rotation and other map settings into a menu.
 * \note added in QGIS 3.0
 */

class QgsMapSettingsAction: public QWidgetAction
{
    Q_OBJECT

  public:

    QgsMapSettingsAction( QWidget *parent = nullptr );

    QgsScaleComboBox *scaleCombo() { return mScaleCombo; }
    QgsDoubleSpinBox *rotationSpinBox() { return mRotationWidget; }
    QgsDoubleSpinBox *magnifierSpinBox() { return mMagnifierWidget; }

  private:
    QgsScaleComboBox *mScaleCombo = nullptr;
    QgsDoubleSpinBox *mRotationWidget = nullptr;
    QgsDoubleSpinBox *mMagnifierWidget = nullptr;
};


#endif // QGSMAPCANVASDOCKWIDGET_H
