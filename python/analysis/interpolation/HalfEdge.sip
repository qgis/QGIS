class HalfEdge
{
%TypeHeaderCode
#include <HalfEdge.h>
%End

  public:
    /** Default constructor. Values for mDual, mNext, mPoint are set to -10 which means that they are undefined*/
    HalfEdge();
    HalfEdge( int dual, int next, int point, bool mbreak, bool forced );
    ~HalfEdge();
    /** Returns the number of the dual HalfEdge*/
    int getDual() const;
    /** Returns the number of the next HalfEdge*/
    int getNext() const;
    /** Returns the number of the point at which this HalfEdge points*/
    int getPoint() const;
    /** Returns, whether the HalfEdge belongs to a break line or not*/
    bool getBreak() const;
    /** Returns, whether the HalfEdge belongs to a constrained edge or not*/
    bool getForced() const;
    /** Sets the number of the dual HalfEdge*/
    void setDual( int d );
    /** Sets the number of the next HalfEdge*/
    void setNext( int n );
    /** Sets the number of point at which this HalfEdge points*/
    void setPoint( int p );
    /** Sets the break flag*/
    void setBreak( bool b );
    /** Sets the forced flag*/
    void setForced( bool f );
};
