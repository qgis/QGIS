/***************************************************************************
     qgsexpressioncontextutils.h
     ---------------------------
    Date                 : April 2015
    Copyright            : (C) 2015 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSEXPRESSIONCONTEXTUTILS_H
#define QGSEXPRESSIONCONTEXTUTILS_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsfeature.h"
#include "qgspointlocator.h"
#include "qgsexpressioncontext.h"
#include "qgsmeshdataprovider.h"
#include <QString>
#include <QVariantMap>

class QgsExpressionContextScope;
class QgsProject;
class QgsLayout;
class QgsSymbol;
class QgsLayoutAtlas;
class QgsLayoutItem;
class QgsProcessingAlgorithm;
class QgsProcessingModelAlgorithm;
class QgsProcessingContext;
class QgsLayoutMultiFrame;

/**
 * \ingroup core
 * \class QgsExpressionContextUtils
 * \brief Contains utilities for working with QgsExpressionContext objects, including methods
 * for creating scopes for specific uses (e.g., project scopes, layer scopes).
 * \since QGIS 2.12
 */

class CORE_EXPORT QgsExpressionContextUtils
{
  public:

    /**
     * Creates a new scope which contains variables and functions relating to the global QGIS context.
     * For instance, QGIS version numbers and variables specified through QGIS options.
     * \see setGlobalVariable()
     */
    static QgsExpressionContextScope *globalScope() SIP_FACTORY;

    /**
     * Creates a new scope which contains functions and variables from the current attribute form/table \a formFeature.
     * The variables and values in this scope will reflect the current state of the form/row being edited.
     * The \a formMode (SingleEditMode etc.) is passed as text
     * \since QGIS 3.2
     */
    static QgsExpressionContextScope *formScope( const QgsFeature &formFeature = QgsFeature( ), const QString &formMode = QString() ) SIP_FACTORY;

    /**
     * Creates a new scope which contains functions and variables from the current parent attribute form/table \a formFeature.
     * The variables and values in this scope will reflect the current state of the parent form/row being edited.
     * The \a formMode (SingleEditMode etc.) is passed as text
     * \since QGIS 3.14
     */
    static QgsExpressionContextScope *parentFormScope( const QgsFeature &formFeature = QgsFeature( ), const QString &formMode = QString() ) SIP_FACTORY;

    /**
     * Sets a global context variable. This variable will be contained within scopes retrieved via
     * globalScope().
     * \param name variable name
     * \param value variable value
     * \see setGlobalVariable()
     * \see globalScope()
     * \see removeGlobalVariable()
     */
    static void setGlobalVariable( const QString &name, const QVariant &value );

    /**
     * Sets all global context variables. Existing global variables will be removed and replaced
     * with the variables specified.
     * \param variables new set of global variables
     * \see setGlobalVariable()
     * \see globalScope()
     * \see removeGlobalVariable()
     */
    static void setGlobalVariables( const QVariantMap &variables );

    /**
     * Remove a global context variable.
     * \param name variable name
     * \see setGlobalVariable()
     * \see setGlobalVariables()
     * \see globalScope()
     */
    static void removeGlobalVariable( const QString &name );

    /**
     * Creates a new scope which contains variables and functions relating to a QGIS project.
     * For instance, project path and title, and variables specified through the project properties.
     * \param project What project to use
     * \see setProjectVariable()
     */
    static QgsExpressionContextScope *projectScope( const QgsProject *project ) SIP_FACTORY;

    /**
     * Sets a project context variable. This variable will be contained within scopes retrieved via
     * projectScope().
     * \param project Project to apply changes to
     * \param name variable name
     * \param value variable value
     * \see setProjectVariables()
     * \see removeProjectVariable()
     * \see projectScope()
     */
    static void setProjectVariable( QgsProject *project, const QString &name, const QVariant &value );

    /**
     * Sets all project context variables. Existing project variables will be removed and replaced
     * with the variables specified.
     * \param project Project to apply changes to
     * \param variables new set of project variables
     * \see setProjectVariable()
     * \see removeProjectVariable()
     * \see projectScope()
     */
    static void setProjectVariables( QgsProject *project, const QVariantMap &variables );

    /**
     * Remove project context variable.
     * \param project Project to apply changes to
     * \param name variable name
     * \see setProjectVariable()
     * \see setProjectVariables()
     * \see projectScope()
     */
    static void removeProjectVariable( QgsProject *project, const QString &name );

    /**
     * Creates a new scope which contains variables and functions relating to a QgsMapLayer.
     * For instance, layer name, id and fields.
     */
    static QgsExpressionContextScope *layerScope( const QgsMapLayer *layer ) SIP_FACTORY;

    /**
     * Creates a list of three scopes: global, layer's project and layer.
     * \since QGIS 3.0
     */
    static QList<QgsExpressionContextScope *> globalProjectLayerScopes( const QgsMapLayer *layer ) SIP_FACTORY;

