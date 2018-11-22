/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsgradientstopeditor.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsGradientStopEditor : QWidget
{
%Docstring
An interactive editor for previewing a gradient color ramp and modifying the position of color
stops along the gradient.

.. versionadded:: 2.16
%End

%TypeHeaderCode
#include "qgsgradientstopeditor.h"
%End
  public:

    QgsGradientStopEditor( QWidget *parent /TransferThis/ = 0, QgsGradientColorRamp *ramp = 0 );
%Docstring
Constructor for QgsGradientStopEditor.

:param parent: parent widget
:param ramp: optional initial gradient ramp
%End

    void setGradientRamp( const QgsGradientColorRamp &ramp );
%Docstring
Sets the current ramp shown in the editor.

:param ramp: color ramp

.. seealso:: :py:func:`gradientRamp`
%End

    QgsGradientColorRamp gradientRamp() const;
%Docstring
Returns the current ramp created by the editor.

.. seealso:: :py:func:`setGradientRamp`
%End

    void selectStop( int index );
%Docstring
Sets the currently selected stop.

:param index: index of stop, where 0 corresponds to the first stop

.. seealso:: :py:func:`selectedStop`
%End

    QgsGradientStop selectedStop() const;
%Docstring
Returns details about the currently selected stop.

.. seealso:: :py:func:`selectStop`
%End

    virtual QSize sizeHint() const;

    virtual void paintEvent( QPaintEvent *event );


  public slots:

    void setSelectedStopColor( const QColor &color );
%Docstring
Sets the color for the current selected stop.

:param color: new stop color

.. seealso:: :py:func:`setSelectedStopOffset`

.. seealso:: :py:func:`setSelectedStopDetails`

.. seealso:: :py:func:`setColor1`

.. seealso:: :py:func:`setColor2`
%End

    void setSelectedStopOffset( double offset );
%Docstring
Sets the offset for the current selected stop. This slot has no effect if either the
first or last stop is selected, as they cannot be repositioned.

:param offset: new stop offset

.. seealso:: :py:func:`setSelectedStopColor`

.. seealso:: :py:func:`setSelectedStopDetails`
%End

    void setSelectedStopDetails( const QColor &color, double offset );
%Docstring
Sets the color and offset for the current selected stop.

:param color: new stop color
:param offset: new stop offset

.. seealso:: :py:func:`setSelectedStopColor`

.. seealso:: :py:func:`setSelectedStopOffset`
%End

    void deleteSelectedStop();
%Docstring
Deletes the current selected stop. This slot has no effect if either the
first or last stop is selected, as they cannot be deleted.
%End

    void setColor1( const QColor &color );
%Docstring
Sets the color for the first stop.

:param color: new stop color

.. seealso:: :py:func:`setColor2`

.. seealso:: :py:func:`setSelectedStopColor`
%End

    void setColor2( const QColor &color );
%Docstring
Sets the color for the last stop.

:param color: new stop color

.. seealso:: :py:func:`setColor1`

.. seealso:: :py:func:`setSelectedStopColor`
%End

  signals:

    void changed();
%Docstring
Emitted when the gradient ramp is changed by a user
%End

    void selectedStopChanged( const QgsGradientStop &stop );
%Docstring
Emitted when the current selected stop changes.

:param stop: details about newly selected stop
%End

  protected:

    virtual void mouseMoveEvent( QMouseEvent *event );

    virtual void mousePressEvent( QMouseEvent *event );

    virtual void mouseDoubleClickEvent( QMouseEvent *event );

    virtual void keyPressEvent( QKeyEvent *event );


    virtual void dragEnterEvent( QDragEnterEvent *e );


    virtual void dropEvent( QDropEvent *e );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsgradientstopeditor.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
