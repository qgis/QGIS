/***************************************************************************
                         qgscomposeritem.cpp
                             -------------------
    begin                : January 2005
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
#include <qwidget.h>
#include <qdom.h>

#include "qgscomposition.h"
#include "qgscomposeritem.h"

#include <iostream>

QgsComposerItem::QgsComposerItem(void)
{
    mSelected;
    mPlotStyle = QgsComposition::Preview;
}

QgsComposerItem::~QgsComposerItem()
{
}

void QgsComposerItem::setPlotStyle ( QgsComposition::PlotStyle d ) { mPlotStyle = d; }

QgsComposition::PlotStyle QgsComposerItem::plotStyle ( void ) { return mPlotStyle; }

void QgsComposerItem::setSelected( bool s ) 
{
    std::cout << "QgsComposerItem::setSelected" << std::endl; 
    mSelected = s; 
}

bool QgsComposerItem::selected( void ) { return mSelected; }


void QgsComposerItem::showOptions ( QWidget * parent ) { }

bool QgsComposerItem::writeSettings ( void )  { return true; }

bool QgsComposerItem::readSettings ( void )  { return true; }
    
bool QgsComposerItem::writeXML( QDomNode & node, QDomDocument & doc, bool templ ) { return true; }

bool QgsComposerItem::readXML( QDomNode & node ) {  return true; }
