/***************************************************************************
                         qgscontinuouscolrenderer.cpp  -  description
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
/* $Id$ */
#include "qgscontinuouscolrenderer.h"
#include "qgis.h"
#include "qgsvectorlayer.h"
#include <cfloat>
#include "qgslegenditem.h"
#include "qgscontcoldialog.h"
#include "qgssymbologyutils.h"
#include "qgsmarkercatalogue.h"
#include "qgssymbol.h"
#include <qdom.h>

QgsContinuousColRenderer::QgsContinuousColRenderer(QGis::VectorType type): mMinimumSymbol(0), mMaximumSymbol(0)
{
    mVectorType = type;
    //call superclass method to set up selection colour
    initialiseSelectionColor();
}

QgsContinuousColRenderer::QgsContinuousColRenderer(const QgsContinuousColRenderer& other)
{
    mVectorType = other.mVectorType;
    mClassificationField = other.mClassificationField;
    mMinimumSymbol = new QgsSymbol(*other.mMinimumSymbol);
    mMaximumSymbol = new QgsSymbol(*other.mMaximumSymbol);
}

QgsContinuousColRenderer& QgsContinuousColRenderer::operator=(const QgsContinuousColRenderer& other)
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
}

QgsContinuousColRenderer::~QgsContinuousColRenderer()
{
  delete mMinimumSymbol;
  delete mMaximumSymbol;
}

void QgsContinuousColRenderer::setMinimumSymbol(QgsSymbol* sy)
{
  delete mMinimumSymbol;
  mMinimumSymbol = sy;
}

void QgsContinuousColRenderer::setMaximumSymbol(QgsSymbol* sy)
{
  delete mMaximumSymbol;
  mMaximumSymbol = sy;
}

void QgsContinuousColRenderer::renderFeature(QPainter * p, QgsFeature * f, QPicture* pic, 
	double* scalefactor, bool selected, int oversampling, double widthScale)
{
    if ((mMinimumSymbol && mMaximumSymbol))
    {
  //first find out the value for the classification attribute
  std::vector < QgsFeatureAttribute > vec = f->attributeMap();
  double fvalue = vec[0].fieldValue().toDouble();
  
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
      
  if ( mVectorType == QGis::Point && pic) {
      // TODO we must get always new marker -> slow, but continuous color for points 
      // probably is not used frequently


      // Use color for both pen and brush (user can add another layer with outpine)
      // later add support for both pen and brush to dialog
      QPen pen = mMinimumSymbol->pen();
      pen.setColor ( QColor(red, green, blue) );
      pen.setWidth ( (int) ( widthScale * pen.width() ) );

      QBrush brush = mMinimumSymbol->brush();

      if ( selected ) {
          pen.setColor ( mSelectionColor );
          brush.setColor ( mSelectionColor );
      } else {
          brush.setColor ( QColor(red, green, blue) );
      }
      brush.setStyle ( Qt::SolidPattern );
      
      // Always with oversampling 0
      *pic = QgsMarkerCatalogue::instance()->marker ( mMinimumSymbol->pointSymbolName(), mMinimumSymbol->pointSize(),
	                                      pen, brush, 0 );

      if ( scalefactor ) *scalefactor = 1;
  } 
  else if ( mVectorType == QGis::Line )
  {
      p->setPen( QPen(QColor(red, green, blue),
		 (int)(widthScale*mMinimumSymbol->pen().width()) ));//make sure the correct line width is used
  } 
  else
        {
      p->setBrush(QColor(red, green, blue));
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

void QgsContinuousColRenderer::readXML(const QDomNode& rnode, QgsVectorLayer& vl)
{
    mVectorType = vl.vectorType();
    QDomNode classnode = rnode.namedItem("classificationfield");
    int classificationfield = classnode.toElement().text().toInt();
    this->setClassificationField(classificationfield);

    //read the settings for the renderitem of the minimum value
    QDomNode lowernode = rnode.namedItem("lowestsymbol");
    QDomNode lsymbolnode = lowernode.namedItem("symbol");
    if( ! lsymbolnode.isNull() )
    {
	QgsSymbol* lsy = new QgsSymbol();
	lsy->readXML ( lsymbolnode );
	this->setMinimumSymbol(lsy);
    }
    QDomNode uppernode = rnode.namedItem("highestsymbol");
    QDomNode usymbolnode = uppernode.namedItem("symbol");
    if( ! usymbolnode.isNull() )
    {
	QgsSymbol* usy = new QgsSymbol();
	usy->readXML ( usymbolnode );
	this->setMaximumSymbol(usy);
    }
    vl.setRenderer(this);
}

std::list<int> QgsContinuousColRenderer::classificationAttributes() const
{
    std::list<int> list;
    list.push_back(mClassificationField);
    return list;
}

QString QgsContinuousColRenderer::name() const
{
    return "Continuous Color";
}

bool QgsContinuousColRenderer::writeXML( QDomNode & layer_node, QDomDocument & document ) const
{
    bool returnval=true;
    QDomElement continuoussymbol=document.createElement("continuoussymbol");
    layer_node.appendChild(continuoussymbol);
    QDomElement classificationfield=document.createElement("classificationfield");
    QDomText classificationfieldtxt=document.createTextNode(QString::number(mClassificationField));
    classificationfield.appendChild(classificationfieldtxt);
    continuoussymbol.appendChild(classificationfield);
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
    return returnval;
}

const std::list<QgsSymbol*> QgsContinuousColRenderer::symbols() const
{
    std::list<QgsSymbol*> list;
    list.push_back(mMinimumSymbol);
    list.push_back(mMaximumSymbol);
    return list;
}

QgsRenderer* QgsContinuousColRenderer::clone() const
{
    QgsContinuousColRenderer* r = new QgsContinuousColRenderer(*this);
    return r;
}
