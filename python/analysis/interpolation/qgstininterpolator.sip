/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/interpolation/qgstininterpolator.h                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsTINInterpolator: QgsInterpolator
{
%Docstring
  Interpolation in a triangular irregular network*
%End

%TypeHeaderCode
#include "qgstininterpolator.h"
%End
  public:
    enum TINInterpolation
    {
      Linear,
      CloughTocher
    };

    QgsTINInterpolator( const QList<QgsInterpolator::LayerData> &inputData, TINInterpolation interpolation = Linear, QgsFeedback *feedback = 0 );
%Docstring
 Constructor for QgsTINInterpolator.
 The ``feedback`` object specifies an optional QgsFeedback object for progress reports and cancelation support.
 Ownership of ``feedback`` is not transferred and callers must ensure that it exists for the lifetime of this object.
%End
    ~QgsTINInterpolator();

    virtual int interpolatePoint( double x, double y, double &result );

%Docstring
 Calculates interpolation value for map coordinates x, y
\param x x-coordinate (in map units)
\param y y-coordinate (in map units)
\param result out: interpolation result
:return: 0 in case of success*
 :rtype: int
%End

    static QgsFields triangulationFields();
%Docstring
 Returns the fields output by features when saving the triangulation.
 These fields should be used when creating
 a suitable feature sink for setTriangulationSink()
.. seealso:: setTriangulationSink()
.. versionadded:: 3.0
 :rtype: QgsFields
%End

    void setTriangulationSink( QgsFeatureSink *sink );
%Docstring
 Sets the optional ``sink`` for saving the triangulation features.

 The sink must be setup to accept LineString features, with fields matching
 those returned by triangulationFields().

.. seealso:: triangulationFields()
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
