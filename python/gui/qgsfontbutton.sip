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

 The button will open a detailed text format settings dialog when clicked. An attached drop down
 menu allows for copying and pasting text styles, picking colors for the text, and for dropping
 colors from other color widgets.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsfontbutton.h"
%End
  public:

    QgsFontButton( QWidget *parent /TransferThis/ = 0, const QString &dialogTitle = QString() );
%Docstring
 Construct a new font button.
 Use ``parent`` to attach a parent QWidget to the dialog.
 Use ``dialogTitle`` string to define the title to show in the text settings dialog.
%End

    virtual QSize sizeHint() const;

    void setDialogTitle( const QString &title );
%Docstring
 Sets the ``title`` for the text settings dialog window.
.. seealso:: dialogTitle()
%End

    QString dialogTitle() const;
%Docstring
 Returns the title for the text settings dialog window.
.. seealso:: setDialogTitle()
 :rtype: str
%End

    QgsMapCanvas *mapCanvas() const;
%Docstring
 Returns the map canvas associated with the widget.
.. seealso:: setMapCanvas()
 :rtype: QgsMapCanvas
%End

    void setMapCanvas( QgsMapCanvas *canvas );
%Docstring
 Sets a map ``canvas`` to associate with the widget. This allows the
 widget to fetch current settings from the map canvas, such as current scale.
.. seealso:: mapCanvas()
%End

    QgsTextFormat textFormat() const;
%Docstring
 Returns the current text formatting set by the widget.
.. seealso:: setTextFormat()
 :rtype: QgsTextFormat
%End

  public slots:

    void setTextFormat( const QgsTextFormat &format );
%Docstring
 Sets the current text ``format`` to show in the widget.
.. seealso:: textFormat()
%End

    void setColor( const QColor &color );
%Docstring
 Sets the current ``color`` for the text. Will emit a changed signal if the color is different
 to the previous text color.
%End

    void copyFormat();
%Docstring
 Copies the current text format to the clipboard.
.. seealso:: pasteFormat()
%End

    void pasteFormat();
%Docstring
 Pastes a format from the clipboard. If clipboard does not contain a valid
 format then no change is applied.
.. seealso:: copyFormat()
%End

    void copyColor();
%Docstring
 Copies the current text color to the clipboard.
.. seealso:: pasteColor()
%End

    void pasteColor();
%Docstring
 Pastes a color from the clipboard to the text format. If clipboard does not contain a valid
 color or string representation of a color, then no change is applied.
.. seealso:: copyColor()
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


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsfontbutton.h                                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
