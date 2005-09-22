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
#include <qglobal.h>
#include "splashscreen.h"
#include "qfont.h"
#include "qgis.h"
#include "qbitmap.h"
#if defined(WIN32) || defined(Q_OS_MACX)
QString PKGDATAPATH = qApp->applicationDirPath() + "/share/qgis";
#endif
#define STATUS_TEXT_X 90
#define STATUS_TEXT_Y 20
SplashScreen::SplashScreen():QWidget(0, 0, WStyle_Customize | WStyle_Splash)
{
  //set up masking
  splashImage.load(QString(PKGDATAPATH) + QString("/images/splash/splash.png"), 0, Qt::ThresholdDither |   Qt::AvoidDither );
  resize(splashImage.size());
  //
  // NOTES! the mask must be a 1 BIT IMAGE or it wont work!
  //
#if QT_VERSION < 0x040000
  QPixmap myMaskPixmap( 564, 300, -1, QPixmap::BestOptim );
#else
  // TODO: Confirm this is all we need under Qt4
  QPixmap myMaskPixmap( 564, 300 );
#endif
  myMaskPixmap.load( QString(PKGDATAPATH) + QString("/images/splash/splash_mask.png"), 0, Qt::ThresholdDither |   Qt::ThresholdAlphaDither | Qt::AvoidDither );
  setBackgroundPixmap(splashImage);
  setMask( myMaskPixmap.createHeuristicMask() );


  setErasePixmap(splashImage);
  
  QRect scr = QApplication::desktop()->screenGeometry();
  move(scr.center() - rect().center());

  QPainter painter(&splashImage);
  painter.setPen(Qt::black);
  QFont myQFont("arial", 18, QFont::Bold);
  painter.setFont(myQFont);

  QString myCaption = tr("Version ");
  // myCaption += QString("%1 ('%2')").arg(QGis::qgisVersion).arg(QGis::qgisReleaseName);
  myCaption += QString("%1").arg(QGis::qgisVersion);

  painter.drawText(250, 50, myCaption);
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
// TODO: Qt4 (and Qt3.3 for that matter) has a dedicated QSplashScreen class;
// this class will need to be refactored to use QSplashScreen and therefore
// avoid the following hack.
#if QT_VERSION < 0x040000
  qt_wait_for_window_manager(mainWin);
#endif
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

  // TODO: Confirm that removing the "const QWidget * copyAttributes" 2nd parameter,
  // in order to make things work in Qt4, doesn't break things in Qt3.
  //QPainter painter(&textPix, this);
  QPainter painter(&textPix);

  painter.setPen(color);
  QFont myQFont("arial", 10, QFont::Bold);
  painter.setFont(myQFont);
  painter.drawText(STATUS_TEXT_X, textPix.height() - STATUS_TEXT_Y, message);
  setErasePixmap(textPix);
  repaint();
}
