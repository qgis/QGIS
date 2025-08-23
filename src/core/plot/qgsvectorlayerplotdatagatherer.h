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
      explicit XySeriesDetails( const QString &xExpression, const QString &yExpression, const QString &orderByExpression = QString(), const QString &filterExpression = QString() )
        : xExpression( xExpression )
        , yExpression( yExpression )
        , orderByExpression( orderByExpression )
        , filterExpression( filterExpression )
      {}

      QString xExpression;
      QString yExpression;
      QString orderByExpression;
      QString filterExpression;
    };

    /**
     * The class constructor.
     * \param layer a vector layer from which features will be iterated against
     * \param seriesDetails a list of XY series details
     * \param xAxisType the type of X axis - interval or categorical - which will decide whether X values are interval based of categories index
     * \param predefinedCategories a list of predefined categories, only used then the X asis type is set to Qgis.PlotAxisType.Categorical
     */
    QgsVectorLayerXyPlotDataGatherer( QgsVectorLayer *layer, const QList<QgsVectorLayerXyPlotDataGatherer::XySeriesDetails> &seriesDetails, Qgis::PlotAxisType xAxisType = Qgis::PlotAxisType::Interval, const QStringList &predefinedCategories = QStringList() );

    /**
     * The class destructor.
     */
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
