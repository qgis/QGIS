/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgscolorrampbutton.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsColorRampButton : QToolButton
{
%Docstring
A cross platform button subclass for selecting color ramps. Will open color ramp dialogs when clicked.
Offers live updates to button from color ramp dialog. An attached drop-down menu allows for access to
saved color ramps, as well as option to invert the current color ramp and create new ramps.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgscolorrampbutton.h"
%End
  public:

    QgsColorRampButton( QWidget *parent /TransferThis/ = 0, const QString &dialogTitle = QString() );
%Docstring
Construct a new color ramp button.
Use ``parent`` to attach a parent QWidget to the dialog.
Use ``dialogTitle`` string  to define the title to show in the color ramp dialog
%End

    ~QgsColorRampButton();

    virtual QSize sizeHint() const;


    QgsColorRamp *colorRamp() const /Factory/;
%Docstring
Return a copy of the current color ramp.

.. seealso:: :py:func:`setColorRamp`
%End

    void setColorRampDialogTitle( const QString &title );
%Docstring
Set the title for the color ramp dialog window.

:param title: Title for the color ramp dialog

.. seealso:: :py:func:`colorRampDialogTitle`
%End

    QString colorRampDialogTitle() const;
%Docstring
Returns the title for the color ramp dialog window.

:return: title for the color ramp dialog

.. seealso:: :py:func:`setColorRampDialogTitle`
%End

    bool acceptLiveUpdates() const;
%Docstring
Returns whether the button accepts live updates from :py:class:`QgsColorRampDialog`.

:return: true if the button will be accepted immediately when the dialog's color ramp changes

.. seealso:: :py:func:`setAcceptLiveUpdates`
%End

    void setAcceptLiveUpdates( const bool accept );
%Docstring
Sets whether the button accepts live updates from :py:class:`QgsColorRampDialog`. Live updates may cause changes
that are not undoable on QColorRampDialog cancel.

:param accept: set to true to enable live updates

.. seealso:: :py:func:`acceptLiveUpdates`
%End

    void setShowMenu( const bool showMenu );
%Docstring
Sets whether the drop-down menu should be shown for the button. The default behavior is to
show the menu.

:param showMenu: set to false to hide the drop-down menu

.. seealso:: :py:func:`showMenu`
%End

    bool showMenu() const;
%Docstring
Returns whether the drop-down menu is shown for the button.

:return: true if drop-down menu is shown

.. seealso:: :py:func:`setShowMenu`
%End

    void setDefaultColorRamp( QgsColorRamp *colorramp );
%Docstring
Sets the default color ramp for the button, which is shown in the button's drop-down menu for the
"default color ramp" option.

:param colorramp: default color ramp for the button. Set to a null pointer to disable the default color
ramp option. The ramp will be cloned and ownership is not transferred.

.. seealso:: :py:func:`defaultColorRamp`
%End

    QgsColorRamp *defaultColorRamp() const /Factory/;
%Docstring
Returns a copy of the default color ramp for the button, which is shown in the button's drop-down menu for the
"default color ramp" option.

:return: default color ramp for the button. Returns a null pointer if the default color ramp
option is disabled.

.. seealso:: :py:func:`setDefaultColorRamp`
%End

    void setShowRandomColorRamp( bool showRandom );
%Docstring
Sets whether a random colors option is shown in the button's drop-down menu.

:param showRandom: set to true to show a random colors option

.. seealso:: :py:func:`showRandom`
%End

    bool showRandomColorRamp() const;
%Docstring
Returns whether random colors option is shown in the button's drop-down menu.

.. seealso:: :py:func:`setShowRandom`
%End

    bool isRandomColorRamp() const;
%Docstring
Returns true if the current color is null.

.. seealso:: :py:func:`setShowNull`

.. seealso:: :py:func:`showNull`
%End

    void setShowNull( bool showNull );
%Docstring
Sets whether a set to null (clear) option is shown in the button's drop-down menu.

:param showNull: set to true to show a null option

.. seealso:: :py:func:`showNull`

.. seealso:: :py:func:`isNull`
%End

    bool showNull() const;
%Docstring
Returns whether the set to null (clear) option is shown in the button's drop-down menu.

.. seealso:: :py:func:`setShowNull`

.. seealso:: :py:func:`isNull`
%End

    bool isNull() const;
%Docstring
Returns true if the current color is null.

.. seealso:: :py:func:`setShowNull`

.. seealso:: :py:func:`showNull`
%End

    void setContext( const QString &context );
%Docstring
Sets the context string for the color ramp button. The context string is passed to all color ramp
preview icons shown in the button's drop-down menu, to (eventually) allow them to customise their display colors
based on the context.

:param context: context string for the color dialog button's color ramp preview icons

.. seealso:: :py:func:`context`
%End

    QString context() const;
%Docstring
Returns the context string for the color ramp button. The context string is passed to all color ramp
preview icons shown in the button's drop-down menu, to (eventually) allow them to customise their display colors
based on the context.

:return: context context string for the color dialog button's color ramp preview icons

.. seealso:: :py:func:`setContext`
%End

    void setShowGradientOnly( bool gradientonly );
%Docstring
Sets whether the color ramp button only shows gradient type ramps

:param gradientonly: set to true to show only gradient type ramps

.. seealso:: :py:func:`showGradientOnly`
%End

    bool showGradientOnly() const;
%Docstring
Returns true if the color ramp button only shows gradient type ramps

.. seealso:: :py:func:`setShowGradientOnly`
%End

    void setColorRampName( const QString &name );
%Docstring
Sets the name of the current color ramp when it's available in the style manager

:param name: Name of the saved color ramp

.. seealso:: :py:func:`colorRampName`
%End

    QString colorRampName() const;
%Docstring
Returns the name of the current color ramp when it's available in the style manager

.. seealso:: :py:func:`setColorRampName`
%End

  public slots:

    void setColorRamp( QgsColorRamp *colorramp );
%Docstring
Sets the current color ramp for the button. Will emit a colorRampChanged() signal if the color ramp is different
to the previous color ramp.

:param colorramp: New color ramp for the button. The ramp will be cloned and ownership is not transferred.

.. seealso:: :py:func:`setRandomColorRamp`
%End

    void setRandomColorRamp();
%Docstring
Sets the current color ramp for the button to random colors. Will emit a colorRampChanged() signal
if the color ramp is different to the previous color ramp.

.. seealso:: :py:func:`setColorRamp`
%End

    void setColorRampFromName( const QString &name = QString() );
%Docstring
Sets the current color ramp for the button using a saved color ramp name. Will emit a colorRampChanged() signal
if the color ramp is different to the previous color ramp.

:param name: Name of saved color ramp

.. seealso:: :py:func:`setColorRamp`
%End

    void setButtonBackground( QgsColorRamp *colorramp = 0 );
%Docstring
Sets the background pixmap for the button based upon current color ramp.

:param colorramp: Color ramp for button background. If no color ramp is specified, the button's current
color ramp will be used
%End

    void setToDefaultColorRamp();
%Docstring
Sets color ramp to the button's default color ramp, if set.

.. seealso:: :py:func:`setDefaultColorRamp`

.. seealso:: :py:func:`defaultColorRamp`

.. seealso:: :py:func:`setToNull`
%End

    void setToNull();
%Docstring
Sets color ramp to null.

.. seealso:: :py:func:`setToDefaultColorRamp`
%End

  signals:

    void colorRampChanged();
%Docstring
Emitted whenever a new color ramp is set for the button. The color ramp is always valid.
In case the new color ramp is the same, no signal is emitted to avoid infinite loops.
%End

  protected:

    virtual bool event( QEvent *e );

    virtual void changeEvent( QEvent *e );

    virtual void showEvent( QShowEvent *e );

    virtual void resizeEvent( QResizeEvent *event );


    virtual void mousePressEvent( QMouseEvent *e );

%Docstring
Reimplemented to detect right mouse button clicks on the color ramp button
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgscolorrampbutton.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
