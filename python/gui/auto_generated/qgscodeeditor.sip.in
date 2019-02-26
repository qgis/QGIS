/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgscodeeditor.h                                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsCodeEditor : QsciScintilla
{
%Docstring
A text editor based on QScintilla2.

.. note::

   may not be available in Python bindings, depending on platform support

.. versionadded:: 2.6
%End

%TypeHeaderCode
#include "qgscodeeditor.h"
%End
  public:

    QgsCodeEditor( QWidget *parent /TransferThis/ = 0, const QString &title = QString(), bool folding = false, bool margin = false );
%Docstring
Construct a new code editor.

:param parent: The parent QWidget
:param title: The title to show in the code editor dialog
:param folding: ``False``: Enable folding for code editor
:param margin: ``False``: Enable margin for code editor

.. versionadded:: 2.6
%End

    void setTitle( const QString &title );
%Docstring
Set the widget title

:param title: widget title
%End

    void setMarginVisible( bool margin );
%Docstring
Set margin visible state

:param margin: Set margin in the editor
%End
    bool marginVisible();

    void setFoldingVisible( bool folding );
%Docstring
Set folding visible state

:param folding: Set folding in the editor
%End
    bool foldingVisible();

    void insertText( const QString &text );
%Docstring
Insert text at cursor position, or replace any selected text if user has
made a selection.

:param text: The text to be inserted
%End

  protected:

    bool isFixedPitch( const QFont &font );

    virtual void focusOutEvent( QFocusEvent *event );

    virtual void keyPressEvent( QKeyEvent *event );


    QFont getMonospaceFont();

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgscodeeditor.h                                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
