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

#include "qgis_analysis.h"
#include "qgspointxy.h"

#define SIP_NO_FILE

/**
 * \ingroup analysis
 * \brief Utilities for calculation of least squares based transformations.
 *
 * \note Not available in Python bindings.
 * \since QGIS 3.20
*/
class ANALYSIS_EXPORT QgsLeastSquares
{
  public:
    /**
     * Transforms the point at \a origin in-place, using a linear transformation calculated from the list of source and destination Ground Control Points (GCPs).
     */
    static void linear( const QVector<QgsPointXY> &sourceCoordinates, const QVector<QgsPointXY> &destinationCoordinates, QgsPointXY &origin, double &pixelXSize, double &pixelYSize );

    /**
     * Transforms the point at \a origin in-place, using a helmert transformation calculated from the list of source and destination Ground Control Points (GCPs).
     * \throws QgsNotSupportedException on QGIS built without GSL.
     */
    static void helmert( const QVector<QgsPointXY> &sourceCoordinates, const QVector<QgsPointXY> &destinationCoordinates, QgsPointXY &origin, double &pixelSize, double &rotation );

#if 0
    static void affine( QVector<QgsPointXY> mapCoords,
                        QVector<QgsPointXY> pixelCoords );

#endif

    /**
     * Calculates projective parameters from the list of source and destination Ground Control Points (GCPs).
     * \throws QgsNotSupportedException on QGIS built without GSL.
     */
    static void projective( const QVector<QgsPointXY> &sourceCoordinates, const QVector<QgsPointXY> &destinationCoordinates, double H[9] );
};

#endif // QGSLEASTSQUARES_H
