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

#include <qpainter.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qbitmap.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qrect.h>
#include <qpointarray.h>
#include <qdir.h>

#include "qgssvgcache.h"
#include "qgsmarkercatalogue.h"

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

    // SVG
    QString appdir;
#if defined(WIN32) || defined(Q_OS_MACX)
    appdir = qApp->applicationDirPath();
#else
    appdir = PREFIX;
#endif

    // TODO recursiv ?
    QDir dir ( appdir + "/share/qgis/svg/" );
    
    QStringList dl = dir.entryList(QDir::Dirs);
    
    for ( QStringList::iterator it = dl.begin(); it != dl.end(); ++it ) {
	if ( *it == "." || *it == ".." ) continue;

	QDir dir2 ( appdir + "/share/qgis/svg/" + *it );

	QStringList dl2 = dir2.entryList("*.svg",QDir::Files);
	
	for ( QStringList::iterator it2 = dl2.begin(); it2 != dl2.end(); ++it2 ) {
	    // TODO test if it is correct SVG
	    mList.append ( "svg:" + appdir + "/share/qgis/svg/" + *it + "/" + *it2 );
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

QPicture QgsMarkerCatalogue::marker ( QString fullName, int size, QPen pen, QBrush brush, int oversample )
{
    QPicture picture;
    
    if ( fullName.left(5) == "hard:" ) {
        return hardMarker ( fullName.mid(5), size, pen, brush, oversample ); 
    } else if ( fullName.left(4) == "svg:" ) {
        return svgMarker ( fullName.mid(4), size ); 
    }

    return picture; // empty
}

QPicture QgsMarkerCatalogue::svgMarker ( QString name, int s )
{
    QPicture picture;

    QPixmap pixmap = QgsSVGCache::instance().getPixmap(name,1.);
    
    double scale = 1. * s / ( ( pixmap.width() + pixmap.height() ) / 2 ) ;
    
    pixmap = QgsSVGCache::instance().getPixmap(name,scale);

    QPainter painter;
    painter.begin(&picture);
    painter.drawPixmap ( 0, 0, pixmap );
    painter.end();

    return picture;
}

QPicture QgsMarkerCatalogue::hardMarker ( QString name, int s, QPen pen, QBrush brush, int oversampling )
{
    // Size of polygon symbols is calculated so that the area is equal to circle with 
    // diameter mPointSize
    
    QPicture picture;
    
    // Size for circle
    int half = (int)floor(s/2); // number of points from center
    int size = 2*half + 1;  // must be odd
    double area = 3.14 * (size/2.) * (size/2.);

    // Picture
    QPainter picpainter;
    picpainter.begin(&picture);
    
    // Also width must be odd otherwise there are discrepancies visible in canvas!
    int lw = (int)(2*floor(pen.width()/2)+1);
    pen.setWidth(lw);
    picpainter.setPen ( pen );

    picpainter.setBrush( brush);

    if ( name == "circle" ) 
    {
	picpainter.drawEllipse(0, 0, size, size);
    } 
    else if ( name == "rectangle" ) 
    {
	size = (int) (2*floor(sqrt(area)/2.) + 1);
	picpainter.drawRect(0, 0, size, size);
    } 
    else if ( name == "diamond" ) 
    {
	half = (int) ( sqrt(area/2.) );
	QPointArray pa(4);
	pa.setPoint ( 0, 0, half);
	pa.setPoint ( 1, half, 2*half);
	pa.setPoint ( 2, 2*half, half);
	pa.setPoint ( 3, half, 0);
	picpainter.drawPolygon ( pa );
    }
    // Warning! if pen width > 0 picpainter.drawLine(x1,y1,x2,y2) will draw only (x1,y1,x2-1,y2-1) !
    // It is impossible to use drawLine(x1,y1+1,x2,y2+1) because then the bounding box is incorrect 
    // and the picture is shifted in drawFeature. 
    // -> draw line width 1 as 0 and width > 1 as rectangle
    else if ( name == "cross" ) 
    {
	pen.setWidth(0);
	picpainter.setPen ( pen );
	if ( lw < 3 ) {
	    // Draw line
	    picpainter.drawLine(0, half, size-1, half); // horizontal
	    picpainter.drawLine(half, 0, half, size-1); // vertical
	} else {
	    // Draw rectangle
	    brush.setColor( pen.color() );
	    picpainter.setBrush ( brush );
	    int off = (int) floor(lw/2);
	    picpainter.drawRect(0, half-off, size, lw);
	    picpainter.drawRect(half-off, 0, lw, size);
	}
    }
    else if ( name == "cross2" ) 
    {
	pen.setWidth(0);
	picpainter.setPen ( pen );
	if ( lw < 3 ) {
	    // Draw line
	    half = (int) floor( s/2/sqrt(2));
	    size = 2*half + 1;
	    picpainter.drawLine( 0, 0, size-1, size-1);
	    picpainter.drawLine( 0, size-1, size-1, 0);
	} else {
	    // Draw rectangle
	    brush.setColor( pen.color() );
	    picpainter.setBrush ( brush );
	    int off = (int) floor(lw/2);
	    picpainter.rotate ( 45 );
	    picpainter.drawRect(0, half-off, size, lw);
	    picpainter.drawRect(half-off, 0, lw, size);
	}
    }
    picpainter.end();

    // If oversampling > 1 create pixmap 
    if ( oversampling > 1 ) {
	QRect br = picture.boundingRect();
	QPixmap pixmap ( oversampling * br.width(), oversampling * br.height() );

	// Find bg color (must differ from line and fill)
	QColor transparent;
	for ( int i = 0; i < 255; i++ ) {
	    if ( pen.color().red() != i &&  brush.color().red() != i ) {
		transparent = QColor ( i, 0, 0 );
		break;
	    }
	}
	
	pixmap.fill( transparent );
	QPainter pixpainter;
	pixpainter.begin(&pixmap);
	pixpainter.scale ( oversampling, oversampling );
	pixpainter.drawPicture ( -br.x(), -br.y(), picture );
	pixpainter.end();

	QImage img = pixmap.convertToImage();
	img.setAlphaBuffer(true);
	for ( int i = 0; i < img.width(); i++ ) {
	    for ( int j = 0; j < img.height(); j++ ) {
		QRgb pixel = img.pixel(i, j);
		int alpha = 255;
		if ( qRed(pixel) == transparent.red() ) {
		    alpha = 0;
		}
		img.setPixel ( i, j, qRgba(qRed(pixel), qGreen(pixel), qBlue(pixel), alpha) );
	    }
	}
	img = img.smoothScale( br.width(), br.height());
	pixmap.convertFromImage ( img );

        picpainter.begin(&picture);
	picpainter.drawPixmap ( 0, 0, pixmap );
	picpainter.end();
    } 

    return picture;
}

