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

#include "qgspostrequesthandler.h"
#include "qgslogger.h"

QgsPostRequestHandler::QgsPostRequestHandler()
{
}

QgsPostRequestHandler::~QgsPostRequestHandler()
{
}

std::map<QString, QString> QgsPostRequestHandler::parseInput()
{
  QgsDebugMsg( "QgsPostRequestHandler::parseInput" );
  std::map<QString, QString> parameters;
  QString inputString = readPostBody();
  QgsDebugMsg( inputString );
  requestStringToParameterMap( inputString, parameters );
  return parameters;
}
