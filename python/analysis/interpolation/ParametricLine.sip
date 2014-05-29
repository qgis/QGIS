class ParametricLine
{
%TypeHeaderCode
#include <ParametricLine.h>
%End
  public:
    /**Default constructor*/
    ParametricLine();
    /**Constructor, par is a pointer to the parent object, controlpoly the controlpolygon
      @note not available in python binding */
    // ParametricLine( ParametricLine* par /Transfer/, QVector<Point3D*>* controlpoly );
    /**Destructor*/
    virtual ~ParametricLine();
    virtual void add( ParametricLine* pl ) = 0;
    virtual void calcFirstDer( float t, Vector3D* v ) = 0;
    virtual void calcSecDer( float t, Vector3D* v ) = 0;
    //virtual Point3D calcPoint(float t);
    virtual void calcPoint( float t, Point3D* ) = 0;
    virtual void changeDirection() = 0;
    //virtual void draw(QPainter* p);
    virtual const Point3D* getControlPoint( int number ) const = 0;
    //! @note not available in python binding
    // virtual const QVector<Point3D*>* getControlPoly() const = 0;
    virtual int getDegree() const = 0;
    virtual ParametricLine* getParent() const = 0;
    //virtual bool intersects(ParametricLine* pal);
    virtual void remove( int i ) = 0;
    //! @note not available in python binding
    // virtual void setControlPoly( QVector<Point3D*>* cp ) = 0;
    virtual void setParent( ParametricLine* paral ) = 0;
};
