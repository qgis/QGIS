/***************************************************************************
    qgsdatetimeedit.h
     --------------------------------------
    Date                 : 08.2014
    Copyright            : (C) 2014 Denis Rouzaud
    Email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDATETIMEEDIT_H
#define QGSDATETIMEEDIT_H

#include <QDateTimeEdit>
#include "qgis_sip.h"
#include "qgis_gui.h"

/**
 * \ingroup gui
 * \brief The QgsDateTimeEdit class is a QDateTimeEdit with the capability of setting/reading null date/times.
 *
 * \warning You should use the signal valueChanged of this subclass
 * rather than QDateTimeEdit::dateTimeChanged. If you consequently connect parent's
 * dateTimeChanged signal and call dateTime() afterwards there is no warranty to
 * have a proper NULL value handling.
 */
class GUI_EXPORT QgsDateTimeEdit : public QDateTimeEdit
{
    Q_OBJECT
    Q_PROPERTY( bool allowNull READ allowNull WRITE setAllowNull )

  public:

    //! Constructor for QgsDateTimeEdit
    explicit QgsDateTimeEdit( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    //! Determines if the widget allows setting null date/time.
    void setAllowNull( bool allowNull );
    bool allowNull() const {return mAllowNull;}

    /**
     * \brief setDateTime set the date time in the widget and handles null date times.
     * \note since QDateTimeEdit::setDateTime() is not virtual, setDateTime must be called for QgsDateTimeEdit.
     */
    void setDateTime( const QDateTime &dateTime );

    /**
     * \brief dateTime returns the date time which can eventually be a null date/time
     * \note since QDateTimeEdit::dateTime() is not virtual, dateTime must be called for QgsDateTimeEdit.
     */
    QDateTime dateTime() const;

    /**
     * Set the current date as NULL
     * \note if the widget is not configured to accept NULL dates, this will have no effect
     */
    void clear() override;

    /**
     * Resets the widget to show no value (ie, an "unknown" state).
     * \since QGIS 2.16
     */
    void setEmpty();

  signals:

    /**
     * signal emitted whenever the value changes.
     * \param date the new date/time value.
     */
    void valueChanged( const QDateTime &date );

  protected:
    void mousePressEvent( QMouseEvent *event ) override;
    void focusOutEvent( QFocusEvent *event ) override;
    void wheelEvent( QWheelEvent *event ) override;
    void showEvent( QShowEvent *event ) override;

  private slots:
    void changed( const QDateTime &dateTime );

  private:
    bool mAllowNull = true;
    bool mIsNull = false;
    bool mIsEmpty = false;
    bool mCurrentPressEvent = false;

    QString mOriginalStyleSheet = QString();
    QAction *mClearAction;

    void displayNull( bool updateCalendar = false );

    //! reset the value to current date time
    void resetBeforeChange( int delta );

    /**
     * Set the lowest Date that can be displayed with the Qt::ISODate format
     *  - uses QDateTimeEdit::setMinimumDateTime (since Qt 4.4)
     * \note
     *  - QDate and QDateTime does not support minus years for the Qt::ISODate format
     *  -> returns empty (toString) or invalid (fromString) values
     *  - QDateTimeEdit::setMinimumDateTime does not support dates < '0100-01-01'
     *  -> it is not for us to wonder why [defined in qdatetimeparser_p.h]
    * \note not available in Python bindings
    * \since QGIS 3.0
    */
    void setMinimumEditDateTime()
    {
      setMinimumDateTime( QDateTime::fromString( QStringLiteral( "0100-01-01" ), Qt::ISODate ) );
    }
};

#endif // QGSDATETIMEEDIT_H
