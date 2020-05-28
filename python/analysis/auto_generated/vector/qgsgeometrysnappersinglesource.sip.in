/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/vector/qgsgeometrysnappersinglesource.h                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsGeometrySnapperSingleSource
{
%Docstring

Makes sure that any two vertices of the vector layer are at least at distance given by the threshold value.
The algorithm moves nearby vertices to one location and adds vertices to segments that are passing around other
vertices within the threshold. It does not remove any vertices. Also, it does not modify geometries unless
needed (it does not snap coordinates to a grid).

This algorithm comes handy when doing vector overlay operations such as intersection, union or difference
to prevent possible topological errors caused by numerical errors if coordinates are very close to each other.

After running the algorithm some previously valid geometries may become invalid and therefore it may be useful
to run Fix geometries algorithm afterwards.

.. note::

   Originally ported from GRASS implementation of Vect_snap_lines_list()

.. versionadded:: 3.4
%End

%TypeHeaderCode
#include "qgsgeometrysnappersinglesource.h"
%End
  public:

    static int run( const QgsFeatureSource &source, QgsFeatureSink &sink, double thresh, QgsFeedback *feedback );
%Docstring
Run the algorithm on given source and output results to the sink, using threshold value in the source's map units.
Returns number of modified geometries.
%End
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/vector/qgsgeometrysnappersinglesource.h                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
