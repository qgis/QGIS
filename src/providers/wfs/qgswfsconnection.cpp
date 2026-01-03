/***************************************************************************
    qgswfsconnection.cpp
    ---------------------
    begin                : February 2016
    copyright            : (C) 2016 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswfsconnection.h"

#include "qgslogger.h"
#include "qgssettingsentryenumflag.h"
#include "qgssettingsentryimpl.h"
#include "qgswfsconstants.h"

#include "moc_qgswfsconnection.cpp"

static const QString SERVICE_WFS = u"WFS"_s;


QgsWfsConnection::QgsWfsConnection( const QString &connName )
  : QgsOwsConnection( SERVICE_WFS, connName )
{
  const QStringList detailsParameters = { service().toLower(), connName };
  const QString version = settingsVersion->value( detailsParameters );
  if ( !version.isEmpty() )
  {
    mUri.removeParam( QgsWFSConstants::URI_PARAM_VERSION ); // setParam allow for duplicates!
    mUri.setParam( QgsWFSConstants::URI_PARAM_VERSION, version );
  }

  const QString maxnumfeatures = settingsMaxNumFeatures->value( detailsParameters );
  if ( !maxnumfeatures.isEmpty() )
  {
    mUri.removeParam( QgsWFSConstants::URI_PARAM_MAXNUMFEATURES ); // setParam allow for duplicates!
    mUri.setParam( QgsWFSConstants::URI_PARAM_MAXNUMFEATURES, maxnumfeatures );
  }

  const QString pagesize = settingsPagesize->value( detailsParameters );
  if ( !pagesize.isEmpty() )
  {
    mUri.removeParam( QgsWFSConstants::URI_PARAM_PAGE_SIZE ); // setParam allow for duplicates!
    mUri.setParam( QgsWFSConstants::URI_PARAM_PAGE_SIZE, pagesize );
  }

  if ( settingsPagingEnabled->exists( detailsParameters ) )
  {
    mUri.removeParam( QgsWFSConstants::URI_PARAM_PAGING_ENABLED ); // setParam allow for duplicates!
    mUri.setParam( QgsWFSConstants::URI_PARAM_PAGING_ENABLED, settingsPagingEnabled->value( detailsParameters ) );
  }

  if ( settingsWfsForceInitialGetFeature->exists( detailsParameters ) )
  {
    mUri.removeParam( QgsWFSConstants::URI_PARAM_FORCE_INITIAL_GET_FEATURE ); // setParam allow for duplicates!
    mUri.setParam( QgsWFSConstants::URI_PARAM_FORCE_INITIAL_GET_FEATURE, settingsWfsForceInitialGetFeature->value( detailsParameters ) ? u"true"_s : u"false"_s );
  }

  if ( settingsPreferCoordinatesForWfsT11->exists( detailsParameters ) )
  {
    mUri.removeParam( QgsWFSConstants::URI_PARAM_WFST_1_1_PREFER_COORDINATES ); // setParam allow for duplicates!
    mUri.setParam( QgsWFSConstants::URI_PARAM_WFST_1_1_PREFER_COORDINATES, settingsPreferCoordinatesForWfsT11->value( detailsParameters ) ? u"true"_s : u"false"_s );
  }

  if ( settingsPreferredHttpMethod->exists( detailsParameters ) )
  {
    mUri.removeParam( QgsWFSConstants::URI_PARAM_HTTPMETHOD ); // setParam allow for duplicates!
    switch ( settingsPreferredHttpMethod->value( detailsParameters ) )
    {
      case Qgis::HttpMethod::Get:
        // default, we don't set to explicitly set
        break;

      case Qgis::HttpMethod::Post:
        mUri.setParam( QgsWFSConstants::URI_PARAM_HTTPMETHOD, u"post"_s );
        break;

      case Qgis::HttpMethod::Head:
      case Qgis::HttpMethod::Put:
      case Qgis::HttpMethod::Delete:
        // not supported
        break;
    }
  }

  if ( settingsWfsFeatureMode->exists( detailsParameters ) )
  {
    mUri.removeParam( QgsWFSConstants::URI_PARAM_FEATURE_MODE ); // setParam allow for duplicates!
    const QString featureMode = settingsWfsFeatureMode->value( detailsParameters );
    if ( featureMode != "default"_L1 )
    {
      mUri.setParam( QgsWFSConstants::URI_PARAM_FEATURE_MODE, featureMode );
    }
  }

  QgsDebugMsgLevel( u"WFS full uri: '%1'."_s.arg( QString( mUri.uri() ) ), 4 );
}

QStringList QgsWfsConnection::connectionList()
{
  return QgsOwsConnection::connectionList( SERVICE_WFS );
}

void QgsWfsConnection::deleteConnection( const QString &name )
{
  QgsOwsConnection::deleteConnection( SERVICE_WFS, name );
}

QString QgsWfsConnection::selectedConnection()
{
  return QgsOwsConnection::selectedConnection( SERVICE_WFS );
}

void QgsWfsConnection::setSelectedConnection( const QString &name )
{
  QgsOwsConnection::setSelectedConnection( SERVICE_WFS, name );
}
