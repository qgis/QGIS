/***************************************************************************
                              qgspostrequesthandler.cpp
                            ------------------------------
  begin                :  2011
  copyright            : (C) 2011 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <stdlib.h>
#include "qgspostrequesthandler.h"
#include "qgslogger.h"
#include <QDomDocument>

QgsPostRequestHandler::QgsPostRequestHandler()
{
}

QgsPostRequestHandler::~QgsPostRequestHandler()
{
}

QMap<QString, QString> QgsPostRequestHandler::parseInput()
{
  QgsDebugMsg( "QgsPostRequestHandler::parseInput" );
  QMap<QString, QString> parameters;
  QString inputString = readPostBody();
  QgsDebugMsg( inputString );

  QDomDocument doc;
  QString errorMsg;
  if ( !doc.setContent( inputString, true, &errorMsg ) )
  {
    requestStringToParameterMap( inputString, parameters );
  }
  else
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
      QgsDebugMsg( "error, no query string found but a QDomDocument" );
      return parameters; //no query string? something must be wrong...
    }

    requestStringToParameterMap( queryString, parameters );

    QDomElement docElem = doc.documentElement();
    if ( docElem.hasAttribute( "version" ) )
      parameters.insert( "VERSION", docElem.attribute( "version" ) );
    if ( docElem.hasAttribute( "service" ) )
      parameters.insert( "SERVICE", docElem.attribute( "service" ) );
    parameters.insert( "REQUEST", docElem.tagName() );
    parameters.insert( "REQUEST_BODY", inputString );
  }

  return parameters;
}
