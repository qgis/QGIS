/***************************************************************************
                          QgsSymbol.cpp  -  description
                             -------------------
    begin                : Sun Aug 11 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc dot com
       Romans 3:23=>Romans 6:23=>Romans 5:8=>Romans 10:9,10=>Romans 12
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */
#include <cmath>
#include <iostream>

#include "qgssymbol.h"
#include "qgssymbologyutils.h"
#include "qgssvgcache.h"

#include <qpainter.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qbitmap.h>
#include <qstring.h>
#include <qrect.h>
#include <qpointarray.h>

QgsSymbol::QgsSymbol()
{
  mPointSymbol = Circle;
  mPointSize = 6;
  mPointSymbolPixmap.resize(1,1);
  mCacheUpToDate = false;
}

QgsSymbol::QgsSymbol(QColor c)
{
  mPen.setColor(c);
  mBrush.setColor(c);
  mPointSymbol = Circle;
  mPointSize = 6;
  mPointSymbolPixmap.resize(1,1);
  mCacheUpToDate = false;
}

QgsSymbol::~QgsSymbol()
{
}
QColor QgsSymbol::color() const
{
  return mPen.color();
}

void QgsSymbol::setColor(QColor c)
{
  mPen.setColor(c);
  mCacheUpToDate = false;
}

QColor QgsSymbol::fillColor() const
{
  return mBrush.color();
}

void QgsSymbol::setFillColor(QColor c)
{
  mBrush.setColor(c);
  mCacheUpToDate = false;
}

int QgsSymbol::lineWidth() const
{
  return mPen.width();
}

void QgsSymbol::setLineWidth(int w)
{
  mPen.setWidth(w);
  mCacheUpToDate = false;
}

void QgsSymbol::setLineStyle( Qt::PenStyle s )
{
  mPen.setStyle(s);
  mCacheUpToDate = false;
}

void QgsSymbol::setFillStyle( Qt::BrushStyle s )
{
  mBrush.setStyle(s);
  mCacheUpToDate = false;
}

void QgsSymbol::setPointSymbol(PointSymbol ps)
{
    if (  ps >= NPointSymbols || ps < 0 )
	mPointSymbol = Circle;
    else
        mPointSymbol = ps;
    
    mCacheUpToDate = false;
}

void QgsSymbol::setNamedPointSymbol(QString name)
{
    if ( name == "circle" ) 
        mPointSymbol = Circle;	
    else if ( name == "rectangle" ) 
        mPointSymbol = Rectangle;	
    else if ( name == "diamond" ) 
        mPointSymbol = Diamond;	
    else if ( name == "cross" ) 
        mPointSymbol = Cross;	
    else if ( name == "cross2" ) 
        mPointSymbol = Cross2;	
    else 
        mPointSymbol = Circle;	

    mCacheUpToDate = false;
}

QgsSymbol::PointSymbol QgsSymbol::pointSymbol() const
{
  return mPointSymbol;
}

QString QgsSymbol::pointSymbolName() const
{
    switch ( mPointSymbol ) {
	case Circle:
    	    return "circle";
	    break;
	case Rectangle:
    	    return "rectangle";
	    break;
	case Diamond:
    	    return "diamond";
	    break;
	case Cross:
    	    return "cross";
	    break;
	case Cross2:
    	    return "cross2";
	    break;
    }
  
    return "circle";
}

void QgsSymbol::setPointSize(int s)
{
    if ( s < 3 )  
	mPointSize = 3;
    else 
    	mPointSize = s;

    mCacheUpToDate = false;
}

int QgsSymbol::pointSize() const
{
    return mPointSize;
}

QPixmap  QgsSymbol::getSymbolAsPixmap(int xDim, int yDim)
{
        QPixmap myQPixmap(xDim,yDim);
        QPainter myQPainter;
        myQPainter.begin(&myQPixmap);
        myQPainter.setBrush(mBrush);
        myQPainter.setPen(mPen);
        myQPainter.drawRect(0, 0, xDim, yDim);
        myQPainter.end();
        return myQPixmap;
}

QPixmap QgsSymbol::getPointSymbolAsPixmap()
{
    if ( !mCacheUpToDate) cache();
    return mPointSymbolPixmap;
}

