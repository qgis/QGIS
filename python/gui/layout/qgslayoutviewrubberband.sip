/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutviewrubberband.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsLayoutViewRubberBand
{
%Docstring
 QgsLayoutViewRubberBand is an abstract base class for temporary rubber band items
 in various shapes, for use within QgsLayoutView widgets.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutviewrubberband.h"
%End
%ConvertToSubClassCode
    if ( dynamic_cast<QgsLayoutViewMouseEvent *>( sipCpp ) )
      sipType = sipType_QgsLayoutViewMouseEvent;
    else
      sipType = 0;
%End
  public:

    QgsLayoutViewRubberBand( QgsLayoutView *view = 0 );
%Docstring
 Constructor for QgsLayoutViewRubberBand.
%End

    virtual ~QgsLayoutViewRubberBand();

    virtual QgsLayoutViewRubberBand *create( QgsLayoutView *view ) const = 0 /Factory/;
%Docstring
 Creates a new instance of the QgsLayoutViewRubberBand subclass.
 :rtype: QgsLayoutViewRubberBand
%End

    virtual void start( QPointF position, Qt::KeyboardModifiers modifiers ) = 0;
%Docstring
 Called when a rubber band should be created at the specified
 starting ``position`` (in layout coordinate space).
%End

    virtual void update( QPointF position, Qt::KeyboardModifiers modifiers ) = 0;
%Docstring
 Called when a rubber band should be updated to reflect a temporary
 ending ``position`` (in layout coordinate space).
%End

    virtual QRectF finish( QPointF position = QPointF(), Qt::KeyboardModifiers modifiers = 0 ) = 0;
%Docstring
 Called when a rubber band use has finished and the rubber
 band is no longer required.
 Returns the final bounding box of the rubber band.
 :rtype: QRectF
%End

    QgsLayoutView *view() const;
%Docstring
 Returns the view associated with the rubber band.
.. seealso:: layout()
 :rtype: QgsLayoutView
%End

    QgsLayout *layout() const;
%Docstring
 Returns the layout associated with the rubber band.
.. seealso:: view()
 :rtype: QgsLayout
%End

    QBrush brush() const;
%Docstring
 Returns the brush used for drawing the rubber band.
.. seealso:: setBrush()
.. seealso:: pen()
 :rtype: QBrush
%End

    void setBrush( const QBrush &brush );
%Docstring
 Sets the ``brush`` used for drawing the rubber band.
.. seealso:: brush()
.. seealso:: setPen()
%End

    QPen pen() const;
%Docstring
 Returns the pen used for drawing the rubber band.
.. seealso:: setPen()
.. seealso:: brush()
 :rtype: QPen
%End

    void setPen( const QPen &pen );
%Docstring
 Sets the ``pen`` used for drawing the rubber band.
.. seealso:: pen()
.. seealso:: setBrush()
%End

  protected:

    QRectF updateRect( QPointF start, QPointF position, bool constrainSquare, bool fromCenter );
%Docstring
 Calculates an updated bounding box rectangle from a original ``start`` position
 and new ``position``. If ``constrainSquare`` is true then the bounding box will be
 forced to a square shape. If ``fromCenter`` is true then the original ``start``
 position will form the center point of the returned rectangle.
 :rtype: QRectF
%End

};


class QgsLayoutViewRectangularRubberBand : QgsLayoutViewRubberBand
{
%Docstring
 QgsLayoutViewRectangularRubberBand is rectangular rubber band for use within QgsLayoutView widgets.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutviewrubberband.h"
%End
  public:

    QgsLayoutViewRectangularRubberBand( QgsLayoutView *view = 0 );
%Docstring
 Constructor for QgsLayoutViewRectangularRubberBand.
%End
    virtual QgsLayoutViewRectangularRubberBand *create( QgsLayoutView *view ) const /Factory/;


    ~QgsLayoutViewRectangularRubberBand();

    virtual void start( QPointF position, Qt::KeyboardModifiers modifiers );

    virtual void update( QPointF position, Qt::KeyboardModifiers modifiers );

    virtual QRectF finish( QPointF position = QPointF(), Qt::KeyboardModifiers modifiers = 0 );


};

class QgsLayoutViewEllipticalRubberBand : QgsLayoutViewRubberBand
{
%Docstring
 QgsLayoutViewEllipseRubberBand is elliptical rubber band for use within QgsLayoutView widgets.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutviewrubberband.h"
%End
  public:

    QgsLayoutViewEllipticalRubberBand( QgsLayoutView *view = 0 );
%Docstring
 Constructor for QgsLayoutViewEllipticalRubberBand.
%End
    virtual QgsLayoutViewEllipticalRubberBand *create( QgsLayoutView *view ) const /Factory/;


    ~QgsLayoutViewEllipticalRubberBand();

    virtual void start( QPointF position, Qt::KeyboardModifiers modifiers );

    virtual void update( QPointF position, Qt::KeyboardModifiers modifiers );

    virtual QRectF finish( QPointF position = QPointF(), Qt::KeyboardModifiers modifiers = 0 );


};

class QgsLayoutViewTriangleRubberBand : QgsLayoutViewRubberBand
{
%Docstring
 QgsLayoutViewTriangleRubberBand is triangular rubber band for use within QgsLayoutView widgets.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutviewrubberband.h"
%End
  public:

    QgsLayoutViewTriangleRubberBand( QgsLayoutView *view = 0 );
%Docstring
 Constructor for QgsLayoutViewTriangleRubberBand.
%End
    virtual QgsLayoutViewTriangleRubberBand *create( QgsLayoutView *view ) const /Factory/;


    ~QgsLayoutViewTriangleRubberBand();

    virtual void start( QPointF position, Qt::KeyboardModifiers modifiers );

    virtual void update( QPointF position, Qt::KeyboardModifiers modifiers );

    virtual QRectF finish( QPointF position = QPointF(), Qt::KeyboardModifiers modifiers = 0 );


};
/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutviewrubberband.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
