/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/interpolation/qgsinterpolator.h                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





struct QgsInterpolatorVertexData
{

  QgsInterpolatorVertexData( double x, double y, double z );
%Docstring
 Constructor for QgsInterpolatorVertexData with the specified
 ``x``, ``y``, and ``z`` coordinate.
%End

  QgsInterpolatorVertexData();
%Docstring
Constructor for QgsInterpolatorVertexData
%End

  double x;
%Docstring
X-coordinate
%End
  double y;
%Docstring
Y-coordinate
%End
  double z;
%Docstring
Z-coordinate
%End
};

class QgsInterpolator
{
%Docstring
 Interface class for interpolations. Interpolators take
 the vertices of a vector layer as base data. The z-Value
 can be an attribute or the z-coordinates in case of 3D types.
%End

%TypeHeaderCode
#include "qgsinterpolator.h"
%End
  public:

    enum SourceType
    {
      SourcePoints,
      SourceStructureLines,
      SourceBreakLines,
    };

    enum ValueSource
    {
      ValueAttribute,
      ValueZ,
      ValueM,
    };

    enum Result
    {
      Success,
      Canceled,
      InvalidSource,
      FeatureGeometryError,
    };

    struct LayerData
    {
      QgsFeatureSource *source;
%Docstring
Feature source
%End
      QgsInterpolator::ValueSource valueSource;
%Docstring
Source for feature values to interpolate
%End
      int interpolationAttribute;
%Docstring
Index of feature attribute to use for interpolation
%End
      QgsInterpolator::SourceType sourceType;
%Docstring
Source type
%End
    };

    QgsInterpolator( const QList<QgsInterpolator::LayerData> &layerData );

    virtual ~QgsInterpolator();

    virtual int interpolatePoint( double x, double y, double &result, QgsFeedback *feedback = 0 ) = 0;
%Docstring
 Calculates interpolation value for map coordinates x, y
 \param x x-coordinate (in map units)
 \param y y-coordinate (in map units)
 \param result out: interpolation result
 \param feedback optional feedback object for progress and cancelation support
 :return: 0 in case of success*
 :rtype: int
%End


  protected:

    Result cacheBaseData( QgsFeedback *feedback = 0 );
%Docstring
 Caches the vertex and value data from the provider. All the vertex data
 will be held in virtual memory.

 An optional ``feedback`` argument may be specified to allow cancelation and
 progress reports from the cache operation.

 :return: Success in case of success
 :rtype: Result
%End




};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/interpolation/qgsinterpolator.h                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
