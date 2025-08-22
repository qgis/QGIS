/***************************************************************************
                         qgsvectorlayerplotdatagatherer.h
                         -----------------
    begin                : August 2025
    copyright            : (C) 2025 by Mathieu
    email                : mathieu at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSVECTORLAYERPLOTDATAGATHERER_H
#define QGSVECTORLAYERPLOTDATAGATHERER_H

#include "qgis.h"
#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsplot.h"
#include "qgsvectorlayer.h"
#include <qgsvectorlayerfeatureiterator.h>

#include <QThread>


/**
 * \ingroup core
 * \class QgsVectorLayerAbstractPlotDataGatherer
 *
 * \brief An abstract vector layer plot data gatherer base class.
 *
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsVectorLayerAbstractPlotDataGatherer : public QThread
{
    Q_OBJECT

  public:

    QgsVectorLayerAbstractPlotDataGatherer() = default;
    virtual ~QgsVectorLayerAbstractPlotDataGatherer() = default;

  private:

};


/**
 * \ingroup core
 * \class QgsVectorLayerXyPlotDataGatherer
 *
 * \brief An vector layer plot data gatherer class for XY series.
 *
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsVectorLayerXyPlotDataGatherer : public QgsVectorLayerAbstractPlotDataGatherer
{
  public:

    /**
     * XY series details
     */
    struct XySeriesDetails
    {
      explicit XySeriesDetails( const QString &xExpression, const QString &yExpression, const QString &filterExpression )
        : xExpression( xExpression )
        , yExpression( yExpression )
        , filterExpression( filterExpression )
      {}

      QString xExpression;
      QString yExpression;
      QString filterExpression;
    };

    QgsVectorLayerXyPlotDataGatherer( QgsVectorLayer *layer, const QList<QgsVectorLayerXyPlotDataGatherer::XySeriesDetails> &seriesDetails, Qgis::PlotAxisType xAxisType = Qgis::PlotAxisType::Interval, const QStringList &predefinedCategories = QStringList() );
    ~QgsVectorLayerXyPlotDataGatherer() override = default;

    void run() override;
    //! Informs the gatherer to immediately stop collecting data
    void stop();

    //! Returns TRUE if collection was canceled before completion
    bool wasCanceled() const;

    //! Returns the plot data
    QgsPlotData data() const;

  protected:
    QgsPlotData mData;

  private:

    bool mWasCanceled = false;
    mutable QMutex mCancelMutex;

    std::unique_ptr<QgsVectorLayerFeatureSource> mSource;
    QgsExpressionContext mExpressionContext;

    Qgis::PlotAxisType mXAxisType = Qgis::PlotAxisType::Interval;
    QList<QgsVectorLayerXyPlotDataGatherer::XySeriesDetails> mSeriesDetails;
    QStringList mPredefinedCategories;
};

#endif // QGSVECTORLAYERPLOTDATAGATHERER_H
