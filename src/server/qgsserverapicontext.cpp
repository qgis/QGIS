/***************************************************************************
  qgsserverapicontext.cpp - QgsServerApiContext

 ---------------------
 begin                : 13.5.2019
 copyright            : (C) 2019 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsserverapicontext.h"

#include "qgsserverrequest.h"
#include "qgsserverresponse.h"
#include "qgsproject.h"
#include "qgsserverinterface.h"

QgsServerApiContext::QgsServerApiContext( const QgsServerRequest *request, QgsServerResponse *response, const QgsProject *project, QgsServerInterface *serverInterface ):
  mRequest( request ),
  mResponse( response ),
  mProject( project ),
  mServerInterface( serverInterface )
{

}

const QgsServerRequest *QgsServerApiContext::request() const
{
  return mRequest;
}


QgsServerResponse *QgsServerApiContext::response() const
{
  return mResponse;
}


const QgsProject *QgsServerApiContext::project() const
{
  return mProject;
}

void QgsServerApiContext::setProject( const QgsProject *project )
{
  mProject = project;
}

QgsServerInterface *QgsServerApiContext::serverInterface() const
{
  return mServerInterface;
}
