/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgssurface.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsSurface: QgsAbstractGeometry
{

%TypeHeaderCode
#include "qgssurface.h"
%End
  public:

    virtual QgsPolygon *surfaceToPolygon() const = 0 /Factory/;
%Docstring
 Get a polygon representation of this surface.
 Ownership is transferred to the caller.
 :rtype: QgsPolygon
%End

    virtual QgsRectangle boundingBox() const;
%Docstring
 Returns the minimal bounding box for the geometry
 :rtype: QgsRectangle
%End

  protected:

    virtual void clearCache() const;

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgssurface.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
