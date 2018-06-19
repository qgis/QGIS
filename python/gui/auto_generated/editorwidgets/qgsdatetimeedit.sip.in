/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/editorwidgets/qgsdatetimeedit.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsDateTimeEdit : QDateTimeEdit
{
%Docstring
The QgsDateTimeEdit class is a QDateTimeEdit with the capability of setting/reading null date/times.

.. warning::

   You should use the signal valueChanged of this subclass
   rather than QDateTimeEdit.dateTimeChanged. If you consequently connect parent's
   dateTimeChanged signal and call dateTime() afterwards there is no warranty to
   have a proper NULL value handling.
%End

%TypeHeaderCode
#include "qgsdatetimeedit.h"
%End
  public:

    explicit QgsDateTimeEdit( QWidget *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsDateTimeEdit
%End

    void setAllowNull( bool allowNull );
%Docstring
Determines if the widget allows setting null date/time.
%End
    bool allowNull() const;

    void setDateTime( const QDateTime &dateTime );
%Docstring
setDateTime set the date time in the widget and handles null date times.

.. note::

   since QDateTimeEdit.setDateTime() is not virtual, setDateTime must be called for QgsDateTimeEdit.
%End

    QDateTime dateTime() const;
%Docstring
dateTime returns the date time which can eventually be a null date/time

.. note::

   since QDateTimeEdit.dateTime() is not virtual, dateTime must be called for QgsDateTimeEdit.
%End

    virtual void clear();

%Docstring
Set the current date as NULL

.. note::

   if the widget is not configured to accept NULL dates, this will have no effect
%End

    void setEmpty();
%Docstring
Resets the widget to show no value (ie, an "unknown" state).

.. versionadded:: 2.16
%End

  signals:

    void valueChanged( const QDateTime &date );
%Docstring
signal emitted whenever the value changes.

:param date: the new date/time value.
%End

  protected:
    virtual void mousePressEvent( QMouseEvent *event );

    virtual void focusOutEvent( QFocusEvent *event );

    virtual void wheelEvent( QWheelEvent *event );

    virtual void showEvent( QShowEvent *event );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/editorwidgets/qgsdatetimeedit.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
