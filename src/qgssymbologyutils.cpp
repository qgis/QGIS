/***************************************************************************
                         qgssymbologyutils.cpp  -  description
                             -------------------
    begin                : Oct 2003
    copyright            : (C) 2003 by Marco Hugentobler
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

#include "qgssymbologyutils.h"

QString QgsSymbologyUtils::penStyle2QString(Qt::PenStyle penstyle)
{
    if(penstyle==Qt::NoPen)
    {
	return "NoPen";
    }
    else if(penstyle==Qt::SolidLine)
    {
	return "SolidLine";
    }
    else if(penstyle==Qt::DashLine)
    {
	return "DashLine";
    }
    else if(penstyle==Qt::DotLine)
    {
	return "DotLine";
    }
    else if(penstyle==Qt::DashDotLine)
    {
	return "DashDotLine";
    }
    else if(penstyle==Qt::DashDotDotLine)
    {
	return "DashDotDotLine";
    }
    else if(penstyle==Qt::MPenStyle)
    {
	return "MPenStyle";
    }
    else//return a null string 
    {
	return QString();
    }
}

Qt::PenStyle QgsSymbologyUtils::qString2PenStyle(QString string)
{
    if(string=="NoPen")
    {
	return Qt::NoPen;	
    }
    else if(string=="SolidLine")
    {
	return Qt::SolidLine;
    }
    else if(string=="DashLine")
    {
	return Qt::DashLine;
    }
    else if(string=="DotLine")
    {
	return Qt::DotLine;
    }
    else if(string=="DashDotLine")
    {
	return Qt::DashDotLine;
    }
    else if(string=="DashDotDotLine")
    {
	return Qt::DashDotDotLine;
    }
    else if(string=="MPenStyle")
    {
	return Qt::MPenStyle;
    }
    else 
    {
	return Qt::NoPen;
    }
}

QString QgsSymbologyUtils::brushStyle2QString(Qt::BrushStyle brushstyle)
{
    if(brushstyle==Qt::NoBrush)
    {
	return "NoBrush";
    } 
    else if(brushstyle==Qt::SolidPattern)
    {
	return "SolidPattern";
    }
    else if(brushstyle==Qt::Dense1Pattern)
    {
	return "Dense1Pattern";
    }
    else if(brushstyle==Qt::Dense2Pattern)
    {
	return "Dense2Pattern";
    }
    else if(brushstyle==Qt::Dense3Pattern)
    {
	return "Dense3Pattern";
    }
    else if(brushstyle==Qt::Dense4Pattern)
    {
	return "Dense4Pattern";
    }
    else if(brushstyle==Qt::Dense5Pattern)
    {
	return "Dense5Pattern";
    }
    else if(brushstyle==Qt::Dense6Pattern)
    {
	return "Dense6Pattern";
    }
    else if(brushstyle==Qt::Dense7Pattern)
    {
	return "Dense7Pattern";
    }
    else if(brushstyle==Qt::HorPattern)
    {
	return "HorPattern";
    }
    else if(brushstyle==Qt::VerPattern)
    {
	return "VerPattern";
    }
    else if(brushstyle==Qt::CrossPattern)
    {
	return "CrossPattern";
    }
    else if(brushstyle==Qt::BDiagPattern)
    {
	return "BDiagPattern";
    }
    else if(brushstyle==Qt::FDiagPattern)
    {
	return "FDiagPattern";
    }
    else if(brushstyle==Qt::DiagCrossPattern)
    {
	return "DiagCrossPattern";
    }
    else if(brushstyle==Qt::CustomPattern)
    {
	return "CustomPattern";
    }
    else//return a null string
    {
	return QString();
    }
}

Qt::BrushStyle QgsSymbologyUtils::qString2BrushStyle(QString string)
{
    if(string=="NoBrush")
    {
	return Qt::NoBrush;
    } 
    else if(string=="SolidPattern")
    {
	return Qt::SolidPattern;
    }
    else if(string=="Dense1Pattern")
    {
	return Qt::Dense1Pattern;
    }
    else if(string=="Dense2Pattern")
    {
	return Qt::Dense2Pattern;
    }
    else if(string=="Dense3Pattern")
    {
	return Qt::Dense3Pattern;
    }
    else if(string=="Dense4Pattern")
    {
	return Qt::Dense4Pattern;
    }
    else if(string=="Dense5Pattern")
    {
	return Qt::Dense5Pattern;
    }
    else if(string=="Dense6Pattern")
    {
	return Qt::Dense6Pattern;
    }
    else if(string=="Dense7Pattern")
    {
	return Qt::Dense7Pattern;
    }
    else if(string=="HorPattern")
    {
	return Qt::HorPattern;
    }
    else if(string=="VerPattern")
    {
	return Qt::VerPattern;
    }
    else if(string=="CrossPattern")
    {
	return Qt::CrossPattern;
    }
    else if(string=="BDiagPattern")
    {
	return Qt::BDiagPattern;
    }
    else if(string=="FDiagPattern")
    {
	return Qt::FDiagPattern;
    }
    else if(string=="DiagCrossPattern")
    {
	return Qt::DiagCrossPattern;
    }
    else if(string=="CustomPattern")
    {
	return Qt::CustomPattern;
    }
    else//return a null string
    {
	return Qt::NoBrush;
    }
}

QPixmap* QgsSymbologyUtils::qString2Pixmap(QString string)
{
    //soon
}
