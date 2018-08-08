/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/raster/qgsaspectfilter.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsAspectFilter: QgsDerivativeFilter
{
%Docstring
Calculates aspect values in a window of 3x3 cells based on first order derivatives in x- and y- directions. Direction is clockwise starting from north*
%End

%TypeHeaderCode
#include "qgsaspectfilter.h"
%End
  public:
    QgsAspectFilter( const QString &inputFile, const QString &outputFile, const QString &outputFormat );

     virtual float processNineCellWindow( float *x11, float *x21, float *x31,
                                 float *x12, float *x22, float *x32,
                                 float *x13, float *x23, float *x33 );
%Docstring
Calculates output value from nine input values. The input values and the output value can be equal to the
nodata value if not present or outside of the border. Must be implemented by subclasses*
%End


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/raster/qgsaspectfilter.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
