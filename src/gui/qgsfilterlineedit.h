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

    QString nullValue() const {return mNullValue;}

  signals:
    void cleared();

  protected:
    void resizeEvent( QResizeEvent * );
    void changeEvent( QEvent * );

  private slots:
    void clear();
    void toggleClearButton( const QString &text );

  private:
    QString mNullValue;
    QToolButton *btnClear;
};

#endif // QGSFILTERLINEEDIT_H
