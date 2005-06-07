/***************************************************************************
                         qgssinglesymrenderer.cpp  -  description
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
/* $Id$ */
#include "qgis.h"
#include "qgssinglesymrenderer.h"
#include "qgsfeature.h"
#include "qgis.h"
#include "qgsvectorlayer.h"
#include "qstring.h"
#include "qgssisydialog.h"
#include "qgslegenditem.h"
#include "qgssymbologyutils.h"
#include "qgssvgcache.h"
#include <qdom.h>
#include <qpainter.h>
#include <qpicture.h>
#include <qpixmap.h>

QgsSingleSymRenderer::QgsSingleSymRenderer(QGis::VectorType type)
{
    mVectorType=type;
    //call superclass method to set up selection colour
    initialiseSelectionColor();
  
    //initial setting based on random color
    QgsSymbol* sy = new QgsSymbol(mVectorType);
  
    //random fill colors for points and polygons and pen colors for lines
    int red = 1 + (int) (255.0 * rand() / (RAND_MAX + 1.0));
    int green = 1 + (int) (255.0 * rand() / (RAND_MAX + 1.0));
    int blue = 1 + (int) (255.0 * rand() / (RAND_MAX + 1.0));

    if (type == QGis::Line)
    {
	sy->pen().setColor(QColor(red, green, blue));
    } 
    else
    {
	sy->setFillColor(QColor(red, green, blue));
	sy->setFillStyle(Qt::SolidPattern);
	sy->pen().setColor(QColor(0, 0, 0));
    }
    sy->setLineWidth(1);
    mSymbol=sy;
}

QgsSingleSymRenderer::~QgsSingleSymRenderer()
{
    delete mSymbol;
}

void QgsSingleSymRenderer::addSymbol(QgsSymbol* sy)
{
    delete mSymbol;
    mSymbol=sy;
}

void QgsSingleSymRenderer::renderFeature(QPainter * p, QgsFeature * f, QPicture* pic, 
	         double* scalefactor, bool selected, int oversampling, double widthScale)
{
	// Point 
	if ( pic && mVectorType == QGis::Point) {
	    *pic = mSymbol->getPointSymbolAsPicture( oversampling, widthScale, 
					 selected, mSelectionColor );
	    
	    if ( scalefactor ) *scalefactor = 1;
	} 

        // Line, polygon
 	if ( mVectorType != QGis::Point )
	{
	    if( !selected ) 
	    {
		QPen pen=mSymbol->pen();
		pen.setWidth ( (int) (widthScale * pen.width()) );
		p->setPen(pen);
		p->setBrush(mSymbol->brush());
	    }
	    else
	    {
		QPen pen=mSymbol->pen();
		pen.setWidth ( (int) (widthScale * pen.width()) );
		pen.setColor(mSelectionColor);
		QBrush brush=mSymbol->brush();
		brush.setColor(mSelectionColor);
		p->setPen(pen);
		p->setBrush(brush);
	    }
	}
}

void QgsSingleSymRenderer::readXML(const QDomNode& rnode, QgsVectorLayer& vl)
{
    mVectorType = vl.vectorType();
    QgsSymbol* sy = new QgsSymbol();

    QDomNode synode = rnode.namedItem("symbol");
    
    if ( synode.isNull() )
    {
        qDebug( "%s:%d in project file no symbol node in renderitem DOM" );
        // XXX abort?
    }
    else
    {
        sy->readXML ( synode );
    }

    //create a renderer and add it to the vector layer
    this->addSymbol(sy);
    vl.setRenderer(this);
}

bool QgsSingleSymRenderer::writeXML( QDomNode & layer_node, QDomDocument & document )
{
  bool returnval=false;
  QDomElement singlesymbol=document.createElement("singlesymbol");
  layer_node.appendChild(singlesymbol);
  if(mSymbol)
  {
    returnval=mSymbol->writeXML(singlesymbol,document);
  }
  return returnval;
}


std::list<int> QgsSingleSymRenderer::classificationAttributes()
{
  std::list<int> list;
  return list;//return an empty list
}

QString QgsSingleSymRenderer::name()
{
  return "Single Symbol";
}

const std::list<QgsSymbol*> QgsSingleSymRenderer::symbols() const
{
    std::list<QgsSymbol*> list;
    list.push_back(mSymbol);
    return list;
}

