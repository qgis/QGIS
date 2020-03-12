/***************************************************************************
                         qgstemporalcontrollerdockwidget.h
                         ---------------
    begin                : February 2020
    copyright            : (C) 2020 by Samweli Mwakisambwe
    email                : samweli at kartoza dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTEMPORALCONTROLLERDOCKWIDGET_H
#define QGSTEMPORALCONTROLLERDOCKWIDGET_H

#include "ui_qgstemporalcontrollerdockwidgetbase.h"

#include "qgsdockwidget.h"
#include "qgis_gui.h"
#include "qgsrange.h"

class QgsMapLayer;
class QgsTemporalNavigationObject;
class QgsTemporalMapSettingsWidget;
class QgsTemporalMapSettingsDialog;
class QgsTemporalController;
class QgsInterval;

/**
 * \ingroup gui
 * The QgsTemporalControllerDockWidget class
 *
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsTemporalControllerDockWidget : public QgsDockWidget, private Ui::QgsTemporalControllerDockWidgetBase
{
    Q_OBJECT
  public:

    /**
      * Constructor for QgsTemporalControllerDockWidget, with the specified \a parent widget.
      */
    QgsTemporalControllerDockWidget( const QString &name, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the temporal controller object used by this object in navigation.
     *
     * The dock widget retains ownership of the returned object.
     */
    QgsTemporalController *temporalController();

  private:

    /**
     * Updates the controller widget navigation buttons enabled status.
     */
    void updateButtonsEnable( bool enabled );

    /**
     * Sets the enable status of the widget date time inputs.
     **/
    void setDateInputsEnable( bool enabled );

    //! Handles all non gui navigation logic
    QgsTemporalNavigationObject *mNavigationObject = nullptr;

  private slots:

    /**
     * Handles the action to be done when the
     * time slider value has changed.
     **/
    void timeSlider_valueChanged( int value );

    /**
     * Loads a temporal map settings dialog.
     **/
    void settings_clicked();

    /**
     * Updates the controller dates time inputs.
     */
    void setDatesToProjectTime();

    /**
     * Updates the value of the slider
     **/
    void updateSlider( const QgsDateTimeRange &range );

    /**
     * Updates the current range label
     **/
    void updateRangeLabel( const QgsDateTimeRange &range );

    /**
     * Updates the navigation temporal extent.
     **/
    void updateTemporalExtent();

    /**
     * Updates the navigation frame duration.
     **/
    void updateFrameDuration();

};

#endif // QGSTEMPORALCONTROLLERDOCKWIDGET_H
