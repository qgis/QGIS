/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgscolorbutton.h                                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsColorButton : QToolButton
{
%Docstring
A cross platform button subclass for selecting colors. Will open a color chooser dialog when clicked.
Offers live updates to button from color chooser dialog. An attached drop-down menu allows for copying
and pasting colors, picking colors from the screen, and selecting colors from color swatch grids.

.. versionadded:: 2.5
%End

%TypeHeaderCode
#include "qgscolorbutton.h"
%End
%ConvertToSubClassCode
    if ( qobject_cast<QgsColorButton *>( sipCpp ) )
      sipType = sipType_QgsColorButton;
    else
      sipType = NULL;
%End
  public:

    enum Behavior
    {
      ShowDialog,
      SignalOnly
    };

    QgsColorButton( QWidget *parent /TransferThis/ = 0, const QString &cdt = QString(), QgsColorSchemeRegistry *registry = 0 );
%Docstring
Construct a new color ramp button.
Use ``parent`` to attach a parent QWidget to the dialog.
Use ``cdt`` string to define the title to show in the color ramp dialog
Use a color scheme ``registry`` for color swatch grids to show in the drop-down menu. If not specified,
the button will use the global color scheme registry instead
%End

    virtual QSize minimumSizeHint() const;

    virtual QSize sizeHint() const;


    QColor color() const;
%Docstring
Return the currently selected color.

:return: currently selected color

.. seealso:: :py:func:`setColor`
%End

    void setAllowOpacity( const bool allowOpacity );
%Docstring
Sets whether opacity modification (transparency) is permitted
for the color. Defaults to false.

:param allowOpacity: set to true to allow opacity modification

.. seealso:: :py:func:`allowOpacity`

.. versionadded:: 3.0
%End

    bool allowOpacity() const;
%Docstring
Returns whether opacity modification (transparency) is permitted
for the color.

:return: true if opacity modification is allowed

.. seealso:: :py:func:`setAllowOpacity`

.. versionadded:: 3.0
%End

    void setColorDialogTitle( const QString &title );
%Docstring
Set the title for the color chooser dialog window.

:param title: Title for the color chooser dialog

.. seealso:: :py:func:`colorDialogTitle`
%End

    QString colorDialogTitle() const;
%Docstring
Returns the title for the color chooser dialog window.

:return: title for the color chooser dialog

.. seealso:: :py:func:`setColorDialogTitle`
%End

    bool acceptLiveUpdates() const;
%Docstring
Returns whether the button accepts live updates from QColorDialog.

:return: true if the button will be accepted immediately when the dialog's color changes

.. seealso:: :py:func:`setAcceptLiveUpdates`
%End

    void setAcceptLiveUpdates( const bool accept );
%Docstring
Sets whether the button accepts live updates from QColorDialog. Live updates may cause changes
that are not undoable on QColorDialog cancel.

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

    void setBehavior( const Behavior behavior );
%Docstring
Sets the behavior for when the button is clicked. The default behavior is to show
a color picker dialog.

:param behavior: behavior when button is clicked

.. seealso:: :py:func:`behavior`
%End

    Behavior behavior() const;
%Docstring
Returns the behavior for when the button is clicked.

:return: behavior when button is clicked

.. seealso:: :py:func:`setBehavior`
%End

    void setDefaultColor( const QColor &color );
%Docstring
Sets the default color for the button, which is shown in the button's drop-down menu for the
"default color" option.

:param color: default color for the button. Set to an invalid QColor to disable the default color
option.

.. seealso:: :py:func:`defaultColor`
%End

    QColor defaultColor() const;
%Docstring
Returns the default color for the button, which is shown in the button's drop-down menu for the
"default color" option.

:return: default color for the button. Returns an invalid QColor if the default color
option is disabled.

.. seealso:: :py:func:`setDefaultColor`
%End

    void setShowNoColor( const bool showNoColorOption );
%Docstring
Sets whether the "no color" option should be shown in the button's drop-down menu. If selected,
the "no color" option sets the color button's color to a totally transparent color.

:param showNoColorOption: set to true to show the no color option. This is disabled by default.

.. seealso:: :py:func:`showNoColor`

.. seealso:: :py:func:`setNoColorString`

.. note::

   The "no color" option is only shown if the color button is set to show an alpha channel in the color
dialog (see setColorDialogOptions)
%End

    bool showNoColor() const;
%Docstring
Returns whether the "no color" option is shown in the button's drop-down menu. If selected,
the "no color" option sets the color button's color to a totally transparent color.

:return: true if the no color option is shown.

.. seealso:: :py:func:`setShowNoColor`

.. seealso:: :py:func:`noColorString`

.. note::

   The "no color" option is only shown if the color button is set to show an alpha channel in the color
dialog (see setColorDialogOptions)
%End

    void setNoColorString( const QString &noColorString );
%Docstring
Sets the string to use for the "no color" option in the button's drop-down menu.

:param noColorString: string to use for the "no color" menu option

.. seealso:: :py:func:`noColorString`

.. seealso:: :py:func:`setShowNoColor`

.. note::

   The "no color" option is only shown if the color button is set to show an alpha channel in the color
dialog (see setColorDialogOptions)
%End

    void setShowNull( bool showNull );
%Docstring
Sets whether a set to null (clear) option is shown in the button's drop-down menu.

:param showNull: set to true to show a null option

.. versionadded:: 2.16

.. seealso:: :py:func:`showNull`

.. seealso:: :py:func:`isNull`
%End

    bool showNull() const;
%Docstring
Returns whether the set to null (clear) option is shown in the button's drop-down menu.

.. versionadded:: 2.16

.. seealso:: :py:func:`setShowNull`

.. seealso:: :py:func:`isNull`
%End

    bool isNull() const;
%Docstring
Returns true if the current color is null.

.. versionadded:: 2.16

.. seealso:: :py:func:`setShowNull`

.. seealso:: :py:func:`showNull`
%End

    QString noColorString() const;
%Docstring
Returns the string used for the "no color" option in the button's drop-down menu.

:return: string used for the "no color" menu option

.. seealso:: :py:func:`setNoColorString`

.. seealso:: :py:func:`showNoColor`

.. note::

   The "no color" option is only shown if the color button is set to show an alpha channel in the color
dialog (see setColorDialogOptions)
%End

    void setContext( const QString &context );
%Docstring
Sets the context string for the color button. The context string is passed to all color swatch
grids shown in the button's drop-down menu, to allow them to customise their display colors
based on the context.

:param context: context string for the color button's color swatch grids

.. seealso:: :py:func:`context`
%End

    QString context() const;
%Docstring
Returns the context string for the color button. The context string is passed to all color swatch
grids shown in the button's drop-down menu, to allow them to customise their display colors
based on the context.

:return: context string for the color button's color swatch grids

.. seealso:: :py:func:`setContext`
%End

    void setColorSchemeRegistry( QgsColorSchemeRegistry *registry );
%Docstring
Sets the color scheme registry for the button, which controls the color swatch grids
that are shown in the button's drop-down menu.

:param registry: color scheme registry for the button. Set to 0 to hide all color
swatch grids from the button's drop-down menu.

.. seealso:: :py:func:`colorSchemeRegistry`
%End

    QgsColorSchemeRegistry *colorSchemeRegistry();
%Docstring
Returns the color scheme registry for the button, which controls the color swatch grids
that are shown in the button's drop-down menu.

:return: color scheme registry for the button. If returned value is 0 then all color
swatch grids are hidden from the button's drop-down menu.

.. seealso:: :py:func:`setColorSchemeRegistry`
%End

  public slots:

    void setColor( const QColor &color );
%Docstring
Sets the current color for the button. Will emit a colorChanged signal if the color is different
to the previous color.

:param color: new color for the button

.. seealso:: :py:func:`color`
%End

    void setButtonBackground( const QColor &color = QColor() );
%Docstring
Sets the background pixmap for the button based upon color and transparency.
Call directly to update background after adding/removing QColorDialog.ShowAlphaChannel option
but the color has not changed, i.e. setColor() wouldn't update button and
you want the button to retain the set color's alpha component regardless

:param color: Color for button background. If no color is specified, the button's current
color will be used
%End

    void copyColor();
%Docstring
Copies the current color to the clipboard

.. seealso:: :py:func:`pasteColor`
%End

    void pasteColor();
%Docstring
Pastes a color from the clipboard to the color button. If clipboard does not contain a valid
color or string representation of a color, then no change is applied.

.. seealso:: :py:func:`copyColor`
%End

    void activatePicker();
%Docstring
Activates the color picker tool, which allows for sampling a color from anywhere on the screen
%End

    void setToNoColor();
%Docstring
Sets color to a totally transparent color.

.. note::

   If the color button is not set to show an opacity channel in the color
dialog (see setColorDialogOptions) then the color will not be changed.

.. seealso:: :py:func:`setToNull`
%End

    void setToDefaultColor();
%Docstring
Sets color to the button's default color, if set.

.. seealso:: :py:func:`setDefaultColor`

.. seealso:: :py:func:`defaultColor`

.. seealso:: :py:func:`setToNull`
%End

    void setToNull();
%Docstring
Sets color to null.

.. seealso:: :py:func:`setToDefaultColor`

.. seealso:: :py:func:`setToNoColor`

.. versionadded:: 2.16
%End

  signals:

    void colorChanged( const QColor &color );
%Docstring
Is emitted whenever a new color is set for the button. The color is always valid.
In case the new color is the same no signal is emitted, to avoid infinite loops.

:param color: New color
%End

    void colorClicked( const QColor &color );
%Docstring
Emitted when the button is clicked, if the button's behavior is set to SignalOnly

:param color: button color

.. seealso:: :py:func:`setBehavior`

.. seealso:: :py:func:`behavior`
%End

  protected:

    virtual bool event( QEvent *e );

    virtual void changeEvent( QEvent *e );

    virtual void showEvent( QShowEvent *e );

    virtual void resizeEvent( QResizeEvent *event );


    static const QPixmap &transparentBackground();
%Docstring
Returns a checkboard pattern pixmap for use as a background to transparent colors
%End

    virtual void mousePressEvent( QMouseEvent *e );

%Docstring
Reimplemented to detect right mouse button clicks on the color button and allow dragging colors
%End

    virtual void mouseMoveEvent( QMouseEvent *e );

%Docstring
Reimplemented to allow dragging colors from button
%End

    virtual void mouseReleaseEvent( QMouseEvent *e );

%Docstring
Reimplemented to allow color picking
%End

    virtual void keyPressEvent( QKeyEvent *e );

%Docstring
Reimplemented to allow canceling color pick via keypress, and sample via space bar press
%End

    virtual void dragEnterEvent( QDragEnterEvent *e );

%Docstring
Reimplemented to accept dragged colors
%End

    virtual void dragLeaveEvent( QDragLeaveEvent *e );

%Docstring
Reimplemented to reset button appearance after drag leave
%End

    virtual void dropEvent( QDropEvent *e );

%Docstring
Reimplemented to accept dropped colors
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgscolorbutton.h                                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
