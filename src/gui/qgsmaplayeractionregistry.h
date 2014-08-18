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
    enum AvailabityFlag
    {
      Layer = 1,
      Feature = 2,
      LayerAndFeature = Layer | Feature
    };
    Q_DECLARE_FLAGS( Availability, AvailabityFlag )

    /**Creates a map layer action which can run on any layer*/
    QgsMapLayerAction( QString name, QObject *parent, Availability availability = LayerAndFeature );
    /**Creates a map layer action which can run only on a specific layer*/
    QgsMapLayerAction( QString name, QObject *parent, QgsMapLayer* layer, Availability availability = LayerAndFeature );
    /**Creates a map layer action which can run on a specific type of layer*/
    QgsMapLayerAction( QString name, QObject *parent, QgsMapLayer::LayerType layerType, Availability availability = LayerAndFeature );

    ~QgsMapLayerAction();

    /** True if action can run using the specified layer */
    bool canRunUsingLayer( QgsMapLayer* layer ) const;

    /** Triggers the action with the specified layer and feature. This also emits the triggeredForLayer( QgsMapLayer *)
     * and triggered() slots */
    void triggerForFeature( QgsMapLayer* layer, QgsFeature* feature );

    /** Triggers the action with the specified layer. This also emits the triggered() slot. */
    void triggerForLayer( QgsMapLayer* layer );

    /** Define the availibility of the action */
    void setAvailability( Availability availability ) {mAvailability = availability;}
    /** Return availibity of action */
    Availability availability() const {return mAvailability;}

  signals:
    /** Triggered when action has been run for a specific feature */
    void triggeredForFeature( QgsMapLayer* layer, QgsFeature* feature );

    /** Triggered when action has been run for a specific layer */
    void triggeredForLayer( QgsMapLayer* layer );

  private:

    //true if action is only valid for a single layer
    bool mSingleLayer;
    //layer if action is only valid for a single layer
    QgsMapLayer* mActionLayer;

    //true if action is only valid for a specific layer type
    bool mSpecificLayerType;
    //layer type if action is only valid for a specific layer type
    QgsMapLayer::LayerType mLayerType;

    // determine if the action can be run on feature and/or layer
    Availability mAvailability;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsMapLayerAction::Availability )

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
    QList<QgsMapLayerAction *> mapLayerActions( QgsMapLayer* layer );

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
