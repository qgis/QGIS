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
#include "qgsgraduatedsymbolrenderer.h"
#include "qgssymbol.h"
#include "qgssymbologyutils.h"
#include "qgsvectorlayer.h"
#include <math.h>
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
  const QList<QgsSymbol*> s = other.symbols();
  for(QList<QgsSymbol*>::const_iterator it=s.begin(); it!=s.end(); ++it)
  {
    addSymbol(new QgsSymbol(**it));
  }
  updateSymbolAttributes();
}

QgsGraduatedSymbolRenderer& QgsGraduatedSymbolRenderer::operator=(const QgsGraduatedSymbolRenderer& other)
{
  if(this != &other)
  {
    mVectorType = other.mVectorType; 
    mClassificationField = other.mClassificationField;
    removeSymbols();
    const QList<QgsSymbol*> s = other.symbols();
    for(QList<QgsSymbol*>::const_iterator it=s.begin(); it!=s.end(); ++it)
    {
      addSymbol(new QgsSymbol(**it));
    }
    updateSymbolAttributes();
  }

  return *this;
}

QgsGraduatedSymbolRenderer::~QgsGraduatedSymbolRenderer()
{

}

const QList<QgsSymbol*> QgsGraduatedSymbolRenderer::symbols() const
{
  return mSymbols;
}

void QgsGraduatedSymbolRenderer::removeSymbols()
{
  //free the memory first
  for (QList<QgsSymbol*>::iterator it = mSymbols.begin(); it != mSymbols.end(); ++it)
  {
    delete *it;
  }

  //and remove the pointers then
  mSymbols.clear();
  updateSymbolAttributes();
}

bool QgsGraduatedSymbolRenderer::willRenderFeature(QgsFeature *f)
{
  return (symbolForFeature(f) != 0);
}

void QgsGraduatedSymbolRenderer::renderFeature(QPainter * p, QgsFeature & f, QImage* img, bool selected, double widthScale, double rasterScaleFactor)
{
  QgsSymbol* theSymbol = symbolForFeature(&f);
  if(!theSymbol)
    {
      if ( img && mVectorType == QGis::Point )
	{
	  img->fill(0);
	}
      else if ( mVectorType != QGis::Point )
	{
	  p->setPen(Qt::NoPen);
	  p->setBrush(Qt::NoBrush);
	}
      return;
    }

  //set the qpen and qpainter to the right values
  // Point 
  if ( img && mVectorType == QGis::Point ) 
  {
    double fieldScale = 1.0;
    double rotation = 0.0;

    if ( theSymbol->scaleClassificationField() >= 0)
    {
      //first find out the value for the scale classification attribute
      const QgsAttributeMap& attrs = f.attributeMap();
      fieldScale = sqrt(fabs(attrs[theSymbol->scaleClassificationField()].toDouble()));
      QgsDebugMsg(QString("Feature has field scale factor %1").arg(fieldScale));
    }
    if ( theSymbol->rotationClassificationField() >= 0 )
    {
      const QgsAttributeMap& attrs = f.attributeMap();
      rotation = attrs[theSymbol->rotationClassificationField()].toDouble();
      QgsDebugMsg(QString("Feature has rotation factor %1").arg(rotation));
    }
    *img = theSymbol->getPointSymbolAsImage( widthScale, selected, mSelectionColor, fieldScale, rotation, rasterScaleFactor);
  } 

  // Line, polygon
  if ( mVectorType != QGis::Point )
    {
      if( !selected ) 
	{
	  QPen pen=theSymbol->pen();
	  pen.setWidthF ( widthScale * pen.widthF() );
	  p->setPen(pen);
	  p->setBrush(theSymbol->brush());
	}
      else
	{
	  QPen pen=theSymbol->pen();
	  pen.setColor(mSelectionColor);
	  pen.setWidthF ( widthScale * pen.widthF() );
	  QBrush brush=theSymbol->brush();
	  brush.setColor(mSelectionColor);
	  p->setPen(pen);
	  p->setBrush(brush);
	}
    }
}

QgsSymbol *QgsGraduatedSymbolRenderer::symbolForFeature(const QgsFeature* f)
{
  //first find out the value for the classification attribute
  const QgsAttributeMap& attrs = f->attributeMap();
  double value = attrs[mClassificationField].toDouble();

  QList<QgsSymbol*>::iterator it;
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
    return 0;
  }
  return (*it);
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
  updateSymbolAttributes();
  vl.setRenderer(this);
}

QgsAttributeList QgsGraduatedSymbolRenderer::classificationAttributes() const
{
  QgsAttributeList list(mSymbolAttributes);
  if ( ! list.contains(mClassificationField) )
  {
    list.append(mClassificationField);
  }
  return list;
}

void QgsGraduatedSymbolRenderer::updateSymbolAttributes()
{
  // This function is only called after changing field specifier in the GUI.
  // Timing is not so important.

  mSymbolAttributes.clear();

  QList<QgsSymbol*>::iterator it;
  for (it = mSymbols.begin(); it != mSymbols.end(); ++it)
  {
    int rotationField = (*it)->rotationClassificationField();
    if ( rotationField >= 0 && !(mSymbolAttributes.contains(rotationField)) )
    {
      mSymbolAttributes.append(rotationField);
    }
    int scaleField = (*it)->scaleClassificationField(); 
    if ( scaleField >= 0 && !(mSymbolAttributes.contains(scaleField)) )
    {
      mSymbolAttributes.append(scaleField);
    }
  }
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
  for (QList<QgsSymbol*>::const_iterator it = mSymbols.begin(); it != mSymbols.end(); ++it)
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
