/***************************************************************************
   qgshanacrsutils.h
   --------------------------------------
   Date      : 13-10-2020
   Copyright : (C) SAP SE
   Author    : Maxim Rylov
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#ifndef QGSHANACRSUTILS_H
#define QGSHANACRSUTILS_H

#include "qgscoordinatereferencesystem.h"

class QgsHanaCrsUtils
{
  public:
    QgsHanaCrsUtils() = delete;

    static double getAngularUnits( const QgsCoordinateReferenceSystem &crs );
    static bool identifyCrs( const QgsCoordinateReferenceSystem &crs, QString &name, long &srid );
};

#endif // QGSHANACRSUTILS_H