    /**
      * Sets a layer context variable. This variable will be contained within scopes retrieved via
      * layerScope().
      * \param layer map layer
      * \param name variable name
      * \param value variable value
      * \see setLayerVariables()
      * \see layerScope()
      */
    static void setLayerVariable( QgsMapLayer *layer, const QString &name, const QVariant &value );

    /**
     * Sets all layer context variables. Existing layer variables will be removed and replaced
     * with the variables specified.
     * \param layer map layer
     * \param variables new set of layer variables
     * \see setLayerVariable()
     * \see layerScope()
     */
    static void setLayerVariables( QgsMapLayer *layer, const QVariantMap &variables );

    /**
     * Creates a new scope which contains variables and functions relating to a QgsMapSettings object.
     * For instance, map scale and rotation.
     */
    static QgsExpressionContextScope *mapSettingsScope( const QgsMapSettings &mapSettings ) SIP_FACTORY;

    /**
     * Sets the expression context variables which are available for expressions triggered by
     * a map tool capture like add feature.
     *
     * \since QGIS 3.0
     */
    static QgsExpressionContextScope *mapToolCaptureScope( const QList<QgsPointLocator::Match> &matches ) SIP_FACTORY;

    /**
     * Sets the expression context variables which are available for expressions triggered by moving the mouse over a feature
     * of the currently selected layer.
     * \param position map coordinates of the current pointer position in the CRS of the layer which triggered the action.
     *
     * \since QGIS 3.30
     */
    static QgsExpressionContextScope *mapLayerPositionScope( const QgsPointXY &position ) SIP_FACTORY;

    /**
     * Updates a symbol scope related to a QgsSymbol to an expression context.
     * \param symbol symbol to extract properties from
     * \param symbolScope pointer to an existing scope to update
     * \since QGIS 2.14
     */
    static QgsExpressionContextScope *updateSymbolScope( const QgsSymbol *symbol, QgsExpressionContextScope *symbolScope = nullptr );

    /**
     * Creates a new scope which contains variables and functions relating to a QgsLayout \a layout.
     * For instance, number of pages and page sizes.
     * \since QGIS 3.0
     */
    static QgsExpressionContextScope *layoutScope( const QgsLayout *layout ) SIP_FACTORY;

    /**
     * Sets a layout context variable. This variable will be contained within scopes retrieved via
     * layoutScope().
     * \param layout target layout
     * \param name variable name
     * \param value variable value
     * \see setLayoutVariables()
     * \see layoutScope()
     * \since QGIS 3.0
     */
    static void setLayoutVariable( QgsLayout *layout, const QString &name, const QVariant &value );

    /**
     * Sets all layout context variables. Existing layout variables will be removed and replaced
     * with the variables specified.
     * \param layout target layout
     * \param variables new set of layer variables
     * \see setLayoutVariable()
     * \see layoutScope()
     * \since QGIS 3.0
     */
    static void setLayoutVariables( QgsLayout *layout, const QVariantMap &variables );

    /**
     * Creates a new scope which contains variables and functions relating to a QgsLayoutAtlas.
     * For instance, current page name and number.
     * \param atlas source atlas. If NULLPTR, a set of default atlas variables will be added to the scope.
     */
    static QgsExpressionContextScope *atlasScope( const QgsLayoutAtlas *atlas ) SIP_FACTORY;

    /**
     * Creates a new scope which contains variables and functions relating to a QgsLayoutItem.
     * For instance, item size and position.
     * \see setLayoutItemVariable()
     * \see setLayoutItemVariables()
     * \since QGIS 3.0
     */
    static QgsExpressionContextScope *layoutItemScope( const QgsLayoutItem *item ) SIP_FACTORY;

    /**
     * Sets a layout \a item context variable, with the given \a name and \a value.
     * This variable will be contained within scopes retrieved via
     * layoutItemScope().
     * \see setLayoutItemVariables()
     * \see layoutItemScope()
     * \since QGIS 3.0
     */
    static void setLayoutItemVariable( QgsLayoutItem *item, const QString &name, const QVariant &value );

    /**
     * Sets all layout item context variables for an \a item. Existing variables will be removed and replaced
     * with the \a variables specified.
     * \see setLayoutItemVariable()
     * \see layoutItemScope()
     * \since QGIS 3.0
     */
    static void setLayoutItemVariables( QgsLayoutItem *item, const QVariantMap &variables );

    /**
     * Creates a new scope which contains variables and functions relating to a QgsLayoutMultiFrame.
     * \see setLayoutMultiFrameVariable()
     * \see setLayoutMultiFrameVariables()
     * \since QGIS 3.10
     */
    static QgsExpressionContextScope *multiFrameScope( const QgsLayoutMultiFrame *frame ) SIP_FACTORY;

