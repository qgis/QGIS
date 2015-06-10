/***************************************************************************
                          qgsseerverfilter.cpp
 Server I/O filters class for Qgis Mapserver for use by plugins
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

/**
 * QgsServerFilter
 * Class defining I/O filters for Qgis Mapserver and
 * implemented in plugins
 *
 */

QgsServerFilter::QgsServerFilter( QgsServerInterface *serverInterface ):
    mServerInterface( serverInterface )
{
}

QgsServerFilter::~QgsServerFilter()
{
}


void QgsServerFilter::requestReady()
{
  QgsDebugMsg( "QgsServerFilter plugin default requestReady called" );
}

void QgsServerFilter::responseComplete()
{
  QgsDebugMsg( "QgsServerFilter plugin default responseComplete called" );
}


void QgsServerFilter::sendResponse()
{
  QgsDebugMsg( "QgsServerFilter plugin default sendResponse called" );
}
