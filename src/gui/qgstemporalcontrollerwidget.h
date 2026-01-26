/***************************************************************************
                         qgstemporalcontrollerwidget.h
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

#ifndef QGSTEMPORALCONTROLLERWIDGET_H
#define QGSTEMPORALCONTROLLERWIDGET_H

#include "ui_qgstemporalcontrollerwidgetbase.h"

#include "qgis_gui.h"
#include "qgsrange.h"
#include "qgstemporalnavigationobject.h"

class QgsMapLayer;
class QgsMapLayerModel;
class QgsTemporalNavigationObject;
class QgsTemporalController;
class QgsInterval;

/**
 * \ingroup gui
 * \brief A widget for controlling playback properties of a QgsTemporalController.
 *
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsTemporalControllerWidget : public QgsPanelWidget, private Ui::QgsTemporalControllerWidgetBase
{
    Q_OBJECT
  public:
    /**
      * Constructor for QgsTemporalControllerWidget, with the specified \a parent widget.
      */
    QgsTemporalControllerWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the temporal controller object used by this object in navigation.
     *
     * The dock widget retains ownership of the returned object.
     */
    QgsTemporalNavigationObject *temporalController();

    bool applySizeConstraintsToStack() const override;

#ifndef SIP_RUN

  signals:

    /**
     * Triggered when an animation should be exported
     */
    void exportAnimation();

#endif

  protected:
    void keyPressEvent( QKeyEvent *e ) override;

  private:
    //! Handles all non-GUI navigation logic
    QgsTemporalNavigationObject *mNavigationObject = nullptr;

    int mBlockSettingUpdates = 0;
    int mBlockFrameDurationUpdates = 0;

    bool mHasTemporalLayersLoaded = false;

    std::unique_ptr<QMenu> mRangeMenu;
    QAction *mRangeSetToProjectAction = nullptr;
    QAction *mRangeSetToAllLayersAction = nullptr;

    std::unique_ptr<QMenu> mRangeLayersSubMenu;
    QgsMapLayerModel *mMapLayerModel = nullptr;

    void firstTemporalLayerLoaded( QgsMapLayer *layer );
    void setTimeStep( const QgsInterval &timeStep );

    /**
     * Updates the widget timestep and timestep unit inputs using the passed
     * interval \a timeStep original duration and original unit.
     *
     * If the passed interval \a timeStep has different values of original duration
     * and original unit compared to the widget input values for timestep and
     * timestep unit then the corresponding widget input values will be updated
     * to match the \a timeStep original duration and unit.
     *
     * After updating the widget inputs, an update to the frame duration is executed.
     *
     * \note Updates will only be made if the \a timeStep is valid.
     *
     * \since QGIS 3.18
     */
    void updateTimeStepInputs( const QgsInterval &timeStep );

  private slots:

    /**
     * Handles the action to be done when the
     * time slider value has changed.
     */
    void timeSlider_valueChanged( int value );

    /**
     * Loads a temporal map settings dialog.
     */
    void settings_clicked();

    /**
     * Set date widgets to match the given \a range.
     */
    void setDates( const QgsDateTimeRange &range );

    /**
     * Updates the controller dates time inputs using the full range of all layers.
     */
    void setDatesToAllLayers();

    /**
     * Updates the controller dates time inputs from the preset project range. If
     * that isn't defined, the range will fallback to the full range of all
     * layers.
     */
    void setDatesToProjectTime( bool tryLastStoredRange );

    /**
     * Updates the value of the slider
     */
    void updateSlider( const QgsDateTimeRange &range );

    void totalMovieFramesChanged( long long frames );

    /**
     * Updates the current range label
     */
    void updateRangeLabel( const QgsDateTimeRange &range );

    /**
     * Updates the navigation temporal extent.
     */
    void updateTemporalExtent();

    /**
     * Updates the navigation frame duration.
     */
    void updateFrameDuration();

    void setWidgetStateFromProject();

    void mNavigationOff_clicked();
    void mNavigationFixedRange_clicked();
    void mNavigationAnimated_clicked();
    void mNavigationMovie_clicked();
    void setWidgetStateFromNavigationMode( Qgis::TemporalNavigationMode mode );

    void onLayersAdded( const QList<QgsMapLayer *> &layers );
    void onProjectCleared();

    void startEndDateTime_changed();
    void fixedRangeStartEndDateTime_changed();

    void saveRangeToProject();

    void aboutToShowRangeMenu();

    void mRangeSetToProjectAction_triggered();
    void mRangeSetToAllLayersAction_triggered();
};

#endif // QGSTEMPORALCONTROLLERWIDGET_H
