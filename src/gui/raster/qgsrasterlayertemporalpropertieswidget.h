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
class QgsMapLayerConfigWidget;

/**
 * \ingroup gui
 * \class QgsRasterLayerTemporalPropertiesWidget
 * \brief A widget for configuring the temporal properties for a raster layer.
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

    /**
     * Updates the widget state to match the current layer state.
     */
    void syncToLayer();

    /**
     * Adds a child \a widget to the properties widget.
     *
     * \since QGIS 3.20
     */
    void addWidget( QgsMapLayerConfigWidget *widget SIP_TRANSFER );

  private slots:
    void temporalGroupBoxChecked( bool checked );

  private:

    /**
     * The corresponding map layer with temporal attributes
     */
    QgsRasterLayer *mLayer = nullptr;
    QVBoxLayout *mExtraWidgetLayout = nullptr;

    QList< QgsMapLayerConfigWidget * > mExtraWidgets;

};
#endif // QGSRASTERLAYERTEMPORALPROPERTIESWIDGET_H
