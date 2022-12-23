/***************************************************************************
    qgsmaplayeraction.h
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

#ifndef QGSMAPLAYERACTION_H
#define QGSMAPLAYERACTION_H

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
class QgsMapLayerActionContext;


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

    /**
     * Creates a map layer action which can run on any layer
     * \note using AllActions as a target probably does not make a lot of sense. This default action was settled for API compatibility reasons.
     */
    QgsMapLayerAction( const QString &name, QObject *parent SIP_TRANSFERTHIS, Qgis::MapLayerActionTargets targets = Qgis::MapLayerActionTarget::AllActions, const QIcon &icon = QIcon(), Qgis::MapLayerActionFlags flags = Qgis::MapLayerActionFlags() );

    //! Creates a map layer action which can run only on a specific layer
    QgsMapLayerAction( const QString &name, QObject *parent SIP_TRANSFERTHIS, QgsMapLayer *layer, Qgis::MapLayerActionTargets targets = Qgis::MapLayerActionTarget::AllActions, const QIcon &icon = QIcon(), Qgis::MapLayerActionFlags flags = Qgis::MapLayerActionFlags() );

    //! Creates a map layer action which can run on a specific type of layer
    QgsMapLayerAction( const QString &name, QObject *parent SIP_TRANSFERTHIS, QgsMapLayerType layerType, Qgis::MapLayerActionTargets targets = Qgis::MapLayerActionTarget::AllActions, const QIcon &icon = QIcon(), Qgis::MapLayerActionFlags flags = Qgis::MapLayerActionFlags() );

    ~QgsMapLayerAction() override;

    /**
     * Layer behavior flags.
     * \since QGIS 3.0
     */
    Qgis::MapLayerActionFlags flags() const;

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
    void setTargets( Qgis::MapLayerActionTargets targets ) {mTargets = targets;}
    //! Returns availibity of action
    Qgis::MapLayerActionTargets targets() const {return mTargets;}

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
    Qgis::MapLayerActionTargets mTargets = Qgis::MapLayerActionTargets();

    Qgis::MapLayerActionFlags mFlags = Qgis::MapLayerActionFlags();
};


#endif // QGSMAPLAYERACTION_H
