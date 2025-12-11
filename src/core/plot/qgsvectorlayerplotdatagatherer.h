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
#include "qgsvectorlayerfeatureiterator.h"

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
    ~QgsVectorLayerAbstractPlotDataGatherer() override = default;

    //! Returns the plot data.
    virtual QgsPlotData data() const = 0;

    //! Sets the feature \a iterator used to gather data from.
    void setFeatureIterator( QgsFeatureIterator &iterator ) { mIterator = iterator; }

    //! Sets the expression \a context used when evaluating values being gathered.
    void setExpressionContext( const QgsExpressionContext &context ) { mExpressionContext = context; }

  protected:

    QgsFeatureIterator mIterator;
    QgsExpressionContext mExpressionContext;

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
     * The vector layer XY plot data gatherer constructor.
     * \param xAxisType The X-axis type that will define what type of X values to gather.
     */
    explicit QgsVectorLayerXyPlotDataGatherer( Qgis::PlotAxisType xAxisType = Qgis::PlotAxisType::Interval );
    ~QgsVectorLayerXyPlotDataGatherer() override = default;

    //! Sets the series \a details list that will be used to prepare the data being gathered.
    void setSeriesDetails( const QList<QgsVectorLayerXyPlotDataGatherer::XySeriesDetails> &details );

    /**
     * Sets the predefined \a categories list that will be used to restrict the categories used when gathering the data.
     * \note This is only used when the gatherer's X-axis type is set to categorical.
     */
    void setPredefinedCategories( const QStringList &categories );

    bool run() override;

    QgsPlotData data() const override;

  protected:

    QgsPlotData mData;

  private:

    Qgis::PlotAxisType mXAxisType = Qgis::PlotAxisType::Interval;
    QList<QgsVectorLayerXyPlotDataGatherer::XySeriesDetails> mSeriesDetails;
    QStringList mPredefinedCategories;
};

#endif // QGSVECTORLAYERPLOTDATAGATHERER_H
