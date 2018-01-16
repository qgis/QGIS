/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsfontbutton.h                                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsFontButton : QToolButton
{
%Docstring
A button for customising QgsTextFormat settings.

The button will open a detailed text format settings dialog when clicked. An attached drop-down
menu allows for copying and pasting text styles, picking colors for the text, and for dropping
colors from other color widgets.

The button can be used in two different modes(). The default behavior is to include
all settings used for configuring QgsTextFormat/QgsTextRenderer classes. A cut down
mode (without settings for color) is also available when the resultant font is
used only in a QFont object.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsfontbutton.h"
%End
  public:

    enum Mode
    {
      ModeTextRenderer,
      ModeQFont,
    };


    QgsFontButton( QWidget *parent /TransferThis/ = 0, const QString &dialogTitle = QString() );
%Docstring
Construct a new font button.
Use ``parent`` to attach a parent QWidget to the dialog.
Use ``dialogTitle`` string to define the title to show in the text settings dialog.
%End

    virtual QSize minimumSizeHint() const;

    virtual QSize sizeHint() const;


    QgsFontButton::Mode mode() const;
%Docstring
Returns the current button mode.

.. seealso:: :py:func:`setMode`
%End

    void setMode( const QgsFontButton::Mode &mode );
%Docstring
Sets the current button ``mode``. This can be used to toggle between
the full capabilities of the button (for configuring QgsTextFormat/QgsTextRenderer objects)
and a cut-back version for configuring QFont object properties (i.e. with
no color settings or the other advanced options QgsTextFormat allows).

.. seealso:: :py:func:`mode`
%End

    void setDialogTitle( const QString &title );
%Docstring
Sets the ``title`` for the text settings dialog window.

.. seealso:: :py:func:`dialogTitle`
%End

    QString dialogTitle() const;
%Docstring
Returns the title for the text settings dialog window.

.. seealso:: :py:func:`setDialogTitle`
%End

    QgsMapCanvas *mapCanvas() const;
%Docstring
Returns the map canvas associated with the widget.

.. seealso:: :py:func:`setMapCanvas`
%End

    void setMapCanvas( QgsMapCanvas *canvas );
%Docstring
Sets a map ``canvas`` to associate with the widget. This allows the
widget to fetch current settings from the map canvas, such as current scale.

.. seealso:: :py:func:`mapCanvas`
%End

    QgsTextFormat textFormat() const;
%Docstring
Returns the current text formatting set by the widget.
This is only used when mode() is ModeTextRenderer.

.. seealso:: :py:func:`setTextFormat`
%End

    QFont currentFont() const;
%Docstring
Returns the current QFont set by the widget.
This is only used when mode() is ModeQFont.

.. seealso:: :py:func:`setCurrentFont`
%End


  public slots:

    void setTextFormat( const QgsTextFormat &format );
%Docstring
Sets the current text ``format`` to show in the widget.
This is only used when mode() is ModeTextRenderer.

.. seealso:: :py:func:`textFormat`
%End

    void setCurrentFont( const QFont &font );
%Docstring
Sets the current text ``font`` to show in the widget.
This is only used when mode() is ModeQFont.

.. seealso:: :py:func:`currentFont`
%End

    void setColor( const QColor &color );
%Docstring
Sets the current ``color`` for the text. Will emit a changed signal if the color is different
to the previous text color.
This is only used when mode() is ModeTextRenderer.
%End

    void copyFormat();
%Docstring
Copies the current text format to the clipboard.

.. seealso:: :py:func:`pasteFormat`
%End

    void pasteFormat();
%Docstring
Pastes a format from the clipboard. If clipboard does not contain a valid
format then no change is applied.

.. seealso:: :py:func:`copyFormat`
%End

    void copyColor();
%Docstring
Copies the current text color to the clipboard.
This is only used when mode() is ModeTextRenderer.

.. seealso:: :py:func:`pasteColor`
%End

    void pasteColor();
%Docstring
Pastes a color from the clipboard to the text format. If clipboard does not contain a valid
color or string representation of a color, then no change is applied.
This is only used when mode() is ModeTextRenderer.

.. seealso:: :py:func:`copyColor`
%End

  signals:

    void changed();
%Docstring
Emitted when the widget's text format settings are changed.
%End

  protected:

    virtual bool event( QEvent *e );

    virtual void changeEvent( QEvent *e );

    virtual void showEvent( QShowEvent *e );

    virtual void resizeEvent( QResizeEvent *event );


    virtual void mousePressEvent( QMouseEvent *e );

    virtual void mouseMoveEvent( QMouseEvent *e );


    virtual void dragEnterEvent( QDragEnterEvent *e );


    virtual void dragLeaveEvent( QDragLeaveEvent *e );


    virtual void dropEvent( QDropEvent *e );


    virtual void wheelEvent( QWheelEvent *event );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsfontbutton.h                                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
