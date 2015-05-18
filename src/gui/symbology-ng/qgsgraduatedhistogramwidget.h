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

class QwtPlotPicker;
class QgsGraduatedHistogramEventFilter;

/** \ingroup gui
 * \class QgsGraduatedHistogramWidget
 * \brief Graphical histogram for displaying distribution of field values and
 * editing range breaks for a QgsGraduatedSymbolRendererV2 renderer.
 *
 * \note Added in version 2.9
 */

class GUI_EXPORT QgsGraduatedHistogramWidget : public QgsHistogramWidget
{
    Q_OBJECT

  public:

    /** QgsGraduatedHistogramWidget constructor
     * @param parent parent widget
     */
    QgsGraduatedHistogramWidget( QWidget *parent = 0 );
    ~QgsGraduatedHistogramWidget();

    /** Sets the QgsGraduatedSymbolRendererV2 renderer associated with the histogram.
     * The histogram will fetch the ranges from the renderer before every refresh.
     * @param renderer associated QgsGraduatedSymbolRendererV2
     */
    void setRenderer( QgsGraduatedSymbolRendererV2* renderer );

  signals:

    /** Emitted when the user modifies the graduated ranges using the histogram widget.
     * @param rangesAdded true if the user has added ranges, false if the user has just
     * modified existing range breaks
     */
    void rangesModified( bool rangesAdded );

  protected:

    virtual void drawHistogram() override;

  private slots:

    void mousePress( double value );
    void mouseRelease( double value );

  private:

    QgsGraduatedSymbolRendererV2* mRenderer;
    QwtPlotPicker* mHistoPicker;
    QgsGraduatedHistogramEventFilter* mFilter;
    double mPressedValue;

    void findClosestRange( double value, int &closestRangeIndex, int &pixelDistance ) const;

#if defined(QWT_VERSION) && QWT_VERSION>=0x060000
    QwtPlotHistogram* createPlotHistogram( const QString& title, const QColor& color ) const;
#else
    HistogramItem* createHistoItem( const QString& title, const QColor& color ) const;
#endif

};

//
// NOTE:
// For private use by QgsGraduatedHistogramWidget only,
// not part of stable api or exposed to Python bindings
//

class GUI_EXPORT QgsGraduatedHistogramEventFilter: public QObject
{
    Q_OBJECT

  public:

    QgsGraduatedHistogramEventFilter( QwtPlot *plot );

    virtual ~QgsGraduatedHistogramEventFilter() {}

    virtual bool eventFilter( QObject* object, QEvent* event ) override;

  signals:

    void mousePress( double );
    void mouseRelease( double );

  private:

    QwtPlot* mPlot;
    double posToValue( const QPointF& point ) const;
};

#endif //QGSGRADUATEDHISTOGRAMWIDGET_H
