class ParametricLine
{
%TypeHeaderCode
#include <ParametricLine.h>
%End
  public:
    /** Default constructor*/
    ParametricLine();
    /** Constructor, par is a pointer to the parent object, controlpoly the controlpolygon
     */
    ParametricLine( ParametricLine *par /Transfer/, QVector<QgsPoint*> *controlpoly );
    /** Destructor*/
    virtual ~ParametricLine();
    virtual void add( ParametricLine *pl ) = 0;
    virtual void calcFirstDer( float t, Vector3D *v ) = 0;
    virtual void calcSecDer( float t, Vector3D *v ) = 0;
    //virtual QgsPoint calcPoint(float t);
    virtual void calcPoint( float t, QgsPoint* ) = 0;
    virtual void changeDirection() = 0;
    //virtual void draw(QPainter *p);
    virtual const QgsPoint *getControlPoint( int number ) const = 0;
    virtual const QVector<QgsPoint*> *getControlPoly() const = 0;
    virtual int getDegree() const = 0;
    virtual ParametricLine *getParent() const = 0;
    //virtual bool intersects(ParametricLine *pal);
    virtual void remove( int i ) = 0;
    virtual void setControlPoly( QVector<QgsPoint*> *cp ) = 0;
    virtual void setParent( ParametricLine *paral ) = 0;
};
