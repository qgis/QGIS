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
#include <cfloat>
#include "qgis.h"
#include "qgsfeature.h"
#include "qgsgraduatedsymrenderer.h"
#include "qgsgrasydialog.h"
#include "qgslegenditem.h"
#include "qgssymbologyutils.h"
#include "qgssvgcache.h"
#include <qdom.h>
#include <qpixmap.h>
#include <qpicture.h>


QgsGraduatedSymRenderer::QgsGraduatedSymRenderer(QGis::VectorType type)
{
    mVectorType=type;
    //call superclass method to set up selection colour
    initialiseSelectionColor();
}

QgsGraduatedSymRenderer::QgsGraduatedSymRenderer(const QgsGraduatedSymRenderer& other)
{
    mVectorType = other.mVectorType;
    mClassificationField = other.mClassificationField;
    const std::list<QgsSymbol*> s = other.symbols();
    for(std::list<QgsSymbol*>::const_iterator it=s.begin(); it!=s.end(); ++it)
    {
	addSymbol(new QgsSymbol(**it));
    }
}

QgsGraduatedSymRenderer& QgsGraduatedSymRenderer::operator=(const QgsGraduatedSymRenderer& other)
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
}

QgsGraduatedSymRenderer::~QgsGraduatedSymRenderer()
{
 
}

const std::list<QgsSymbol*> QgsGraduatedSymRenderer::symbols() const
{
    return mSymbols;
}

void QgsGraduatedSymRenderer::removeSymbols()
{
    //free the memory first
    for (std::list < QgsSymbol * >::iterator it = mSymbols.begin(); it != mSymbols.end(); ++it)
    {
	delete *it;
    }

    //and remove the pointers then
    mSymbols.clear();
}

void QgsGraduatedSymRenderer::renderFeature(QPainter * p, QgsFeature * f, QPicture* pic, 
	double* scalefactor, bool selected, int oversampling, double widthScale)
{
    //first find out the value for the classification attribute
    std::vector < QgsFeatureAttribute > vec = f->attributeMap();
    double value = vec[0].fieldValue().toDouble();

    std::list < QgsSymbol* >::iterator it;
    //find the first render item which contains the feature
    for (it = mSymbols.begin(); it != mSymbols.end(); ++it)
    {
	if (value >= (*it)->lowerValue().toDouble() && value <= (*it)->upperValue().toDouble())
        {
	    break;
        }
    }
    
    if (it == mSymbols.end())      //value is contained in no item
    {
	std::cout << "Warning, value is contained in no class" << std::endl << std::flush;
	return;
    } 
    else
    {
	//set the qpen and qpainter to the right values
	// Point 
	if ( pic && mVectorType == QGis::Point ) 
	{
	    *pic = (*it)->getPointSymbolAsPicture( oversampling, widthScale,
		                                             selected, mSelectionColor );
	    
	    if ( scalefactor ) *scalefactor = 1;
	} 

        // Line, polygon
 	if ( mVectorType != QGis::Point )
	{
	    if( !selected ) 
	    {
		QPen pen=(*it)->pen();
		pen.setWidth ( (int) (widthScale * pen.width()) );
		p->setPen(pen);
		p->setBrush((*it)->brush());
	    }
	    else
	    {
		QPen pen=(*it)->pen();
		pen.setColor(mSelectionColor);
		pen.setWidth ( (int) (widthScale * pen.width()) );
		QBrush brush=(*it)->brush();
		brush.setColor(mSelectionColor);
		p->setPen(pen);
		p->setBrush(brush);
	    }
	}
    }
}

void QgsGraduatedSymRenderer::readXML(const QDomNode& rnode, QgsVectorLayer& vl)
{
    mVectorType = vl.vectorType();
    QDomNode classnode = rnode.namedItem("classificationfield");
    int classificationfield = classnode.toElement().text().toInt();
   
    this->setClassificationField(classificationfield);

    QDomNode symbolnode = rnode.namedItem("symbol");
    while (!symbolnode.isNull())
    {
	QgsSymbol* sy = new QgsSymbol();
	sy->readXML ( symbolnode );
	this->addSymbol(sy);

	symbolnode = symbolnode.nextSibling();
    }

    vl.setRenderer(this);
}

std::list<int> QgsGraduatedSymRenderer::classificationAttributes() const
{
    std::list<int> list;
    list.push_back(mClassificationField);
    return list;
}

QString QgsGraduatedSymRenderer::name() const
{
    return "Graduated Symbol";
}

bool QgsGraduatedSymRenderer::writeXML( QDomNode & layer_node, QDomDocument & document ) const
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

QgsRenderer* QgsGraduatedSymRenderer::clone() const
{
    QgsGraduatedSymRenderer* r = new QgsGraduatedSymRenderer(*this);
    return r;
}
