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
 * rather than QDateTimeEdit::dateTimeChanged. (If you consequently connect parent's
 * dateTimeChanged signal and call dateTime() afterwards there is no guarantee that
 * NULL values will be correctly handled).
 *
 * \see QgsDateEdit
 * \see QgsTimeEdit
 *
 */
class GUI_EXPORT QgsDateTimeEdit : public QDateTimeEdit
{
    Q_OBJECT
    Q_PROPERTY( bool allowNull READ allowNull WRITE setAllowNull )

  public:

    /**
     * Constructor for QgsDateTimeEdit.
     * The current date and time is used by default.
     * The widget is allowing null by default.
     * If allow null is disabled, you should check allowNull before getting values from the widget.
     */
    explicit QgsDateTimeEdit( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Determines if the widget allows setting null date/time.
     * \see allowNull
     */
    void setAllowNull( bool allowNull );

    /**
     * If the widget allows setting null date/time.
     * \see setAllowNull
     */
    bool allowNull() const {return mAllowNull;}

    /**
     * \brief Set the date time in the widget and handles null date times.
     * \note Since QDateTimeEdit::setDateTime() is not virtual, setDateTime must be called for QgsDateTimeEdit.
     */
    void setDateTime( const QDateTime &dateTime );

    /**
     * \brief Returns the date time which can be a null date/time.
     * \note Before QGIS 3.10, you mustn't call date() or time() because they can't return a NULL value.
     * \note Since QDateTimeEdit::dateTime() is not virtual, dateTime must be called for QgsDateTimeEdit.
     */
    QDateTime dateTime() const;

    /**
     * \brief Returns the time which can be a null time.
     * \since QGIS 3.10
     */
    QTime time() const;

    /**
     * \brief Returns the date which can be a null date.
     * \since QGIS 3.10
     */
    QDate date() const;

    /**
     * Set the current date as NULL.
     * \note If the widget is not configured to accept NULL dates, this will have no effect.
     */
    void clear() override;

    /**
     * Resets the widget to show no value (ie, an "unknown" state).
     * \since QGIS 2.16
     */
    void setEmpty();

    /**
     * Returns the widget's NULL representation, which defaults
     * to QgsApplication::nullRepresentation().
     *
     * \see setNullRepresentation()
     * \since QGIS 3.14
     */
    QString nullRepresentation() const;

    /**
     * Sets the widget's \a null representation, which defaults
     * to QgsApplication::nullRepresentation().
     *
     * \see nullRepresentation()
     * \since QGIS 3.14
     */
    void setNullRepresentation( const QString &null );

  signals:

    /**
     * Signal emitted whenever the value changes.
     * \param date The new date/time value.
     */
    void valueChanged( const QDateTime &date );

  protected:
    void mousePressEvent( QMouseEvent *event ) override;
    void focusOutEvent( QFocusEvent *event ) override;
    void focusInEvent( QFocusEvent *event ) override;
    void wheelEvent( QWheelEvent *event ) override;
    void showEvent( QShowEvent *event ) override;

#ifndef SIP_RUN
///@cond PRIVATE
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QgsDateTimeEdit( const QVariant &var, QVariant::Type parserType, QWidget *parent );
#else
    QgsDateTimeEdit( const QVariant &var, QMetaType::Type parserType, QWidget *parent );
#endif
///@endcond
#endif

    //! TRUE if the widget is empty
    bool mIsEmpty = false;

    //! Block change signals if TRUE
    int mBlockChangedSignal = 0;

    /**
    * write the null value representation to the line edit without changing the value
    * \param updateCalendar Flag if calendar is open and minimum date needs to be set
    */
    void displayNull( bool updateCalendar = false );

    /**
     * Emits the widget's correct value changed signal.
     */
    virtual void emitValueChanged( const QVariant &value );

    /**
     * Returns TRUE if the widget is currently set to a null value
     */
    bool isNull() const;

  protected slots:
#ifndef SIP_RUN
    ///@cond PRIVATE
    void changed( const QVariant &dateTime );
    ///@endcond
#endif


  private:
    bool mCurrentPressEvent = false;

    QString mOriginalStyleSheet = QString();
    QAction *mClearAction;
    QString mNullRepresentation;

    //! TRUE if the widget allows null values
    bool mAllowNull = true;

    //! TRUE if the widget is currently set to a null value
    bool mIsNull = false;

    /**
    * write the current date into the line edit without changing the value
    */
    void displayCurrentDate();

    //! reset the value to current date time
    void resetBeforeChange( int delta );

    /**
     * Set the lowest Date that can be stored in a Shapefile or Geopackage Date field
     *
     * - uses QDateTimeEdit::setDateTimeRange (since Qt 4.4)
     *
     * \note
     *
     * - QDate and QDateTime does not support minus years for the Qt::ISODate format
     *   -> returns empty (toString) or invalid (fromString) values
    *
    * \note not available in Python bindings
    * \since QGIS 3.0
    */
    void setMinimumEditDateTime();

    friend class TestQgsDateTimeEdit;
};


/**
 * \ingroup gui
 * \brief The QgsTimeEdit class is a QTimeEdit widget with the capability of setting/reading null date/times.
 *
 * \warning You should use the signal valueChanged of this subclass
 * rather than QDateTimeEdit::timeChanged. (If you consequently connect parent's
 * timeChanged signal and call time() afterwards there is no guarantee that
 * NULL values will be correctly handled).
 *
 * \see QgsDateTimeEdit
 * \see QgsDateEdit
 *
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsTimeEdit : public QgsDateTimeEdit
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsTimeEdit.
     * The current time is used by default.
     * The widget is allowing null by default.
     * If allow null is disabled, you should check allowNull before getting values from the widget.
     */
    explicit QgsTimeEdit( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets the \a time for the widget and handles null times.
     * \note Since QDateTimeEdit::setTime() is not virtual, setTime must be called for QgsTimeEdit.
     */
    void setTime( const QTime &time );

  signals:

    /**
     * Signal emitted whenever the time changes.
     */
    void timeValueChanged( const QTime &time );

  protected:
    void emitValueChanged( const QVariant &value ) override;

};

/**
 * \ingroup gui
 * \brief The QgsDateEdit class is a QDateEdit widget with the capability of setting/reading null dates.
 *
 * \warning You should use the signal valueChanged of this subclass
 * rather than QDateTimeEdit::dateChanged. (If you consequently connect parent's
 * dateChanged signal and call date() afterwards there is no guarantee that
 * NULL values will be correctly handled).
 *
 * \see QgsDateTimeEdit
 * \see QgsTimeEdit
 *
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsDateEdit : public QgsDateTimeEdit
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsDateEdit.
     * The current time is used by default.
     * The widget is allowing null by default.
     * If allow null is disabled, you should check allowNull before getting values from the widget.
     */
    explicit QgsDateEdit( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets the \a date for the widget and handles null dates.
     * \note Since QDateTimeEdit::setDate() is not virtual, setDate must be called for QgsDateEdit.
     */
    void setDate( const QDate &date );

  signals:

    /**
     * Signal emitted whenever the date changes.
     */
    void dateValueChanged( const QDate &date );

  protected:
    void emitValueChanged( const QVariant &value ) override;

};

#endif // QGSDATETIMEEDIT_H
