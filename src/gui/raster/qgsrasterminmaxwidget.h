/***************************************************************************
                         qgsrasterminmaxwidget.h
                         ---------------------------------
    begin                : July 2012
    copyright            : (C) 2012 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERMINMAXWIDGET_H
#define QGSRASTERMINMAXWIDGET_H

#include "ui_qgsrasterminmaxwidgetbase.h"
#include "qgsrasterlayer.h"
#include "qgsrectangle.h"

class QgsMapCanvas;

/** \ingroup gui
 * \class QgsRasterMinMaxWidget
 */
class GUI_EXPORT QgsRasterMinMaxWidget: public QWidget, private Ui::QgsRasterMinMaxWidgetBase
{
    Q_OBJECT
  public:
    QgsRasterMinMaxWidget( QgsRasterLayer* theLayer, QWidget *parent = nullptr );
    ~QgsRasterMinMaxWidget();

    /** Sets the extent to use for minimum and maximum value calculation.
     * @param theExtent extent in raster layer's CRS
     * @note if a map canvas is set using setMapCanvas(), its extent will take
     * precedence over any extent set using this method.
     */
    void setExtent( const QgsRectangle & theExtent ) { mExtent = theExtent; }

    /** Sets the map canvas associated with the widget. This allows the widget to retrieve the current
     * map extent from the canvas. If a canvas is set it will take precedence over any extent
     * set from calling setExtent().
     * @param canvas map canvas
     * @see mapCanvas()
     * @note added in QGIS 2.16
     */
    void setMapCanvas( QgsMapCanvas* canvas );

    /** Returns the map canvas associated with the widget.
     * @see setMapCanvas()
     * @see canvasExtent()
     * @note added in QGIS 2.16
     */
    QgsMapCanvas* mapCanvas();

    void setBands( const QList<int> & theBands ) { mBands = theBands; }

    /** Return the extent selected by the user.
     * Either an empty extent for 'full' or the current visible extent.
    */
    QgsRectangle extent();

    /** Return the selected sample size. */
    int sampleSize() { return cboAccuracy->currentIndex() == 0 ? 250000 : 0; }

    // Load programmaticaly with current values
    void load() { on_mLoadPushButton_clicked(); }

  signals:
    void load( int theBandNo, double theMin, double theMax, int origin );

  private slots:
    void on_mLoadPushButton_clicked();

  private:
    QgsRasterLayer* mLayer;
    QList<int> mBands;
    QgsRectangle mExtent;

    QgsMapCanvas* mCanvas;
};

#endif // QGSRASTERMINMAXWIDGET_H
