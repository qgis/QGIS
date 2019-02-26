/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsmessagebar.h                                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsMessageBar: QFrame
{
%Docstring
A bar for displaying non-blocking messages to the user.
%End

%TypeHeaderCode
#include "qgsmessagebar.h"
%End
  public:

    QgsMessageBar( QWidget *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsMessageBar
%End

    void pushItem( QgsMessageBarItem *item /Transfer/ );
%Docstring
Display a message item on the bar after hiding the currently visible one
and putting it in a stack.
The message bar will take ownership of the item.

:param item: Item to display
%End

    QgsMessageBarItem *pushWidget( QWidget *widget /Transfer/, Qgis::MessageLevel level = Qgis::Info, int duration = 0 );
%Docstring
Display a widget as a message on the bar after hiding the currently visible one
and putting it in a stack.

:param widget: message widget to display
:param level: is Qgis.Info, Warning, Critical or Success
:param duration: timeout duration of message in seconds, 0 value indicates no timeout
%End

    bool popWidget( QgsMessageBarItem *item );
%Docstring
Remove the passed widget from the bar (if previously added),
then display the next one in the stack if any or hide the bar

:param item: item to remove

:return: ``True`` if the widget was removed, ``False`` otherwise
%End

    static QgsMessageBarItem *createMessage( const QString &text, QWidget *parent = 0 ) /Factory/;
%Docstring
make out a widget containing a message to be displayed on the bar
%End
    static QgsMessageBarItem *createMessage( const QString &title, const QString &text, QWidget *parent = 0 ) /Factory/;
%Docstring
make out a widget containing title and message to be displayed on the bar
%End
    static QgsMessageBarItem *createMessage( QWidget *widget, QWidget *parent = 0 ) /Factory/;
%Docstring
make out a widget containing title and message to be displayed on the bar
%End

    void pushMessage( const QString &text, Qgis::MessageLevel level = Qgis::Info, int duration = 5 );
%Docstring
convenience method for pushing a message to the bar
%End
    void pushMessage( const QString &title, const QString &text, Qgis::MessageLevel level = Qgis::Info, int duration = 5 );
%Docstring
convenience method for pushing a message with title to the bar
%End

    void pushMessage( const QString &title, const QString &text, const QString &showMore, Qgis::MessageLevel level = Qgis::Info, int duration = 5 );
%Docstring
convenience method for pushing a message to the bar with a detail text which be shown when pressing a "more" button
%End

    QgsMessageBarItem *currentItem();

  signals:
    void widgetAdded( QgsMessageBarItem *item );
%Docstring
emitted when a message widget is added to the bar
%End

    void widgetRemoved( QgsMessageBarItem *item );
%Docstring
emitted when a message widget was removed from the bar
%End

  public slots:

    bool popWidget();
%Docstring
Remove the currently displayed widget from the bar and
display the next in the stack if any or hide the bar.

:return: ``True`` if the widget was removed, ``False`` otherwise
%End

    bool clearWidgets();
%Docstring
Remove all items from the bar's widget list

:return: ``True`` if all items were removed, ``False`` otherwise
%End

    void pushSuccess( const QString &title, const QString &message );
%Docstring
Pushes a success message with default timeout to the message bar

:param title: title string for message
:param message: The message to be displayed

.. versionadded:: 2.8
%End

    void pushInfo( const QString &title, const QString &message );
%Docstring
Pushes a information message with default timeout to the message bar

:param title: title string for message
:param message: The message to be displayed

.. versionadded:: 2.8
%End

    void pushWarning( const QString &title, const QString &message );
%Docstring
Pushes a warning with default timeout to the message bar

:param title: title string for message
:param message: The message to be displayed

.. versionadded:: 2.8
%End

    void pushCritical( const QString &title, const QString &message );
%Docstring
Pushes a critical warning with default timeout to the message bar

:param title: title string for message
:param message: The message to be displayed

.. versionadded:: 2.8
%End

  protected:
    virtual void mousePressEvent( QMouseEvent *e );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsmessagebar.h                                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
