/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/interpolation/qgsgridfilewriter.h                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsGridFileWriter
{

%TypeHeaderCode
#include "qgsgridfilewriter.h"
%End
  public:

    QgsGridFileWriter( QgsInterpolator *interpolator, const QString &outputPath, const QgsRectangle &extent, int nCols, int nRows );
%Docstring
 Constructor for QgsGridFileWriter, for the specified ``interpolator``.

 The ``outputPath`` argument is used to set the output file path.

 The ``extent`` and ``nCols``, ``nRows`` arguments dictate the extent and size of the output raster.
%End

    int writeFile( QgsFeedback *feedback = 0 );
%Docstring
 Writes the grid file.

 An optional ``feedback`` object can be set for progress reports and cancelation support

 :return: 0 in case of success
 :rtype: int
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/interpolation/qgsgridfilewriter.h                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
