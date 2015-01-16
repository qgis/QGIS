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
#include <QColor>

/** \ingroup core
    Cubic Raster Resampler
*/
class CORE_EXPORT QgsCubicRasterResampler: public QgsRasterResampler
{
  public:
    QgsCubicRasterResampler();
    ~QgsCubicRasterResampler();
    QgsRasterResampler * clone() const override;
    void resample( const QImage& srcImage, QImage& dstImage ) override;
    QString type() const override { return "cubic"; }

  private:
    static void xDerivativeMatrix( int nCols, int nRows, double* matrix, const int* colorMatrix );
    static void yDerivativeMatrix( int nCols, int nRows, double* matrix, const int* colorMatrix );

    void calculateControlPoints( int nCols, int nRows, int currentRow, int currentCol, int* redMatrix, int* greenMatrix, int* blueMatrix,
                                 int* alphaMatrix, double* xDerivativeMatrixRed, double* xDerivativeMatrixGreen, double* xDerivativeMatrixBlue,
                                 double* xDerivativeMatrixAlpha, double* yDerivativeMatrixRed, double* yDerivativeMatrixGreen, double* yDerivativeMatrixBlue,
                                 double* yDerivativeMatrixAlpha );

    /**Use cubic curve interpoation at the borders of the raster*/
    QRgb curveInterpolation( QRgb pt1, QRgb pt2, double t, double d1red, double d1green, double d1blue, double d1alpha, double d2red, double d2green,
                             double d2blue, double d2alpha );

    static double calcBernsteinPoly( int n, int i, double t );
    static int lower( int n, int i );
    static double power( double a, int b );//calculates a power b
    static int faculty( int n );

    //control points

    //red
    double cRed00; double cRed10; double cRed20; double cRed30; double cRed01; double cRed11; double cRed21; double cRed31;
    double cRed02; double cRed12; double cRed22; double cRed32; double cRed03; double cRed13; double cRed23; double cRed33;
    //green
    double cGreen00; double cGreen10; double cGreen20; double cGreen30; double cGreen01; double cGreen11; double cGreen21; double cGreen31;
    double cGreen02; double cGreen12; double cGreen22; double cGreen32; double cGreen03; double cGreen13; double cGreen23; double cGreen33;
    //blue
    double cBlue00; double cBlue10; double cBlue20; double cBlue30; double cBlue01; double cBlue11; double cBlue21; double cBlue31;
    double cBlue02; double cBlue12; double cBlue22; double cBlue32; double cBlue03; double cBlue13; double cBlue23; double cBlue33;
    //alpha
    double cAlpha00; double cAlpha10; double cAlpha20; double cAlpha30; double cAlpha01; double cAlpha11; double cAlpha21; double cAlpha31;
    double cAlpha02; double cAlpha12; double cAlpha22; double cAlpha32; double cAlpha03; double cAlpha13; double cAlpha23; double cAlpha33;
};

#endif // QGSCUBICRASTERRESAMPLER_H
