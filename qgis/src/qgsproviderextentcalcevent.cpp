/***************************************************************************
      qgsproviderextentcalcevent.cpp  -  Notification that the exact extent
                                         of a layer has been calculated.
                             -------------------
    begin                : Feb 1, 2005
    copyright            : (C) 2005 by Brendan Morley
    email                : morb at ozemail dot com dot au
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include "qgsproviderextentcalcevent.h"

QgsProviderExtentCalcEvent::QgsProviderExtentCalcEvent( QgsRect* layerExtent )
    : QCustomEvent( QGis::ProviderExtentCalcEvent ),
      le( layerExtent )
{
  // NO-OP
}


QgsRect* QgsProviderExtentCalcEvent::layerExtent() const
{
  return le;
}

