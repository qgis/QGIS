/***************************************************************************
    qgsrastertransparencywidget.h
    ---------------------
    begin                : May 2016
    copyright            : (C) 2016 by Nathan Woodrow
    email                : woodrow dot nathan at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRASTERTRANSPARENCYWIDGET_H
#define QGSRASTERTRANSPARENCYWIDGET_H

#include <QWidget>

#include "ui_qgsrastertransparencywidget.h"

#include "qgsmaplayerconfigwidget.h"
#include "qgis_gui.h"

class QgsRasterLayer;
class QgsRasterRenderer;
class QgsMapCanvas;
class QgsMapToolEmitPoint;
class QgsPointXY;


/**
 * \ingroup gui
 * \brief Widget to control a layers transparency and related options
 */
class GUI_EXPORT QgsRasterTransparencyWidget : public QgsMapLayerConfigWidget, private Ui::QgsRasterTransparencyWidget
{
    Q_OBJECT
  public:

    /**
     * \brief Widget to control a layers transparency and related options
     */
    QgsRasterTransparencyWidget( QgsRasterLayer *layer, QgsMapCanvas *canvas, QWidget *parent = nullptr );

  public slots:

    /**
     * Sync the widget state to the layer set for the widget.
     */
    void syncToLayer();

    /**
     * Apply any changes on the widget to the set layer.
     */
    void apply() override;

  private slots:

    void pixelSelected( const QgsPointXY &canvasPoint );

    //! Transparency cell changed
    void transparencyCellTextEdited( const QString &text );

    //! \brief slot executed when user presses "Add Values From Display" button on the transparency page
    void pbnAddValuesFromDisplay_clicked();

    //! \brief slot executed when user presses "Add Values Manually" button on the transparency page
    void pbnAddValuesManually_clicked();

    //! \brief slot executed when user wishes to reset noNoDataValue and transparencyTable to default value
    void pbnDefaultValues_clicked();

    //! \brief slot executed when user wishes to export transparency values
    void pbnExportTransparentPixelValues_clicked();

    //! \brief slow executed when user wishes to import transparency values
    void pbnImportTransparentPixelValues_clicked();
    //! \brief slot executed when user presses "Remove Selected Row" button on the transparency page
    void pbnRemoveSelectedRow_clicked();

  private:
    //! \brief  A constant that signals property not used
    const QString TRSTRING_NOT_SET;

    bool rasterIsMultiBandColor();

    //! \brief Clear the current transparency table and populate the table with the correct types for current drawing mode and data type
    void populateTransparencyTable( QgsRasterRenderer *renderer );

    void setupTransparencyTable( int nBands );

    void setTransparencyCell( int row, int column, double value );

    void adjustTransparencyCellWidth( int row, int column );

    void setTransparencyToEdited( int row );

    double transparencyCellValue( int row, int column );

    QgsRasterLayer *mRasterLayer = nullptr;

    QgsMapCanvas *mMapCanvas = nullptr;

    QgsMapToolEmitPoint *mPixelSelectorTool = nullptr;

    QVector<bool> mTransparencyToEdited;
};
#endif // QGSRASTERTRANSPARENCYWIDGET_H
