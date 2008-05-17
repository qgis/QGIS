/***************************************************************************
                         qgscontinuouscolorrenderer.cpp  -  description
                             -------------------
    begin                : Nov 2003
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
/* $Id: qgscontinuouscolorrenderer.cpp 5371 2006-04-25 01:52:13Z wonder $ */

#include "qgscontinuouscolorrenderer.h"
#include "qgsmarkercatalogue.h"
#include "qgssymbol.h"
#include "qgssymbologyutils.h"
#include "qgsvectorlayer.h"

#include <cfloat>
#include <QDomElement>
#include <QPainter>
#include <QImage>

QgsContinuousColorRenderer::QgsContinuousColorRenderer(QGis::VectorType type): mMinimumSymbol(0), mMaximumSymbol(0)
{
    mVectorType = type;
}

QgsContinuousColorRenderer::QgsContinuousColorRenderer(const QgsContinuousColorRenderer& other)
{
    mVectorType = other.mVectorType;
    mClassificationField = other.mClassificationField;
    mMinimumSymbol = new QgsSymbol(*other.mMinimumSymbol);
    mMaximumSymbol = new QgsSymbol(*other.mMaximumSymbol);
}

QgsContinuousColorRenderer& QgsContinuousColorRenderer::operator=(const QgsContinuousColorRenderer& other)
{
    if(this != &other)
    {
	mVectorType = other.mVectorType;
	mClassificationField = other.mClassificationField;
	delete mMinimumSymbol;
	delete mMaximumSymbol;
	mMinimumSymbol = new QgsSymbol(*other.mMinimumSymbol);
	mMaximumSymbol = new QgsSymbol(*other.mMaximumSymbol);
    }
    return *this;
}

QgsContinuousColorRenderer::~QgsContinuousColorRenderer()
{
  delete mMinimumSymbol;
  delete mMaximumSymbol;
}

void QgsContinuousColorRenderer::setMinimumSymbol(QgsSymbol* sy)
{
  delete mMinimumSymbol;
  mMinimumSymbol = sy;
}

void QgsContinuousColorRenderer::setMaximumSymbol(QgsSymbol* sy)
{
  delete mMaximumSymbol;
  mMaximumSymbol = sy;
}

void QgsContinuousColorRenderer::renderFeature(QPainter * p, QgsFeature & f, QImage* img, bool selected, double widthScale, double rasterScaleFactor)
{
  if ((mMinimumSymbol && mMaximumSymbol))
  {
    //first find out the value for the classification attribute
    const QgsAttributeMap& attrs = f.attributeMap();
    if( attrs[mClassificationField].isNull() )
    {
      if(img)
        *img = QImage();
      return;
    }
    double fvalue = attrs[mClassificationField].toDouble();

    //double fvalue = vec[mClassificationField].fieldValue().toDouble();
    double minvalue = mMinimumSymbol->lowerValue().toDouble();
    double maxvalue = mMaximumSymbol->lowerValue().toDouble();

    QColor mincolor, maxcolor;

    if ( mVectorType == QGis::Line || mVectorType == QGis::Point )
    {
      mincolor = mMinimumSymbol->pen().color();
      maxcolor = mMaximumSymbol->pen().color();
    } 
    else                    //if polygon
    {
      p->setPen(mMinimumSymbol->pen());
      mincolor = mMinimumSymbol->fillColor();
      maxcolor = mMaximumSymbol->fillColor();
    }

    int red,green,blue;

    if((maxvalue - minvalue)!=0)
    {
      red = int (maxcolor.red() * (fvalue - minvalue) / (maxvalue - minvalue) + mincolor.red() * (maxvalue - fvalue) / (maxvalue - minvalue));
      green = int (maxcolor.green() * (fvalue - minvalue) / (maxvalue - minvalue) + mincolor.green() * (maxvalue - fvalue) / (maxvalue - minvalue));
      blue =  int (maxcolor.blue() * (fvalue - minvalue) / (maxvalue - minvalue) + mincolor.blue() * (maxvalue - fvalue) / (maxvalue - minvalue));
    }
    else
    {
      red = int (mincolor.red());
      green = int (mincolor.green());
      blue = int (mincolor.blue());
    }

    if ( mVectorType == QGis::Point && img) {
      // TODO we must get always new marker -> slow, but continuous color for points 
      // probably is not used frequently


      // Use color for both pen and brush (user can add another layer with outpine)
      // later add support for both pen and brush to dialog
      QPen pen = mMinimumSymbol->pen();
      pen.setColor ( QColor(red, green, blue) );
      pen.setWidthF ( widthScale * pen.widthF() );

      QBrush brush = mMinimumSymbol->brush();

      if ( selected ) {
        pen.setColor ( mSelectionColor );
        brush.setColor ( mSelectionColor );
      } else {
        brush.setColor ( QColor(red, green, blue) );
      }
      brush.setStyle ( Qt::SolidPattern );

      *img = QgsMarkerCatalogue::instance()->imageMarker ( mMinimumSymbol->pointSymbolName(), mMinimumSymbol->pointSize() *widthScale * rasterScaleFactor,
          pen, brush);
    } 
    else if ( mVectorType == QGis::Line )
    {
      QPen linePen;
      linePen.setColor(QColor(red, green, blue));
      linePen.setWidthF(widthScale*mMinimumSymbol->pen().widthF());
      p->setPen(linePen);
    } 
    else
    {
      p->setBrush(QColor(red, green, blue));
      if (mDrawPolygonOutline)
      {
        QPen pen;
        pen.setColor(QColor(0,0,0));
        pen.setWidthF(widthScale*mMinimumSymbol->pen().widthF());
        p->setPen(pen);
      }
      else
        p->setPen(Qt::NoPen);
    }
    if(selected)
    {
      QPen pen=mMinimumSymbol->pen();
      pen.setColor(mSelectionColor);
      QBrush brush=mMinimumSymbol->brush();
      brush.setColor(mSelectionColor);
      p->setPen(pen);
      p->setBrush(brush);
    }
  }
}

