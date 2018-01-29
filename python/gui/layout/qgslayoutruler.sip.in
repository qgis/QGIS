/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutruler.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsLayoutRuler: QWidget
{
%Docstring
A custom ruler widget for use with QgsLayoutView, displaying the
current zoom and position of the visible layout and for interacting
with guides in a layout.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutruler.h"
%End
  public:

    explicit QgsLayoutRuler( QWidget *parent /TransferThis/ = 0, Qt::Orientation orientation = Qt::Horizontal );
%Docstring
Constructor for QgsLayoutRuler, with the specified ``parent`` widget and ``orientation``.
%End

    virtual QSize minimumSizeHint() const;


    void setSceneTransform( const QTransform &transform );
%Docstring
Sets the current scene ``transform``. This is usually the transform set for a view
showing the associated scene, in order to synchronize the view's display of
the scene with the rulers.
%End

    QgsLayoutView *layoutView();
%Docstring
Returns the current layout view associated with the ruler.

.. seealso:: :py:func:`setLayoutView`
%End

    void setLayoutView( QgsLayoutView *view );
%Docstring
Sets the current layout ``view`` to synchronize the ruler with.

.. seealso:: :py:func:`layoutView`
%End

    int rulerSize() const;
%Docstring
Returns the ruler size (either the height of a horizontal ruler or the
width of a vertical rule).
%End

    void setContextMenu( QMenu *menu );
%Docstring
Sets a context ``menu`` to show when right clicking occurs on the ruler.
Ownership of ``menu`` is unchanged.
%End

  public slots:

    void setCursorPosition( QPointF position );
%Docstring
Updates the ``position`` of the marker showing the current mouse position within
the view.
``position`` is in layout coordinates.
%End

  signals:
    void cursorPosChanged( QPointF );
%Docstring
Is emitted when mouse cursor coordinates change
%End

  protected:
    virtual void paintEvent( QPaintEvent *event );

    virtual void mouseMoveEvent( QMouseEvent *event );

    virtual void mousePressEvent( QMouseEvent *event );

    virtual void mouseReleaseEvent( QMouseEvent *event );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutruler.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
