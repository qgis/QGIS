/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoututils.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsLayoutUtils
{
%Docstring
 Utilities for layouts.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoututils.h"
%End
  public:

    static double normalizedAngle( const double angle, const bool allowNegative = false );
%Docstring
 Ensures that an ``angle`` (in degrees) is in the range 0 <= angle < 360.
 If ``allowNegative`` is true then angles between (-360, 360) are allowed. If false,
 angles are converted to positive angles in the range [0, 360).
 :rtype: float
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoututils.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
