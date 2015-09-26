/***************************************************************************
                            qgsgrassaddfeature.h
                             -------------------
    begin                : September, 2015
    copyright            : (C) 2015 by Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgrassaddfeature.h"

QgsGrassAddFeature::QgsGrassAddFeature( QgsMapCanvas* canvas, CaptureMode mode )
    : QgsMapToolAddFeature( canvas, mode )
{
  mCheckGeometryType = false;
}

QgsGrassAddFeature::~QgsGrassAddFeature()
{
}
