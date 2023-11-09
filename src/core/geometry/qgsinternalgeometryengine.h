/***************************************************************************
  qgsinternalgeometryengine.h - QgsInternalGeometryEngine

 ---------------------
 begin                : 13.1.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSINTERNALGEOMETRYENGINE_H
#define QGSINTERNALGEOMETRYENGINE_H

#define SIP_NO_FILE

#include <functional>

#include "qgspointxy.h"

class QgsGeometry;
class QgsAbstractGeometry;
class QgsLineString;
class QgsLineSegment2D;
class QgsFeedback;

/**
 * \ingroup core
 * \brief This class offers geometry processing methods.
 *
 * The methods are available via QgsGeometry::[geometryfunction]
 * and therefore this does not need to be accessed directly.
 *
 * \note not available in Python bindings
 */

class QgsInternalGeometryEngine
{
  public:

    /**
     * The caller is responsible that the geometry is available and unchanged
     * for the whole lifetime of this object.
     * \param geometry
     */
    explicit QgsInternalGeometryEngine( const QgsGeometry &geometry );

    /**
     * Returns an error string referring to the last error encountered.
     *
     * \since QGIS 3.16
     */
    QString lastError() const;

    /**
     * Returns TRUE if the geometry is a polygon that is almost an axis-parallel rectangle.
     *
     * The \a maximumDeviation argument specifes the maximum angle (in degrees) that the polygon edges
     * are allowed to deviate from axis parallel lines.
     *
     * By default the check will permit polygons with more than 4 edges, so long as the overall shape of
     * the polygon is an axis-parallel rectangle (i.e. it is tolerant to rectangles with additional vertices
     * added along the rectangle sides). If \a simpleRectanglesOnly is set to TRUE then the method will
     * only return TRUE if the geometry is a simple rectangle consisting of 4 edges.
     *
     * \since QGIS 3.20
     */
    bool isAxisParallelRectangle( double maximumDeviation, bool simpleRectanglesOnly = false ) const;

    /**
     * Will extrude a line or (segmentized) curve by a given offset and return a polygon
     * representation of it.
     *
     * \param x offset in x direction
     * \param y offset in y direction
     * \returns an extruded polygon
     */
    QgsGeometry extrude( double x, double y ) const;

    /**
     * Calculates the approximate pole of inaccessibility for a surface, which is the
     * most distant internal point from the boundary of the surface. This function
     * uses the 'polylabel' algorithm (Vladimir Agafonkin, 2016), which is an iterative
     * approach guaranteed to find the true pole of inaccessibility within a specified
     * tolerance. More precise tolerances require more iterations and will take longer
     * to calculate.
     * Optionally, the distance to the polygon boundary from the pole can be stored.
     */
    QgsGeometry poleOfInaccessibility( double precision, double *distanceFromBoundary = nullptr ) const;

    /**
     * Attempts to orthogonalize a line or polygon geometry by shifting vertices to make the geometries
     * angles either right angles or flat lines. This is an iterative algorithm which will loop until
     * either the vertices are within a specified tolerance of right angles or a set number of maximum
     * iterations is reached. The angle threshold parameter specifies how close to a right angle or
     * straight line an angle must be before it is attempted to be straightened.
     * \since QGIS 3.0
     */
    QgsGeometry orthogonalize( double tolerance = 1.0E-8, int maxIterations = 1000, double angleThreshold = 15.0 ) const;

    /**
     * Densifies the geometry by adding the specified number of extra nodes within each
     * segment of the geometry.
     * If the geometry has z or m values present then these will be linearly interpolated
     * at the added nodes.
     * Curved geometry types are automatically segmentized by this routine.
     * \since QGIS 3.0
     */
    QgsGeometry densifyByCount( int extraNodesPerSegment ) const;

    /**
     * Densifies the geometry by adding regularly placed extra nodes inside each segment
     * so that the maximum distance between any two nodes does not exceed the
     * specified \a distance.
     * E.g. specifying a distance 3 would cause the segment [0 0] -> [10 0]
     * to be converted to [0 0] -> [2.5 0] -> [5 0] -> [7.5 0] -> [10 0], since
     * 3 extra nodes are required on the segment and spacing these at 2.5 increments
     * allows them to be evenly spaced over the segment.
     * If the geometry has z or m values present then these will be linearly interpolated
     * at the added nodes.
     * Curved geometry types are automatically segmentized by this routine.
     * \since QGIS 3.0
     */
    QgsGeometry densifyByDistance( double distance ) const;

