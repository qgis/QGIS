/***************************************************************************
     qgsleastsquares.h
     --------------------------------------
    Date                 : Sun Sep 16 12:03:47 AKDT 2007
    Copyright            : (C) 2007 by Gary E. Sherman
    Email                : sherman at mrcc dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLEASTSQUARES_H
#define QGSLEASTSQUARES_H

#include <QVector>
#include <cstdarg>
#include <stdexcept>

#include "qgspoint.h"


class QgsLeastSquares
{
  public:
    static void linear( QVector<QgsPoint> mapCoords,
                        QVector<QgsPoint> pixelCoords,
                        QgsPoint& origin, double& pixelXSize, double& pixelYSize );

    static void helmert( QVector<QgsPoint> mapCoords,
                         QVector<QgsPoint> pixelCoords,
                         QgsPoint& origin, double& pixelSize, double& rotation );

    static void affine( QVector<QgsPoint> mapCoords,
                        QVector<QgsPoint> pixelCoords );

    static void projective( QVector<QgsPoint> mapCoords,
                            QVector<QgsPoint> pixelCoords,
                            double H[9] );
};


#endif
