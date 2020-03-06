/***************************************************************************
                         qgstemporalvcrdockwidget.h
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

#ifndef QGSTEMPORALVCRDOCKWIDGET_H
#define QGSTEMPORALVCRDOCKWIDGET_H

#include "ui_qgstemporalvcrdockwidgetbase.h"

#include "qgsdockwidget.h"
#include "qgis_gui.h"
#include "qgsrange.h"

class QgsMapLayer;
class QgsTemporalNavigationObject;

/**
 * \ingroup gui
 * The QgsTemporalVcrDockWidget class
 *
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsTemporalVcrDockWidget : public QgsDockWidget, private Ui::QgsTemporalVcrDockWidgetBase
{
    Q_OBJECT
  public:

    /**
      * Constructor for QgsTemporalVcrDockWidget
      *
      */
    QgsTemporalVcrDockWidget( const QString &name, QWidget *parent = nullptr );

    ~QgsTemporalVcrDockWidget() override;

  private:

    /**
     * Initialize the widget with default state.
     */
    void init();

    /**
     * Updates the VCR dates time inputs.
     * Checks if it should update the inputs using project time settings.
     */
    void updateDatesLabels( bool useProjectTime );

    /**
     * Sets the VCR widget time slider.
     */
    void setSliderRange();

    /**
     * Updates the VCR widget navigation buttons enabled status.
     */
    void updateButtonsEnable( bool enabled );

    /**
     * Sets the enable status of the widget date time inputs.
     **/
    void setDateInputsEnable( bool enabled );

    //! Timer to set navigation time interval
    QTimer *mTimer = nullptr;

    QgsTemporalNavigationObject *mNavigationObject = nullptr;

  private slots:

    /**
     * Handles the action to be done when the
     * previous button on the widget is clicked.
     **/
    void previousButton_clicked();

    /**
     * Handles the action to be done when the
     * next button on the widget is clicked.
     **/
    void nextButton_clicked();

    /**
     * Handles the action to be done when the
     * stop button on the widget is clicked.
     **/
    void stopButton_clicked();

    /**
     * Handles the action to be done when the
     * back button on the widget is clicked.
     **/
    void backButton_clicked();

    /**
     * Handles the action to be done when the
     * forward button on the widget is clicked.
     **/
    void forwardButton_clicked();

    /**
     * Handles the action to be done when the
     * spin box value on the widget has changed.
     **/
    void spinBox_valueChanged( int value );

    /**
     * Handles the action to be done when the
     * time steps combo box index has changed.
     **/
    void timeStepsComboBox_currentIndexChanged( int index );

    /**
     * Handles the action to be done when the
     * mode combo box index has changed.
     **/
    void modeComboBox_currentIndexChanged( int index );

    /**
     * Handles the action to be done when the
     * time slider value has changed.
     **/
    void timeSlider_valueChanged( int value );

    /**
     * Called when this widget timer has timeout.
     **/
    void timerTimeout();

    /**
     * Handles the input change on the start date time input.
     **/
    void startDateTime_changed( const QDateTime &datetime );

    /**
     *Handles the input change on the end date time input.
     **/
    void endDateTime_changed( const QDateTime &datetime );

};

#endif // QGSTEMPORALVCRDOCKWIDGET_H
