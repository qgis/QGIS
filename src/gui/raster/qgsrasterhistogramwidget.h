/***************************************************************************
                         qgsrasterrendererwidget.h
                         ---------------------------
    begin                : July 2012
    copyright            : (C) 2012 by Etienne Tourigny
    email                : etourigny dot dev at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRASTERHISTOGRAMWIDGET_H
#define QGSRASTERHISTOGRAMWIDGET_H

#include "ui_qgsrasterhistogramwidgetbase.h"
#include "qgis_sip.h"
#include "qgis.h"

#include "qgsmaplayerconfigwidget.h"
#include "qgis_gui.h"

class QgsRasterLayer;
class QgsRasterRendererWidget;
class QwtPlotPicker;
class QwtPlotMarker;
class QwtPlotZoomer;

// fix for qwt5/qwt6 QwtDoublePoint vs. QPointF
typedef QPointF QwtDoublePoint SIP_SKIP;

/**
 * \ingroup gui
 * \brief Histogram widget
  */

class GUI_EXPORT QgsRasterHistogramWidget : public QgsMapLayerConfigWidget, private Ui::QgsRasterHistogramWidgetBase
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsRasterHistogramWidget, for the specified raster \a layer.
     */
    QgsRasterHistogramWidget( QgsRasterLayer *layer, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    //! Save the histogram as an image to disk
    bool histoSaveAsImage( const QString &filename, int width = 600, int height = 600, int quality = -1 );

    //! Sets the renderer widget (or just its name if there is no widget)
    void setRendererWidget( const QString &name, QgsRasterRendererWidget *rendererWidget = nullptr );

    //! Activate the histogram widget
    void setActive( bool activeFlag );

    //! \brief Compute the histogram on demand.
    bool computeHistogram( bool forceComputeFlag );

    //! Apply a histoActionTriggered() event.
    void histoAction( const QString &actionName, bool actionFlag = true );

    //! Apply a histoActionTriggered() event.
    void setSelectedBand( int index );

  public slots:
    //! \brief slot executed when user wishes to refresh raster histogramwidget
    void refreshHistogram();

    void apply() override;

  private slots:
    //! This slot lets you save the histogram as an image to disk
    void mSaveAsImageButton_clicked();
    //! Used when the histogram band selector changes, or when tab is loaded.
    void cboHistoBand_currentIndexChanged( int );
    //! Applies the selected min/max values to the renderer widget.
    void applyHistoMin();
    void applyHistoMax();
    //! Button to activate picking of the min/max value on the graph.
    void btnHistoMin_toggled();
    void btnHistoMax_toggled();
    //! Called when a selection has been made using the plot picker.
    void histoPickerSelected( QPointF );

    /**
     * Called when a selection has been made using the plot picker (for qwt5 only).
     * \note not available in Python bindings
      */
    void histoPickerSelectedQwt5( QwtDoublePoint ) SIP_SKIP;
    //! Various actions that are stored in btnHistoActions.
    void histoActionTriggered( QAction * );
    //! Draw the min/max markers on the histogram plot.
    void updateHistoMarkers();
    //! Button to compute the histogram, appears when no cached histogram is available.
    void btnHistoCompute_clicked();

  private:
    enum HistoShowBands
    {
      ShowAll = 0,
      ShowSelected = 1,
      ShowRGB = 2
    };

    //! \brief Pointer to the raster layer that this property dilog changes the behavior of.
    QgsRasterLayer *mRasterLayer = nullptr;
    //! \brief Pointer to the renderer widget, to get/set min/max.
    QgsRasterRendererWidget *mRendererWidget = nullptr;
    //! \brief Name of the renderer widget (see QgsRasterRendererRegistry).
    QString mRendererName;

    QwtPlotPicker *mHistoPicker = nullptr;
    QwtPlotZoomer *mHistoZoomer = nullptr;
    QwtPlotMarker *mHistoMarkerMin = nullptr;
    QwtPlotMarker *mHistoMarkerMax = nullptr;
    double mHistoMin;
    double mHistoMax;
    QVector<QColor> mHistoColors;
    bool mHistoShowMarkers;
    bool mHistoZoomToMinMax;
    bool mHistoUpdateStyleToMinMax;
    bool mHistoDrawLines;
    /* bool mHistoLoadApplyAll; */
    HistoShowBands mHistoShowBands;
    //! \brief Returns a list of selected bands in the histogram widget- or empty if there is no selection restriction.
    QList<int> histoSelectedBands();
    //! \brief Returns a list of selected bands in the renderer widget.
    QList<int> rendererSelectedBands();
    QPair<QString, QString> rendererMinMax( int bandNo );
};
#endif
