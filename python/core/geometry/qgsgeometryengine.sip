/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsgeometryengine.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsGeometryEngine
{
%Docstring
 Contains geometry relation and modification algorithms.

.. versionadded:: 2.10
%End

%TypeHeaderCode
#include "qgsgeometryengine.h"
%End
  public:

    enum EngineOperationResult
    {
      Success,
      NothingHappened,
      MethodNotImplemented,
      EngineError,
      NodedGeometryError,
      InvalidBaseGeometry,
      InvalidInput,
      SplitCannotSplitPoint,
    };

    virtual ~QgsGeometryEngine();

    virtual void geometryChanged() = 0;
%Docstring
Should be called whenever the geometry associated with the engine
has been modified and the engine must be updated to suit.
%End

    virtual void prepareGeometry() = 0;
%Docstring
Prepares the geometry, so that subsequent calls to spatial relation methods
are much faster.

This should be called for any geometry which is used for multiple relation
tests against other geometries.

.. seealso:: :py:func:`geometryChanged`
%End

    virtual QgsAbstractGeometry *intersection( const QgsAbstractGeometry *geom, QString *errorMsg = 0 ) const = 0 /Factory/;
%Docstring
Calculate the intersection of this and ``geom``.

.. versionadded:: 3.0
%End

    virtual QgsAbstractGeometry *difference( const QgsAbstractGeometry *geom, QString *errorMsg = 0 ) const = 0 /Factory/;
%Docstring
Calculate the difference of this and ``geom``.

.. versionadded:: 3.0
%End

    virtual QgsAbstractGeometry *combine( const QgsAbstractGeometry *geom, QString *errorMsg = 0 ) const = 0 /Factory/;
%Docstring
Calculate the combination of this and ``geom``.

.. versionadded:: 3.0
%End

    virtual QgsAbstractGeometry *combine( const QVector<QgsAbstractGeometry *> &geomList, QString *errorMsg ) const = 0 /Factory/;
%Docstring
Calculate the combination of this and ``geometries``.

.. versionadded:: 3.0
%End

    virtual QgsAbstractGeometry *combine( const QVector< QgsGeometry > &geometries, QString *errorMsg = 0 ) const = 0 /Factory/;
%Docstring
Calculate the combination of this and ``geometries``.

.. versionadded:: 3.0
%End

    virtual QgsAbstractGeometry *symDifference( const QgsAbstractGeometry *geom, QString *errorMsg = 0 ) const = 0 /Factory/;
%Docstring
Calculate the symmetric difference of this and ``geom``.

.. versionadded:: 3.0
%End
    virtual QgsAbstractGeometry *buffer( double distance, int segments, QString *errorMsg = 0 ) const = 0 /Factory/;
    virtual QgsAbstractGeometry *buffer( double distance, int segments, int endCapStyle, int joinStyle, double miterLimit, QString *errorMsg = 0 ) const = 0 /Factory/;
    virtual QgsAbstractGeometry *simplify( double tolerance, QString *errorMsg = 0 ) const = 0 /Factory/;
    virtual QgsAbstractGeometry *interpolate( double distance, QString *errorMsg = 0 ) const = 0 /Factory/;
    virtual QgsAbstractGeometry *envelope( QString *errorMsg = 0 ) const = 0 /Factory/;

    virtual QgsPoint *centroid( QString *errorMsg = 0 ) const = 0 /Factory/;
%Docstring
Calculates the centroid of this.
May return a `None`.

.. versionadded:: 3.0
%End

    virtual QgsPoint *pointOnSurface( QString *errorMsg = 0 ) const = 0 /Factory/;
%Docstring
Calculate a point that is guaranteed to be on the surface of this.
May return a `None`.

.. versionadded:: 3.0
%End

    virtual QgsAbstractGeometry *convexHull( QString *errorMsg = 0 ) const = 0 /Factory/;
%Docstring
Calculate the convex hull of this.
%End

    virtual double distance( const QgsAbstractGeometry *geom, QString *errorMsg = 0 ) const = 0;
%Docstring
Calculates the distance between this and ``geom``.

.. versionadded:: 3.0
%End

    virtual bool intersects( const QgsAbstractGeometry *geom, QString *errorMsg = 0 ) const = 0;
%Docstring
Checks if ``geom`` intersects this.

.. versionadded:: 3.0
%End

    virtual bool touches( const QgsAbstractGeometry *geom, QString *errorMsg = 0 ) const = 0;
%Docstring
Checks if ``geom`` touches this.

.. versionadded:: 3.0
%End

    virtual bool crosses( const QgsAbstractGeometry *geom, QString *errorMsg = 0 ) const = 0;
%Docstring
Checks if ``geom`` crosses this.

.. versionadded:: 3.0
%End

    virtual bool within( const QgsAbstractGeometry *geom, QString *errorMsg = 0 ) const = 0;
%Docstring
Checks if ``geom`` is within this.

.. versionadded:: 3.0
%End

    virtual bool overlaps( const QgsAbstractGeometry *geom, QString *errorMsg = 0 ) const = 0;
%Docstring
Checks if ``geom`` overlaps this.

.. versionadded:: 3.0
%End

    virtual bool contains( const QgsAbstractGeometry *geom, QString *errorMsg = 0 ) const = 0;
%Docstring
Checks if ``geom`` contains this.

.. versionadded:: 3.0
%End

    virtual bool disjoint( const QgsAbstractGeometry *geom, QString *errorMsg = 0 ) const = 0;
%Docstring
Checks if ``geom`` is disjoint from this.

.. versionadded:: 3.0
%End

    virtual QString relate( const QgsAbstractGeometry *geom, QString *errorMsg = 0 ) const = 0;
%Docstring
Returns the Dimensional Extended 9 Intersection Model (DE-9IM) representation of the
relationship between the geometries.

:param geom: geometry to relate to
:param errorMsg: destination storage for any error message

:return: DE-9IM string for relationship, or an empty string if an error occurred

.. versionadded:: 2.12
%End

    virtual bool relatePattern( const QgsAbstractGeometry *geom, const QString &pattern, QString *errorMsg = 0 ) const = 0;
%Docstring
Tests whether two geometries are related by a specified Dimensional Extended 9 Intersection Model (DE-9IM)
pattern.

:param geom: geometry to relate to
:param pattern: DE-9IM pattern for match
:param errorMsg: destination storage for any error message

:return: true if geometry relationship matches with pattern

.. versionadded:: 2.14
%End

    virtual double area( QString *errorMsg = 0 ) const = 0;
    virtual double length( QString *errorMsg = 0 ) const = 0;
    virtual bool isValid( QString *errorMsg = 0 ) const = 0;

    virtual bool isEqual( const QgsAbstractGeometry *geom, QString *errorMsg = 0 ) const = 0;
%Docstring
Checks if this is equal to ``geom``.
If both are Null geometries, `false` is returned.

.. versionadded:: 3.0
%End
    virtual bool isEmpty( QString *errorMsg ) const = 0;

    virtual bool isSimple( QString *errorMsg = 0 ) const = 0;
%Docstring
Determines whether the geometry is simple (according to OGC definition).

.. versionadded:: 3.0
%End

    virtual QgsGeometryEngine::EngineOperationResult splitGeometry( const QgsLineString &splitLine,
        QVector<QgsGeometry > &newGeometries /Out/,
        bool topological,
        QgsPointSequence &topologyTestPoints, QString *errorMsg = 0 ) const;
%Docstring
Splits this geometry according to a given line.

:param splitLine: the line that splits the geometry
\param[out] newGeometries list of new geometries that have been created with the split
:param topological: true if topological editing is enabled
\param[out] topologyTestPoints points that need to be tested for topological completeness in the dataset
\param[out] errorMsg error messages emitted, if any

:return: 0 in case of success, 1 if geometry has not been split, error else
%End

    virtual QgsAbstractGeometry *offsetCurve( double distance, int segments, int joinStyle, double miterLimit, QString *errorMsg = 0 ) const = 0 /Factory/;

  protected:

    QgsGeometryEngine( const QgsAbstractGeometry *geometry );
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsgeometryengine.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
