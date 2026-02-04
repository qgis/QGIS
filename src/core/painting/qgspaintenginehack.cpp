/***************************************************************************
  qgspaintenginehack.cpp
  Hack paint engine flags
  -------------------
         begin                : July 2012
         copyright            : (C) Juergen E. Fischer
         email                : jef at norbit dot de

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsconfig.h"
#include "qgspaintenginehack.h"

// Hack to workaround Qt #5114 by disabling PatternTransform
void QgsPaintEngineHack::fixFlags()
{}

void QgsPaintEngineHack::fixEngineFlags( QPaintEngine *engine )
{
  ( void )engine;
}
