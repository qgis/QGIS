/***************************************************************************
                         qgssinglesymbolrenderer.cpp  -  description
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
/* $Id: qgsgraduatedsymbolrenderer.cpp 5371 2006-04-25 01:52:13Z wonder $ */

#include "qgis.h"
#include "qgslogger.h"
#include "qgsfeature.h"
#include "qgsfeatureattribute.h"
#include "qgsgraduatedsymbolrenderer.h"
#include "qgssymbol.h"
#include "qgssymbologyutils.h"
#include "qgsvectorlayer.h"

#include <QDomNode>
#include <QDomElement>
#include <QImage>
#include <QPainter>


QgsGraduatedSymbolRenderer::QgsGraduatedSymbolRenderer(QGis::VectorType type)
{
    mVectorType=type;
}

QgsGraduatedSymbolRenderer::QgsGraduatedSymbolRenderer(const QgsGraduatedSymbolRenderer& other)
{
    mVectorType = other.mVectorType;
    mClassificationField = other.mClassificationField;
    const std::list<QgsSymbol*> s = other.symbols();
    for(std::list<QgsSymbol*>::const_iterator it=s.begin(); it!=s.end(); ++it)
    {
	addSymbol(new QgsSymbol(**it));
    }
}

QgsGraduatedSymbolRenderer& QgsGraduatedSymbolRenderer::operator=(const QgsGraduatedSymbolRenderer& other)
{
    if(this != &other)
    {
        mVectorType = other.mVectorType; 
        mClassificationField = other.mClassificationField;
        removeSymbols();
        const std::list<QgsSymbol*> s = other.symbols();
        for(std::list<QgsSymbol*>::const_iterator it=s.begin(); it!=s.end(); ++it)
        {
            addSymbol(new QgsSymbol(**it));
        }
    }

    return *this;
}

QgsGraduatedSymbolRenderer::~QgsGraduatedSymbolRenderer()
{
 
}

const std::list<QgsSymbol*> QgsGraduatedSymbolRenderer::symbols() const
{
    return mSymbols;
}

void QgsGraduatedSymbolRenderer::removeSymbols()
{
    //free the memory first
    for (std::list < QgsSymbol * >::iterator it = mSymbols.begin(); it != mSymbols.end(); ++it)
    {
	delete *it;
    }

    //and remove the pointers then
    mSymbols.clear();
}

void QgsGraduatedSymbolRenderer::renderFeature(QPainter * p, QgsFeature & f, QImage* img, 
	double* scalefactor, bool selected, double widthScale)
{
  //first find out the value for the classification attribute
  const QgsAttributeMap& attrs = f.attributeMap();
  double value = attrs[mClassificationField].fieldValue().toDouble();

  std::list < QgsSymbol* >::iterator it;
  //find the first render item which contains the feature
  for (it = mSymbols.begin(); it != mSymbols.end(); ++it)
  {
    if (value >= (*it)->lowerValue().toDouble() && value <= (*it)->upperValue().toDouble())
    {
      break;
    }
  }

  if (it == mSymbols.end())      //only draw features which are covered by a render item
  {
    QgsDebugMsg("Warning, value is contained in no class");
    p->setPen(QPen(Qt::NoPen));
    p->setBrush(QBrush(Qt::NoBrush));
    return;
  } 
  else
  {
    //set the qpen and qpainter to the right values
    // Point 
    if ( img && mVectorType == QGis::Point ) 
    {
      *img = (*it)->getPointSymbolAsImage(  widthScale,
          selected, mSelectionColor );

      if ( scalefactor ) *scalefactor = 1;
    } 

    // Line, polygon
    if ( mVectorType != QGis::Point )
    {
      if( !selected ) 
      {
        QPen pen=(*it)->pen();
        pen.setWidthF ( widthScale * pen.width() );
        p->setPen(pen);
        p->setBrush((*it)->brush());
      }
      else
      {
        QPen pen=(*it)->pen();
        pen.setColor(mSelectionColor);
        pen.setWidthF ( widthScale * pen.width() );
        QBrush brush=(*it)->brush();
        brush.setColor(mSelectionColor);
        p->setPen(pen);
        p->setBrush(brush);
      }
    }
  }
}

void QgsGraduatedSymbolRenderer::readXML(const QDomNode& rnode, QgsVectorLayer& vl)
{
    mVectorType = vl.vectorType();
    QDomNode classnode = rnode.namedItem("classificationfield");
    int classificationfield = classnode.toElement().text().toInt();
   
    this->setClassificationField(classificationfield);

    QDomNode symbolnode = rnode.namedItem("symbol");
    while (!symbolnode.isNull())
    {
	QgsSymbol* sy = new QgsSymbol(mVectorType);
	sy->readXML ( symbolnode );
	this->addSymbol(sy);

	symbolnode = symbolnode.nextSibling();
    }

    vl.setRenderer(this);
}

QgsAttributeList QgsGraduatedSymbolRenderer::classificationAttributes() const
{
    QgsAttributeList list;
    list.append(mClassificationField);
    return list;
}

QString QgsGraduatedSymbolRenderer::name() const
{
    return "Graduated Symbol";
}

bool QgsGraduatedSymbolRenderer::writeXML( QDomNode & layer_node, QDomDocument & document ) const
{
    bool returnval=true;
    QDomElement graduatedsymbol=document.createElement("graduatedsymbol");
    layer_node.appendChild(graduatedsymbol);
    QDomElement classificationfield=document.createElement("classificationfield");
    QDomText classificationfieldtxt=document.createTextNode(QString::number(mClassificationField));
    classificationfield.appendChild(classificationfieldtxt);
    graduatedsymbol.appendChild(classificationfield);
    for(std::list<QgsSymbol*>::const_iterator it=mSymbols.begin();it!=mSymbols.end();++it)
    {
	if(!(*it)->writeXML(graduatedsymbol,document))
	{
	    returnval=false;
	}
    }
    return returnval;
}

QgsRenderer* QgsGraduatedSymbolRenderer::clone() const
{
    QgsGraduatedSymbolRenderer* r = new QgsGraduatedSymbolRenderer(*this);
    return r;
}
