/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/characterwidget.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class CharacterWidget : QWidget
{
%Docstring

A widget for displaying characters available in a preset font, and allowing
users to select an individual character.
%End

%TypeHeaderCode
#include "characterwidget.h"
%End
  public:

    CharacterWidget( QWidget *parent /TransferThis/ = 0 );
%Docstring
Constructor for CharacterWidget.
%End

    virtual QSize sizeHint() const;


    int columns() const;
%Docstring
Returns the number of columns of characters shown in the widget.
%End

    int squareSize() const;
%Docstring
Returns the size (in pixels) of the square used to render each character preview.
%End

    QChar character() const;
%Docstring
Returns the currently selected character in the widget.

.. seealso:: :py:func:`setCharacter`

.. versionadded:: 3.0
%End

    QFont font() const;
%Docstring
Returns the font shown in the widget

.. seealso:: :py:func:`setFont`

.. versionadded:: 3.0
%End

  public slots:

    void setFont( const QFont &font );
%Docstring
Sets the ``font`` to show in the widget.

.. seealso:: :py:func:`font`

.. versionadded:: 3.0
%End

    void setFontSize( double fontSize );
%Docstring
Sets the font size (in points) to render in the widget.

.. versionadded:: 3.0
%End

    void setFontStyle( const QString &fontStyle );
%Docstring
Sets the font style to show in the widget.

.. versionadded:: 3.0
%End

    void updateFontMerging( bool enable );

    void setColumns( int columns );
%Docstring
Sets the number of columns of characters to show in the widget.

.. versionadded:: 3.0
%End

    void setCharacter( QChar character );
%Docstring
Sets the currently selected ``character`` in the widget.

.. seealso:: :py:func:`character`

.. seealso:: :py:func:`characterSelected`
%End

  signals:

    void characterSelected( QChar character );
%Docstring
Emitted when a character is selected in the widget.
%End

  protected:
    virtual void mouseMoveEvent( QMouseEvent *event );

    virtual void mousePressEvent( QMouseEvent *event );

    virtual void paintEvent( QPaintEvent *event );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/characterwidget.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
