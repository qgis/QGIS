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

//qt includes
#include <qapplication.h>
#include <qpainter.h>
#include <qlabel.h>
#include <iostream>
#include <qspinbox.h>
#include <qslider.h>
#include <qtabwidget.h>
#include <qcombobox.h>
#include <qcheckbox.h>
//standard includes
#include <math.h>

PluginGui::PluginGui() : PluginGuiBase()
{

  //temporary hack until this is implemented
  tabNorthArrowOptions->removePage( tabIcon );
  rotatePixmap(0);
}

    PluginGui::PluginGui( QWidget* parent , const char* name , bool modal , WFlags fl  )
: PluginGuiBase( parent, name, modal, fl )
{
  //temporary hack until this is implemented
  tabNorthArrowOptions->removePage( tabIcon );
}  
PluginGui::~PluginGui()
{
}

void PluginGui::pbnOK_clicked()
{
  // Hide the dialog
  hide();
  //close the dialog
  emit rotationChanged(sliderRotation->value());
  emit changePlacement(cboPlacement->currentText());
  emit enableNorthArrow(cboxShow->isChecked());
  done(1);
}
void PluginGui::pbnCancel_clicked()
{
  close(1);
}

void PluginGui::setRotation(int theInt)
{
  rotatePixmap(theInt);
  sliderRotation->setValue(theInt);
}

void PluginGui::setPlacement(QString thePlacementQString)
{
  cboPlacement->setCurrentText(tr(thePlacementQString));
}

void PluginGui::setEnabled(bool theBool)
{
  cboxShow->setChecked(theBool);
}


//overides function byt the same name created in .ui
void PluginGui::spinSize_valueChanged( int theInt)
{

}

//overides function byt the same name created in .ui
void PluginGui::sliderRotation_valueChanged( int theInt)
{
  rotatePixmap(theInt);
}

void PluginGui::rotatePixmap(int theRotationInt)
{
  QPixmap myQPixmap;
#ifdef WIN32
  QString PKGDATAPATH = qApp->applicationDirPath() + "/share/qgis";
#endif
  QString myFileNameQString = QString(PKGDATAPATH) + QString("/images/north_arrows/default.png");
  //std::cout << "Trying to load " << myFileNameQString << std::cout;
  if (myQPixmap.load(myFileNameQString))
  {
    QPixmap  myPainterPixmap(myQPixmap.height(),myQPixmap.width());
    myPainterPixmap.fill();
    QPainter myQPainter;
    myQPainter.begin(&myPainterPixmap);	

    double centerXDouble = myQPixmap.width()/2;
    double centerYDouble = myQPixmap.height()/2;
    //save the current canvas rotation
    myQPainter.save();
    //myQPainter.translate( (int)centerXDouble, (int)centerYDouble );

    //rotate the canvas
    myQPainter.rotate( theRotationInt );
    //work out how to shift the image so that it appears in the center of the canvas
    //(x cos a + y sin a - x, -x sin a + y cos a - y)
    const double PI = 3.14159265358979323846;
    double myRadiansDouble = (PI/180) * theRotationInt;
    int xShift = static_cast<int>((
                (centerXDouble * cos(myRadiansDouble)) + 
                (centerYDouble * sin(myRadiansDouble))
                ) - centerXDouble);
    int yShift = static_cast<int>((
                (-centerXDouble * sin(myRadiansDouble)) + 
                (centerYDouble * cos(myRadiansDouble))
                ) - centerYDouble);	



    //draw the pixmap in the proper position
    myQPainter.drawPixmap(xShift,yShift,myQPixmap);	


    //unrotate the canvase again
    myQPainter.restore();
    myQPainter.end();

    //determine the center of the canvas given that we will bitblt form the origin of the narrow
    int myCenterXInt = static_cast<int>((pixmapLabel->width()-myQPixmap.width())/2);
    int myCenterYInt = static_cast<int>((pixmapLabel->height()-myQPixmap.height())/2);
    bitBlt ( pixmapLabel, myCenterXInt,myCenterYInt, &myPainterPixmap, 0, 0, -1 , -1, Qt::CopyROP, false);


    //pixmapLabel1->setPixmap(myPainterPixmap);            
  }
  else
  {
    QPixmap  myPainterPixmap(200,200);
    myPainterPixmap.fill();
    QPainter myQPainter;
    myQPainter.begin(&myPainterPixmap);	
    QFont myQFont("time", 18, QFont::Bold);
    myQPainter.setFont(myQFont);
    myQPainter.setPen(Qt::red);
    myQPainter.drawText(10, 20, QString("Pixmap Not Found"));
    myQPainter.end();
    pixmapLabel->setPixmap(myPainterPixmap);    
  }

}

// Called when the widget needs to be updated.
//

void PluginGui::paintEvent( QPaintEvent * thePaintEvent)
{
  rotatePixmap(sliderRotation->value());
}

//
// Called when the widget has been resized.
// 

void PluginGui::resizeEvent( QResizeEvent * theResizeEvent)
{
  rotatePixmap(sliderRotation->value());  
}
