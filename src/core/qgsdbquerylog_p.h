/***************************************************************************
  qgsdbquerylog_p.h
  ------------
  Date                 : February 2024
  Copyright            : (C) 2024 by Matthias Kuhn
  Email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDBQUERYLOG_P_H
#define QGSDBQUERYLOG_P_H

#include "qgsconfig.h"
constexpr int sQueryLoggerFilePrefixLength = CMAKE_SOURCE_DIR[sizeof( CMAKE_SOURCE_DIR ) - 1] == '/' ? sizeof( CMAKE_SOURCE_DIR ) + 1 : sizeof( CMAKE_SOURCE_DIR );
#define QgsSetQueryLogClass(entry, _class) ( entry ).initiatorClass = _class; ( entry ).origin = QString(QString( __FILE__ ).mid( sQueryLoggerFilePrefixLength ) + ':' + QString::number( __LINE__ ) + " (" + __FUNCTION__ + ")");
#define QGS_QUERY_LOG_ORIGIN QString(QString( __FILE__ ).mid( sQueryLoggerFilePrefixLength ) + ':' + QString::number( __LINE__ ) + " (" + ( __FUNCTION__ ) + ")")

#endif // QGSDBQUERYLOG_P_H
