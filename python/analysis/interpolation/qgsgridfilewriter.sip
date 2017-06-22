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
    QgsGridFileWriter( QgsInterpolator *i, const QString &outputPath, const QgsRectangle &extent, int nCols, int nRows, double cellSizeX, double cellSizeY );


    int writeFile( bool showProgressDialog = false );
%Docstring
 Writes the grid file.
\param showProgressDialog shows a dialog with the possibility to cancel
:return: 0 in case of success*
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
