/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsfocuswatcher.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsFocusWatcher : QObject
{
%Docstring
A event filter for watching for focus events on a parent object. Usually QObjects must
subclass and override methods like focusOutEvent to handle focus events. Using this class
as an event filter avoids the need to subclass objects and the focus events can be directly
caught using the emitted signals.

.. versionadded:: 2.16
%End

%TypeHeaderCode
#include "qgsfocuswatcher.h"
%End
  public:

    explicit QgsFocusWatcher( QObject *parent /TransferThis/ );
%Docstring
Constructor for QgsFocusWatcher.

:param parent: parent widget to catch focus events for. This class will automatically be
               installed as an event filter for parent.
%End

    virtual bool eventFilter( QObject *obj, QEvent *event );


  signals:

    void focusChanged( bool focused );
%Docstring
Emitted when parent object's focus changes.

:param focused: ``True`` if object gained focus, ``False`` if object lost focus
%End

    void focusIn();
%Docstring
Emitted when parent object gains focus.
%End

    void focusOut();
%Docstring
Emitted when parent object loses focus.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsfocuswatcher.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
