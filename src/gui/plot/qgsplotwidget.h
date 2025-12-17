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
#include "qgsexpressioncontext.h"
#include "qgsexpressioncontextgenerator.h"
#include "qgsnumericformat.h"
#include "qgspanelwidget.h"
#include "qgsplot.h"
#include "qgspropertycollection.h"
#include "qgspropertyoverridebutton.h"

#include <QWidget>

/**
 * \ingroup gui
 * \class QgsPlotWidget
 * \brief Base class for widgets which allow control over the properties of plots.
 * \since QGIS 4.0
 */
class GUI_EXPORT QgsPlotWidget : public QgsPanelWidget, public QgsExpressionContextGenerator
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
     * Sets the widget to match the settings of the \a plot. Ownership is not transferred.
     * \see createPlot()
     */
    virtual void setPlot( QgsPlot *plot ) = 0;

    /**
     * Creates a plot defined by the current settings in the widget.
     * \see setPlot()
     */
    virtual QgsPlot *createPlot() = 0 SIP_FACTORY;

    /**
     * Register an expression context generator class that will be used to retrieve
     * an expression context for configuration widgets when required.
     */
    void registerExpressionContextGenerator( QgsExpressionContextGenerator *generator );

    QgsExpressionContext createExpressionContext() const override;

  protected:
    /**
     * Initiate a data-defined property button tied to a plot widget.
     */
    void initializeDataDefinedButton( QgsPropertyOverrideButton *button, QgsPlot::DataDefinedProperty key );

    /**
     * Initiate a data-defined property button tied to a plot widget.
     */
    void updateDataDefinedButton( QgsPropertyOverrideButton *button );

    QgsPropertyCollection mPropertyCollection;

  private slots:

    void updateProperty();

  private:
    QgsExpressionContextGenerator *mExpressionContextGenerator = nullptr;
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
    /**
     * Constructor for QgsBarChartPlotWidget.
     * \param parent parent widget
     */
    QgsBarChartPlotWidget( QWidget *parent = nullptr );

    void setPlot( QgsPlot *plot ) override;
    QgsPlot *createPlot() override SIP_FACTORY;

    //! Creates a new bar chart plot configuration widget.
    static QgsPlotWidget *create( QWidget *parent ) SIP_FACTORY { return new QgsBarChartPlotWidget( parent ); }

  private slots:
    void mAddSymbolPushButton_clicked();
    void mRemoveSymbolPushButton_clicked();

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
    /**
     * Constructor for QgsLineChartPlotWidget.
     * \param parent parent widget
     */
    QgsLineChartPlotWidget( QWidget *parent = nullptr );

    void setPlot( QgsPlot *plot ) override;
    QgsPlot *createPlot() override SIP_FACTORY;

    //! Creates a new line chart plot configuration widget.
    static QgsPlotWidget *create( QWidget *parent ) SIP_FACTORY { return new QgsLineChartPlotWidget( parent ); }

  private slots:
    void mAddSymbolPushButton_clicked();
    void mRemoveSymbolPushButton_clicked();

  private:
    int mBlockChanges = 0;

    std::unique_ptr< QgsNumericFormat > mXAxisNumericFormat;
    std::unique_ptr< QgsNumericFormat > mYAxisNumericFormat;
};

//
// QgsLineChartPlotWidget
//

#include "ui_qgspiechartplotwidgetbase.h"

/**
 * \ingroup gui
 * \class QgsPieChartPlotWidget
 * \brief Widget class to control the properties of pie chart plots.
 * \since QGIS 4.0
 */
class GUI_EXPORT QgsPieChartPlotWidget : public QgsPlotWidget, private Ui::QgsPieChartPlotWidgetBase
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsLineChartPlotWidget.
     * \param parent parent widget
     */
    QgsPieChartPlotWidget( QWidget *parent = nullptr );

    void setPlot( QgsPlot *plot ) override;
    QgsPlot *createPlot() override SIP_FACTORY;

    //! Creates a new line chart plot configuration widget.
    static QgsPlotWidget *create( QWidget *parent ) SIP_FACTORY { return new QgsPieChartPlotWidget( parent ); }

  private slots:
    void mAddSymbolPushButton_clicked();
    void mRemoveSymbolPushButton_clicked();

  private:
    int mBlockChanges = 0;

    std::unique_ptr< QgsNumericFormat > mNumericFormat;
};

#endif //QGSPLOTWIDGET_H
