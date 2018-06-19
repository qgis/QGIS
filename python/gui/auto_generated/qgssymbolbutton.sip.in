/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgssymbolbutton.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsSymbolButton : QToolButton
{
%Docstring
A button for creating and modifying QgsSymbol settings.

The button shows a preview icon for the current symbol, and will open a detailed symbol editor dialog (or
panel widget) when clicked.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgssymbolbutton.h"
%End
  public:

    QgsSymbolButton( QWidget *parent /TransferThis/ = 0, const QString &dialogTitle = QString() );
%Docstring
Construct a new symbol button.
Use ``dialogTitle`` string to define the title to show in the symbol settings dialog.
%End

    virtual QSize minimumSizeHint() const;

    virtual QSize sizeHint() const;


    void setSymbolType( QgsSymbol::SymbolType type );
%Docstring
Sets the symbol ``type`` which the button requires.
If the type differs from the current symbol type, the symbol will be reset
to a default symbol style of the new type.

.. seealso:: :py:func:`symbolType`
%End

    QgsSymbol::SymbolType symbolType() const;
%Docstring
Returns the symbol type which the button requires.

.. seealso:: :py:func:`setSymbolType`
%End

    void setDialogTitle( const QString &title );
%Docstring
Sets the ``title`` for the symbol settings dialog window.

.. seealso:: :py:func:`dialogTitle`
%End

    QString dialogTitle() const;
%Docstring
Returns the title for the symbol settings dialog window.

.. seealso:: :py:func:`setDialogTitle`
%End

    QgsSymbol *symbol();
%Docstring
Returns the current symbol defined by the button.

.. seealso:: :py:func:`setSymbol`

.. seealso:: :py:func:`changed`
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

    QgsVectorLayer *layer() const;
%Docstring
Returns the layer associated with the widget.

.. seealso:: :py:func:`setLayer`
%End

    void setLayer( QgsVectorLayer *layer );
%Docstring
Sets a ``layer`` to associate with the widget. This allows the
widget to setup layer related settings within the symbol settings dialog,
such as correctly populating data defined override buttons.

.. seealso:: :py:func:`layer`
%End

    void registerExpressionContextGenerator( QgsExpressionContextGenerator *generator );
%Docstring
Register an expression context generator class that will be used to retrieve
an expression context for the button when required.
%End

  public slots:

    void setSymbol( QgsSymbol *symbol /Transfer/ );
%Docstring
Sets the ``symbol`` for the button. Ownership of ``symbol`` is transferred to the
button.

.. seealso:: :py:func:`symbol`

.. seealso:: :py:func:`changed`
%End

    void setColor( const QColor &color );
%Docstring
Sets the current ``color`` for the symbol. Will emit a changed() signal if the color is different
to the previous symbol color.
%End

    void copySymbol();
%Docstring
Copies the current symbol to the clipboard.

.. seealso:: :py:func:`pasteSymbol`
%End

    void pasteSymbol();
%Docstring
Pastes a symbol from the clipboard. If clipboard does not contain a valid
symbol then no change is applied.

.. seealso:: :py:func:`copySymbol`
%End

    void copyColor();
%Docstring
Copies the current symbol color to the clipboard.

.. seealso:: :py:func:`pasteColor`
%End

    void pasteColor();
%Docstring
Pastes a color from the clipboard to the symbol. If clipboard does not contain a valid
color or string representation of a color, then no change is applied.

.. seealso:: :py:func:`copyColor`
%End

  signals:

    void changed();
%Docstring
Emitted when the symbol's settings are changed.

.. seealso:: :py:func:`symbol`

.. seealso:: :py:func:`setSymbol`
%End

  protected:

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
 * src/gui/qgssymbolbutton.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
