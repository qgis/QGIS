/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/raster/qgsruggednessfilter.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsRuggednessFilter: QgsNineCellFilter
{
%Docstring
Calculates the ruggedness index based on a 3x3 moving window.
Algorithm from Riley et al. 1999: A terrain ruggedness index that quantifies topographic heterogeneity*
%End

%TypeHeaderCode
#include "qgsruggednessfilter.h"
%End
  public:
    QgsRuggednessFilter( const QString &inputFile, const QString &outputFile, const QString &outputFormat );

  protected:

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
 * src/analysis/raster/qgsruggednessfilter.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
