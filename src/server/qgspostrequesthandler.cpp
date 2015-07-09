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

QgsPostRequestHandler::QgsPostRequestHandler( const bool captureOutput /*= FALSE*/ )
    : QgsHttpRequestHandler( captureOutput )
{

}

QgsPostRequestHandler::~QgsPostRequestHandler()
{
}

void QgsPostRequestHandler::parseInput()
{
  QgsDebugMsg( "QgsPostRequestHandler::parseInput" );
  QString inputString = readPostBody();
  QgsDebugMsg( inputString );

  //Map parameter in QUERY_STRING?
  const char* qs = getenv( "QUERY_STRING" );
  QMap<QString, QString> getParameters;
  QString queryString;
  QString mapParameter;
  if ( qs )
  {
    queryString = QString( qs );
    requestStringToParameterMap( queryString, getParameters );
    mapParameter = getParameters.value( "MAP" );
  }


  QDomDocument doc;
  QString errorMsg;
  if ( !doc.setContent( inputString, true, &errorMsg ) )
  {
    requestStringToParameterMap( inputString, mParameterMap );
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
      return; //no query string? something must be wrong...
    }

    requestStringToParameterMap( queryString, mParameterMap );

    QDomElement docElem = doc.documentElement();
    if ( docElem.hasAttribute( "version" ) )
      mParameterMap.insert( "VERSION", docElem.attribute( "version" ) );
    if ( docElem.hasAttribute( "service" ) )
      mParameterMap.insert( "SERVICE", docElem.attribute( "service" ) );
    mParameterMap.insert( "REQUEST", docElem.tagName() );
    mParameterMap.insert( "REQUEST_BODY", inputString );
  }

  if ( !mapParameter.isEmpty() && !mParameterMap.contains( "MAP" ) )
  {
    mParameterMap.insert( "MAP", mapParameter );
  }
}
