/***************************************************************************
    qgsgetrequesthandler.cpp
    ---------------------
    begin                : August 2010
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsgetrequesthandler.h"
#include "qgslogger.h"
#include "qgsremotedatasourcebuilder.h"
#include <QStringList>
#include <QUrl>
#include <stdlib.h>

QgsGetRequestHandler::QgsGetRequestHandler()
    : QgsHttpRequestHandler()
{
}

void QgsGetRequestHandler::parseInput()
{
  QString queryString;

  const char* qs = getenv( "QUERY_STRING" );
  if ( qs )
  {
    queryString = QString( qs );
    QgsDebugMsg( "query string is: " + queryString );
  }
  else
  {
    QgsDebugMsg( "error, no query string found" );
    return; //no query string? something must be wrong...
  }

  requestStringToParameterMap( queryString, mParameterMap );
}
