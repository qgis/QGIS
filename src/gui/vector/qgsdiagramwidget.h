/***************************************************************************
  qgsdiagramwidget.h
  Container widget for diagram layers
  -------------------
         begin                : September 2024
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

#ifndef QGSDIAGRAMWIDGET_H
#define QGSDIAGRAMWIDGET_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include <qgsmaplayerconfigwidget.h>
#include "ui_qgsdiagramwidget.h"

#include <QWidget>

/**
 * \ingroup gui
 * \class QgsDiagramWidget
 *
 * \since QGIS 3.40
 */
class GUI_EXPORT QgsDiagramWidget : public QgsMapLayerConfigWidget, private Ui::QgsDiagramWidget
{
    Q_OBJECT

  public:
    //! constructor
    QgsDiagramWidget( QgsVectorLayer *layer, QgsMapCanvas *canvas, QWidget *parent = nullptr );

    /**
     * Updates the widget to reflect the layer's current diagram settings.
     */
    void syncToOwnLayer();

  public slots:
    //! Saves the labeling configuration and immediately updates the map canvas to reflect the changes
    void apply() override;

  signals:
    //! Emitted when an auxiliary field is created
    void auxiliaryFieldCreated();

  private slots:
    void mDiagramTypeComboBox_currentIndexChanged( int index );
    void showEngineConfigDialog();

  private:
    enum Mode
    {
      ModeNone,
      ModePie,
      ModeText,
      ModeHistogram,
      ModeStackedBar,
      ModeStacked
    };

    QgsVectorLayer *mLayer = nullptr;
    QgsMapCanvas *mCanvas = nullptr;

    QWidget *mWidget = nullptr;
};

#endif // QGSDIAGRAMWIDGET_H