void QgsContinuousColorRenderer::readXML(const QDomNode& rnode, QgsVectorLayer& vl)
{
    mVectorType = vl.vectorType();
    QDomNode classnode = rnode.namedItem("classificationfield");
    int classificationfield = classnode.toElement().text().toInt();
    this->setClassificationField(classificationfield);
    
    //polygon outline
    QDomNode polyoutlinenode = rnode.namedItem("polygonoutline");
    QString polyoutline = polyoutlinenode.toElement().text();
    if(polyoutline == "0")
    {
	    mDrawPolygonOutline = false;
    }
    else if(polyoutline == "1")
    {
	    mDrawPolygonOutline = true;
    }

    //read the settings for the renderitem of the minimum value
    QDomNode lowernode = rnode.namedItem("lowestsymbol");
    QDomNode lsymbolnode = lowernode.namedItem("symbol");
    if( ! lsymbolnode.isNull() )
    {
	QgsSymbol* lsy = new QgsSymbol(mVectorType);
	lsy->readXML ( lsymbolnode );
	this->setMinimumSymbol(lsy);
    }
    QDomNode uppernode = rnode.namedItem("highestsymbol");
    QDomNode usymbolnode = uppernode.namedItem("symbol");
    if( ! usymbolnode.isNull() )
    {
	QgsSymbol* usy = new QgsSymbol(mVectorType);
	usy->readXML ( usymbolnode );
	this->setMaximumSymbol(usy);
    }
    vl.setRenderer(this);
}

QgsAttributeList QgsContinuousColorRenderer::classificationAttributes() const
{
    QgsAttributeList list;
    list.append(mClassificationField);
    return list;
}

QString QgsContinuousColorRenderer::name() const
{
    return "Continuous Color";
}

bool QgsContinuousColorRenderer::writeXML( QDomNode & layer_node, QDomDocument & document ) const
{
    bool returnval=true;
#ifndef WIN32
    QDomElement continuoussymbol=document.createElement("continuoussymbol");
    layer_node.appendChild(continuoussymbol);
    QDomElement classificationfield=document.createElement("classificationfield");
    QDomText classificationfieldtxt=document.createTextNode(QString::number(mClassificationField));
    classificationfield.appendChild(classificationfieldtxt);
    continuoussymbol.appendChild(classificationfield);
    
    //polygon outlines
    QDomElement drawPolygonOutlines = document.createElement("polygonoutline");
    int drawPolyInt = mDrawPolygonOutline ? 1 : 0;
    QDomText drawPolygonText = document.createTextNode(QString::number(drawPolyInt));
    drawPolygonOutlines.appendChild(drawPolygonText);
    continuoussymbol.appendChild(drawPolygonOutlines);
    
    QDomElement lowestsymbol=document.createElement("lowestsymbol");
    continuoussymbol.appendChild(lowestsymbol);
    if(mMinimumSymbol)
    {
	mMinimumSymbol->writeXML(lowestsymbol,document);
    }
    QDomElement highestsymbol=document.createElement("highestsymbol");
    continuoussymbol.appendChild(highestsymbol);
    if(mMaximumSymbol)
    {
	mMaximumSymbol->writeXML(highestsymbol,document);
    }
#endif
    return returnval;
}

const QList<QgsSymbol*> QgsContinuousColorRenderer::symbols() const
{
    QList<QgsSymbol*> list;
    list.append(mMinimumSymbol);
    list.append(mMaximumSymbol);
    return list;
}

QgsRenderer* QgsContinuousColorRenderer::clone() const
{
    QgsContinuousColorRenderer* r = new QgsContinuousColorRenderer(*this);
    return r;
}
