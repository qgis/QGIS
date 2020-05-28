/***************************************************************************
                         qgsprocessingalgorithmconfig.h
                         --------------------------
    begin                : April 2018
    copyright            : (C) 2018 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSPROCESSINGALGORITHMCONFIGURATIONWIDGET_H
#define QGSPROCESSINGALGORITHMCONFIGURATIONWIDGET_H

#include <QWidget>
#include <QVariantMap>

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgsprocessingwidgetwrapper.h"

class QgsProcessingAlgorithm;
class QgsProcessingAlgorithmConfigurationWidget;


/**
 * A configuration widget for processing algorithms allows providing additional
 * configuration options directly on algorithm level, in addition to parameters.
 *
 * \ingroup gui
 * \since QGIS 3.2
 */
class GUI_EXPORT QgsProcessingAlgorithmConfigurationWidget : public QWidget, public QgsExpressionContextGenerator
{
    Q_OBJECT

  public:

    /**
     * Creates a new QgsProcessingAlgorithmConfigurationWidget
     */
    QgsProcessingAlgorithmConfigurationWidget( QWidget *parent = nullptr );
    ~QgsProcessingAlgorithmConfigurationWidget() override = default;

    /**
     * Read the current configuration from this widget.
     */
    virtual QVariantMap configuration() const = 0;

    /**
     * Set the configuration which this widget should represent.
     */
    virtual void setConfiguration( const QVariantMap &configuration ) = 0;

    /**
     * Sets the \a context in which the Processing algorithm widget is shown, e.g., the
     * parent model algorithm, a linked map canvas, and other relevant information which allows the widget
     * to fine-tune its behavior.
     *
     * Subclasses should take care to call the base class method when reimplementing this method.
     *
     * \see widgetContext()
     * \since QGIS 3.6
     */
    virtual void setWidgetContext( const QgsProcessingParameterWidgetContext &context );

    /**
     * Returns the context in which the Processing algorithm widget is shown, e.g., the
     * parent model algorithm, a linked map canvas, and other relevant information which allows the widget
     * to fine-tune its behavior.
     *
     * \see setWidgetContext()
     * \since QGIS 3.6
     */
    const QgsProcessingParameterWidgetContext &widgetContext() const;

    /**
     * Sets the algorithm instance associated with the widget.
     *
     * \see algorithm()
     * \since QGIS 3.6
     */
    void setAlgorithm( const QgsProcessingAlgorithm *algorithm );

    /**
     * Returns the algorithm instance associated with this widget.
     *
     * \see setAlgorithm()
     * \since QGIS 3.6
     */
    const QgsProcessingAlgorithm *algorithm() const { return mAlgorithm; }

    /**
     * Registers a Processing context \a generator class that will be used to retrieve
     * a Processing context for the widget when required.
     *
     * \since QGIS 3.6
     */
    void registerProcessingContextGenerator( QgsProcessingContextGenerator *generator );

    QgsExpressionContext createExpressionContext() const override;

  private:

    QgsProcessingContextGenerator *mContextGenerator = nullptr;
    const QgsProcessingAlgorithm *mAlgorithm = nullptr;
    QgsProcessingParameterWidgetContext mWidgetContext;
};


/**
 * Interface base class for factories for algorithm configuration widgets.
 *
 * \ingroup gui
 * \since QGIS 3.2
 */
class GUI_EXPORT QgsProcessingAlgorithmConfigurationWidgetFactory
{
  public:
    virtual ~QgsProcessingAlgorithmConfigurationWidgetFactory() = default;

    /**
     * Create a new configuration widget for \a algorithm.
     */
    virtual QgsProcessingAlgorithmConfigurationWidget *create( const QgsProcessingAlgorithm *algorithm ) const = 0 SIP_FACTORY;

    /**
     * Check if this factory can create widgets for \a algorithm.
     */
    virtual bool canCreateFor( const QgsProcessingAlgorithm *algorithm ) const = 0;
};


#endif // QGSPROCESSINGALGORITHMCONFIGURATIONWIDGET_H
