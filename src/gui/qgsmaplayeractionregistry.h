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

#include "qgsmaplayer.h"
#include "qgis_gui.h"

class QgsFeature;

/**
 * \ingroup gui
* An action which can run on map layers
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
    QgsMapLayerAction( const QString &name, QObject *parent SIP_TRANSFERTHIS, Targets targets = AllActions, const QIcon &icon = QIcon(), QgsMapLayerAction::Flags flags = nullptr );

    //! Creates a map layer action which can run only on a specific layer
    QgsMapLayerAction( const QString &name, QObject *parent SIP_TRANSFERTHIS, QgsMapLayer *layer, Targets targets = AllActions, const QIcon &icon = QIcon(), QgsMapLayerAction::Flags flags = nullptr );

    //! Creates a map layer action which can run on a specific type of layer
    QgsMapLayerAction( const QString &name, QObject *parent SIP_TRANSFERTHIS, QgsMapLayer::LayerType layerType, Targets targets = AllActions, const QIcon &icon = QIcon(), QgsMapLayerAction::Flags flags = nullptr );

    ~QgsMapLayerAction() override;

    /**
     * Layer behavior flags.
     * \since QGIS 3.0
     */
    QgsMapLayerAction::Flags flags() const;

    //! True if action can run using the specified layer
    bool canRunUsingLayer( QgsMapLayer *layer ) const;

    //! Triggers the action with the specified layer and list of feature.
    void triggerForFeatures( QgsMapLayer *layer, const QList<QgsFeature> &featureList );

    //! Triggers the action with the specified layer and feature.
    void triggerForFeature( QgsMapLayer *layer, const QgsFeature *feature );

    //! Triggers the action with the specified layer.
    void triggerForLayer( QgsMapLayer *layer );

    //! Define the targets of the action
    void setTargets( Targets targets ) {mTargets = targets;}
    //! Returns availibity of action
    const Targets &targets() const {return mTargets;}

    /**
     * Returns true if the action is only enabled for layers in editable mode.
     * \since QGIS 3.0
     */
    bool isEnabledOnlyWhenEditable() const;

  signals:
    //! Triggered when action has been run for a specific list of features
    void triggeredForFeatures( QgsMapLayer *layer, const QList<QgsFeature> &featureList );

    //! Triggered when action has been run for a specific feature
    void triggeredForFeature( QgsMapLayer *layer, const QgsFeature &feature );

    //! Triggered when action has been run for a specific layer
    void triggeredForLayer( QgsMapLayer *layer );

  private:

    // true if action is only valid for a single layer
    bool mSingleLayer = false;
    // layer if action is only valid for a single layer
    QgsMapLayer *mActionLayer = nullptr;

    // true if action is only valid for a specific layer type
    bool mSpecificLayerType = false;
    // layer type if action is only valid for a specific layer type
    QgsMapLayer::LayerType mLayerType = QgsMapLayer::VectorLayer;

    // determine if the action can be run on layer and/or single feature and/or multiple features
    Targets mTargets = nullptr;

    QgsMapLayerAction::Flags mFlags = nullptr;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsMapLayerAction::Targets )

/**
 * \ingroup gui
* This class tracks map layer actions.
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

    //! Returns the map layer actions which can run on the specified layer
    QList<QgsMapLayerAction *> mapLayerActions( QgsMapLayer *layer, QgsMapLayerAction::Targets targets = QgsMapLayerAction::AllActions );

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
