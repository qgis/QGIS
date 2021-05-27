/***************************************************************************
                              qgsaccesscontrol.cpp
                              --------------------
  begin                : 22-05-2015
  copyright            : (C) 2008 by Stéphane Brunner
  email                : stephane dot brunner at camptocamp dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsaccesscontrol.h"
#include "qgsfeaturerequest.h"
#include "qgsmaplayer.h"
#include "qgsvectorlayer.h"


void QgsAccessControl::resolveFilterFeatures( const QList<QgsMapLayer *> &layers )
{
  for ( QgsMapLayer *l : layers )
  {
    if ( l->type() == QgsMapLayerType::VectorLayer )
    {
      const QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( l );
      mFilterFeaturesExpressions[vl->id()] = resolveFilterFeatures( vl );
    }
  }

  mResolved = true;
}

QString QgsAccessControl::resolveFilterFeatures( const QgsVectorLayer *layer ) const
{
  QStringList expressions = QStringList();
  QgsAccessControlFilterMap::const_iterator acIterator;

  for ( acIterator = mPluginsAccessControls->constBegin(); acIterator != mPluginsAccessControls->constEnd(); ++acIterator )
  {
    QString expression = acIterator.value()->layerFilterExpression( layer );
    if ( !expression.isEmpty() )
    {
      expressions.append( expression );
    }
  }

  QString expression;
  if ( !expressions.isEmpty() )
  {
    expression = QStringLiteral( "((" ).append( expressions.join( QLatin1String( ") AND (" ) ) ).append( "))" );
  }

  return expression;
}

//! Filter the features of the layer
void QgsAccessControl::filterFeatures( const QgsVectorLayer *layer, QgsFeatureRequest &featureRequest ) const
{

  QString expression;

  if ( mResolved && mFilterFeaturesExpressions.contains( layer->id() ) )
  {
    expression = mFilterFeaturesExpressions[layer->id()];
  }
  else
  {
    expression = resolveFilterFeatures( layer );
  }

  if ( !expression.isEmpty() )
  {
    featureRequest.combineFilterExpression( expression );
  }
}

//! Clone the object
QgsFeatureFilterProvider *QgsAccessControl::clone() const
{
  return new QgsAccessControl( *this );
}

//! Returns an additional subset string (typically SQL) filter
QString QgsAccessControl::extraSubsetString( const QgsVectorLayer *layer ) const
{
  QStringList sqls;
  QgsAccessControlFilterMap::const_iterator acIterator;
  for ( acIterator = mPluginsAccessControls->constBegin(); acIterator != mPluginsAccessControls->constEnd(); ++acIterator )
  {
    QString sql = acIterator.value()->layerFilterSubsetString( layer );
    if ( !sql.isEmpty() )
    {
      sqls.append( sql );
    }
  }
  return sqls.isEmpty() ? QString() : QStringLiteral( "((" ).append( sqls.join( QLatin1String( ") AND (" ) ) ).append( "))" );
}

//! Returns the layer read right
bool QgsAccessControl::layerReadPermission( const QgsMapLayer *layer ) const
{
  QgsAccessControlFilterMap::const_iterator acIterator;
  for ( acIterator = mPluginsAccessControls->constBegin(); acIterator != mPluginsAccessControls->constEnd(); ++acIterator )
  {
    if ( !acIterator.value()->layerPermissions( layer ).canRead )
    {
      return false;
    }
  }
  return true;
}

//! Returns the layer insert right
bool QgsAccessControl::layerInsertPermission( const QgsVectorLayer *layer ) const
{
  QgsAccessControlFilterMap::const_iterator acIterator;
  for ( acIterator = mPluginsAccessControls->constBegin(); acIterator != mPluginsAccessControls->constEnd(); ++acIterator )
  {
    if ( !acIterator.value()->layerPermissions( layer ).canInsert )
    {
      return false;
    }
  }
  return true;
}

//! Returns the layer update right
bool QgsAccessControl::layerUpdatePermission( const QgsVectorLayer *layer ) const
{
  QgsAccessControlFilterMap::const_iterator acIterator;
  for ( acIterator = mPluginsAccessControls->constBegin(); acIterator != mPluginsAccessControls->constEnd(); ++acIterator )
  {
    if ( !acIterator.value()->layerPermissions( layer ).canUpdate )
    {
      return false;
    }
  }
  return true;
}

//! Returns the layer delete right
bool QgsAccessControl::layerDeletePermission( const QgsVectorLayer *layer ) const
{
  QgsAccessControlFilterMap::const_iterator acIterator;
  for ( acIterator = mPluginsAccessControls->constBegin(); acIterator != mPluginsAccessControls->constEnd(); ++acIterator )
  {
    if ( !acIterator.value()->layerPermissions( layer ).canDelete )
    {
      return false;
    }
  }
  return true;
}

//! Returns the authorized layer attributes
QStringList QgsAccessControl::layerAttributes( const QgsVectorLayer *layer, const QStringList &attributes ) const
{
  QStringList currentAttributes( attributes );
  QgsAccessControlFilterMap::const_iterator acIterator;
  for ( acIterator = mPluginsAccessControls->constBegin(); acIterator != mPluginsAccessControls->constEnd(); ++acIterator )
  {
    currentAttributes = acIterator.value()->authorizedLayerAttributes( layer, currentAttributes );
  }
  return currentAttributes;
}

//! Are we authorized to modify the following geometry
bool QgsAccessControl::allowToEdit( const QgsVectorLayer *layer, const QgsFeature &feature ) const
{
  QgsAccessControlFilterMap::const_iterator acIterator;
  for ( acIterator = mPluginsAccessControls->constBegin(); acIterator != mPluginsAccessControls->constEnd(); ++acIterator )
  {
    if ( !acIterator.value()->allowToEdit( layer, feature ) )
    {
      return false;
    }
  }
  return true;
}

//! Fill the capabilities caching key
bool QgsAccessControl::fillCacheKey( QStringList &cacheKey ) const
{
  QgsAccessControlFilterMap::const_iterator acIterator;
  for ( acIterator = mPluginsAccessControls->constBegin(); acIterator != mPluginsAccessControls->constEnd(); ++acIterator )
  {
    QString newKey = acIterator.value()->cacheKey();
    if ( ! newKey.isEmpty() )
    {
      cacheKey.clear();
      return false;
    }
  }
  return true;
}

//! Register a new access control filter
void QgsAccessControl::registerAccessControl( QgsAccessControlFilter *accessControl, int priority )
{
  mPluginsAccessControls->insert( priority, accessControl );
}