    /**
     * Sets a layout multi \a frame context variable, with the given \a name and \a value.
     * This variable will be contained within scopes retrieved via
     * multiFrameScope().
     * \see setLayoutItemVariables()
     * \see multiFrameScope()
     * \since QGIS 3.10
     */
    static void setLayoutMultiFrameVariable( QgsLayoutMultiFrame *frame, const QString &name, const QVariant &value );

    /**
     * Sets all layout multiframe context variables for an \a frame. Existing variables will be removed and replaced
     * with the \a variables specified.
     * \see setLayoutMultiFrameVariable()
     * \see multiFrameScope()
     * \since QGIS 3.10
     */
    static void setLayoutMultiFrameVariables( QgsLayoutMultiFrame *frame, const QVariantMap &variables );

    /**
     * Helper function for creating an expression context which contains just a feature and fields
     * collection. Generally this method should not be used as the created context does not include
     * standard scopes such as the global and project scopes.
     */
    static QgsExpressionContext createFeatureBasedContext( const QgsFeature &feature, const QgsFields &fields );

    /**
     * Creates a new scope which contains variables and functions relating to a processing \a algorithm,
     * when used with the specified \a parameters and \a context.
     * For instance, algorithm name and parameter functions.
     * \see processingModelAlgorithmScope()
     */
    static QgsExpressionContextScope *processingAlgorithmScope( const QgsProcessingAlgorithm *algorithm, const QVariantMap &parameters, QgsProcessingContext &context ) SIP_FACTORY;

    /**
     * Creates a new scope which contains variables and functions relating to a processing \a model algorithm,
     * when used with the specified \a parameters and \a context.
     * For instance, model name and path variables.
     * \since QGIS 3.6
     */
    static QgsExpressionContextScope *processingModelAlgorithmScope( const QgsProcessingModelAlgorithm *model, const QVariantMap &parameters, QgsProcessingContext &context ) SIP_FACTORY;

    /**
     * Creates a new scope which contains variables and functions relating to provider notifications
     * \param message the notification message
     */
    static QgsExpressionContextScope *notificationScope( const QString &message = QString() ) SIP_FACTORY;

    /**
     * Registers all known core functions provided by QgsExpressionContextScope objects.
     */
    static void registerContextFunctions();

    /**
     * Creates a new scope which contains functions relating to mesh layer element \a elementType
     * \since QGIS 3.22
     */
    static QgsExpressionContextScope *meshExpressionScope( QgsMesh::ElementType elementType ) SIP_FACTORY;

  private:

    class GetLayerVisibility : public QgsScopedExpressionFunction
    {
      public:
        GetLayerVisibility( const QList<QgsMapLayer *> &layers, double scale = 0 );
        QVariant func( const QVariantList &values, const QgsExpressionContext *, QgsExpression *, const QgsExpressionNodeFunction * ) override;
        QgsScopedExpressionFunction *clone() const override;

      private:
        GetLayerVisibility();

        QList< QPointer< QgsMapLayer > > mLayers;
        QMap< QPointer< QgsMapLayer >, QPair< double, double > > mScaleBasedVisibilityDetails;
        double mScale = 0.0;

    };

    friend class QgsLayoutItemMap; // needs access to GetLayerVisibility

};

///@cond PRIVATE
#ifndef SIP_RUN
class LoadLayerFunction : public QgsScopedExpressionFunction
{
  public:
    LoadLayerFunction()
      : QgsScopedExpressionFunction( QStringLiteral( "load_layer" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "uri" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "provider" ) ), QStringLiteral( "Map Layers" ) )
    {}

    QVariant func( const QVariantList &, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * ) override;
    bool isStatic( const QgsExpressionNodeFunction *node, QgsExpression *parent, const QgsExpressionContext *context ) const override;

    QgsScopedExpressionFunction *clone() const override;

};
#endif
///@endcond

#ifndef SIP_RUN

/**
 * \class QgsExpressionContextScopePopper
 * \brief RAII class to pop scope from an expression context on destruction
 * \ingroup core
 * \since QGIS 3.10
 */
class QgsExpressionContextScopePopper
{
  public:

    /**
     * Constructor for QgsExpressionContextScopePopper. Appends the specified \a scope to the
     * end of \a context. \a scope will be automatically popped and deleted when this QgsExpressionContextScopePopper
     * is destroyed.
     *
     * Ownership of \a scope is transferred to the popper, but it is guaranteed to exist of the lifetime
     * of the popper.
     */
    QgsExpressionContextScopePopper( QgsExpressionContext &context, QgsExpressionContextScope *scope )
      : mContext( context )
    {
      mContext.appendScope( scope );
    }

    ~QgsExpressionContextScopePopper()
    {
      delete mContext.popScope();
    }

  private:
    QgsExpressionContext &mContext;
};
#endif

#endif // QGSEXPRESSIONCONTEXTUTILS_H
