/***************************************************************************
 *   Copyright (C) 2003 by Tim Sutton                                      *
 *   tim@linfiniti.com                                                     *
 *                                                                         *
 *   This is a plugin generated from the QGIS plugin template              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "plugingui.h"
#include "qgsapplication.h"
#include "qgscontexthelp.h"

#include <QPainter>
#include <cmath>
#include "qgslogger.h"


QgsNorthArrowPluginGui::QgsNorthArrowPluginGui( QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );
}

QgsNorthArrowPluginGui::~QgsNorthArrowPluginGui()
{
}

void QgsNorthArrowPluginGui::on_buttonBox_accepted()
{
  // Hide the dialog
  hide();
  //close the dialog
  emit rotationChanged( sliderRotation->value() );
  emit enableAutomatic( cboxAutomatic->isChecked() );
  emit changePlacement( cboPlacement->currentIndex() );
  emit enableNorthArrow( cboxShow->isChecked() );
  emit needToRefresh();

  accept();
}

void QgsNorthArrowPluginGui::on_buttonBox_rejected()
{
  reject();
}

void QgsNorthArrowPluginGui::setRotation( int theInt )
{
  rotatePixmap( theInt );
  //sliderRotation->setValue(theInt);
  // signal/slot connection defined in 'designer' causes the slider to
  // be moved to reflect the change in the spinbox.
  spinAngle->setValue( theInt );
}

void QgsNorthArrowPluginGui::setPlacementLabels( QStringList& labels )
{
  cboPlacement->clear();
  cboPlacement->addItems( labels );
}

void QgsNorthArrowPluginGui::setPlacement( int placementIndex )
{
  cboPlacement->setCurrentIndex( placementIndex );
}

void QgsNorthArrowPluginGui::setEnabled( bool theBool )
{
  cboxShow->setChecked( theBool );
}

void QgsNorthArrowPluginGui::setAutomatic( bool theBool )
{
  cboxAutomatic->setChecked( theBool );
}

void QgsNorthArrowPluginGui::setAutomaticDisabled()
{
  cboxAutomatic->setEnabled( false );
}


//overides function by the same name created in .ui
void QgsNorthArrowPluginGui::on_spinAngle_valueChanged( int theInt )
{
  Q_UNUSED( theInt );
}

void QgsNorthArrowPluginGui::on_sliderRotation_valueChanged( int theInt )
{
  rotatePixmap( theInt );
}

void QgsNorthArrowPluginGui::rotatePixmap( int theRotationInt )
{
  QPixmap myQPixmap;
  QString myFileNameQString = QgsApplication::pkgDataPath() + "/images/north_arrows/default.png";
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

//
// Called when the widget has been resized.
//

void QgsNorthArrowPluginGui::resizeEvent( QResizeEvent *theResizeEvent )
{
  Q_UNUSED( theResizeEvent );
  rotatePixmap( sliderRotation->value() );
}
