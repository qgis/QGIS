/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/interpolation/NormVecDecorator.h                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class NormVecDecorator: TriDecorator
{
%Docstring
 Decorator class which adds the functionality of estimating normals at the data points*
%End

%TypeHeaderCode
#include "NormVecDecorator.h"
%End
  public:
    enum PointState {Normal, BreakLine, EndPoint};
    NormVecDecorator();
%Docstring
Enumeration for the state of a point. Normal means, that the point is not on a BreakLine, BreakLine means that the point is on a breakline (but not an end point of it) and EndPoint means, that it is an endpoint of a breakline.
%End
    NormVecDecorator( Triangulation *tin );
    virtual ~NormVecDecorator();
    virtual int addPoint( QgsPoint *p );

%Docstring
Adds a point to the triangulation
 :rtype: int
%End
    virtual bool calcNormal( double x, double y, Vector3D *result /Out/ );

%Docstring
Calculates the normal at a point on the surface and assigns it to 'result'. Returns true in case of success and false in case of failure
 :rtype: bool
%End
    bool calcNormalForPoint( double x, double y, int point, Vector3D *result /Out/ );
%Docstring
Calculates the normal of a triangle-point for the point with coordinates x and y. This is needed, if a point is on a break line and there is no unique normal stored in 'mNormVec'. Returns false, it something went wrong and true otherwise
 :rtype: bool
%End
    virtual bool calcPoint( double x, double y, QgsPoint *result /Out/ );

%Docstring
Calculates x-, y and z-value of the point on the surface and assigns it to 'result'. Returns true in case of success and flase in case of failure
 :rtype: bool
%End
    virtual void eliminateHorizontalTriangles();
%Docstring
Eliminates the horizontal triangles by swapping or by insertion of new points. If alreadyestimated is true, a re-estimation of the normals will be done
%End
    bool estimateFirstDerivative( int pointno );
%Docstring
Estimates the first derivative a point. Return true in case of success and false otherwise
 :rtype: bool
%End
    bool estimateFirstDerivatives( QgsFeedback *feedback = 0 );
%Docstring
This method adds the functionality of estimating normals at the data points. Return true in the case of success and false otherwise
 :rtype: bool
%End
    Vector3D *getNormal( int n ) const;
%Docstring
Returns a pointer to the normal vector for the point with the number n
 :rtype: Vector3D
%End
    bool getTriangle( double x, double y, QgsPoint *p1 /Out/, Vector3D *v1 /Out/, QgsPoint *p2 /Out/, Vector3D *v2 /Out/, QgsPoint *p3 /Out/, Vector3D *v3 /Out/ )  /PyName=getTriangleVertices/;
%Docstring
Finds out, in which triangle a point with coordinates x and y is and assigns the triangle points to p1, p2, p3 and the estimated normals to v1, v2, v3. The vectors are normally taken from 'mNormVec', except if p1, p2 or p3 is a point on a breakline. In this case, the normal is calculated on-the-fly. Returns false, if something went wrong and true otherwise
 :rtype: bool
%End
    bool getTriangle( double x, double y, QgsPoint *p1 /Out/, int *ptn1 /Out/, Vector3D *v1 /Out/, PointState *state1 /Out/, QgsPoint *p2 /Out/, int *ptn2 /Out/, Vector3D *v2 /Out/, PointState *state2 /Out/, QgsPoint *p3 /Out/, int *ptn3 /Out/, Vector3D *v3 /Out/, PointState *state3 /Out/ );
%Docstring
This function behaves similar to the one above. Additionally, the numbers of the points are returned (ptn1, ptn2, ptn3) as well as the PointStates of the triangle points (state1, state2, state3)
 :rtype: bool
%End
    PointState getState( int pointno ) const;
%Docstring
Returns the state of the point with the number 'pointno'
 :rtype: PointState
%End
    virtual void setTriangleInterpolator( TriangleInterpolator *inter );

%Docstring
Sets an interpolator
%End
    virtual bool swapEdge( double x, double y );
%Docstring
Swaps the edge which is closest to the point with x and y coordinates (if this is possible) and forces recalculation of the concerned normals (if alreadyestimated is true)
 :rtype: bool
%End

    virtual bool saveAsShapefile( const QString &fileName ) const;
%Docstring
 Saves the triangulation as a (line) shapefile
:return: true in case of success*
 :rtype: bool
%End

  protected:
    void setState( int pointno, PointState s );
%Docstring
Sets the state (BreakLine, Normal, EndPoint) of a point
%End
};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/interpolation/NormVecDecorator.h                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
