/***************************************************************************
                         qgslayoutitemchart.h
                         -------------------
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
#ifndef QGSLAYOUTITEMCHART_H
#define QGSLAYOUTITEMCHART_H

#include "qgis_core.h"
#include "qgslayoutitem.h"
#include "qgsplot.h"
#include "qgsvectorlayerplotdatagatherer.h"
#include "qgsvectorlayerref.h"

#include <QTimer>

/**
 * \ingroup core
 * \brief A layout item subclass that renders chart plots.
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsLayoutItemChart : public QgsLayoutItem
{
    Q_OBJECT

  public:

    /**
     * Chart series details covering all supported series types.
     *
     * \note this class is experimental and threfore is not considered as stable API, it may
     * change in the future as more chart plot types are implemented.
     *
     * \ingroup core
     * \since QGIS 4.0
     */
    class SeriesDetails
    {
      public:

        /**
         * Constructor for SeriesDetails with an optional \a name parameter to
         * provide a name string to the series.
         */
        explicit SeriesDetails( const QString &name = QString() )
          : mName( name )
        {};

        bool operator==( const SeriesDetails &other ) const
        {
          return mName == other.mName
                 && mXExpression == other.mXExpression
                 && mYExpression == other.mYExpression
                 && mFilterExpression == other.mFilterExpression;
        }

        /**
         * Returns the series name.
         */
        QString name() const { return mName; }

        /**
         * Sets the series name.
         */
        void setName( const QString &name ) { mName = name; }

        /**
         * Returns the expression used to generate X-axis values. If the associated
         * chart X axis type is set to Qgis.PlotAxisType.Categorical, the generated values will
         * be converted to strings. For Qgis.PlotAxisType.Interval, the generated values will
         * be converted to double.
         */
        QString xExpression() const { return mXExpression; }

        /**
         * Sets the expression used to generate X-axis values. If the associated
         * chart X axis type is set to Qgis.PlotAxisType.Categorical, the generated values will
         * be converted to strings. For Qgis.PlotAxisType.Interval, the generated values will
         * be converted to double.
         */
        void setXExpression( const QString &xExpression ) { mXExpression = xExpression; }

        /**
         * Returns the expression used to generate Y-axis values. The generated values will
         * be converted to double.
         */
        QString yExpression() const { return mYExpression; }

        /**
         * Sets the expression used to generate Y-axis values. The generated values will
         * be converted to double.
         */
        void setYExpression( const QString &yExpression ) { mYExpression = yExpression; }

        /**
         * Returns the filter expression used to generate a series against a subset of
         * the source layer.
         */
        QString filterExpression() const { return mFilterExpression; }

        /**
         * Sets the filter expression used to generate a series against a subset of
         * the source layer.
         */
        void setFilterExpression( const QString &filterExpression ) { mFilterExpression = filterExpression; }

      private:
        QString mName;
        QString mXExpression;
        QString mYExpression;
        QString mFilterExpression;
    };

    /**
     * Constructor for QgsLayoutItemChart, with the specified parent \a layout.
     */
    QgsLayoutItemChart( QgsLayout *layout );

    int type() const override;
    QIcon icon() const override;

    /**
     * Sets the \a plot used to render the chart.
     *
     * Ownership is transferred to the item.
     */
    void setPlot( QgsPlot *plot SIP_TRANSFER );

    /**
     * Returns the plot used the render the chart.
     */
    QgsPlot *plot() { return mPlot.get(); }

    /**
     * Sets the source vector \a layer from which the plot data wil be gathered from.
     *
     * \see sourceLayer()
     */
    void setSourceLayer( QgsVectorLayer *layer );

    /**
     * Returns the source vector layer from which the plot data will be gathered from.
     *
     * \see setSourceLayer()
     */
    QgsVectorLayer *sourceLayer() const { return mVectorLayer.get(); }

    /**
     * Sets the plot series details used to generate the plot data.
     *
     * \see seriesList()
     */
    void setSeriesList( const QList<QgsLayoutItemChart::SeriesDetails> &seriesList );

    /**
     * Returns the plot series details used to generate the plot data.
     *
     * \see setSeriesList()
     */
    QList<QgsLayoutItemChart::SeriesDetails> seriesList() const { return mSeriesList; }

    /**
     * Returns a new chart item for the specified \a layout.
     *
     * The caller takes responsibility for deleting the returned object.
     */
    static QgsLayoutItemChart *create( QgsLayout *layout ) SIP_FACTORY;

    void paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget ) override;

  protected:

    void draw( QgsLayoutItemRenderContext &context ) override;
    bool writePropertiesToElement( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const override;
    bool readPropertiesFromElement( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context ) override;

  private:

    void refreshData();
    void gatherData();
    void processData();

    void prepareGatherer();

    QgsLayoutItemChart() = delete;
    QgsLayoutItemChart( const QgsLayoutItemChart & ) = delete;
    QgsLayoutItemChart &operator=( const QgsLayoutItemChart & ) = delete;

    std::unique_ptr<Qgs2DPlot> mPlot;
    QgsPlotData mPlotData;

    //! Associated vector layer
    QgsVectorLayerRef mVectorLayer = nullptr;
    QList<QgsLayoutItemChart::SeriesDetails> mSeriesList;

    bool mNeedsGathering = false;
    bool mIsGathering = false;
    QTimer mGathererTimer;
    QPointer<QgsVectorLayerAbstractPlotDataGatherer> mGatherer;
};

#endif // QGSLAYOUTITEMCHART_H
