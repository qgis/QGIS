/***************************************************************************
                          qgsseerverfilter.cpp
 Server I/O filters class for QGIS Server for use by plugins
                             -------------------
  begin                : 2014-09-10
  copyright            : (C) 2014 by Alessandro Pasotti
  email                : a dot pasotti at itopen dot it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsserverfilter.h"
#include "qgslogger.h"
#include "qgis.h"

/**
 * QgsServerFilter
 * Class defining I/O filters for QGIS Server and
 * implemented in plugins
 *
 */

QgsServerFilter::QgsServerFilter( QgsServerInterface *serverInterface ):
  mServerInterface( serverInterface )
{
}

void QgsServerFilter::requestReady()
{
  QgsDebugMsg( QStringLiteral( "QgsServerFilter plugin default requestReady called" ) );
}

void QgsServerFilter::responseComplete()
{
  QgsDebugMsg( QStringLiteral( "QgsServerFilter plugin default responseComplete called" ) );
}

void QgsServerFilter::sendResponse()
{
  QgsDebugMsg( QStringLiteral( "QgsServerFilter plugin default sendResponse called" ) );
}

bool QgsServerFilter::onRequestReady()
{
  Q_NOWARN_DEPRECATED_PUSH
  requestReady();
  Q_NOWARN_DEPRECATED_POP
  return true;
}

bool QgsServerFilter::onResponseComplete()
{
  Q_NOWARN_DEPRECATED_PUSH
  responseComplete();
  Q_NOWARN_DEPRECATED_POP
  return true;
}

bool QgsServerFilter::onSendResponse()
{
  Q_NOWARN_DEPRECATED_PUSH
  sendResponse();
  Q_NOWARN_DEPRECATED_POP
  return true;
}


