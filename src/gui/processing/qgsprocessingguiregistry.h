/***************************************************************************
                         qgsprocessingguiregistry.h
                         ---------------------
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

#ifndef QGSPROCESSINGGUIREGISTRY_H
#define QGSPROCESSINGGUIREGISTRY_H

#include "qgis_gui.h"
#include "qgis_sip.h"

#include <QList>

class QgsProcessingAlgorithm;
class QgsProcessingAlgorithmConfigurationWidget;
class QgsProcessingAlgorithmConfigurationWidgetFactory;

/**
 * The QgsProcessingGuiRegistry is a home for widgets for processing
 * configuration widgets.
 *
 * \ingroup gui
 * \since QGIS 3.2
 */
class GUI_EXPORT QgsProcessingGuiRegistry
{
  public:

    /**
     * Constructor. Should never be called manually, is already
     * created by QgsGui.
     */
    QgsProcessingGuiRegistry();
    ~QgsProcessingGuiRegistry();

    /**
     * Add a new configuration widget factory for customized algorithm configuration
     * widgets. Ownership is taken.
     *
     * \since QGIS 3.2
     */
    void addAlgorithmConfigurationWidgetFactory( QgsProcessingAlgorithmConfigurationWidgetFactory *factory SIP_TRANSFER );

    /**
     * Remove a configuration widget factory for customized algorithm configuration
     * widgets.
     *
     * \since QGIS 3.2
     */
    void removeAlgorithmConfigurationWidgetFactory( QgsProcessingAlgorithmConfigurationWidgetFactory *factory );

    /**
     * Gets the configuration widget for an \a algorithm. This widget will be shown
     * next to parameter widgets. Most algorithms do not have a configuration widget
     * and in this case, nullptr will be returned.
     *
     * \since QGIS 3.2
     */
    QgsProcessingAlgorithmConfigurationWidget *algorithmConfigurationWidget( const QgsProcessingAlgorithm *algorithm ) const;

  private:

    QList <QgsProcessingAlgorithmConfigurationWidgetFactory *> mAlgorithmConfigurationWidgetFactories;
};

#endif // QGSPROCESSINGGUIREGISTRY_H
