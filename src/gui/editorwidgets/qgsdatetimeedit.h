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
#include "qgis.h"
#include "qgis_gui.h"

class QToolButton;
class QLineEdit;

/**
 * \ingroup gui
 * \brief The QgsDateTimeEdit class is a QDateTimeEdit with the capability of setting/reading null date/times.
 */
class GUI_EXPORT QgsDateTimeEdit : public QDateTimeEdit
{
    Q_OBJECT
    Q_PROPERTY( bool allowNull READ allowNull WRITE setAllowNull )

  public:
    explicit QgsDateTimeEdit( QWidget *parent SIP_TRANSFERTHIS = 0 );

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
    virtual void clear() override;

    /**
     * Resets the widget to show no value (ie, an "unknown" state).
     * \since QGIS 2.16
     */
    void setEmpty();

  protected:
    virtual void resizeEvent( QResizeEvent *event ) override;

    void mousePressEvent( QMouseEvent *event ) override;


  private slots:
    void changed( const QDateTime &dateTime );


  private:
    int spinButtonWidth() const;
    int frameWidth() const;

    bool mAllowNull = true;
    bool mIsNull = true;
    bool mIsEmpty = false;

    QLineEdit *mNullLabel = nullptr;
    QToolButton *mClearButton = nullptr;

    /**
     * Set the lowest Date that can be displayed with the Qt::ISODate format
     *  - uses QDateTimeEdit::setMinimumDateTime (since Qt 4.4)
     * \note
     *  - QDate and QDateTime does not support minus years for the Qt::ISODate format
     *  -> returns empty (toString) or invalid (fromString) values
     *  - QDateTimeEdit::setMinimumDateTime does not support dates < '0100-01-01'
     *  -> it is not for us to wonder why [defined in qdatetimeparser_p.h]
    * \since QGIS 3.0
    * \note not available in Python bindings
    */
    void setMinimumEditDateTime()
    {
      setMinimumDateTime( QDateTime::fromString( QStringLiteral( "0100-01-01" ), Qt::ISODate ) );
    }

};

#endif // QGSDATETIMEEDIT_H
