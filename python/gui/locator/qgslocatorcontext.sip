/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/locator/qgslocatorcontext.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsLocatorContext
{
%Docstring
 Encapsulates the properties relating to the context of a locator search.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslocatorcontext.h"
%End
  public:

    QgsLocatorContext();
%Docstring
 Constructor for QgsLocatorContext.
%End

    QgsRectangle targetExtent;
%Docstring
 Map extent to target in results. This can be used to prioritize searching
 for results close to the current map extent. The CRS for the extent
 is specified by targetExtentCrs.
.. seealso:: targetExtentCrs
%End

    QgsCoordinateReferenceSystem targetExtentCrs;
%Docstring
 Coordinate reference system for the map extent variable.
.. seealso:: targetExtent
%End

};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/locator/qgslocatorcontext.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
