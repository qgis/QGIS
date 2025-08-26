/***************************************************************************
    qgsplotwidget.h
    ---------------
    begin                : August 2025
    copyright            : (C) 2025 by Mathieu Pellerin
    email                : mathieu dot opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPLOTWIDGET_H
#define QGSPLOTWIDGET_H

#include "qgis_sip.h"
#include "qgspanelwidget.h"
#include "qgsplot.h"
#include "qgsnumericformat.h"

#include <QWidget>

/**
 * \ingroup gui
 * \class QgsPlotWidget
 * \brief Base class for widgets which allow control over the properties of plots.
 * \since QGIS 4.0
 */
class GUI_EXPORT QgsPlotWidget : public QgsPanelWidget
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsPlotWidget.
     * \param parent parent widget
     */
    QgsPlotWidget( QWidget *parent = nullptr )
      : QgsPanelWidget( parent )
    {}

    /**
     * Sets the \a plot to show in the widget. Ownership is not transferred.
     * \see plot()
     */
    virtual void setPlot( QgsPlot *plot ) = 0;

    /**
     * Returns the plot defined by the current settings in the widget.
     * \see setPlot()
     */
    virtual QgsPlot *plot() = 0;
};


//
// QgsBarChartPlotWidget
//

#include "ui_qgsbarchartplotwidgetbase.h"

/**
 * \ingroup gui
 * \class QgsBarChartPlotWidget
 * \brief Widget class to control the properties of bar chart plots.
 * \since QGIS 4.0
 */
class GUI_EXPORT QgsBarChartPlotWidget : public QgsPlotWidget, private Ui::QgsBarChartPlotWidgetBase
{
    Q_OBJECT

  public:
    QgsBarChartPlotWidget( QWidget *parent = nullptr );

    virtual void setPlot( QgsPlot *plot ) override;
    virtual QgsPlot *plot() override;

    static QgsPlotWidget *create( QWidget *parent ) SIP_FACTORY { return new QgsBarChartPlotWidget( parent ); }

  private:
    int mBlockChanges = 0;

    std::unique_ptr< QgsNumericFormat > mXAxisNumericFormat;
    std::unique_ptr< QgsNumericFormat > mYAxisNumericFormat;
};


//
// QgsLineChartPlotWidget
//

#include "ui_qgslinechartplotwidgetbase.h"

/**
 * \ingroup gui
 * \class QgsLineChartPlotWidget
 * \brief Widget class to control the properties of line chart plots.
 * \since QGIS 4.0
 */
class GUI_EXPORT QgsLineChartPlotWidget : public QgsPlotWidget, private Ui::QgsLineChartPlotWidgetBase
{
    Q_OBJECT

  public:
    QgsLineChartPlotWidget( QWidget *parent = nullptr );

    virtual void setPlot( QgsPlot *plot ) override;
    virtual QgsPlot *plot() override;

    static QgsPlotWidget *create( QWidget *parent ) SIP_FACTORY { return new QgsLineChartPlotWidget( parent ); }

  private:
    int mBlockChanges = 0;

    std::unique_ptr< QgsNumericFormat > mXAxisNumericFormat;
    std::unique_ptr< QgsNumericFormat > mYAxisNumericFormat;
};

#endif //QGSPLOTWIDGET_H
