/***************************************************************************
                         qgsrasterlayertemporalpropertieswidget.h
                         ------------------------------
    begin                : January 2020
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

#ifndef QGSRASTERLAYERTEMPORALPROPERTIESWIDGET_H
#define QGSRASTERLAYERTEMPORALPROPERTIESWIDGET_H

#include "ui_qgsrasterlayertemporalpropertieswidgetbase.h"
#include "qgis_gui.h"

class QgsRasterLayer;

/**
 * \ingroup gui
 * \class QgsRasterLayerTemporalPropertiesWidget
 * A widget for configuring the temporal properties for a raster layer.
 *
 * \since QGIS 3.14
 */

class GUI_EXPORT QgsRasterLayerTemporalPropertiesWidget : public QWidget, private Ui::QgsRasterLayerTemporalPropertiesWidgetBase
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsRasterLayerTemporalPropertiesWidget.
     */
    QgsRasterLayerTemporalPropertiesWidget( QWidget *parent = nullptr, QgsRasterLayer *layer = nullptr );

    /**
     * Save widget temporal properties inputs.
     */
    void saveTemporalProperties();

  private:

    /**
     * Initialize the widget with default state.
     */
    void init();

    /**
     * The corresponding map layer with temporal attributes
     */
    QgsRasterLayer *mLayer = nullptr;

    /**
     * Mode used to determine if temporal properties dimensional status.
     */
    enum TemporalDimension
    {
      NormalTemporal, //! When temporal properties have single temporal dimension.

      /**
       * When temporal properties have bi-temporal dimension,
       * eg. have normal time and reference time or reference time only.
       */
      BiTemporal
    };

  private slots:

    /**
     *  Sets the input end datetime the same as start datetime input.
     **/
    void setEndAsStartNormalButton_clicked();

    /**
     * Sets the reference input end datetime the same as start datetime input, from
     * advance options.
     */
    void setEndAsStartReferenceButton_clicked();

    /**
     *  Handles actions to follow when layer radio button is toggled.
     **/
    void layerRadioButton_toggled( bool checked );

    /**
     * Updates the ui states to show current project temporal range, which is
     * intended to be assigned to the layer
     **/
    void projectRadioButton_toggled( bool checked );

    /**
     *  Resets the datetimes inputs to the layer's fixed temporal range.
     **/
    void resetDatesButton_clicked();

    /**
     *  Enabled inputs in reference datetimes group.
     **/
    void referenceCheckBox_clicked();

    /**
     * Sets the input widgets enable state in this temporal widget.
     *
     * \param dimension determine to either enable normal time or reference time.
     * \param enabled new enable status
     */
    void setInputWidgetState( TemporalDimension dimension, bool enabled );

    /**
     * Updates the range label with current set datetime range.
     *
     **/
    void updateRangeLabel( QLabel *label );

    /**
     * Sets the temporal date time inputs with layer's lower and upper
     * temporal range limits.
     *
     **/
    void setDateTimeInputsLimit();

    /**
     * Sets the temporal date time inputs with the default
     * locale from the system.
     *
     **/
    void setDateTimeInputsLocale();

};
#endif // QGSRASTERLAYERTEMPORALPROPERTIESWIDGET_H
