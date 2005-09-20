/***************************************************************************
                          qgsmarkersymbol.cpp  -  description
                             -------------------
    begin                : March 2004
    copyright            : (C) 2004 by Marco Hugentobler
    email                : mhugent@geo.unizh.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id */

#include "qgsmarkersymbol.h"
#include "qpainter.h"
#include "qgssymbologyutils.h"

void QgsMarkerSymbol::setPicture(const QString& svgpath)
{
    mSvgPath=svgpath;
}

bool QgsMarkerSymbol::writeXML( QDomNode & item, QDomDocument & document )
{
    bool returnval = false;
    returnval = true; // remove this if anyone ever adds checking here
    QDomElement markersymbol=document.createElement("markersymbol");
    item.appendChild(markersymbol);
    QDomElement svgpath=document.createElement("svgpath");
    QDomText svgpathtxt=document.createTextNode(mSvgPath);
    svgpath.appendChild(svgpathtxt);
    markersymbol.appendChild(svgpath);
    QDomElement scalefactor=document.createElement("scalefactor");
    QDomText scalefactortxt=document.createTextNode(QString::number(mScaleFactor,'f',2));
    scalefactor.appendChild(scalefactortxt);
    markersymbol.appendChild(scalefactor);
    markersymbol.appendChild(scalefactor);
    QDomElement outlinecolor=document.createElement("outlinecolor");
    outlinecolor.setAttribute("red",QString::number(mPen.color().red()));
    outlinecolor.setAttribute("green",QString::number(mPen.color().green()));
    outlinecolor.setAttribute("blue",QString::number(mPen.color().blue()));
    markersymbol.appendChild(outlinecolor);
    QDomElement outlinestyle=document.createElement("outlinestyle");
    QDomText outlinestyletxt=document.createTextNode(QgsSymbologyUtils::penStyle2QString(mPen.style()));
    outlinestyle.appendChild(outlinestyletxt);
    markersymbol.appendChild(outlinestyle);
    QDomElement outlinewidth=document.createElement("outlinewidth");
    QDomText outlinewidthtxt=document.createTextNode(QString::number(mPen.width()));
    outlinewidth.appendChild(outlinewidthtxt);
    markersymbol.appendChild(outlinewidth);
    QDomElement fillcolor=document.createElement("fillcolor");
    fillcolor.setAttribute("red",QString::number(mBrush.color().red()));
    fillcolor.setAttribute("green",QString::number(mBrush.color().green()));
    fillcolor.setAttribute("blue",QString::number(mBrush.color().blue()));
    markersymbol.appendChild(fillcolor);
    QDomElement fillpattern=document.createElement("fillpattern");
    QDomText fillpatterntxt=document.createTextNode(QgsSymbologyUtils::brushStyle2QString(mBrush.style()));
    fillpattern.appendChild(fillpatterntxt);
    markersymbol.appendChild(fillpattern);
    fillpattern.appendChild(fillpatterntxt);
    return returnval;
}
    