QPicture QgsSymbol::getPointSymbolAsPicture()
{
    if ( !mCacheUpToDate) cache();
    return mPointSymbolPicture;
}

void QgsSymbol::cache()
{
    // Size of polygon symbols is calculated so that the area is equal to circle with 
    // diameter mPointSize
    
    // Size for circle
    int half = (int)floor(mPointSize/2); // number of points from center
    int size = 2*half + 1;  // must be odd
    double area = 3.14 * (size/2.) * (size/2.);

    // Picture
    QPainter picpainter;
    picpainter.begin(&mPointSymbolPicture);
    
    // Also width must be odd otherwise there are discrepancies visible in canvas!
    QPen pen = mPen;
    int lw = (int)(2*floor(pen.width()/2)+1);
    pen.setWidth(lw);
    picpainter.setPen ( pen );

    picpainter.setBrush(mBrush);

    switch ( mPointSymbol ) {
	case Circle:
	    picpainter.drawEllipse(0, 0, size, size);
	    break;
	case Rectangle:
	    size = (int) (2*floor(sqrt(area)/2.) + 1);
	    picpainter.drawRect(0, 0, size, size);
	    break;
	case Diamond:
	    {
	    half = (int) ( sqrt(area/2.) );
	    QPointArray pa(4);
	    pa.setPoint ( 0, 0, half);
	    pa.setPoint ( 1, half, 2*half);
	    pa.setPoint ( 2, 2*half, half);
	    pa.setPoint ( 3, half, 0);
	    picpainter.drawPolygon ( pa );
	    }
	    break;
    
    // Warning! if pen width > 0 picpainter.drawLine(x1,y1,x2,y2) will draw only (x1,y1,x2-1,y2-1) !
    // It is impossible to use drawLine(x1,y1+1,x2,y2+1) because then the bounding box is incorrect 
    // and the picture is shifted in drawFeature. 
    // -> draw line width 1 as 0 and width > 1 as rectangle

	case Cross:
	    pen.setWidth(0);
	    picpainter.setPen ( pen );
	    if ( lw < 3 ) {
		// Draw line
		picpainter.drawLine(0, half, size-1, half); // horizontal
		picpainter.drawLine(half, 0, half, size-1); // vertical
	    } else {
		// Draw rectangle
		QBrush brush = mBrush;
		brush.setColor( mPen.color() );
		picpainter.setBrush ( brush );
		int off = (int) floor(lw/2);
	        picpainter.drawRect(0, half-off, size, lw);
	        picpainter.drawRect(half-off, 0, lw, size);
	    }
	    break;
	case Cross2:
	    pen.setWidth(0);
	    picpainter.setPen ( pen );
	    if ( lw < 3 ) {
		// Draw line
		half = (int) floor( mPointSize/2/sqrt(2));
		size = 2*half + 1;
		picpainter.drawLine( 0, 0, size-1, size-1);
		picpainter.drawLine( 0, size-1, size-1, 0);
	    } else {
		// Draw rectangle
		QBrush brush = mBrush;
		brush.setColor( mPen.color() );
		picpainter.setBrush ( brush );
		int off = (int) floor(lw/2);
		picpainter.rotate ( 45 );
	        picpainter.drawRect(0, half-off, size, lw);
	        picpainter.drawRect(half-off, 0, lw, size);
	    }
	    break;
	default:
	    break;
    }
    picpainter.end();

    // Pixmap
    int oversampling = QgsSVGCache::instance().getOversampling();
    QRect br = mPointSymbolPicture.boundingRect();
    mPointSymbolPixmap.resize ( oversampling * br.width(), oversampling * br.height() );

    // Find bg color (must differ from line and fill)
    QColor transparent;
    for ( int i = 0; i < 255; i++ ) {
	if ( mPen.color().red() != i &&  mBrush.color().red() != i ) {
	    transparent = QColor ( i, 0, 0 );
	    break;
	}
    }
    
    mPointSymbolPixmap.fill( transparent );
    QPainter pixpainter;
    pixpainter.begin(&mPointSymbolPixmap);
    pixpainter.scale ( oversampling, oversampling );
    pixpainter.drawPicture ( -br.x(), -br.y(), mPointSymbolPicture );
    pixpainter.end();

    QImage img = mPointSymbolPixmap.convertToImage();
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
    mPointSymbolPixmap.convertFromImage ( img );

    mCacheUpToDate = true;
}

