/***************************************************************************
                         qgscubicrasterresampler.h
                         ----------------------------
    begin                : December 2011
    copyright            : (C) 2011 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCUBICRASTERRESAMPLER_H
#define QGSCUBICRASTERRESAMPLER_H

#include "qgsrasterresampler.h"
#include "qgis_sip.h"
#include <QColor>

#include "qgis_core.h"

/**
 * \ingroup core
    Cubic Raster Resampler
*/
class CORE_EXPORT QgsCubicRasterResampler: public QgsRasterResampler
{
  public:

    /**
     * Constructor for QgsCubicRasterResampler.
     */
    QgsCubicRasterResampler() = default;
    QgsCubicRasterResampler *clone() const override SIP_FACTORY;
    void resample( const QImage &srcImage, QImage &dstImage ) override;
    QString type() const override { return QStringLiteral( "cubic" ); }

  private:
    static void xDerivativeMatrix( int nCols, int nRows, double *matrix, const int *colorMatrix );
    static void yDerivativeMatrix( int nCols, int nRows, double *matrix, const int *colorMatrix );

    void calculateControlPoints( int nCols, int nRows, int currentRow, int currentCol, int *redMatrix, int *greenMatrix, int *blueMatrix,
                                 int *alphaMatrix, double *xDerivativeMatrixRed, double *xDerivativeMatrixGreen, double *xDerivativeMatrixBlue,
                                 double *xDerivativeMatrixAlpha, double *yDerivativeMatrixRed, double *yDerivativeMatrixGreen, double *yDerivativeMatrixBlue,
                                 double *yDerivativeMatrixAlpha );

    //! Use cubic curve interpoation at the borders of the raster
    QRgb curveInterpolation( QRgb pt1, QRgb pt2, double t, double d1red, double d1green, double d1blue, double d1alpha, double d2red, double d2green,
                             double d2blue, double d2alpha );

    static inline double calcBernsteinPolyN3( int i, double t );
    static inline int lowerN3( int i );

    //creates a QRgb by applying bounds checks
    static inline QRgb createPremultipliedColor( int r, int g, int b, int a );

    //control points

    //red
    double cRed00 = 0.0;
    double cRed10 = 0.0;
    double cRed20 = 0.0;
    double cRed30 = 0.0;
    double cRed01 = 0.0;
    double cRed11 = 0.0;
    double cRed21 = 0.0;
    double cRed31 = 0.0;
    double cRed02 = 0.0;
    double cRed12 = 0.0;
    double cRed22 = 0.0;
    double cRed32 = 0.0;
    double cRed03 = 0.0;
    double cRed13 = 0.0;
    double cRed23 = 0.0;
    double cRed33 = 0.0;
    //green
    double cGreen00 = 0.0;
    double cGreen10 = 0.0;
    double cGreen20 = 0.0;
    double cGreen30 = 0.0;
    double cGreen01 = 0.0;
    double cGreen11 = 0.0;
    double cGreen21 = 0.0;
    double cGreen31 = 0.0;
    double cGreen02 = 0.0;
    double cGreen12 = 0.0;
    double cGreen22 = 0.0;
    double cGreen32 = 0.0;
    double cGreen03 = 0.0;
    double cGreen13 = 0.0;
    double cGreen23 = 0.0;
    double cGreen33 = 0.0;
    //blue
    double cBlue00 = 0.0;
    double cBlue10 = 0.0;
    double cBlue20 = 0.0;
    double cBlue30 = 0.0;
    double cBlue01 = 0.0;
    double cBlue11 = 0.0;
    double cBlue21 = 0.0;
    double cBlue31 = 0.0;
    double cBlue02 = 0.0;
    double cBlue12 = 0.0;
    double cBlue22 = 0.0;
    double cBlue32 = 0.0;
    double cBlue03 = 0.0;
    double cBlue13 = 0.0;
    double cBlue23 = 0.0;
    double cBlue33 = 0.0;
    //alpha
    double cAlpha00 = 0.0;
    double cAlpha10 = 0.0;
    double cAlpha20 = 0.0;
    double cAlpha30 = 0.0;
    double cAlpha01 = 0.0;
    double cAlpha11 = 0.0;
    double cAlpha21 = 0.0;
    double cAlpha31 = 0.0;
    double cAlpha02 = 0.0;
    double cAlpha12 = 0.0;
    double cAlpha22 = 0.0;
    double cAlpha32 = 0.0;
    double cAlpha03 = 0.0;
    double cAlpha13 = 0.0;
    double cAlpha23 = 0.0;
    double cAlpha33 = 0.0;


};

#endif // QGSCUBICRASTERRESAMPLER_H
