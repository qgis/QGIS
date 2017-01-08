/***************************************************************************
                         qgsprocessingprovider.cpp
                         --------------------------
    begin                : December 2016
    copyright            : (C) 2016 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingprovider.h"
#include "qgsapplication.h"

QIcon QgsProcessingProvider::icon() const
{
  return QgsApplication::getThemeIcon( "/alg.svg" );
}
