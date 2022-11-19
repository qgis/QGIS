/***************************************************************************
    qgsmaplayeractionregistry.h
    ---------------------------
    begin                : January 2014
    copyright            : (C) 2014 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPLAYERACTIONREGISTRY_H
#define QGSMAPLAYERACTIONREGISTRY_H

#include <QObject>
#include "qgis_sip.h"
#include <QList>
#include <QMap>
#include <QAction>
#include <QPointer>

#include "qgis.h"
#include "qgis_gui.h"

class QgsFeature;
class QgsMapLayer;
class QgsAttributeDialog;
class QgsMessageBar;

/**
 * \ingroup gui
 * \brief Encapsulates the context in which a QgsMapLayerAction action is executed.
 * \since QGIS 3.30
 */
class GUI_EXPORT QgsMapLayerActionContext
{
  public:

    QgsMapLayerActionContext();

    /**
     * Returns the attribute dialog associated with the action's execution.
     *
     * May be NULLPTR if the action is not being executed from an attribute dialog.
     *
     * \see setAttributeDialog()
     */
    QgsAttributeDialog *attributeDialog() const;

    /**
     * Sets the attribute \a dialog associated with the action's execution.
     *
     * \see attributeDialog()
     */
    void setAttributeDialog( QgsAttributeDialog *dialog );

    /**
     * Returns the message bar associated with the action's execution.
     *
     * May be NULLPTR.
     *
     * \see setMessageBar()
     */
    QgsMessageBar *messageBar() const;

    /**
     * Sets the message \a bar associated with the action's execution.
     *
     * \see messageBar()
     */
    void setMessageBar( QgsMessageBar *bar );

  private:

    QPointer< QgsAttributeDialog > mAttributeDialog;
    QPointer< QgsMessageBar > mMessageBar;
};

Q_DECLARE_METATYPE( QgsMapLayerActionContext )

/**
 * \ingroup gui
 * \brief An interface for objects which can create a QgsMapLayerActionContext.
 * \since QGIS 3.30
 */
class GUI_EXPORT QgsMapLayerActionContextGenerator
{
  public:

    virtual ~QgsMapLayerActionContextGenerator();

    /**
     * Creates a QgsMapLayerActionContext.
     */
    virtual QgsMapLayerActionContext createActionContext() = 0;
};

/**
 * \ingroup gui
* \brief An action which can run on map layers
* The class can be used in two manners:
* * by instantiating it and connecting to its signals to perform an action
* * by subclassing and reimplementing its method (only since QGIS 3.18.2)
*/
class GUI_EXPORT QgsMapLayerAction : public QAction
{
    Q_OBJECT

  public:
    enum Target
    {
      Layer = 1,
      SingleFeature = 2,
      MultipleFeatures = 4,
      AllActions = Layer | SingleFeature | MultipleFeatures
    };
    Q_DECLARE_FLAGS( Targets, Target )
    Q_FLAG( Targets )

    /**
     * Flags which control action behavior
     * /since QGIS 3.0
     */
    enum Flag
    {
      EnabledOnlyWhenEditable = 1 << 1, //!< Action should be shown only for editable layers
    };

    /**
     * Action behavior flags.
     * \since QGIS 3.0
     */
    Q_DECLARE_FLAGS( Flags, Flag )
    Q_FLAG( Flags )

    /**
     * Creates a map layer action which can run on any layer
     * \note using AllActions as a target probably does not make a lot of sense. This default action was settled for API compatibility reasons.
     */
    QgsMapLayerAction( const QString &name, QObject *parent SIP_TRANSFERTHIS, Targets targets = AllActions, const QIcon &icon = QIcon(), QgsMapLayerAction::Flags flags = QgsMapLayerAction::Flags() );

    //! Creates a map layer action which can run only on a specific layer
    QgsMapLayerAction( const QString &name, QObject *parent SIP_TRANSFERTHIS, QgsMapLayer *layer, Targets targets = AllActions, const QIcon &icon = QIcon(), QgsMapLayerAction::Flags flags = QgsMapLayerAction::Flags() );

    //! Creates a map layer action which can run on a specific type of layer
    QgsMapLayerAction( const QString &name, QObject *parent SIP_TRANSFERTHIS, QgsMapLayerType layerType, Targets targets = AllActions, const QIcon &icon = QIcon(), QgsMapLayerAction::Flags flags = QgsMapLayerAction::Flags() );

    ~QgsMapLayerAction() override;

    /**
     * Layer behavior flags.
     * \since QGIS 3.0
     */
    QgsMapLayerAction::Flags flags() const;

    /**
     * Returns TRUE if the action can run using the specified layer.
     *
     * \deprecated use the version with QgsMapLayerActionContext instead.
     */
    Q_DECL_DEPRECATED virtual bool canRunUsingLayer( QgsMapLayer *layer ) const SIP_DEPRECATED;

    /**
     * Returns TRUE if the action can run using the specified layer.
     *
     * \note Classes which implement this should return FALSE to the deprecated canRunUsingLayer() method which does not accept a QgsMapLayerActionContext argument.
     *
     * \since QGIS 3.30
     */
    virtual bool canRunUsingLayer( QgsMapLayer *layer, const QgsMapLayerActionContext &context ) const;

    /**
     * Triggers the action with the specified layer and list of feature.
     *
     * \deprecated use the version with QgsMapLayerActionContext instead.
     */
    Q_DECL_DEPRECATED virtual void triggerForFeatures( QgsMapLayer *layer, const QList<QgsFeature> &featureList ) SIP_DEPRECATED;

