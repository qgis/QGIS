/***************************************************************************
                         qgslayoutchartseriesdetailswidget.h
                         --------------------------
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

#ifndef QGSLAYOUTCHARTSERIESDETAILSWIDGET_H
#define QGSLAYOUTCHARTSERIESDETAILSWIDGET_H

// We don't want to expose this in the public API

#include "ui_qgslayoutchartseriesdetailswidgetbase.h"

#include "qgis_gui.h"
#include "qgslayoutitemchart.h"
#include "qgspanelwidget.h"

#define SIP_NO_FILE

/**
 * \ingroup gui
 * \brief A widget for editing series details for a layout chart item.
 * \since QGIS 4.0
*/
class GUI_EXPORT QgsLayoutChartSeriesDetailsWidget : public QgsPanelWidget, public QgsExpressionContextGenerator, private Ui::QgsLayoutChartSeriesDetailsWidgetBase
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsLayoutChartSeriesDetailsWidget
     * \param layer the vector layer associated to the series
     * \param index the series index
     * \param seriesDetails the series details
     * \param parent the parent widget
     */
    QgsLayoutChartSeriesDetailsWidget( QgsVectorLayer *layer, int index, const QgsLayoutItemChart::SeriesDetails &seriesDetails, QWidget *parent = nullptr );

    //! Returns the series index
    int index() const;

    //! Returns the X-axis expression
    QString xExpression() const;

    //! Returns the Y-axis expression
    QString yExpression() const;

    //! Returns the filter expression
    QString filterExpression() const;

    /**
     * Register an expression context generator class that will be used to retrieve
     * an expression context for configuration widgets when required.
     */
    void registerExpressionContextGenerator( QgsExpressionContextGenerator *generator );

    QgsExpressionContext createExpressionContext() const override;

  private slots:
    void mFilterButton_clicked();

  private:
    QPointer<QgsVectorLayer> mVectorLayer;
    int mIndex = 0;

    QgsExpressionContextGenerator *mExpressionContextGenerator = nullptr;
};

#endif // QGSLAYOUTCHARTSERIESDETAILSWIDGET_H
