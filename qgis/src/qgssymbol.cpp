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
#include "qgsmarkercatalogue.h"

#include <qpainter.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qbitmap.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qrect.h>
#include <qpointarray.h>

QgsSymbol::QgsSymbol()
{
  mPointSymbolName = "hard:circle";
  mPointSize = 6;
  mPointSymbolPixmap.resize(1,1);
  mCacheUpToDate = false;
}

QgsSymbol::QgsSymbol(QColor c)
{
  mPointSymbolName = "hard:circle";
  mPen.setColor(c);
  mBrush.setColor(c);
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

void QgsSymbol::setNamedPointSymbol(QString name)
{
    mPointSymbolName = name;
    mCacheUpToDate = false;
}

QString QgsSymbol::pointSymbolName() const
{
    return mPointSymbolName;
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

    mPointSymbolPicture = QgsMarkerCatalogue::instance()->marker ( mPointSymbolName, mPointSize,
	                        mPen, mBrush, QgsSVGCache::instance().getOversampling() );

    QRect br = mPointSymbolPicture.boundingRect();
    mPointSymbolPixmap.resize ( br.width(), br.height() );

    // TODO - this is not correct, the background must be transparent
    mPointSymbolPixmap.fill ( QColor(255,255,255) );

    QPainter pixpainter;
    pixpainter.begin(&mPointSymbolPixmap);
    pixpainter.drawPicture ( -br.x(), -br.y(), mPointSymbolPicture );
    pixpainter.end();

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
