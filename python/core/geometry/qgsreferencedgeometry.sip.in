/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsreferencedgeometry.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsReferencedGeometryBase
{
%Docstring
A base class for geometry primitives which are stored with an associated reference system.

QgsReferencedGeometryBase classes represent some form of geometry primitive
(such as rectangles) which have an optional coordinate reference system
associated with them.

.. versionadded:: 3.0

.. seealso:: :py:class:`QgsReferencedRectangle`
%End

%TypeHeaderCode
#include "qgsreferencedgeometry.h"
%End
  public:

    QgsReferencedGeometryBase( const QgsCoordinateReferenceSystem &crs = QgsCoordinateReferenceSystem() );
%Docstring
Constructor for QgsReferencedGeometryBase, with the specified ``crs``.
%End

    QgsCoordinateReferenceSystem crs() const;
%Docstring
Returns the associated coordinate reference system, or an invalid CRS if
no reference system is set.

.. seealso:: :py:func:`setCrs`
%End

    void setCrs( const QgsCoordinateReferenceSystem &crs );
%Docstring
Sets the associated ``crs``. Set to an invalid CRS if
no reference system is required.

.. seealso:: :py:func:`crs`
%End

};

class QgsReferencedRectangle : QgsRectangle, QgsReferencedGeometryBase
{
%Docstring
A QgsRectangle with associated coordinate reference system.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsreferencedgeometry.h"
%End
  public:

    QgsReferencedRectangle( const QgsRectangle &rectangle, const QgsCoordinateReferenceSystem &crs );
%Docstring
Constructor for QgsReferencedRectangle, with the specified initial ``rectangle``
and ``crs``.
%End

    QgsReferencedRectangle();
%Docstring
Constructor for QgsReferencedRectangle.
%End

    operator QVariant() const;

};


class QgsReferencedPointXY : QgsPointXY, QgsReferencedGeometryBase
{
%Docstring
A QgsPointXY with associated coordinate reference system.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsreferencedgeometry.h"
%End
  public:

    QgsReferencedPointXY( const QgsPointXY &point, const QgsCoordinateReferenceSystem &crs );
%Docstring
Constructor for QgsReferencedPointXY, with the specified initial ``point``
and ``crs``.
%End

    QgsReferencedPointXY();
%Docstring
Constructor for QgsReferencedPointXY.
%End

    operator QVariant() const;

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsreferencedgeometry.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
