/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsfloatingwidget.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsFloatingWidget: QWidget
{
%Docstring
A QWidget subclass for creating widgets which float outside of the normal Qt layout
system. Floating widgets use an "anchor widget" to determine how they are anchored
within their parent widget.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsfloatingwidget.h"
%End
  public:

    enum AnchorPoint
    {
      TopLeft,
      TopMiddle,
      TopRight,
      MiddleLeft,
      Middle,
      MiddleRight,
      BottomLeft,
      BottomMiddle,
      BottomRight,
    };

    QgsFloatingWidget( QWidget *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsFloatingWidget.

:param parent: parent widget
%End

    void setAnchorWidget( QWidget *widget );
%Docstring
Sets the widget to "anchor" the floating widget to. The floating widget will be repositioned whenever the
anchor widget moves or is resized so that it maintains the same relative position to the anchor widget.

:param widget: anchor widget. Both the floating widget and the anchor widget must share some common parent.

.. seealso:: :py:func:`anchorWidget`
%End

    QWidget *anchorWidget();
%Docstring
Returns the widget that the floating widget is "anchored" tto. The floating widget will be repositioned whenever the
anchor widget moves or is resized so that it maintains the same relative position to the anchor widget.

.. seealso:: :py:func:`setAnchorWidget`
%End

    AnchorPoint anchorPoint() const;
%Docstring
Returns the floating widget's anchor point, which corresponds to the point on the widget which should remain
fixed in the same relative position whenever the widget's parent is resized or moved.

.. seealso:: :py:func:`setAnchorPoint`
%End

    void setAnchorPoint( AnchorPoint point );
%Docstring
Sets the floating widget's anchor point, which corresponds to the point on the widget which should remain
fixed in the same relative position whenever the widget's parent is resized or moved.

:param point: anchor point

.. seealso:: :py:func:`anchorPoint`
%End

    AnchorPoint anchorWidgetPoint() const;
%Docstring
Returns the anchor widget's anchor point, which corresponds to the point on the anchor widget which
the floating widget should "attach" to. The floating widget should remain fixed in the same relative position
to this anchor widget whenever the widget's parent is resized or moved.

.. seealso:: :py:func:`setAnchorWidgetPoint`
%End

    void setAnchorWidgetPoint( AnchorPoint point );
%Docstring
Returns the anchor widget's anchor point, which corresponds to the point on the anchor widget which
the floating widget should "attach" to. The floating widget should remain fixed in the same relative position
to this anchor widget whenever the widget's parent is resized or moved.

.. seealso:: :py:func:`setAnchorWidgetPoint`
%End

  signals:

    void anchorWidgetChanged( QWidget *widget );
%Docstring
Emitted when the anchor widget changes
%End

    void anchorPointChanged( AnchorPoint point );
%Docstring
Emitted when the anchor point changes
%End

    void anchorWidgetPointChanged( AnchorPoint point );
%Docstring
Emitted when the anchor widget point changes
%End

  protected:
    virtual void showEvent( QShowEvent *e );

    virtual void paintEvent( QPaintEvent *e );


};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsfloatingwidget.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