    /**
     * Calculates a variable width buffer for a (multi)curve geometry.
     *
     * The width of the buffer at each node in the input linestrings is calculated by
     * calling the specified \a widthFunction, which must return an array of the buffer widths
     * for every node in the line.
     *
     * The \a segments argument specifies the number of segments to approximate quarter-circle
     * curves in the buffer.
     *
     * Non (multi)curve input geometries will return a null output geometry.
     *
     * \since QGIS 3.2
     */
    QgsGeometry variableWidthBuffer( int segments, const std::function< std::unique_ptr< double[] >( const QgsLineString *line ) > &widthFunction ) const;

    /**
     * Calculates a tapered width buffer for a (multi)curve geometry.
     *
     * The buffer begins at a width of \a startWidth at the start of each curve, and
     * ends at a width of \a endWidth. Note that unlike QgsGeometry::buffer() methods, \a startWidth
     * and \a endWidth are the diameter of the buffer at these points, not the radius.
     *
     * The \a segments argument specifies the number of segments to approximate quarter-circle
     * curves in the buffer.
     *
     * Non (multi)curve input geometries will return a null output geometry.
     *
     * \since QGIS 3.2
     */
    QgsGeometry taperedBuffer( double startWidth, double endWidth, int segments ) const;

    /**
     * Calculates a variable width buffer using the m-values from a (multi)line geometry.
     *
     * The \a segments argument specifies the number of segments to approximate quarter-circle
     * curves in the buffer.
     *
     * Non (multi)line input geometries will return a null output geometry.
     *
     * \since QGIS 3.2
     */
    QgsGeometry variableWidthBufferByM( int segments ) const;

    /**
     * Returns a list of \a count random points generated inside a \a polygon geometry
     * (if \a acceptPoint is specified, and restrictive, the number of points returned may
     * be less than \a count).
     *
     * Optionally, a specific random \a seed can be used when generating points. If \a seed
     * is 0, then a completely random sequence of points will be generated.
     *
     * The \a acceptPoint function is used to filter result candidates. If the function returns
     * FALSE, then the point will not be accepted and another candidate generated.
     *
     * The optional \a feedback argument can be used to provide cancellation support during
     * the point generation.
     *
     * When \a acceptPoint is specified, \a maxTriesPerPoint
     * defines how many attempts to perform before giving up generating
     * a point.
     *
     * \since QGIS 3.10
     */
    static QVector< QgsPointXY > randomPointsInPolygon( const QgsGeometry &polygon, int count,
        const std::function< bool( const QgsPointXY & ) > &acceptPoint, unsigned long seed = 0, QgsFeedback *feedback = nullptr, int maxTriesPerPoint = 0 );

    /**
     * Attempts to convert a non-curved geometry into a curved geometry type (e.g.
     * LineString to CompoundCurve, Polygon to CurvePolygon).
     *
     * The \a distanceTolerance specifies the maximum deviation allowed between the original location
     * of vertices and where they would fall on the candidate curved geometry.
     *
     * This method only consider a segments as suitable for replacing with an arc if the points are all
     * regularly spaced on the candidate arc. The \a pointSpacingAngleTolerance parameter specifies the maximum
     * angular deviation (in radians) allowed when testing for regular point spacing.
     *
     * \note The API is considered EXPERIMENTAL and can be changed without a notice
     *
     * \since QGIS 3.14
     */
    QgsGeometry convertToCurves( double distanceTolerance, double angleTolerance ) const;

    /**
     * Returns the oriented minimum bounding box for the geometry, which is the smallest (by area)
     * rotated rectangle which fully encompasses the geometry.
     * The area, angle of the long axis (clockwise in degrees from North),
     * width and height of the rotated bounding box will also be returned.
     *
     * If an error was encountered while creating the result, more information can be retrieved
     * by calling lastError().
     *
     * \since QGIS 3.16
     */
    QgsGeometry orientedMinimumBoundingBox( double &area SIP_OUT, double &angle SIP_OUT, double &width SIP_OUT, double &height SIP_OUT ) const;

