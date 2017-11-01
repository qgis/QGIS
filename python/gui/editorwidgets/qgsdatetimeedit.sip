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
%End

%TypeHeaderCode
#include "qgsdatetimeedit.h"
%End
  public:
    explicit QgsDateTimeEdit( QWidget *parent /TransferThis/ = 0 );

    void setAllowNull( bool allowNull );
%Docstring
Determines if the widget allows setting null date/time.
%End
    bool allowNull() const;
%Docstring
 :rtype: bool
%End

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
 :rtype: QDateTime
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

  protected:
    virtual void resizeEvent( QResizeEvent *event );

    virtual void mousePressEvent( QMouseEvent *event );



};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/editorwidgets/qgsdatetimeedit.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
