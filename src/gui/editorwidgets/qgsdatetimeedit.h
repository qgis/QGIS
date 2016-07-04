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

class QToolButton;
class QLineEdit;

/** \ingroup gui
 * @brief The QgsDateTimeEdit class is a QDateTimeEdit with the capability of setting/reading null date/times.
 */
class GUI_EXPORT QgsDateTimeEdit : public QDateTimeEdit
{
    Q_OBJECT
    Q_PROPERTY( bool allowNull READ allowNull WRITE setAllowNull )

  public:
    explicit QgsDateTimeEdit( QWidget *parent = nullptr );

    //! determines if the widget allows setting null date/time.
    void setAllowNull( bool allowNull );
    bool allowNull() const {return mAllowNull;}

    /**
     * @brief setDateTime set the date time in the widget and handles null date times.
     * @note since QDateTimeEdit::setDateTime() is not virtual, setDateTime must be called for QgsDateTimeEdit.
     */
    void setDateTime( const QDateTime &dateTime );

    /**
     * @brief dateTime returns the date time which can eventually be a null date/time
     * @note since QDateTimeEdit::dateTime() is not virtual, dateTime must be called for QgsDateTimeEdit.
     */
    QDateTime dateTime() const;

    //! Set the current date as NULL
    //! @note if the widget is not configured to accept NULL dates, this will have no effect
    virtual void clear() override;

    /** Resets the widget to show no value (ie, an "unknown" state).
     * @note added in QGIS 2.16
     */
    void setEmpty();

  protected:
    virtual void resizeEvent( QResizeEvent* event ) override;

    void mousePressEvent( QMouseEvent*event ) override;


  private slots:
    void changed( const QDateTime & dateTime );


  private:
    int spinButtonWidth() const;
    int frameWidth() const;

    bool mAllowNull;
    bool mIsNull;
    bool mIsEmpty;

    QLineEdit* mNullLabel;
    QToolButton* mClearButton;

};

#endif // QGSDATETIMEEDIT_H
