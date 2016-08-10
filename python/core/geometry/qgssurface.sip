class QgsSurface: public QgsAbstractGeometry
{
%TypeHeaderCode
#include <qgssurface.h>
%End

  public:

    virtual QgsPolygonV2* surfaceToPolygon() const = 0;

    virtual QgsRectangle boundingBox() const;

  protected:

    virtual void clearCache() const;

};
