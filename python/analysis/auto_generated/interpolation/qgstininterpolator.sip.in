/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/interpolation/qgstininterpolator.h                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsTinInterpolator: QgsInterpolator
{
%Docstring
Interpolation in a triangular irregular network

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgstininterpolator.h"
%End
  public:

    enum TinInterpolation
    {
      Linear,
      CloughTocher,
    };

    QgsTinInterpolator( const QList<QgsInterpolator::LayerData> &inputData, TinInterpolation interpolation = Linear, QgsFeedback *feedback = 0 );
%Docstring
Constructor for QgsTinInterpolator.
The ``feedback`` object specifies an optional QgsFeedback object for progress reports and cancelation support.
Ownership of ``feedback`` is not transferred and callers must ensure that it exists for the lifetime of this object.
%End
    ~QgsTinInterpolator();

    virtual int interpolatePoint( double x, double y, double &result /Out/, QgsFeedback *feedback );


    static QgsFields triangulationFields();
%Docstring
Returns the fields output by features when saving the triangulation.
These fields should be used when creating
a suitable feature sink for setTriangulationSink()

.. seealso:: :py:func:`setTriangulationSink`

.. versionadded:: 3.0
%End

    void setTriangulationSink( QgsFeatureSink *sink );
%Docstring
Sets the optional ``sink`` for saving the triangulation features.

The sink must be setup to accept LineString features, with fields matching
those returned by triangulationFields().

.. seealso:: :py:func:`triangulationFields`

.. versionadded:: 3.0
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/interpolation/qgstininterpolator.h                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
