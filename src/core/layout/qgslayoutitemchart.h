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
     */
    struct SeriesDetails
    {
      explicit SeriesDetails( const QString &name )
        : name( name )
      {};
      bool operator==( const SeriesDetails &other ) const
      {
        return name == other.name
               && xExpression == other.xExpression
               && yExpression == other.yExpression
               && filterExpression == other.filterExpression;
      }

      QString name;
      QString xExpression;
      QString yExpression;
      QString filterExpression;
    };

    /**
     * Constructor for QgsLayoutItemChart, with the specified parent \a layout.
     */
    QgsLayoutItemChart( QgsLayout *layout );

    int type() const override;
    QIcon icon() const override;

    /**
     * Sets the \a plot used to render the chart.
     */
    void setPlot( QgsPlot *plot );

    /**
     * Returns the plot used the render the chart.
     */
    QgsPlot *plot() const { return mPlot.get(); }

    /**
     * Sets the vector \a layer from which the plot data wil be gathered from.
     *
     * \see vectorLayer()
     */
    void setVectorLayer( QgsVectorLayer *layer );

    /**
     * Returns the vector layer from which the plot data will be gathered from.
     *
     * \see setVectorLayer()
     */
    QgsVectorLayer *vectorLayer() const { return mVectorLayer.get(); }

    /**
     * Sets the plot series details used to generate the plot data.
     *
     * \see seriesList()
     */
    void setSeriesList( const QList<QgsLayoutItemChart::SeriesDetails> seriesList );

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
    std::unique_ptr<QgsVectorLayerAbstractPlotDataGatherer> mGatherer;
};

#endif // QGSLAYOUTITEMCHART_H
