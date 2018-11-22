/***************************************************************************
    qgscurveeditorwidget.h
    ----------------------
    begin                : February 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCURVEEDITORWIDGET_H
#define QGSCURVEEDITORWIDGET_H

#include <QWidget>
#include "qgis.h"
#include <QThread>
#include <QMutex>
#include <QPen>
#include <QPointer>
#include <qwt_global.h>
#include "qgis_gui.h"
#include "qgspropertytransformer.h"
#include "qgshistogram.h"
#include "qgsvectorlayer.h"

class QwtPlot;
class QwtPlotCurve;
class QwtPlotMarker;
class QwtPlotHistogram;
class HistogramItem;
class QgsCurveEditorPlotEventFilter;

// fix for qwt5/qwt6 QwtDoublePoint vs. QPointF
typedef QPointF QwtDoublePoint SIP_SKIP;

#ifndef SIP_RUN

// just internal guff - definitely not for exposing to public API!
///@cond PRIVATE

/**
 * \class QgsHistogramValuesGatherer
 * Calculates a histogram in a thread.
 * \note not available in Python bindings
 */
class QgsHistogramValuesGatherer: public QThread
{
    Q_OBJECT

  public:
    QgsHistogramValuesGatherer() = default;

    void run() override
    {
      mWasCanceled = false;
      if ( mExpression.isEmpty() || !mLayer )
      {
        mHistogram.setValues( QList<double>() );
        return;
      }

      // allow responsive cancelation
      mFeedback = new QgsFeedback();

      mHistogram.setValues( mLayer, mExpression, mFeedback );

      // be overly cautious - it's *possible* stop() might be called between deleting mFeedback and nulling it
      mFeedbackMutex.lock();
      delete mFeedback;
      mFeedback = nullptr;
      mFeedbackMutex.unlock();

      emit calculatedHistogram();
    }

    //! Informs the gatherer to immediately stop collecting values
    void stop()
    {
      // be cautious, in case gatherer stops naturally just as we are canceling it and mFeedback gets deleted
      mFeedbackMutex.lock();
      if ( mFeedback )
        mFeedback->cancel();
      mFeedbackMutex.unlock();

      mWasCanceled = true;
    }

    //! Returns true if collection was canceled before completion
    bool wasCanceled() const { return mWasCanceled; }

    const QgsHistogram &histogram() const { return mHistogram; }

    const QgsVectorLayer *layer() const
    {
      return mLayer;
    }
    void setLayer( const QgsVectorLayer *layer )
    {
      mLayer = const_cast< QgsVectorLayer * >( layer );
    }

    QString expression() const
    {
      return mExpression;
    }
    void setExpression( const QString &expression )
    {
      mExpression = expression;
    }

  signals:

    /**
     * Emitted when histogram has been calculated
     */
    void calculatedHistogram();

  private:

    QPointer< const QgsVectorLayer > mLayer = nullptr;
    QString mExpression;
    QgsHistogram mHistogram;
    QgsFeedback *mFeedback = nullptr;
    QMutex mFeedbackMutex;
    bool mWasCanceled = false;
};

///@endcond

#endif

/**
 * \ingroup gui
 * \class QgsCurveEditorWidget
 * A widget for manipulating QgsCurveTransform curves.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsCurveEditorWidget : public QWidget
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsCurveEditorWidget.
     */
    QgsCurveEditorWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr, const QgsCurveTransform &curve = QgsCurveTransform() );

    ~QgsCurveEditorWidget() override;

    /**
     * Returns a curve representing the current curve from the widget.
     * \see setCurve()
     */
    QgsCurveTransform curve() const { return mCurve; }

    /**
     * Sets the \a curve to show in the widget.
     * \see curve()
     */
    void setCurve( const QgsCurveTransform &curve );

    /**
     * Sets a \a layer and \a expression source for values to show in a histogram
     * behind the curve. The histogram is generated in a background thread to keep
     * the widget responsive.
     * \see minHistogramValueRange()
     * \see maxHistogramValueRange()
     */
    void setHistogramSource( const QgsVectorLayer *layer, const QString &expression );

    /**
     * Returns the minimum expected value for the range of values shown in the histogram.
     * \see maxHistogramValueRange()
     * \see setMinHistogramValueRange()
     */
    double minHistogramValueRange() const { return mMinValueRange; }

    /**
     * Returns the maximum expected value for the range of values shown in the histogram.
     * \see minHistogramValueRange()
     * \see setMaxHistogramValueRange()
     */
    double maxHistogramValueRange() const { return mMaxValueRange; }

  public slots:

    /**
     * Sets the minimum expected value for the range of values shown in the histogram.
     * \see setMaxHistogramValueRange()
     * \see minHistogramValueRange()
     */
    void setMinHistogramValueRange( double minValueRange );

    /**
     * Sets the maximum expected value for the range of values shown in the histogram.
     * \see setMinHistogramValueRange()
     * \see maxHistogramValueRange()
     */
    void setMaxHistogramValueRange( double maxValueRange );

  signals:

    //! Emitted when the widget curve changes
    void changed();

  protected:

    void keyPressEvent( QKeyEvent *event ) override;

  private slots:

    void plotMousePress( QPointF point );
    void plotMouseRelease( QPointF point );
    void plotMouseMove( QPointF point );

  private:

    QgsCurveTransform mCurve;

    QwtPlot *mPlot = nullptr;

    QwtPlotCurve *mPlotCurve = nullptr;

    QList< QwtPlotMarker * > mMarkers;
    QgsCurveEditorPlotEventFilter *mPlotFilter = nullptr;
    int mCurrentPlotMarkerIndex = -1;
    //! Background histogram gatherer thread
    std::unique_ptr< QgsHistogramValuesGatherer > mGatherer;
    std::unique_ptr< QgsHistogram > mHistogram;
    double mMinValueRange = 0.0;
    double mMaxValueRange = 1.0;

    QwtPlotHistogram *mPlotHistogram = nullptr;

    void updatePlot();
    void addPlotMarker( double x, double y, bool isSelected = false );
    void updateHistogram();

    int findNearestControlPoint( QPointF point ) const;

    QwtPlotHistogram *createPlotHistogram( const QBrush &brush, const QPen &pen = Qt::NoPen ) const;

};




#ifndef SIP_RUN
//
// NOTE:
// For private only, not part of stable api or exposed to Python bindings
//
/// @cond PRIVATE
class GUI_EXPORT QgsCurveEditorPlotEventFilter: public QObject
{
    Q_OBJECT

  public:

    QgsCurveEditorPlotEventFilter( QwtPlot *plot );

    bool eventFilter( QObject *object, QEvent *event ) override;

  signals:

    void mousePress( QPointF );
    void mouseRelease( QPointF );
    void mouseMove( QPointF );

  private:

    QwtPlot *mPlot = nullptr;
    QPointF mapPoint( QPointF point ) const;
};
///@endcond
#endif

#endif // QGSCURVEEDITORWIDGET_H
