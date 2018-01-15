/***************************************************************************
                         qgsgraduatedhistogramwidget.h
                         -----------------------------
    begin                : May 2015
    copyright            : (C) 2015 by Nyall Dawson
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
#ifndef QGSGRADUATEDHISTOGRAMWIDGET_H
#define QGSGRADUATEDHISTOGRAMWIDGET_H

#include "qgshistogramwidget.h"
#include "qgis.h"
#include "qgis_gui.h"

class QwtPlotPicker;
class QgsGraduatedHistogramEventFilter;

/**
 * \ingroup gui
 * \class QgsGraduatedHistogramWidget
 * \brief Graphical histogram for displaying distribution of field values and
 * editing range breaks for a QgsGraduatedSymbolRenderer renderer.
 *
 * \since QGIS 2.9
 */

class GUI_EXPORT QgsGraduatedHistogramWidget : public QgsHistogramWidget
{
    Q_OBJECT

  public:

    /**
     * QgsGraduatedHistogramWidget constructor
     * \param parent parent widget
     */
    QgsGraduatedHistogramWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets the QgsGraduatedSymbolRenderer renderer associated with the histogram.
     * The histogram will fetch the ranges from the renderer before every refresh.
     * \param renderer associated QgsGraduatedSymbolRenderer
     */
    void setRenderer( QgsGraduatedSymbolRenderer *renderer );

  signals:

    /**
     * Emitted when the user modifies the graduated ranges using the histogram widget.
     * \param rangesAdded true if the user has added ranges, false if the user has just
     * modified existing range breaks
     */
    void rangesModified( bool rangesAdded );

  protected:

    void drawHistogram() override;

  private slots:

    void mousePress( double value );
    void mouseRelease( double value );

  private:

    QgsGraduatedSymbolRenderer *mRenderer = nullptr;
    QwtPlotPicker *mHistoPicker = nullptr;
    QgsGraduatedHistogramEventFilter *mFilter = nullptr;
    double mPressedValue = 0;

    void findClosestRange( double value, int &closestRangeIndex, int &pixelDistance ) const;

    QwtPlotHistogram *createPlotHistogram( const QString &title, const QColor &color ) const;

};


#ifndef SIP_RUN
//
// NOTE:
// For private use by QgsGraduatedHistogramWidget only,
// not part of stable api or exposed to Python bindings
//
/// @cond PRIVATE
class GUI_EXPORT QgsGraduatedHistogramEventFilter: public QObject
{
    Q_OBJECT

  public:

    QgsGraduatedHistogramEventFilter( QwtPlot *plot );

    bool eventFilter( QObject *object, QEvent *event ) override;

  signals:

    void mousePress( double );
    void mouseRelease( double );

  private:

    QwtPlot *mPlot = nullptr;
    double posToValue( QPointF point ) const;
};
///@endcond
#endif

#endif //QGSGRADUATEDHISTOGRAMWIDGET_H
