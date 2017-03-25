class QgsGeometryUtils
{
%TypeHeaderCode
#include <qgsgeometryutils.h>
%End

  public:

    static QList<QgsLineString*> extractLineStrings( const QgsAbstractGeometry* geom );

    static QgsPointV2 closestVertex( const QgsAbstractGeometry& geom, const QgsPointV2& pt, QgsVertexId& id /Out/ );

    static double distanceToVertex( const QgsAbstractGeometry& geom, QgsVertexId id );

    static bool verticesAtDistance( const QgsAbstractGeometry& geometry,
                                    double distance,
                                    QgsVertexId& previousVertex /Out/,
                                    QgsVertexId& nextVertex /Out/ );

    static void adjacentVertices( const QgsAbstractGeometry& geom, QgsVertexId atVertex, QgsVertexId& beforeVertex /Out/, QgsVertexId& afterVertex /Out/ );

    static double sqrDistance2D( const QgsPointV2& pt1, const QgsPointV2& pt2 );

    static double sqrDistToLine( double ptX, double ptY, double x1, double y1, double x2, double y2, double& minDistX /Out/, double& minDistY /Out/, double epsilon );

    static bool lineIntersection( const QgsPointV2& p1, QgsVector v, const QgsPointV2& q1, QgsVector w, QgsPointV2& inter /Out/ );

    static bool segmentIntersection( const QgsPointV2 &p1, const QgsPointV2 &p2, const QgsPointV2 &q1, const QgsPointV2 &q2, QgsPointV2& inter /Out/, double tolerance );

    static QgsPointV2 projPointOnSegment( const QgsPointV2& p, const QgsPointV2& s1, const QgsPointV2& s2 );

    static double leftOfLine( double x, double y, double x1, double y1, double x2, double y2 );

    static QgsPointV2 pointOnLineWithDistance( const QgsPointV2& startPoint, const QgsPointV2& directionPoint, double distance );

    static double ccwAngle( double dy, double dx );

    static void circleCenterRadius( const QgsPointV2& pt1, const QgsPointV2& pt2, const QgsPointV2& pt3, double& radius /Out/,
                                    double& centerX /Out/, double& centerY /Out/ );

    static bool circleClockwise( double angle1, double angle2, double angle3 );

    static bool circleAngleBetween( double angle, double angle1, double angle2, bool clockwise );

    static bool angleOnCircle( double angle, double angle1, double angle2, double angle3 );

    static double circleLength( double x1, double y1, double x2, double y2, double x3, double y3 );

    static double sweepAngle( double centerX, double centerY, double x1, double y1, double x2, double y2, double x3, double y3 );

    static bool segmentMidPoint( const QgsPointV2& p1, const QgsPointV2& p2, QgsPointV2& result /Out/, double radius, const QgsPointV2& mousePos );

    static double circleTangentDirection( const QgsPointV2& tangentPoint, const QgsPointV2& cp1, const QgsPointV2& cp2, const QgsPointV2& cp3 );

    static double normalizedAngle( double angle );

    static double lineAngle( double x1, double y1, double x2, double y2 );

    static double angleBetweenThreePoints( double x1, double y1, double x2, double y2,
                                           double x3, double y3 );

    static double linePerpendicularAngle( double x1, double y1, double x2, double y2 );

    static double averageAngle( double x1, double y1, double x2, double y2, double x3, double y3 );

    static double averageAngle( double a1, double a2 );

    static QgsPointV2 midpoint (const QgsPointV2& pt1, const QgsPointV2& pt2);

    static double gradient( const QgsPointV2& pt1, const QgsPointV2& pt2 );

    static void coefficients(const QgsPointV2 &pt1, const QgsPointV2 &pt2, double& a /Out/, double& b /Out/, double& c /Out/);

    static QgsLineString perpendicularSegment( const QgsPointV2& p, const QgsPointV2& s1, const QgsPointV2& s2 );

};
