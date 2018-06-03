/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsmapunitscale.h                                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsMapUnitScale
{
%Docstring
Struct for storing maximum and minimum scales for measurements in map units

For measurements in map units, a minimum and a maximum scale can be defined.
Outside this range, the measurements aren't scaled anymore proportionally to
the map scale.
%End

%TypeHeaderCode
#include "qgsmapunitscale.h"
%End
  public:

    explicit QgsMapUnitScale( double minScale = 0.0, double maxScale = 0.0 );
%Docstring
Constructor for QgsMapUnitScale

:param minScale: minimum allowed scale, or 0.0 if no minimum scale set
:param maxScale: maximum allowed scale, or 0.0 if no maximum scale set
                 The scale values indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
%End

    double minScale;

    double maxScale;

    bool minSizeMMEnabled;
    double minSizeMM;
    bool maxSizeMMEnabled;
    double maxSizeMM;

    double computeMapUnitsPerPixel( const QgsRenderContext &c ) const;
%Docstring
Computes a map units per pixel scaling factor, respecting the minimum and maximum scales
set for the object.

:param c: render context

:return: map units per pixel, limited between minimum and maximum scales
%End

    bool operator==( const QgsMapUnitScale &other ) const;

    bool operator!=( const QgsMapUnitScale &other ) const;
};





/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsmapunitscale.h                                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
