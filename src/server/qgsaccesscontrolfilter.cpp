/***************************************************************************
                          qgsaccesscontrolplugin.cpp
                          --------------------------
 Access control interface for Qgis Server plugins

  begin                : 2015-05-19
  copyright            : (C) 2015 by Stéphane Brunner
  email                : stephane dot brunner at camptocamp dot org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsaccesscontrolfilter.h"
#include "qgsmessagelog.h"
#include "qgsfeature.h"

#include <QString>
#include <QStringList>


/** Constructor */
QgsAccessControlFilter::QgsAccessControlFilter( const QgsServerInterface* serverInterface ):
    mServerInterface( serverInterface )
{
}

/** Destructor */
QgsAccessControlFilter::~QgsAccessControlFilter()
{
}

/** Return an additional layer expression filter */
const QString QgsAccessControlFilter::layerFilterExpression( const QString& layerId ) const
{
  QgsMessageLog::logMessage( "QgsAccessControlFilter plugin default layerFilterExpression called", "AccessControlFilter", QgsMessageLog::INFO );
  Q_UNUSED( layerId );
  return nullptr;
}

/** Return an additional layer subset string (typically SQL) filter */
const QString QgsAccessControlFilter::layerFilterSubsetString( const QString& layerId ) const
{
  QgsMessageLog::logMessage( "QgsAccessControlFilter plugin default layerFilterSQL called", "AccessControlFilter", QgsMessageLog::INFO );
  Q_UNUSED( layerId );
  return nullptr;
}

/** Return the layer permissions */
const QgsAccessControlFilter::LayerPermissions QgsAccessControlFilter::layerPermissions( const QString& layerId ) const
{
  QgsMessageLog::logMessage( "QgsAccessControlFilter plugin default layerPermissions called", "AccessControlFilter", QgsMessageLog::INFO );
  Q_UNUSED( layerId );
  LayerPermissions permissions = QgsAccessControlFilter::LayerPermissions();
  permissions.canRead = permissions.canUpdate = permissions.canInsert = permissions.canDelete = true;
  return permissions;
}

/** Return the authorized layer attributes */
const QStringList* QgsAccessControlFilter::authorizedLayerAttributes( const QString& layerId, const QStringList& attributes ) const
{
  Q_UNUSED( layerId );
  Q_UNUSED( attributes );
  QgsMessageLog::logMessage( "QgsAccessControlFilter plugin default authorizedLayerAttributes called", "AccessControlFilter", QgsMessageLog::INFO );
  return nullptr;
}

/** Are we authorized to modify the feature */
bool QgsAccessControlFilter::allowToEdit( const QString& layerId, const QgsFeature& feature ) const
{
  QgsMessageLog::logMessage( "QgsAccessControlFilter plugin default allowToEdit called", "AccessControlFilter", QgsMessageLog::INFO );
  Q_UNUSED( layerId );
  Q_UNUSED( feature );
  return true;
}

/** Cache key to used to create the capabilities cache, "" for no cache */
const QString QgsAccessControlFilter::cacheKey() const
{
  return "";
}
