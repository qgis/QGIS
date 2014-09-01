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
#include <QList>
#include <QMap>
#include <QAction>

#include "qgsmaplayer.h"

class QgsFeature;

/**
* An action which can run on map layers
*/
class GUI_EXPORT QgsMapLayerAction : public QAction
{
    Q_OBJECT
    Q_FLAGS( Availability )

  public:
    enum Target
    {
      Layer = 1,
      SingleFeature = 2,
      MultipleFeatures = 4,
      AllActions = Layer | SingleFeature | MultipleFeatures
    };
    Q_DECLARE_FLAGS( Targets, Target )

    //! Creates a map layer action which can run on any layer
    //! @note using AllActions as a target probably does not make a lot of sense. This default action was settled for API compatiblity reasons.
    QgsMapLayerAction( QString name, QObject *parent, Targets targets = AllActions );

    /**Creates a map layer action which can run only on a specific layer*/
    QgsMapLayerAction( QString name, QObject *parent, QgsMapLayer* layer, Targets targets = AllActions );

    /**Creates a map layer action which can run on a specific type of layer*/
    QgsMapLayerAction( QString name, QObject *parent, QgsMapLayer::LayerType layerType, Targets targets = AllActions );

    ~QgsMapLayerAction();

    /** True if action can run using the specified layer */
    bool canRunUsingLayer( QgsMapLayer* layer ) const;

    /** Triggers the action with the specified layer and list of feature. */
    void triggerForFeatures( QgsMapLayer* layer, QList<const QgsFeature*> featureList );

    /** Triggers the action with the specified layer and feature.  */
    void triggerForFeature( QgsMapLayer* layer, const QgsFeature* feature );

    /** Triggers the action with the specified layer. */
    void triggerForLayer( QgsMapLayer* layer );

    /** Define the targets of the action */
    void setTargets( Targets targets ) {mTargets = targets;}
    /** Return availibity of action */
    const Targets& targets() const {return mTargets;}

  signals:
    /** Triggered when action has been run for a specific list of features */
    void triggeredForFeatures( QgsMapLayer* layer, QList<const QgsFeature*> featureList );

    /** Triggered when action has been run for a specific feature */
    void triggeredForFeature( QgsMapLayer* layer, const QgsFeature* feature );

    /** Triggered when action has been run for a specific layer */
    void triggeredForLayer( QgsMapLayer* layer );

  private:

    // true if action is only valid for a single layer
    bool mSingleLayer;
    // layer if action is only valid for a single layer
    QgsMapLayer* mActionLayer;

    // true if action is only valid for a specific layer type
    bool mSpecificLayerType;
    // layer type if action is only valid for a specific layer type
    QgsMapLayer::LayerType mLayerType;

    // determine if the action can be run on layer and/or single feature and/or multiple features
    Targets mTargets;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsMapLayerAction::Targets )

/**
* This class tracks map layer actions
*/
class GUI_EXPORT QgsMapLayerActionRegistry : public QObject
{
    Q_OBJECT

  public:
    //! Returns the instance pointer, creating the object on the first call
    static QgsMapLayerActionRegistry * instance();

    ~QgsMapLayerActionRegistry();

    /**Adds a map layer action to the registry*/
    void addMapLayerAction( QgsMapLayerAction * action );

    /**Returns the map layer actions which can run on the specified layer*/
    QList<QgsMapLayerAction *> mapLayerActions( QgsMapLayer* layer, QgsMapLayerAction::Targets targets = QgsMapLayerAction::AllActions );

    /**Removes a map layer action from the registry*/
    bool removeMapLayerAction( QgsMapLayerAction *action );

    /**Sets the default action for a layer*/
    void setDefaultActionForLayer( QgsMapLayer* layer, QgsMapLayerAction* action );
    /**Returns the default action for a layer*/
    QgsMapLayerAction * defaultActionForLayer( QgsMapLayer* layer );

  protected:
    //! protected constructor
    QgsMapLayerActionRegistry( QObject * parent = 0 );

  signals:
    /** Triggered when an action is added or removed from the registry */
    void changed();

  private:

    static QgsMapLayerActionRegistry *mInstance;

    QList< QgsMapLayerAction* > mMapLayerActionList;

    QMap< QgsMapLayer*, QgsMapLayerAction* > mDefaultLayerActionMap;

};

#endif // QGSMAPLAYERACTIONREGISTRY_H
