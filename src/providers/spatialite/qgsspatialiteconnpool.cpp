/***************************************************************************
    qgsspatialiteconnpool.cpp
    ---------------------
    begin                : February 2014
    copyright            : (C) 2014 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsspatialiteconnpool.h"


QgsSpatiaLiteConnPool* QgsSpatiaLiteConnPool::instance()
{
  static QgsSpatiaLiteConnPool sInstance;
  return &sInstance;
}
