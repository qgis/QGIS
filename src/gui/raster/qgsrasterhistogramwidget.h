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

class QgsRasterLayer;
class QgsRasterRendererWidget;
class QwtPlotPicker;
class QwtPlotMarker;
class QwtPlotZoomer;

// fix for qwt5/qwt6 QwtDoublePoint vs. QPointF
#if defined(QWT_VERSION) && QWT_VERSION>=0x060000
typedef QPointF QwtDoublePoint;
#endif

/** Histogram widget
  *@author Etienne Tourigny
  */

class QgsRasterHistogramWidget : public QWidget, private Ui::QgsRasterHistogramWidgetBase
{
    Q_OBJECT

  public:
    QgsRasterHistogramWidget( QgsRasterLayer *lyr, QWidget *parent = 0 );
    ~QgsRasterHistogramWidget();

    /** Save the histogram as an image to disk */
    void histoSaveAsImage( const QString& theFilename );

    /** Set the renderer widget (or just its name if there is no widget) */
    void setRendererWidget( const QString& name, QgsRasterRendererWidget* rendererWidget = NULL );

    /** Activate the histogram widget */
    void setActive( bool theActiveFlag );

    /** \brief Compute the histogram on demand. */
    bool computeHistogram( bool forceComputeFlag );

  public slots:
    /** \brief slot executed when user wishes to refresh raster histogramwidget */
    void refreshHistogram();
    /** This slot lets you save the histogram as an image to disk */
    void on_mSaveAsImageButton_clicked();

  private slots:
    /** Used when the histogram band selector changes, or when tab is loaded. */
    void on_cboHistoBand_currentIndexChanged( int );
    /** Applies the selected min/max values to the renderer widget. */
    void applyHistoMin( );
    void applyHistoMax( );
    /** Button to activate picking of the min/max value on the graph. */
    void on_btnHistoMin_toggled();
    void on_btnHistoMax_toggled();
    /** Called when a selection has been made using the plot picker. */
    void histoPickerSelected( const QPointF & );
    /** Called when a selection has been made using the plot picker (for qwt5 only). */
    void histoPickerSelectedQwt5( const QwtDoublePoint & );
    /** Various actions that are stored in btnHistoActions. */
    void histoActionTriggered( QAction* );
    /** Draw the min/max markers on the histogram plot. */
    void updateHistoMarkers();
    /** Button to compute the histogram, appears when no cached histogram is available. */
    void on_btnHistoCompute_clicked();

    //signals:

  private:

    enum HistoShowBands
    {
      ShowAll = 0,
      ShowSelected = 1,
      ShowRGB = 2
    };

    /** \brief Pointer to the raster layer that this property dilog changes the behaviour of. */
    QgsRasterLayer * mRasterLayer;
    /** \brief Pointer to the renderer widget, to get/set min/max. */
    QgsRasterRendererWidget* mRendererWidget;
    /** \brief Name of the renderer widget (see QgsRasterRendererRegistry). */
    QString mRendererName;

    QwtPlotPicker* mHistoPicker;
    QwtPlotZoomer* mHistoZoomer;
    QwtPlotMarker* mHistoMarkerMin;
    QwtPlotMarker* mHistoMarkerMax;
    double mHistoMin;
    double mHistoMax;
    QVector<QColor> mHistoColors;
    bool mHistoShowMarkers;
    bool mHistoLoadApplyAll;
    HistoShowBands mHistoShowBands;
    /** \brief Returns a list of selected bands in the histogram widget- or empty if there is no selection restriction. */
    QList< int > histoSelectedBands();
    /** \brief Returns a list of selected bands in the renderer widget. */
    QList< int > rendererSelectedBands();
    QPair< QString, QString > rendererMinMax( int theBandNo );
};
#endif
