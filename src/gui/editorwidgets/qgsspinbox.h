/***************************************************************************
    qgsspinbox.h
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

#ifndef QGSSPPINBOX_H
#define QGSSPPINBOX_H

#include <QSpinBox>
#include <QToolButton>

/**
 * @brief The QgsSpinBox is a spin box with a clear button that will set the value to the minimum. This minum can then be handled by a special value text.
 */
class GUI_EXPORT QgsSpinBox : public QSpinBox
{
    Q_OBJECT
    Q_PROPERTY( bool showClearButton READ showClearButton WRITE setShowClearButton )

  public:
    explicit QgsSpinBox( QWidget *parent = 0 );

    //! determines if the widget will show a clear button
    //! @note the clear button will set the widget to its minimum value
    void setShowClearButton( const bool showClearButton );
    bool showClearButton() const {return mShowClearButton;}

    //! Set the current value to the minimum
    virtual void clear();

  protected:
    virtual void resizeEvent( QResizeEvent* event );
    virtual void changeEvent( QEvent* event );

  private slots:
    void changed( const int& value );

  private:
    int frameWidth() const;

    bool mShowClearButton;

    QToolButton* mClearButton;
};

#endif // QGSSPPINBOX_H