bool QgsSymbol::writeXML( QDomNode & item, QDomDocument & document )
{
    bool returnval=false;
    QDomElement symbol=document.createElement("symbol");
    item.appendChild(symbol);

    QDomElement pointsymbol=document.createElement("pointsymbol");
    QDomText pointsymboltxt=document.createTextNode(pointSymbolName());
    symbol.appendChild(pointsymbol);
    pointsymbol.appendChild(pointsymboltxt);

    QDomElement pointsize=document.createElement("pointsize");
    QDomText pointsizetxt=document.createTextNode( QString::number(pointSize()) );
    symbol.appendChild(pointsize);
    pointsize.appendChild(pointsizetxt);

    QDomElement outlinecolor=document.createElement("outlinecolor");
    outlinecolor.setAttribute("red",QString::number(mPen.color().red()));
    outlinecolor.setAttribute("green",QString::number(mPen.color().green()));
    outlinecolor.setAttribute("blue",QString::number(mPen.color().blue()));
    symbol.appendChild(outlinecolor);
    QDomElement outlinestyle=document.createElement("outlinestyle");
    QDomText outlinestyletxt=document.createTextNode(QgsSymbologyUtils::penStyle2QString(mPen.style()));
    outlinestyle.appendChild(outlinestyletxt);
    symbol.appendChild(outlinestyle);
    QDomElement outlinewidth=document.createElement("outlinewidth");
    QDomText outlinewidthtxt=document.createTextNode(QString::number(mPen.width()));
    outlinewidth.appendChild(outlinewidthtxt);
    symbol.appendChild(outlinewidth);
    QDomElement fillcolor=document.createElement("fillcolor");
    fillcolor.setAttribute("red",QString::number(mBrush.color().red()));
    fillcolor.setAttribute("green",QString::number(mBrush.color().green()));
    fillcolor.setAttribute("blue",QString::number(mBrush.color().blue()));
    symbol.appendChild(fillcolor);
    QDomElement fillpattern=document.createElement("fillpattern");
    QDomText fillpatterntxt=document.createTextNode(QgsSymbologyUtils::brushStyle2QString(mBrush.style()));
    fillpattern.appendChild(fillpatterntxt);
    symbol.appendChild(fillpattern);
    fillpattern.appendChild(fillpatterntxt);
}

bool QgsSymbol::readXML( QDomNode & synode )
{
    QDomNode psymbnode = synode.namedItem("pointsymbol");
    QDomElement psymbelement = psymbnode.toElement();
    setNamedPointSymbol( psymbelement.text() );
    
    QDomNode psizenode = synode.namedItem("pointsize");
    QDomElement psizeelement = psizenode.toElement();
    setPointSize( psizeelement.text().toInt() );

    QDomNode outlcnode = synode.namedItem("outlinecolor");
    QDomElement oulcelement = outlcnode.toElement();
    int red = oulcelement.attribute("red").toInt();
    int green = oulcelement.attribute("green").toInt();
    int blue = oulcelement.attribute("blue").toInt();
    setColor(QColor(red, green, blue));

    QDomNode outlstnode = synode.namedItem("outlinestyle");
    QDomElement outlstelement = outlstnode.toElement();
    setLineStyle(QgsSymbologyUtils::qString2PenStyle(outlstelement.text()));

    QDomNode outlwnode = synode.namedItem("outlinewidth");
    QDomElement outlwelement = outlwnode.toElement();
    setLineWidth(outlwelement.text().toInt());

    QDomNode fillcnode = synode.namedItem("fillcolor");
    QDomElement fillcelement = fillcnode.toElement();
    red = fillcelement.attribute("red").toInt();
    green = fillcelement.attribute("green").toInt();
    blue = fillcelement.attribute("blue").toInt();
    setFillColor(QColor(red, green, blue));

    QDomNode fillpnode = synode.namedItem("fillpattern");
    QDomElement fillpelement = fillpnode.toElement();
    setFillStyle(QgsSymbologyUtils::qString2BrushStyle(fillpelement.text()));

    return true;
}
