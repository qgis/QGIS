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

QgsServerApiContext::QgsServerApiContext( const QString &apiRootPath, const QgsServerRequest *request, QgsServerResponse *response, const QgsProject *project, QgsServerInterface *serverInterface ):
  mApiRootPath( apiRootPath ),
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

const QString QgsServerApiContext::matchedPath() const
{
  QString path { mRequest->url().path( )};
  const auto idx { path.indexOf( mApiRootPath )};
  if ( idx != -1 )
  {
    path.truncate( idx + mApiRootPath.length() );
    return path;
  }
  else
  {
    return QString();
  }
}

QString QgsServerApiContext::apiRootPath() const
{
  return mApiRootPath;
}

void QgsServerApiContext::setRequest( const QgsServerRequest *request )
{
  mRequest = request;
}

QString QgsServerApiContext::handlerPath() const
{
  const QUrl url { request()->url() };
  const QString urlBasePath { matchedPath() };
  if ( ! urlBasePath.isEmpty() )
  {
    return url.path().mid( urlBasePath.length() );
  }
  else
  {
    return url.path();
  }
}
