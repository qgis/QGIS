class QgsIDWInterpolator: QgsInterpolator
{
%TypeHeaderCode
#include <qgsidwinterpolator.h>
%End
  public:
    QgsIDWInterpolator( const QList<QgsInterpolator::LayerData>& layerData );
    ~QgsIDWInterpolator();

    /** Calculates interpolation value for map coordinates x, y
       @param x x-coordinate (in map units)
       @param y y-coordinate (in map units)
       @param result out: interpolation result
       @return 0 in case of success*/
    int interpolatePoint( double x, double y, double& result );

    void setDistanceCoefficient( double p );
};
