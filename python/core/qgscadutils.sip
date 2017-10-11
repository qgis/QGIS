/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgscadutils.h                                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsCadUtils
{
%Docstring
 The QgsCadUtils class provides routines for CAD editing.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgscadutils.h"
%End
  public:

    struct AlignMapPointConstraint
    {
      AlignMapPointConstraint( bool locked = false, bool relative = false, double value = 0 );

      bool locked;
%Docstring
Whether the constraint is active, i.e. should be considered
%End
      bool relative;
%Docstring
Whether the value is relative to previous value
%End
      double value;
%Docstring
Numeric value of the constraint (coordinate/distance in map units or angle in degrees)
%End
    };

    struct AlignMapPointContext
    {
      QgsSnappingUtils *snappingUtils;
%Docstring
Snapping utils that will be used to snap point to map. Must not be null
%End
      double mapUnitsPerPixel;
%Docstring
Map units/pixel ratio from map canvas. Needed for
%End

      QgsCadUtils::AlignMapPointConstraint xConstraint;
%Docstring
Constraint for X coordinate
%End
      QgsCadUtils::AlignMapPointConstraint yConstraint;
%Docstring
Constraint for Y coordinate
%End
      QgsCadUtils::AlignMapPointConstraint distanceConstraint;
%Docstring
Constraint for distance
%End
      QgsCadUtils::AlignMapPointConstraint angleConstraint;
%Docstring
Constraint for angle
%End
      QgsCadUtils::AlignMapPointConstraint commonAngleConstraint;
%Docstring
Constraint for soft lock to a common angle
%End

      QList<QgsPointXY> cadPointList;
%Docstring
 List of recent CAD points in map coordinates. These are used to turn relative constraints to absolute.
 First point is the most recent point. Currently using only "previous" point (index 1) and "penultimate"
 point (index 2) for alignment purposes.
%End

    };

    struct AlignMapPointOutput
    {
      bool valid;
%Docstring
Whether the combination of constraints is actually valid
%End

      QgsPointXY finalMapPoint;
%Docstring
map point aligned according to the constraints
%End

      QgsPointLocator::Match edgeMatch;
%Docstring
Snapped segment - only valid if actually used for something
%End

      int softLockCommonAngle;
%Docstring
Angle (in degrees) to which we have soft-locked ourselves (if not set it is -1)
%End
    };

    static QgsCadUtils::AlignMapPointOutput alignMapPoint( const QgsPointXY &originalMapPoint, const QgsCadUtils::AlignMapPointContext &ctx );
%Docstring
 Applies X/Y/angle/distance constraints from the given context to a map point.
 Returns a structure containing aligned map point, whether the constraints are valid and
 some extra information.
 :rtype: QgsCadUtils.AlignMapPointOutput
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgscadutils.h                                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
