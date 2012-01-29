/***************************************************************************
 *   Copyright (C) 2012 by Marco Bernasocchi                               *
 *   marco at bernawebdesign.ch                                            *
 *                                                                         *
 *   GUI for showing a QtSensors compass reading                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "qgscompassplugingui.h"

#include "qgisinterface.h"
#include "qgscontexthelp.h"
#include "qgslogger.h"
#include <QPainter>

QgsCompassPluginGui::QgsCompassPluginGui( QWidget * parent, Qt::WFlags fl )
    : QWidget( parent, fl )
{
  setupUi( this );
  mTimer = new QTimer(this);
  connect(mTimer, SIGNAL(timeout()), this, SLOT(updateReading()));
  mTimer->start(1000);
}

QgsCompassPluginGui::~QgsCompassPluginGui()
{
  mTimer->stop();
}

void QgsCompassPluginGui::updateReading()
{
  int azimut = mCompass.azimuth();
  this->mAzimutDisplay->setText( QString::number( azimut ) );
  this->mCalibrationDisplay->setText( QString::number( mCompass.calibrationLevel() ) );
  rotatePixmap( qrand() );
  //rotatePixmap( azimut );
}

//Copied from QgsDecorationNorthArrowDialog
void QgsCompassPluginGui::rotatePixmap( int theRotationInt )
{
  QPixmap myQPixmap;
  QString myFileNameQString = ":/images/north_arrows/default.png";
// QgsDebugMsg(QString("Trying to load %1").arg(myFileNameQString));
  if ( myQPixmap.load( myFileNameQString ) )
  {
    QPixmap  myPainterPixmap( myQPixmap.height(), myQPixmap.width() );
    myPainterPixmap.fill();
    QPainter myQPainter;
    myQPainter.begin( &myPainterPixmap );

    myQPainter.setRenderHint( QPainter::SmoothPixmapTransform );

    double centerXDouble = myQPixmap.width() / 2;
    double centerYDouble = myQPixmap.height() / 2;
    //save the current canvas rotation
    myQPainter.save();
    //myQPainter.translate( (int)centerXDouble, (int)centerYDouble );

    //rotate the canvas
    myQPainter.rotate( theRotationInt );
    //work out how to shift the image so that it appears in the center of the canvas
    //(x cos a + y sin a - x, -x sin a + y cos a - y)
    const double PI = 3.14159265358979323846;
    double myRadiansDouble = ( PI / 180 ) * theRotationInt;
    int xShift = static_cast<int>((
                                    ( centerXDouble * cos( myRadiansDouble ) ) +
                                    ( centerYDouble * sin( myRadiansDouble ) )
                                  ) - centerXDouble );
    int yShift = static_cast<int>((
                                    ( -centerXDouble * sin( myRadiansDouble ) ) +
                                    ( centerYDouble * cos( myRadiansDouble ) )
                                  ) - centerYDouble );

    //draw the pixmap in the proper position
    myQPainter.drawPixmap( xShift, yShift, myQPixmap );

    //unrotate the canvas again
    myQPainter.restore();
    myQPainter.end();

    pixmapLabel->setPixmap( myPainterPixmap );
  }
  else
  {
    QPixmap  myPainterPixmap( 200, 200 );
    myPainterPixmap.fill();
    QPainter myQPainter;
    myQPainter.begin( &myPainterPixmap );
    QFont myQFont( "time", 12, QFont::Bold );
    myQPainter.setFont( myQFont );
    myQPainter.setPen( Qt::red );
    myQPainter.drawText( 10, 20, tr( "Pixmap not found" ) );
    myQPainter.end();
    pixmapLabel->setPixmap( myPainterPixmap );
  }
}
