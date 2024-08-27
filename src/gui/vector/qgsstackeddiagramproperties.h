/***************************************************************************
  qgsstackeddiagramproperties.h
  Properties for stacked diagram layers
  -------------------
         begin                : August 2024
         copyright            : (C) Germán Carrillo
         email                : german at opengis dot ch

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSTACKEDDIAGRAMPROPERTIES_H
#define QGSSTACKEDDIAGRAMPROPERTIES_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "qgis_gui.h"
#include "ui_qgsstackeddiagrampropertiesbase.h"

#include <QWidget>
#include <QDialog>

class QgsVectorLayer;
class QgsMapCanvas;
class QgsDiagramProperties;


/**
 * \ingroup gui
 * \class QgsStackedDiagramProperties
 *
 * \since QGIS 3.40
 */
class GUI_EXPORT QgsStackedDiagramProperties : public QWidget, private Ui::QgsStackedDiagramPropertiesBase
{
    Q_OBJECT

  public:
    explicit QgsStackedDiagramProperties( QgsVectorLayer *layer, QWidget *parent, QgsMapCanvas *canvas );

    /**
     * Updates the widget to reflect the layer's current diagram settings.
     */
    void syncToLayer();

  signals:
    void auxiliaryFieldCreated();

  public slots:
    void apply();
    void mDiagramTypeComboBox_currentIndexChanged( int index );

  private slots:

    /**
     * Adds a diagram tab to the current QgsStackedDiagramProperties.
     *
     * \since QGIS 3.40
     */
    void addSubDiagram();

    /**
     * Removes a diagram tab from the current QgsStackedDiagramProperties.
     * Diagram tabs are removed only if the tab count is greeater than 2.
     * Tab texts are adjusted after tab removal, to keep sequential order.
     *
     * \since QGIS 3.40
     */
    void removeSubDiagram();

  private:
    QgsVectorLayer *mLayer = nullptr;
    QgsMapCanvas *mMapCanvas = nullptr;

};

#endif // QGSSTACKEDDIAGRAMPROPERTIES_H
