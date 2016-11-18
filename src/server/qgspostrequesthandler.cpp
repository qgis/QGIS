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
#include "qgsmessagelog.h"
#include <QDomDocument>

QgsPostRequestHandler::QgsPostRequestHandler( const bool captureOutput )
    : QgsHttpRequestHandler( captureOutput )
{
}

QgsPostRequestHandler::~QgsPostRequestHandler()
{
}

void QgsPostRequestHandler::parseInput()
{
  QgsMessageLog::logMessage( QStringLiteral( "QgsPostRequestHandler::parseInput" ) );

  QString inputString = readPostBody();
  QgsMessageLog::logMessage( inputString );

  //Map parameter in QUERY_STRING?
  const char* qs = getenv( "QUERY_STRING" );
  QMap<QString, QString> getParameters;
  QString queryString;
  QString mapParameter;
  if ( qs )
  {
    queryString = QString( qs );
    requestStringToParameterMap( queryString, getParameters );
    mapParameter = getParameters.value( QStringLiteral( "MAP" ) );
  }

  QDomDocument doc;
  QString errorMsg;
  int line;
  int column;
  if ( !doc.setContent( inputString, true, &errorMsg, &line, &column ) )
  {
    char* requestMethod = getenv( "REQUEST_METHOD" );
    if ( requestMethod && strcmp( requestMethod, "POST" ) == 0 )
    {
      QgsMessageLog::logMessage( QStringLiteral( "Error at line %1, column %2: %3." ).arg( line ).arg( column ).arg( errorMsg ) );
    }
    requestStringToParameterMap( inputString, mParameterMap );
  }
  else
  {
    QString queryString;
    const char* qs = getenv( "QUERY_STRING" );
    if ( qs )
    {
      queryString = QString( qs );
      QgsMessageLog::logMessage( "query string is: " + queryString );
    }
    else
    {
      QgsMessageLog::logMessage( QStringLiteral( "error, no query string found but a QDomDocument" ) );
      return; //no query string? something must be wrong…
    }

    requestStringToParameterMap( queryString, mParameterMap );

    QDomElement docElem = doc.documentElement();
    if ( docElem.hasAttribute( QStringLiteral( "version" ) ) )
      mParameterMap.insert( QStringLiteral( "VERSION" ), docElem.attribute( QStringLiteral( "version" ) ) );
    if ( docElem.hasAttribute( QStringLiteral( "service" ) ) )
      mParameterMap.insert( QStringLiteral( "SERVICE" ), docElem.attribute( QStringLiteral( "service" ) ) );
    mParameterMap.insert( QStringLiteral( "REQUEST" ), docElem.tagName() );
    mParameterMap.insert( QStringLiteral( "REQUEST_BODY" ), inputString );
  }

  if ( !mapParameter.isEmpty() && !mParameterMap.contains( QStringLiteral( "MAP" ) ) )
  {
    mParameterMap.insert( QStringLiteral( "MAP" ), mapParameter );
  }
}
