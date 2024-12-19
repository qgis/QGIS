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
  QgsDebugMsgLevel( QStringLiteral( "QgsServerFilter plugin default requestReady called" ), 2 );
}

void QgsServerFilter::responseComplete()
{
  QgsDebugMsgLevel( QStringLiteral( "QgsServerFilter plugin default responseComplete called" ), 2 );
}

void QgsServerFilter::sendResponse()
{
  QgsDebugMsgLevel( QStringLiteral( "QgsServerFilter plugin default sendResponse called" ), 2 );
}

bool QgsServerFilter::onRequestReady()
{
  Q_NOWARN_DEPRECATED_PUSH
  requestReady();
  Q_NOWARN_DEPRECATED_POP
  return true;
}

bool QgsServerFilter::onProjectReady()
{
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
