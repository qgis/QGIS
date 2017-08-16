/***************************************************************************
                             qgslayoutgridsettings.cpp
                             -------------------------
    begin                : July 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgslayoutgridsettings.h"

QgsLayoutGridSettings::QgsLayoutGridSettings()
  : mGridResolution( QgsLayoutMeasurement( 10 ) )
{
  mGridPen = QPen( QColor( 190, 190, 190, 100 ), 0 );
  mGridPen.setCosmetic( true );
}

