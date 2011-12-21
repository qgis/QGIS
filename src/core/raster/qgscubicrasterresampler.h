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

class QgsCubicRasterResampler: public QgsRasterResampler
{
  public:
    QgsCubicRasterResampler();
    ~QgsCubicRasterResampler();
    void resample( const QImage& srcImage, QImage& dstImage );

  private:
    static void xDerivativeMatrix( int nCols, int nRows, double* matrix, const int* colorMatrix );
    static void yDerivativeMatrix( int nCols, int nRows, double* matrix, const int* colorMatrix );

    void calculateControlPoints( int nCols, int nRows, int currentRow, int currentCol, int* redMatrix, int* greenMatrix, int* blueMatrix,
                                 double* xDerivativeMatrixRed, double* xDerivativeMatrixGreen, double* xDerivativeMatrixBlue,
                                 double* yDerivativeMatrixRed, double* yDerivativeMatrixGreen, double* yDerivativeMatrixBlue );

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
};

#endif // QGSCUBICRASTERRESAMPLER_H
