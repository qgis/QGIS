/***************************************************************************
                          splashscreen.h  -  description
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
 *  Based on an example by Trolltech                                                                       *
 ***************************************************************************/
/* $Id$ */

#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <qpixmap.h>
#include <qwidget.h>
    
/**A splash screen to show on application startup. Based on code found at: http://doc.trolltech.com/qq/qq04-splashscreen.html
  *@author Tim Sutton
  */


class SplashScreen : public QWidget
{
  Q_OBJECT
public:
  SplashScreen();

  void setStatus( const QString &message, int alignment = AlignLeft, const QColor &color = Qt::black );
  void finish( QWidget *mainWin );
  void repaint();

protected:
  void mousePressEvent( QMouseEvent * );

private:
  QPixmap splashImage;
};

#endif
