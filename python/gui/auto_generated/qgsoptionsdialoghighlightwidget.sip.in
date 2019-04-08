/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsoptionsdialoghighlightwidget.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsOptionsDialogHighlightWidget : QObject
{
%Docstring
Container for a widget to be used to search text in the option dialog
If the widget type is handled, it is valid.
It can perform a text search in the widget and highlight it in case of success.
This uses stylesheets.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsoptionsdialoghighlightwidget.h"
%End
  public:

    static QgsOptionsDialogHighlightWidget *createWidget( QWidget *widget ) /Factory/;
%Docstring
create a highlight widget implementation for the proper widget type.
For instance a QgsOptionsDialogHighlightButton for button.

:return: a QgsOptionsDialogHighlightWidget or ``None`` if there is no implementation
         for the given widget.
%End

    bool isValid();
%Docstring
Returns if it valid: if the widget type is handled and if the widget is not still available
%End

    bool searchHighlight( const QString &text );
%Docstring
search for a text pattern and highlight the widget if the text is found

:return: ``True`` if the text pattern is found
%End

    QWidget *widget();
%Docstring
Returns the widget
%End


    virtual bool eventFilter( QObject *obj, QEvent *event );



  protected:

    virtual bool searchText( const QString &text ) = 0;
%Docstring
Search for the ``text`` in the widget and return ``True`` if it was found
%End

    virtual bool highlightText( const QString &text ) = 0;
%Docstring
Highlight the ``text`` in the widget.

:return: ``True`` if the text could be highlighted.
%End

    virtual void reset() = 0;
%Docstring
reset the style of the widgets to its original state
%End

    explicit QgsOptionsDialogHighlightWidget( QWidget *widget = 0 );
%Docstring
Constructor

:param widget: the widget used to search text into
%End


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsoptionsdialoghighlightwidget.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
