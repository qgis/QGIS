/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsreferencedgeometry.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




template<T>
class QgsReferencedGeometryPrimitive
{
%Docstring
 A template based class for storing geometry primitives with an associated reference system.

 QgsReferencedGeometryPrimitive classes represent some form of geometry primitive
 (such as rectangles) which have an optional coordinate reference system
 associated with them.

.. versionadded:: 3.0
.. seealso:: QgsReferencedRectangle
.. note::

   Not available in Python bindings (although SIP file is present for specific implementations).
%End

%TypeHeaderCode
#include "qgsreferencedgeometry.h"
%End
  public:

    QgsReferencedGeometryPrimitive( T primitive, const QgsCoordinateReferenceSystem &crs = QgsCoordinateReferenceSystem() );
%Docstring
 Constructor for QgsReferencedGeometryPrimitive, for the specified ``primitive`` and ``crs``.
%End

    T primitive() const;
%Docstring
 Returns the geometry primitive.
 :rtype: T
%End

    T &primitive();
%Docstring
 Returns the geometry primitive.
 :rtype: T
%End

    QgsCoordinateReferenceSystem crs() const;
%Docstring
 Returns the associated coordinate reference system, or an invalid CRS if
 no reference system is set.
.. seealso:: setCrs()
 :rtype: QgsCoordinateReferenceSystem
%End

    void setCrs( const QgsCoordinateReferenceSystem &crs );
%Docstring
 Sets the associated ``crs``. Set to an invalid CRS if
 no reference system is required.
.. seealso:: crs()
%End

};


typedef QgsReferencedGeometryPrimitive<QgsRectangle> QgsReferencedGeometryPrimitiveQgsRectangleBase;

class QgsReferencedRectangle : QgsReferencedGeometryPrimitiveQgsRectangleBase
{
%Docstring
 A QgsRectangle with associated coordinate reference system.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsreferencedgeometry.h"
typedef QgsReferencedGeometryPrimitive<QgsRectangle> QgsReferencedGeometryPrimitiveQgsRectangleBase;
%End
  public:

    QgsReferencedRectangle();
%Docstring
 Construct a default optional expression.
 It will be disabled and with an empty expression.
%End

    QgsRectangle &rect();
%Docstring
 :rtype: QgsRectangle
%End


};





/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsreferencedgeometry.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
