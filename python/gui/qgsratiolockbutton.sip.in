/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsratiolockbutton.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsRatioLockButton : QToolButton
{
%Docstring
A cross platform button subclass used to represent a locked / unlocked ratio state.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsratiolockbutton.h"
%End
  public:

    QgsRatioLockButton( QWidget *parent /TransferThis/ = 0 );
%Docstring
Construct a new ratio lock button.
Use ``parent`` to attach a parent QWidget to the button.
%End

    void setLocked( const bool locked );
%Docstring
Sets whether the button state is locked.

:param locked: locked state

.. seealso:: :py:func:`locked`
%End

    bool locked() const;
%Docstring
Returns whether the button state is locked.

:return: true if the button state is locked.

.. seealso:: :py:func:`setLocked`
%End

    void setWidthSpinBox( QDoubleSpinBox *widget );
%Docstring
Registers a spin box ``widget`` as the linked "width" spin box.

If both a width and height spin box are linked to the button, they will automatically
have their values updates when if the other spin box value is changed. I.e. changing the
width spin box will automatically update the height spin box to a value which keeps the
same locked ratio.

.. seealso:: :py:func:`setHeightSpinBox`
%End

    void setHeightSpinBox( QDoubleSpinBox *widget );
%Docstring
Registers a spin box ``widget`` as the linked "height" spin box.

If both a width and height spin box are linked to the button, they will automatically
have their values updates when if the other spin box value is changed. I.e. changing the
width spin box will automatically update the height spin box to a value which keeps the
same locked ratio.

.. seealso:: :py:func:`setWidthSpinBox`
%End

    void resetRatio();
%Docstring
Resets the current width/height ratio, taking the width and height
from the current values of the width and height spin boxes.
%End

  signals:

    void lockChanged( const bool locked );
%Docstring
Emitted whenever the lock state changes.
%End

  protected:

    virtual void changeEvent( QEvent *e );

    virtual void showEvent( QShowEvent *e );

    virtual void resizeEvent( QResizeEvent *event );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsratiolockbutton.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
