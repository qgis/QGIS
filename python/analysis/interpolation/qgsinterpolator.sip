/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/interpolation/qgsinterpolator.h                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





struct vertexData
{
  double x;
  double y;
  double z;
};

class QgsInterpolator
{
%Docstring
 Interface class for interpolations. Interpolators take
the vertices of a vector layer as base data. The z-Value
can be an attribute or the z-coordinates in case of 25D types*
%End

%TypeHeaderCode
#include "qgsinterpolator.h"
%End
  public:
    enum InputType
    {
      POINTS,
      STRUCTURE_LINES,
      BREAK_LINES
    };

    struct LayerData
    {
      QgsVectorLayer *vectorLayer;
      bool zCoordInterpolation;
      int interpolationAttribute;
      QgsInterpolator::InputType mInputType;
    };

    QgsInterpolator( const QList<QgsInterpolator::LayerData> &layerData );

    virtual ~QgsInterpolator();

    virtual int interpolatePoint( double x, double y, double &result ) = 0;
%Docstring
 Calculates interpolation value for map coordinates x, y
\param x x-coordinate (in map units)
\param y y-coordinate (in map units)
\param result out: interpolation result
:return: 0 in case of success*
 :rtype: int
%End


  protected:

    int cacheBaseData();
%Docstring
 Caches the vertex and value data from the provider. All the vertex data
will be held in virtual memory
:return: 0 in case of success*
 :rtype: int
%End




};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/interpolation/qgsinterpolator.h                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