    /**
     * Triggers the action with the specified layer and feature.
     *
     * \deprecated use the version with QgsMapLayerActionContext instead.
     */
    Q_DECL_DEPRECATED virtual void triggerForFeature( QgsMapLayer *layer, const QgsFeature &feature ) SIP_DEPRECATED;

    /**
     * Triggers the action with the specified layer.
     *
     * \deprecated use the version with QgsMapLayerActionContext instead.
     */
    Q_DECL_DEPRECATED virtual void triggerForLayer( QgsMapLayer *layer ) SIP_DEPRECATED;

    /**
     * Triggers the action with the specified layer and list of feature.
     *
     * \since QGIS 3.30
     */
    virtual void triggerForFeatures( QgsMapLayer *layer, const QList<QgsFeature> &featureList, const QgsMapLayerActionContext &context );

    /**
     * Triggers the action with the specified layer and feature.
     *
     * \since QGIS 3.30
     */
    virtual void triggerForFeature( QgsMapLayer *layer, const QgsFeature &feature, const QgsMapLayerActionContext &context );

    /**
     * Triggers the action with the specified layer.
     *
     * \since QGIS 3.30
     */
    virtual void triggerForLayer( QgsMapLayer *layer, const QgsMapLayerActionContext &context );

    //! Define the targets of the action
    void setTargets( Targets targets ) {mTargets = targets;}
    //! Returns availibity of action
    const Targets &targets() const {return mTargets;}

    /**
     * Returns TRUE if the action is only enabled for layers in editable mode.
     * \since QGIS 3.0
     */
    bool isEnabledOnlyWhenEditable() const;

  signals:

    /**
     * Triggered when action has been run for a specific list of features
     * \deprecated use the version with QgsMapLayerActionContext instead.
     */
    Q_DECL_DEPRECATED void triggeredForFeatures( QgsMapLayer *layer, const QList<QgsFeature> &featureList ) SIP_DEPRECATED;

    /**
     * Triggered when action has been run for a specific feature
     *
     * \deprecated use the version with QgsMapLayerActionContext instead.
     */
    Q_DECL_DEPRECATED void triggeredForFeature( QgsMapLayer *layer, const QgsFeature &feature ) SIP_DEPRECATED;

    /**
     * Triggered when action has been run for a specific layer
     *
     * \deprecated use the version with QgsMapLayerActionContext instead.
     */
    Q_DECL_DEPRECATED void triggeredForLayer( QgsMapLayer *layer ) SIP_DEPRECATED;

    /**
     * Triggered when action has been run for a specific list of features
     *
     * \since QGIS 3.30
     */
    void triggeredForFeaturesV2( QgsMapLayer *layer, const QList<QgsFeature> &featureList, const QgsMapLayerActionContext &context );

    /**
     * Triggered when action has been run for a specific feature.
     *
     * \since QGIS 3.30
     */
    void triggeredForFeatureV2( QgsMapLayer *layer, const QgsFeature &feature, const QgsMapLayerActionContext &context );

    /**
     * Triggered when action has been run for a specific layer.
     *
     * \since QGIS 3.30
     */
    void triggeredForLayerV2( QgsMapLayer *layer, const QgsMapLayerActionContext &context );

  private:

    // true if action is only valid for a single layer
    bool mSingleLayer = false;
    // layer if action is only valid for a single layer
    QgsMapLayer *mActionLayer = nullptr;

    // true if action is only valid for a specific layer type
    bool mSpecificLayerType = false;
    // layer type if action is only valid for a specific layer type
    QgsMapLayerType mLayerType = QgsMapLayerType::VectorLayer;

    // determine if the action can be run on layer and/or single feature and/or multiple features
    Targets mTargets = Targets();

    QgsMapLayerAction::Flags mFlags = QgsMapLayerAction::Flags();
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsMapLayerAction::Targets )

/**
 * \ingroup gui
* \brief This class tracks map layer actions.
*
* QgsMapLayerActionRegistry is not usually directly created, but rather accessed through
* QgsGui::mapLayerActionRegistry().
*/
class GUI_EXPORT QgsMapLayerActionRegistry : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsMapLayerActionRegistry.
     *
     * QgsMapLayerActionRegistry is not usually directly created, but rather accessed through
     * QgsGui::mapLayerActionRegistry().
     */
    QgsMapLayerActionRegistry( QObject *parent = nullptr );

    //! Adds a map layer action to the registry
    void addMapLayerAction( QgsMapLayerAction *action );

    /**
     * Returns the map layer actions which can run on the specified layer.
     *
     * The \a context argument was added in QGIS 3.30.
     */
    QList<QgsMapLayerAction *> mapLayerActions( QgsMapLayer *layer, QgsMapLayerAction::Targets targets = QgsMapLayerAction::AllActions, const QgsMapLayerActionContext &context = QgsMapLayerActionContext() );

    //! Removes a map layer action from the registry
    bool removeMapLayerAction( QgsMapLayerAction *action );

    //! Sets the default action for a layer
    void setDefaultActionForLayer( QgsMapLayer *layer, QgsMapLayerAction *action );
    //! Returns the default action for a layer
    QgsMapLayerAction *defaultActionForLayer( QgsMapLayer *layer );

  protected:

    QList< QgsMapLayerAction * > mMapLayerActionList;

  signals:
    //! Triggered when an action is added or removed from the registry
    void changed();

  private:

    QMap< QgsMapLayer *, QgsMapLayerAction * > mDefaultLayerActionMap;

};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsMapLayerAction::Flags )

#endif // QGSMAPLAYERACTIONREGISTRY_H