    /**
     * Constructs triangular waves along the boundary of the geometry, with the
     * specified \a wavelength and \a amplitude.
     *
     * By default the \a wavelength argument is treated as a "maximum wavelength", where the actual
     * wavelength will be dynamically adjusted so that an exact number of triangular waves are created
     * along the boundaries of the geometry. If \a strictWavelength is set to TRUE then the \a wavelength
     * will be used exactly and an incomplete pattern may be used for the final waveform.
     *
     * \see triangularWavesRandomized()
     * \since QGIS 3.24
     */
    QgsGeometry triangularWaves( double wavelength, double amplitude, bool strictWavelength = false ) const;

    /**
     * Constructs randomized triangular waves along the boundary of the geometry, with the
     * specified wavelength and amplitude ranges.
     *
     * The \a minimumWavelength and \a maximumWavelength arguments set the range for the randomized
     * wavelength. This is evaluated for each individual triangular waveform created along the geometry
     * boundaries, so the resultant geometry will consist of many different wavelengths.
     *
     * Similarly, the \a minimumAmplitude and \a maximumAmplitude arguments define the range for the
     * randomized amplitude of the triangular components. Randomized amplitude values will be calculated
     * individiually for triangles placed on each either side of the input geometry boundaries.
     *
     * Optionally, a specific random \a seed can be used when generating points. If \a seed
     * is 0, then a completely random sequence of points will be generated.
     *
     * \see triangularWaves()
     * \since QGIS 3.24
     */
    QgsGeometry triangularWavesRandomized( double minimumWavelength, double maximumWavelength, double minimumAmplitude, double maximumAmplitude, unsigned long seed = 0 ) const;

    /**
     * Constructs square waves along the boundary of the geometry, with the
     * specified \a wavelength and \a amplitude.
     *
     * By default the \a wavelength argument is treated as a "maximum wavelength", where the actual
     * wavelength will be dynamically adjusted so that an exact number of square waves are created
     * along the boundaries of the geometry. If \a strictWavelength is set to TRUE then the \a wavelength
     * will be used exactly and an incomplete pattern may be used for the final waveform.
     *
     * \see squareWavesRandomized()
     * \since QGIS 3.24
     */
    QgsGeometry squareWaves( double wavelength, double amplitude, bool strictWavelength = false ) const;

    /**
     * Constructs randomized square waves along the boundary of the geometry, with the
     * specified wavelength and amplitude ranges.
     *
     * The \a minimumWavelength and \a maximumWavelength arguments set the range for the randomized
     * wavelength. This is evaluated for each individual square waveform created along the geometry
     * boundaries, so the resultant geometry will consist of many different wavelengths.
     *
     * Similarly, the \a minimumAmplitude and \a maximumAmplitude arguments define the range for the
     * randomized amplitude of the square components. Randomized amplitude values will be calculated
     * individiually for squares placed on each either side of the input geometry boundaries.
     *
     * Optionally, a specific random \a seed can be used when generating points. If \a seed
     * is 0, then a completely random sequence of points will be generated.
     *
     * \see squareWaves()
     * \since QGIS 3.24
     */
    QgsGeometry squareWavesRandomized( double minimumWavelength, double maximumWavelength, double minimumAmplitude, double maximumAmplitude, unsigned long seed = 0 ) const;


    /**
     * Constructs rounded (sine-like) waves along the boundary of the geometry, with the
     * specified \a wavelength and \a amplitude.
     *
     * By default the \a wavelength argument is treated as a "maximum wavelength", where the actual
     * wavelength will be dynamically adjusted so that an exact number of waves are created
     * along the boundaries of the geometry. If \a strictWavelength is set to TRUE then the \a wavelength
     * will be used exactly and an incomplete pattern may be used for the final waveform.
     *
     * \see roundWavesRandomized()
     * \since QGIS 3.24
     */
    QgsGeometry roundWaves( double wavelength, double amplitude, bool strictWavelength = false ) const;

