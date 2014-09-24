/***************************************************************************
                              qgsfilterlineedit.h
                              ------------------------
  begin                : October 27, 2012
  copyright            : (C) 2012 by Alexander Bruy
  email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFILTERLINEEDIT_H
#define QGSFILTERLINEEDIT_H

#include <QLineEdit>

class QToolButton;

/** \ingroup gui
 * Lineedit with builtin clear button
 **/
class GUI_EXPORT QgsFilterLineEdit : public QLineEdit
{
    Q_OBJECT
    Q_PROPERTY( QString nullValue READ nullValue WRITE setNullValue )

  public:
    QgsFilterLineEdit( QWidget* parent = 0, QString nullValue = QString::null );

    void setNullValue( QString nullValue ) { mNullValue = nullValue; }

    QString nullValue() const { return mNullValue; }

    /**
     * Sets the current text with NULL support
     *
     * @param value The text to set. If a Null string is provided, the text will match the nullValue.
     */
    void setValue( QString value ) { setText( value.isNull() ? mNullValue : value ); }

    /**
     * Returns the text of this edit with NULL support
     *
     * @return Current text (Null string if it matches the nullValue property )
     */
    QString value() const { return isNull() ? QString::null : text(); }

    /**
     * Determine if the current text represents Null.
     *
     * @return True if the value is Null.
     */
    inline bool isNull() const { return text() == mNullValue; }

  signals:
    void cleared();

    /**
     * Same as textChanged(const QString& ) but with support for Null values.
     *
     * @param value The current text or Null string if it matches the nullValue property.
     */
    void valueChanged( const QString& value );

  protected:
    void mousePressEvent( QMouseEvent* e );
    void focusInEvent( QFocusEvent* e );
    void resizeEvent( QResizeEvent* e );
    void changeEvent( QEvent* e );
    void paintEvent( QPaintEvent* e );

  private slots:
    void clear();
    void onTextChanged( const QString &text );

  private:
    QString mNullValue;
    QToolButton *btnClear;
    QString mStyleSheet;
    bool mFocusInEvent;
};

#endif // QGSFILTERLINEEDIT_H
