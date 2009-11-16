/***************************************************************************
    qgscolorbutton.h - Color button
     --------------------------------------
    Date                 : 12-Dec-2006
    Copyright            : (C) 2006 by Tom Elwertowski
    Email                : telwertowski at users dot sourceforge dot net
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id: qgscolorbutton.h 6251 2006-12-13 23:23:50Z telwertowski $ */
#ifndef QGSCOLORBUTTON_H
#define QGSCOLORBUTTON_H

#include <QToolButton>
#include <QPushButton>

/** \ingroup gui
 * A cross platform button subclass for selecting colors.
 */
class GUI_EXPORT QgsColorButton: public QToolButton
{
  public:
    QgsColorButton( QWidget *parent = 0 );
    ~QgsColorButton();

    void setColor( const QColor &color );
    QColor color() const { return mColor; }

  protected:
    void paintEvent( QPaintEvent *e );

  private:
    QColor mColor;
};


class QgsColorButtonV2 : public QPushButton
{
  public:
    QgsColorButtonV2( QWidget* parent = 0 );

    void setColor( const QColor &color );
    QColor color() const { return mColor; }

  private:
    QColor mColor;
};


#endif