    /**
     * Constructs randomized rounded (sine-like) waves along the boundary of the geometry, with the
     * specified wavelength and amplitude ranges.
     *
     * The \a minimumWavelength and \a maximumWavelength arguments set the range for the randomized
     * wavelength. This is evaluated for each individual waveform created along the geometry
     * boundaries, so the resultant geometry will consist of many different wavelengths.
     *
     * Similarly, the \a minimumAmplitude and \a maximumAmplitude arguments define the range for the
     * randomized amplitude of the square components. Randomized amplitude values will be calculated
     * individually for waves placed on each either side of the input geometry boundaries.
     *
     * Optionally, a specific random \a seed can be used when generating points. If \a seed
     * is 0, then a completely random sequence of points will be generated.
     *
     * \see squareWaves()
     * \since QGIS 3.24
     */
    QgsGeometry roundWavesRandomized( double minimumWavelength, double maximumWavelength, double minimumAmplitude, double maximumAmplitude, unsigned long seed = 0 ) const;

    /**
     * Applies a dash pattern to a geometry, returning a MultiLineString geometry which is the
     * input geometry stroked along each line/ring with the specified \a pattern.
     *
     * The \a startRule and \a endRule options can be set to control how the dash pattern is adjusted
     * at line endings. If a \a startRule or \a endRule is set, the \a adjustment option defines whether
     * both dash and gaps, or only dash or gap sizes are adjusted to apply the rules.
     *
     * The \a patternOffset option specifies how far along the pattern the result should start at.
     * The offset is applied AFTER any start/end rules are applied.
     *
     * \since QGIS 3.24
     */
    QgsGeometry applyDashPattern( const QVector< double > &pattern,
                                  Qgis::DashPatternLineEndingRule startRule = Qgis::DashPatternLineEndingRule::NoRule,
                                  Qgis::DashPatternLineEndingRule endRule = Qgis::DashPatternLineEndingRule::NoRule,
                                  Qgis::DashPatternSizeAdjustment adjustment = Qgis::DashPatternSizeAdjustment::ScaleBothDashAndGap,
                                  double patternOffset = 0 ) const;

  private:
    const QgsAbstractGeometry *mGeometry = nullptr;

    mutable QString mLastError;
};

/**
 * \brief A 2D ray which extends from an origin point to an infinite distance in a given direction.
 * \ingroup core
 * \note not available in Python bindings
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsRay2D
{
  public:

    /**
     * Constructor for a ray starting at the given \a origin and extending an infinite distance
     * in the specified \a direction.
     */
    QgsRay2D( const QgsPointXY &origin, QgsVector direction )
      : origin( origin )
      , direction( direction )
    {}

    /**
     * Finds the closest intersection point of the ray and a line \a segment.
     *
     * If found, the intersection point will be stored in \a intersectPoint.
     *
     * Returns TRUE if the ray intersects the line segment.
     */
    bool intersects( const QgsLineSegment2D &segment, QgsPointXY &intersectPoint ) const;

  private:

    QgsPointXY origin;
    QgsVector direction;
};

///@cond PRIVATE

// adapted for QGIS geometry classes from original work at https://github.com/trylock/visibility by trylock

/**
 * \brief Compares two line segments based on their distance from a given point.
 *
 * Assumes: (1) the line segments are intersected by some ray from the origin
 *          (2) the line segments do not intersect except at their endpoints
 *          (3) no line segment is collinear with the origin
 * \ingroup core
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsLineSegmentDistanceComparer
{
  public:

    /**
     * Constructor for QgsLineSegmentDistanceComparer, comparing points
     * to the specified \a origin point.
     */
    explicit QgsLineSegmentDistanceComparer( const QgsPointXY &origin )
      : mOrigin( origin )
    {}

    /**
     * Checks whether the line segment \a ab is closer to the origin than the
     * line segment \a cd.
     * \param ab line segment: left hand side of the comparison operator
     * \param cd line segment: right hand side of the comparison operator
     * \returns TRUE if ab < cd (ab is closer than cd) to origin
     */
    bool operator()( QgsLineSegment2D ab, QgsLineSegment2D cd ) const;

  private:

    QgsPointXY mOrigin;

};


// adapted for QGIS geometry classes from original work at https://github.com/trylock/visibility by trylock

/**
 * \brief Compares angles from an origin to points clockwise, starting at the positive y-axis.
 *
 * \ingroup core
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsClockwiseAngleComparer
{
  public:
    explicit QgsClockwiseAngleComparer( const QgsPointXY &origin )
      : mVertex( origin )
    {}

    bool operator()( const QgsPointXY &a, const QgsPointXY &b ) const;

  private:

    QgsPointXY mVertex;

};

///@endcond PRIVATE

#endif // QGSINTERNALGEOMETRYENGINE_H
