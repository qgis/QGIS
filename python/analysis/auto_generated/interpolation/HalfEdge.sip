/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/interpolation/HalfEdge.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class HalfEdge
{

%TypeHeaderCode
#include "HalfEdge.h"
%End
  protected:

  public:
    HalfEdge();
%Docstring
Default constructor. Values for mDual, mNext, mPoint are set to -10 which means that they are undefined
%End
    HalfEdge( int dual, int next, int point, bool mbreak, bool forced );

    int getDual() const;
%Docstring
Returns the number of the dual HalfEdge
 :rtype: int
%End
    int getNext() const;
%Docstring
Returns the number of the next HalfEdge
 :rtype: int
%End
    int getPoint() const;
%Docstring
Returns the number of the point at which this HalfEdge points
 :rtype: int
%End
    bool getBreak() const;
%Docstring
Returns, whether the HalfEdge belongs to a break line or not
 :rtype: bool
%End
    bool getForced() const;
%Docstring
Returns, whether the HalfEdge belongs to a constrained edge or not
 :rtype: bool
%End
    void setDual( int d );
%Docstring
Sets the number of the dual HalfEdge
%End
    void setNext( int n );
%Docstring
Sets the number of the next HalfEdge
%End
    void setPoint( int p );
%Docstring
Sets the number of point at which this HalfEdge points
%End
    void setBreak( bool b );
%Docstring
Sets the break flag
%End
    void setForced( bool f );
%Docstring
Sets the forced flag
%End
};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/interpolation/HalfEdge.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
