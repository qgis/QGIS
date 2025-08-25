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
#include "qgstaskmanager.h"
#include <qgsvectorlayerfeatureiterator.h>


/**
 * \ingroup core
 * \class QgsVectorLayerAbstractPlotDataGatherer
 *
 * \brief An abstract vector layer plot data gatherer base class.
 *
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsVectorLayerAbstractPlotDataGatherer : public QgsTask
{
    Q_OBJECT

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( QgsVectorLayerAbstractPlotDataGatherer *item = dynamic_cast< QgsVectorLayerAbstractPlotDataGatherer * >( sipCpp ) )
    {
      if ( dynamic_cast<QgsVectorLayerXyPlotDataGatherer *>( item ) != NULL )
      {
        sipType = sipType_QgsVectorLayerXyPlotDataGatherer;
      }
      else
      {
        sipType = sipType_QgsVectorLayerAbstractPlotDataGatherer;
      }
    }
    else
    {
      sipType = NULL;
    }
    SIP_END
#endif

  public:

    QgsVectorLayerAbstractPlotDataGatherer() = default;
    virtual ~QgsVectorLayerAbstractPlotDataGatherer() = default;

    bool run() override = 0;

    //! Returns the plot data
    virtual QgsPlotData data() const = 0;

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
    Q_OBJECT

  public:

    /**
     * XY series details
     */
    struct XySeriesDetails
    {
      explicit XySeriesDetails( const QString &xExpression, const QString &yExpression, const QString &filterExpression = QString() )
        : xExpression( xExpression )
        , yExpression( yExpression )
        , filterExpression( filterExpression )
      {}

      QString xExpression;
      QString yExpression;
      QString filterExpression;
    };

    /**
     * The class constructor.
     * \param iterator a feature iterator
     * \param expressionContext an expression conext
     * \param seriesDetails a list of XY series details
     * \param xAxisType the type of X axis - interval or categorical - which will decide whether X values are interval based of categories index
     * \param predefinedCategories a list of predefined categories, only used then the X asis type is set to Qgis.PlotAxisType.Categorical
     */
    QgsVectorLayerXyPlotDataGatherer( const QgsFeatureIterator &iterator, const QgsExpressionContext &expressionContext, const QList<QgsVectorLayerXyPlotDataGatherer::XySeriesDetails> &seriesDetails, Qgis::PlotAxisType xAxisType = Qgis::PlotAxisType::Interval, const QStringList &predefinedCategories = QStringList() );

    /**
     * The class destructor.
     */
    ~QgsVectorLayerXyPlotDataGatherer() override = default;

    bool run() override;

    //! Returns the plot data
    QgsPlotData data() const override;

  protected:
    QgsPlotData mData;

  private:

    QgsFeatureIterator mIterator;
    QgsExpressionContext mExpressionContext;

    Qgis::PlotAxisType mXAxisType = Qgis::PlotAxisType::Interval;
    QList<QgsVectorLayerXyPlotDataGatherer::XySeriesDetails> mSeriesDetails;
    QStringList mPredefinedCategories;
};

#endif // QGSVECTORLAYERPLOTDATAGATHERER_H
