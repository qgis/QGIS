class QgsSurface: public QgsAbstractGeometry
{
%TypeHeaderCode
#include <qgssurfacev2.h>
%End

  public:

    virtual QgsPolygonV2* surfaceToPolygon() const = 0;

    virtual QgsRectangle boundingBox() const;

  protected:

    virtual void clearCache() const;

};
