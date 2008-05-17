/***************************************************************************
  qgsmarkercatalogue.cpp
  -------------------
begin                : March 2005
copyright            : (C) 2005 by Radim Blazek
email                : blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <cmath>
#include <iostream>
#include <assert.h>

#include <QPen>
#include <QBrush>
#include <QPainter>
#include <QImage>
#include <QString>
#include <QStringList>
#include <QRect>
#include <QPolygon>
#include <QDir>
#include <QPicture>
#include <QSvgRenderer>

#include "qgis.h"
#include "qgsapplication.h"
#include "qgsmarkercatalogue.h"
#include "qgslogger.h"

//#define IMAGEDEBUG

QgsMarkerCatalogue *QgsMarkerCatalogue::mMarkerCatalogue = 0;

QgsMarkerCatalogue::QgsMarkerCatalogue()
{
  // Init list

  // Hardcoded markers
  mList.append ( "hard:circle" );
  mList.append ( "hard:rectangle" );
  mList.append ( "hard:diamond" );
  mList.append ( "hard:cross" );
  mList.append ( "hard:cross2" );
  mList.append ( "hard:triangle");
  mList.append ( "hard:star");

  // SVG
  QString svgPath = QgsApplication::svgPath();

  // TODO recursiv ?
  QDir dir ( svgPath );

  QStringList dl = dir.entryList(QDir::Dirs);

  for ( QStringList::iterator it = dl.begin(); it != dl.end(); ++it ) {
    if ( *it == "." || *it == ".." ) continue;

    QDir dir2 ( svgPath + *it );

    QStringList dl2 = dir2.entryList("*.svg",QDir::Files);

    for ( QStringList::iterator it2 = dl2.begin(); it2 != dl2.end(); ++it2 ) {
      // TODO test if it is correct SVG
      mList.append ( "svg:" + svgPath + *it + "/" + *it2 );
    }
  }
}

QStringList QgsMarkerCatalogue::list()
{
  return mList;
}

QgsMarkerCatalogue::~QgsMarkerCatalogue()
{
}

QgsMarkerCatalogue *QgsMarkerCatalogue::instance()
{
  if ( !QgsMarkerCatalogue::mMarkerCatalogue ) {
    QgsMarkerCatalogue::mMarkerCatalogue = new QgsMarkerCatalogue();
  }

  return QgsMarkerCatalogue::mMarkerCatalogue;
}

QImage QgsMarkerCatalogue::imageMarker ( QString fullName, double size, QPen pen, QBrush brush, bool qtBug )
{
      
  // 
  // First prepare the paintdevice that the marker will be drawn onto 
  // 
  QImage myImage;
  if ( fullName.left(5) == "hard:" )
  {
    myImage = QImage (size + 1, size + 1, QImage::Format_ARGB32_Premultiplied);
  }
  else
  {
    // TODO Change this logic so width is size and height is same
    // proportion of scale factor as in oritignal SVG TS XXX
    if (size < 1) size=1;
    //QPixmap myPixmap = QPixmap(width,height);
    myImage = QImage(size ,size , QImage::Format_ARGB32_Premultiplied);
  }

  // starting with transparent QImage
  myImage.fill(0);

  QPainter myPainter;
  myPainter.begin(&myImage);
  myPainter.setRenderHint(QPainter::Antialiasing);

  //
  // Now pass the paintdevice along to have the marker rendered on it
  //

  if ( fullName.left(5) == "hard:" )
  {
    hardMarker ( &myPainter, fullName.mid(5), size, pen, brush, qtBug );
#ifdef IMAGEDEBUG
    QgsDebugMsg("*** Saving hard marker to hardMarker.png ***");
#ifdef QGISDEBUG
    myImage.save("hardMarker.png");
#endif
#endif
    return myImage;
  }
  else if ( fullName.left(4) == "svg:" )
  {
    svgMarker ( &myPainter, fullName.mid(4), size );
    return myImage;
  }
  return QImage(); // empty
}

QPicture QgsMarkerCatalogue::pictureMarker ( QString fullName, double size, QPen pen, QBrush brush, bool qtBug )
{

  //
  // First prepare the paintdevice that the marker will be drawn onto
  //
  QPicture myPicture;
  if ( fullName.left(5) == "hard:" )
  {
    //Note teh +1 offset below is required because the
    //otherwise the icons are getting clipped
    myPicture = QPicture (size+1);
  }
  else
  {
    // TODO Change this logic so width is size and height is same
    // proportion of scale factor as in oritignal SVG TS XXX
    if (size < 1) size=1;
    myPicture = QPicture(size);
  }

  QPainter myPainter(&myPicture);
  myPainter.setRenderHint(QPainter::Antialiasing);

  //
  // Now pass the paintdevice along to have the marker rndered on it
  //

  if ( fullName.left(5) == "hard:" )
  {
    hardMarker ( &myPainter, fullName.mid(5), size, pen, brush, qtBug );
    return myPicture;
  }
  else if ( fullName.left(4) == "svg:" )
  {
    svgMarker ( &myPainter, fullName.mid(4), size );
    return myPicture;
  }
  return QPicture(); // empty
}

void QgsMarkerCatalogue::svgMarker ( QPainter * thepPainter, QString filename, double scaleFactor)
{
  QSvgRenderer mySVG;
  mySVG.load(filename);
  mySVG.render(thepPainter);
}

void QgsMarkerCatalogue::hardMarker (QPainter * thepPainter, QString name, double s, QPen pen, QBrush brush, bool qtBug )
{
  // Size of polygon symbols is calculated so that the boundingbox is circumscribed
  // around a circle with diameter mPointSize

  double half = s/2; // number of points from center

  QgsDebugMsg(QString("Hard marker size %1").arg(s));

  // Find out center coordinates.
  double x_c = s/2;
  double y_c = x_c;

  // Picture
  QPicture picture;
  thepPainter->begin(&picture);
  thepPainter->setRenderHint(QPainter::Antialiasing);

  // Also width must be odd otherwise there are discrepancies visible in canvas!
  double lw = pen.widthF();//(int)(2*floor((double)pen.widthF()/2)+1); // -> lw > 0
  pen.setWidthF(lw);
  thepPainter->setPen ( pen );
  thepPainter->setBrush( brush);
  QRect box;

  // Circle radius, is used for other figures also, when compensating for line
  // width is necessary.

  double r = (s-2*lw)/2-1;
  QgsDebugMsg(QString("Hard marker radius %1").arg(r));

  if ( name == "circle" ) 
  {
    // "A stroked ellipse has a size of rectangle.size() plus the pen width."
    // (from Qt doc)
    // It doesn't seem like it is centered, however. Fudge...
    // Is this a Qt bug or feature?
    x_c -= ((lw+5)/4);
    y_c -= ((lw+5)/4);

    thepPainter->drawEllipse(x_c-r, y_c-r, x_c+r, y_c+r);
  } 
  else if ( name == "rectangle" ) 
  {
    // Same fudge as for circle...
    x_c -= ((lw+5)/4);
    y_c -= ((lw+5)/4);

    thepPainter->drawRect(x_c-r, y_c-r, x_c+r, y_c+r);
  } 
  else if ( name == "diamond" ) 
  {
    QPolygon pa(4);
    pa.setPoint ( 0, x_c-r, y_c);
    pa.setPoint ( 1, x_c, y_c+r);
    pa.setPoint ( 2, x_c+r, y_c);
    pa.setPoint ( 3, x_c, y_c-r);
    thepPainter->drawPolygon ( pa );
  }
  else if ( name == "cross" ) 
  {
    thepPainter->drawLine(x_c-half, y_c, x_c+half, y_c); // horizontal
    thepPainter->drawLine(x_c, y_c-half, x_c, y_c+half); // vertical
  }
  else if ( name == "cross2" ) 
  {
    thepPainter->drawLine( x_c-half, y_c-half, x_c+half, y_c+half);
    thepPainter->drawLine( x_c-half, y_c+half, x_c+half, y_c-half);
  }
  else if ( name == "triangle")
    {
      QPolygon pa(3);
      
      pa.setPoint ( 0, x_c-r, y_c+r);
      pa.setPoint ( 1, x_c+r, y_c+r);
      pa.setPoint ( 2, x_c, y_c-r);
      thepPainter->drawPolygon ( pa );
    }
  else if (name == "star")
    {
      int oneThird = 2*r/3;
      int twoThird = 4*r/3;
      int oneSixth = 2*r/6;

      QPolygon pa(10);
      pa.setPoint(0, x_c, y_c-half);
      pa.setPoint(1, x_c-oneSixth, y_c-oneSixth);
      pa.setPoint(2, x_c-half, y_c-oneSixth);
      pa.setPoint(3, x_c-oneSixth, y_c);
      pa.setPoint(4, x_c-half, y_c+half);
      pa.setPoint(5, x_c, y_c+oneSixth);
      pa.setPoint(6, x_c+half, y_c+half);
      pa.setPoint(7, x_c+oneSixth, y_c);
      pa.setPoint(8, x_c+half, y_c-oneSixth);
      pa.setPoint(9, x_c+oneSixth, y_c-oneSixth);
      thepPainter->drawPolygon ( pa );
    }
  thepPainter->end();
}
