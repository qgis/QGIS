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
#include "qgsrasterpipe.h"
#include "qgssymbolwidgetcontext.h"

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
class GUI_EXPORT QgsRasterTransparencyWidget : public QgsMapLayerConfigWidget, private QgsExpressionContextGenerator, public Ui::QgsRasterTransparencyWidget
{
    Q_OBJECT
  public:

    /**
     * \brief Widget to control a layers transparency and related options
     */
    QgsRasterTransparencyWidget( QgsRasterLayer *layer, QgsMapCanvas *canvas, QWidget *parent = nullptr );

    /**
     * Sets the \a context in which the dialog is shown, e.g., the associated map canvas and expression contexts.
     * \since QGIS 3.22
     */
    void setContext( const QgsSymbolWidgetContext &context );

    QgsExpressionContext createExpressionContext() const override;

    /**
     * Returns the (possibly nullptr) map pixel selector tool.
     * \since QGIS 3.22
     */
    QgsMapToolEmitPoint *pixelSelectorTool() const;

  public slots:

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"
#endif

    /**
     * Sync the widget state to the layer set for the widget.
     */
    void syncToLayer();
#ifdef __clang__
#pragma clang diagnostic pop
#endif

    /**
     * Apply any changes on the widget to the set layer.
     */
    void apply() override;

  protected:

#ifndef SIP_RUN

    // TODO -- consider moving these to a common raster widget base class

    /**
     * Registers a property override button, setting up its initial value, connections and description.
     * \param button button to register
     * \param key corresponding data defined property key
     * \note Not available in Python bindings
     * \since QGIS 3.22
     */
    void initializeDataDefinedButton( QgsPropertyOverrideButton *button, QgsRasterPipe::Property key );

    /**
     * Updates all property override buttons to reflect the widgets's current properties.
     * \note Not available in Python bindings
     * \since QGIS 3.22
     */
    void updateDataDefinedButtons();

    /**
     * Updates a specific property override \a button to reflect the widgets's current properties.
     * \note Not available in Python bindings
     * \since QGIS 3.22
     */
    void updateDataDefinedButton( QgsPropertyOverrideButton *button );

    //! Temporary property collection
    QgsPropertyCollection mPropertyCollection;

#endif

  private slots:

    void updateProperty();

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

    //! Context in which widget is shown
    QgsSymbolWidgetContext mContext;
};
#endif // QGSRASTERTRANSPARENCYWIDGET_H
