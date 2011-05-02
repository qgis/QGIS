/***************************************************************************
    qgispython.cpp - python support in QGIS
    ---------------------
    begin                : May 2008
    copyright            : (C) 2008 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include "qgis.h"
#include "qgspythonutilsimpl.h"

QGISEXTERN QgsPythonUtils* instance()
{
  return new QgsPythonUtilsImpl();
}
