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
QString QgsAccessControlFilter::layerFilterExpression( const QgsVectorLayer* layer ) const
{
  QgsMessageLog::logMessage( "QgsAccessControlFilter plugin default layerFilterExpression called", "AccessControlFilter", QgsMessageLog::INFO );
  Q_UNUSED( layer );
  return QString();
}

/** Return an additional layer subset string (typically SQL) filter */
QString QgsAccessControlFilter::layerFilterSubsetString( const QgsVectorLayer* layer ) const
{
  QgsMessageLog::logMessage( "QgsAccessControlFilter plugin default layerFilterSQL called", "AccessControlFilter", QgsMessageLog::INFO );
  Q_UNUSED( layer );
  return QString();
}

/** Return the layer permissions */
QgsAccessControlFilter::LayerPermissions QgsAccessControlFilter::layerPermissions( const QgsMapLayer* layer ) const
{
  QgsMessageLog::logMessage( "QgsAccessControlFilter plugin default layerPermissions called", "AccessControlFilter", QgsMessageLog::INFO );
  Q_UNUSED( layer );
  LayerPermissions permissions = QgsAccessControlFilter::LayerPermissions();
  permissions.canRead = permissions.canUpdate = permissions.canInsert = permissions.canDelete = true;
  return permissions;
}

/** Return the authorized layer attributes */
QStringList QgsAccessControlFilter::authorizedLayerAttributes( const QgsVectorLayer* layer, const QStringList& attributes ) const
{
  Q_UNUSED( layer );
  QgsMessageLog::logMessage( "QgsAccessControlFilter plugin default authorizedLayerAttributes called", "AccessControlFilter", QgsMessageLog::INFO );
  return attributes;
}

/** Are we authorized to modify the feature */
bool QgsAccessControlFilter::allowToEdit( const QgsVectorLayer* layer, const QgsFeature& feature ) const
{
  QgsMessageLog::logMessage( "QgsAccessControlFilter plugin default allowToEdit called", "AccessControlFilter", QgsMessageLog::INFO );
  Q_UNUSED( layer );
  Q_UNUSED( feature );
  return true;
}

/** Cache key to used to create the capabilities cache, "" for no cache */
QString QgsAccessControlFilter::cacheKey() const
{
  return QString();
}
