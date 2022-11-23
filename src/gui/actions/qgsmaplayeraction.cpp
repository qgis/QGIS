/***************************************************************************
    qgsmaplayeraction.cpp
    -----------------------------
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

#include "qgsmaplayeraction.h"
#include "qgsgui.h"
#include "qgsmaplayeractionregistry.h"
#include "qgsmaplayer.h"
#include "qgsvectorlayer.h"
#include "qgsmaplayeractioncontext.h"

QgsMapLayerAction::QgsMapLayerAction( const QString &name, QObject *parent, Qgis::MapLayerActionTargets targets, const QIcon &icon, Qgis::MapLayerActionFlags flags )
  : QAction( icon, name, parent )
  , mTargets( targets )
  , mFlags( flags )
{
}

QgsMapLayerAction::QgsMapLayerAction( const QString &name, QObject *parent, QgsMapLayer *layer, Qgis::MapLayerActionTargets targets, const QIcon &icon, Qgis::MapLayerActionFlags flags )
  : QAction( icon, name, parent )
  , mSingleLayer( true )
  , mActionLayer( layer )
  , mTargets( targets )
  , mFlags( flags )
{
}

QgsMapLayerAction::QgsMapLayerAction( const QString &name, QObject *parent, QgsMapLayerType layerType, Qgis::MapLayerActionTargets targets, const QIcon &icon, Qgis::MapLayerActionFlags flags )
  : QAction( icon, name, parent )
  , mSpecificLayerType( true )
  , mLayerType( layerType )
  , mTargets( targets )
  , mFlags( flags )
{
}

QgsMapLayerAction::~QgsMapLayerAction()
{
  //remove action from registry
  QgsGui::mapLayerActionRegistry()->removeMapLayerAction( this );
}

Qgis::MapLayerActionFlags QgsMapLayerAction::flags() const
{
  return mFlags;
}

bool QgsMapLayerAction::canRunUsingLayer( QgsMapLayer *layer ) const
{
  return canRunUsingLayer( layer, QgsMapLayerActionContext() );
}

bool QgsMapLayerAction::canRunUsingLayer( QgsMapLayer *layer, const QgsMapLayerActionContext & ) const
{
  if ( mFlags & Qgis::MapLayerActionFlag::EnabledOnlyWhenEditable )
  {
    // action is only enabled for editable layers
    if ( !layer )
      return false;
    if ( layer->type() != QgsMapLayerType::VectorLayer )
      return false;
    if ( !qobject_cast<QgsVectorLayer *>( layer )->isEditable() )
      return false;
  }

  //check layer details
  if ( !mSingleLayer && !mSpecificLayerType )
  {
    //action is not a single layer of specific layer type action,
    //so return true
    return true;
  }
  if ( mSingleLayer && layer == mActionLayer )
  {
    //action is a single layer type and layer matches
    return true;
  }
  else if ( mSpecificLayerType && layer && layer->type() == mLayerType )
  {
    //action is for a layer type and layer type matches
    return true;
  }

  return false;
}

void QgsMapLayerAction::triggerForFeatures( QgsMapLayer *layer, const QList<QgsFeature> &featureList )
{
  Q_NOWARN_DEPRECATED_PUSH
  emit triggeredForFeatures( layer, featureList );
  Q_NOWARN_DEPRECATED_POP
}

void QgsMapLayerAction::triggerForFeature( QgsMapLayer *layer, const QgsFeature &feature )
{
  Q_NOWARN_DEPRECATED_PUSH
  emit triggeredForFeature( layer, feature );
  Q_NOWARN_DEPRECATED_POP
}

void QgsMapLayerAction::triggerForLayer( QgsMapLayer *layer )
{
  Q_NOWARN_DEPRECATED_PUSH
  emit triggeredForLayer( layer );
  Q_NOWARN_DEPRECATED_POP
}

void QgsMapLayerAction::triggerForFeatures( QgsMapLayer *layer, const QList<QgsFeature> &featureList, const QgsMapLayerActionContext &context )
{
  emit triggeredForFeaturesV2( layer, featureList, context );
}

void QgsMapLayerAction::triggerForFeature( QgsMapLayer *layer, const QgsFeature &feature, const QgsMapLayerActionContext &context )
{
  emit triggeredForFeatureV2( layer, feature, context );
}

void QgsMapLayerAction::triggerForLayer( QgsMapLayer *layer, const QgsMapLayerActionContext &context )
{
  emit triggeredForLayerV2( layer, context );
}

bool QgsMapLayerAction::isEnabledOnlyWhenEditable() const
{
  return mFlags & Qgis::MapLayerActionFlag::EnabledOnlyWhenEditable;
}

