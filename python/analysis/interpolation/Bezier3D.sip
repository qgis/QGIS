class Bezier3D : ParametricLine
{
%TypeHeaderCode
#include <Bezier3D.h>
%End

  public:
    /** Default constructor*/
    Bezier3D();
    /** Constructor, par is a pointer to the parent, controlpoly a controlpolygon*/
    Bezier3D( ParametricLine *par, QVector<QgsPoint*> *controlpoly );
    /** Destructor*/
    virtual ~Bezier3D();
    /** Do not use this method, since a Bezier curve does not consist of other curves*/
    virtual void add( ParametricLine *pl );
    /** Calculates the first derivative and assigns it to v*/
    virtual void calcFirstDer( float t, Vector3D *v );
    /** Calculates the second derivative and assigns it to v*/
    virtual void calcSecDer( float t, Vector3D *v );
    //virtual QgsPoint calcPoint(float t);
    /** Calculates the point on the curve and assigns it to p*/
    virtual void calcPoint( float t, QgsPoint *p );
    /** Changes the order of control points*/
    virtual void changeDirection();
    //virtual void draw(QPainter *p);
    //virtual bool intersects(ParametricLine *pal);
    /** Do not use this method, since a Bezier curve does not consist of other curves*/
    virtual void remove( int i );
    /** Returns a control point*/
    virtual const QgsPoint *getControlPoint( int number ) const;
    /** Returns a pointer to the control polygon*/
    virtual const QVector<QgsPoint*> *getControlPoly() const;
    /** Returns the degree of the curve*/
    virtual int getDegree() const;
    /** Returns the parent*/
    virtual ParametricLine *getParent() const;
    /** Sets the parent*/
    virtual void setParent( ParametricLine *par );
    /** Sets the control polygon*/
    virtual void setControlPoly( QVector<QgsPoint*> *cp );

};
