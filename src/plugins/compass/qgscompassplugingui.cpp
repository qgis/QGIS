/***************************************************************************
                          qgscompassplugingui.cpp
 Functions:
                             -------------------
    begin                : Jan 28, 2012
    copyright            : (C) 2012 by Marco Bernasocchi
    email                : marco@bernawebdesign.ch

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgisinterface.h"
//#include "qgscontexthelp.h"
#include "qgslogger.h"
#include <QPainter>

#include "qgscompassplugingui.h"
#include "compass.h"

QgsCompassPluginGui::QgsCompassPluginGui( QWidget * parent, Qt::WFlags fl )
    : QWidget( parent, fl )
{
  setupUi( this );

  compass = new Compass();

  if ( ! compass->isActive() )
  {
    this->mWarningLabel->setText( "<font color='red'>No compass detected</font>" );
  }

  QObject::connect( compass, SIGNAL( azimuthChanged( const QVariant&, const QVariant& ) ), this, SLOT( handleAzimuth( const QVariant&, const QVariant& ) ) );
}

QgsCompassPluginGui::~QgsCompassPluginGui()
{
}

void QgsCompassPluginGui::handleVisibilityChanged( bool visible )
{
  if ( visible )
  {
    compass->start();
  }
  else
  {
    compass->stop();
  }
}

void QgsCompassPluginGui::handleAzimuth( const QVariant &azimuth, const QVariant &calLevel )
{
  this->mAzimutDisplay->setText( QString( "%1" ).arg( azimuth.toInt() ) + QString::fromUtf8( "°" ) );

  //TODO check when https://sourceforge.net/p/necessitas/tickets/153/ is fixed
  qreal calibrationLevel = calLevel.toReal() / 3;
  if ( calibrationLevel == 1 )
  {
    this->mCalibrationLabel->setStyleSheet( "Background-color:green" );
  }
  else if ( calibrationLevel <= 1 / 3 )
  {
    this->mCalibrationLabel->setStyleSheet( "Background-color:red" );
    this->mWarningLabel->setText( "<font color='red'><a href='http://www.youtube.com/watch?v=oNJJPeoG8lQ'>Compass calibration</a> needed</font>" );
  }
  else
  {
    this->mCalibrationLabel->setStyleSheet( "Background-color:yellow" );
  }
  rotatePixmap( this->mArrowPixmapLabel, QString( ":/images/north_arrows/default.png" ), -azimuth.toInt() );
}

//Copied from QgsDecorationNorthArrowDialog adapted to be portable
void QgsCompassPluginGui::rotatePixmap( QLabel * pixmapLabel, QString myFileNameQString, int theRotationInt )
{
  QPixmap myQPixmap;
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
