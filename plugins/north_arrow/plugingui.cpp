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
#include <qpainter.h>
#include <qlabel.h>
#include <iostream>
#include <qspinbox.h>
//standard includes

PluginGui::PluginGui() : PluginGuiBase()
{
  
}

PluginGui::PluginGui( QWidget* parent , const char* name , bool modal , WFlags fl  )
: PluginGuiBase( parent, name, modal, fl )
{
   
}  
PluginGui::~PluginGui()
{
}

void PluginGui::pbnOK_clicked()
{
  //
  // If you have a produced a raster layer using your plugin, you can ask qgis to 
  // add it to the view using:
  // emit drawRasterLayer(QString("layername"));
  // or for a vector layer
  // emit drawVectorLayer(QString("pathname"),QString("layername"),QString("provider name (either ogr or postgres"));
  //
  //close the dialog
  emit rotationChanged(spinRotation->value());
  done(1);
} 
void PluginGui::pbnCancel_clicked()
{
 close(1);
}

void PluginGui::setRotation(int theInt)
{
  spinRotation->setValue(theInt);
  rotatePixmap(theInt);
}
  

//overides function byt the same name created in .ui
void PluginGui::spinRotation_valueChanged( int theInt)
{
  rotatePixmap(theInt);
}

//overides function byt the same name created in .ui
void PluginGui::sliderRotation_valueChanged( int theInt)
{
  rotatePixmap(theInt);
}

void PluginGui::rotatePixmap(int theRotationInt)
{
    QPixmap myQPixmap;


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
	
	bitBlt ( pixmapLabel, 0, 0, &myPainterPixmap, 0, 0, -1 , -1, Qt::CopyROP, false);
	 
	
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
        myQPainter.setPen(Qt::black);
        myQPainter.drawText(10, 20, QString("Pixmap Not Found"));
        myQPainter.end();
        pixmapLabel->setPixmap(myPainterPixmap);    
    }

}
