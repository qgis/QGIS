/***************************************************************************
    qgsdoublespinbox.h
     --------------------------------------
    Date                 : 09.2014
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

#ifndef QGSDOUBLESPPINBOX_H
#define QGSDOUBLESPPINBOX_H

#include <QDoubleSpinBox>
#include <QToolButton>

/**
 * @brief The QgsSpinBox is a spin box with a clear button that will set the value to the minimum. This minum can then be handled by a special value text.
 */
class GUI_EXPORT QgsDoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT
    Q_PROPERTY( bool allowNull READ allowNull WRITE setAllowNull )

  public:
    explicit QgsDoubleSpinBox( QWidget *parent = 0 );

    //! determines if the widget allows setting null date/time.
    void setAllowNull( bool allowNull );
    bool allowNull() const {return mAllowNull;}


    //! Set the current value to the minimum
    //! @note if the widget is not configured to accept NULL values, this will have no effect
    virtual void clear();

  protected:
    virtual void resizeEvent( QResizeEvent* event );

  private:
    int spinButtonWidth() const;
    int frameWidth() const;

    bool mAllowNull;

    QToolButton* mClearButton;
};

#endif // QGSDOUBLESPPINBOX_H
