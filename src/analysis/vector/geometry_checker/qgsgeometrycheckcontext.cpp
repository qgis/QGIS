/***************************************************************************
    qgsgeometrycheckcontext.h
    ---------------------
    begin                : September 2018
    copyright            : (C) 2018 Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeometrycheckcontext.h"
#include "qgsreadwritelocker.h"
#include "qgsthreadingutils.h"
#include "qgsvectorlayer.h"

QgsGeometryCheckContext::QgsGeometryCheckContext( int precision, const QgsCoordinateReferenceSystem &mapCrs, const QgsCoordinateTransformContext &transformContext )
  : tolerance( std::pow( 10, -precision ) )
  , reducedTolerance( std::pow( 10, -precision / 2 ) )
  , mapCrs( mapCrs )
  , transformContext( transformContext )
{
}
