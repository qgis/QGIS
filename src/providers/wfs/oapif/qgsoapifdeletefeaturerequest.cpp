/***************************************************************************
    qgsoapifdeletefeaturerequest.cpp
    --------------------------------
    begin                : March 2023
    copyright            : (C) 2023 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslogger.h"
#include "qgsoapifdeletefeaturerequest.h"
#include "moc_qgsoapifdeletefeaturerequest.cpp"

QgsOapifDeleteFeatureRequest::QgsOapifDeleteFeatureRequest( const QgsDataSourceUri &uri )
  : QgsBaseNetworkRequest( QgsAuthorizationSettings( uri.username(), uri.password(), uri.authConfigId() ), "OAPIF" )
{
}

QString QgsOapifDeleteFeatureRequest::errorMessageWithReason( const QString &reason )
{
  return tr( "Delete Feature request failed: %1" ).arg( reason );
}
