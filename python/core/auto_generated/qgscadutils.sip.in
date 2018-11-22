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
      bool relative;
      double value;
    };

    struct AlignMapPointContext
    {
      QgsSnappingUtils *snappingUtils;
      double mapUnitsPerPixel;

      QgsCadUtils::AlignMapPointConstraint xConstraint;
      QgsCadUtils::AlignMapPointConstraint yConstraint;
      QgsCadUtils::AlignMapPointConstraint distanceConstraint;
      QgsCadUtils::AlignMapPointConstraint angleConstraint;
      QgsCadUtils::AlignMapPointConstraint commonAngleConstraint;

      QList<QgsPointXY> cadPointList;

    };

    struct AlignMapPointOutput
    {
      bool valid;

      QgsPointXY finalMapPoint;

      QgsPointLocator::Match edgeMatch;

      double softLockCommonAngle;
    };

    static QgsCadUtils::AlignMapPointOutput alignMapPoint( const QgsPointXY &originalMapPoint, const QgsCadUtils::AlignMapPointContext &ctx );
%Docstring
Applies X/Y/angle/distance constraints from the given context to a map point.
Returns a structure containing aligned map point, whether the constraints are valid and
some extra information.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgscadutils.h                                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
