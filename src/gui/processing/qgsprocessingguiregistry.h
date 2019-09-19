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
#include "qgsprocessinggui.h"
#include "qgsprocessingwidgetwrapper.h"
#include <QList>
#include <QMap>

class QgsProcessingAlgorithm;
class QgsProcessingAlgorithmConfigurationWidget;
class QgsProcessingAlgorithmConfigurationWidgetFactory;
class QgsProcessingModelerParameterWidget;
class QgsProcessingParameterWidgetContext;

/**
 * The QgsProcessingGuiRegistry is a home for widgets for processing
 * configuration widgets.
 *
 * QgsProcessingGuiRegistry is not usually directly created, but rather accessed through
 * QgsGui::processingGuiRegistry().
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
     * and in this case, NULLPTR will be returned.
     *
     * \since QGIS 3.2
     */
    QgsProcessingAlgorithmConfigurationWidget *algorithmConfigurationWidget( const QgsProcessingAlgorithm *algorithm ) const;

    /**
     * Adds a parameter widget \a factory to the registry, allowing widget creation for parameters of the matching
     * type via createParameterWidgetWrapper() and createModelerParameterWidget().
     *
     * Ownership of \a factory is transferred to the registry.
     *
     * Returns TRUE if the factory was successfully added, or FALSE if the factory could not be added. Each
     * factory must return a unique value for QgsProcessingParameterWidgetFactoryInterface::parameterType(),
     * and attempting to add a new factory with a duplicate type will result in failure.
     *
     * \see removeParameterWidgetFactory()
     * \see createParameterWidgetWrapper()
     * \see createModelerParameterWidget()
     *
     * \since QGIS 3.4
     */
    bool addParameterWidgetFactory( QgsProcessingParameterWidgetFactoryInterface *factory SIP_TRANSFER );

    /**
     * Removes a parameter widget \a factory from the registry. The factory will be deleted.
     *
     * \see addParameterWidgetFactory()
     *
     * \since QGIS 3.4
     */
    void removeParameterWidgetFactory( QgsProcessingParameterWidgetFactoryInterface *factory );

    /**
     * Creates a new parameter widget wrapper for the given \a parameter. The \a type argument
     * dictates the type of dialog the wrapper should be created for. The caller takes ownership
     * of the returned wrapper.
     *
     * If no factory is registered which handles the given \a parameter, NULLPTR will be returned.
     *
     * \see createModelerParameterWidget()
     * \see addParameterWidgetFactory()
     *
     * \since QGIS 3.4
     */
    QgsAbstractProcessingParameterWidgetWrapper *createParameterWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) SIP_FACTORY;

    /**
     * Creates a new modeler parameter widget for the given \a parameter. This widget allows
     * configuration of the parameter's value when used inside a Processing \a model.
     *
     * The ID of the child algorithm within the model must be specified via the \a childId
     * argument. This value corresponds to the QgsProcessingModelChildAlgorithm::childId()
     * string, which uniquely identifies which child algorithm the parameter is associated
     * with inside the given \a model.
     *
     * The caller takes ownership of the returned widget. If no factory is registered which
     * handles the given \a parameter, NULLPTR will be returned.
     *
     * \see createParameterWidgetWrapper()
     * \see addParameterWidgetFactory()
     *
     * \since QGIS 3.4
     */
    QgsProcessingModelerParameterWidget *createModelerParameterWidget( QgsProcessingModelAlgorithm *model,
        const QString &childId,
        const QgsProcessingParameterDefinition *parameter, QgsProcessingContext &context ) SIP_FACTORY;

    /**
     * Creates a new parameter definition widget allowing for configuration of an instance of
     * a specific parameter \a type.
     *
     * The \a context argument must specify a Processing context, which will be used
     * by the widget to evaluate existing \a definition properties such as default values. Similarly,
     * the \a widgetContext argument specifies the wider GUI context in which the widget
     * will be used.
     *
     * The optional \a definition argument may specify an existing parameter definition which
     * will be reflected in the initial state of the returned widget. If \a definition is NULLPTR,
     * then the returned widget will use default settings instead.
     *
     * Additionally, the optional \a algorithm parameter may be used to specify the algorithm or model
     * associated with the parameter.
     *
     * If NULLPTR is returned for a particular parameter \a type,
     * it indicates that the parameter type cannot be configured via GUI.
     *
     * \since QGIS 3.10
     */
    QgsProcessingAbstractParameterDefinitionWidget *createParameterDefinitionWidget( const QString &type,
        QgsProcessingContext &context,
        const QgsProcessingParameterWidgetContext &widgetContext,
        const QgsProcessingParameterDefinition *definition = nullptr,
        const QgsProcessingAlgorithm *algorithm = nullptr ) SIP_FACTORY;

  private:

    QList <QgsProcessingAlgorithmConfigurationWidgetFactory *> mAlgorithmConfigurationWidgetFactories;
    QMap< QString, QgsProcessingParameterWidgetFactoryInterface *> mParameterWidgetFactories;
};

#endif // QGSPROCESSINGGUIREGISTRY_H
