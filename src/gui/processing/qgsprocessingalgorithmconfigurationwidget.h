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

class QgsProcessingAlgorithm;
class QgsProcessingAlgorithmConfigurationWidget;

/**
 * A configuration widget for processing algorithms allows providing additional
 * configuration options directly on algorithm level, in addition to parameters.
 *
 * \ingroup gui
 * \since QGIS 3.2
 */
class GUI_EXPORT QgsProcessingAlgorithmConfigurationWidget : public QWidget
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
