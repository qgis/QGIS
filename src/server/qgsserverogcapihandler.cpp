/***************************************************************************
  qgsserverogcapihandler.cpp - QgsServerOgcApiHandler

 ---------------------
 begin                : 10.7.2019
 copyright            : (C) 2019 by ale
 email                : [your-email-here]
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsserverogcapihandler.h"


void QgsServerOgcApiHandler::handleRequest( const QgsServerApiContext &context ) const
{

}


QVariantMap QgsServerOgcApiHandler::validate( const QgsServerApiContext &context )
{
  QVariantMap result ;
  const auto constParameters { parameters() };
  for ( const auto &p : constParameters )
  {
    result[p.name()] = p.value( context );
  }
  return result;
}
