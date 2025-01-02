/***************************************************************************
                         qgsvectorlayertemporalpropertieswidget.h
                         ------------------------------
    begin                : May 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORLAYERTEMPORALPROPERTIESWIDGET_H
#define QGSVECTORLAYERTEMPORALPROPERTIESWIDGET_H

#include "ui_qgsvectorlayertemporalpropertieswidgetbase.h"
#include "qgsexpressioncontextgenerator.h"
#include "qgis_gui.h"

class QgsVectorLayer;

/**
 * \ingroup gui
 * \class QgsVectorLayerTemporalPropertiesWidget
 * \brief A widget for configuring the temporal properties for a vector layer.
 *
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsVectorLayerTemporalPropertiesWidget : public QWidget, public QgsExpressionContextGenerator, private Ui::QgsVectorLayerTemporalPropertiesWidgetBase
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsVectorLayerTemporalPropertiesWidget.
     */
    QgsVectorLayerTemporalPropertiesWidget( QWidget *parent = nullptr, QgsVectorLayer *layer = nullptr );

    /**
     * Save widget temporal properties inputs.
     */
    void saveTemporalProperties();

    QgsExpressionContext createExpressionContext() const override;

    /**
     * Updates the widget state to match the current layer state.
     */
    void syncToLayer();

  private:
    /**
     * The corresponding map layer with temporal attributes
     */
    QgsVectorLayer *mLayer = nullptr;
};
#endif // QGSVECTORLAYERTEMPORALPROPERTIESWIDGET_H
