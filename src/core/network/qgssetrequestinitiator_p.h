/***************************************************************************
                          qgssetrequestinitiator.h  -  description
                             -------------------
    begin                : 2024-02-10
    copyright            : (C) 2024 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSETREQUESTINITIATOR_P_H
#define QGSSETREQUESTINITIATOR_P_H

#include "qgsnetworkaccessmanager.h"

constexpr int sFilePrefixLength = CMAKE_SOURCE_DIR[sizeof( CMAKE_SOURCE_DIR ) - 1] == '/' ? sizeof( CMAKE_SOURCE_DIR ) + 1 : sizeof( CMAKE_SOURCE_DIR );

#define QgsSetRequestInitiatorClass(request, _class) ( request ).setAttribute( static_cast< QNetworkRequest::Attribute >( QgsNetworkRequestParameters::AttributeInitiatorClass ), _class ); ( request ).setAttribute( static_cast< QNetworkRequest::Attribute >( QgsNetworkRequestParameters::AttributeInitiatorRequestId ), QString(QString( __FILE__ ).mid( sFilePrefixLength ) + ':' + QString::number( __LINE__ ) + " (" + ( __FUNCTION__ ) + ")") );
#define QgsSetRequestInitiatorId(request, str) ( request ).setAttribute( static_cast< QNetworkRequest::Attribute >( QgsNetworkRequestParameters::AttributeInitiatorRequestId ), QString(QString( __FILE__ ).mid( sFilePrefixLength ) + ':' + QString::number( __LINE__ ) + " (" + ( __FUNCTION__ ) + "): " + ( str ) ) );

#define QgsSetCPLHTTPFetchOverriderInitiatorClass(overrider, _class) QgsSetRequestInitiatorClass((overrider), _class)
#endif // QGSSETREQUESTINITIATOR_P_H
