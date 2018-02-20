/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/interpolation/qgsidwinterpolator.h                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsIDWInterpolator: QgsInterpolator
{
%Docstring
Inverse distance weight interpolator.
%End

%TypeHeaderCode
#include "qgsidwinterpolator.h"
%End
  public:

    QgsIDWInterpolator( const QList<QgsInterpolator::LayerData> &layerData );
%Docstring
Constructor for QgsIDWInterpolator, with the specified ``layerData`` sources.
%End

    virtual int interpolatePoint( double x, double y, double &result /Out/, QgsFeedback *feedback = 0 );


    void setDistanceCoefficient( double coefficient );
%Docstring
Sets the distance ``coefficient``, the parameter that sets how the values are
weighted with distance. Smaller values mean sharper peaks at the data points.

Point values are weighted by 1 / ( distance ^ coefficient ).

.. seealso:: :py:func:`distanceCoefficient`

.. versionadded:: 3.0
%End

    double distanceCoefficient() const;
%Docstring
Returns the distance coefficient, the parameter that sets how the values are
weighted with distance. Smaller values mean sharper peaks at the data points.
The default is a coefficient of 2.

Point values are weighted by 1 / ( distance ^ coefficient ).

.. seealso:: :py:func:`setDistanceCoefficient`

.. versionadded:: 3.0
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/interpolation/qgsidwinterpolator.h                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
