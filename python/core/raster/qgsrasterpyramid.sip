/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrasterpyramid.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsRasterPyramid
{
%Docstring
 This struct is used to store pyramid info for the raster layer.
%End

%TypeHeaderCode
#include "qgsrasterpyramid.h"
%End
  public:
    int level;
%Docstring
 The pyramid level as implemented in gdal (level 2 is half original raster size etc)
%End
    int xDim;
%Docstring
 XDimension for this pyramid layer
%End
    int yDim;
%Docstring
 YDimension for this pyramid layer
%End
    bool exists;
%Docstring
 Whether the pyramid layer has been built yet
%End
    bool build;
%Docstring
 Whether the pyramid should be built
%End

    QgsRasterPyramid();

};
/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrasterpyramid.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
