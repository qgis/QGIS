/***************************************************************************
                          splashscreen.cpp  -  description
                             -------------------
    begin                : Sat May 17 2003
    copyright            : (C) 2003 by Tim Sutton
    email                : tim@linfiniti.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *    Based on an example provided by Trolltech                                                                     *
 ***************************************************************************/
/* $Id$ */
#include <qapplication.h>
#include <qpainter.h>
#include <qpixmap.h>
#include "splashscreen.h"
#include "qfont.h"
#include "qgis.h"
//splashscreen image
#include "xpm/splash.xpm"

SplashScreen::SplashScreen():QWidget(0, 0, WStyle_Customize | WStyle_Splash), splashImage((const char **) splash_xpm)
{
  setErasePixmap(splashImage);
  resize(splashImage.size());
  QRect scr = QApplication::desktop()->screenGeometry();
  move(scr.center() - rect().center());

  QPainter painter(&splashImage);
  painter.setPen(Qt::red);
  QFont myQFont("arial", 36, QFont::Bold);
  painter.setFont(myQFont);
  painter.drawText(20, 50, VERSION);
  repaint();

  show();
}

void SplashScreen::repaint()
{
  QWidget::repaint();
  QApplication::flush();
}

#if defined(Q_WS_X11)
void qt_wait_for_window_manager(QWidget * widget);
#endif

void SplashScreen::finish(QWidget * mainWin)
{
#if defined(Q_WS_X11)
  qt_wait_for_window_manager(mainWin);
#endif
  close();
}

void SplashScreen::mousePressEvent(QMouseEvent *)
{
  hide();
}

void SplashScreen::setStatus(const QString & message, int alignment, const QColor & color)
{
  QPixmap textPix = splashImage;
  QPainter painter(&textPix, this);
  painter.setPen(color);
  QFont myQFont("arial", 18, QFont::Bold);
  painter.setFont(myQFont);
  QRect r = rect();
  r.setRect(r.x() + 50, r.y() + 250, r.width() - 20, r.height() - 20);
  painter.drawText(r, alignment, message);
  setErasePixmap(textPix);
  repaint();
}
