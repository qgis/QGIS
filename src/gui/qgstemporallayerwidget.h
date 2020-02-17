/***************************************************************************
                         qgstemporallayerwidget.h
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

#ifndef QGSTEMPORALLAYERWIDGET_H
#define QGSTEMPORALLAYERWIDGET_H

#include "ui_qgstemporallayerwidgetbase.h"
#include "qgis_gui.h"
#include "qgsmapcanvas.h"
#include "qgsrasterlayer.h"


class QgsMapLayer;

/**
 * \ingroup gui
 * \class QgsTemporalLayerWidget
 * A widget for filtering temporal layer based on its available time values
 *
 * \since QGIS 3.14
 */

class GUI_EXPORT QgsTemporalLayerWidget : public QWidget, private Ui::QgsTemporalLayerWidgetBase
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsTemporalLayerWidget.
     */
    QgsTemporalLayerWidget( QWidget *parent = nullptr, QgsMapLayer *layer = nullptr );

    /**
     * Sets the map canvas associtated with this temporal widget.
     *
     */
    void setMapCanvas( QgsMapCanvas *canvas );

    /**
     * Save widget temporal properties inputs.
     */
    void saveTemporalProperties();

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
     *  Handles actions to follow when layer radio button is clicked.
     **/
    void layerRadioButton_clicked();

    /**
     *  Handles actions to follow when project radio button is clicked.
     **/
    void projectRadioButton_clicked();

    /**
     *  Handles actions to follow when reference checkbox is clicked.
     **/
    void referenceCheckBox_clicked();

    /**
     * Sets the input widgets enable state in this temporal widget.
     *
     * \param type widget type
     * \param enabled new enable status
     */
    void setInputWidgetState( QString type, bool enabled );

    /**
     * Updates the range label with current set datetime range.
     *
     **/
    void updateRangeLabel( QString type, QLabel *label );

    /**
     * Sets the temporal date time inputs with layer's lower and upper
     * temporal range limits.
     *
     **/
    void setDateTimeInputsLimit();

  private:

    /**
     * Initialize the widget with default state.
     */
    void init();

    /**
     * Map canvas associtated with this temporal widget.
     *
     * This can be used to get current project map settings.
     */
    QgsMapCanvas *mCanvas = nullptr;

    /**
     * Update temporal layer Uri
     *
     */
    QString updateTemporalDataSource( QString sourceUri, QgsDateTimeRange sourceRange );

    /**
     * The corresponding map layer with temporal attributes
     */
    QgsMapLayer *mLayer = nullptr;


};
#endif // QGSTEMPORALLAYERWIDGET_H
