/***************************************************************************
                         qgshistogramwidget.h
                         --------------------
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
#ifndef QGSHISTOGRAMWIDGET_H
#define QGSHISTOGRAMWIDGET_H

#include "ui_qgshistogramwidgetbase.h"

#include "qgshistogram.h"
#include "qgsstatisticalsummary.h"
#include "qgsgraduatedsymbolrendererv2.h"
#include <QPen>
#include <QBrush>

class QgsVectorLayer;
class QgsGraduatedSymbolRendererV2;
class QwtPlotPicker;
class QwtPlotMarker;
class QwtPlot;
class HistogramItem;
class QwtPlotHistogram;

// fix for qwt5/qwt6 QwtDoublePoint vs. QPointF
#if defined(QWT_VERSION) && QWT_VERSION>=0x060000
typedef QPointF QwtDoublePoint;
#endif


/** \ingroup gui
 * \class QgsHistogramWidget
 * \brief Graphical histogram for displaying distributions of field values.
 *
 * \note Added in version 2.9
 */

class GUI_EXPORT QgsHistogramWidget : public QWidget, private Ui::QgsHistogramWidgetBase
{
    Q_OBJECT

  public:

    /** QgsHistogramWidget constructor. If layer and fieldOrExp are specified then the histogram
     * will be initially populated with the corresponding values.
     * @param parent parent widget
     * @param layer source vector layer
     * @param fieldOrExp field name or expression string
     */
    QgsHistogramWidget( QWidget *parent = nullptr, QgsVectorLayer* layer = nullptr, const QString& fieldOrExp = QString() );

    ~QgsHistogramWidget();

    /** Returns the layer currently associated with the widget.
     * @see setLayer
     * @see sourceFieldExp
     */
    QgsVectorLayer* layer() { return mVectorLayer; }

    /** Returns the source field name or expression used to calculate values displayed
     * in the histogram.
     * @see setSourceFieldExp
     * @see layer
     */
    QString sourceFieldExp() const { return mSourceFieldExp; }

    /** Sets the pen to use when drawing histogram bars. If set to Qt::NoPen then the
     * pen will be automatically calculated. If ranges have been set using @link setGraduatedRanges @endlink
     * then the pen and brush will have no effect.
     * @param pen histogram pen
     * @see pen
     * @see setBrush
     */
    void setPen( const QPen& pen ) { mPen = pen; }

    /** Returns the pen used when drawing histogram bars.
     * @see setPen
     * @see brush
     */
    QPen pen() const { return mPen; }

    /** Sets the brush used for drawing histogram bars. If ranges have been set using @link setGraduatedRanges @endlink
     * then the pen and brush will have no effect.
     * @param brush histogram brush
     * @see brush
     * @see setPen
     */
    void setBrush( const QBrush& brush ) { mBrush = brush; }

    /** Returns the brush used when drawing histogram bars.
     * @see setBrush
     * @see pen
     */
    QBrush brush() const { return mBrush; }

    /** Sets the graduated ranges associated with the histogram. If set, the ranges will be used to color the histogram
     * bars and for showing vertical dividers at the histogram breaks.
     * @param ranges graduated range list
     * @see graduatedRanges
     */
    void setGraduatedRanges( const QgsRangeList& ranges );

    /** Returns the graduated ranges associated with the histogram. If set, the ranges will be used to color the histogram
     * bars and for showing vertical dividers at the histogram breaks.
     * @returns graduated range list
     * @see setGraduatedRanges
     */
    QgsRangeList graduatedRanges() const { return mRanges; }

    /** Returns the title for the histogram's x-axis.
     * @see setXAxisTitle
     * @see yAxisTitle
     */
    QString xAxisTitle() const { return mXAxisTitle; }

    /** Sets the title for the histogram's x-axis.
     * @param title x-axis title, or empty string to remove title
     * @see xAxisTitle
     * @see setYAxisTitle
     */
    void setXAxisTitle( const QString& title ) { mXAxisTitle = title; }

    /** Returns the title for the histogram's y-axis.
     * @see setYAxisTitle
     * @see xAxisTitle
     */
    QString yAxisTitle() const { return mYAxisTitle; }

    /** Sets the title for the histogram's y-axis.
     * @param title y-axis title, or empty string to remove title
     * @see yAxisTitle
     * @see setXAxisTitle
     */
    void setYAxisTitle( const QString& title ) { mYAxisTitle = title; }

  public slots:

    /** Refreshes the values for the histogram by fetching them from the layer.
     */
    void refreshValues();

    /** Redraws the histogram. Calling this slot does not update the values
     * for the histogram, use @link refreshValues @endlink to do this.
     */
    void refresh();

    /** Sets the vector layer associated with the histogram.
     * @param layer source vector layer
     * @see setSourceFieldExp
     */
    void setLayer( QgsVectorLayer* layer );

    /** Sets the source field or expression to use for values in the histogram.
     * @param fieldOrExp field name or expression string
     * @see setLayer
     */
    void setSourceFieldExp( const QString& fieldOrExp );

  protected:

    /** Updates and redraws the histogram.
     */
    virtual void drawHistogram();

    QwtPlot* mPlot;
    QgsRangeList mRanges;
    QList< QwtPlotMarker* > mRangeMarkers;

  private:

    QgsVectorLayer * mVectorLayer;
    QString mSourceFieldExp;
    QList<double> mValues;
    QgsStatisticalSummary mStats;
    QgsHistogram mHistogram;
    QVector<QColor> mHistoColors;
    QPen mPen;
    QBrush mBrush;
    QPen mMeanPen;
    QPen mStdevPen;
    QPen mGridPen;
    QString mXAxisTitle;
    QString mYAxisTitle;

    void clearHistogram();

#if defined(QWT_VERSION) && QWT_VERSION>=0x060000
    QwtPlotHistogram* createPlotHistogram( const QString& title, const QBrush &brush, const QPen &pen = Qt::NoPen ) const;
#else
    HistogramItem* createHistoItem( const QString& title, const QBrush& brush, const QPen& pen = Qt::NoPen ) const;
#endif

};

#endif //QGSHISTOGRAMWIDGET_H
