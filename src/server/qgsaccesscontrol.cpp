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

#include <QStringList>


/** Filter the features of the layer */
void QgsAccessControl::filterFeatures( const QString& layerId, QgsFeatureRequest& featureRequest ) const
{
  QStringList expressions = QStringList();
  QgsAccessControlFilterMap::const_iterator acIterator;
  for ( acIterator = mPluginsAccessControls->constBegin(); acIterator != mPluginsAccessControls->constEnd(); ++acIterator )
  {
    const QString expression = acIterator.value()->layerFilterExpression( layerId );
    if ( !expression.isEmpty() )
    {
      expressions.append( expression );
    }
  }
  if ( !expressions.isEmpty() )
  {
    featureRequest.setFilterExpression( expressions.join( " AND " ) );
  }
}

/** Clone the object */
QgsFeatureFilterProvider* QgsAccessControl::clone() const
{
  return new QgsAccessControl( *this );
}

/** Return an additional subset string (typically SQL) filter */
const QString QgsAccessControl::extraSubsetString( const QString& layerId ) const
{
  QStringList sqls = QStringList();
  QgsAccessControlFilterMap::const_iterator acIterator;
  for ( acIterator = mPluginsAccessControls->constBegin(); acIterator != mPluginsAccessControls->constEnd(); ++acIterator )
  {
    const QString sql = acIterator.value()->layerFilterSubsetString( layerId );
    if ( !sql.isEmpty() )
    {
      sqls.append( sql );
    }
  }
  return sqls.isEmpty() ? QString::null : sqls.join( " AND " );
}

/** Return the layer read right */
bool QgsAccessControl::layerReadPermission( const QString& layerId ) const
{
  QgsAccessControlFilterMap::const_iterator acIterator;
  for ( acIterator = mPluginsAccessControls->constBegin(); acIterator != mPluginsAccessControls->constEnd(); ++acIterator )
  {
    if ( !acIterator.value()->layerPermissions( layerId ).canRead )
    {
      return false;
    }
  }
  return true;
}

/** Return the layer insert right */
bool QgsAccessControl::layerInsertPermission( const QString& layerId ) const
{
  QgsAccessControlFilterMap::const_iterator acIterator;
  for ( acIterator = mPluginsAccessControls->constBegin(); acIterator != mPluginsAccessControls->constEnd(); ++acIterator )
  {
    if ( !acIterator.value()->layerPermissions( layerId ).canInsert )
    {
      return false;
    }
  }
  return true;
}

/** Return the layer update right */
bool QgsAccessControl::layerUpdatePermission( const QString& layerId ) const
{
  QgsAccessControlFilterMap::const_iterator acIterator;
  for ( acIterator = mPluginsAccessControls->constBegin(); acIterator != mPluginsAccessControls->constEnd(); ++acIterator )
  {
    if ( !acIterator.value()->layerPermissions( layerId ).canUpdate )
    {
      return false;
    }
  }
  return true;
}

/** Return the layer delete right */
bool QgsAccessControl::layerDeletePermission( const QString& layerId ) const
{
  QgsAccessControlFilterMap::const_iterator acIterator;
  for ( acIterator = mPluginsAccessControls->constBegin(); acIterator != mPluginsAccessControls->constEnd(); ++acIterator )
  {
    if ( !acIterator.value()->layerPermissions( layerId ).canDelete )
    {
      return false;
    }
  }
  return true;
}

/** Return the authorized layer attributes */
const QStringList QgsAccessControl::layerAttributes( const QString& layerId, const QStringList attributes ) const
{
  QStringList currentAttributes( attributes );
  QgsAccessControlFilterMap::const_iterator acIterator;
  for ( acIterator = mPluginsAccessControls->constBegin(); acIterator != mPluginsAccessControls->constEnd(); ++acIterator )
  {
    const QStringList* newAttributes = acIterator.value()->authorizedLayerAttributes( layerId, currentAttributes );
    if ( newAttributes )
    {
      currentAttributes = *newAttributes;
    }
  }
  return currentAttributes;
}

/** Are we authorized to modify the following geometry */
bool QgsAccessControl::allowToEdit( const QString& layerId, const QgsFeature& feature ) const
{
  QgsAccessControlFilterMap::const_iterator acIterator;
  for ( acIterator = mPluginsAccessControls->constBegin(); acIterator != mPluginsAccessControls->constEnd(); ++acIterator )
  {
    if ( !acIterator.value()->allowToEdit( layerId, feature ) )
    {
      return false;
    }
  }
  return true;
}

/** Fill the capabilities caching key */
bool QgsAccessControl::fillCacheKey( QStringList& cacheKey ) const
{
  QgsAccessControlFilterMap::const_iterator acIterator;
  for ( acIterator = mPluginsAccessControls->constBegin(); acIterator != mPluginsAccessControls->constEnd(); ++acIterator )
  {
    QString newKey = acIterator.value()->cacheKey();
    if ( newKey.length() == 0 )
    {
      cacheKey.clear();
      return false;
    }
    cacheKey << newKey;
  }
  return true;
}

/** Register a new access control filter */
void QgsAccessControl::registerAccessControl( QgsAccessControlFilter* accessControl, int priority )
{
  mPluginsAccessControls->insert( priority, accessControl );
}
