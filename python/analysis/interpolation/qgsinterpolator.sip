/**Interface class for interpolations. Interpolators take
the vertices of a vector layer as base data. The z-Value
can be an attribute or the z-coordinates in case of 25D types*/
class QgsInterpolator
{
%TypeHeaderCode
#include "qgsinterpolator.h"
%End

  public:
    /**Describes the type of input data*/
    enum InputType
    {
      POINTS,
      STRUCTURE_LINES,
      BREAK_LINES
    };

    /**A layer together with the information about interpolation attribute / z-coordinate interpolation and the type (point, structure line, breakline)*/
    struct LayerData
    {
      QgsVectorLayer* vectorLayer;
      bool zCoordInterpolation;
      int interpolationAttribute;
      QgsInterpolator::InputType mInputType;
    };

    QgsInterpolator( const QList<QgsInterpolator::LayerData>& layerData );

    virtual ~QgsInterpolator();

    /**Calculates interpolation value for map coordinates x, y
       @param x x-coordinate (in map units)
       @param y y-coordinate (in map units)
       @param result out: interpolation result
       @return 0 in case of success*/
    virtual int interpolatePoint( double x, double y, double& result ) = 0;

    // @note not available in python bindings
    // const QList<LayerData>& layerData() const;

  protected:
    /**Caches the vertex and value data from the provider. All the vertex data
     will be held in virtual memory
    @return 0 in case of success*/
    int cacheBaseData();
};
