class QgsSurfaceV2: public QgsAbstractGeometryV2
{
%TypeHeaderCode
#include <qgssurfacev2.h>
%End

  public:
    virtual QgsPointV2 centroid() const = 0;
    virtual QgsPointV2 pointOnSurface() const = 0;
    virtual QgsPolygonV2* surfaceToPolygon() const = 0;
};
