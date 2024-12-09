/***************************************************************************
    qgsoapifoptionsrequest.cpp
    --------------------------
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

#include "qgsoapifoptionsrequest.h"
#include "moc_qgsoapifoptionsrequest.cpp"

QgsOapifOptionsRequest::QgsOapifOptionsRequest( const QgsDataSourceUri &uri )
  : QgsBaseNetworkRequest( QgsAuthorizationSettings( uri.username(), uri.password(), uri.authConfigId() ), "OAPIF" )
{
}

QString QgsOapifOptionsRequest::errorMessageWithReason( const QString &reason )
{
  return tr( "Download of OPTIONS failed: %1" ).arg( reason );
}
