/***************************************************************************
    qgsrendererrasterpropertieswidget.h
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
#ifndef QGSRENDERERRASTERPROPERTIESDIALOG_H
#define QGSRENDERERRASTERPROPERTIESDIALOG_H

#include <QObject>
#include <QDialog>

#include "ui_qgsrendererrasterpropswidgetbase.h"

#include "qgsmaplayerconfigwidget.h"
#include "qgsresamplingutils.h"
#include "qgis_gui.h"


class QgsRasterLayer;
class QgsMapCanvas;
class QgsRasterRendererWidget;

/**
 * \ingroup gui
 * \class QgsRendererRasterPropertiesWidget
 */
class GUI_EXPORT QgsRendererRasterPropertiesWidget : public QgsMapLayerConfigWidget, private Ui::QgsRendererRasterPropsWidgetBase
{
    Q_OBJECT

  public:
    /**
     * A widget to hold the renderer properties for a raster layer.
     * \param layer The raster layer to style
     * \param canvas The canvas object used to calculate the max and min values from the extent.
     * \param parent Parent object
     */
    QgsRendererRasterPropertiesWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, QWidget *parent = nullptr );

    /**
     * Sets the map canvas associated with the dialog. This allows the widget to retrieve the current
     * map scale and other properties from the canvas.
     * \param canvas map canvas
     */
    void setMapCanvas( QgsMapCanvas *canvas );

    /**
     * Returns the active render widget. Can be NULLPTR.
     */
    QgsRasterRendererWidget *currentRenderWidget() { return mRendererWidget; }

  public slots:
    //! called when user changes renderer type
    void rendererChanged();

    //! Apply the changes from the dialog to the layer.
    void apply() override;


#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#endif

    /**
     * \brief Sync the widget to the given layer.
     * \param layer The layer to use for the widget
     */
    void syncToLayer( QgsRasterLayer *layer );
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

  private slots:
    //! Slot to reset all color rendering options to default
    void mResetColorRenderingBtn_clicked();

    //! Enable or disable saturation controls depending on choice of grayscale mode
    void toggleSaturationControls( int grayscaleMode );

    //! Enable or disable colorize controls depending on checkbox
    void toggleColorizeControls( bool colorizeEnabled );

    void refreshAfterStyleChanged();

    /**
     * updates gamma spinbox on slider changes
     * \since QGIS 3.16
     */
    void updateGammaSpinBox( int value );

    /**
     * updates gamma slider on spinbox changes
     * \since QGIS 3.16
     */
    void updateGammaSlider( double value );

  private:
    static void initRendererWidgetFunctions();
    void setRendererWidget( const QString &rendererName );

    QgsRasterLayer *mRasterLayer = nullptr;
    QgsRasterRendererWidget *mRendererWidget = nullptr;

    QgsResamplingUtils mResamplingUtils;
};

#endif // QGSRENDERERRASTERPROPERTIESDIALOG_H
